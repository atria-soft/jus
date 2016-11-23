/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Service.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>

#include <mutex>
#include <ejson/ejson.hpp>

#include <etk/stdTools.hpp>


static std::mutex g_mutex;
static std::string g_basePath;
static std::string g_baseDBName = std::string(SERVICE_NAME) + "-database.json";
static ejson::Document g_database;

namespace appl {
	class SystemService {
		private:
			ememory::SharedPtr<zeus::ClientProperty> m_client;
		public:
			SystemService(ememory::SharedPtr<zeus::ClientProperty> _client) :
			  m_client(_client) {
				APPL_WARNING("New SystemService ... for user: ");
			}
			~SystemService() {
				APPL_WARNING("Delete service-user interface.");
			}
		public:
			std::vector<std::string> getGroups(std::string _clientName) {
				std::vector<std::string> out;
				if (m_client == nullptr) {
					return out;
				}
				// TODO: check if basished ...
				if (m_client->getName() != "") {
					std::unique_lock<std::mutex> lock(g_mutex);
					std::vector<std::string> out;
					ejson::Object clients = g_database["client"].toObject();
					if (clients.exist() == false) {
						// Section never created
						return out;
					}
					ejson::Object client = clients[m_client->getName()].toObject();
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
				}
				// TODO: Check default visibility ... (if user want to have default visibility at Noone ==> then public must be removed...
				if (true) {
					out.push_back("public");
				}
				return out;
			}
			bool checkTocken(std::string _clientName, std::string _tocken) {
				std::unique_lock<std::mutex> lock(g_mutex);
				ejson::Object clients = g_database["client"].toObject();
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
			bool checkAuth(std::string _password) {
				std::unique_lock<std::mutex> lock(g_mutex);
				std::string pass = g_database["password"].toString().get();
				if (pass == "") {
					// pb password
					return false;
				}
				if (pass == _password) {
					return true;
				}
				return false;
			}
			std::vector<std::string> filterServices(std::string _clientName, std::vector<std::string> _currentList) {
				std::unique_lock<std::mutex> lock(g_mutex);
				// When connected to our session ==> we have no control access ...
				if (_clientName == g_basePath) {
					return _currentList;
				}
				std::vector<std::string> out;
				APPL_TODO("Filter service list ==> not implemented...");
				return out;
			}
	};
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], std::string _basePath) {
	// Already init : etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	g_basePath = _basePath;
	std::unique_lock<std::mutex> lock(g_mutex);
	APPL_WARNING("Load USER: " << g_basePath);
	bool ret = g_database.load(g_basePath + g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
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
	zeus::ServiceType<appl::SystemService> serviceInterface([](ememory::SharedPtr<zeus::ClientProperty> _client){
	                                                        	return ememory::makeShared<appl::SystemService>(_client);
	                                                        });
	if (_ip != "") {
		serviceInterface.propertyIp.set(_ip);
	}
	if (_port != 0) {
		serviceInterface.propertyPort.set(_port);
	}
	serviceInterface.propertyNameService.set(SERVICE_NAME);
	serviceInterface.setDescription("user interface management");
	serviceInterface.setVersion("0.1.0");
	serviceInterface.setType("USER", 1);
	serviceInterface.addAuthor("Heero Yui", "yui.heero@gmail.com");
	zeus::AbstractFunction* func = serviceInterface.advertise("checkTocken", &appl::SystemService::checkTocken);
	if (func != nullptr) {
		func->setDescription("Check if a user tocken is correct or not");
		func->addParam("clientName", "Name of the client");
		func->addParam("tocken", "String containing the Tocken");
	}
	func = serviceInterface.advertise("checkAuth", &appl::SystemService::checkAuth);
	if (func != nullptr) {
		func->setDescription("Check the password of the curent user");
		func->addParam("password", "client/user password");
	}
	func = serviceInterface.advertise("getGroups", &appl::SystemService::getGroups);
	if (func != nullptr) {
		func->setDescription("Get list of group availlable for a client name");
		func->addParam("clientName", "Name of the client");
	}
	func = serviceInterface.advertise("filterServices", &appl::SystemService::filterServices);
	if (func != nullptr) {
		func->setDescription("Filter a list of service with the cuurent profile of the user (restrict area)");
		func->addParam("clientName", "Name of the client");
		func->addParam("currentList", "Vector of name of the services");
	}
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
		std::this_thread::sleep_for(std::chrono::seconds(10));
		serviceInterface.pingIsAlive();
		APPL_INFO("service in waiting ... " << iii << "/inf");
		iii++;
	}
	APPL_INFO("Disconnect service ...");
	serviceInterface.disconnect();
	APPL_INFO("===========================================================");
	APPL_INFO("== ZEUS service: " << *serviceInterface.propertyNameService << " [STOP] GateWay Stop");
	APPL_INFO("===========================================================");
	return true;
}
