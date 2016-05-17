/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>

namespace jus {
	class GateWayClient {
		public:
			jus::TcpString m_interfaceClient;
			esignal::Signal<bool> signalIsConnected;
			esignal::Connection m_dataCallback;
		public:
			GateWayClient();
			virtual ~GateWayClient();
			void start(const std::string& _ip, uint16_t _port);
			void stop();
			void onClientData(const std::string& _value);
	};
}

