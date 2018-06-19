/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/ClientInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/Router.hpp>


#include <zeus/AbstractFunction.hpp>


static const etk::String protocolError = "PROTOCOL-ERROR";

appl::ClientInterface::ClientInterface(enet::Tcp _connection, appl::Router* _routerInterface) :
  m_routerInterface(_routerInterface),
  m_interfaceClient(etk::move(_connection), true),
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

etk::String appl::ClientInterface::requestURI(const etk::String& _uri, const etk::Map<etk::String,etk::String>& _options) {
	APPL_INFO("request connect on CLIENT: '" << _uri << "' from " << m_interfaceClient.getRemoteAddress());
	if(m_routerInterface == null) {
		APPL_ERROR("Can not access to the main GateWay interface (null)");
		return "CLOSE";
	}
	etk::String tmpURI = _uri;
	if (tmpURI.size() == 0) {
		APPL_ERROR("Empty URI ... not supported ...");
		return "CLOSE";
	}
	if (tmpURI[0] == '/') {
		tmpURI = etk::String(tmpURI.begin() + 1, tmpURI.end());
	}
	m_userGateWay = m_routerInterface->get(tmpURI);
	APPL_INFO("Connect on client done : '" << tmpURI << "'");
	if (m_userGateWay == null) {
		APPL_ERROR("Can not connect on Client ==> it does not exist ...");
		return "CLOSE";
	}
	uint16_t externalPort = m_userGateWay->getOpenExternalPort();
	if (externalPort != 0) {
		if (m_interfaceClient.getRemoteAddress().startWith("127.0.0.1:") == true) {
			// find a local port ==> can redirect stream.
			APPL_WARNING("Request redirect of the connection, because it is possible");
			// remove reference on the client befor it was inform of our connection
			m_userGateWay->rmClient(sharedFromThis());
			m_userGateWay = null;
			m_interfaceRedirect = true;
			return etk::String("REDIRECT:") + m_routerInterface->propertyClientIp.get() + ":" + etk::toString(externalPort);
		}
	}
	return "OK";
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
	if (m_userGateWay != null) {
		m_userGateWay->rmClient(sharedFromThis());
	}
}

bool appl::ClientInterface::isAlive() {
	APPL_ERROR("check if alive");
	// kill interface in case of redirection
	if (m_interfaceRedirect == true) {
		APPL_ERROR(" ===> plop");
		return false;
	}
	//APPL_INFO("is alive : " << m_interfaceClient.isActive());
	bool ret = m_interfaceClient.isActive();
	if (ret == true) {
		if (m_userGateWay == null) {
			return false;
		}
		m_userGateWay->clientAlivePing();
		if (m_userGateWay->isAlive() == false) {
			return false;
		}
	}
	return ret;
}

void appl::ClientInterface::answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, 0, 0, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}


void appl::ClientInterface::onClientData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == null) {
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
		answerProtocolError(transactionId, "message with the wrong source ID: " + etk::toString(_value->getSourceId()) + " != " + etk::toString(m_uid));
		return;
	}
	// Check gateway corectly connected
	if (m_userGateWay == null) {
		answerProtocolError(transactionId, "GateWay error (not connected)");
		return;
	}
	// TODO: Special hook for the first call that we need to get the current ID of the connection, think to set this at an other position ...
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
		m_interfaceClient.setInterfaceName("cli-" + etk::toString(m_uid));
		m_interfaceClient.answerValue(transactionId, _value->getDestination(), _value->getSource(), m_uid);
	} else {
		// send data to the gateway
		m_userGateWay->send(_value);
	}
}

void appl::ClientInterface::send(ememory::SharedPtr<zeus::Message> _data) {
	if (_data == null) {
		return;
	}
	m_interfaceClient.writeBinary(_data);
}

