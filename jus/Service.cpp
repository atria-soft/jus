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
	m_interfaceClient = std::make_shared<jus::TcpString>();
	m_interfaceClient->connect(this, &jus::Service::onClientData);
	
	advertise("getExtention", &jus::Service::getExtention);
	setLastFuncDesc("Get List of availlable extention of this service");
	addLastFuncReturn("A list of extention register in the service");
	
}

jus::Service::~Service() {
	
}


std::vector<std::string> jus::Service::getExtention() {
	return std::vector<std::string>();
}


void jus::Service::onClientData(std::string _value) {
	JUS_INFO("Request: " << _value);
	ejson::Object request(_value);
	uint64_t tmpID = request["id"].toNumber().getU64();
	uint64_t clientId = request["client-id"].toNumber().getU64();
	auto it = m_callMultiData.begin();
	while (it != m_callMultiData.end()) {
		if (    it->getTransactionId() == tmpID
		     && it->getClientId() == clientId) {
			JUS_WARNING("Append data ... " << tmpID);
			it->appendData(request);
			if (it->isFinished() == true) {
				JUS_WARNING("CALL Function ...");
				callJson(tmpID, it->getRaw());
				it = m_callMultiData.erase(it);
			}
			return;
		}
		++it;
	}
	jus::FutureCall futCall(clientId, tmpID, request);
	if (futCall.isFinished() == true) {
		callJson(tmpID, futCall.getRaw());
	} else {
		m_callMultiData.push_back(futCall);
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
	m_interfaceClient->setInterface(std::move(connection));
	m_interfaceClient->connect();
	m_interfaceClient->write(std::string("{\"connect-service\":\"") + _serviceName + "\"}");
	JUS_DEBUG("connect [STOP]");
}

void jus::Service::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient->disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

bool jus::Service::GateWayAlive() {
	return m_interfaceClient->isActive();
}

void jus::Service::pingIsAlive() {
	if (std::chrono::steady_clock::now() - m_interfaceClient->getLastTimeSend() >= std::chrono::seconds(30)) {
		m_interfaceClient->write("{\"event\":\"IS-ALIVE\"}");
	}
}

void jus::Service::callJson(uint64_t _transactionId, const ejson::Object& _obj) {
	if (_obj.valueExist("event") == true) {
		std::string event = _obj["event"].toString().get();
		if (event == "IS-ALIVE") {
			// Gateway just aswer a keep alive information ...
			// Nothing to do ...
		} else {
			JUS_ERROR("Unknow event: '" << event << "'");
		}
		return;
	}
	ejson::Object answer;
	uint64_t clientId = _obj["client-id"].toNumber().getU64();
	if (_obj.valueExist("call") == true) {
		std::string call = _obj["call"].toString().get();
		ejson::Array params = _obj["param"].toArray()
		if (call[0] == '_') {
			if (call == "_new") {
				std::string userName = params[0].toString().get();
				std::string clientName = params[1].toString().get();
				std::vector<std::string> clientGroup = convertJsonTo<std::vector<std::string>>(params[2]);
				clientConnect(clientId, userName, clientName, clientGroup);
			} else if (call == "_delete") {
				clientDisconnect(clientId);
			}
		} else if (isFunctionAuthorized(clientId, call) == true) {
			callJson2(_transactionId, clientId, call, params);
			return;
		} else {
			answer.add("id", ejson::Number(_transactionId));
			answer.add("client-id", ejson::Number(clientId));
			answer.add("error", ejson::String("NOT-AUTHORIZED-FUNCTION"));
			JUS_INFO("Answer: " << answer.generateHumanString());
			m_interfaceClient->write(answer.generateMachineString());
			return;
		}
	}
	answer.add("id", ejson::Number(_transactionId));
	answer.add("client-id", ejson::Number(clientId));
	answer.add("error", ejson::String("NOT-IMPLEMENTED-FUNCTION"));
	JUS_INFO("Answer: " << answer.generateHumanString());
	m_interfaceClient->write(answer.generateMachineString());
}
