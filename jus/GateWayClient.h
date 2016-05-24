/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>
#include <jus/GateWayService.h>

namespace jus {
	class GateWay;
	class GateWayClient {
		private:
			jus::GateWay* m_gatewayInterface;
			jus::TcpString m_interfaceClient;
		public:
			esignal::Signal<bool> signalIsConnected;
			esignal::Connection m_dataCallback;
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_listConnectedService;
			size_t m_uid;
			std::string m_userConnectionName;
		public:
			GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface);
			virtual ~GateWayClient();
			void start(size_t _uid);
			void stop();
			void onClientData(const std::string& _value);
			void returnMessage(ejson::Object _data);
			size_t getId() const {
				return m_uid;
			}
			bool isAlive();
	};
}

