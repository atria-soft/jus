/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */


#include <appl/debug.hpp>

int32_t appl::getLogId() {
	static int32_t g_val = elog::registerInstance("zeus-server-remote");
	return g_val;
}
