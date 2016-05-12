/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <enet/Tcp.h>
#include <thread>
#include <memory>

namespace jus {
	class TcpString : public eproperty::Interface {
		private:
			enet::Tcp m_connection;
			std::unique_ptr<std::thread> m_receiveThread;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
			eproperty::Value<bool> propertyServer;
		public:
			TcpString();
			virtual ~TcpString();
			void connect();
			void disconnect();
			int32_t write(const std::string& _data);
			std::string read();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
			void onPropertyChangeServer();
	};
}

