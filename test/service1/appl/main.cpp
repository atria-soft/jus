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
	class Service1 : public jus::Service {
		private:
			double mul(const double& _val1) {//, const double& _val2) {
				double _val2 = 1.0f;
				return _val1*_val2;
			}
		public:
			Service1() {
				advertise("mul", &appl::Service1::mul, "simple multiplication to test double IO");
			}
	};
}


int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	appl::Service1 service1;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			service1.propertyIp.set(std::string(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			service1.propertyPort.set(etk::string_to_uint16_t(std::string(&data[7])));
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
	/*
	service1.connect();
	int32_t iii=0;
	while (true) {
		usleep(500000);
		APPL_INFO("Appl in waiting ... " << iii << "/inf");
		iii++;
	}
	service1.disconnect();
	*/
	APPL_INFO("==================================");
	APPL_INFO("== JUS test service1 stop       ==");
	APPL_INFO("==================================");
	return 0;
}
