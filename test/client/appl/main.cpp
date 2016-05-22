/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/Client.h>
#include <etk/etk.h>
#include <unistd.h>

#include <etk/stdTools.h>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	jus::Client client1;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			client1.propertyIp.set(std::string(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			client1.propertyPort.set(etk::string_to_uint16_t(std::string(&data[7])));
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
	APPL_INFO("== JUS test client start        ==");
	APPL_INFO("==================================");
	client1.connect("userName");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count            --");
	APPL_INFO("    ----------------------------------");
	
	std::vector<double> tmp;
	tmp.push_back(1);
	tmp.push_back(22);
	tmp.push_back(333);
	tmp.push_back(4444);
	int32_t val = client1.call_i("", "getServiceCount", tmp, "coucou", false);
	APPL_INFO("Nb services = " << val);
	std::vector<std::string> val2 = client1.call_vs("", "getServiceList");
	APPL_INFO("List services:");
	for (auto &it: val2) {
		APPL_INFO("    - " << it);
	}
	// TODO:  add return value
	bool valConnect = client1.call_b("serviceTest1", "link");
	APPL_INFO("Link service 'serviceTest1' ret=" << valConnect);
	
	bool retCall = client1.call_d("serviceTest1", "mul", 13.1, 2.0);
	APPL_INFO("serviceTest1.mul = " << retCall);
	
	valConnect = client1.call_b("serviceTest1", "unlink");
	APPL_INFO("un-Link service 'serviceTest1' ret=" << valConnect);
	
	int32_t iii=0;
	while (iii < 3) {
		usleep(500000);
		APPL_INFO("Appl in waiting ... " << iii << "/3");
		iii++;
	}
	client1.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== JUS test client stop        ==");
	APPL_INFO("==================================");
	return 0;
}
