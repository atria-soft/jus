/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Service.h>
#include <jus/debug.h>
#include <etk/stdTools.h>
#include <enet/TcpClient.h>
#include <ejson/ejson.h>

#include <unistd.h>



jus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1982, "Port to connect server", &jus::Service::onPropertyChangePort),
  m_id(0) {
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::Service::onClientData);
}

jus::Service::~Service() {
	
}

void jus::Service::onClientData(const std::string& _value) {
	ejson::Object request(_value);
	JUS_INFO("Request: " << _value);
	ejson::Object answer = callJson(request);
	std::string answerString = answer.generate();
	JUS_INFO("Answer: " << answerString);
	m_interfaceClient.write(answerString);
}

void jus::Service::onPropertyChangeIp() {
	disconnect();
}

void jus::Service::onPropertyChangePort(){
	disconnect();
}


void jus::Service::connect(const std::string& _serviceName){
	disconnect();
	JUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient.setInterface(std::move(connection));
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
		tmpp.add("client-id", ejson::String(etk::to_string(clientId)));
		tmpp.add("return", ejson::String("OK"));
		return tmpp;
	} else if (action == "delete") {
		uint64_t clientId = etk::string_to_uint64_t(_obj["client-id"].toString().get());
		clientDisconnect(clientId);
		ejson::Object tmpp;
		tmpp.add("client-id", ejson::String(etk::to_string(clientId)));
		tmpp.add("return", ejson::String("OK"));
		return tmpp;
	} else if (    action == "call"
	            || action == "") {
		uint64_t clientId = etk::string_to_uint64_t(_obj["client-id"].toString().get());
		ejson::Object tmpp = callJson2(clientId, _obj);
		tmpp.add("client-id", ejson::String(etk::to_string(clientId)));
		return tmpp;
	} else {
		ejson::Object tmpp;
		tmpp.add("error", ejson::String("NOT-IMPLEMENTED-ACTION"));
		return tmpp;
	}
	return ejson::Object();
}
