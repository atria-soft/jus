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


void jus::Service::onClientData(jus::Buffer& _value) {
	uint32_t tmpID = _value.getTransactionId();
	uint32_t clientId = _value.getClientId();;
	auto it = m_callMultiData.begin();
	while (it != m_callMultiData.end()) {
		if (    it->getTransactionId() == tmpID
		     && it->getClientId() == clientId) {
			JUS_WARNING("Append data ... " << tmpID);
			it->appendData(_value);
			if (it->isFinished() == true) {
				JUS_WARNING("CALL Function ...");
				callBinary(tmpID, it->getRaw());
				it = m_callMultiData.erase(it);
			}
			return;
		}
		++it;
	}
	jus::FutureCall futCall(clientId, tmpID, _value);
	if (futCall.isFinished() == true) {
		callBinary(tmpID, futCall.getRaw());
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
	ejson::Object tmpp;
	tmpp.add("id", ejson::Number(1));
	tmpp.add("call", ejson::String("connect-service"));
	ejson::Array params;
	params.add(ejson::String(_serviceName));
	tmpp.add("param", params);
	m_interfaceClient->writeJson(tmpp);
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
		/*
		ejson::Object tmpp;
		tmpp.add("event", ejson::String("IS-ALIVE"));
		m_interfaceClient->writeJson(tmpp);
		*/
	}
}

void jus::Service::callBinary(uint32_t _transactionId, jus::Buffer& _obj) {
	if (_obj.getType() == jus::Buffer::typeMessage::event) {
		/*
		std::string event = _obj["event"].toString().get();
		if (event == "IS-ALIVE") {
			// Gateway just aswer a keep alive information ...
			// Nothing to do ...
		} else {
			JUS_ERROR("Unknow event: '" << event << "'");
		}
		*/
		JUS_ERROR("Unknow event: '...'");
		return;
	}
	if (_obj.getType() == jus::Buffer::typeMessage::answer) {
		JUS_ERROR("Local Answer: '...'");
		return;
	}
	//if (_obj.getType() == jus::Buffer::typeMessage::event) {
	if (m_interfaceClient->getMode() == jus::connectionMode::modeBinary) {
		
	} else if (m_interfaceClient->getMode() == jus::connectionMode::modeJson) {
		uint32_t clientId = _obj.getClientId();
		std::string callFunction = _obj.getCall();
		if (callFunction[0] == '_') {
			if (callFunction == "_new") {
				std::string userName = _obj.getParameter<std::string>(0);
				std::string clientName = _obj.getParameter<std::string>(1);
				std::vector<std::string> clientGroup = _obj.getParameter<std::vector<std::string>>(2);
				clientConnect(clientId, userName, clientName, clientGroup);
			} else if (callFunction == "_delete") {
				clientDisconnect(clientId);
			}
			m_interfaceClient->answerValue(_transactionId, true, clientId);
			return;
		} else if (isFunctionAuthorized(clientId, callFunction) == true) {
			callBinary2(_transactionId, clientId, callFunction, _obj);
			return;
		} else {
			m_interfaceClient->answerError(_transactionId, "NOT-AUTHORIZED-FUNCTION", "", clientId);
			return;
		}
	} else {
		JUS_ERROR("Not manage transfer mode ");
	}
}
