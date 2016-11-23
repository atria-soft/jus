/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/GateWayInterface.hpp>
#include <appl/ClientInterface.hpp>
#include <appl/Router.hpp>

// todo : cHANGE THIS ...
static const std::string protocolError = "PROTOCOL-ERROR";



appl::GateWayInterface::GateWayInterface(enet::Tcp _connection, appl::Router* _routerInterface) :
  m_routerInterface(_routerInterface),
  m_interfaceClient(std::move(_connection), true) {
	ZEUS_INFO("--------------------------");
	ZEUS_INFO("-- NEW GateWay Back-end --");
	ZEUS_INFO("--------------------------");
	
}

appl::GateWayInterface::~GateWayInterface() {
	
	ZEUS_INFO("------------------------------");
	ZEUS_INFO("-- DELETE GateWay Back-end  --");
	ZEUS_INFO("------------------------------");
}

bool appl::GateWayInterface::isAlive() {
	return m_interfaceClient.isActive();
}

bool appl::GateWayInterface::requestURI(const std::string& _uri) {
	ZEUS_INFO("request connect on User - GateWay: '" << _uri << "'");
	if(m_routerInterface == nullptr) {
		ZEUS_ERROR("Can not access to the main GateWay interface (nullptr)");
		return false;
	}
	std::string tmpURI = _uri;
	if (tmpURI.size() == 0) {
		ZEUS_ERROR("Empty URI ... not supported ...");
		return false;
	}
	if (tmpURI[0] == '/') {
		tmpURI = std::string(tmpURI.begin() + 1, tmpURI.end());
	}
	// TODO : Remove subParameters xxx?YYY
	// check if the USER is already connected:
	ememory::SharedPtr<appl::GateWayInterface> tmp = m_routerInterface->get(tmpURI);
	if (tmp != nullptr) {
		ZEUS_ERROR("User is already connected ==> this is a big error ...");
		return false;
	}
	m_name = tmpURI;
	ZEUS_WARNING("Connection of user : '" << tmpURI << "'");
	return true;
}

void appl::GateWayInterface::start() {
	m_interfaceClient.connect(this, &appl::GateWayInterface::onServiceData);
	m_interfaceClient.connectUri(this, &appl::GateWayInterface::requestURI);
	m_interfaceClient.connect();
	m_interfaceClient.setInterfaceName("srv-?");
}

void appl::GateWayInterface::stop() {
	m_interfaceClient.disconnect();
}


void appl::GateWayInterface::SendData(uint64_t _userSessionId, ememory::SharedPtr<zeus::Buffer> _data) {
	_data->setClientId(_userSessionId);
	m_interfaceClient.writeBinary(_data);
}

void appl::GateWayInterface::onServiceData(ememory::SharedPtr<zeus::Buffer> _value) {
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
	m_routerInterface->answer(_value->getClientId(), _value);
}


void appl::GateWayInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}