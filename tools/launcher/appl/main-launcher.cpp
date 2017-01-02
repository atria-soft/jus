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
#include <zeus/Object.hpp>
#include <zeus/Client.hpp>
#include <zeus/zeus.hpp>

typedef bool (*SERVICE_IO_init_t)(int _argc, const char *_argv[], std::string _basePath);
typedef bool (*SERVICE_IO_uninit_t)();
typedef void (*SERVICE_IO_peridic_call_t)();
typedef zeus::Object* (*SERVICE_IO_instanciate_t)(uint32_t, ememory::SharedPtr<zeus::WebServer>&, uint32_t);

class PlugginAccess {
	private:
		std::string m_name;
		void* m_handle;
		SERVICE_IO_init_t m_SERVICE_IO_init;
		SERVICE_IO_uninit_t m_SERVICE_IO_uninit;
		SERVICE_IO_peridic_call_t m_SERVICE_IO_peridic_call;
		SERVICE_IO_instanciate_t m_SERVICE_IO_instanciate;
	public:
		PlugginAccess(const std::string& _name) :
		  m_name(_name),
		  m_handle(nullptr),
		  m_SERVICE_IO_init(nullptr),
		  m_SERVICE_IO_uninit(nullptr),
		  m_SERVICE_IO_instanciate(nullptr) {
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
			m_SERVICE_IO_peridic_call = (SERVICE_IO_peridic_call_t)dlsym(m_handle, "SERVICE_IO_peridic_call");
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
		bool publish(zeus::Client& _client) {
			if (m_SERVICE_IO_instanciate == nullptr) {
				return false;
			}
			_client.serviceAdd(m_name,  [=](uint32_t _transactionId, ememory::SharedPtr<zeus::WebServer>& _iface, uint32_t _destination) {
			                            	(*m_SERVICE_IO_instanciate)(_transactionId, _iface, _destination);
			                            });
			return true;
		}
		bool disconnect(zeus::Client& _client) {
			_client.serviceRemove(m_name);
			return true;
		}
		bool uninit() {
			if (m_SERVICE_IO_uninit == nullptr) {
				return false;
			}
			return (*m_SERVICE_IO_uninit)();
		}
		void peridic_call() {
			if (m_SERVICE_IO_peridic_call == nullptr) {
				return;
			}
			(*m_SERVICE_IO_peridic_call)();
		}
};


int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	std::string ip;
	uint16_t port = 1985;
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
			APPL_PRINT("        --port=XXX           Server connection PORT (default: 1985)");
			APPL_PRINT("        --srv=XXX            service path (N)");
			return -1;
		}
	}
	zeus::Client m_client;
	std::vector<ememory::SharedPtr<PlugginAccess>> listElements;
	
	for (auto &it: services) {
		ememory::SharedPtr<PlugginAccess> tmp = ememory::makeShared<PlugginAccess>(it);
		listElements.push_back(tmp);
	}
	
	for (auto &it: listElements) {
		it->init(_argc, _argv, basePath);
	}
	if (ip != "") {
		m_client.propertyIp.set(ip);
	}
	if (port != 0) {
		m_client.propertyPort.set(port);
	}
	if (m_client.connect() == false) {
		return -1;
	}
	for (auto &it: listElements) {
		it->publish(m_client);
	}
	uint32_t iii = 0;
	while(m_client.isAlive() == true) {
		m_client.pingIsAlive();
		m_client.displayConnectedObject();
		m_client.cleanDeadObject();
		for (auto &it: listElements) {
			it->peridic_call();
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		APPL_INFO("service in waiting ... " << iii << "/inf");
		iii++;
	}
	for (auto &it: listElements) {
		it->disconnect(m_client);
	}
	m_client.disconnect();
	APPL_INFO("Stop service*** ==> flush internal datas ...");
	for (auto &it: listElements) {
		it->uninit();
	}
	return 0;
}

