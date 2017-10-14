/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <ememory/memory.hpp>

namespace appl {
	class Router;
	class ClientInterface;
	class GateWayInterface {
		private:
			appl::Router* m_routerInterface;
			zeus::WebServer m_interfaceClient;
			uint16_t m_openExternPort;
			uint16_t m_lastSourceID; //!< The source dynbamic generated ID is manage in 2 part the value <= 0x7FFF is used by the gateway and the value >= 0x8000 is manage by the router
			etk::Vector<ememory::SharedPtr<appl::ClientInterface>> m_clientConnected;
			etk::String m_name;
			etk::String requestURI(const etk::String& _uri, const etk::Map<etk::String,etk::String>& _options);
		public:
			GateWayInterface(enet::Tcp _connection, appl::Router* _routerInterface);
			virtual ~GateWayInterface();
			void start();
			void stop();
			void onServiceData(ememory::SharedPtr<zeus::Message> _value);
			void clientAlivePing();
		public:
			uint16_t addClient(ememory::SharedPtr<appl::ClientInterface> _value);
			void rmClient(ememory::SharedPtr<appl::ClientInterface> _value);
			void send(ememory::SharedPtr<zeus::Message> _data);
			const etk::String& getName() {
				return m_name;
			}
			bool isAlive();
			/**
			 * @brief Get the open port for external direct connection on the gateway
			 * @return Port id or 0 if not open.
			 */
			uint16_t getOpenExternalPort() const {
				return m_openExternPort;
			}
		protected:
			void answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp);
	};
}

