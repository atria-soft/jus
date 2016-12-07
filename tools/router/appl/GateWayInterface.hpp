/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
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
			uint16_t m_lastSourceID; //!< The source dynbamic generated ID is manage in 2 part the value <= 0x7FFF is used by the gateway and the value >= 0x8000 is manage by the router
			std::vector<ememory::SharedPtr<appl::ClientInterface>> m_clientConnected;
			std::string m_name;
			bool requestURI(const std::string& _uri);
		public:
			GateWayInterface(enet::Tcp _connection, appl::Router* _routerInterface);
			virtual ~GateWayInterface();
			void start();
			void stop();
			void onServiceData(ememory::SharedPtr<zeus::Message> _value);
		public:
			uint16_t addClient(ememory::SharedPtr<appl::ClientInterface> _value);
			void rmClient(ememory::SharedPtr<appl::ClientInterface> _value);
			void send(ememory::SharedPtr<zeus::Message> _data);
			const std::string& getName() {
				return m_name;
			}
			bool isAlive();
		protected:
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
	};
}

