/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
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
			etk::Vector<ememory::SharedPtr<appl::GateWayInterface>> m_GateWayList; //!< List of all service availlable with their specific connection interface
			etk::Vector<ememory::SharedPtr<appl::ClientInterface>> m_clientList; //!< List of all Client interface with their own connection
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceClientServer;
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceGateWayServer;
			ejson::Document m_listUser;
		public:
			eproperty::Value<bool> propertyStdOut; //!< not set the log in the stdout or in the local file
			eproperty::Value<etk::String> propertyClientIp;
			eproperty::Value<uint16_t> propertyClientPort;
			eproperty::Value<uint16_t> propertyClientMax;
			eproperty::Value<etk::String> propertyGateWayIp;
			eproperty::Value<uint16_t> propertyGateWayPort;
			eproperty::Value<uint16_t> propertyGateWayMax;
			eproperty::Value<int32_t> propertyDelayToStop;
		public:
			Router();
			virtual ~Router();
			void start();
			void stop();
			// Get a specific user gateway:
			ememory::SharedPtr<appl::GateWayInterface> get(const etk::String& _userName);
			bool userIsConnected(const etk::String& _userName);
			
			etk::Vector<etk::String> getAllUserName();
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

