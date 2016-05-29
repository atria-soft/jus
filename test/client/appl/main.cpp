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
	if (false) {
		jus::Future<bool> retIdentify = client1.call("identify", "clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
		retIdentify.wait();
		if (retIdentify.get() == false) {
			APPL_ERROR("    ==> NOT Connected with 'clientTest1#atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'clientTest1#atria-soft.com'");
		}
	} else if (true) {
		jus::Future<bool> retIdentify = client1.call("auth", "coucou");
		retIdentify.wait();
		if (retIdentify.get() == false) {
			APPL_ERROR("    ==> NOT Authentify with 'test1#atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Authentify with 'test1#atria-soft.com'");
		}
	} else {
		jus::Future<bool> retIdentify = client1.call("anonymous");
		retIdentify.wait();
		if (retIdentify.get() == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous'");
		}
	}
	// Connect to ourself:
	//client1.authentificate("coucou");
	//bool retAuthentify = client1.call_b("authentify", "coucou");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count");
	APPL_INFO("    ----------------------------------");
	jus::Future<int32_t> retNbService = client1.call("getServiceCount");
	retNbService.wait();
	APPL_INFO("Nb services = " << retNbService.get());
	jus::Future<std::vector<std::string>> retServiceList = client1.call("getServiceList");
	retServiceList.wait();
	APPL_INFO("List services:");
	for (auto &it: retServiceList.get()) {
		APPL_INFO("    - " << it);
	}
	/*
	jus::ServiceRemote localService = client1.getService("serviceTest1");
	if (localService.exist() == true) {
		double retCall = localService.call_d("mul", 13.1, 2.0);
		APPL_INFO("serviceTest1.mul = " << retCall);
	}
	*/
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service system-user");
	APPL_INFO("    ----------------------------------");
	
	jus::ServiceRemote remoteServiceUser = client1.getService("system-user");
	if (remoteServiceUser.exist() == true) {
		jus::Future<std::vector<std::string>> retCall = remoteServiceUser.call("getGroups", "clientTest1#atria-soft.com");
		retCall.wait();
		APPL_INFO("system-user.getGroups() = " << retCall.get());
		jus::Future<std::string> retVersion = remoteServiceUser.call("srv.getVersion");
		jus::Future<std::string> retType = remoteServiceUser.call("srv.getType");
		jus::Future<std::vector<std::string>> retExtention = remoteServiceUser.call("srv.getExtention");
		jus::Future<std::vector<std::string>> retMaintainer = remoteServiceUser.call("srv.getMaintainer");
		jus::Future<std::vector<std::string>> retFuctions = remoteServiceUser.call("srv.getFunctions");
		jus::Future<std::vector<std::string>> retFunctionSignature = remoteServiceUser.call("srv.getFunctionSignature", "filterServices");
		jus::Future<std::string> retFunctionPrototype = remoteServiceUser.call("srv.getFunctionPrototype", "filterServices");
		jus::Future<std::string> retFunctionHelp = remoteServiceUser.call("srv.getFunctionHelp", "filterServices");
		retVersion.wait();
		retType.wait();
		retExtention.wait();
		retMaintainer.wait();
		retFuctions.wait();
		retFunctionSignature.wait();
		retFunctionPrototype.wait();
		retFunctionHelp.wait();
		
		APPL_INFO("system-user.getClientGroups() = " << retCall.get());
		
	}
	int32_t iii=0;
	while (iii < 3) {
		usleep(500000);
		APPL_INFO("Appl in waiting ... " << iii << "/3");
		iii++;
	}
	client1.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== JUS test client stop");
	APPL_INFO("==================================");
	return 0;
}
