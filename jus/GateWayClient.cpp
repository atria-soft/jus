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

#include <jus/AbstractFunction.h>

jus::GateWayClient::GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface) :
  m_state(jus::GateWayClient::state::unconnect),
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

void jus::GateWayClient::start(uint64_t _uid, uint64_t _uid2) {
	m_uid = _uid;
	m_uid2 = _uid2;
	m_state = jus::GateWayClient::state::connect;
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
		m_userService->SendData(m_uid2, linkService);
		m_userService = nullptr;
	}
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool jus::GateWayClient::isAlive() {
	return m_interfaceClient.isActive();
}

void jus::GateWayClient::protocolError(const std::string& _errorHelp) {
	ejson::Object answer;
	answer.add("error", ejson::String("PROTOCOL-ERROR"));
	answer.add("error-help", ejson::String(_errorHelp));
	JUS_DEBUG("answer: " << answer.generateHumanString());
	m_interfaceClient.write(answer.generateMachineString());
	m_state = jus::GateWayClient::state::disconnect;
	m_interfaceClient.disconnect(true);
}

void jus::GateWayClient::returnBool(int32_t _transactionId, bool _value) {
	ejson::Object answer;
	answer.add("id", ejson::Number(_transactionId));
	answer.add("return", ejson::Boolean(_value));
	JUS_DEBUG("answer: " << answer.generateHumanString());
	m_interfaceClient.write(answer.generateMachineString());
}

void jus::GateWayClient::onClientData(std::string _value) {
	JUS_DEBUG("On data: " << _value);
	ejson::Object data(_value);
	int32_t transactionId = data["id"].toNumber().get();
	if (transactionId == 0) {
		JUS_ERROR("Protocol error ==>missing id");
		protocolError("missing parameter: 'id'");
		return;
	}
	switch (m_state) {
		case jus::GateWayClient::state::disconnect:
		case jus::GateWayClient::state::unconnect:
			{
				JUS_ERROR("Must never appear");
				protocolError("Gateway internal error");
				return;
			}
		case jus::GateWayClient::state::connect:
			{
				if (m_userConnectionName != "") {
					protocolError("Gateway internal error 2");
					return;
				}
				std::string call = data["call"].toString().get();
				if (call == "connectToUser") {
					m_userConnectionName = data["param"].toArray()[0].toString().get();
					if (m_userConnectionName == "") {
						protocolError("Call connectToUser with no parameter 'user'");
					} else {
						JUS_WARNING("[" << m_uid << "] Set client connect to user : '" << m_userConnectionName << "'");
						
						m_userService = m_gatewayInterface->get("system-user");
						if (m_userService == nullptr) {
							protocolError("Gateway internal error 'No user interface'");
						} else {
							ejson::Object linkService;
							linkService.add("event", ejson::String("new"));
							linkService.add("user", ejson::String(m_userConnectionName));
							m_userService->SendData(m_uid2, linkService);
							m_state = jus::GateWayClient::state::userIdentify;
							returnBool(transactionId, true);
						}
					}
					return;
				}
				JUS_WARNING("[" << m_uid << "] Client must send conection to user name ...");
				protocolError("Missing call of connectToUser");
				return;
			}
		case jus::GateWayClient::state::userIdentify:
			{
				std::string call = data["call"].toString().get();
				if (call == "identify") {
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
						    	JUS_ERROR("    ==> Tocken ckeck return ...");
						    	if (_data["return"].toBoolean().get() == true) {
						    		m_clientName = clientName;
						    		m_clientgroups.clear();
						    		ejson::Object gwCall;
						    		int32_t tmpID = m_transactionLocalId--;
						    		gwCall.add("id", ejson::Number(tmpID));
						    		gwCall.add("call", ejson::String("getGroups"));
						    		ejson::Array gwParam;
						    		gwParam.add(ejson::String(clientName));
						    		gwCall.add("param", gwParam);
						    		{
						    			std::unique_lock<std::mutex> lock(m_mutex);
						    			m_actions.push_back(std::make_pair(tmpID,
						    			    [=](ejson::Object& _data) {
						    			    	JUS_ERROR("    ==> group get return ...");
						    			    	if (_data["return"].isArray() == false) {
						    			    		returnBool(transactionId, false);
						    			    	} else {
						    			    		m_clientgroups = convertJsonTo<std::vector<std::string>>(_data["return"]);
						    			    		returnBool(transactionId, true);
						    			    	}
						    			    }));
						    		}
						    		if (m_userService != nullptr) {
						    			m_userService->SendData(m_uid2, gwCall);
						    		} else {
						    			protocolError("gateWay internal error 3");
						    		}
						    	} else {
						    		returnBool(transactionId, false);
						    	}
						    }));
					}
					if (m_userService != nullptr) {
						m_userService->SendData(m_uid2, gwCall);
					} else {
						protocolError("gateWay internal error 3");
					}
					return;
				}
				if (call == "auth") {
					std::string password = data["param"].toArray()[0].toString().get();
					protocolError("Not implemented");
					return;
				}
				if (call == "anonymous") {
					protocolError("Not implemented");
					return;
				}
				protocolError("Client must call: identify/auth/anonymous");
				return;
			}
			break;
		case jus::GateWayClient::state::clientIdentify:
			{
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
								linkService.add("client", ejson::String(m_clientName));
								linkService.add("groups", convertToJson(m_clientgroups));
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