/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/IOInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const std::string protocolError = "PROTOCOL-ERROR";

appl::IOInterface::IOInterface() {
	m_uid = 0;
	m_state = appl::clientState::unconnect;
	APPL_INFO("------------");
	APPL_INFO("-- NEW IO --");
	APPL_INFO("------------");
}

appl::IOInterface::~IOInterface() {
	APPL_INFO("---------------");
	APPL_INFO("-- DELETE IO --");
	APPL_INFO("---------------");
}

void appl::IOInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	zeus::WebServer* iface = getInterface();
	iface->answerError(_transactionId, 0, ZEUS_ID_SERVICE_ROOT, protocolError, _errorHelp);
	//m_interfaceRouterClient->sendCtrl(m_routeurUID, ZEUS_ID_SERVICE_ROOT, "DISCONNECT");
	APPL_TODO("Do this error return ... " << _errorHelp);
	m_state = appl::clientState::disconnect;
}

bool appl::IOInterface::start(appl::GateWay* _gateway, uint16_t _id) {
	m_gateway = _gateway;
	m_uid = _id;
	m_state = appl::clientState::connect;
	//m_interfaceRouterClient->setInterfaceName("cli-" + etk::to_string(m_uid));
	APPL_WARNING("[" << m_uid << "] New IO interface");
	return true;
}


void appl::IOInterface::receive(ememory::SharedPtr<zeus::Buffer> _value) {
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
	// Check if we are the destinated Of this message 
	if (    _value->getDestinationId() == ZEUS_ID_GATEWAY
	     && _value->getDestinationObjectId() == ZEUS_ID_GATEWAY_OBJECT) {
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
					/*
					m_clientServices.clear();
					m_clientgroups.clear();
					m_clientName.clear();
					*/
					if (    callFunction != "identify"
					     && callFunction != "auth"
					     && callFunction != "anonymous") {
						answerProtocolError(transactionId, "Client must call: identify/auth/anonymous");
						return;
					}
					#if 0
						if (callFunction == "identify") {
							std::string clientName = callObj->getParameter<std::string>(0);
							std::string clientTocken = callObj->getParameter<std::string>(1);
							if (m_userService == nullptr) {
								answerProtocolError(transactionId, "gateWay internal error 3");
								return;
							}
							
							zeus::Future<bool> fut = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "checkTocken", clientName, clientTocken);
							fut.wait(); // TODO: Set timeout ...
							if (fut.hasError() == true) {
								APPL_ERROR("Get error from the service ...");
								m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
								answerProtocolError(transactionId, "connection refused 1");
								return;
							} else if (fut.get() == false) {
								m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
								answerProtocolError(transactionId, "connection refused 2");
								return;
							}
							m_clientName = clientName;
						}
						if (callFunction == "auth") {
							std::string password = callObj->getParameter<std::string>(0);
							zeus::Future<bool> fut = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "checkAuth", password);
							fut.wait(); // TODO: Set timeout ...
							if (fut.hasError() == true) {
								APPL_ERROR("Get error from the service ...");
								m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
								answerProtocolError(transactionId, "connection refused 1");
								return;
							} else if (fut.get() == false) {
								m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
								answerProtocolError(transactionId, "connection refused 2");
								return;
							}
							m_clientName = m_userConnectionName;
						}
						if (callFunction == "anonymous") {
							m_clientName = "";
						}
					#endif
					#if 0
						// --------------------------------
						// -- Get groups:
						// --------------------------------
						zeus::Future<std::vector<std::string>> futGroup = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "clientGroupsGet", m_clientName);
						futGroup.wait(); // TODO: Set timeout ...
						if (futGroup.hasError() == true) {
							APPL_ERROR("Get error from the service ...");
							m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
							answerProtocolError(transactionId, "grouping error");
							return;
						}
						m_clientgroups = futGroup.get();
						// --------------------------------
						// -- Get services:
						// --------------------------------
						std::vector<std::string> currentServices = m_gatewayInterface->getAllServiceName();
						zeus::Future<std::vector<std::string>> futServices = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "filterClientServices", m_clientName, currentServices);
						futServices.wait(); // TODO: Set timeout ...
						if (futServices.hasError() == true) {
							APPL_ERROR("Get error from the service ...");
							m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
							answerProtocolError(transactionId, "service filtering error");
							return;
						}
						m_clientServices = futServices.get();
						APPL_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
						APPL_WARNING("       groups: " << etk::to_string(m_clientgroups));
						APPL_WARNING("     services: " << etk::to_string(m_clientServices));
					#endif
					zeus::WebServer* iface = getInterface();
					iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
					m_state = appl::clientState::clientIdentify;
					return;
				}
				break;
			case appl::clientState::clientIdentify:
				{
					uint32_t serviceId = callObj->getSourceId();
					
					if (serviceId == 0) {
						// This is 2 default service for the cient interface that manage the authorisation of view:
						if (callFunction == "getServiceCount") {
							// TODO : m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_clientServices.size());
							
							return;
						}
						if (callFunction == "getServiceList") {
							// TODO : m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_clientServices);
							//"ServiceManager/v0.1.0"
							return;
						}
						if (callFunction == "link") {
							// first param:
							std::string serviceName = callObj->getParameter<std::string>(0);
							ZEUS_ERROR("Connect to service : " << serviceName << " " << m_uid);
							// Check if service already link:
							/*
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
									m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "UN-AUTHORIZED-SERVICE");
									return;
								}
								ememory::SharedPtr<appl::ServiceInterface> srv = m_gatewayInterface->get(serviceName);
								if (srv != nullptr) {
									zeus::Future<bool> futLink = srv->m_interfaceClient.call(m_uid, ZEUS_ID_SERVICE_ROOT, "_new", m_userConnectionName, m_clientName, m_clientgroups);
									futLink.wait(); // TODO: Set timeout ...
									if (futLink.hasError() == true) {
										APPL_ERROR("Get error from the service ... LINK");
										m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "ERROR-CREATE-SERVICE-INSTANCE");
										return;
									}
									m_listConnectedService.push_back(srv);
									ZEUS_ERROR("      ==> get ID : " << m_uid);
									m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_listConnectedService.size());
									return;
								}
								m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "CAN-NOT-CONNECT-SERVICE");
								return;
							}
							m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "SERVICE-ALREADY-CONNECTED");;
							return;
							*/
						}
						if (callFunction == "unlink") {
							ZEUS_ERROR("Disconnnect from service : " << m_uid);
							// first param: the service we want to unconnect ...
							/*
							int64_t localServiceID = callObj->getParameter<int64_t>(0)-1;
							// Check if service already link:
							if (localServiceID >= m_listConnectedService.size()) {
								m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "NOT-CONNECTED-SERVICE");
								return;
							}
							zeus::Future<bool> futUnLink = m_listConnectedService[localServiceID]->m_interfaceClient.call(m_uid, ZEUS_ID_SERVICE_ROOT, "_delete");
							futUnLink.wait(); // TODO: Set timeout ...
							if (futUnLink.hasError() == true) {
								APPL_ERROR("Get error from the service ... UNLINK");
								m_interfaceRouterClient->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "ERROR-CREATE-SERVICE-INSTANCE");
								return;
							}
							m_listConnectedService[localServiceID] = nullptr;
							m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), true);
							return;
							*/
						}
						APPL_ERROR("Function does not exist ... '" << callFunction << "'");
						zeus::WebServer* iface = getInterface();
						iface->answerError(transactionId, _value->getDestination(), _value->getSource(), "CALL-UNEXISTING");
						return;
					}
				}
				return;
		}
	}
	if (m_gateway != nullptr) {
		m_gateway->send(_value);
	}
}



