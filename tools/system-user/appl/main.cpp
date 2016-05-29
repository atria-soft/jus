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

#include <etk/stdTools.h>
namespace appl {
	class User {
		private:
			std::mutex m_mutex;
			std::string m_userName;
			ejson::Document m_database;
		public:
			User(const std::string& _userName) :
			  m_userName(_userName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				APPL_WARNING("new USER: " << m_userName);
				bool ret = m_database.load(std::string("USERDATA:") + m_userName + ".json");
				if (ret == false) {
					APPL_WARNING("    ==> LOAD error");
				}
			}
			~User() {
				std::unique_lock<std::mutex> lock(m_mutex);
				APPL_WARNING("delete USER [START]");
				APPL_DEBUG("Store User Info:");
				bool ret = m_database.storeSafe(std::string("USERDATA:") + m_userName + ".json");
				if (ret == false) {
					APPL_WARNING("    ==> Store error");
				}
				APPL_WARNING("delete USER [STOP]");
			}
			const std::string& getName() {
				return m_userName;
			}
			std::vector<std::string> getGroups(const std::string& _clientName) {
				std::unique_lock<std::mutex> lock(m_mutex);
				std::vector<std::string> out;
				ejson::Object clients = m_database["client"].toObject();
				if (clients.exist() == false) {
					// Section never created
					return out;
				}
				ejson::Object client = clients[_clientName].toObject();
				if (clients.exist() == false) {
					// No specificity for this client (in case it have no special right)
					return out;
				}
				if (client["tocken"].toString().get() != "") {
					out.push_back("connected");
				}
				// TODO: check banishing ...
				ejson::Array groups = client["group"].toArray();
				for (auto it : groups) {
					out.push_back(it.toString().get());
				}
				return out;
			}
			bool checkTocken(const std::string& _clientName, const std::string& _tocken) {
				std::unique_lock<std::mutex> lock(m_mutex);
				ejson::Object clients = m_database["client"].toObject();
				if (clients.exist() == false) {
					// Section never created
					return false;
				}
				ejson::Object client = clients[_clientName].toObject();
				if (clients.exist() == false) {
					// No specificity for this client (in case it have no special right)
					return false;
				}
				// TODO: check banishing ...
				// TODO: Do it better ...
				std::string registerTocken = client["tocken"].toString().get();
				if (registerTocken == _tocken) {
					return true;
				}
				return false;
			}
			bool checkAuth(const std::string& _password) {
				std::unique_lock<std::mutex> lock(m_mutex);
				std::string pass = m_database["password"].toString().get();
				if (pass == "") {
					// pb password
					return false;
				}
				if (pass == _password) {
					return true;
				}
				return false;
			}
			std::vector<std::string> filterServices(const std::string& _clientName, std::vector<std::string> _inputList) {
				std::unique_lock<std::mutex> lock(m_mutex);
				// When connected to our session ==> we have no control access ...
				if (_clientName == m_userName) {
					return _inputList;
				}
				std::vector<std::string> out;
				APPL_TODO("Filter service list ==> not implemented...");
				return out;
			}
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
	class SystemService {
		private:
			ememory::SharedPtr<appl::User> m_user;
		private:
			ememory::SharedPtr<jus::ClientProperty> m_client;
		public:
			SystemService() {
				APPL_WARNING("New SystemService ...");
			}
			SystemService(ememory::SharedPtr<appl::User> _user, ememory::SharedPtr<jus::ClientProperty> _client) :
			  m_user(_user),
			  m_client(_client) {
				APPL_WARNING("New SystemService ... for user: ");
			}
			~SystemService() {
				APPL_WARNING("delete SystemService ...");
			}
		public:
			std::vector<std::string> getGroups(std::string _clientName) {
				std::vector<std::string> out;
				if (m_client == nullptr) {
					return out;
				}
				// TODO: check if basished ...
				if (m_client->getName() != "") {
					out = m_user->getGroups(m_client->getName());
				}
				// TODO: Check default visibility ... (if user want to have default visibility at Noone ==> then public must be removed...
				if (true) {
					out.push_back("public");
				}
				return out;
			}
			bool checkTocken(std::string _clientName, std::string _tocken) {
				return m_user->checkTocken(_clientName, _tocken);
			}
			bool checkAuth(std::string _password) {
				return m_user->checkAuth(_password);
			}
			std::vector<std::string> filterServices(std::string _clientName, std::vector<std::string> _currentList) {
				return m_user->filterServices(_clientName, _currentList);
			}
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
		jus::ServiceType<appl::SystemService, appl::UserManager> serviceInterface(userMng);
		if (ip != "") {
			serviceInterface.propertyIp.set(ip);
		}
		if (port != 0) {
			serviceInterface.propertyPort.set(port);
		}
		serviceInterface.setDescription("user interface management");
		serviceInterface.setVersion("0.1.0");
		serviceInterface.addAuthor("Heero Yui", "yui.heero@gmail.com");
		serviceInterface.advertise("checkTocken", &appl::SystemService::checkTocken);
		serviceInterface.setLastFuncDesc("Check if a user tocken is correct or not");
		serviceInterface.addLastFuncParam("clientName", "Name of the client");
		serviceInterface.addLastFuncParam("tocken", "String containing the Tocken");
		serviceInterface.advertise("checkAuth", &appl::SystemService::checkAuth);
		serviceInterface.setLastFuncDesc("Check the password of the curent user");
		serviceInterface.addLastFuncParam("password", "client/user password");
		serviceInterface.advertise("getGroups", &appl::SystemService::getGroups);
		serviceInterface.setLastFuncDesc("Get list of group availlable for a client name");
		serviceInterface.addLastFuncParam("clientName", "Name of the client");
		serviceInterface.advertise("filterServices", &appl::SystemService::filterServices);
		serviceInterface.setLastFuncDesc("Filter a list of service with the cuurent profile of the user (restrict area)");
		serviceInterface.addLastFuncParam("clientName", "Name of the client");
		serviceInterface.addLastFuncParam("currentList", "Vector of name of the services");
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
