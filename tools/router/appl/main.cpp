/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/Router.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>


#include <etk/stdTools.hpp>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	appl::Router basicRouter;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--client-ip=") == true) {
			basicRouter.propertyClientIp.set(std::string(&data[12]));
		} else if (etk::start_with(data, "--client-port=") == true) {
			basicRouter.propertyClientPort.set(etk::string_to_uint16_t(std::string(&data[14])));
		} else if (etk::start_with(data, "--client-max=") == true) {
			basicRouter.propertyClientMax.set(etk::string_to_uint16_t(std::string(&data[13])));
		} else if (etk::start_with(data, "--gw-ip=") == true) {
			basicRouter.propertyGateWayIp.set(std::string(&data[8]));
		} else if (etk::start_with(data, "--gw-port=") == true) {
			basicRouter.propertyGateWayPort.set(etk::string_to_uint16_t(std::string(&data[10])));
		} else if (etk::start_with(data, "--gw-max=") == true) {
			basicRouter.propertyGateWayMax.set(etk::string_to_uint16_t(std::string(&data[9])));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --client-ip=XXX      Client connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --client-port=XXX    Client connection PORT (default: 1983)");
			APPL_PRINT("        --client-max=XXX     Client Maximum parallele connection (default: 80)");
			APPL_PRINT("        --gw-ip=XXX          Gateway connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --gw-port=XXX        Gateway connection PORT (default: 1984)");
			APPL_PRINT("        --gw-max=XXX         Gateway Maximum IO (default: 15)");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS router start            ==");
	APPL_INFO("==================================");
	basicRouter.start();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		basicRouter.cleanIO();
	}
	basicRouter.stop();
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS router stop             ==");
	APPL_INFO("==================================");
	return 0;
}
