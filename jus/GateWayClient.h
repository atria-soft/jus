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
		public:
			jus::TcpString m_interfaceClient;
			esignal::Signal<bool> signalIsConnected;
			esignal::Connection m_dataCallback;
			bool m_returnValueOk;
			ejson::Object m_returnMessage;
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_listConnectedService;
			size_t m_uid;
		public:
			GateWayClient(jus::GateWay* _gatewayInterface);
			virtual ~GateWayClient();
			void start(const std::string& _ip, uint16_t _port, size_t _uid);
			void stop();
			void onClientData(const std::string& _value);
			void returnMessage(ejson::Object _data);
	};
}

