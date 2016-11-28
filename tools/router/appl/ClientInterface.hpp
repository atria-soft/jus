/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/Router.hpp>
#include <appl/GateWayInterface.hpp>

namespace appl {
	class Router;
	class ClientInterface {
		private:
			enum class state {
				unconnect, // starting sate
				connect, // zeust get a TCP connection
				disconnect // client is dead or loal disconnection
			};
			enum state m_state; // state machine ...
		private:
			appl::Router* m_routerInterface;
			zeus::WebServer m_interfaceClient;
			bool requestURI(const std::string& _uri);
		public:
			ememory::SharedPtr<appl::GateWayInterface> m_userGateWay;
			uint64_t m_uid; //!< gateway unique ID ==> to have an internal routage ...
		public:
			ClientInterface(enet::Tcp _connection, appl::Router* _routerInterface);
			virtual ~ClientInterface();
			void start(uint64_t _uid);
			void stop();
			void onClientData(ememory::SharedPtr<zeus::Buffer> _value);
			void returnMessage(ememory::SharedPtr<zeus::Buffer> _data);
			bool checkId(uint64_t _id) const {
				return m_uid == _id;
			}
			bool isAlive();
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			
			
	};
}

