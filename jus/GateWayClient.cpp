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


static const std::string protocolError = "PROTOCOL-ERROR";

jus::GateWayClient::GateWayClient(enet::Tcp _connection, jus::GateWay* _gatewayInterface) :
  m_state(jus::GateWayClient::state::unconnect),
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
		it->m_interfaceClient.callClient(m_uid, "_delete");
		/*
		ejson::Object linkService;
		linkService.add("call", ejson::String("_delete"));
		linkService.add("id", ejson::Number(m_transactionLocalId++));
		linkService.add("param", ejson::Array());
		it->SendData(m_uid, linkService);
		*/
	}
	if (m_userService != nullptr) {
		m_userService->m_interfaceClient.callClient(m_uid2, "_delete");
		/*
		ejson::Object linkService;
		linkService.add("call", ejson::String("_delete"));
		linkService.add("id", ejson::Number(m_transactionLocalId++));
		linkService.add("param", ejson::Array());
		m_userService->SendData(m_uid2, linkService);
		*/
		m_userService = nullptr;
	}
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool jus::GateWayClient::isAlive() {
	return m_interfaceClient.isActive();
}

void jus::GateWayClient::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, protocolError, _errorHelp);
	m_state = jus::GateWayClient::state::disconnect;
	m_interfaceClient.disconnect(true);
}


void jus::GateWayClient::onClientData(jus::Buffer& _value) {
	uint32_t transactionId = _value.getTransactionId();
	if (transactionId == 0) {
		JUS_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	if (_value.getType() != jus::Buffer::typeMessage::call) {
		JUS_ERROR("Protocol error ==>missing 'call'");
		answerProtocolError(transactionId, "missing parameter: 'call' / wrong type 'call'");
		return;
	}
	std::string callFunction = _value.getCall();
	switch (m_state) {
		case jus::GateWayClient::state::disconnect:
		case jus::GateWayClient::state::unconnect:
			{
				JUS_ERROR("Must never appear");
				answerProtocolError(transactionId, "Gateway internal error");
				return;
			}
		case jus::GateWayClient::state::connect:
			{
				if (m_userConnectionName != "") {
					answerProtocolError(transactionId, "Gateway internal error 2");
					return;
				}
				if (callFunction == "setMode") {
					std::string mode = _value.getParameter<std::string>(0);
					if (mode == "JSON") {
						JUS_WARNING("[" << m_uid << "] Change mode in: JSON");
						m_interfaceClient.answerValue(transactionId, true);
						m_interfaceClient.setMode(jus::connectionMode::modeJson);
					} else if (mode == "BIN") {
						JUS_WARNING("[" << m_uid << "] Change mode in: BINARY");
						m_interfaceClient.answerValue(transactionId, true);
						m_interfaceClient.setMode(jus::connectionMode::modeBinary);
					} else if (mode == "XML") {
						JUS_WARNING("[" << m_uid << "] Change mode in: XML");
						//m_interfaceMode = jus::connectionMode::modeXml;
						m_interfaceClient.answerValue(transactionId, false);
					} else {
						answerProtocolError(transactionId, std::string("Call setMode with unknow argument : '") /*+ etk::to_string(int32_t(mode))*/ + "' supported [JSON/XML/BIN]");
					}
					return;
				} else if (callFunction == "connectToUser") {
					m_userConnectionName = _value.getParameter<std::string>(0);
					if (m_userConnectionName == "") {
						answerProtocolError(transactionId, "Call connectToUser with no parameter 'user'");
					} else {
						JUS_WARNING("[" << m_uid << "] Set client connect to user : '" << m_userConnectionName << "'");
						m_userService = m_gatewayInterface->get("system-user");
						if (m_userService == nullptr) {
							answerProtocolError(transactionId, "Gateway internal error 'No user interface'");
						} else {
							jus::Future<bool> futLocalService = m_userService->m_interfaceClient.callClient(m_uid2, "_new", m_userConnectionName, "**Gateway**", std::vector<std::string>());
							futLocalService.wait(); // TODO: Set timeout ...
							m_state = jus::GateWayClient::state::userIdentify;
							m_interfaceClient.answerValue(transactionId, true);
						}
					}
					return;
				}
				JUS_WARNING("[" << m_uid << "] Client must send conection to user name ...");
				answerProtocolError(transactionId, "Missing call of connectToUser");
				return;
			}
		case jus::GateWayClient::state::userIdentify:
			{
				m_clientServices.clear();
				m_clientgroups.clear();
				m_clientName.clear();
				if (    callFunction != "identify"
				     && callFunction != "auth"
				     && callFunction != "anonymous") {
					answerProtocolError(transactionId, "Client must call: identify/auth/anonymous");
					return;
				}
				if (callFunction == "identify") {
					std::string clientName = _value.getParameter<std::string>(0);
					std::string clientTocken = _value.getParameter<std::string>(1);
					if (m_userService == nullptr) {
						answerProtocolError(transactionId, "gateWay internal error 3");
						return;
					}
					
					jus::Future<bool> fut = m_userService->m_interfaceClient.callClient(m_uid2, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						m_interfaceClient.answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceClient.answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 2");
						return;
					}
					m_clientName = clientName;
				}
				if (callFunction == "auth") {
					std::string password = _value.getParameter<std::string>(0);
					jus::Future<bool> fut = m_userService->m_interfaceClient.callClient(m_uid2, "checkAuth", password);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						m_interfaceClient.answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceClient.answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 2");
						return;
					}
					m_clientName = m_userConnectionName;
				}
				if (callFunction == "anonymous") {
					m_clientName = "";
				}
				// --------------------------------
				// -- Get groups:
				// --------------------------------
				jus::Future<std::vector<std::string>> futGroup = m_userService->m_interfaceClient.callClient(m_uid2, "getGroups", m_clientName);
				futGroup.wait(); // TODO: Set timeout ...
				if (futGroup.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					m_interfaceClient.answerValue(transactionId, false);
					answerProtocolError(transactionId, "grouping error");
					return;
				}
				m_clientgroups = futGroup.get();
				// --------------------------------
				// -- Get services:
				// --------------------------------
				std::vector<std::string> currentServices = m_gatewayInterface->getAllServiceName();
				jus::Future<std::vector<std::string>> futServices = m_userService->m_interfaceClient.callClient(m_uid2, "filterServices", m_clientName, currentServices);
				futServices.wait(); // TODO: Set timeout ...
				if (futServices.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					m_interfaceClient.answerValue(transactionId, false);
					answerProtocolError(transactionId, "service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				JUS_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				JUS_WARNING("       groups: " << etk::to_string(m_clientgroups));
				JUS_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				m_interfaceClient.answerValue(transactionId, true);
				m_state = jus::GateWayClient::state::clientIdentify;
				return;
			}
			break;
		case jus::GateWayClient::state::clientIdentify:
			{
				uint32_t serviceId = _value.getServiceId();
				if (serviceId == 0) {
					// This is 2 default service for the cient interface that manage the authorisation of view:
					if (callFunction == "getServiceCount") {
						m_interfaceClient.answerValue(transactionId, m_clientServices.size());
						return;
					}
					if (callFunction == "getServiceList") {
						m_interfaceClient.answerValue(transactionId, m_clientServices);
						//listService.add(ejson::String("ServiceManager/v0.1.0"));
						return;
					}
					if (callFunction == "link") {
						// first param:
						std::string serviceName = _value.getParameter<std::string>(0);
						// Check if service already link:
						auto it = m_listConnectedService.begin();
						while (it != m_listConnectedService.end()) {
							if (*it == nullptr) {
								++it;
								continue;
							}
							if ((*it)->getName() != serviceName) {
								++it;
								continue;
							}
							break;
						}
						if (it == m_listConnectedService.end()) {
							// check if service is connectable ...
							if (std::find(m_clientServices.begin(), m_clientServices.end(), serviceName) == m_clientServices.end()) {
								m_interfaceClient.answerError(transactionId, "UN-AUTHORIZED-SERVICE");
								return;
							}
							ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								jus::Future<bool> futLink = srv->m_interfaceClient.callClient(m_uid, "_new", m_userConnectionName, m_clientName, m_clientgroups);
								futLink.wait(); // TODO: Set timeout ...
								if (futLink.hasError() == true) {
									JUS_ERROR("Get error from the service ... LINK");
									m_interfaceClient.answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
									return;
								}
								m_listConnectedService.push_back(srv);
								m_interfaceClient.answerValue(transactionId, m_listConnectedService.size());
								return;
							}
							m_interfaceClient.answerError(transactionId, "CAN-NOT-CONNECT-SERVICE");
							return;
						}
						m_interfaceClient.answerError(transactionId, "SERVICE-ALREADY-CONNECTED");;
						return;
					}
					if (callFunction == "unlink") {
						// first param: the service we want to unconnect ...
						int64_t localServiceID = _value.getParameter<int64_t>(0)-1;
						// Check if service already link:
						if (localServiceID >= m_listConnectedService.size()) {
							m_interfaceClient.answerError(transactionId, "NOT-CONNECTED-SERVICE");
							return;
						}
						jus::Future<bool> futUnLink = m_listConnectedService[localServiceID]->m_interfaceClient.callClient(m_uid, "_delete");
						futUnLink.wait(); // TODO: Set timeout ...
						if (futUnLink.hasError() == true) {
							JUS_ERROR("Get error from the service ... UNLINK");
							m_interfaceClient.answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
							return;
						}
						m_listConnectedService[localServiceID] = nullptr;
						m_interfaceClient.answerValue(transactionId, true);
						return;
					}
					JUS_ERROR("Function does not exist ... '" << callFunction << "'");
					m_interfaceClient.answerError(transactionId, "CALL-UNEXISTING");
					return;
				}
				// decrease service ID ...
				serviceId -= 1;
				if (serviceId >= m_listConnectedService.size()) {
					m_interfaceClient.answerError(transactionId, "NOT-CONNECTED-SERVICE");
					return;
				} else {
					if (m_listConnectedService[serviceId] == nullptr) {
						// TODO ...
						JUS_ERROR("TODO : Manage this case ...");
						return;
					}
					uint16_t partId = _value.getPartId();
					if (partId != 0) {
						m_listConnectedService[serviceId]->m_interfaceClient.callForwardMultiple(
						    m_uid,
						    _value,
						    (uint64_t(m_uid) << 32) + uint64_t(transactionId));
						return;
					}
					m_listConnectedService[serviceId]->m_interfaceClient.callForward(
					    m_uid,
					    _value,
					    (uint64_t(m_uid) << 32) + uint64_t(transactionId),
					    [=](jus::FutureBase _ret) {
					    		jus::Buffer tmpp = _ret.getRaw();
					    		JUS_DEBUG("    ==> transmit : " << tmpp.getTransactionId() << " -> " << transactionId);
					    		JUS_DEBUG("    msg=" << tmpp.generateHumanString());
					    		tmpp.setTransactionId(transactionId);
					    		tmpp.setServiceId(serviceId+1);
					    		JUS_DEBUG("transmit=" << tmpp.generateHumanString());
					    		if (m_interfaceClient.getMode() == jus::connectionMode::modeJson) {
					    			ejson::Object obj = tmpp.toJson();
					    			m_interfaceClient.writeJson(obj);
					    		} else if (m_interfaceClient.getMode() == jus::connectionMode::modeBinary) {
					    			m_interfaceClient.writeBinary(tmpp);
					    		} else if (m_interfaceClient.getMode() == jus::connectionMode::modeXml) {
					    			JUS_ERROR("TODO ... ");
					    		} else {
					    			JUS_ERROR("wrong type of communication");
					    		}
					    		// multiple send element ...
					    		return tmpp.getPartFinish();
					    });
				}
			}
	}
}

void jus::GateWayClient::returnMessage(jus::Buffer& _data) {
	JUS_ERROR("Get call from the Service to the user ...");
}