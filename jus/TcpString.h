/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <eproperty/Value.h>
#include <esignal/Signal.h>
#include <enet/Tcp.h>
#include <thread>
#include <memory>

namespace jus {
	class TcpString : public eproperty::Interface {
		private:
			enet::Tcp m_connection;
			std::thread* m_thread;
			bool m_threadRunning;
		public:
			eproperty::Value<std::string> propertyIp;
			eproperty::Value<uint16_t> propertyPort;
			eproperty::Value<bool> propertyServer;
			esignal::Signal<bool> signalIsConnected;
			esignal::Signal<std::string> signalData;
		public:
			TcpString();
			virtual ~TcpString();
			void connect(bool _async = false);
			void disconnect();
			void setInterfaceName(const std::string& _name);
			int32_t write(const std::string& _data);
			std::string asyncRead();
		private:
			std::string read();
		private:
			void onPropertyChangeIp();
			void onPropertyChangePort();
			void onPropertyChangeServer();
			void threadCallback();
	};
}

