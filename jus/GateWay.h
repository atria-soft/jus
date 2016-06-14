/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <jus/GateWayService.h>
#include <jus/GateWayClient.h>
#include <ejson/ejson.h>

namespace jus {
	class TcpServerInput;
	class GateWay : public eproperty::Interface {
		private:
			uint64_t m_clientUID;
		private:
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_serviceList; //!< List of all service availlable with their specific connection interface
			std::vector<ememory::SharedPtr<jus::GateWayClient>> m_clientList; //!< List of all Client interface with their own connection
			//TODO: std::vector<jus::GateWayServer> m_ServerList; //!< List of all Server connected to this gateway
			ememory::SharedPtr<jus::TcpServerInput> m_interfaceClientServer;
			ememory::SharedPtr<jus::TcpServerInput> m_interfaceServiceServer;
		public:
			eproperty::Value<std::string> propertyClientIp;
			eproperty::Value<uint16_t> propertyClientPort;
			eproperty::Value<uint16_t> propertyClientMax;
			eproperty::Value<std::string> propertyServiceIp;
			eproperty::Value<uint16_t> propertyServicePort;
			eproperty::Value<uint16_t> propertyServiceMax;
		public:
			GateWay();
			virtual ~GateWay();
			void start();
			void stop();
			ememory::SharedPtr<jus::GateWayService> get(const std::string& _serviceName);
			std::vector<std::string> getAllServiceName();
			void answer(uint64_t _userSessionId, jus::Buffer& _data);
			void newService(enet::Tcp _connection);
			void newClient(enet::Tcp _connection);
			void cleanIO();
		private:
			void onPropertyChangeClientIp();
			void onPropertyChangeClientPort();
			void onPropertyChangeClientMax();
			void onPropertyChangeServiceIp();
			void onPropertyChangeServicePort();
			void onPropertyChangeServiceMax();
			void onClientConnect(const bool& _value);
			void onServiceConnect(const bool& _value);
	};
}

