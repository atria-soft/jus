/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/RouterInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const std::string protocolError = "PROTOCOL-ERROR";

appl::userSpecificInterface::userSpecificInterface(const std::string& _userName) {
	m_uid = 0;
	m_localIdUser = 0;
	m_userConnectionName = _userName;
	m_state = appl::clientState::unconnect;
	APPL_INFO("----------------");
	APPL_INFO("-- NEW Client --");
	APPL_INFO("----------------");
}

appl::userSpecificInterface::~userSpecificInterface() {
	APPL_INFO("-------------------");
	APPL_INFO("-- DELETE Client --");
	APPL_INFO("-------------------");
}

void appl::userSpecificInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceRouterClient->answerError(_transactionId, protocolError, _errorHelp);
	m_interfaceRouterClient->sendCtrl("DISCONNECT", m_uid);
	m_state = appl::clientState::disconnect;
}

bool appl::userSpecificInterface::start(uint32_t _transactionId, appl::GateWay* _gatewayInterface, zeus::WebServer* _interfaceGateWayClient, uint64_t _id) {
	m_interfaceRouterClient = _interfaceGateWayClient;
	m_gatewayInterface = _gatewayInterface;
	m_uid = _id;
	m_localIdUser = _id+1;
	m_state = appl::clientState::connect;
	//m_interfaceRouterClient->setInterfaceName("cli-" + etk::to_string(m_uid));
	
	APPL_WARNING("[" << m_uid << "] New client : " << m_clientName);
	
	m_userService = m_gatewayInterface->get("user");
	if (m_userService == nullptr) {
		APPL_ERROR("missing service 'user'");
		answerProtocolError(_transactionId, "Gateway internal error 'No user interface'");
		return false;
	}
	zeus::Future<bool> futLocalService = m_userService->m_interfaceClient.callClient(m_localIdUser, "_new", m_userConnectionName, "**Gateway**", std::vector<std::string>());
	futLocalService.wait(); // TODO: Set timeout ...
	if (futLocalService.get() == false) {
		answerProtocolError(_transactionId, "Gateway internal error 'Can not create client in user backend'");
		return false;
	}
	return true;
}


void appl::userSpecificInterface::returnMessage(ememory::SharedPtr<zeus::Buffer> _data) {
	APPL_ERROR("Get call from the Service to the user ...");
}

void appl::userSpecificInterface::onClientData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	//APPL_ERROR("    ==> parse DATA ...");
	uint32_t transactionId = _value->getTransactionId();
	if (transactionId == 0) {
		APPL_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		// TRANSMIT DATA ...
		if (m_state != appl::clientState::clientIdentify) {
			answerProtocolError(transactionId, "Not identify to send 'data' buffer (multiple packet element)");
			return;
		}
		uint32_t serviceId = _value->getServiceId();
		if (serviceId == 0) {
			answerProtocolError(transactionId, "Can not send multiple data on the gateway");
			return;
		}
		serviceId--;
		if (serviceId >= m_listConnectedService.size()) {
			m_interfaceRouterClient->answerError(transactionId, "NOT-CONNECTED-SERVICE");
			return;
		}
		if (m_listConnectedService[serviceId] == nullptr) {
			// TODO ...
			APPL_ERROR("TODO : Manage this case ...");
			return;
		}
		m_listConnectedService[serviceId]->m_interfaceClient.callForwardMultiple(
		    m_uid,
		    _value,
		    (uint64_t(m_uid) << 32) + uint64_t(transactionId));
		return;
	}
	if (_value->getType() != zeus::Buffer::typeMessage::call) {
		APPL_ERROR("Protocol error ==>missing 'call'");
		answerProtocolError(transactionId, "missing parameter: 'call' / wrong type 'call'");
		return;
	}
	ememory::SharedPtr<zeus::BufferCall> callObj = ememory::staticPointerCast<zeus::BufferCall>(_value);
	std::string callFunction = callObj->getCall();
	switch (m_state) {
		case appl::clientState::disconnect:
		case appl::clientState::unconnect:
			{
				APPL_ERROR("Must never appear");
				answerProtocolError(transactionId, "Gateway internal error");
				return;
			}
		case appl::clientState::connect:
			{
				uint32_t serviceId = callObj->getServiceId();
				if (serviceId != 0) {
					APPL_ERROR("Call at a service at this state is not allowed serviceID=" << serviceId);
					answerProtocolError(transactionId, "MISSING IDENTIFICATION STEP");
					return;
				}
				
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
					std::string clientName = callObj->getParameter<std::string>(0);
					std::string clientTocken = callObj->getParameter<std::string>(1);
					if (m_userService == nullptr) {
						answerProtocolError(transactionId, "gateWay internal error 3");
						return;
					}
					
					zeus::Future<bool> fut = m_userService->m_interfaceClient.callClient(m_localIdUser, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						APPL_ERROR("Get error from the service ...");
						m_interfaceRouterClient->answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceRouterClient->answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 2");
						return;
					}
					m_clientName = clientName;
				}
				if (callFunction == "auth") {
					std::string password = callObj->getParameter<std::string>(0);
					zeus::Future<bool> fut = m_userService->m_interfaceClient.callClient(m_localIdUser, "checkAuth", password);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						APPL_ERROR("Get error from the service ...");
						m_interfaceRouterClient->answerValue(transactionId, false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceRouterClient->answerValue(transactionId, false);
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
				zeus::Future<std::vector<std::string>> futGroup = m_userService->m_interfaceClient.callClient(m_localIdUser, "clientGroupsGet", m_clientName);
				futGroup.wait(); // TODO: Set timeout ...
				if (futGroup.hasError() == true) {
					APPL_ERROR("Get error from the service ...");
					m_interfaceRouterClient->answerValue(transactionId, false);
					answerProtocolError(transactionId, "grouping error");
					return;
				}
				m_clientgroups = futGroup.get();
				// --------------------------------
				// -- Get services:
				// --------------------------------
				std::vector<std::string> currentServices = m_gatewayInterface->getAllServiceName();
				zeus::Future<std::vector<std::string>> futServices = m_userService->m_interfaceClient.callClient(m_localIdUser, "filterClientServices", m_clientName, currentServices);
				futServices.wait(); // TODO: Set timeout ...
				if (futServices.hasError() == true) {
					APPL_ERROR("Get error from the service ...");
					m_interfaceRouterClient->answerValue(transactionId, false);
					answerProtocolError(transactionId, "service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				APPL_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				APPL_WARNING("       groups: " << etk::to_string(m_clientgroups));
				APPL_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				m_interfaceRouterClient->answerValue(transactionId, true);
				m_state = appl::clientState::clientIdentify;
				return;
			}
			break;
		case appl::clientState::clientIdentify:
			{
				uint32_t serviceId = callObj->getServiceId();
				
				if (serviceId == 0) {
					// This is 2 default service for the cient interface that manage the authorisation of view:
					if (callFunction == "getServiceCount") {
						m_interfaceRouterClient->answerValue(transactionId, m_clientServices.size(), m_uid);
						return;
					}
					if (callFunction == "getServiceList") {
						m_interfaceRouterClient->answerValue(transactionId, m_clientServices, m_uid);
						//"ServiceManager/v0.1.0"
						return;
					}
					if (callFunction == "link") {
						// first param:
						std::string serviceName = callObj->getParameter<std::string>(0);
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
								m_interfaceRouterClient->answerError(transactionId, "UN-AUTHORIZED-SERVICE");
								return;
							}
							ememory::SharedPtr<appl::ServiceInterface> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								zeus::Future<bool> futLink = srv->m_interfaceClient.callClient(m_uid, "_new", m_userConnectionName, m_clientName, m_clientgroups);
								futLink.wait(); // TODO: Set timeout ...
								if (futLink.hasError() == true) {
									APPL_ERROR("Get error from the service ... LINK");
									m_interfaceRouterClient->answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
									return;
								}
								m_listConnectedService.push_back(srv);
								m_interfaceRouterClient->answerValue(transactionId, m_listConnectedService.size(), m_uid);
								return;
							}
							m_interfaceRouterClient->answerError(transactionId, "CAN-NOT-CONNECT-SERVICE");
							return;
						}
						m_interfaceRouterClient->answerError(transactionId, "SERVICE-ALREADY-CONNECTED");;
						return;
					}
					if (callFunction == "unlink") {
						// first param: the service we want to unconnect ...
						int64_t localServiceID = callObj->getParameter<int64_t>(0)-1;
						// Check if service already link:
						if (localServiceID >= m_listConnectedService.size()) {
							m_interfaceRouterClient->answerError(transactionId, "NOT-CONNECTED-SERVICE");
							return;
						}
						zeus::Future<bool> futUnLink = m_listConnectedService[localServiceID]->m_interfaceClient.callClient(m_uid, "_delete");
						futUnLink.wait(); // TODO: Set timeout ...
						if (futUnLink.hasError() == true) {
							APPL_ERROR("Get error from the service ... UNLINK");
							m_interfaceRouterClient->answerError(transactionId, "ERROR-CREATE-SERVICE-INSTANCE");
							return;
						}
						m_listConnectedService[localServiceID] = nullptr;
						m_interfaceRouterClient->answerValue(transactionId, true);
						return;
					}
					APPL_ERROR("Function does not exist ... '" << callFunction << "'");
					m_interfaceRouterClient->answerError(transactionId, "CALL-UNEXISTING");
					return;
				}
				// decrease service ID ...
				serviceId -= 1;
				if (serviceId >= m_listConnectedService.size()) {
					m_interfaceRouterClient->answerError(transactionId, "NOT-CONNECTED-SERVICE");
					return;
				} else {
					if (m_listConnectedService[serviceId] == nullptr) {
						// TODO ...
						APPL_ERROR("TODO : Manage this case ...");
						return;
					}
					m_listConnectedService[serviceId]->m_interfaceClient.callForward(
					    m_uid,
					    _value,
					    (uint64_t(m_uid) << 32) + uint64_t(transactionId),
					    [=](zeus::FutureBase _ret) {
					    		ememory::SharedPtr<zeus::Buffer> tmpp = _ret.getRaw();
					    		if (tmpp == nullptr) {
					    			return true;
					    		}
					    		APPL_DEBUG("    ==> transmit : " << tmpp->getTransactionId() << " -> " << transactionId);
					    		APPL_DEBUG("    msg=" << tmpp);
					    		tmpp->setTransactionId(transactionId);
					    		tmpp->setServiceId(serviceId+1);
					    		APPL_DEBUG("transmit=" << tmpp);
					    		m_interfaceRouterClient->writeBinary(tmpp);
					    		// multiple send element ...
					    		return tmpp->getPartFinish();
					    });
				}
			}
	}
}




appl::RouterInterface::RouterInterface(const std::string& _ip, uint16_t _port, const std::string& _userName, appl::GateWay* _gatewayInterface) :
  m_state(appl::clientState::unconnect),
  m_gatewayInterface(_gatewayInterface),
  m_interfaceRouterClient() {
	APPL_INFO("----------------------------------------");
	APPL_INFO("-- NEW Connection to GateWay Font-end --");
	APPL_INFO("----------------------------------------");
	enet::Tcp connection = std::move(enet::connectTcpClient(_ip, _port));
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		APPL_ERROR("Can not connect the GateWay-front-end");
		return;
	}
	m_interfaceRouterClient.setInterface(std::move(connection), false, _userName);
	m_userConnectionName = _userName;
	m_state = appl::clientState::connect;
	m_interfaceRouterClient.connect(this, &appl::RouterInterface::onClientData);
	m_interfaceRouterClient.connect(true);
	m_interfaceRouterClient.setInterfaceName("cli-GateWay-front-end");
	// TODO : Check if user name is accepted ...
}

appl::RouterInterface::~RouterInterface() {
	APPL_TODO("Call All unlink ...");
	stop();
	APPL_INFO("-------------------------------------------");
	APPL_INFO("-- DELETE Connection to GateWay Font-end --");
	APPL_INFO("-------------------------------------------");
}

void appl::RouterInterface::stop() {
	/* TODO ...
	for (auto &it : m_listConnectedService) {
		if (it == nullptr) {
			continue;
		}
		it->m_interfaceClient.callClient(m_uid, "_delete");
	}
	if (m_userService != nullptr) {
		m_userService->m_interfaceClient.callClient(m_uid2, "_delete");
		m_userService = nullptr;
	}
	m_listConnectedService.clear();
	*/
	m_interfaceRouterClient.disconnect();
}

bool appl::RouterInterface::isAlive() {
	return m_interfaceRouterClient.isActive();
}



void appl::RouterInterface::onClientData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	// Get client ID:
	uint64_t clientId = _value->getClientId();
	//APPL_ERROR("[" << clientId << "] get message from front-end gateWay: " << _value);
	int64_t localId = -1;
	for (size_t iii=0; iii<m_listUser.size(); ++iii) {
		if (m_listUser[iii].m_uid == clientId) {
			localId = iii;
			break;
		}
	}
	if (localId == -1) {
		m_listUser.push_back(userSpecificInterface(m_userConnectionName));
		localId = m_listUser.size()-1;
		bool ret = m_listUser[localId].start(_value->getTransactionId(), m_gatewayInterface, &m_interfaceRouterClient, clientId);
		if (ret == false) {
			return;
		}
	}
	m_listUser[localId].onClientData(std::move(_value));
}

void appl::RouterInterface::answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data) {
	for (auto &it : m_listUser) {
		if (it.checkId(_userSessionId) == false) {
			continue;
		}
		it.returnMessage(_data);
		return;
	}
}

void appl::RouterInterface::clean() {
	auto it = m_listUser.begin();
	while (it != m_listUser.end()) {
		if (it->m_state == appl::clientState::disconnect) {
			it = m_listUser.erase(it);
			continue;
		}
		++it;
	}
}