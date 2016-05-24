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
			jus::TcpString m_interfaceClient;
			std::string m_name;
		public:
			esignal::Signal<bool> signalIsConnected;
		public:
			GateWayService(enet::Tcp _connection, jus::GateWay* _gatewayInterface);
			virtual ~GateWayService();
			void start();
			void stop();
			void onServiceData(std::string _value);
		public:
			void SendData(size_t _userSessionId, ejson::Object _data, const std::string& _action="call");
			const std::string& getName() {
				return m_name;
			}
			bool isAlive();
	};
}

