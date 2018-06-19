/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/DirectInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const etk::String protocolError = "PROTOCOL-ERROR";

appl::DirectInterface::DirectInterface(enet::Tcp _connection, bool _serviceAccess) :
  m_interfaceWeb(etk::move(_connection), true),
  m_serviceAccess(_serviceAccess) {
	m_uid = 0;
	m_state = appl::clientState::unconnect;
	APPL_INFO("------------------------");
	if (m_serviceAccess == true) {
		APPL_INFO("-- NEW SERVICE Client --");
	} else {
		APPL_INFO("-- NEW Direct Client  --");
	}
	APPL_INFO("------------------------");
}

appl::DirectInterface::~DirectInterface() {
	APPL_INFO("---------------------------");
	if (m_serviceAccess == true) {
		APPL_INFO("-- DELETE SERVICE Client --");
	} else {
		APPL_INFO("-- DELETE Direct  Client --");
	}
	APPL_INFO("---------------------------");
	m_interfaceWeb.disconnect();
	APPL_INFO("---------------------------");
}
/*
void appl::clientSpecificInterface::answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp) {
	m_interfaceWeb->answerError(_transactionId, m_routeurUID, ZEUS_ID_SERVICE_ROOT, protocolError, _errorHelp);
	m_interfaceWeb->sendCtrl(m_routeurUID, ZEUS_ID_SERVICE_ROOT, "DISCONNECT");
	m_state = appl::clientState::disconnect;
}
*/
etk::String appl::DirectInterface::requestURI(const etk::String& _uri, const etk::Map<etk::String,etk::String>& _options) {
	APPL_WARNING("request Direct connection: '" << _uri << "'");
	etk::String tmpURI = _uri;
	if (tmpURI.size() == 0) {
		APPL_ERROR("Empty URI ... not supported ...");
		return "CLOSE";
	}
	if (tmpURI[0] == '/') {
		tmpURI = etk::String(tmpURI.begin() + 1, tmpURI.end());
	}
	etk::Vector<etk::String> listValue = etk::split(tmpURI, '?');
	if (listValue.size() == 0) {
		APPL_ERROR("can not parse URI ...");
		return "CLOSE";
	}
	tmpURI = listValue[0];
	if (tmpURI == m_gateway->propertyUserName.get() ) {
		return "OK";
	}
	if (etk::start_with(tmpURI, "directIO") == false ) {
		APPL_ERROR("Missing 'directIO:' at the start of the URI ...");
		return "CLOSE";
	}
	return "OK";
}

bool appl::DirectInterface::start(appl::GateWay* _gateway) {
	appl::IOInterface::start(_gateway, 0, m_serviceAccess==false);
	m_interfaceWeb.connect(this, &appl::DirectInterface::receive);
	m_interfaceWeb.connectUri(this, &appl::DirectInterface::requestURI);
	m_interfaceWeb.connect();
	m_interfaceWeb.setInterfaceName("DIO-?");
	//APPL_WARNING("[" << m_uid << "] New client : " << m_clientName);
	return true;
}

void appl::DirectInterface::receive(ememory::SharedPtr<zeus::Message> _value) {
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
		answerProtocolError(transactionId, "message with the wrong source ID");
		return;
	}
	// Check gateway corectly connected
	if (m_gateway == null) {
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
		m_uid = m_gateway->getId();
		APPL_INFO("clent call getAddress ==> return addresse " << m_uid);
		if (m_uid == 0) {
			answerProtocolError(transactionId, "Can not get the Client ID...");
			return;
		}
		m_interfaceWeb.setInterfaceName("cli-" + etk::toString(m_uid));
		m_interfaceWeb.answerValue(transactionId, _value->getDestination(), _value->getSource(), m_uid);
	} else {
		appl::IOInterface::receive(_value);
	}
}

void appl::DirectInterface::send(ememory::SharedPtr<zeus::Message> _value) {
	m_interfaceWeb.writeBinary(_value);
}

