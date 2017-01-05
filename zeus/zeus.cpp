/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <enet/enet.hpp>
#include <zeus/debug.hpp>
#include <zeus/zeus.hpp>

static bool& getInitSatatus() {
	static bool isInit = false;
	return isInit;
}

void zeus::init(int _argc, const char** _argv) {
	for (int32_t iii=0; iii<_argc; ++iii) {
		std::string value = _argv[iii];
		if (etk::start_with(value, "--zeus") == true) {
			ZEUS_ERROR("Unknow parameter type: '" << value << "'");
		}
	}
	if (getInitSatatus() == false) {
		enet::init(_argc, _argv);
		getInitSatatus() = true;
	}
}

void zeus::unInit() {
	if (getInitSatatus() == false) {
		ZEUS_ERROR("Request UnInit of enent already done ...");
	} else {
		enet::unInit();
	}
	getInitSatatus() = false;
}


bool zeus::isInit() {
	return getInitSatatus();
}

