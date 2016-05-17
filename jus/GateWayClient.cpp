/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>
#include <jus/GateWayClient.h>
#include <ejson/ejson.h>

jus::GateWayClient::GateWayClient() {
	
}

jus::GateWayClient::~GateWayClient() {
	
}

void jus::GateWayClient::start(const std::string& _ip, uint16_t _port) {
	m_interfaceClient.propertyIp.set(_ip);
	m_interfaceClient.propertyPort.set(_port);
	m_interfaceClient.propertyServer.set(true);
	m_interfaceClient.connect();
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::GateWayClient::onClientData);
}

void jus::GateWayClient::stop() {
	m_interfaceClient.disconnect();
}

void jus::GateWayClient::onClientData(const std::string& _value) {
	JUS_DEBUG("On data: " << _value);
	ejson::Object data(_value);
	if (data.valueExist("service") == false) {
		// add default service
		data.add("service", ejson::String("ServiceManager"));
		JUS_WARNING("missing service name ==> set it by default at ServiceManager");
	}
	std::string service = data["service"].toString().get();
	// Thsi is 2 default service for the cient interface that manage the authorisation of view:
	if (service == "ServiceManager") {
		std::string call = data["call"].toString().get();
		ejson::Object answer;
		answer.add("from-service", ejson::String("ServiceManager"));
		answer.add("transaction-id", data["transaction-id"]);
		if (call == "getServiceCount") {
			// TODO : Do it better:
			answer.add("return", ejson::Number(2));
		} else if (call == "getServiceList") {
			ejson::Array listService;
			listService.add(ejson::String("ServiceManager/v0.1.0"));
			listService.add(ejson::String("getServiceInformation/v0.1.0"));
			answer.add("return", listService);
		} else if (call == "getServiceInformation") {
			
		} else {
			JUS_ERROR("Function does not exist ... '" << call << "'");
			answer.add("error", ejson::String("CALL-UNEXISTING"));
		}
		std::string valueReturn = answer.generate();
		JUS_DEBUG("answer: " << valueReturn);
		m_interfaceClient.write(valueReturn);
	} else if (service == "Authentification") {
		std::string call = data["call"].toString().get();
		
	} else {
		JUS_TODO("mmmmmmmmmmm: " << _value);
	}
}

