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
	uint64_t transactionId = data["id"].toNumber().getU64();
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
							linkService.add("client", ejson::String("**Gateway**"));
							linkService.add("groups", ejson::Array());
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
				std::string callFunction = data["call"].toString().get();
				m_clientServices.clear();
				m_clientgroups.clear();
				m_clientName.clear();
				
				if (    callFunction != "identify"
				     && callFunction != "auth"
				     && callFunction != "anonymous") {
					protocolError("Client must call: identify/auth/anonymous");
					return;
				}
				if (callFunction == "identify") {
					std::string clientName = data["param"].toArray()[0].toString().get();
					std::string clientTocken = data["param"].toArray()[1].toString().get();
					if (m_userService == nullptr) {
						protocolError("gateWay internal error 3");
						return;
					}
					jus::Future<bool> fut = call(m_uid2, m_userService, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						JUS_ERROR("Get error from the service ...");
						returnBool(transactionId, false);
						protocolError("connection refused 1");
						return;
					} else if (fut.get() == false) {
						returnBool(transactionId, false);
						protocolError("connection refused 2");
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
						returnBool(transactionId, false);
						protocolError("connection refused 1");
						return;
					} else if (fut.get() == false) {
						returnBool(transactionId, false);
						protocolError("connection refused 2");
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
					returnBool(transactionId, false);
					protocolError("grouping error");
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
					returnBool(transactionId, false);
					protocolError("service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				JUS_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				JUS_WARNING("       groups: " << etk::to_string(m_clientgroups));
				JUS_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				returnBool(transactionId, true);
				m_state = jus::GateWayClient::state::clientIdentify;
				return;
			}
			break;
		case jus::GateWayClient::state::clientIdentify:
			{
				// This is 2 default service for the cient interface that manage the authorisation of view:
				if (data.valueExist("service") == false) {
					std::string callFunction = data["call"].toString().get();
					ejson::Object answer;
					//answer.add("from-service", ejson::String(""));
					answer.add("id", data["id"]);
					if (callFunction == "getServiceCount") {
						answer.add("return", ejson::Number(m_clientServices.size()));
						JUS_DEBUG("answer: " << answer.generateHumanString());
						m_interfaceClient.write(answer.generateMachineString());
						return;
					}
					if (callFunction == "getServiceList") {
						ejson::Array listService;
						for (auto &it : m_clientServices) {
							listService.add(ejson::String(it));
						}
						//listService.add(ejson::String("ServiceManager/v0.1.0"));
						answer.add("return", listService);
						JUS_DEBUG("answer: " << answer.generateHumanString());
						m_interfaceClient.write(answer.generateMachineString());
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
								answer.add("return", ejson::Boolean(false));
								JUS_DEBUG("answer: (NOT authorized service) " << answer.generateHumanString());
								m_interfaceClient.write(answer.generateMachineString());
								return;
							}
							ememory::SharedPtr<jus::GateWayService> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								ejson::Object linkService;
								linkService.add("event", ejson::String("new"));
								linkService.add("user", ejson::String(m_userConnectionName));
								linkService.add("client", ejson::String(m_clientName));
								// TODO ==> remove events ...
								std::vector<ActionAsyncClient> asyncAction;
								linkService.add("groups", convertToJson(asyncAction, 0, m_clientgroups));
								if (asyncAction.size() != 0) {
									JUS_ERROR("Missing send async messages");
								}
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
					if (callFunction == "unlink") {
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
					JUS_ERROR("Function does not exist ... '" << callFunction << "'");
					answer.add("error", ejson::String("CALL-UNEXISTING"));
					JUS_DEBUG("answer: " << answer.generateHumanString());
					m_interfaceClient.write(answer.generateMachineString());
					return;
				}
				std::string service = data["service"].toString().get();
				if (service == "") {
					protocolError("call with \"service\"=\"\" ==> not permited");
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
									if (*it == nullptr) {
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
										(*it)->SendData(m_uid, obj);
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
					                  *it,
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

uint64_t jus::GateWayClient::getId() {
	return m_transactionLocalId++;
}

jus::FutureBase jus::GateWayClient::callJson(uint64_t _callerId, ememory::SharedPtr<jus::GateWayService> _srv, uint64_t _clientTransactionId, uint64_t _transactionId, const ejson::Object& _obj, jus::FutureData::ObserverFinish _callback) {
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

