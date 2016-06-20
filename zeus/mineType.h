/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>

namespace zeus {
	std::string getMineType(const std::string& _extention);
	std::string getExtention(const std::string& _mineType);
};
