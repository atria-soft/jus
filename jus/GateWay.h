/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <jus/GateWayService.h>
#include <jus/GateWayClient.h>

namespace jus {
	class GateWay : public eproperty::Interface {
		private:
			std::vector<ememory::SharedPtr<jus::GateWayService>> m_serviceList; //!< List of all service availlable with their specific connection interface
			std::vector<ememory::SharedPtr<jus::GateWayClient>> m_clientList; //!< List of all Client interface with their own connection
			//TODO: std::vector<jus::GateWayServer> m_ServerList; //!< List of all Server connected to this gateway
			ememory::SharedPtr<jus::GateWayClient> m_clientWaiting;
			esignal::Connection m_clientConnected;
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
		private:
			void onPropertyChangeClientIp();
			void onPropertyChangeClientPort();
			void onPropertyChangeClientMax();
			void onPropertyChangeServiceIp();
			void onPropertyChangeServicePort();
			void onPropertyChangeServiceMax();
			void onClientConnect(const bool& _value);
	};
}

