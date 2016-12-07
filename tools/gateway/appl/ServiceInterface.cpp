/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/ServiceInterface.hpp>
#include <appl/GateWay.hpp>
#include <etk/stdTools.hpp>

// todo : cHANGE THIS ...
static const std::string protocolError = "PROTOCOL-ERROR";


bool appl::ServiceInterface::requestURI(const std::string& _uri) {
	APPL_WARNING("request connect on SERVICE: '" << _uri << "'");
	if(m_gatewayInterface == nullptr) {
		APPL_ERROR("Can not access to the main GateWay interface (nullptr)");
		return false;
	}
	std::string tmpURI = _uri;
	if (tmpURI.size() == 0) {
		APPL_ERROR("Empty URI ... not supported ...");
		return false;
	}
	if (tmpURI[0] == '/') {
		tmpURI = std::string(tmpURI.begin() + 1, tmpURI.end());
	}
	std::vector<std::string> listValue = etk::split(tmpURI, '?');
	if (listValue.size() == 0) {
		APPL_ERROR("can not parse URI ...");
		return false;
	}
	tmpURI = listValue[0];
	if (etk::start_with(tmpURI, "service:") == false ) {
		APPL_ERROR("Missing 'service:' at the start of the URI ...");
		return false;
	}
	m_name = &tmpURI[8];
	APPL_INFO("Connect service name : '" << m_name << "'");
	return true;
}


appl::ServiceInterface::ServiceInterface(enet::Tcp _connection, appl::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection), true) {
	APPL_INFO("-----------------");
	APPL_INFO("-- NEW Service --");
	APPL_INFO("-----------------");
}

appl::ServiceInterface::~ServiceInterface() {
	APPL_INFO("--------------------");
	APPL_INFO("-- DELETE Service --");
	APPL_INFO("--------------------");
}

bool appl::ServiceInterface::isAlive() {
	return m_interfaceClient.isActive();
}

void appl::ServiceInterface::start() {
	m_interfaceClient.connect(this, &appl::ServiceInterface::onServiceData);
	m_interfaceClient.connectUri(this, &appl::ServiceInterface::requestURI);
	m_interfaceClient.connect();
	m_interfaceClient.setInterfaceName("srv-?");
}

void appl::ServiceInterface::stop() {
	m_interfaceClient.disconnect();
}


void appl::ServiceInterface::SendData(uint64_t _userSessionId, ememory::SharedPtr<zeus::Message> _data) {
	_data->setClientId(_userSessionId);
	m_interfaceClient.writeBinary(_data);
}

void appl::ServiceInterface::onServiceData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	uint32_t transactionId = _value->getTransactionId();
	//data.add("from-service", ejson::String(m_name));
	if (_value->getType() == zeus::Message::type::event) {
		/*
		if (data.valueExist("event") == true) {
			// No need to have a user ID ...
			if (data["event"].toString().get() == "IS-ALIVE") {
				APPL_VERBOSE("Service Alive ...");
				if (std::chrono::steady_clock::now() - m_interfaceClient.getLastTimeSend() >= std::chrono::seconds(20)) {
					ejson::Object tmpp;
					tmpp.add("event", ejson::String("IS-ALIVE"));
					m_interfaceClient.writeJson(tmpp);
				}
			} else {
				APPL_INFO("Unknow service event: '" << data["event"].toString().get() << "'");
			}
			return;
		}
		*/
		return;
	}
	if (_value->getType() == zeus::Message::type::call) {
		ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
		std::string callFunction = callObj->getCall();
		if (callFunction == "getUserName") {
			m_interfaceClient.answerValue(transactionId, _value->getClientId(), _value->getServiceId(), *m_gatewayInterface->propertyUserName);
			return;
		}
		answerProtocolError(transactionId, "unknow function");
	}
	if (_value->getClientId() == 0) {
		APPL_ERROR("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	m_gatewayInterface->answer(_value->getClientId(), _value);
}


void appl::ServiceInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, 0, 0, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}