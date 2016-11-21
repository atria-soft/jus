/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <appl/ServiceInterface.hpp>
#include <appl/ClientGateWayInterface.hpp>
#include <eproperty/Value.hpp>

namespace appl {
	class TcpServerInput;
	class GateWay : public eproperty::Interface {
		private:
			uint64_t m_clientUID;
		private:
			std::vector<ememory::SharedPtr<appl::ServiceInterface>> m_serviceList; //!< List of all service availlable with their specific connection interface
			ememory::SharedPtr<appl::ClientGateWayInterface> m_gateWayClient; //!< Interface with the Gateway Front End
			
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceServiceServer;
		public:
			eproperty::Value<std::string> propertyUserName;
			eproperty::Value<std::string> propertyGateWayClientIp;
			eproperty::Value<uint16_t> propertyGateWayClientPort;
			eproperty::Value<std::string> propertyServiceIp;
			eproperty::Value<uint16_t> propertyServicePort;
			eproperty::Value<uint16_t> propertyServiceMax;
		public:
			GateWay();
			virtual ~GateWay();
			void start();
			void stop();
			ememory::SharedPtr<appl::ServiceInterface> get(const std::string& _serviceName);
			std::vector<std::string> getAllServiceName();
			void answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data);
			void newService(enet::Tcp _connection);
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

