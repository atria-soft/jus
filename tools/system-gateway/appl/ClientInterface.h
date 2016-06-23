/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.h>
#include <appl/GateWay.h>
#include <appl/ServiceInterface.h>

namespace appl {
	class GateWay;
	class ClientInterface {
		private:
			enum class state {
				unconnect, // starting sate
				connect, // zeust get a TCP connection
				userIdentify, // client set the user it want to access
				clientIdentify, // client defien the mode of the acces (anonymous,client/user)
				disconnect // client is dead or loal disconnection
			};
			enum state m_state; // state machine ...
		private:
			appl::GateWay* m_gatewayInterface;
			zeus::WebServer m_interfaceClient;
		public:
			ememory::SharedPtr<appl::ServiceInterface> m_userService;
			std::vector<ememory::SharedPtr<appl::ServiceInterface>> m_listConnectedService;
			uint64_t m_uid;
			uint64_t m_uid2;
			std::string m_userConnectionName;
			std::string m_clientName;
			std::vector<std::string> m_clientgroups;
			std::vector<std::string> m_clientServices;
		public:
			ClientInterface(enet::Tcp _connection, appl::GateWay* _gatewayInterface);
			virtual ~ClientInterface();
			void start(uint64_t _uid, uint64_t _uid2);
			void stop();
			void onClientData(const ememory::SharedPtr<zeus::Buffer>& _value);
			void returnMessage(const ememory::SharedPtr<zeus::Buffer>& _data);
			bool checkId(uint64_t _id) const {
				return    m_uid == _id
				       || m_uid2 == _id;
			}
			bool isAlive();
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			
			
	};
}
