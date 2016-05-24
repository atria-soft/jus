/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/GateWay.h>
#include <etk/etk.h>
#include <unistd.h>

#include <etk/stdTools.h>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	jus::GateWay basicGateway;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--client-ip=") == true) {
			basicGateway.propertyClientIp.set(std::string(&data[12]));
		} else if (etk::start_with(data, "--client-port=") == true) {
			basicGateway.propertyClientPort.set(etk::string_to_uint16_t(std::string(&data[14])));
		} else if (etk::start_with(data, "--client-max=") == true) {
			basicGateway.propertyClientMax.set(etk::string_to_uint16_t(std::string(&data[13])));
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
			APPL_PRINT("        --client-ip=XXX      Client connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --client-port=XXX    Client connection PORT (default: 1983)");
			APPL_PRINT("        --client-max=XXX     Clainet Maximum parallele connection (default: 80)");
			APPL_PRINT("        --service-ip=XXX     Service connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --service-port=XXX   Service connection PORT (default: 1984)");
			APPL_PRINT("        --service-max=XXX    Service Maximum IO (default: 15)");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== JUS gateway start            ==");
	APPL_INFO("==================================");
	basicGateway.start();
	while (true) {
		usleep(100000);
		basicGateway.cleanIO();
	}
	basicGateway.stop();
	APPL_INFO("==================================");
	APPL_INFO("== JUS gateway stop             ==");
	APPL_INFO("==================================");
	return 0;
}
