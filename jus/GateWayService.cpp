/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>
#include <jus/GateWayService.h>
#include <ejson/ejson.h>

jus::GateWayService::GateWayService(jus::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface) {
	
}

jus::GateWayService::~GateWayService() {
	
}

void jus::GateWayService::start(const std::string& _ip, uint16_t _port) {
	m_interfaceClient.propertyIp.set(_ip);
	m_interfaceClient.propertyPort.set(_port);
	m_interfaceClient.propertyServer.set(true);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("srv-?");
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::GateWayService::onClientData);
}

void jus::GateWayService::stop() {
	m_interfaceClient.disconnect();
}

void jus::GateWayService::SendData(size_t _userSessionId, ejson::Object _data) {
	_data.add("client-id", ejson::String(etk::to_string(_userSessionId)));
	_data.add("action", ejson::String("call"));
	std::string value = _data.generate();
	JUS_DEBUG("Send Service: " << value);
	m_interfaceClient.write(value);
}

void jus::GateWayService::onClientData(const std::string& _value) {
	JUS_DEBUG("On service data: " << _value);
	ejson::Object data(_value);
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
		JUS_WARNING("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	JUS_TODO("plop...........");
}

