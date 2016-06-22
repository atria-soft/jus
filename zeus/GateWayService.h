/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>

namespace zeus {
	class GateWay;
	class GateWayClient;
	class GateWayService {
		friend class zeus::GateWayClient;
		private:
			zeus::GateWay* m_gatewayInterface;
			zeus::TcpString m_interfaceClient;
			std::string m_name;
		public:
			esignal::Signal<bool> signalIsConnected;
		public:
			GateWayService(enet::Tcp _connection, zeus::GateWay* _gatewayInterface);
			virtual ~GateWayService();
			void start();
			void stop();
			void onServiceData(const ememory::SharedPtr<zeus::Buffer>& _value);
		public:
			void SendData(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data);
			const std::string& getName() {
				return m_name;
			}
			bool isAlive();
		protected:
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
	};
}

