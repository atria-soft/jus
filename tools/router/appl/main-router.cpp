/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/Router.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>
#include <etk/Allocator.hpp>


#include <etk/stdTools.hpp>
#include <algue/sha512.hpp>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	zeus::init(_argc, _argv);
	//etk::String OUT2 = algue::stringConvert(algue::sha512::encode("jkhlkjhlkjhlkjhlkjhlkjhlkjhiugouuyrtfkvjhbnj,owixhuvkfn r;,dcxwjo ppxicodsn,kwp<uivfknejc<wxphogkbezdsj<cxhwdséznreS<WJCXHKJ"));
	//APPL_PRINT("2: " << OUT2);
	//return 0;
	appl::Router basicRouter;
	for (int32_t iii=0; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
		if (data == "--stdout") {
			basicRouter.propertyStdOut.set(true);
		} else if (etk::start_with(data, "--client-ip=") == true) {
			basicRouter.propertyClientIp.set(etk::String(&data[12]));
		} else if (etk::start_with(data, "--client-port=") == true) {
			basicRouter.propertyClientPort.set(etk::string_to_uint16_t(etk::String(&data[14])));
		} else if (etk::start_with(data, "--client-max=") == true) {
			basicRouter.propertyClientMax.set(etk::string_to_uint16_t(etk::String(&data[13])));
		} else if (etk::start_with(data, "--gw-ip=") == true) {
			basicRouter.propertyGateWayIp.set(etk::String(&data[8]));
		} else if (etk::start_with(data, "--gw-port=") == true) {
			basicRouter.propertyGateWayPort.set(etk::string_to_uint16_t(etk::String(&data[10])));
		} else if (etk::start_with(data, "--gw-max=") == true) {
			basicRouter.propertyGateWayMax.set(etk::string_to_uint16_t(etk::String(&data[9])));
		} else if (etk::start_with(data, "--delay-stop-user=") == true) {
			basicRouter.propertyDelayToStop.set(etk::string_to_int32_t(etk::String(&data[18])));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --stdout                stdout log");
			APPL_PRINT("        --client-ip=XXX         Client connection IP (default: " << basicRouter.propertyClientIp.get() << ")");
			APPL_PRINT("        --client-port=XXX       Client connection PORT (default: " << basicRouter.propertyClientPort.get() << ")");
			APPL_PRINT("        --client-max=XXX        Client Maximum parallele connection (default: " << basicRouter.propertyClientMax.get() << ")");
			APPL_PRINT("        --gw-ip=XXX             Gateway connection IP (default: " << basicRouter.propertyGateWayIp.get() << ")");
			APPL_PRINT("        --gw-port=XXX           Gateway connection PORT (default: " << basicRouter.propertyGateWayPort.get() << ")");
			APPL_PRINT("        --gw-max=XXX            Gateway Maximum IO (default: " << basicRouter.propertyGateWayMax.get() << ")");
			APPL_PRINT("        --delay-stop-user=XXX   Delay before the client stop the connection in second (default: " << basicRouter.propertyDelayToStop.get() << "0=automatic set by the gateway; -1=never disconnect; other the time )");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS router start            ==");
	APPL_INFO("==================================");
	basicRouter.start();
	int32_t countMemeCheck = 0;
	while (true) {
		ethread::sleepMilliSeconds((100));
		basicRouter.cleanIO();
		if (countMemeCheck++ >= 200) {
			countMemeCheck = 0;
			ETK_MEM_SHOW_LOG(true);
		}
	}
	basicRouter.stop();
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS router stop             ==");
	APPL_INFO("==================================");
	elog::flush();
	return 0;
}
