/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <zeus/GateWayService.h>
#include <zeus/GateWayClient.h>

namespace zeus {
	class TcpServerInput;
	class GateWay : public eproperty::Interface {
		private:
			uint64_t m_clientUID;
		private:
			std::vector<ememory::SharedPtr<zeus::GateWayService>> m_serviceList; //!< List of all service availlable with their specific connection interface
			std::vector<ememory::SharedPtr<zeus::GateWayClient>> m_clientList; //!< List of all Client interface with their own connection
			//TODO: std::vector<zeus::GateWayServer> m_ServerList; //!< List of all Server connected to this gateway
			ememory::SharedPtr<zeus::TcpServerInput> m_interfaceClientServer;
			ememory::SharedPtr<zeus::TcpServerInput> m_interfaceServiceServer;
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
			ememory::SharedPtr<zeus::GateWayService> get(const std::string& _serviceName);
			std::vector<std::string> getAllServiceName();
			void answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data);
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

