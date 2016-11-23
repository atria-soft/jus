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

typedef bool (*SERVICE_IO_init_t)(int _argc, const char *_argv[], std::string _basePath);
typedef bool (*SERVICE_IO_uninit_t)();
typedef bool (*SERVICE_IO_execute_t)(std::string _ip, uint16_t _port);

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	std::string ip;
	uint16_t port = 0;
	std::string basePath;
	std::string service;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			ip = std::string(&data[5]);
		} else if (etk::start_with(data, "--port=") == true) {
			port = etk::string_to_uint16_t(std::string(&data[7]));
		} else if (etk::start_with(data, "--base-path=") == true) {
			basePath = std::string(&data[12]);
		} else if (etk::start_with(data, "--srv=") == true) {
			if (service != "") {
				APPL_ERROR("Drop sercice : '" << data << "' ==> support run only one service");
				return -1;
			}
			service = std::string(&data[6]);
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --base-path=XXX      base path to search data (default: 'USERDATA:')");
			APPL_PRINT("        --ip=XXX             Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX           Server connection PORT (default: 1983)");
			APPL_PRINT("        --srv=XXX            service path");
			return -1;
		}
	}
	if (basePath.size() == 0) {
		basePath = "USERDATA:" + service + "/";
		APPL_PRINT("Use base path: " << basePath);
	}
	void *handle = nullptr;
	std::string srv = etk::FSNodeGetApplicationPath() + "/../lib/libzeus-service-" + service + ".so";
	APPL_PRINT("Try to open service with name: '" << service << "' at position: '" << srv << "'");
	handle = dlopen(srv.c_str(), RTLD_LAZY);
	if (!handle) {
		APPL_ERROR("Can not load Lbrary:" << dlerror());
		return -1;
	}
	char *error = nullptr;
	SERVICE_IO_init_t SERVICE_IO_init = nullptr;
	SERVICE_IO_uninit_t SERVICE_IO_uninit = nullptr;
	SERVICE_IO_execute_t SERVICE_IO_execute = nullptr;
	SERVICE_IO_init = (SERVICE_IO_init_t)dlsym(handle, "SERVICE_IO_init");
	error = dlerror();
	if (error != nullptr) {
		APPL_WARNING("Can not function SERVICE_IO_init :" << error);
	}
	SERVICE_IO_uninit = (SERVICE_IO_uninit_t)dlsym(handle, "SERVICE_IO_uninit");
	error = dlerror();
	if (error != nullptr) {
		APPL_WARNING("Can not function SERVICE_IO_uninit :" << error);
	}
	SERVICE_IO_execute = (SERVICE_IO_execute_t)dlsym(handle, "SERVICE_IO_execute");
	error = dlerror();
	if (error != nullptr) {
		APPL_WARNING("Can not function SERVICE_IO_execute:" << error);
	}
	if (SERVICE_IO_init != nullptr) {
		(*SERVICE_IO_init)(_argc, _argv, basePath);
	}
	while (true) {
		if (SERVICE_IO_execute == nullptr) {
			// nothing to do ...
			break;
		}
		if ((*SERVICE_IO_execute)(ip, port) == true) {
			break;
		}
		APPL_INFO("wait 5 second ...");
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	APPL_INFO("Stop service ==> flush internal datas ...");
	if (SERVICE_IO_uninit != nullptr) {
		(*SERVICE_IO_uninit)();
	}
	return 0;
}
