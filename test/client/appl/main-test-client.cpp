/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Client.hpp>
#include <zeus/ObjectRemote.hpp>
#include <zeus/mineType.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>
#include <etk/uri/uri.hpp>
#include <elog/elog.hpp>


#include <etk/stdTools.hpp>
#include <zeus/service/test/ProxyService1.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	zeus::Client client1;
	etk::String fromUser = "test1";
	etk::String toUser = "test1";
	etk::String pass = "coucou";
	for (int32_t iii=0; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			client1.propertyIp.set(etk::String(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			client1.propertyPort.set(etk::string_to_uint16_t(etk::String(&data[7])));
		} else if (etk::start_with(data, "--from=") == true) {
			fromUser = &data[7];
		} else if (etk::start_with(data, "--to=") == true) {
			toUser = &data[5];
		} else if (etk::start_with(data, "--pass=") == true) {
			pass = &data[7];
		} else if (etk::start_with(data, "--tocken=") == true) {
			pass = &data[9];
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --ip=XXX      Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX    Server connection PORT (default: 1983)");
			APPL_PRINT("        --from=XXX    user that request connection");
			APPL_PRINT("        --to=XXX      destination user that we want to connect");
			APPL_PRINT("        --pass=XXX    pass or tocker to connect to the user");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client start        ==");
	APPL_INFO("==================================");
	
	if (fromUser == toUser) {
		bool ret = client1.connect(fromUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with '" << toUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Authentify with '" << toUser << "'");
		}
	} else if (fromUser != "") {
		bool ret = client1.connect(fromUser, toUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to '" << toUser << "' with '" << fromUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with '" << toUser << "'");
		}
	} else {
		bool ret = client1.connect(toUser);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous' to '" << toUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous' to '" << toUser << "'");
		}
	}
	// Connect to ourself:
	//client1.authentificate("coucou");
	//bool retAuthentify = client1.call_b("authentify", "coucou");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count");
	APPL_INFO("    ----------------------------------");
	zeus::Future<int32_t> retNbService = client1.getServiceCount();
	zeus::Future<etk::Vector<etk::String>> retServiceList = client1.getServiceList();
	retNbService.wait();
	APPL_INFO("Nb services = " << retNbService.get());
	retServiceList.wait();
	APPL_INFO("List services:");
	for (auto &it: retServiceList.get()) {
		APPL_INFO("    - " << it);
	}
	
	zeus::service::test::ProxyService1 srv = client1.getService("test-service1");
	if (srv.exist() == false) {
		APPL_ERROR("can not connect service ... 'test-service1'");
	} else {
		{
			auto retCall = srv.getU32(11);
			retCall.wait();
			APPL_INFO("value = " << retCall.get());
		}
		{
			int32_t lasNumberSend = 3000;
			auto retCall = srv.doSomething(lasNumberSend);
			int32_t lastValue=0;
			retCall.onSignal(
			    [&](int32_t _value) {
			    	APPL_DEBUG("getSignal : " << _value);
			    	if (lastValue+1 != _value) {
			    		APPL_ERROR("Send Event Wrong ORDER ... last=" << lastValue << " new=" << _value);
			    	}
			    	lastValue = _value;
			    });
			while (lasNumberSend != lastValue) {
				ethread::sleepMilliSeconds((50));
			}
			retCall.wait();
			APPL_INFO("END (receive " << lastValue << " signals");
		}
	}
	int32_t iii=0;
	while (iii < 3) {
		ethread::sleepMilliSeconds((500));
		APPL_INFO("Appl in waiting ... " << iii << "/3");
		iii++;
	}
	APPL_INFO("Request disconnect");
	client1.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client stop");
	APPL_INFO("==================================");
	return 0;
}
