/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/Service.hpp>
#include <zeus/debug.hpp>
#include <etk/stdTools.hpp>
#include <enet/TcpClient.hpp>



zeus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &zeus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1982, "Port to connect server", &zeus::Service::onPropertyChangePort),
  propertyNameService(this, "name", "no-name", "Sevice name", &zeus::Service::onPropertyChangeServiceName) {
	zeus::AbstractFunction* func = advertise("getExtention", &zeus::Service::getExtention);
	if (func != nullptr) {
		func->setDescription("Get List of availlable extention of this service");
		func->setReturn("A list of extention register in the service");
	}
}

zeus::Service::~Service() {
	
}


std::vector<std::string> zeus::Service::getExtention() {
	return std::vector<std::string>();
}


void zeus::Service::onClientData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	ZEUS_WARNING("BUFFER" << _value);
	uint32_t tmpID = _value->getTransactionId();
	uint32_t clientId = _value->getClientId();
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		auto it = m_callMultiData.begin();
		while (it != m_callMultiData.end()) {
			if (    it->getTransactionId() == tmpID
			     && it->getClientId() == clientId) {
				ZEUS_WARNING("Append data ... " << tmpID);
				it->appendData(_value);
				if (it->isFinished() == true) {
					ZEUS_WARNING("CALL Function ...");
					callBinary(it->getRaw());
					it = m_callMultiData.erase(it);
				}
				return;
			}
			++it;
		}
		ZEUS_ERROR("Un-associated data ...");
		return;
	}
	ZEUS_WARNING("direct call");
	zeus::FutureBase futData(tmpID, _value, clientId);
	if (futData.isFinished() == true) {
		ZEUS_INFO("Call Binary ..");
		callBinary(futData.getRaw());
	} else {
		ZEUS_INFO("ADD ...");
		m_callMultiData.push_back(futData);
	}
}

void zeus::Service::onPropertyChangeServiceName() {
	disconnect();
}
void zeus::Service::onPropertyChangeIp() {
	disconnect();
}

void zeus::Service::onPropertyChangePort(){
	disconnect();
}


bool zeus::Service::connect(uint32_t _numberRetry){
	disconnect();
	ZEUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort, _numberRetry));
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		ZEUS_DEBUG("connect [STOP] ==> can not connect");
		return false;
	}
	m_interfaceClient = ememory::makeShared<zeus::WebServer>();
	if (m_interfaceClient == nullptr) {
		ZEUS_ERROR("Can not allocate interface ...");
		return false;
	}
	m_interfaceClient->connect(this, &zeus::Service::onClientData);
	m_interfaceClient->setInterface(std::move(connection), false, std::string("service:") + propertyNameService.get());
	m_interfaceClient->connect();
	if (m_interfaceClient->isActive() == false) {
		ZEUS_ERROR("Can not connect service ...");
		return false;
	}
	
	zeus::Future<std::string> ret = m_interfaceClient->call(ZEUS_NO_ID_CLIENT, ZEUS_ID_SERVICE_ROOT, "getUserName");
	ret.wait();
	m_nameUser = ret.get();
	ZEUS_ERROR("Connect with name user: '" << m_nameUser << "'");
	
	ZEUS_DEBUG("connect [STOP]");
	return true;
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

void zeus::Service::callBinary(ememory::SharedPtr<zeus::Buffer> _obj) {
	ZEUS_INFO("plop 1 ...");
	if (_obj == nullptr) {
		return;
	}
	ZEUS_INFO("plop 2 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::event) {
		ZEUS_ERROR("Unknow event: '...'");
		return;
	}
	ZEUS_INFO("plop 3 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::answer) {
		ZEUS_ERROR("Local Answer: '...'");
		return;
	}
	ZEUS_INFO("plop 4 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::call) {
		ZEUS_INFO("plop 5 ... ");
		ememory::SharedPtr<zeus::BufferCall> callObj = ememory::staticPointerCast<zeus::BufferCall>(_obj);
		uint32_t clientId = callObj->getClientId();
		std::string callFunction = callObj->getCall();
		ZEUS_INFO("plop - ... " << callFunction);
		if (callFunction[0] == '_') {
			if (callFunction == "_new") {
				std::string userName = callObj->getParameter<std::string>(0);
				std::string clientName = callObj->getParameter<std::string>(1);
				std::vector<std::string> clientGroup = callObj->getParameter<std::vector<std::string>>(2);
				clientConnect(clientId, userName, clientName, clientGroup);
			} else if (callFunction == "_delete") {
				clientDisconnect(clientId);
			}
			m_interfaceClient->answerValue(callObj->getTransactionId(), clientId, m_id, true);
			return;
		} else if (isFunctionAuthorized(clientId, callFunction) == true) {
			ZEUS_INFO("plop 6 ...");
			callBinary2(callFunction, callObj);
			return;
		} else {
			ZEUS_INFO("plop 7 ...");
			m_interfaceClient->answerError(callObj->getTransactionId(), clientId, m_id, "NOT-AUTHORIZED-FUNCTION", "");
			return;
		}
	}
}


