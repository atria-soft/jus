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
  m_interfaceClient(std::move(_connection)),
  m_transactionLocalId(-1) {
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
	m_interfaceClient.connect(this, &jus::GateWayClient::onClientData);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("cli-" + etk::to_string(m_uid));
}

void jus::GateWayClient::stop() {
	for (auto &it : m_listConnectedService) {
		if (it == nullptr) {
			continue;
		}
		ejson::Object linkService;
		linkService.add("event", ejson::String("delete"));
		it->SendData(m_uid, linkService);
	}
	if (m_userService != nullptr) {
		ejson::Object linkService;
		linkService.add("event", ejson::String("delete"));
		m_userService->SendData(-m_uid, linkService);
		m_userService = nullptr;
	}
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool jus::GateWayClient::isAlive() {
	return m_interfaceClient.isActive();
}

void jus::GateWayClient::onClientData(std::string _value) {
	JUS_DEBUG("On data: " << _value);
	ejson::Object data(_value);
	int32_t transactionId = data["id"].toNumber().get();
	if (m_userConnectionName == "") {
		if (data.valueExist("connect-to-user") == true) {
			m_userConnectionName = data["connect-to-user"].toString().get();
			JUS_WARNING("[" << m_uid << "] Set client connect to user : '" << m_userConnectionName << "'");
			
			m_userService = m_gatewayInterface->get("system-user");
			ejson::Object linkService;
			linkService.add("event", ejson::String("new"));
			linkService.add("user", ejson::String(m_userConnectionName));
			m_userService->SendData(-m_uid, linkService);
			
			// TODO : Return something ...
			return;
		}
		JUS_WARNING("[" << m_uid << "] Client must send conection to user name ...");
		// TODO : Return something ...
		return;
	}
	std::string service = data["service"].toString().get();
	// Thsi is 2 default service for the cient interface that manage the authorisation of view:
	if (service == "") {
		std::string call = data["call"].toString().get();
		ejson::Object answer;
		//answer.add("from-service", ejson::String(""));
		answer.add("id", data["id"]);
		if (call == "getServiceCount") {
			// TODO : Do it better:
			answer.add("return", ejson::Number(2));
			JUS_DEBUG("answer: " << answer.generateHumanString());
			m_interfaceClient.write(answer.generateMachineString());
			return;
		}
		if (call == "getServiceList") {
			ejson::Array listService;
			listService.add(ejson::String("ServiceManager/v0.1.0"));
			listService.add(ejson::String("getServiceInformation/v0.1.0"));
			answer.add("return", listService);
			JUS_DEBUG("answer: " << answer.generateHumanString());
			m_interfaceClient.write(answer.generateMachineString());
			return;
		}
		if (call == "identify") {
			// Identify Client has an extern user ...
			std::string clientName = data["param"].toArray()[0].toString().get();
			std::string clientTocken = data["param"].toArray()[1].toString().get();
			ejson::Object gwCall;
			int32_t tmpID = m_transactionLocalId--;
			gwCall.add("id", ejson::Number(tmpID));
			gwCall.add("call", ejson::String("checkTocken"));
			ejson::Array gwParam;
			gwParam.add(ejson::String(clientName));
			gwParam.add(ejson::String(clientTocken));
			gwCall.add("param", gwParam);
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_actions.push_back(std::make_pair(tmpID,
				    [=](ejson::Object& _data) {
				    	ejson::Object tmpAnswer;
				    	tmpAnswer.add("id", ejson::Number(transactionId));
				    	JUS_ERROR("    ==> Tocken ckeck return ...");
				    	if (_data["return"].toBoolean().get() == true) {
				    		m_clientName = clientName;
				    		m_clientgroups.clear();
				    		// TODO : Update all service name and group ...
				    		tmpAnswer.add("return", ejson::Boolean(true));
				    	} else {
				    		tmpAnswer.add("return", ejson::Boolean(false));
				    	}
				    	JUS_DEBUG("answer: " << tmpAnswer.generateHumanString());
				    	m_interfaceClient.write(tmpAnswer.generateMachineString());
				    }));
			}
			if (m_userService != nullptr) {
				m_userService->SendData(-m_uid, gwCall);
			} else {
				// TODO ...
			}
			return;
		}
		if (call == "authentify") {
			// Identify Client has an local user ... (connection to is the same ...)
			std::string password = data["param"].toArray()[0].toString().get();
			// TODO: ...
			return;
		}
		if (call == "link") {
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
					linkService.add("event", ejson::String("new"));
					linkService.add("user", ejson::String(m_userConnectionName));
					srv->SendData(m_uid, linkService);
					m_listConnectedService.push_back(srv);
					answer.add("return", ejson::Boolean(true));
				} else {
					answer.add("return", ejson::Boolean(false));
				}
			} else {
				// TODO : Service already connected;
				answer.add("return", ejson::Boolean(false));
			}
			JUS_DEBUG("answer: " << answer.generateHumanString());
			m_interfaceClient.write(answer.generateMachineString());
			return;
		}
		if (call == "unlink") {
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
				ejson::Object linkService;
				linkService.add("event", ejson::String("delete"));
				(*it)->SendData(m_uid, linkService);
				m_listConnectedService.erase(it);
				answer.add("return", ejson::Boolean(true));
			}
			JUS_DEBUG("answer: " << answer.generateHumanString());
			m_interfaceClient.write(answer.generateMachineString());
			return;
		}
		JUS_ERROR("Function does not exist ... '" << call << "'");
		answer.add("error", ejson::String("CALL-UNEXISTING"));
		JUS_DEBUG("answer: " << answer.generateHumanString());
		m_interfaceClient.write(answer.generateMachineString());
		return;
	}
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
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_actions.push_back(std::make_pair(transactionId,
			    [=](ejson::Object& _data) {
			    	JUS_DEBUG("    ==> transmit");
			    	m_interfaceClient.write(_data.generateMachineString());
			    }));
		}
		(*it)->SendData(m_uid, data);
	}
}

void jus::GateWayClient::returnMessage(ejson::Object _data) {
	JUS_DEBUG("answer: " << _data.generateHumanString());
	int32_t id = _data["id"].toNumber().get();
	if (id == 0) {
		JUS_ERROR("gateway reject transaction ... ==> No 'id' or 'id' == 0");
		return;
	}
	jus::GateWayClient::Observer obs;
	ejson::Object localData;
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_actions.begin();
		while (it != m_actions.end()) {
			if (it->first != id) {
				++it;
				continue;
			}
			obs = (*it).second;
			m_actions.erase(it);
			break;
		}
		if (obs == nullptr) {
			JUS_ERROR("gateway reject transaction ... (not find answer)" << _data.generateHumanString());
			return;
		}
	}
	obs(_data);
	if (id >= 0) {
		m_interfaceClient.write(_data.generateMachineString());
	} else {
		JUS_WARNING("Action to do ...");
	}
}