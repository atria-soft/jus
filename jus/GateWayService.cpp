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
  m_interfaceClient(std::move(_connection)) {
	
}

jus::GateWayService::~GateWayService() {
	
}

void jus::GateWayService::start() {
	m_interfaceClient.connect();
	m_interfaceClient.setInterfaceName("srv-?");
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::GateWayService::onServiceData);
}

void jus::GateWayService::stop() {
	m_interfaceClient.disconnect();
}

void jus::GateWayService::SendData(size_t _userSessionId, ejson::Object _data, const std::string& _action) {
	_data.add("client-id", ejson::String(etk::to_string(_userSessionId)));
	_data.add("action", ejson::String(_action));
	std::string value = _data.generate();
	JUS_DEBUG("Send Service: " << value);
	m_interfaceClient.write(value);
}

void jus::GateWayService::onServiceData(const std::string& _value) {
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
		JUS_ERROR("Service interface ==> wrong service answer ==> missing 'client-id'");
		return;
	}
	uint64_t userSessionId = etk::string_to_uint64_t(data["client-id"].toString().get());
	data.remove("client-id");
	data.remove("action");
	m_gatewayInterface->answer(userSessionId, data);
}

