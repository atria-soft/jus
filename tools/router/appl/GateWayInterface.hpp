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
		friend class appl::ClientInterface;
		private:
			appl::Router* m_routerInterface;
			zeus::WebServer m_interfaceClient;
			std::string m_name;
			bool requestURI(const std::string& _uri);
		public:
			GateWayInterface(enet::Tcp _connection, appl::Router* _routerInterface);
			virtual ~GateWayInterface();
			void start();
			void stop();
			void onServiceData(ememory::SharedPtr<zeus::Buffer> _value);
		public:
			void SendData(uint64_t _userSessionId, ememory::SharedPtr<zeus::Buffer> _data);
			const std::string& getName() {
				return m_name;
			}
			bool isAlive();
		protected:
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
	};
}

