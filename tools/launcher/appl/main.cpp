/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <etk/etk.hpp>

#include <mutex>
#include <etk/os/FSNode.hpp>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <thread>
#include <etk/stdTools.hpp>
#include <zeus/Service.hpp>
#include <zeus/zeus.hpp>

typedef bool (*SERVICE_IO_init_t)(int _argc, const char *_argv[], std::string _basePath);
typedef bool (*SERVICE_IO_uninit_t)();
typedef zeus::Service* (*SERVICE_IO_instanciate_t)();

class PlugginAccess {
	private:
		std::string m_name;
		void* m_handle;
		SERVICE_IO_init_t m_SERVICE_IO_init;
		SERVICE_IO_uninit_t m_SERVICE_IO_uninit;
		SERVICE_IO_instanciate_t m_SERVICE_IO_instanciate;
		zeus::Service* m_srv;
	public:
		PlugginAccess(const std::string& _name) :
		  m_name(_name),
		  m_handle(nullptr),
		  m_SERVICE_IO_init(nullptr),
		  m_SERVICE_IO_uninit(nullptr),
		  m_SERVICE_IO_instanciate(nullptr),
		  m_srv(nullptr){
			std::string srv = etk::FSNodeGetApplicationPath() + "/../lib/libzeus-service-" + m_name + "-impl.so";
			APPL_PRINT("Try to open service with name: '" << m_name << "' at position: '" << srv << "'");
			m_handle = dlopen(srv.c_str(), RTLD_LAZY);
			if (!m_handle) {
				APPL_ERROR("Can not load Lbrary:" << dlerror());
				return;
			}
			char *error = nullptr;
			m_SERVICE_IO_init = (SERVICE_IO_init_t)dlsym(m_handle, "SERVICE_IO_init");
			error = dlerror();
			if (error != nullptr) {
				m_SERVICE_IO_init = nullptr;
				APPL_WARNING("Can not function SERVICE_IO_init :" << error);
			}
			m_SERVICE_IO_uninit = (SERVICE_IO_uninit_t)dlsym(m_handle, "SERVICE_IO_uninit");
			error = dlerror();
			if (error != nullptr) {
				m_SERVICE_IO_uninit = nullptr;
				APPL_WARNING("Can not function SERVICE_IO_uninit :" << error);
			}
			m_SERVICE_IO_instanciate = (SERVICE_IO_instanciate_t)dlsym(m_handle, "SERVICE_IO_instanciate");
			error = dlerror();
			if (error != nullptr) {
				m_SERVICE_IO_instanciate = nullptr;
				APPL_WARNING("Can not function SERVICE_IO_instanciate:" << error);
			}
		}
		~PlugginAccess() {
			if (m_srv != nullptr) {
				delete m_srv;
				m_srv = nullptr;
			}
		}
		bool init(int _argc, const char *_argv[], std::string _basePath) {
			if (m_SERVICE_IO_init == nullptr) {
				return false;
			}
			
			if (_basePath.size() == 0) {
				_basePath = "USERDATA:" + m_name + "/";
				APPL_PRINT("Use base path: " << _basePath);
			}
			return (*m_SERVICE_IO_init)(_argc, _argv, _basePath);
		}
		bool uninit() {
			if (m_SERVICE_IO_uninit == nullptr) {
				return false;
			}
			return (*m_SERVICE_IO_uninit)();
		}
		bool instanciate() {
			if (m_SERVICE_IO_instanciate == nullptr) {
				return false;
			}
			m_srv = (*m_SERVICE_IO_instanciate)();
			return m_srv != nullptr;
		}
		bool connect(std::string _ip, uint16_t _port) {
			if (m_srv == nullptr) {
				return false;
			}
			if (_ip != "") {
				m_srv->propertyIp.set(_ip);
			}
			if (_port != 0) {
				m_srv->propertyPort.set(_port);
			}
			if (m_srv->connect() == false) {
				return false;
			}
			if (m_srv->GateWayAlive() == false) {
				APPL_INFO("===========================================================");
				APPL_INFO("== ZEUS service: " << *m_srv->propertyNameService << " [STOP] Can not connect to the GateWay");
				APPL_INFO("===========================================================");
				//delete m_srv;
				//m_srv = nullptr;
				return false;
			}
			return true;
		}
		bool ping() {
			if (m_srv == nullptr) {
				return false;
			}
			if (m_srv->GateWayAlive() == true) {
				m_srv->pingIsAlive();
			}
			return m_srv->GateWayAlive();
		}
		bool disconnect() {
			if (m_srv == nullptr) {
				return false;
			}
			m_srv->disconnect();
			APPL_INFO("===========================================================");
			APPL_INFO("== ZEUS service: " << *m_srv->propertyNameService << " [STOP] GateWay Stop");
			APPL_INFO("===========================================================");
			return true;
		}
};


int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	std::string ip;
	uint16_t port = 0;
	std::string basePath;
	std::vector<std::string> services;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			ip = std::string(&data[5]);
		} else if (etk::start_with(data, "--port=") == true) {
			port = etk::string_to_uint16_t(std::string(&data[7]));
		} else if (etk::start_with(data, "--base-path=") == true) {
			basePath = std::string(&data[12]);
		} else if (etk::start_with(data, "--srv=") == true) {
			services.push_back(std::string(&data[6]));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --base-path=XXX      base path to search data (default: 'USERDATA:')");
			APPL_PRINT("        --ip=XXX             Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX           Server connection PORT (default: 1983)");
			APPL_PRINT("        --srv=XXX            service path (N)");
			return -1;
		}
	}
	std::vector<ememory::SharedPtr<PlugginAccess>> listElements;
	
	for (auto &it: services) {
		ememory::SharedPtr<PlugginAccess> tmp = ememory::makeShared<PlugginAccess>(it);
		listElements.push_back(tmp);
	}
	
	for (auto &it: listElements) {
		it->init(_argc, _argv, basePath);
	}
	for (auto &it: listElements) {
		it->instanciate();
	}
	for (auto &it: listElements) {
		it->connect(ip, port);
	}
	uint32_t iii = 0;
	bool oneAlive = true;
	while (oneAlive == true) {
		oneAlive = false;
		for (auto &it: listElements) {
			if (it->ping() == true) {
				oneAlive = true;
			}
		}
		if (oneAlive == true) {
			std::this_thread::sleep_for(std::chrono::seconds(10));
			APPL_INFO("service in waiting ... " << iii << "/inf");
			iii++;
		}
	}
	for (auto &it: listElements) {
		it->disconnect();
	}
	APPL_INFO("Stop service*** ==> flush internal datas ...");
	for (auto &it: listElements) {
		it->uninit();
	}
	return 0;
}

