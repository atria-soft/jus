/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/GateWay.h>
#include <jus/debug.h>



jus::GateWay::GateWay() :
  propertyClientIp(this, "client-ip", "127.0.0.1", "Ip to listen client", &jus::GateWay::onPropertyChangeClientIp),
  propertyClientPort(this, "client-port", 1983, "Port to listen client", &jus::GateWay::onPropertyChangeClientPort),
  propertyClientMax(this, "client-max", 80, "Maximum of client at the same time", &jus::GateWay::onPropertyChangeClientMax),
  propertyServiceIp(this, "service-ip", "127.0.0.1", "Ip to listen client", &jus::GateWay::onPropertyChangeServiceIp),
  propertyServicePort(this, "service-port", 1982, "Port to listen client", &jus::GateWay::onPropertyChangeServicePort),
  propertyServiceMax(this, "service-max", 80, "Maximum of client at the same time", &jus::GateWay::onPropertyChangeServiceMax) {
	
}

jus::GateWay::~GateWay() {
	
}

void jus::GateWay::start() {
	m_clientWaiting = std::make_shared<jus::GateWayClient>();
	m_clientConnected = m_clientWaiting->signalIsConnected.connect(this, &jus::GateWay::onClientConnect);
	m_clientWaiting->start(*propertyClientIp, *propertyClientPort);
}

void jus::GateWay::stop() {
	
}

void jus::GateWay::onClientConnect(const bool& _value) {
	JUS_TODO("lklklk: " << _value);
}

void jus::GateWay::onPropertyChangeClientIp() {
	
}

void jus::GateWay::onPropertyChangeClientPort() {
	
}

void jus::GateWay::onPropertyChangeClientMax() {
	
}

void jus::GateWay::onPropertyChangeServiceIp() {
	
}

void jus::GateWay::onPropertyChangeServicePort() {
	
}

void jus::GateWay::onPropertyChangeServiceMax() {
	
}
