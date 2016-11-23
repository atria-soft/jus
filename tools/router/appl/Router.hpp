/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <appl/GateWayInterface.hpp>
#include <appl/ClientInterface.hpp>
#include <eproperty/Value.hpp>
#include <ejson/Document.hpp>

namespace appl {
	class TcpServerInput;
	class Router : public eproperty::Interface {
		private:
			uint64_t m_clientUID;
		private:
			std::vector<ememory::SharedPtr<appl::GateWayInterface>> m_GateWayList; //!< List of all service availlable with their specific connection interface
			std::vector<ememory::SharedPtr<appl::ClientInterface>> m_clientList; //!< List of all Client interface with their own connection
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceClientServer;
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceGateWayServer;
			ejson::Document m_listUser;
		public:
			eproperty::Value<std::string> propertyClientIp;
			eproperty::Value<uint16_t> propertyClientPort;
			eproperty::Value<uint16_t> propertyClientMax;
			eproperty::Value<std::string> propertyGateWayIp;
			eproperty::Value<uint16_t> propertyGateWayPort;
			eproperty::Value<uint16_t> propertyGateWayMax;
		public:
			Router();
			virtual ~Router();
			void start();
			void stop();
			// Get a specific user gateway:
			ememory::SharedPtr<appl::GateWayInterface> get(const std::string& _userName);
			
			std::vector<std::string> getAllUserName();
			void answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data);
			void newClientGateWay(enet::Tcp _connection);
			void newClient(enet::Tcp _connection);
			void cleanIO();
		private:
			void onPropertyChangeClientIp();
			void onPropertyChangeClientPort();
			void onPropertyChangeClientMax();
			void onPropertyChangeGateWayIp();
			void onPropertyChangeGateWayPort();
			void onPropertyChangeGateWayMax();
			void onClientConnect(const bool& _value);
			void onServiceConnect(const bool& _value);
	};
}

