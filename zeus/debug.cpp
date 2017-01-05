/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/debug.hpp>

int32_t zeus::getLogId() {
	static int32_t g_val = elog::registerInstance("zeus");
	return g_val;
}
