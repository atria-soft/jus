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
	class GateWay;
	class GateWayClient;
	class GateWayService {
		friend class jus::GateWayClient;
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
			void onServiceData(jus::Buffer& _value);
		public:
			void SendData(uint64_t _userSessionId, jus::Buffer& _data);
			const std::string& getName() {
				return m_name;
			}
			bool isAlive();
		protected:
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
	};
}
