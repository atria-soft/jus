/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Client.h>
#include <jus/debug.h>

jus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &jus::Client::onPropertyChangePort) {
	m_interfaceClient.propertyIp.set(*propertyIp);
	m_interfaceClient.propertyPort.set(*propertyPort);
	m_interfaceClient.propertyServer.set(false);
}

jus::Client::~Client() {
	
}

void jus::Client::onPropertyChangeIp() {
	m_interfaceClient.propertyIp.set(*propertyIp);
}

void jus::Client::onPropertyChangePort(){
	m_interfaceClient.propertyPort.set(*propertyPort);
}


void jus::Client::connect(){
	JUS_DEBUG("connect [START]");
	m_interfaceClient.connect();
	JUS_DEBUG("connect [STOP]");
}

void jus::Client::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}