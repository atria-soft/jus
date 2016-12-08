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
  m_interfaceClient(std::move(_connection), true),
  m_lastSourceID(0x8000) {
	ZEUS_INFO("-----------------");
	ZEUS_INFO("-- NEW GateWay --");
	ZEUS_INFO("-----------------");
	
}

appl::GateWayInterface::~GateWayInterface() {
	
	ZEUS_INFO("---------------------");
	ZEUS_INFO("-- DELETE GateWay  --");
	ZEUS_INFO("---------------------");
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
	m_interfaceClient.setInterfaceName("GW-?");
}

void appl::GateWayInterface::stop() {
	m_interfaceClient.disconnect();
}


void appl::GateWayInterface::send(ememory::SharedPtr<zeus::Message> _data) {
	m_interfaceClient.writeBinary(_data);
}

uint16_t appl::GateWayInterface::addClient(ememory::SharedPtr<appl::ClientInterface> _value) {
	m_clientConnected.push_back(_value);
	return m_lastSourceID++;
}

void appl::GateWayInterface::rmClient(ememory::SharedPtr<appl::ClientInterface> _value) {
	if (_value == nullptr) {
		return;
	}
	uint16_t id = _value->getId();
	auto it = m_clientConnected.begin();
	while (it != m_clientConnected.end()) {
		if (*it == _value) {
			it = m_clientConnected.erase(it);
		} else {
			++it;
		}
	}
	m_interfaceClient.event(uint32_t(id)<<16, ZEUS_ID_GATEWAY, "removeRouterClient", id);
}

void appl::GateWayInterface::onServiceData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	if (m_name == "") {
		uint32_t transactionId = _value->getTransactionId();
		if (_value->getType() == zeus::message::type::call) {
			ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
			std::string callFunction = callObj->getCall();
			if (callFunction == "connect-service") {
				if (m_name != "") {
					ZEUS_WARNING("Service interface ==> try change the servie name after init: '" << callObj->getParameter<std::string>(0));
					m_interfaceClient.answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
					return;
				}
				m_name = callObj->getParameter<std::string>(0);
				m_interfaceClient.setInterfaceName("srv-" + m_name);
				m_interfaceClient.answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
				return;
			}
			answerProtocolError(transactionId, "unknow function");
		}
		return;
	}
	uint16_t destinationId = _value->getDestinationId();
	for (auto &it : m_clientConnected) {
		if (it->checkId(destinationId) == true) {
			it->send(_value);
			return;
		}
	}
	m_interfaceClient.answerError(_value->getTransactionId(), _value->getDestination(), _value->getSource(), "UNKNOW-DESTINATION", "the Id=" + etk::to_string(destinationId) + " is unknow");
}


void appl::GateWayInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, 0, 0, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}