/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/ClientInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/Router.hpp>


#include <zeus/AbstractFunction.hpp>


static const std::string protocolError = "PROTOCOL-ERROR";

appl::ClientInterface::ClientInterface(enet::Tcp _connection, appl::Router* _routerInterface) :
  m_routerInterface(_routerInterface),
  m_interfaceClient(std::move(_connection), true),
  m_uid(0) {
	APPL_INFO("----------------");
	APPL_INFO("-- NEW Client --");
	APPL_INFO("----------------");
}

appl::ClientInterface::~ClientInterface() {
	APPL_INFO("Call All unlink ...");
	stop();
	APPL_INFO("-------------------");
	APPL_INFO("-- DELETE Client --");
	APPL_INFO("-------------------");
}

bool appl::ClientInterface::requestURI(const std::string& _uri) {
	APPL_WARNING("request connect on CLIENT: '" << _uri << "'");
	if(m_routerInterface == nullptr) {
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
	// TODO : Remove subParameters xxx?YYY
	m_userGateWay = m_routerInterface->get(tmpURI);
	APPL_INFO("Connect on client done : '" << tmpURI << "'");
	if (m_userGateWay == nullptr) {
		APPL_ERROR("Can not connect on Client ==> it does not exist ...");
		return false;
	}
	return true;
}

void appl::ClientInterface::start() {
	m_interfaceClient.connect(this, &appl::ClientInterface::onClientData);
	m_interfaceClient.connectUri(this, &appl::ClientInterface::requestURI);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("cli-");
}

void appl::ClientInterface::stop() {
	if (m_interfaceClient.isActive() == true) {
		m_interfaceClient.disconnect();
	}
	if (m_userGateWay != nullptr) {
		m_userGateWay->rmClient(sharedFromThis());
	}
}

bool appl::ClientInterface::isAlive() {
	//APPL_INFO("is alive : " << m_interfaceClient.isActive());
	return m_interfaceClient.isActive();
}

void appl::ClientInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, 0, 0, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}


void appl::ClientInterface::onClientData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	// check transaction ID != 0
	uint32_t transactionId = _value->getTransactionId();
	if (transactionId == 0) {
		APPL_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	// check correct SourceID
	if (_value->getSourceId() != m_uid) {
		answerProtocolError(transactionId, "message with the wrong source ID : " + etk::to_string(_value->getSourceId()) + " != " + etk::to_string(m_uid));
		return;
	}
	// Check gateway corectly connected
	if (m_userGateWay == nullptr) {
		answerProtocolError(transactionId, "GateWay error");
		return;
	}
	// TODO: Special hook for the first call that we need to get the curretn ID of the connection, think to set this at an other position ...
	if (m_uid == 0) {
		APPL_INFO("special case, we need to get the ID Of the client:");
		if (_value->getType() != zeus::message::type::call) {
			answerProtocolError(transactionId, "Must get first the Client ID... call 'getAddress'");
			return;
		}
		ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
		if (callObj->getCall() != "getAddress") {
			answerProtocolError(transactionId, "Must get first the Client ID... call 'getAddress' and not '" + callObj->getCall() + "'");
			return;
		}
		APPL_INFO("Get the unique ID...");
		m_uid = m_userGateWay->addClient(sharedFromThis());
		APPL_INFO("get ID : " << m_uid);
		if (m_uid == 0) {
			answerProtocolError(transactionId, "Can not get the Client ID...");
			return;
		}
		m_interfaceClient.setInterfaceName("cli-" + etk::to_string(m_uid));
		m_interfaceClient.answerValue(transactionId, _value->getDestination(), _value->getSource(), m_uid);
	} else {
		// send data to the gateway
		m_userGateWay->send(_value);
	}
}

void appl::ClientInterface::send(ememory::SharedPtr<zeus::Message> _data) {
	if (_data == nullptr) {
		return;
	}
	m_interfaceClient.writeBinary(_data);
}

