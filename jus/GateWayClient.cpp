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
  m_interfaceClient(std::move(_connection)),
  m_interfaceMode(jus::connectionMode::modeJson),
  m_transactionLocalId(1) {
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
		linkService.add("call", ejson::String("_delete"));
		linkService.add("id", ejson::Number(m_transactionLocalId++));
		linkService.add("param", ejson::Array());
		it->SendData(m_uid, linkService);
	}
	if (m_userService != nullptr) {
		ejson::Object linkService;
		linkService.add("call", ejson::String("_delete"));
		linkService.add("id", ejson::Number(m_transactionLocalId++));
		linkService.add("param", ejson::Array());
		m_userService->SendData(m_uid2, linkService);
		m_userService = nullptr;
	}
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool jus::GateWayClient::isAlive() {
	return m_interfaceClient.isActive();
}

void jus::GateWayClient::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	answerError(_transactionId, protocolError, _errorHelp);
	m_state = jus::GateWayClient::state::disconnect;
	m_interfaceClient.disconnect(true);
}
/*
void jus::GateWayClient::answerValue(int32_t _transactionId, bool _value) {
	ejson::Object answer;
	answer.add("id", ejson::Number(_transactionId));
	answer.add("return", ejson::Boolean(_value));
	JUS_DEBUG("answer: " << answer.generateHumanString());
	m_interfaceClient.write(answer.generateMachineString());
}
*/

void jus::GateWayClient::answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorHelp) {
	if (m_interfaceMode == jus::connectionMode::modeJson) {
		ejson::Object answer;
		answer.add("error", ejson::String(protocolError));
		answer.add("id", ejson::Number(_clientTransactionId));
		answer.add("error-help", ejson::String(_errorHelp));
		JUS_DEBUG("answer: " << answer.generateHumanString());
		m_interfaceClient.write(answer.generateMachineString());
	} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
		jus::Buffer answer;
		answer.setType(jus::Buffer::typeMessage::answer);
		answer.setTransactionId(_clientTransactionId);
		answer.addError(protocolError, _errorHelp);
		m_interfaceClient.writeBinary(answer);
	} else if (m_interfaceMode == jus::connectionMode::modeXml) {
		JUS_ERROR("TODO ... ");
	} else {
		JUS_ERROR("wrong type of communication");
	}
}

void jus::GateWayClient::onClientDataRaw(jus::Buffer& _value) {
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
						answerValue(transactionId, true);
						m_interfaceMode = jus::connectionMode::modeJson;
						m_interfaceClient.connectCleanRaw();
						m_interfaceClient.connect(this, &jus::GateWayClient::onClientData);
					} else if (mode == "BIN") {
						JUS_WARNING("[" << m_uid << "] Change mode in: BINARY");
						answerValue(transactionId, true);
						m_interfaceMode = jus::connectionMode::modeBinary;
						m_interfaceClient.connectClean();
						m_interfaceClient.connectRaw(this, &jus::GateWayClient::onClientDataRaw);
					} else if (mode == "XML") {
						JUS_WARNING("[" << m_uid << "] Change mode in: XML");
						//m_interfaceMode = jus::connectionMode::modeXml;
						answerValue(transactionId, false);
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
							jus::Future<bool> futLocalService = call(m_uid2, m_userService, "_new", m_userConnectionName, "**Gateway**", std::vector<std::string>());
							futLocalService.wait(); // TODO: Set timeout ...
							m_state = jus::GateWayClient::state::userIdentify;
							answerValue(transactionId, true);
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
					jus::Future<bool> fut = call(m_uid2, m_userService, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 2");
						return;
					}
					m_clientName = clientName;
				}
				if (callFunction == "auth") {
					std::string password = _value.getParameter<std::string>(0);
					jus::Future<bool> fut = call(m_uid2, m_userService, "checkAuth", password);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						answerValue(transactionId, false);
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
				jus::Future<std::vector<std::string>> futGroup = call(m_uid2, m_userService, "getGroups", m_clientName);
				futGroup.wait(); // TODO: Set timeout ...
				if (futGroup.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					answerValue(transactionId, false);
					answerProtocolError(transactionId, "grouping error");
					return;
				}
				m_clientgroups = futGroup.get();
				// --------------------------------
				// -- Get services:
				// --------------------------------
				std::vector<std::string> currentServices = m_gatewayInterface->getAllServiceName();
				jus::Future<std::vector<std::string>> futServices = call(m_uid2, m_userService, "filterServices", m_clientName, currentServices);
				futServices.wait(); // TODO: Set timeout ...
				if (futServices.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					answerValue(transactionId, false);
					answerProtocolError(transactionId, "service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				JUS_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				JUS_WARNING("       groups: " << etk::to_string(m_clientgroups));
				JUS_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				answerValue(transactionId, true);
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
						answerValue(transactionId, m_clientServices.size());
						return;
					}
					if (callFunction == "getServiceList") {
						answerValue(transactionId, m_clientServices);
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
								answerError(transactionId, "UN-AUTHORIZED-SERVICE");
								return;
							}
							ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								jus::Future<bool> futLink = call(m_uid, srv, "_new", m_userConnectionName, m_clientName, m_clientgroups);
								futLink.wait(); // TODO: Set timeout ...
								if (futLink.hasError() == true) {
									JUS_ERROR("Get error from the service ... LINK");
									answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
									return;
								}
								m_listConnectedService.push_back(srv);
								answerValue(transactionId, m_listConnectedService.size());
								return;
							}
							answerError(transactionId, "CAN-NOT-CONNECT-SERVICE");
							return;
						}
						answerError(transactionId, "SERVICE-ALREADY-CONNECTED");;
						return;
					}
					if (callFunction == "unlink") {
						// first param: the service we want to unconnect ...
						int64_t localServiceID = _value.getParameter<int64_t>(0)-1;
						// Check if service already link:
						if (localServiceID >= m_listConnectedService.size()) {
							answerError(transactionId, "NOT-CONNECTED-SERVICE");
							return;
						}
						jus::Future<bool> futUnLink = call(m_uid, m_listConnectedService[localServiceID], "_delete");
						futUnLink.wait(); // TODO: Set timeout ...
						if (futUnLink.hasError() == true) {
							JUS_ERROR("Get error from the service ... UNLINK");
							answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
							return;
						}
						m_listConnectedService[localServiceID] = nullptr;
						answerValue(transactionId, true);
						return;
					}
					JUS_ERROR("Function does not exist ... '" << callFunction << "'");
					answerError(transactionId, "CALL-UNEXISTING");
					return;
				}
				// decrease service ID ...
				serviceId -= 1;
				if (serviceId >= m_listConnectedService.size()) {
					answerError(transactionId, "NOT-CONNECTED-SERVICE");
					return;
				} else {
					if (m_listConnectedService[serviceId] == nullptr) {
						// TODO ...
						JUS_ERROR("TODO : Manage this case ...");
						return;
					}
					bool finish = _value.getPartFinish();
					uint16_t partId = _value.getPartId();
					if (partId != 0) {
						// subMessage ... ==> try to forward message:
						std::unique_lock<std::mutex> lock(m_mutex);
						for (auto &itCall : m_pendingCall) {
							JUS_INFO(" compare : " << itCall.first << " =?= " << transactionId);
							if (itCall.first == transactionId) {
								// Find element ==> transit it ...
								_value.setTransactionId(itCall.second.getTransactionId());
								m_listConnectedService[serviceId]->SendData(m_uid, _value);
								return;
							}
						}
						JUS_ERROR("Can not transfer part of a message ...");
						return;
					}
					callActionForward(m_listConnectedService[serviceId],
					                  _value,
					                  [=](jus::FutureBase _ret) {
					                  		// TODO : Check if it is a JSON or binary ...
					                  		ejson::Object tmpp = _ret.getRaw();
					                  		JUS_VERBOSE("    ==> transmit : " << tmpp["id"].toNumber().getU64() << " -> " << transactionId);
					                  		JUS_VERBOSE("    msg=" << tmpp.generateMachineString());
					                  		tmpp["id"].toNumber().set(uint64_t(transactionId));
					                  		JUS_DEBUG("transmit=" << tmpp.generateMachineString());
					                  		m_interfaceClient.write(tmpp.generateMachineString());
					                  		if (tmpp.valueExist("part") == true) {
					                  			// multiple send element ...
					                  			if (tmpp.valueExist("finish") == true) {
					                  				return tmpp["finish"].toBoolean().get();
					                  			}
					                  			return false;
					                  		}
					                  		return true;
					                  });
				}
			}
	}
}

void jus::GateWayClient::onClientData(std::string _value) {
	JUS_DEBUG("On data: " << _value);
	ejson::Object data(_value);
	uint64_t transactionId = data["id"].toNumber().getU64();
	if (transactionId == 0) {
		JUS_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	std::string callFunction = data["call"].toString().get();
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
					std::string mode = data["param"].toArray()[0].toString().get();
					if (mode == "JSON") {
						JUS_WARNING("[" << m_uid << "] Change mode in: JSON");
						answerValue(transactionId, true);
						m_interfaceMode = jus::connectionMode::modeJson;
						m_interfaceClient.connectCleanRaw();
						m_interfaceClient.connect(this, &jus::GateWayClient::onClientData);
					} else if (mode == "BIN") {
						JUS_WARNING("[" << m_uid << "] Change mode in: BINARY");
						answerValue(transactionId, true);
						m_interfaceMode = jus::connectionMode::modeBinary;
						m_interfaceClient.connectClean();
						m_interfaceClient.connectRaw(this, &jus::GateWayClient::onClientDataRaw);
					} else if (mode == "XML") {
						JUS_WARNING("[" << m_uid << "] Change mode in: XML");
						//m_interfaceMode = jus::connectionMode::modeXml;
						answerValue(transactionId, false);
					} else {
						answerProtocolError(transactionId, std::string("Call setMode with unknow argument : '") /*+ etk::to_string(int32_t(mode))*/ + "' supported [JSON/XML/BIN]");
					}
					return;
				} else if (callFunction == "connectToUser") {
					m_userConnectionName = data["param"].toArray()[0].toString().get();
					if (m_userConnectionName == "") {
						answerProtocolError(transactionId, "Call connectToUser with no parameter 'user'");
					} else {
						JUS_WARNING("[" << m_uid << "] Set client connect to user : '" << m_userConnectionName << "'");
						
						m_userService = m_gatewayInterface->get("system-user");
						if (m_userService == nullptr) {
							answerProtocolError(transactionId, "Gateway internal error 'No user interface'");
						} else {
							jus::Future<bool> futLocalService = call(m_uid2, m_userService, "_new", m_userConnectionName, "**Gateway**", std::vector<std::string>());
							futLocalService.wait(); // TODO: Set timeout ...
							m_state = jus::GateWayClient::state::userIdentify;
							answerValue(transactionId, true);
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
					std::string clientName = data["param"].toArray()[0].toString().get();
					std::string clientTocken = data["param"].toArray()[1].toString().get();
					if (m_userService == nullptr) {
						answerProtocolError(transactionId, "gateWay internal error 3");
						return;
					}
					jus::Future<bool> fut = call(m_uid2, m_userService, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 2");
						return;
					}
					m_clientName = clientName;
				}
				if (callFunction == "auth") {
					std::string password = data["param"].toArray()[0].toString().get();
					jus::Future<bool> fut = call(m_uid2, m_userService, "checkAuth", password);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						answerValue(transactionId, false);
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
				jus::Future<std::vector<std::string>> futGroup = call(m_uid2, m_userService, "getGroups", m_clientName);
				futGroup.wait(); // TODO: Set timeout ...
				if (futGroup.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					answerValue(transactionId, false);
					answerProtocolError(transactionId, "grouping error");
					return;
				}
				m_clientgroups = futGroup.get();
				// --------------------------------
				// -- Get services:
				// --------------------------------
				std::vector<std::string> currentServices = m_gatewayInterface->getAllServiceName();
				jus::Future<std::vector<std::string>> futServices = call(m_uid2, m_userService, "filterServices", m_clientName, currentServices);
				futServices.wait(); // TODO: Set timeout ...
				if (futServices.hasError() == true) {
					JUS_ERROR("Get error from the service ...");
					answerValue(transactionId, false);
					answerProtocolError(transactionId, "service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				JUS_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				JUS_WARNING("       groups: " << etk::to_string(m_clientgroups));
				JUS_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				answerValue(transactionId, true);
				m_state = jus::GateWayClient::state::clientIdentify;
				return;
			}
			break;
		case jus::GateWayClient::state::clientIdentify:
			{
				ejson::Number numService = data["service"].toNumber();
				if (    numService.exist() == false
				     || numService.getU64() == 0) {
					// This is 2 default service for the cient interface that manage the authorisation of view:
					if (callFunction == "getServiceCount") {
						answerValue(transactionId, m_clientServices.size());
						return;
					}
					if (callFunction == "getServiceList") {
						answerValue(transactionId, m_clientServices);
						//listService.add(ejson::String("ServiceManager/v0.1.0"));
						return;
					}
					if (callFunction == "link") {
						// first param:
						std::string serviceName = data["param"].toArray()[0].toString().get();
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
								answerError(transactionId, "UN-AUTHORIZED-SERVICE");
								return;
							}
							ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								jus::Future<bool> futLink = call(m_uid, srv, "_new", m_userConnectionName, m_clientName, m_clientgroups);
								futLink.wait(); // TODO: Set timeout ...
								if (futLink.hasError() == true) {
									JUS_ERROR("Get error from the service ... LINK");
									answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
									return;
								}
								m_listConnectedService.push_back(srv);
								answerValue(transactionId, m_listConnectedService.size());
								return;
							}
							answerError(transactionId, "CAN-NOT-CONNECT-SERVICE");
							return;
						}
						answerError(transactionId, "SERVICE-ALREADY-CONNECTED");;
						return;
					}
					if (callFunction == "unlink") {
						// first param:
						int64_t localServiceID = data["param"].toArray()[0].toNumber().getI64()-1;
						// Check if service already link:
						if (localServiceID >= m_listConnectedService.size()) {
							answerError(transactionId, "NOT-CONNECTED-SERVICE");
							return;
						}
						jus::Future<bool> futUnLink = call(m_uid, m_listConnectedService[localServiceID], "_delete");
						futUnLink.wait(); // TODO: Set timeout ...
						if (futUnLink.hasError() == true) {
							JUS_ERROR("Get error from the service ... UNLINK");
							answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
							return;
						}
						m_listConnectedService[localServiceID] = nullptr;
						answerValue(transactionId, true);
						return;
					}
					JUS_ERROR("Function does not exist ... '" << callFunction << "'");
					answerError(transactionId, "CALL-UNEXISTING");
					return;
				}
				
				uint64_t serviceId = numService.getU64()-1;
				if (serviceId >= m_listConnectedService.size()) {
					ejson::Object answer;
					answer.add("id", data["id"]);
					answer.add("error", ejson::String("NOT-CONNECTED-SERVICE"));
					JUS_DEBUG("answer: " << answer.generateHumanString());
					m_interfaceClient.write(answer.generateMachineString());
					return;
				} else {
					bool finish = false;
					if (data.valueExist("finish") == true) {
						finish = data["finish"].toBoolean().get();
					}
					int64_t partTmp = -1;
					if (data.valueExist("part") == true) {
						uint64_t part = data["part"].toNumber().getU64();
						partTmp = part;
						if (part != 0) {
							// subMessage ... ==> try to forward message:
							std::unique_lock<std::mutex> lock(m_mutex);
							for (auto &itCall : m_pendingCall) {
								JUS_INFO(" compare : " << itCall.first << " =?= " << transactionId);
								if (itCall.first == transactionId) {
									// Find element ==> transit it ...
									if (m_listConnectedService[serviceId] == nullptr) {
										// TODO ...
									} else {
										ejson::Object obj;
										obj.add("id", ejson::Number(itCall.second.getTransactionId()));
										obj.add("param-id", data["param-id"]);
										obj.add("part", ejson::Number(part));
										obj.add("data", data["data"]);
										if (finish == true) {
											obj.add("finish", ejson::Boolean(true));
										}
										m_listConnectedService[serviceId]->SendData(m_uid, obj);
									}
									return;
								}
							}
							JUS_ERROR("Can not transfer part of a message ...");
							return;
						}
					}
					callActionForward(m_uid,
					                  transactionId,
					                  m_listConnectedService[serviceId],
					                  data["call"].toString().get(),
					                  data["param"].toArray(),
					                  [=](jus::FutureBase _ret) {
					                  		ejson::Object tmpp = _ret.getRaw();
					                  		JUS_VERBOSE("    ==> transmit : " << tmpp["id"].toNumber().getU64() << " -> " << data["id"].toNumber().getU64());
					                  		JUS_VERBOSE("    msg=" << tmpp.generateMachineString());
					                  		tmpp["id"].toNumber().set(transactionId);
					                  		JUS_DEBUG("transmit=" << tmpp.generateMachineString());
					                  		m_interfaceClient.write(tmpp.generateMachineString());
					                  		if (tmpp.valueExist("part") == true) {
					                  			// multiple send element ...
					                  			if (tmpp.valueExist("finish") == true) {
					                  				return tmpp["finish"].toBoolean().get();
					                  			}
					                  			return false;
					                  		}
					                  		return true;
					                  },
					                  partTmp,
					                  finish);
				}
			}
	}
}

jus::FutureBase jus::GateWayClient::callActionForward(uint64_t _callerId,
                                                      uint64_t _clientTransactionId,
                                                      ememory::SharedPtr<jus::GateWayService> _srv,
                                                      const std::string& _functionName,
                                                      ejson::Array _params,
                                                      jus::FutureData::ObserverFinish _callback,
                                                      int64_t _part,
                                                      bool _finish) {
	uint64_t id = getId();
	ejson::Object callElem = jus::createCallJson(id, _functionName, _params);
	if (_part != -1) {
		callElem.add("part", ejson::Number(uint64_t(_part)));
	}
	if (_finish == true) {
		callElem.add("finish", ejson::Boolean(true));
	}
	jus::FutureBase ret = callJson(_callerId, _srv, _clientTransactionId, id, callElem, _callback);
	ret.setSynchronous();
	return ret;
}

jus::FutureBase jus::GateWayClient::callBinary(uint64_t _callerId,
                                               ememory::SharedPtr<jus::GateWayService> _srv,
                                               uint64_t _clientTransactionId,
                                               uint64_t _transactionId,
                                               jus::Buffer& _obj,
                                               jus::FutureData::ObserverFinish _callback) {
	JUS_VERBOSE("Send BINARY [START] ");
	if (_srv == nullptr) {
		// TODO : Change this ...
		jus::Buffer obj;
		obj.setTransactionId(_transactionId);
		obj.setClientId(_callerId);
		obj.setType(jus::Buffer::typeMessage::answer);
		obj.addError("NOT-CONNECTED", "Client interface not connected (no TCP)");
		return jus::FutureBase(_transactionId, true, obj, _callback);
	}
	jus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_pendingCall.push_back(std::make_pair(_clientTransactionId, tmpFuture));
	}
	_srv->SendData(_callerId, _obj);
	JUS_VERBOSE("Send BINARY [STOP]");
	return tmpFuture;
}

jus::FutureBase jus::GateWayClient::callActionForward(ememory::SharedPtr<jus::GateWayService> _srv,
                                                      jus::Buffer& _Buffer,
                                                      jus::FutureData::ObserverFinish _callback) {
	uint64_t id = getId();
	uint64_t clientTransactionId = _Buffer.getTransactionId();
	jus::FutureBase ret = callBinary(m_uid, _srv, clientTransactionId, id, _Buffer, _callback);
	ret.setSynchronous();
	return ret;
}


uint64_t jus::GateWayClient::getId() {
	return m_transactionLocalId++;
}

jus::FutureBase jus::GateWayClient::callJson(uint64_t _callerId,
                                             ememory::SharedPtr<jus::GateWayService> _srv,
                                             uint64_t _clientTransactionId,
                                             uint64_t _transactionId,
                                             const ejson::Object& _obj,
                                             jus::FutureData::ObserverFinish _callback) {
	JUS_VERBOSE("Send JSON [START] ");
	if (_srv == nullptr) {
		ejson::Object obj;
		obj.add("error", ejson::String("NOT-CONNECTED"));
		obj.add("error-help", ejson::String("Client interface not connected (no TCP)"));
		return jus::FutureBase(_transactionId, true, obj, _callback);
	}
	jus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_pendingCall.push_back(std::make_pair(_clientTransactionId, tmpFuture));
	}
	_srv->SendData(_callerId, _obj);
	JUS_VERBOSE("Send JSON [STOP]");
	return tmpFuture;
}





void jus::GateWayClient::returnMessage(ejson::Object _data) {
	jus::FutureBase future;
	uint64_t tid = _data["id"].toNumber().get();
	if (tid == 0) {
		if (_data["error"].toString().get() == "PROTOCOL-ERROR") {
			JUS_ERROR("Get a Protocol error ...");
			std::unique_lock<std::mutex> lock(m_mutex);
			for (auto &it : m_pendingCall) {
				if (it.second.isValid() == false) {
					continue;
				}
				it.second.setAnswer(_data);
			}
			m_pendingCall.clear();
		} else {
			JUS_ERROR("call with no ID ==> error ...");
		}
		return;
	}
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_pendingCall.begin();
		while (it != m_pendingCall.end()) {
			if (it->second.isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->second.getTransactionId() != tid) {
				++it;
				continue;
			}
			// TODO : Do it better ...
			future = it->second;
			break;
		}
	}
	if (future.isValid() == false) {
		JUS_WARNING("Action to do ...");
		return;
	}
	bool ret = future.setAnswer(_data);
	if (ret == true) {
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_pendingCall.begin();
		while (it != m_pendingCall.end()) {
			if (it->second.isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->second.getTransactionId() != tid) {
				++it;
				continue;
			}
			it = m_pendingCall.erase(it);
			break;
		}
	}
}

