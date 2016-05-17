/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Service.h>
#include <jus/debug.h>

#include <unistd.h>

jus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1984, "Port to connect server", &jus::Service::onPropertyChangePort),
  m_id(0) {
	m_interfaceClient.propertyIp.set(*propertyIp);
	m_interfaceClient.propertyPort.set(*propertyPort);
	m_interfaceClient.propertyServer.set(false);
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::Service::onClientData);
}

jus::Service::~Service() {
	
}

void jus::Service::onClientData(const std::string& _value) {
	ejson::Object request(_value);
	JUS_INFO("Request : " << _value);
	m_interfaceClient.write("{ \"error\": \"NOT-IMPLEMENTED\"}");
}

void jus::Service::onPropertyChangeIp() {
	m_interfaceClient.propertyIp.set(*propertyIp);
}

void jus::Service::onPropertyChangePort(){
	m_interfaceClient.propertyPort.set(*propertyPort);
}


void jus::Service::connect(){
	JUS_DEBUG("connect [START]");
	m_interfaceClient.connect();
	JUS_DEBUG("connect [STOP]");
}

void jus::Service::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

ejson::Object jus::Service::callJson(const ejson::Object& _obj) {
	/*
	JUS_VERBOSE("Call JSON [START] ");
	std::string tmpVal = _obj.generate();
	JUS_DEBUG("Call JSON '" << tmpVal << "'");
	m_interfaceClient.write(_obj.generate());
	std::string ret = asyncRead();
	JUS_VERBOSE("Call JSON [STOP]");
	*/
	return ejson::Object();
}
