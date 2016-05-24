/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/Service.h>
#include <etk/etk.h>
#include <unistd.h>

#include <etk/stdTools.h>
namespace appl {
	class User {
		public:
			User() {
				APPL_WARNING("new USER");
			}
			~User() {
				APPL_WARNING("delete USER");
			}
			
	};
	
	class UserManager {
		private:
			std::map<std::string, ememory::SharedPtr<appl::User>> m_listLoaded;
		public:
			UserManager() {
				
			}
			ememory::SharedPtr<appl::User> getUser(const std::string& _userName) {
				// TODO : Lock ...
				auto it = m_listLoaded.find(_userName);
				if (it != m_listLoaded.end()) {
					// User already loaded:
					return it->second;
				}
				// load New User:
				ememory::SharedPtr<appl::User> tmp(new appl::User);
				m_listLoaded.insert(std::make_pair(_userName, tmp));
				return tmp;
			}
	};
	class ClientProperty {
		public:
			ClientProperty() {}
		private:
			std::string m_name;
		public:
			void setName(const std::string& _name) {
				m_name = _name
			}
			const std::string& getName() {
				return m_name;
			}
		private:
			std::vector<std::string> m_groups;
		public:
			void setGroups(std::vector<std::string> _groups) {
				m_groups = _groups
			}
			const std::vector<std::string>& getGroups() {
				return m_groups;
			}
	}
	class SystemService {
		private:
			
		public:
			SystemService() {
				APPL_WARNING("New SystemService ...");
			}
			~SystemService() {
				APPL_WARNING("delete SystemService ...");
			}
		private:
			ememory::SharedPtr<appl::User> m_user;
		public:
			int32_t getServiceCount() {
				return 0;
			}
			std::vector<std::string> getServiceList() {
				return std::vector<std::string>();
			}
		public:
			SystemService(ememory::SharedPtr<appl::User> _user) :
			  m_user(_user) {
				
			}
	};
}


int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	appl::UserManager userMng;
	jus::ServiceType<appl::SystemService, appl::UserManager> serviceInterface(userMng);
	serviceInterface.setDescription("SystemService interface");
	serviceInterface.setVersion("0.1.1");
	serviceInterface.addAuthor("Heero Yui", "yui.heero@gmail.com");
	serviceInterface.advertise("mul", &appl::SystemService::mul);
	serviceInterface.setLastFuncDesc("simple multiplication to test double IO");
	serviceInterface.addLastFuncParam("val1", "First Parameter To multiply");
	serviceInterface.addLastFuncParam("val2", "Second Parameter To multiply");
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			serviceInterface.propertyIp.set(std::string(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			serviceInterface.propertyPort.set(etk::string_to_uint16_t(std::string(&data[7])));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --ip=XXX      Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX    Server connection PORT (default: 1983)");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== JUS test service1 start      ==");
	APPL_INFO("==================================");
	serviceInterface.connect("serviceTest1");
	int32_t iii=0;
	while (true) {
		usleep(1000000);
		serviceInterface.pingIsAlive();
		APPL_INFO("service in waiting ... " << iii << "/inf");
		iii++;
	}
	serviceInterface.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== JUS test service1 stop       ==");
	APPL_INFO("==================================");
	return 0;
}
