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
static etk::String g_basePath;
static etk::String g_basePathCover;
static etk::String g_basePathCoverGroup;
static etk::String g_baseDBName = etk::String(SERVICE_NAME) + "-database.json";

static etk::Vector<ememory::SharedPtr<zeus::MediaImpl>> m_listFile;

static uint64_t m_lastMaxId = 0;
static uint64_t m_lastMaxImportId = 0;
static bool g_needToStore = false;

static uint64_t createUniqueID() {
	m_lastMaxId++;
	return m_lastMaxId;
}

static uint64_t createUniqueImportID() {
	m_lastMaxImportId++;
	return m_lastMaxImportId;
}

static etk::String removeSpaceOutQuote(const etk::String& _in) {
	etk::String out;
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

static etk::Vector<etk::String> splitAction(const etk::String& _in) {
	etk::Vector<etk::String> out;
	bool insideQuote = false;
	etk::String value;
	for (auto &it : _in) {
		if (it == '\'') {
			if (insideQuote == false) {
				insideQuote = true;
			} else {
				insideQuote = false;
			}
			if (value != "") {
				out.pushBack(value);
				value.clear();
			}
		} else {
			value += it;
		}
	}
	if (value != "") {
		out.pushBack(value);
	}
	return out;
}

static void metadataChange(zeus::MediaImpl* _element, const etk::String& _key) {
	g_needToStore = true;
	// meta_data have chage ==> we need to upfdate the path of the file where the data is stored ...
	if (_element == nullptr) {
		return;
	}
	_element->forceUpdateDecoratedName();
	etk::String current = _element->getFileName();
	etk::String next = _element->getDecoratedName() + "_" + _element->getSha512();
	if (next == current) {
		return;
	}
	_element->move(next);
}

namespace appl {
	class VideoService : public zeus::service::Video  {
		private:
			//ememory::SharedPtr<zeus::ClientProperty>& m_client;
			zeus::ProxyClientProperty m_client;
			etk::String m_userName;
		public:
			/*
			VideoService(ememory::SharedPtr<zeus::ClientProperty>& _client, const etk::String& _userName) :
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
			
			etk::Vector<uint32_t> getIds(uint32_t _start, uint32_t _stop) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				etk::Vector<uint32_t> out;
				for (size_t iii=_start; iii<m_listFile.size() && iii<_stop; ++iii) {
					if (m_listFile[iii] == nullptr) {
						continue;
					}
					out.pushBack(m_listFile[iii]->getUniqueId());
				}
				return out;
			}
			uint32_t getId(etk::String _sha512) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				uint32_t out;
				for (size_t iii=0; iii<m_listFile.size(); ++iii) {
					if (m_listFile[iii] == nullptr) {
						continue;
					}
					if (m_listFile[iii]->getSha512() == _sha512) {
						return m_listFile[iii]->getUniqueId();
					}
				}
				throw std::invalid_argument("sha512 not find...");
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
			uint32_t add(zeus::ActionNotification<etk::String>& _notifs, zeus::ProxyFile _dataFile) override {
				//_action.setProgress("{\"pourcent\":" + etk::toString(23.54) + ", \"comment\":\"Start loading file\"");
				//_action.setProgress("{\"pourcent\":" + etk::toString(23.54) + ", \"comment\":\"transfering file\"");;
				//_action.setProgress("{\"pourcent\":" + etk::toString(23.54) + ", \"comment\":\"synchronize meta-data\"");
				uint64_t id = 0;
				uint64_t importId = 0;
				{
					std::unique_lock<std::mutex> lock(g_mutex);
					// TODO : Check right ...
					id = createUniqueID();
					importId = createUniqueImportID();
				}
				auto futRemoteSha512 = _dataFile.getSha512();
				auto futType = _dataFile.getMineType();
				auto futName = _dataFile.getName();
				// wait the sha1 to check his existance:
				futRemoteSha512.wait();
				etk::String sha512StringRemote = futRemoteSha512.get();
				{
					std::unique_lock<std::mutex> lock(g_mutex);
					for (auto &it : m_listFile) {
						if (it == nullptr) {
							continue;
						}
						if (it->getSha512() == sha512StringRemote) {
							APPL_INFO("File already registered at ");// << it.m_creationData);
							// simply send the Id of the file
							// TODO : Check right of this file ...
							return it->getUniqueId();
						}
					}
				}
				etk::String tmpFileName = g_basePath + "tmpImport_" + etk::toString(importId);
				etk::String sha512String = zeus::storeInFile(_dataFile, tmpFileName);
				futType.wait();
				futName.wait();
				// move the file at the good position:
				APPL_DEBUG("move temporay file in : " << g_basePath << sha512String);
				if (etk::FSNodeGetSize(tmpFileName) == 0) {
					APPL_ERROR("try to store an empty file");
					throw std::runtime_error("file size == 0");
				}
				if (zeus::getExtention(futType.get()) != "") {
					std::unique_lock<std::mutex> lock(g_mutex);
					etk::FSNodeMove(tmpFileName, g_basePath + sha512String + "." + zeus::getExtention(futType.get()));
					ememory::SharedPtr<zeus::MediaImpl> property = ememory::makeShared<zeus::MediaImpl>(id, sha512String + "." + zeus::getExtention(futType.get()), g_basePath);
					property->setMetadata("sha512", sha512String);
					property->setMetadata("mime-type", futType.get());
					property->setCallbackMetadataChange(&metadataChange);
					m_listFile.pushBack(property);
					g_needToStore = true;
				} else {
					std::unique_lock<std::mutex> lock(g_mutex);
					etk::FSNodeMove(tmpFileName, g_basePath + sha512String);
					ememory::SharedPtr<zeus::MediaImpl> property = ememory::makeShared<zeus::MediaImpl>(id, sha512String, g_basePath);
					property->setMetadata("sha512", sha512String);
					property->setCallbackMetadataChange(&metadataChange);
					m_listFile.pushBack(property);
					g_needToStore = true;
				}
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
					g_needToStore = true;
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
			
			etk::Vector<etk::Vector<etk::String>> interpreteSQLRequest(const etk::String& _sqlLikeRequest) {
				etk::Vector<etk::Vector<etk::String>> out;
				if (_sqlLikeRequest != "*") {
					etk::Vector<etk::String> listAnd = etk::split(_sqlLikeRequest, "AND");
					APPL_INFO("Find list AND : ");
					for (auto &it : listAnd) {
						it = removeSpaceOutQuote(it);
						etk::Vector<etk::String> elements = splitAction(it);
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
						out.pushBack(elements);
					}
				}
				return out;
			}
			
			bool isValid(const etk::Vector<etk::Vector<etk::String>>& _listElement,
			             const etk::Map<etk::String, etk::String>& _metadata) {
				for (auto &itCheck : _listElement) {
					// find matadataValue:
					auto itM = _metadata.find(itCheck[0]);
					if (itM == _metadata.end()) {
						// not find key ==> no check to do ...
						return false;
					}
					if (itCheck[1] == "==") {
						if (itM->second != itCheck[2]) {
							return false;
						}
					} else if (itCheck[1] == "!=") {
						if (itM->second == itCheck[2]) {
							return false;
						}
					} else if (itCheck[1] == "<=") {
						if (itM->second < itCheck[2]) {
							return false;
						}
					} else if (itCheck[1] == ">=") {
						if (itM->second > itCheck[2]) {
							return false;
						}
					} else if (itCheck[1] == "<") {
						if (itM->second <= itCheck[2]) {
							return false;
						}
					} else if (itCheck[1] == ">") {
						if (itM->second >= itCheck[2]) {
							return false;
						}
					}
				}
				return true;
			}
			
			etk::String mapToString(const etk::Map<etk::String, etk::String>& _metadata) {
				etk::String out = "{";
				for (auto &it : _metadata) {
					out += it.first + ":" + it.second + ",";
				}
				out += "}";
				return out;
			}
			
			etk::Vector<uint32_t> getSQL(etk::String _sqlLikeRequest) override {
				etk::Vector<uint32_t> out;
				if (_sqlLikeRequest == "") {
					throw std::invalid_argument("empty request");
				}
				APPL_DEBUG("check : " << _sqlLikeRequest);
				etk::Vector<etk::Vector<etk::String>> listAndParsed = interpreteSQLRequest(_sqlLikeRequest);
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listFile) {
					if (it == nullptr) {
						continue;
					}
					APPL_DEBUG("    [" << it->getUniqueId() << "   list=" << mapToString(it->getMetadataDirect()));
					bool isCorrectElement = isValid(listAndParsed, it->getMetadataDirect());
					if (isCorrectElement == true) {
						APPL_DEBUG("        select");
						out.pushBack(it->getUniqueId());
					}
				}
				return out;
			}
			
			etk::Vector<etk::String> getMetadataValuesWhere(etk::String _keyName, etk::String _sqlLikeRequest) override {
				etk::Vector<etk::String> out;
				if (_sqlLikeRequest == "") {
					throw std::invalid_argument("empty request");
				}
				etk::Vector<etk::Vector<etk::String>> listAndParsed = interpreteSQLRequest(_sqlLikeRequest);
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listFile) {
					if (it == nullptr) {
						continue;
					}
					bool isCorrectElement = isValid(listAndParsed, it->getMetadataDirect());
					if (isCorrectElement == false) {
						continue;
					}
					auto it2 = it->getMetadataDirect().find(_keyName);
					if (it2 == it->getMetadataDirect().end()) {
						continue;
					}
					etk::String value = it2->second;
					isCorrectElement = false;
					for (auto &it2: out) {
						if (it2 == value) {
							isCorrectElement = true;
							break;
						}
					}
					if (isCorrectElement == false) {
						out.pushBack(value);
					}
				}
				return out;
			}
			
			ememory::SharedPtr<zeus::File> internalGetCover(const etk::String& _baseName, const etk::String& _mediaString, uint32_t _maxSize) {
				if (etk::FSNodeExist(_baseName + _mediaString + ".jpg") == true) {
					return zeus::File::create(_baseName + _mediaString + ".jpg");
				}
				if (etk::FSNodeExist(_baseName + _mediaString + ".png") == true) {
					return zeus::File::create(_baseName + _mediaString + ".png");
				}
				throw std::runtime_error("No cover availlable");
			}
			
			void internalSetCover(const etk::String& _baseName, zeus::ActionNotification<etk::String>& _notifs, zeus::ProxyFile _cover, etk::String _mediaString) {
				uint64_t importId = 0;
				{
					std::unique_lock<std::mutex> lock(g_mutex);
					importId = createUniqueImportID();
				}
				auto futType = _cover.getMineType();
				etk::String tmpFileName = g_basePath + "tmpImport_" + etk::toString(importId);
				etk::String sha512String = zeus::storeInFile(_cover, tmpFileName);
				futType.wait();
				if (etk::FSNodeGetSize(tmpFileName) == 0) {
					APPL_ERROR("try to store an empty file");
					throw std::runtime_error("file size == 0");
				}
				if (etk::FSNodeGetSize(tmpFileName) > 1024*1024) {
					APPL_ERROR("try to store a Bigger file");
					throw std::runtime_error("file size > 1Mo");
				}
				if (futType.get() == "image/png") {
					std::unique_lock<std::mutex> lock(g_mutex);
					etk::FSNodeRemove(_baseName + _mediaString + ".jpg");
					etk::FSNodeMove(tmpFileName, _baseName + _mediaString + ".png");
				} else if (futType.get() == "image/jpeg") {
					std::unique_lock<std::mutex> lock(g_mutex);
					etk::FSNodeRemove(_baseName + _mediaString + ".png");
					etk::FSNodeMove(tmpFileName, _baseName + _mediaString + ".jpg");
				} else {
					APPL_ERROR("try to store a file with the wrong format");
					throw std::runtime_error("wrong foramt :" + futType.get() + " support only image/jpeg, image/png");
				}
				
			}
			
			ememory::SharedPtr<zeus::File> getCover(uint32_t _mediaId, uint32_t _maxSize) override {
				return internalGetCover(g_basePathCover, etk::toString(_mediaId), _maxSize);
			}
			
			void setCover(zeus::ActionNotification<etk::String>& _notifs, zeus::ProxyFile _cover, uint32_t _mediaId) override {
				return internalSetCover(g_basePathCover, _notifs, _cover, etk::toString(_mediaId));
			}
			
			ememory::SharedPtr<zeus::File> getGroupCover(etk::String _groupName, uint32_t _maxSize) override {
				return internalGetCover(g_basePathCoverGroup, _groupName, _maxSize);
			}
			
			void setGroupCover(zeus::ActionNotification<etk::String>& _notifs, zeus::ProxyFile _cover, etk::String _groupName) override {
				return internalSetCover(g_basePathCoverGroup, _notifs, _cover, _groupName);
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
			property->setCallbackMetadataChange(&metadataChange);
			m_listFile.pushBack(property);
		}
	}
	g_needToStore = false;
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], etk::String _basePath) {
	g_basePath = _basePath;
	g_basePathCover = _basePath + "/AAAASDGDFGQN4352SCVdfgBSXDFGFCVQDSGFQSfd_cover/";
	g_basePathCoverGroup = _basePath + "/AAAASDGDFGQN4352SCVdfgBSXDFGFCVQDSGFQSfd_cover_group/";
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

