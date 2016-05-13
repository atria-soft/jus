/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>

namespace jus {
	class Client : public eproperty::Interface {
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
		private:
			jus::TcpString m_interfaceClient;
		public:
			Client();
			virtual ~Client();
			void connect();
			void disconnect();
			
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
	};
}

