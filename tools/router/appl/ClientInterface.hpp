/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/Router.hpp>
#include <appl/GateWayInterface.hpp>
#include <ememory/memory.hpp>

namespace appl {
	class Router;
	class GateWayInterface;
	class ClientInterface : public ememory::EnableSharedFromThis<appl::ClientInterface> {
		private:
			appl::Router* m_routerInterface;
			zeus::WebServer m_interfaceClient;
			bool requestURI(const std::string& _uri);
		public:
			ememory::SharedPtr<appl::GateWayInterface> m_userGateWay;
			uint16_t m_uid; //!< gateway unique ID ==> to have an internal routage ...
		public:
			ClientInterface(enet::Tcp _connection, appl::Router* _routerInterface);
			virtual ~ClientInterface();
			void start();
			void stop();
			void onClientData(ememory::SharedPtr<zeus::Message> _value);
			void send(ememory::SharedPtr<zeus::Message> _data);
			bool checkId(uint16_t _id) const {
				return m_uid == _id;
			}
			bool isAlive();
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			
			
	};
}

