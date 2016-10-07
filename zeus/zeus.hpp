/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

namespace zeus {
	/**
	 * @brief Initialize zeus
	 * @param[in] _argc Number of argument list
	 * @param[in] _argv List of arguments
	 */
	void init(int _argc, const char** _argv);
	/**
	 * @brief un-Initialize zeus
	 */
	void unInit();
	/**
	 * @brief Check if the library zeus is initialized
	 * @return bool value to chek if initialize ot not
	 */
	bool isInit();
}
