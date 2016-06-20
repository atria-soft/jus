/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/TcpClient.h>
#include <jus/Client.h>
#include <jus/debug.h>

#include <unistd.h>

jus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &jus::Client::onPropertyChangePort) {
	
}

jus::Client::~Client() {
	
}

void jus::Client::onClientData(jus::Buffer& _value) {
	JUS_ERROR("Get Data On the Communication interface that is not understand ... : " << _value.generateHumanString());
}

jus::ServiceRemote jus::Client::getService(const std::string& _name) {
	return jus::ServiceRemote(m_interfaceClient, _name);
}

void jus::Client::onPropertyChangeIp() {
	disconnect();
}

void jus::Client::onPropertyChangePort(){
	disconnect();
}


bool jus::Client::connect(const std::string& _remoteUserToConnect){
	JUS_DEBUG("connect [START]");
	disconnect();
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient = std::make_shared<jus::TcpString>();
	if (m_interfaceClient == nullptr) {
		JUS_ERROR("Allocate connection error");
		return false;
	}
	m_interfaceClient->connect(this, &jus::Client::onClientData);
	m_interfaceClient->setInterface(std::move(connection));
	m_interfaceClient->connect();
	
	JUS_WARNING("Request connect user " << _remoteUserToConnect);
	jus::Future<bool> ret = call("connectToUser", _remoteUserToConnect, "jus-client");
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not connect to user named: '" << _remoteUserToConnect << "' ==> return error");
		return false;
	}
	if (ret.get() == true) {
		JUS_WARNING("    ==> accepted connection");
	} else {
		JUS_WARNING("    ==> Refuse connection");
	}
	return ret.get();
}

void jus::Client::disconnect() {
	JUS_DEBUG("disconnect [START]");
	if (m_interfaceClient != nullptr) {
		m_interfaceClient->disconnect();
		m_interfaceClient.reset();
	} else {
		JUS_VERBOSE("Nothing to disconnect ...");
	}
	JUS_DEBUG("disconnect [STOP]");
}
