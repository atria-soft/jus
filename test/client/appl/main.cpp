/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/Client.h>
#include <jus/ServiceRemote.h>
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
	client1.connect("test1#atria-soft.com");
	// Connect that is not us
	//client1.identify("clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
	jus::Future<bool> retIdentify = client1.callAsync<bool>("identify", "clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
	retIdentify.wait();
	//bool retIdentify = client1.call_b("identify", "clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
	
	// Connect to ourself:
	//client1.authentificate("coucou");
	//bool retAuthentify = client1.call_b("authentify", "coucou");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count            --");
	APPL_INFO("    ----------------------------------");
	/*
	std::vector<double> tmp;
	tmp.push_back(1);
	tmp.push_back(22);
	tmp.push_back(333);
	tmp.push_back(4444);
	int32_t val = client1.call_i("getServiceCount", tmp, "coucou", false);
	APPL_INFO("Nb services = " << val);
	std::vector<std::string> val2 = client1.call_vs("getServiceList");
	APPL_INFO("List services:");
	for (auto &it: val2) {
		APPL_INFO("    - " << it);
	}
	jus::ServiceRemote localService = client1.getService("serviceTest1");
	if (localService.exist() == true) {
		double retCall = localService.call_d("mul", 13.1, 2.0);
		APPL_INFO("serviceTest1.mul = " << retCall);
	}
	*/
	
	jus::ServiceRemote remoteServiceUser = client1.getService("system-user");
	if (remoteServiceUser.exist() == true) {
		std::vector<std::string> retCall = remoteServiceUser.call_vs("getClientGroups");
		APPL_INFO("system-user.getClientGroups() = " << retCall);
	}
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
