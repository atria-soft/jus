/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/GateWay.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>


#include <etk/stdTools.hpp>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	appl::GateWay basicGateway;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--user=") == true) {
			basicGateway.propertyUserName.set(std::string(&data[7]));
		} else if (etk::start_with(data, "--router-ip=") == true) {
			basicGateway.propertyRouterIp.set(std::string(&data[12]));
		} else if (etk::start_with(data, "--router-port=") == true) {
			basicGateway.propertyRouterPort.set(etk::string_to_uint16_t(std::string(&data[14])));
		} else if (etk::start_with(data, "--service-ip=") == true) {
			basicGateway.propertyServiceIp.set(std::string(&data[13]));
		} else if (etk::start_with(data, "--service-port=") == true) {
			basicGateway.propertyServicePort.set(etk::string_to_uint16_t(std::string(&data[15])));
		} else if (etk::start_with(data, "--service-max=") == true) {
			basicGateway.propertyServiceMax.set(etk::string_to_uint16_t(std::string(&data[14])));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --user=XXX           Name of the user that we are connected.");
			APPL_PRINT("        --router-ip=XXX      Router connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --router-port=XXX    Router connection PORT (default: 1984)");
			APPL_PRINT("        --service-ip=XXX     Service connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --service-port=XXX   Service connection PORT (default: 1982)");
			APPL_PRINT("        --service-max=XXX    Service Maximum IO (default: 15)");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS gateway start            ==");
	APPL_INFO("==================================");
	basicGateway.start();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		basicGateway.cleanIO();
	}
	basicGateway.stop();
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS gateway stop             ==");
	APPL_INFO("==================================");
	return 0;
}
