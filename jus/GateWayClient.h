/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>
#include <esignal/Signal.h>
#include <jus/GateWayService.h>
#include <jus/Future.h>
#include <jus/AbstractFunction.h>



namespace jus {
	class GateWay;
	class GateWayClient {
		private:
			enum class state {
				unconnect, // starting sate
				connect, // just get a TCP connection
				userIdentify, // client set the user it want to access
				clientIdentify, // client defien the mode of the acces (anonymous,client/user)
				disconnect // client is dead or loal disconnection
			};
			enum state m_state; // state machine ...
		private:
			jus::GateWay* m_gatewayInterface;
			jus::TcpString m_interfaceClient;
		public:
			esignal::Signal<bool> signalIsConnected;
			ememory::SharedPtr<jus::GateWayService> m_userService;
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_listConnectedService;
			uint64_t m_uid;
			uint64_t m_uid2;
			std::string m_userConnectionName;
			std::string m_clientName;
			std::vector<std::string> m_clientgroups;
			std::vector<std::string> m_clientServices;
		public:
			GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface);
			virtual ~GateWayClient();
			void start(uint64_t _uid, uint64_t _uid2);
			void stop();
			void onClientData(jus::Buffer& _value);
			void returnMessage(jus::Buffer& _data);
			bool checkId(uint64_t _id) const {
				return    m_uid == _id
				       || m_uid2 == _id;
			}
			bool isAlive();
			
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			
			
	};
}

