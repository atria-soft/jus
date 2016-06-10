/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>
#include <jus/GateWayService.h>
#include <jus/GateWay.h>
#include <ejson/ejson.h>

jus::GateWayService::GateWayService(enet::Tcp _connection, jus::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection)),
  m_interfaceMode(jus::connectionMode::modeJson) {
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
	JUS_DEBUG("Send Service: " << _data.generateHumanString());
	m_interfaceClient.write(_data.generateMachineString());
}
void jus::GateWayService::SendData(uint64_t _userSessionId, jus::Buffer& _data) {
	_data.setClientId(_userSessionId);
	_data.prepare();
	if (m_interfaceMode == jus::connectionMode::modeJson) {
		JUS_ERROR("NOT manage transcriptioon binary to JSON ... ");
		ejson::Object obj = _data.toJson();
		JUS_DEBUG("Send Service: " << obj.generateHumanString());
		m_interfaceClient.write(obj.generateMachineString());
	} else if (m_interfaceMode == jus::connectionMode::modeXml) {
		JUS_ERROR("NOT manage transcriptioon binary to XML ... ");
	} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
		m_interfaceClient.writeBinary(_data);
	} else {
		JUS_ERROR("NOT manage transcriptioon binary to ??? mode ... ");
	}
}

void jus::GateWayService::onServiceData(std::string _value) {
	JUS_DEBUG("On service data: " << _value);
	ejson::Object data(_value);
	data.add("from-service", ejson::String(m_name));
	if (data.valueExist("event") == true) {
		// No need to have a user ID ...
		if (data["event"].toString().get() == "IS-ALIVE") {
			JUS_VERBOSE("Service Alive ...");
			if (std::chrono::steady_clock::now() - m_interfaceClient.getLastTimeSend() >= std::chrono::seconds(20)) {
				m_interfaceClient.write("{\"event\":\"IS-ALIVE\"}");
			}
		} else {
			JUS_INFO("Unknow service event: '" << data["event"].toString().get() << "'");
		}
		return;
	}
	if (data.valueExist("connect-service") == true) {
		if (m_name != "") {
			JUS_WARNING("Service interface ==> try change the servie name after init: '" << data["connect-service"].toString().get());
			// TODO : Return something ...
			return;
		}
		m_name = data["connect-service"].toString().get();
		m_interfaceClient.setInterfaceName("srv-" + m_name);
		JUS_WARNING("Service name configured");
		// TODO : Return something ...
		return;
	}
	if (data.valueExist("client-id") == false) {
		JUS_ERROR("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	uint64_t userSessionId = data["client-id"].toNumber().getU64();
	data.remove("client-id");
	data.remove("action");
	m_gatewayInterface->answer(userSessionId, data);
}

