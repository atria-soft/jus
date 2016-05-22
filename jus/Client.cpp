/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Client.h>
#include <jus/debug.h>

#include <unistd.h>

jus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &jus::Client::onPropertyChangePort),
  m_id(0) {
	m_interfaceClient.propertyIp.set(*propertyIp);
	m_interfaceClient.propertyPort.set(*propertyPort);
	m_interfaceClient.propertyServer.set(false);
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::Client::onClientData);
}

jus::Client::~Client() {
	
}

void jus::Client::onClientData(const std::string& _value) {
	m_newData.push_back(_value);
}

std::string jus::Client::asyncRead() {
	int32_t iii = 5000;
	while (iii>0) {
		usleep(10000);
		if (m_newData.size() != 0) {
			break;
		}
		--iii;
	}
	if (iii == 0) {
		// Time-out ...
		return "";
	}
	std::string out;
	out = std::move(m_newData[0]);
	m_newData.erase(m_newData.begin());
	JUS_DEBUG("get async data: " << out);
	return out;
}

void jus::Client::onPropertyChangeIp() {
	m_interfaceClient.propertyIp.set(*propertyIp);
}

void jus::Client::onPropertyChangePort(){
	m_interfaceClient.propertyPort.set(*propertyPort);
}


void jus::Client::connect(const std::string& _remoteUserToConnect){
	JUS_DEBUG("connect [START]");
	m_interfaceClient.connect();
	m_interfaceClient.write(std::string("{\"connect-to-user\":\"") + _remoteUserToConnect + "\", \"client-type:\":\"jus-client\"}");
	JUS_DEBUG("connect [STOP]");
}

void jus::Client::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

ejson::Object jus::Client::createBaseCall(const std::string& _service, const std::string& _functionName) {
	ejson::Object obj;
	if (_service.size() != 0) {
		obj.add("service", ejson::String(_service));
	}
	obj.add("call", ejson::String(_functionName));
	obj.add("id", ejson::Number(m_id++));
	obj.add("param", ejson::Array());
	return obj;
}

ejson::Object jus::Client::callJson(const ejson::Object& _obj) {
	JUS_VERBOSE("Call JSON [START] ");
	std::string tmpVal = _obj.generate();
	JUS_DEBUG("Call JSON '" << tmpVal << "'");
	m_interfaceClient.write(_obj.generate());
	std::string ret = asyncRead();
	JUS_VERBOSE("Call JSON [STOP]");
	return ejson::Object(ret);
}
