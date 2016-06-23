/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.h>
#include <ememory/memory.h>

namespace appl {
	class GateWay;
	class ClientInterface;
	class ServiceInterface {
		friend class appl::ClientInterface;
		private:
			appl::GateWay* m_gatewayInterface;
			zeus::WebServer m_interfaceClient;
			std::string m_name;
		public:
			ServiceInterface(enet::Tcp _connection, appl::GateWay* _gatewayInterface);
			virtual ~ServiceInterface();
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

