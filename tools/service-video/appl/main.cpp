/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Service.hpp>
#include <zeus/File.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>

#include <mutex>
#include <ejson/ejson.hpp>
#include <etk/os/FSNode.hpp>
#include <sstream>

#include <etk/stdTools.hpp>

static std::mutex g_mutex;
static std::string g_basePath;
static std::string g_baseDBName = std::string(SERVICE_NAME) + "-database.json";
static ejson::Document g_database;
static std::map<uint64_t,std::string> m_listFile;
static uint64_t m_lastMaxId = 0;

static uint64_t createFileID() {
	m_lastMaxId++;
	return m_lastMaxId;
}

namespace appl {
	class VideoService {
		private:
			ememory::SharedPtr<zeus::ClientProperty> m_client;
		public:
			VideoService() {
				APPL_WARNING("New VideoService ...");
			}
			VideoService(ememory::SharedPtr<zeus::ClientProperty> _client) :
			  m_client(_client) {
				APPL_WARNING("New VideoService ... for user: ");
			}
			~VideoService() {
				APPL_WARNING("delete VideoService ...");
			}
		public:
			std::vector<std::string> getAlbums() {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::vector<std::string> out;
				ejson::Array globalGroups = g_database["group-global"].toArray();
				if (globalGroups.exist() == false) {
					APPL_DEBUG("'group-global' ==> does not exist ==> No album");
					return out;
				}
				ejson::Object groups = g_database["groups"].toObject();
				if (groups.exist() == false) {
					APPL_DEBUG("'group' ==> does not exist ==> No album");
					return out;
				}
				APPL_DEBUG("for element in 'group-global'");
				for (auto it: globalGroups) {
					std::string tmpString = it.toString().get();
					if (tmpString == "") {
						continue;
					}
					APPL_DEBUG("    find emlement:" << tmpString);
					out.push_back(tmpString);
				}
				return out;
				/*
				ejson::Object groups = g_database["groups"].toObject();
				if (groups.exist() == false) {
					return std::vector<std::string>();
				}
				groups
				return getSubAlbums("");
				*/
			}
			// Get the list of sub album
			std::vector<std::string> getSubAlbums(std::string _parrentAlbum) {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::vector<std::string> out;
				ejson::Object groups = g_database["groups"].toObject();
				if (groups.exist() == false) {
					return out;
				}
				// find parrentAlbum ==> to get sub group
				/*
				for (size_t iii=0; iii<groups.size(); ++iii) {
					//ejson::Object group = groups[iii].toObject()["sub"];
					if (groups.getKey(iii) != _parrentAlbum) {
						continue;
					}
				}
				*/
				ejson::Object group = groups[_parrentAlbum].toObject();
				if (group.exist() == false) {
					return out;
				}
				ejson::Array groupSubs = group["sub"].toArray();
				for (auto it: groupSubs) {
					std::string tmpString = it.toString().get();
					if (tmpString == "") {
						continue;
					}
					out.push_back(tmpString);
				}
				// TODO: Check right
				return out;
			}
			uint32_t getAlbumCount(std::string _album) {
				std::unique_lock<std::mutex> lock(g_mutex);
				ejson::Object groups = g_database["groups"].toObject();
				if (groups.exist() == false) {
					// TODO : Throw an error ...
					return 0;
				}
				ejson::Object group = groups[_album].toObject();
				if (group.exist() == false) {
					// TODO : Throw an error ...
					return 0;
				}
				ejson::Array groupSubs = group["files"].toArray();
				// TODO: Check right
				return groupSubs.size();
			}
			// Return the list of the album files
			std::vector<std::string> getAlbumListPicture(std::string _album) {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::vector<std::string> out;
				ejson::Object groups = g_database["groups"].toObject();
				if (groups.exist() == false) {
					// TODO : Throw an error ...
					return out;
				}
				ejson::Object group = groups[_album].toObject();
				if (group.exist() == false) {
					// TODO : Throw an error ...
					return out;
				}
				ejson::Array groupSubs = group["files"].toArray();
				
				for (auto it: groupSubs) {
					uint64_t id = it.toNumber().getU64();
					/*
					auto itImage = m_listFile.find(id);
					if (itImage == m_listFile.end()) {
						
					}*/
					if (id == 0) {
						continue;
					}
					out.push_back(etk::to_string(id));
				}
				return out;
			}
			// Return a File Data (might be a picture .tiff/.png/.jpg)
			zeus::FileServer getAlbumPicture(std::string _pictureName) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				uint64_t id = etk::string_to_uint64_t(_pictureName);
				APPL_WARNING("try to get file : " << _pictureName << " with id=" << id);
				{
					auto it = m_listFile.find(id);
					if (it != m_listFile.end()) {
						return zeus::FileServer(g_basePath + it->second);
					}
				}
				for (auto &it : m_listFile) {
					APPL_WARNING("compare: " << it.first << " with " << id << " " << it.second);
					if (it.first == id) {
						return zeus::FileServer(g_basePath + it.second);
					}
				}
				APPL_ERROR("    ==> Not find ...");
				return zeus::FileServer();
			}
			std::string addFile(zeus::File _dataFile) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				APPL_ERROR("    ==> Receive FILE " << _dataFile.getMineType() << " size=" << _dataFile.getData().size());
				uint64_t id = createFileID();
				std::stringstream val;
				val << std::hex << std::setw(16) << std::setfill('0') << id;
				std::string filename = val.str();
				filename += ".";
				filename += zeus::getExtention(_dataFile.getMineType());
				_dataFile.storeIn(g_basePath + filename);
				m_listFile.insert(std::make_pair(id, filename));
				return etk::to_string(id);//zeus::FileServer();
			}
			/*
			// Return a global UTC time
			zeus::Time getAlbumPictureTime(std::string _pictureName) {
				return m_user->getAlbumPictureTime(_pictureName);
			}
			// Return a Geolocalization information (latitude, longitude)
			zeus::Geo getAlbumPictureGeoLocalization(std::string _pictureName) {
				return m_user->getAlbumPictureGeoLocalization(_pictureName);
			}
			*/
			bool removeFile(const std::string& _file) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return false;
			}
			
			std::string createAlbum(const std::string& _name) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return "";
			}
			bool removeAlbum(const std::string& _name) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return false;
			}
			bool setAlbumDescription(const std::string& _name, const std::string& _desc) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return false;
			}
			std::string getAlbumDescription(const std::string& _name) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return "";
			}
			bool addInAlbum(const std::string& _nameAlbum, const std::string& _nameElement) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return false;
			}
			bool removeFromAlbum(const std::string& _nameAlbum, const std::string& _nameElement) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return false;
			}
			/*
			// Return a global UTC time
			zeus::Time getAlbumPictureTime(const std::string& _pictureName) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return zeus::Time();
			}
			// Return a Geolocalization information (latitude, longitude)
			zeus::Geo getAlbumPictureGeoLocalization(const std::string& _pictureName) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// TODO : Check right ...
				return zeus::Geo();
			}
			*/
	};
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], std::string _basePath) {
	g_basePath = _basePath;
	std::unique_lock<std::mutex> lock(g_mutex);
	APPL_WARNING("Load USER: " << g_basePath);
	bool ret = g_database.load(g_basePath + g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	// Load all files (image and video ...)
	etk::FSNode node(g_basePath);
	std::vector<etk::FSNode*> tmpList = node.folderGetSubList(false, false, true, false);
	APPL_WARNING("Find " << tmpList.size() << " files");
	for (auto &it : tmpList) {
		if (it == nullptr) {
			continue;
		}
		if (    etk::end_with(it->getNameFile(), ".svg", false) == true
		     || etk::end_with(it->getNameFile(), ".bmp", false) == true
		     || etk::end_with(it->getNameFile(), ".png", false) == true
		     || etk::end_with(it->getNameFile(), ".jpg", false) == true
		     || etk::end_with(it->getNameFile(), ".tga", false) == true
		     || etk::end_with(it->getNameFile(), ".mp4", false) == true
		     || etk::end_with(it->getNameFile(), ".avi", false) == true
		     || etk::end_with(it->getNameFile(), ".mov", false) == true
		     || etk::end_with(it->getNameFile(), ".mkv", false) == true) {
			// TODO : Do it better (proto ..)
			std::string idString = it->getNameFile();
			idString.resize(idString.size()-4);
			uint64_t id = 0;
			std::stringstream ss;
			ss << std::hex << idString;
			ss >> id;
			if (id <= 1024) {
				APPL_WARNING("    ==> REJECTED file " << it->getNameFile() << " with ID = " << id);
			} else {
				m_listFile.insert(std::make_pair(id, it->getNameFile()));
				m_lastMaxId = std::max(m_lastMaxId,id);
				APPL_WARNING("    ==> load file " << it->getNameFile() << " with ID = " << id);
			}
		} else {
			APPL_WARNING("    ==> REJECT file " << it->getNameFile());
		}
	}
	APPL_WARNING("new USER: [STOP]");
	return true;
}

ETK_EXPORT_API bool SERVICE_IO_uninit() {
	std::unique_lock<std::mutex> lock(g_mutex);
	APPL_DEBUG("Store User Info:");
	bool ret = g_database.storeSafe(g_basePath + g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> Store error");
		return false;
	}
	APPL_WARNING("delete USER [STOP]");
	return true;
}

ETK_EXPORT_API bool SERVICE_IO_execute(std::string _ip, uint16_t _port) {
	APPL_INFO("===========================================================");
	APPL_INFO("== ZEUS instanciate service: " << SERVICE_NAME << " [START]");
	APPL_INFO("===========================================================");
	zeus::ServiceType<appl::VideoService> serviceInterface([](ememory::SharedPtr<zeus::ClientProperty> _client){
	                                                        	return ememory::makeShared<appl::VideoService>(_client);
	                                                        });
	if (_ip != "") {
		serviceInterface.propertyIp.set(_ip);
	}
	if (_port != 0) {
		serviceInterface.propertyPort.set(_port);
	}
	serviceInterface.propertyNameService.set(SERVICE_NAME);
	serviceInterface.setDescription("Video Private Interface");
	serviceInterface.setVersionImplementation("0.1.0");
	serviceInterface.setVersion("1.0");
	serviceInterface.setType("VIDEO");
	serviceInterface.addAuthor("Heero Yui", "yui.heero@gmail.com");
	
	serviceInterface.advertise("getAlbums", &appl::VideoService::getAlbums);
	serviceInterface.advertise("getSubAlbums", &appl::VideoService::getSubAlbums);
	serviceInterface.advertise("getAlbumCount", &appl::VideoService::getAlbumCount);
	serviceInterface.advertise("getAlbumListPicture", &appl::VideoService::getAlbumListPicture);
	serviceInterface.advertise("getAlbumPicture", &appl::VideoService::getAlbumPicture);
	serviceInterface.advertise("addFile", &appl::VideoService::addFile);
	/*
	serviceInterface.advertise("getAlbumPicture", &appl::VideoService::getAlbumPicture);
	serviceInterface.advertise("getAlbumPictureTime", &appl::VideoService::getAlbumPictureTime);
	serviceInterface.advertise("getAlbumPictureGeoLocalization", &appl::VideoService::getAlbumPictureGeoLocalization);
	*/
	APPL_INFO("===========================================================");
	APPL_INFO("== ZEUS service: " << *serviceInterface.propertyNameService << " [service instanciate]");
	APPL_INFO("===========================================================");
	if (serviceInterface.connect() == false) {
		return false;
	}
	if (serviceInterface.GateWayAlive() == false) {
		APPL_INFO("===========================================================");
		APPL_INFO("== ZEUS service: " << *serviceInterface.propertyNameService << " [STOP] Can not connect to the GateWay");
		APPL_INFO("===========================================================");
		return false;
	}
	int32_t iii=0;
	while (serviceInterface.GateWayAlive() == true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		serviceInterface.pingIsAlive();
		/*
		serviceInterface.store();
		serviceInterface.clean();
		*/
		APPL_INFO("service in waiting ... " << iii << "/inf");
		iii++;
	}
	serviceInterface.disconnect();
	APPL_INFO("===========================================================");
	APPL_INFO("== ZEUS service: " << *serviceInterface.propertyNameService << " [STOP] GateWay Stop");
	APPL_INFO("===========================================================");
	return true;
}
