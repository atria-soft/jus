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
  propertyPort(this, "port", 1982, "Port to connect server", &jus::Service::onPropertyChangePort) {
	m_interfaceClient.connect(this, &jus::Service::onClientData);
}

jus::Service::~Service() {
	
}

void jus::Service::onClientData(std::string _value) {
	ejson::Object request(_value);
	ejson::Value tmpID = request["id"];
	request.remove("id");
	JUS_INFO("Request: " << _value);
	ejson::Value answer = callJson(request);
	// check if an answer is needed
	if (answer.isNull() == false) {
		answer.toObject().add("id", tmpID);
		JUS_INFO("Answer: " << answer.generateHumanString());
		m_interfaceClient.write(answer.generateMachineString());
	}
}

void jus::Service::onPropertyChangeIp() {
	disconnect();
}

void jus::Service::onPropertyChangePort(){
	disconnect();
}


void jus::Service::connect(const std::string& _serviceName, uint32_t _numberRetry){
	disconnect();
	JUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort, _numberRetry));
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		JUS_DEBUG("connect [STOP] ==> can not connect");
		return;
	}
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

bool jus::Service::GateWayAlive() {
	return m_interfaceClient.isActive();
}

void jus::Service::pingIsAlive() {
	if (std::chrono::steady_clock::now() - m_interfaceClient.getLastTimeSend() >= std::chrono::seconds(30)) {
		m_interfaceClient.write("{\"event\":\"IS-ALIVE\"}");
	}
}

ejson::Value jus::Service::callJson(const ejson::Object& _obj) {
	if (_obj.valueExist("event") == true) {
		std::string event = _obj["event"].toString().get();
		if (event == "IS-ALIVE") {
			// Gateway just aswer a keep alive information ...
			// Nothing to do ...
		} else if (event == "new") {
			uint64_t clientId = _obj["client-id"].toNumber().getU64();
			std::string userName = _obj["user"].toString().get();
			std::string clientName = _obj["client"].toString().get();
			std::vector<std::string> clientGroup = convertJsonTo<std::vector<std::string>>(_obj["groups"]);
			clientConnect(clientId, userName, clientName, clientGroup);
		} else if (event == "delete") {
			uint64_t clientId = _obj["client-id"].toNumber().getU64();
			clientDisconnect(clientId);
		} else {
			JUS_ERROR("Unknow event: '" << event << "'");
		}
		return ejson::Null();
	}
	ejson::Object tmpp;
	if (_obj.valueExist("call") == true) {
		uint64_t clientId = _obj["client-id"].toNumber().getU64();
		std::string call = _obj["call"].toString().get();
		if (etk::start_with(call, "srv.") == true) {
			tmpp.add("error", ejson::String("NOT-IMPLEMENTED-ACTION **"));
		} else {
			tmpp = callJson2(clientId, _obj);
		}
		tmpp.add("client-id", ejson::Number(clientId));
		return tmpp;
	}
	tmpp.add("error", ejson::String("NOT-IMPLEMENTED-ACTION"));
	return tmpp;
}
