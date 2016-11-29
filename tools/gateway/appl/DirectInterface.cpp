/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/DirectInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const std::string protocolError = "PROTOCOL-ERROR";

appl::DirectInterface::DirectInterface(enet::Tcp _connection) :
  m_interfaceWeb(std::move(_connection), true) {
	m_uid = 0;
	m_state = appl::clientState::unconnect;
	APPL_INFO("-----------------------");
	APPL_INFO("-- NEW Direct Client --");
	APPL_INFO("-----------------------");
}

appl::DirectInterface::~DirectInterface() {
	APPL_INFO("--------------------------");
	APPL_INFO("-- DELETE Direct Client --");
	APPL_INFO("--------------------------");
}
/*
void appl::clientSpecificInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceWeb->answerError(_transactionId, m_routeurUID, ZEUS_ID_SERVICE_ROOT, protocolError, _errorHelp);
	m_interfaceWeb->sendCtrl(m_routeurUID, ZEUS_ID_SERVICE_ROOT, "DISCONNECT");
	m_state = appl::clientState::disconnect;
}
*/
bool appl::DirectInterface::requestURI(const std::string& _uri) {
	APPL_WARNING("request Direct connection: '" << _uri << "'");
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
	if (etk::start_with(tmpURI, "directIO") == false ) {
		APPL_ERROR("Missing 'directIO:' at the start of the URI ...");
		return false;
	}
	return true;
}

bool appl::DirectInterface::start(appl::GateWay* _gateway, uint16_t _id) {
	appl::IOInterface::start(_gateway, _id);
	m_interfaceWeb.connect(this, &appl::DirectInterface::receive);
	m_interfaceWeb.connectUri(this, &appl::DirectInterface::requestURI);
	m_interfaceWeb.connect();
	m_interfaceWeb.setInterfaceName("DIO-?");
	//APPL_WARNING("[" << m_uid << "] New client : " << m_clientName);
	return true;
}

void appl::DirectInterface::receive(ememory::SharedPtr<zeus::Buffer> _value) {
	appl::IOInterface::receive(_value);
}

void appl::DirectInterface::send(ememory::SharedPtr<zeus::Buffer> _value) {
	m_interfaceWeb.writeBinary(_value);
}

