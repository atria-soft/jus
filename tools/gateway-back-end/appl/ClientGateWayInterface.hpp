/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/GateWay.hpp>
#include <appl/ServiceInterface.hpp>

namespace appl {
	class GateWay;
	enum class clientState {
		unconnect, // starting sate
		connect, // zeust get a TCP connection
		clientIdentify, // client defien the mode of the acces (anonymous,client/user)
		disconnect // client is dead or loal disconnection
	};
	class userSpecificInterface {
		public:
			zeus::WebServer* m_interfaceGateWayClient;
			appl::GateWay* m_gatewayInterface;
			uint64_t m_uid;
			uint64_t m_localIdUser;
			enum clientState m_state; // state machine ...
			std::vector<ememory::SharedPtr<appl::ServiceInterface>> m_listConnectedService;
			ememory::SharedPtr<appl::ServiceInterface> m_userService;
			std::string m_userConnectionName;
			std::string m_clientName;
			std::vector<std::string> m_clientgroups;
			std::vector<std::string> m_clientServices;
			userSpecificInterface();
			~userSpecificInterface();
			bool start(uint32_t _transactionId, appl::GateWay* _gatewayInterface, zeus::WebServer* _interfaceGateWayClient, uint64_t _id);
			void onClientData(ememory::SharedPtr<zeus::Buffer> _value);
			void returnMessage(ememory::SharedPtr<zeus::Buffer> _data);
			bool checkId(uint64_t _id) const {
				return    m_uid == _id
				       || m_localIdUser == _id;
			}
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
	};
	class ClientGateWayInterface {
		private:
			enum clientState m_state; // state machine ..
			std::vector<userSpecificInterface> m_listUser;
		private:
			appl::GateWay* m_gatewayInterface;
			zeus::WebServer m_interfaceGateWayClient;
		public:
			ClientGateWayInterface(const std::string& _ip, uint16_t _port, const std::string& _userName, appl::GateWay* _gatewayInterface);
			virtual ~ClientGateWayInterface();
			void stop();
			void onClientData(ememory::SharedPtr<zeus::Buffer> _value);
			bool isAlive();
			void answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data);
			void clean();
			
			
	};
}

