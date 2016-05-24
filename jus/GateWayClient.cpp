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

jus::GateWayClient::GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface) :
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection)) {
	JUS_INFO("----------------");
	JUS_INFO("-- NEW Client --");
	JUS_INFO("----------------");
}

jus::GateWayClient::~GateWayClient() {
	JUS_TODO("Call All unlink ...");
	stop();
	JUS_INFO("-------------------");
	JUS_INFO("-- DELETE Client --");
	JUS_INFO("-------------------");
}

void jus::GateWayClient::start(size_t _uid) {
	m_uid = _uid;
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::GateWayClient::onClientData);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("cli-" + etk::to_string(m_uid));
}

void jus::GateWayClient::stop() {
	for (auto &it : m_listConnectedService) {
		if (it == nullptr) {
			continue;
		}
		it->SendData(m_uid, ejson::Object(), "delete");
	}
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool jus::GateWayClient::isAlive() {
	return m_interfaceClient.isActive();
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
			
		} else if (call == "link") {
			// first param:
			std::string serviceName = data["param"].toArray()[0].toString().get();
			// Check if service already link:
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
				// TODO : check if we have authorisation to connect service
				ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(serviceName);
				if (srv != nullptr) {
					ejson::Object linkService;
					linkService.add("user", ejson::String(m_userConnectionName));
					srv->SendData(m_uid, linkService, "new");
					m_listConnectedService.push_back(srv);
					answer.add("return", ejson::Boolean(true));
				} else {
					answer.add("return", ejson::Boolean(false));
				}
			} else {
				// TODO : Service already connected;
				answer.add("return", ejson::Boolean(false));
			}
		} else if (call == "unlink") {
			// first param:
			std::string serviceName = data["param"].toArray()[0].toString().get();
			// Check if service already link:
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
				answer.add("return", ejson::Boolean(false));
			} else {
				(*it)->SendData(m_uid, ejson::Object(), "delete");
				m_listConnectedService.erase(it);
				answer.add("return", ejson::Boolean(true));
			}
		} else {
			JUS_ERROR("Function does not exist ... '" << call << "'");
			answer.add("error", ejson::String("CALL-UNEXISTING"));
		}
		JUS_DEBUG("answer: " << answer.generateHumanString());
		m_interfaceClient.write(answer.generateMachineString());
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
			// TODO : Generate an ERROR...
			ejson::Object answer;
			answer.add("from-service", ejson::String("ServiceManager"));
			answer.add("id", data["id"]);
			JUS_ERROR("Service not linked ... " << service);
			answer.add("error", ejson::String("SERVICE-NOT-LINK"));
			JUS_DEBUG("answer: " << answer.generateHumanString());
			m_interfaceClient.write(answer.generateMachineString());
		} else {
			JUS_ERROR("Add in link the name of the user in parameter ...");
			data.remove("service");
			(*it)->SendData(m_uid, data);
		}
	}
}

void jus::GateWayClient::returnMessage(ejson::Object _data) {
	JUS_DEBUG("answer: " << _data.generateHumanString());
	m_interfaceClient.write(_data.generateMachineString());
}