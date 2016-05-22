/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/debug.h>
#include <jus/GateWayClient.h>
#include <ejson/ejson.h>
#include <jus/GateWay.h>
#include <unistd.h>

jus::GateWayClient::GateWayClient(jus::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_returnValueOk(false) {
	
}

jus::GateWayClient::~GateWayClient() {
	
}

void jus::GateWayClient::start(const std::string& _ip, uint16_t _port, size_t _uid) {
	m_interfaceClient.propertyIp.set(_ip);
	m_interfaceClient.propertyPort.set(_port);
	m_uid = _uid;
	m_interfaceClient.propertyServer.set(true);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("cli-" + etk::to_string(m_uid));
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::GateWayClient::onClientData);
}

void jus::GateWayClient::stop() {
	m_interfaceClient.disconnect();
}

void jus::GateWayClient::onClientData(const std::string& _value) {
	JUS_DEBUG("On data: " << _value);
	ejson::Object data(_value);
	if (m_userConnectionName == "") {
		if (data.valueExist("connect-to-user") == true) {
			m_userConnectionName = data["connect-to-user"].toString().get();
			JUS_WARNING("[" << m_uid << "] Set client connect to user : '" << m_userConnectionName << "'");
			// TODO : Return something ...
			return;
		}
		JUS_WARNING("[" << m_uid << "] Client must send conection to user name ...");
		// TODO : Return something ...
		return;
	}
	
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
		answer.add("id", data["id"]);
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
		auto it = m_listConnectedService.begin();
		while (it != m_listConnectedService.end()) {
			if (*it == nullptr) {
				++it;
				continue;
			}
			if ((*it)->getName() != service) {
				++it;
				continue;
			}
			break;
		}
		if (it == m_listConnectedService.end()) {
			ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(service);
			if (srv != nullptr) {
				m_listConnectedService.push_back(srv);
				it = m_listConnectedService.end()-1;
			} else {
				// TODO: Return an error ...
			}
		}
		if (it != m_listConnectedService.end()) {
			JUS_CRITICAL("Add in link the name of the user in parameter ..."
			(*it)->SendData(m_uid, data);
			while (m_returnValueOk == false) {
				JUS_DEBUG("wait Return Value");
				usleep(20000);
			}
			std::string valueReturn = m_returnMessage.generate();
			JUS_DEBUG("answer: " << valueReturn);
			m_interfaceClient.write(valueReturn);
			m_returnValueOk = false;
		}
	}
}

void jus::GateWayClient::returnMessage(ejson::Object _data) {
	m_returnMessage = _data;
	m_returnValueOk = true;
}