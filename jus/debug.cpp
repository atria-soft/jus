/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>

int32_t jus::getLogId() {
	static int32_t g_val = elog::registerInstance("jus");
	return g_val;
}
