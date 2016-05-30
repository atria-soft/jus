/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/Service.h>
#include <etk/etk.h>
#include <unistd.h>
#include <mutex>
#include <ejson/ejson.h>
#include <etk/os/FSNode.h>

#include <etk/stdTools.h>
namespace appl {
	class User {
		private:
			std::mutex m_mutex;
			std::string m_userName;
			std::string m_basePath;
		public:
			User(const std::string& _userName) :
			  m_userName(_userName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				APPL_WARNING("new USER: " << m_userName);
				m_basePath = std::string("USERDATA:") + m_userName + "/";
			}
			~User() {
				std::unique_lock<std::mutex> lock(m_mutex);
				APPL_WARNING("delete USER [START]");
				APPL_WARNING("delete USER [STOP]");
			}
			// Return the list of root albums
			std::vector<std::string> getAlbums() {
				return getSubAlbums("");
			}
			// Get the list of sub album
			std::vector<std::string> getSubAlbums(const std::string& _parrentAlbum) {
				std::unique_lock<std::mutex> lock(m_mutex);
				std::vector<std::string> out;
				etk::FSNode node(m_basePath + _parrentAlbum);
				std::vector<etk::FSNode*> tmpList = node.folderGetSubList(false, true, false, false);
				for (auto &it : tmpList) {
					if (it == nullptr) {
						continue;
					}
					if (    _parrentAlbum.size() == 0
					     || _parrentAlbum[_parrentAlbum.size()-1] == '/') {
						out.push_back(_parrentAlbum + it->getNameFile());
					} else {
						out.push_back(_parrentAlbum + "/" + it->getNameFile());
					}
				}
				return out;
			}
			uint32_t getAlbumCount(const std::string& _album) {
				std::unique_lock<std::mutex> lock(m_mutex);
				etk::FSNode node(m_basePath + _album);
				std::vector<etk::FSNode*> tmpList = node.folderGetSubList(false, false, true, false);
				uint32_t nbElem = 0;
				for (auto &it : tmpList) {
					if (it == nullptr) {
						continue;
					}
					if (    etk::end_with(it->getNameFile(), ".svg", false) == true
					     || etk::end_with(it->getNameFile(), ".bmp", false) == true
					     || etk::end_with(it->getNameFile(), ".png", false) == true
					     || etk::end_with(it->getNameFile(), ".jpg", false) == true) {
						nbElem++;
					}
				}
				return nbElem;
			}
			// Return the list of the album files
			std::vector<std::string> getAlbumListPicture(const std::string& _album, uint32_t _startId, uint32_t _stopId) {
				std::unique_lock<std::mutex> lock(m_mutex);
				std::vector<std::string> out;
				return std::vector<std::string>();
			}
			/*
			// Return a File Data (might be a picture .tiff/.png/.jpg)
			jus::File getAlbumPicture(const std::string& _pictureName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				return jus::File();
			}
			// Return a global UTC time
			jus::Time getAlbumPictureTime(const std::string& _pictureName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				return jus::Time();
			}
			// Return a Geolocalization information (latitude, longitude)
			jus::Geo getAlbumPictureGeoLocalization(const std::string& _pictureName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				return jus::Geo();
			}
			*/
	};
	
	class UserManager {
		private:
			std::mutex m_mutex;
			std::map<std::string, ememory::SharedPtr<appl::User>> m_listLoaded;
		public:
			UserManager() {
				
			}
			ememory::SharedPtr<appl::User> getUser(const std::string& _userName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				auto it = m_listLoaded.find(_userName);
				if (it != m_listLoaded.end()) {
					// User already loaded:
					return it->second;
				}
				// load New User:
				ememory::SharedPtr<appl::User> tmp(new appl::User(_userName));
				m_listLoaded.insert(std::make_pair(_userName, tmp));
				return tmp;
			}
	};
	class PictureService {
		private:
			ememory::SharedPtr<appl::User> m_user;
		private:
			ememory::SharedPtr<jus::ClientProperty> m_client;
		public:
			PictureService() {
				APPL_WARNING("New PictureService ...");
			}
			PictureService(ememory::SharedPtr<appl::User> _user, ememory::SharedPtr<jus::ClientProperty> _client) :
			  m_user(_user),
			  m_client(_client) {
				APPL_WARNING("New PictureService ... for user: ");
			}
			~PictureService() {
				APPL_WARNING("delete PictureService ...");
			}
		public:
		
			std::vector<std::string> getAlbums() {
				return m_user->getAlbums();
			}
			// Get the list of sub album
			std::vector<std::string> getSubAlbums(std::string _parrentAlbum) {
				return m_user->getSubAlbums(_parrentAlbum);
			}
			uint32_t getAlbumCount(std::string _album) {
				return m_user->getAlbumCount(_album);
			}
			// Return the list of the album files
			std::vector<std::string> getAlbumListPicture(std::string _album, uint32_t _startId, uint32_t _stopId) {
				return m_user->getAlbumListPicture(_album, _startId, _stopId);
			}
			/*
			// Return a File Data (might be a picture .tiff/.png/.jpg)
			jus::File getAlbumPicture(std::string _pictureName) {
				return m_user->getAlbumPicture(_pictureName);
			}
			// Return a global UTC time
			jus::Time getAlbumPictureTime(std::string _pictureName) {
				return m_user->getAlbumPictureTime(_pictureName);
			}
			// Return a Geolocalization information (latitude, longitude)
			jus::Geo getAlbumPictureGeoLocalization(std::string _pictureName) {
				return m_user->getAlbumPictureGeoLocalization(_pictureName);
			}
			*/
	};
}


int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	std::string ip;
	uint16_t port = 0;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			ip = std::string(&data[5]);
		} else if (etk::start_with(data, "--port=") == true) {
			port = etk::string_to_uint16_t(std::string(&data[7]));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --ip=XXX      Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX    Server connection PORT (default: 1983)");
			return -1;
		}
	}
	while (true) {
		APPL_INFO("===========================================================");
		APPL_INFO("== JUS instanciate service: " << SERVICE_NAME << " [START]");
		APPL_INFO("===========================================================");
		appl::UserManager userMng;
		jus::ServiceType<appl::PictureService, appl::UserManager> serviceInterface(userMng);
		if (ip != "") {
			serviceInterface.propertyIp.set(ip);
		}
		if (port != 0) {
			serviceInterface.propertyPort.set(port);
		}
		serviceInterface.setDescription("Picture Private Interface");
		serviceInterface.setVersion("0.1.0");
		serviceInterface.setType("PICTURE", 1);
		serviceInterface.addAuthor("Heero Yui", "yui.heero@gmail.com");
		
		serviceInterface.advertise("getAlbums", &appl::PictureService::getAlbums);
		serviceInterface.advertise("getSubAlbums", &appl::PictureService::getSubAlbums);
		serviceInterface.advertise("getAlbumCount", &appl::PictureService::getAlbumCount);
		serviceInterface.advertise("getAlbumListPicture", &appl::PictureService::getAlbumListPicture);
		/*
		serviceInterface.advertise("getAlbumPicture", &appl::PictureService::getAlbumPicture);
		serviceInterface.advertise("getAlbumPictureTime", &appl::PictureService::getAlbumPictureTime);
		serviceInterface.advertise("getAlbumPictureGeoLocalization", &appl::PictureService::getAlbumPictureGeoLocalization);
		*/
		APPL_INFO("===========================================================");
		APPL_INFO("== JUS service: " << SERVICE_NAME << " [service instanciate]");
		APPL_INFO("===========================================================");
		serviceInterface.connect(SERVICE_NAME);
		if (serviceInterface.GateWayAlive() == false) {
			APPL_INFO("===========================================================");
			APPL_INFO("== JUS service: " << SERVICE_NAME << " [STOP] Can not connect to the GateWay");
			APPL_INFO("===========================================================");
			APPL_INFO("wait 5 second ...");
			usleep(5000000);
			continue;
		}
		int32_t iii=0;
		while (serviceInterface.GateWayAlive() == true) {
			usleep(1000000);
			serviceInterface.pingIsAlive();
			APPL_INFO("service in waiting ... " << iii << "/inf");
			iii++;
		}
		serviceInterface.disconnect();
		APPL_INFO("===========================================================");
		APPL_INFO("== JUS service: " << SERVICE_NAME << " [STOP] GateWay Stop");
		APPL_INFO("===========================================================");
	}
	return 0;
}
