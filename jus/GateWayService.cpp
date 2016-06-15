/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>
#include <jus/GateWayService.h>
#include <jus/GateWay.h>
#include <ejson/ejson.h>

// todo : cHANGE THIS ...
static const std::string protocolError = "PROTOCOL-ERROR";



jus::GateWayService::GateWayService(enet::Tcp _connection, jus::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection)) {
	JUS_INFO("-----------------");
	JUS_INFO("-- NEW Service --");
	JUS_INFO("-----------------");
}

jus::GateWayService::~GateWayService() {
	
	JUS_INFO("--------------------");
	JUS_INFO("-- DELETE Service --");
	JUS_INFO("--------------------");
}

bool jus::GateWayService::isAlive() {
	return m_interfaceClient.isActive();
}

void jus::GateWayService::start() {
	m_interfaceClient.connect(this, &jus::GateWayService::onServiceData);
	m_interfaceClient.connect();
	m_interfaceClient.setInterfaceName("srv-?");
}

void jus::GateWayService::stop() {
	m_interfaceClient.disconnect();
}

void jus::GateWayService::SendData(uint64_t _userSessionId, ejson::Object _data) {
	_data.add("client-id", ejson::Number(_userSessionId));
	m_interfaceClient.writeJson(_data);
}

void jus::GateWayService::SendData(uint64_t _userSessionId, jus::Buffer& _data) {
	_data.setClientId(_userSessionId);
	_data.prepare();
	if (m_interfaceClient.getMode() == jus::connectionMode::modeJson) {
		ejson::Object obj = _data.toJson();
		m_interfaceClient.writeJson(obj);
	} else if (m_interfaceClient.getMode() == jus::connectionMode::modeXml) {
		JUS_ERROR("NOT manage transcriptioon binary to XML ... ");
	} else if (m_interfaceClient.getMode() == jus::connectionMode::modeBinary) {
		m_interfaceClient.writeBinary(_data);
	} else {
		JUS_ERROR("NOT manage transcriptioon binary to ??? mode ... ");
	}
}

void jus::GateWayService::onServiceData(jus::Buffer& _value) {
	JUS_DEBUG("On service data: " << _value.toJson().generateHumanString());
	uint32_t transactionId = _value.getTransactionId();
	//data.add("from-service", ejson::String(m_name));
	if (_value.getType() == jus::Buffer::typeMessage::event) {
		/*
		if (data.valueExist("event") == true) {
			// No need to have a user ID ...
			if (data["event"].toString().get() == "IS-ALIVE") {
				JUS_VERBOSE("Service Alive ...");
				if (std::chrono::steady_clock::now() - m_interfaceClient.getLastTimeSend() >= std::chrono::seconds(20)) {
					ejson::Object tmpp;
					tmpp.add("event", ejson::String("IS-ALIVE"));
					m_interfaceClient.writeJson(tmpp);
				}
			} else {
				JUS_INFO("Unknow service event: '" << data["event"].toString().get() << "'");
			}
			return;
		}
		*/
		return;
	}
	if (_value.getType() == jus::Buffer::typeMessage::call) {
		std::string callFunction = _value.getCall();
		if (callFunction == "setMode") {
			std::string mode = _value.getParameter<std::string>(0);
			if (mode == "JSON") {
				JUS_WARNING("Change mode in: JSON");
				m_interfaceClient.answerValue(transactionId, true);
				m_interfaceClient.setMode(jus::connectionMode::modeJson);
			} else if (mode == "BIN") {
				JUS_WARNING("Change mode in: BINARY");
				m_interfaceClient.answerValue(transactionId, true);
				m_interfaceClient.setMode(jus::connectionMode::modeBinary);
			} else if (mode == "XML") {
				JUS_WARNING("Change mode in: XML");
				//m_interfaceMode = jus::connectionMode::modeXml;
				m_interfaceClient.answerValue(transactionId, false);
			} else {
				answerProtocolError(transactionId, std::string("Call setMode with unknow argument : '") /*+ etk::to_string(int32_t(mode))*/ + "' supported [JSON/XML/BIN]");
			}
			return;
		} else if (callFunction == "connect-service") {
			if (m_name != "") {
				JUS_WARNING("Service interface ==> try change the servie name after init: '" << _value.getParameter<std::string>(0));
				m_interfaceClient.answerValue(transactionId, false);
				return;
			}
			m_name = _value.getParameter<std::string>(0);
			m_interfaceClient.setInterfaceName("srv-" + m_name);
			m_interfaceClient.answerValue(transactionId, true);
			return;
		}
		answerProtocolError(transactionId, "unknow function");
	}
	if (_value.getClientId() == 0) {
		JUS_ERROR("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	m_gatewayInterface->answer(_value.getClientId(), _value);
}


void jus::GateWayService::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, protocolError, _errorHelp);
	m_interfaceClient.disconnect(true);
}