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



namespace jus {
	class GateWay;
	class GateWayClient {
		using Observer = std::function<void(ejson::Object& _data)>;
		private:
			jus::GateWay* m_gatewayInterface;
			jus::TcpString m_interfaceClient;
		public:
			esignal::Signal<bool> signalIsConnected;
			ememory::SharedPtr<jus::GateWayService> m_userService;
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_listConnectedService;
			size_t m_uid;
			std::string m_userConnectionName;
			std::string m_clientName;
			std::vector<std::string> m_clientgroups;
			std::mutex m_mutex;
			std::vector<std::pair<int32_t, Observer>> m_actions;
			int32_t m_transactionLocalId;
		public:
			GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface);
			virtual ~GateWayClient();
			void start(size_t _uid);
			void stop();
			void onClientData(std::string _value);
			void returnMessage(ejson::Object _data);
			size_t getId() const {
				return m_uid;
			}
			bool isAlive();
	};
}

