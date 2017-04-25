/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <appl/IOInterface.hpp>
#include <eproperty/Value.hpp>

namespace appl {
	class TcpServerInput;
	class DirectInterface;
	class RouterInterface;
	class IOInterface;
	class GateWay : public eproperty::Interface {
		uint16_t m_idIncrement;
		private:
			ememory::SharedPtr<appl::RouterInterface> m_routerClient; //!< Interface with the Gateway Front End
			ememory::SharedPtr<appl::TcpServerInput> m_interfaceNewService;
		public:
			eproperty::Value<std::string> propertyUserName;
			eproperty::Value<bool> propertyRouterNo;
			eproperty::Value<std::string> propertyRouterIp;
			eproperty::Value<uint16_t> propertyRouterPort;
			eproperty::Value<bool> propertyServiceExtern;
			eproperty::Value<std::string> propertyServiceIp;
			eproperty::Value<uint16_t> propertyServicePort;
			eproperty::Value<uint16_t> propertyServiceMax;
		public:
			std::vector<ememory::SharedPtr<appl::DirectInterface>> m_listTemporaryIO; //!< List of all service availlable with their specific connection interface
			std::vector<ememory::SharedPtr<appl::IOInterface>> m_listIO;
			void addIO(const ememory::SharedPtr<appl::IOInterface>& _io);
			void removeIO(const ememory::SharedPtr<appl::IOInterface>& _io);
			
		public:
			GateWay();
			virtual ~GateWay();
			void start();
			void stop();
			//ememory::SharedPtr<appl::ServiceInterface> get(const std::string& _serviceName);
			std::vector<std::string> getAllServiceName();
			bool send(ememory::SharedPtr<zeus::Message> _data);
			void newDirectInterface(enet::Tcp _connection);
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
		public:
			uint16_t getId();
			bool serviceExist(const std::string& _service);
			uint16_t serviceClientIdGet(const std::string& _service);
			// Check if it take a long time without activity to kill itself ...
			bool checkIsAlive(const echrono::Duration& _timeout);
	};
}

