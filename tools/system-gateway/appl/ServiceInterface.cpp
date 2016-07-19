/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <appl/ServiceInterface.h>
#include <appl/ClientInterface.h>
#include <appl/GateWay.h>

// todo : cHANGE THIS ...
static const std::string protocolError = "PROTOCOL-ERROR";



appl::ServiceInterface::ServiceInterface(enet::Tcp _connection, appl::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection), true) {
	ZEUS_INFO("-----------------");
	ZEUS_INFO("-- NEW Service --");
	ZEUS_INFO("-----------------");
}

appl::ServiceInterface::~ServiceInterface() {
	
	ZEUS_INFO("--------------------");
	ZEUS_INFO("-- DELETE Service --");
	ZEUS_INFO("--------------------");
}

bool appl::ServiceInterface::isAlive() {
	return m_interfaceClient.isActive();
}

void appl::ServiceInterface::start() {
	m_interfaceClient.connect(this, &appl::ServiceInterface::onServiceData);
	m_interfaceClient.connect();
	m_interfaceClient.setInterfaceName("srv-?");
}

void appl::ServiceInterface::stop() {
	m_interfaceClient.disconnect();
}


void appl::ServiceInterface::SendData(uint64_t _userSessionId, ememory::SharedPtr<zeus::Buffer> _data) {
	_data->setClientId(_userSessionId);
	m_interfaceClient.writeBinary(_data);
}

void appl::ServiceInterface::onServiceData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	uint32_t transactionId = _value->getTransactionId();
	//data.add("from-service", ejson::String(m_name));
	if (_value->getType() == zeus::Buffer::typeMessage::event) {
		/*
		if (data.valueExist("event") == true) {
			// No need to have a user ID ...
			if (data["event"].toString().get() == "IS-ALIVE") {
				ZEUS_VERBOSE("Service Alive ...");
				if (std::chrono::steady_clock::now() - m_interfaceClient.getLastTimeSend() >= std::chrono::seconds(20)) {
					ejson::Object tmpp;
					tmpp.add("event", ejson::String("IS-ALIVE"));
					m_interfaceClient.writeJson(tmpp);
				}
			} else {
				ZEUS_INFO("Unknow service event: '" << data["event"].toString().get() << "'");
			}
			return;
		}
		*/
		return;
	}
	if (_value->getType() == zeus::Buffer::typeMessage::call) {
		ememory::SharedPtr<zeus::BufferCall> callObj = ememory::staticPointerCast<zeus::BufferCall>(_value);
		std::string callFunction = callObj->getCall();
		if (callFunction == "connect-service") {
			if (m_name != "") {
				ZEUS_WARNING("Service interface ==> try change the servie name after init: '" << callObj->getParameter<std::string>(0));
				m_interfaceClient.answerValue(transactionId, false);
				return;
			}
			m_name = callObj->getParameter<std::string>(0);
			m_interfaceClient.setInterfaceName("srv-" + m_name);
			m_interfaceClient.answerValue(transactionId, true);
			return;
		}
		answerProtocolError(transactionId, "unknow function");
	}
	if (_value->getClientId() == 0) {
		ZEUS_ERROR("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	m_gatewayInterface->answer(_value->getClientId(), _value);
}


void appl::ServiceInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}