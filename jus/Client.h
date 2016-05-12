/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>

namespace jus {
	class Client {
		private:
			jus::TcpString m_interfaceClient;
		public:
			Client() {}
			virtual ~Client() {}
	};
}

