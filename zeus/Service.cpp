/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/Service.h>
#include <zeus/debug.h>
#include <etk/stdTools.h>
#include <enet/TcpClient.h>

#include <unistd.h>

zeus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &zeus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1982, "Port to connect server", &zeus::Service::onPropertyChangePort) {
	
	
	advertise("getExtention", &zeus::Service::getExtention);
	setLastFuncDesc("Get List of availlable extention of this service");
	addLastFuncReturn("A list of extention register in the service");
	
}

zeus::Service::~Service() {
	
}


std::vector<std::string> zeus::Service::getExtention() {
	return std::vector<std::string>();
}


void zeus::Service::onClientData(const ememory::SharedPtr<zeus::Buffer>& _value) {
	uint32_t tmpID = _value->getTransactionId();
	uint32_t clientId = _value->getClientId();;
	auto it = m_callMultiData.begin();
	while (it != m_callMultiData.end()) {
		if (    it->getTransactionId() == tmpID
		     && it->getClientId() == clientId) {
			ZEUS_WARNING("Append data ... " << tmpID);
			it->appendData(_value);
			if (it->isFinished() == true) {
				ZEUS_WARNING("CALL Function ...");
				callBinary(tmpID, it->getRaw());
				it = m_callMultiData.erase(it);
			}
			return;
		}
		++it;
	}
	zeus::FutureCall futCall(clientId, tmpID, _value);
	if (futCall.isFinished() == true) {
		ZEUS_INFO("Call Binary ..");
		callBinary(tmpID, futCall.getRaw());
	} else {
		m_callMultiData.push_back(futCall);
	}
}

void zeus::Service::onPropertyChangeIp() {
	disconnect();
}

void zeus::Service::onPropertyChangePort(){
	disconnect();
}


void zeus::Service::connect(const std::string& _serviceName, uint32_t _numberRetry){
	disconnect();
	ZEUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort, _numberRetry));
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		ZEUS_DEBUG("connect [STOP] ==> can not connect");
		return;
	}
	m_interfaceClient = std::make_shared<zeus::TcpString>();
	if (m_interfaceClient == nullptr) {
		ZEUS_ERROR("Can not allocate interface ...");
		return;
	}
	m_interfaceClient->connect(this, &zeus::Service::onClientData);
	m_interfaceClient->setInterface(std::move(connection), false);
	m_interfaceClient->connect();
	zeus::Future<bool> ret = m_interfaceClient->call("connect-service", _serviceName);
	ret.wait();
	if (ret.get() == false) {
		ZEUS_ERROR("Can not configure the interface for the service with the current name ...");
		m_interfaceClient->disconnect();
		return;
	}
	
	ZEUS_DEBUG("connect [STOP]");
}

void zeus::Service::disconnect(){
	ZEUS_DEBUG("disconnect [START]");
	if (m_interfaceClient != nullptr) {
		m_interfaceClient->disconnect();
		m_interfaceClient.reset();
	} else {
		ZEUS_VERBOSE("Nothing to disconnect ...");
	}
	ZEUS_DEBUG("disconnect [STOP]");
}

bool zeus::Service::GateWayAlive() {
	if (m_interfaceClient == nullptr) {
		return false;
	}
	return m_interfaceClient->isActive();
}

void zeus::Service::pingIsAlive() {
	if (std::chrono::steady_clock::now() - m_interfaceClient->getLastTimeSend() >= std::chrono::seconds(30)) {
		m_interfaceClient->ping();
	}
}

void zeus::Service::callBinary(uint32_t _transactionId, const ememory::SharedPtr<zeus::Buffer>& _obj) {
	if (_obj == nullptr) {
		return;
	}
	if (_obj->getType() == zeus::Buffer::typeMessage::event) {
		ZEUS_ERROR("Unknow event: '...'");
		return;
	}
	if (_obj->getType() == zeus::Buffer::typeMessage::answer) {
		ZEUS_ERROR("Local Answer: '...'");
		return;
	}
	//if (_obj.getType() == zeus::Buffer::typeMessage::event) {
	uint32_t clientId = _obj->getClientId();
	std::string callFunction = _obj->getCall();
	if (callFunction[0] == '_') {
		if (callFunction == "_new") {
			std::string userName = _obj->getParameter<std::string>(0);
			std::string clientName = _obj->getParameter<std::string>(1);
			std::vector<std::string> clientGroup = _obj->getParameter<std::vector<std::string>>(2);
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
}


