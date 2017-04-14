/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Object.hpp>
#include <zeus/File.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>
#include <echrono/Time.hpp>

#include <mutex>
#include <ejson/ejson.hpp>
#include <etk/os/FSNode.hpp>
#include <sstream>

#include <etk/stdTools.hpp>

#include <zeus/service/Video.hpp>
#include <zeus/service/registerVideo.hpp>
#include <zeus/ProxyClientProperty.hpp>
#include <zeus/File.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/zeus-Media.impl.hpp>

static std::mutex g_mutex;
static std::string g_basePath;
static std::string g_baseDBName = std::string(SERVICE_NAME) + "-database.json";

static std::vector<ememory::SharedPtr<zeus::MediaImpl>> m_listFile;

static uint64_t m_lastMaxId = 0;
static bool g_needToStore = false;

static uint64_t createUniqueID() {
	m_lastMaxId++;
	return m_lastMaxId;
}


static std::string removeSpaceOutQuote(const std::string& _in) {
	std::string out;
	bool insideQuote = false;
	for (auto &it : _in) {
		if (it == '\'') {
			if (insideQuote == false) {
				insideQuote = true;
			} else {
				insideQuote = false;
			}
			out += it;
		} else if (    it == ' '
		            && insideQuote == false) {
			// nothing to add ...
		} else {
			out += it;
		}
	}
	return out;
}

static std::vector<std::string> splitAction(const std::string& _in) {
	std::vector<std::string> out;
	bool insideQuote = false;
	std::string value;
	for (auto &it : _in) {
		if (it == '\'') {
			if (insideQuote == false) {
				insideQuote = true;
			} else {
				insideQuote = false;
			}
			if (value != "") {
				out.push_back(value);
				value.clear();
			}
		} else {
			value += it;
		}
	}
	if (value != "") {
		out.push_back(value);
	}
	return out;
}

namespace appl {
	class VideoService : public zeus::service::Video  {
		private:
			//ememory::SharedPtr<zeus::ClientProperty>& m_client;
			zeus::ProxyClientProperty m_client;
			std::string m_userName;
		public:
			/*
			VideoService(ememory::SharedPtr<zeus::ClientProperty>& _client, const std::string& _userName) :
			  m_client(_client),
			  m_userName(_userName) {
				APPL_WARNING("New VideoService ... for user: ");
			}
			*/
			VideoService(uint16_t _clientId) {
				APPL_VERBOSE("New VideoService ... for user: " << _clientId);
			}
			~VideoService() {
				APPL_VERBOSE("delete VideoService ...");
			}
		public:
			uint32_t count() override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return m_listFile.size();
			}
			
			std::vector<uint32_t> getIds(uint32_t _start, uint32_t _stop) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				std::vector<uint32_t> out;
				for (size_t iii=_start; iii<m_listFile.size() && iii<_stop; ++iii) {
					if (m_listFile[iii] == nullptr) {
						continue;
					}
					out.push_back(m_listFile[iii]->getUniqueId());
				}
				return out;
			}
			
			// Return a File Data (might be a video .tiff/.png/.jpg)
			ememory::SharedPtr<zeus::Media> get(uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				ememory::SharedPtr<zeus::MediaImpl> property;
				for (auto &it : m_listFile) {
					if (it == nullptr) {
						continue;
					}
					if (it->getUniqueId() == _mediaId) {
						return it;
					}
				}
				throw std::invalid_argument("Wrong file ID ...");
				//
			}
			uint32_t add(zeus::ProxyFile _dataFile) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				uint64_t id = createUniqueID();
				auto futRemoteSha512 = _dataFile.getSha512();
				auto futType = _dataFile.getMineType();
				auto futName = _dataFile.getName();
				// wait the sha1 to check his existance:
				futRemoteSha512.wait();
				std::string sha512StringRemote = futRemoteSha512.get();
				for (auto &it : m_listFile) {
					if (it == nullptr) {
						continue;
					}
					if (it->getFileName() == sha512StringRemote) {
						APPL_INFO("File already registered at ");// << it.m_creationData);
						// simply send the Id of the file
						// TODO : Check right of this file ...
						return it->getUniqueId();
					}
				}
				std::string tmpFileName = g_basePath + "tmpImport_" + etk::to_string(id);
				std::string sha512String = zeus::storeInFile(_dataFile, tmpFileName);
				futType.wait();
				futName.wait();
				// move the file at the good position:
				APPL_DEBUG("move temporay file in : " << g_basePath << sha512String);
				if (etk::FSNodeGetSize(tmpFileName) == 0) {
					APPL_ERROR("try to store an empty file");
					throw std::runtime_error("file size == 0");
				}
				etk::FSNodeMove(tmpFileName, g_basePath + sha512String + "." + zeus::getExtention(futType.get()));
				ememory::SharedPtr<zeus::MediaImpl> property = ememory::makeShared<zeus::MediaImpl>(id, sha512String + "." + zeus::getExtention(futType.get()));
				//property->setName(futName.get());
				m_listFile.push_back(property);
				g_needToStore = true;
				APPL_DEBUG(" filename : " << sha512String);
				return id;
			}
			void remove(uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				//Check if the file exist:
				bool find = false;
				ememory::SharedPtr<zeus::MediaImpl> property;
				for (auto it = m_listFile.begin();
				     it != m_listFile.end();
				     /* No increment */) {
					if (*it == nullptr) {
						it = m_listFile.erase(it);
						continue;
					}
					if ((*it)->getUniqueId() == _mediaId) {
						property = *it;
						it = m_listFile.erase(it);
						find = true;
					} else {
						++it;
					}
				}
				if (find == false) {
					throw std::invalid_argument("Wrong file name ...");
				}
				// Real Remove definitly the file
				// TODO : Set it in a trash ... For a while ...
				if (property->erase() == false) {
					throw std::runtime_error("Can not remove file ...");
				}
			}
			
			std::vector<uint32_t> getSQL(std::string _sqlLikeRequest) override {
				std::vector<uint32_t> out;
				/*
				if (_sqlLikeRequest == "") {
					throw std::invalid_argument("empty request");
				}
				std::vector<std::vector<std::string>> listAndParsed;
				if (_sqlLikeRequest != "*") {
					std::vector<std::string> listAnd = etk::split(_sqlLikeRequest, "AND");
					APPL_INFO("Find list AND : ");
					for (auto &it : listAnd) {
						it = removeSpaceOutQuote(it);
						std::vector<std::string> elements = splitAction(it);
						if (elements.size() != 3) {
							APPL_ERROR("element : '" + it + "' have wrong spliting " << elements);
							throw std::invalid_argument("element : \"" + it + "\" have wrong spliting");
						}
						if (    elements[1] != "=="
						     && elements[1] != "!="
						     && elements[1] != ">="
						     && elements[1] != "<="
						     && elements[1] != ">"
						     && elements[1] != "<") {
							throw std::invalid_argument("action invalid : '" + elements[1] + "' only availlable : [==,!=,<=,>=,<,>]");
						}
						APPL_INFO("    - '" << elements[0] << "' action='" << elements[1] << "' with='" << elements[2] << "'");
						listAndParsed.push_back(elements);
					}
				}
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listFile) {
					bool isCorrectElement = true;
					for (auto &itCheck : listAndParsed) {
						// find matadataValue:
						auto itM = it.m_metadata.find(itCheck[0]);
						if (itM == it.m_metadata.end()) {
							// not find key ==> no check to do ...
							isCorrectElement = false;
							break;
						}
						if (itCheck[1] == "==") {
							if (itM->second != itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						} else if (itCheck[1] == "!=") {
							if (itM->second == itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						} else if (itCheck[1] == "<=") {
							if (itM->second < itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						} else if (itCheck[1] == ">=") {
							if (itM->second > itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						} else if (itCheck[1] == "<") {
							if (itM->second <= itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						} else if (itCheck[1] == ">") {
							if (itM->second >= itCheck[2]) {
								isCorrectElement = false;
								break;
							}
						}
					}
					if (isCorrectElement == true) {
						out.push_back(it.m_id);
					}
				}
				*/
				return out;
			}
			
			std::vector<std::string> getMetadataValuesWhere(std::string _keyName, std::string _sqlLikeRequest) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::vector<std::string> out;
				
				return out;
			}
			
	};
}

static void store_db() {
	APPL_ERROR("Store database [START]");
	ejson::Document database;
	ejson::Array listFilesArray;
	database.add("list-files", listFilesArray);
	for (auto &it : m_listFile) {
		if (it != nullptr) {
			listFilesArray.add(it->getJson());
		}
	}
	bool retGenerate = database.storeSafe(g_basePath + g_baseDBName);
	APPL_ERROR("Store database [STOP] : " << (g_basePath + g_baseDBName) << " ret = " << retGenerate);
	g_needToStore = false;
}

static void load_db() {
	ejson::Document database;
	bool ret = database.load(g_basePath + g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	ejson::Array listFilesArray = database["list-files"].toArray();
	for (const auto itArray: listFilesArray) {
		auto property = ememory::makeShared<zeus::MediaImpl>(g_basePath, itArray.toObject());
		if (property == nullptr) {
			APPL_ERROR("can not allocate element ...");
			continue;
		}
		if (m_lastMaxId < property->getUniqueId()) {
			m_lastMaxId = property->getUniqueId()+1;
		}
		if (property->getFileName() == "") {
			APPL_ERROR("Can not access on the file : ... No name ");
		} else {
			m_listFile.push_back(property);
		}
	}
	g_needToStore = false;
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], std::string _basePath) {
	g_basePath = _basePath;
	std::unique_lock<std::mutex> lock(g_mutex);
	APPL_WARNING("Load USER: " << g_basePath);
	load_db();
	APPL_WARNING("new USER: [STOP]");
	return true;
}

ETK_EXPORT_API bool SERVICE_IO_uninit() {
	std::unique_lock<std::mutex> lock(g_mutex);
	store_db();
	APPL_WARNING("delete USER [STOP]");
	return true;
}

ETK_EXPORT_API void SERVICE_IO_peridic_call() {
	if (g_needToStore == false) {
		return;
	}
	// try lock mutex:
	if (g_mutex.try_lock() == false) {
		return;
	}
	store_db();
	g_mutex.unlock();
}


ZEUS_SERVICE_VIDEO_DECLARE(appl::VideoService);

