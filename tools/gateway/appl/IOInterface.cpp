/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/IOInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const etk::String protocolError = "PROTOCOL-ERROR";

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
	// TODO : ... m_gateway->removeIO(sharedFromThis());
}

void appl::IOInterface::answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp) {
	zeus::WebServer* iface = getInterface();
	iface->answerError(_transactionId, 0, ZEUS_ID_SERVICE_ROOT, protocolError, _errorHelp);
	//m_interfaceRouterClient->sendCtrl(m_routeurUID, ZEUS_ID_SERVICE_ROOT, "DISCONNECT");
	APPL_TODO("Do this error return ... " << _errorHelp);
	m_state = appl::clientState::disconnect;
}

bool appl::IOInterface::start(appl::GateWay* _gateway, uint16_t _id, bool _directClientConnection) {
	m_gateway = _gateway;
	m_uid = _id;
	if (m_uid != 0) {
		m_state = appl::clientState::connect;
	} else {
		if (_directClientConnection == true) {
			m_state = appl::clientState::connect;
		} else {
			m_state = appl::clientState::connectDirect;
		}
	}
	//m_interfaceRouterClient->setInterfaceName("cli-" + etk::toString(m_uid));
	APPL_WARNING("[" << m_uid << "] New IO interface");
	return true;
}

const etk::Vector<etk::String>& appl::IOInterface::getServiceList() {
	return m_listService;
}

void appl::IOInterface::receive(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == null) {
		return;
	}
	APPL_INFO("RECEIVE message " << _value);
	uint32_t transactionId = _value->getTransactionId();
	if (transactionId == 0) {
		APPL_ERROR("Protocol error ==> missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	// Check if we are the destinated Of this message 
	if (    _value->getDestinationId() == ZEUS_ID_GATEWAY
	     && _value->getDestinationObjectId() == ZEUS_ID_GATEWAY_OBJECT) {
		if (_value->getType() != zeus::message::type::call) {
			APPL_ERROR("Protocol error ==>missing 'call'");
			answerProtocolError(transactionId, "missing parameter: 'call' / wrong type 'call'");
			return;
		}
		ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
		etk::String callFunction = callObj->getCall();
		if (callFunction == "removeRouterClient") {
			// TODO : Broadcast that an IO is remoed ...
			m_state = appl::clientState::unconnect;
			APPL_ERROR("Remote client disconnect ...");
			zeus::WebServer* iface = getInterface();
			iface->answerVoid(transactionId, _value->getDestination(), _value->getSource());
			return;
		}
		switch (m_state) {
			case appl::clientState::disconnect:
			case appl::clientState::unconnect:
				{
					APPL_ERROR("Must never appear");
					answerProtocolError(transactionId, "Gateway internal error");
					return;
				}
			case appl::clientState::connectDirect:
				{
					if (callFunction != "service") {
						answerProtocolError(transactionId, "Client must call: service");
						return;
					}
					if (callFunction == "service") {
						zeus::WebServer* iface = getInterface();
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
						m_gateway->addIO(sharedFromThis());
						m_state = appl::clientState::clientIdentify;
						return;
					}
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
					     && callFunction != "anonymous"
					     && callFunction != "link") {
						answerProtocolError(transactionId, "Client must call: identify/auth/anonymous/link");
						return;
					}
					#if 0
						if (callFunction == "identify") {
							etk::String clientName = callObj->getParameter<etk::String>(0);
							etk::String clientTocken = callObj->getParameter<etk::String>(1);
							if (m_userService == null) {
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
							etk::String password = callObj->getParameter<etk::String>(0);
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
						zeus::Future<etk::Vector<etk::String>> futGroup = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "clientGroupsGet", m_clientName);
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
						etk::Vector<etk::String> currentServices = m_gatewayInterface->getAllServiceName();
						zeus::Future<etk::Vector<etk::String>> futServices = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "filterClientServices", m_clientName, currentServices);
						futServices.wait(); // TODO: Set timeout ...
						if (futServices.hasError() == true) {
							APPL_ERROR("Get error from the service ...");
							m_interfaceRouterClient->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
							answerProtocolError(transactionId, "service filtering error");
							return;
						}
						m_clientServices = futServices.get();
						APPL_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
						APPL_WARNING("       groups: " << etk::toString(m_clientgroups));
						APPL_WARNING("     services: " << etk::toString(m_clientServices));
					#endif
					m_gateway->addIO(sharedFromThis());
					
					zeus::WebServer* iface = getInterface();
					iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
					m_state = appl::clientState::clientIdentify;
					return;
				}
				if (callFunction == "link") {
					// TODO : Filter services access ...
					etk::String serviceName = callObj->getParameter<etk::String>(0);
					if (m_gateway->serviceExist(serviceName) == false) {
						zeus::WebServer* iface = getInterface();
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
						return;
					}
					uint16_t serviceClientId = m_gateway->serviceClientIdGet(serviceName);
					if (serviceClientId == 0) {
						zeus::WebServer* iface = getInterface();
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
						return;
					}
					// Replace the destination with the real owner ID
					_value->setDestinationId(serviceClientId);
					// Forward Call
					if (m_gateway->send(_value) == false) {
						zeus::WebServer* iface = getInterface();
						iface->answerError(transactionId, _value->getDestination(), _value->getSource(), "UNEXISTING-CLIENT");
					}
					// We do not answer, we just transmit the message to the interface that manage the service that might answer ot this call ...
					return;
				}
				break;
			case appl::clientState::clientIdentify:
				{
					if (callFunction == "serviceAdd") {
						zeus::WebServer* iface = getInterface();
						etk::String serviceName = callObj->getParameter<etk::String>(0);
						if (serviceName == "") {
							iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
							return;
						}
						// Check if service exist or not:
						if (m_gateway->serviceExist(serviceName) == true) {
							iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
							return;
						}
						ZEUS_INFO("Register new service '" << serviceName << "' in " << m_uid);
						m_listService.pushBack(serviceName);
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
						return;
					}
					if (callFunction == "serviceRemove") {
						zeus::WebServer* iface = getInterface();
						etk::String serviceName = callObj->getParameter<etk::String>(0);
						if (serviceName == "") {
							iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
						}
						auto it = m_listService.begin();
						while (it != m_listService.end()) {
							if (*it == serviceName) {
								it = m_listService.erase(it);
								iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), true);
								ZEUS_INFO("Remove new service " << serviceName << "' in " << m_uid);
								return;
							}
							++it;
						}
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
						return;
					}
					// This is 2 default service for the cient interface that manage the authorisation of view:
					if (callFunction == "getServiceCount") {
						// TODO : Fileter the size of accessible services
						zeus::WebServer* iface = getInterface();
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), m_gateway->getAllServiceName().size());
						return;
					}
					if (callFunction == "getServiceList") {
						// TODO : Fileter the size of accessible services
						zeus::WebServer* iface = getInterface();
						iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), m_gateway->getAllServiceName());
						return;
					}
					if (callFunction == "link") {
						// TODO : Filter services access ...
						etk::String serviceName = callObj->getParameter<etk::String>(0);
						if (m_gateway->serviceExist(serviceName) == false) {
							zeus::WebServer* iface = getInterface();
							iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
							return;
						}
						uint16_t serviceClientId = m_gateway->serviceClientIdGet(serviceName);
						if (serviceClientId == 0) {
							zeus::WebServer* iface = getInterface();
							iface->answerValue(transactionId, _value->getDestination(), _value->getSource(), false);
							return;
						}
						// Replace the destination with the real owner ID
						_value->setDestinationId(serviceClientId);
						// Forward Call
						if (m_gateway->send(_value) == false) {
							zeus::WebServer* iface = getInterface();
							iface->answerError(transactionId, _value->getDestination(), _value->getSource(), "UNEXISTING-CLIENT");
						}
						// We do not answer, we just transmit the message to the interface that manage the service that might answer ot this call ...
						return;
					}
					APPL_ERROR("Function does not exist ... '" << callFunction << "'");
					zeus::WebServer* iface = getInterface();
					iface->answerError(transactionId, _value->getDestination(), _value->getSource(), "CALL-UNEXISTING");
					return;
				}
				return;
		}
		{
			APPL_ERROR("UNKNOW error ... " << _value);
			zeus::WebServer* iface = getInterface();
			iface->answerError(transactionId, _value->getDestination(), _value->getSource(), "REQUEST-ERROR");
		}
	} else {
		// TODO: Check here if the user is athorised to send data to a specific client ...
		if (m_gateway != null) {
			m_gateway->send(_value);
		}
	}
}



