/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>
#include <ejson/ejson.h>

namespace jus {
	class GateWay;
	class GateWayService {
		private:
			jus::GateWay* m_gatewayInterface;
			std::string m_name;
		public:
			jus::TcpString m_interfaceClient;
			esignal::Signal<bool> signalIsConnected;
			esignal::Connection m_dataCallback;
		public:
			GateWayService(jus::GateWay* _gatewayInterface);
			virtual ~GateWayService();
			void start(const std::string& _ip, uint16_t _port);
			void stop();
			void onClientData(const std::string& _value);
		public:
			void SendData(size_t _userSessionId, ejson::Object _data);
			const std::string& getName() {
				return m_name;
			}
	};
}

