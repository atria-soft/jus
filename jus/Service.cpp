/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Service.h>
#include <jus/debug.h>
#include <etk/stdTools.h>
#include <ejson/ejson.h>

#include <unistd.h>



jus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1982, "Port to connect server", &jus::Service::onPropertyChangePort),
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


void jus::Service::connect(const std::string& _serviceName){
	JUS_DEBUG("connect [START]");
	m_interfaceClient.connect();
	m_interfaceClient.write(std::string("{\"connect-service\":\"") + _serviceName + "\"}");
	JUS_DEBUG("connect [STOP]");
}

void jus::Service::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

ejson::Object jus::Service::callJson(const ejson::Object& _obj) {
	std::string action = _obj["action"].toString().get();
	if (action == "new") {
		uint64_t clientId = etk::string_to_uint64_t(_obj["client-id"].toString().get());
		std::string userName = _obj["user"].toString().get();
		clientConnect(clientId, userName);
		ejson::Object tmpp;
		tmpp.add("return", ejson::String("OK"));
		return tmpp;
	} else if (action == "delete") {
		uint64_t clientId = etk::string_to_uint64_t(_obj["client-id"].toString().get());
		clientDisconnect(clientId);
		ejson::Object tmpp;
		tmpp.add("return", ejson::String("OK"));
		return tmpp;
	} else if (    action == "call"
	            || action == "") {
		uint64_t clientId = etk::string_to_uint64_t(_obj["client-id"].toString().get());
		return callJson2(clientId, _obj);
	} else {
		// TODO : ...
	}
	return ejson::Object();
}
