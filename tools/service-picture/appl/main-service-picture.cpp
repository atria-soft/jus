/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
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

#include <zeus/service/Picture.hpp>
#include <zeus/service/registerPicture.hpp>
#include <zeus/ProxyClientProperty.hpp>
#include <zeus/File.hpp>
#include <zeus/ProxyFile.hpp>

static std::mutex g_mutex;
static std::string g_basePath;
static std::string g_baseDBName = std::string(SERVICE_NAME) + "-database.json";
class FileProperty {
	public:
		uint64_t m_id; //!< use local reference ID to have faster access on the file ...
		std::string m_fileName; // Sha 512
		std::string m_name;
		std::string m_mineType;
		echrono::Time m_creationData;
		std::map<std::string, std::string> m_metadata;
};
static std::vector<FileProperty> m_listFile;

class Album {
	public:
		uint32_t m_id; //!< use local reference ID to have faster access on the file ...
		uint32_t m_parentId; //!< parent Album ID
		std::string m_name; //!< name of the Album
		std::string m_description; //!< description of the album
		std::vector<uint32_t> m_listMedia; //!< List of media in this album
};
static std::vector<Album> m_listAlbum;

static uint64_t m_lastMaxId = 0;
static bool g_needToStore = false;

static uint64_t createUniqueID() {
	m_lastMaxId++;
	return m_lastMaxId;
}

namespace appl {
	class PictureService : public zeus::service::Picture  {
		private:
			//ememory::SharedPtr<zeus::ClientProperty>& m_client;
			zeus::ProxyClientProperty m_client;
			std::string m_userName;
		public:
			/*
			PictureService(ememory::SharedPtr<zeus::ClientProperty>& _client, const std::string& _userName) :
			  m_client(_client),
			  m_userName(_userName) {
				APPL_WARNING("New PictureService ... for user: ");
			}
			*/
			PictureService(uint16_t _clientId) {
				APPL_VERBOSE("New PictureService ... for user: " << _clientId);
			}
			~PictureService() {
				APPL_VERBOSE("delete PictureService ...");
			}
		public:
			uint32_t mediaIdCount() override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return m_listFile.size();
			}
			
			std::vector<uint32_t> mediaIdGetRange(uint32_t _start, uint32_t _stop) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				std::vector<uint32_t> out;
				for (size_t iii=_start; iii<m_listFile.size() && iii<_stop; ++iii) {
					out.push_back(m_listFile[iii].m_id);
				}
				return out;
			}
			// Return a File Data (might be a picture .tiff/.png/.jpg)
			ememory::SharedPtr<zeus::File> mediaGet(uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				//Check if the file exist:
				bool find = false;
				FileProperty property;
				for (auto &it : m_listFile) {
					if (it.m_id == _mediaId) {
						find = true;
						property = it;
						break;
					}
				}
				if (find == false) {
					throw std::invalid_argument("Wrong file name ...");
				}
				return zeus::File::create(g_basePath + property.m_fileName + "." + zeus::getExtention(property.m_mineType), "", property.m_mineType);
			}
			uint32_t mediaAdd(zeus::ProxyFile _dataFile) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				uint64_t id = createUniqueID();
				
				auto futType = _dataFile.getMineType();
				auto futName = _dataFile.getName();
				std::string tmpFileName = g_basePath + "tmpImport_" + etk::to_string(id);
				std::string sha512String = zeus::storeInFile(_dataFile, tmpFileName);
				futType.wait();
				futName.wait();
				// TODO : Get internal data of the file and remove all the meta-data ==> proper files ...
				for (auto &it : m_listFile) {
					if (it.m_fileName == sha512String) {
						APPL_INFO("File already registered at " << it.m_creationData);
						// TODO : Check if data is identical ...
						// remove temporary file
						etk::FSNodeRemove(tmpFileName);
						return it.m_id;
					}
				}
				// move the file at the good position:
				APPL_DEBUG("move temporay file in : " << g_basePath << sha512String);
				if (etk::FSNodeGetSize(tmpFileName) == 0) {
					APPL_ERROR("try to store an empty file");
					throw std::runtime_error("file size == 0");
				}
				etk::FSNodeMove(tmpFileName, g_basePath + sha512String + "." + zeus::getExtention(futType.get()));
				FileProperty property;
				property.m_id = id;
				property.m_fileName = sha512String;
				property.m_name = futName.get();
				property.m_mineType = futType.get();
				property.m_creationData = echrono::Time::now();
				m_listFile.push_back(property);
				g_needToStore = true;
				APPL_DEBUG(" filename : " << sha512String);
				return id;
			}
			void mediaRemove(uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				//Check if the file exist:
				bool find = false;
				FileProperty property;
				for (auto it = m_listFile.begin();
				     it != m_listFile.end();
				     /* No increment */) {
					if (it->m_id == _mediaId) {
						it = m_listFile.erase(it);
						find = true;
						property = *it;
					} else {
						++it;
					}
				}
				if (find == false) {
					throw std::invalid_argument("Wrong file name ...");
				}
				// Remove media in all Album ...
				for (auto &it : m_listAlbum) {
					for (auto elem = it.m_listMedia.begin();
					     elem != it.m_listMedia.end();
					     /* No increment */) {
						if (*elem == _mediaId) {
							elem = it.m_listMedia.erase(elem);
						} else {
							++elem;
						}
					}
				}
				// Real Remove definitly the file
				// TODO : Set it in a trash ... For a while ...
				if (etk::FSNodeRemove(g_basePath + property.m_fileName + "." + zeus::getExtention(property.m_mineType)) == false) {
					throw std::runtime_error("Can not remove file ...");
				}
			}
			
			std::vector<std::string> mediaMetadataGetKeys(uint32_t _mediaId) override {
				std::vector<std::string> out;
				for (auto &it : m_listFile) {
					if (it.m_id == _mediaId) {
						for (auto &itM : it.m_metadata) {
							out.push_back(itM.first);
						}
						return out;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			std::string mediaMetadataGetKey(uint32_t _mediaId, std::string _key) override {
				std::vector<std::string> out;
				for (auto &it : m_listFile) {
					if (it.m_id == _mediaId) {
						auto itM = it.m_metadata.find(_key);
						if (itM != it.m_metadata.end()) {
							return itM->second;
						}
						return "";
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			void mediaMetadataSetKey(uint32_t _mediaId, std::string _key, std::string _value) override {
				for (auto &it : m_listFile) {
					if (it.m_id == _mediaId) {
						auto itM = it.m_metadata.find(_key);
						if (itM != it.m_metadata.end()) {
							itM->second = _value;
						} else {
							it.m_metadata.insert(std::make_pair(_key, _value));
						}
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			uint32_t albumCreate(std::string _albumName) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				for (auto &it : m_listAlbum) {
					if (it.m_name == _albumName) {
						return it.m_id;
					}
				}
				Album album;
				album.m_id = createUniqueID();
				album.m_name = _albumName;
				m_listAlbum.push_back(album);
				return album.m_id;
			}
			
			
			void albumRemove(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				for (auto it = m_listAlbum.begin();
				     it != m_listAlbum.end();
				     /* No increment */) {
					if (it->m_id == _albumId) {
						it = m_listAlbum.erase(it);
						return;
					}
					++it;
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			std::vector<uint32_t> albumGetList() override {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::vector<uint32_t> out;
				for (auto &it : m_listAlbum) {
					out.push_back(it.m_id);
				}
				return out;
			}
			std::string albumNameGet(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						return it.m_name;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
				return "";
			}
			void albumNameSet(uint32_t _albumId, std::string _albumName) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						it.m_name = _albumName;
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			std::string albumDescriptionGet(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						return it.m_description;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
				return "";
			}
			void albumDescriptionSet(uint32_t _albumId, std::string _desc) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						it.m_description = _desc;
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			void albumMediaAdd(uint32_t _albumId, uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						for (auto &elem : it.m_listMedia) {
							if (elem == _mediaId) {
								// already intalles
								return;
							}
						}
						it.m_listMedia.push_back(_mediaId);
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			void albumMediaRemove(uint32_t _albumId, uint32_t _mediaId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						for (auto elem = it.m_listMedia.begin();
						     elem != it.m_listMedia.end();
						     /* No increment */) {
							if (*elem == _mediaId) {
								elem = it.m_listMedia.erase(elem);
								return;
							}
							++elem;
						}
						// Media not find ... ==> not a problem ...
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			uint32_t albumMediaCount(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						return it.m_listMedia.size();
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
				return 0;
			}
			std::vector<uint32_t> albumMediaIdGet(uint32_t _albumId, uint32_t _start, uint32_t _stop) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						std::vector<uint32_t> out;
						for (size_t iii=_start;
						         iii<it.m_listMedia.size()
						      && iii<_stop;
						     ++iii) {
							out.push_back(it.m_listMedia[iii]);
						}
						return out;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			void albumParentSet(uint32_t _albumId, uint32_t _albumParentId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						it.m_parentId = _albumParentId;
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			void albumParentRemove(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						it.m_parentId = 0;
						return;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
			uint32_t albumParentGet(uint32_t _albumId) override {
				std::unique_lock<std::mutex> lock(g_mutex);
				for (auto &it : m_listAlbum) {
					if (it.m_id == _albumId) {
						return it.m_parentId;
					}
				}
				throw std::invalid_argument("Wrong Album ID ...");
			}
	};
}

static void store_db() {
	APPL_ERROR("Store database [START]");
	ejson::Document database;
	ejson::Array listFilesArray;
	database.add("list-files", listFilesArray);
	for (auto &it : m_listFile) {
		ejson::Object fileElement;
		listFilesArray.add(fileElement);
		fileElement.add("id", ejson::Number(it.m_id));
		fileElement.add("file-name", ejson::String(it.m_fileName));
		fileElement.add("name", ejson::String(it.m_name));
		fileElement.add("mine-type", ejson::String(it.m_mineType));
		fileElement.add("add-date", ejson::Number(it.m_creationData.count()));
		if (it.m_metadata.size() != 0) {
			ejson::Object listMetadata;
			fileElement.add("meta", listMetadata);
			for (auto &itM : it.m_metadata) {
				listMetadata.add(itM.first, ejson::String(itM.second));
			}
		}
	}
	ejson::Array listAlbumArray;
	database.add("list-album", listAlbumArray);
	for (auto &it : m_listAlbum) {
		ejson::Object albumElement;
		listAlbumArray.add(albumElement);
		albumElement.add("id", ejson::Number(it.m_id));
		albumElement.add("parent", ejson::Number(it.m_parentId));
		albumElement.add("name", ejson::String(it.m_name));
		albumElement.add("desc", ejson::String(it.m_description));
		ejson::Array listMediaArray;
		albumElement.add("media", listMediaArray);
		for (auto &it2 : it.m_listMedia) {
			listMediaArray.add(ejson::Number(it2));
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
		ejson::Object fileElement = itArray.toObject();
		FileProperty property;
		property.m_id = fileElement["id"].toNumber().getU64();
		APPL_INFO("get ID : " << property.m_id);
		property.m_fileName = fileElement["file-name"].toString().get();
		property.m_name = fileElement["name"].toString().get();
		property.m_mineType = fileElement["mine-type"].toString().get();
		property.m_creationData = echrono::Time(fileElement["add-date"].toNumber().getU64()*1000);
		if (m_lastMaxId < property.m_id) {
			m_lastMaxId = property.m_id+1;
		}
		ejson::Object tmpObj = fileElement["meta"].toObject();
		if (tmpObj.exist() == true) {
			for (auto itValue = tmpObj.begin();
			     itValue != tmpObj.end();
			     ++itValue) {
				property.m_metadata.insert(std::make_pair(itValue.getKey(), (*itValue).toString().get()));
			}
		}
		if (property.m_fileName == "") {
			APPL_ERROR("Can not access on the file : ... No name ");
		} else {
			m_listFile.push_back(property);
		}
	}
	ejson::Array listAlbumArray = database["list-album"].toArray();
	for (const auto itArray: listAlbumArray) {
		ejson::Object albumElement = itArray.toObject();
		Album album;
		album.m_id = albumElement["id"].toNumber().getU64();
		album.m_parentId = albumElement["parent"].toNumber().getU64();
		album.m_name = albumElement["name"].toString().get();
		album.m_description = albumElement["desc"].toString().get();
		ejson::Array listMadiaArray = albumElement["media"].toArray();
		for (const auto itArrayMedia: listMadiaArray) {
			uint64_t tmp = itArrayMedia.toNumber().getU64();
			album.m_listMedia.push_back(tmp);
		}
		m_listAlbum.push_back(album);
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


ZEUS_SERVICE_PICTURE_DECLARE(appl::PictureService);

