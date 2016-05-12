/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/GateWayClient.h>

jus::GateWayClient::GateWayClient() {
	
}

jus::GateWayClient::~GateWayClient() {
	
}

void jus::GateWayClient::start(const std::string& _ip, uint16_t _port) {
	m_interfaceClient.propertyIp.set(_ip);
	m_interfaceClient.propertyPort.set(_port);
	
}

void jus::GateWayClient::stop() {
	
}

