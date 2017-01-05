/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/RouterInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>
#include <enet/TcpClient.hpp>


#include <zeus/AbstractFunction.hpp>

static const std::string protocolError = "PROTOCOL-ERROR";

appl::clientSpecificInterface::clientSpecificInterface() {
	m_uid = 0;
	m_state = appl::clientState::unconnect;
	APPL_INFO("-------------------------------");
	APPL_INFO("-- NEW Client (threw router) --");
	APPL_INFO("-------------------------------");
}

appl::clientSpecificInterface::~clientSpecificInterface() {
	APPL_INFO("----------------------------------");
	APPL_INFO("-- DELETE Client (threw router) --");
	APPL_INFO("----------------------------------");
}

zeus::WebServer* appl::clientSpecificInterface::getInterface() {
	return m_interfaceWeb;
}

bool appl::clientSpecificInterface::isConnected() {
	if (m_interfaceWeb == nullptr) {
		return false;
	}
	if (m_state == appl::clientState::unconnect) {
		return false;
	}
	return m_interfaceWeb->isActive();
};

bool appl::clientSpecificInterface::start(appl::GateWay* _gateway, zeus::WebServer* _interfaceWeb, uint16_t _id) {
	appl::IOInterface::start(_gateway, _id);
	m_interfaceWeb = _interfaceWeb;
	APPL_WARNING("[" << m_uid << "] New client : " << m_clientName);
	return true;
}

void appl::clientSpecificInterface::send(ememory::SharedPtr<zeus::Message> _value) {
	m_interfaceWeb->writeBinary(_value);
}

#if 0
void appl::clientSpecificInterface::receive(ememory::SharedPtr<zeus::Message> _value) {
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
	if (_value->getType() == zeus::Message::type::data) {
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
			m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "NOT-CONNECTED-SERVICE");
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
	if (_value->getType() != zeus::Message::type::call) {
		APPL_ERROR("Protocol error ==>missing 'call'");
		answerProtocolError(transactionId, "missing parameter: 'call' / wrong type 'call'");
		return;
	}
	ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
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
					
					zeus::Future<bool> fut = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "checkTocken", clientName, clientTocken);
					fut.wait(); // TODO: Set timeout ...
					if (fut.hasError() == true) {
						APPL_ERROR("Get error from the service ...");
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
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
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
						answerProtocolError(transactionId, "connection refused 1");
						return;
					} else if (fut.get() == false) {
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
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
				zeus::Future<std::vector<std::string>> futGroup = m_userService->m_interfaceClient.call(m_localIdUser, ZEUS_ID_SERVICE_ROOT, "clientGroupsGet", m_clientName);
				futGroup.wait(); // TODO: Set timeout ...
				if (futGroup.hasError() == true) {
					APPL_ERROR("Get error from the service ...");
					m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
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
					m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), false);
					answerProtocolError(transactionId, "service filtering error");
					return;
				}
				m_clientServices = futServices.get();
				APPL_WARNING("Connection of: '" << m_clientName << "' to '" << m_userConnectionName << "'");
				APPL_WARNING("       groups: " << etk::to_string(m_clientgroups));
				APPL_WARNING("     services: " << etk::to_string(m_clientServices));
				
				
				m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), true);
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
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_clientServices.size());
						return;
					}
					if (callFunction == "getServiceList") {
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_clientServices);
						//"ServiceManager/v0.1.0"
						return;
					}
					if (callFunction == "link") {
						// first param:
						std::string serviceName = callObj->getParameter<std::string>(0);
						ZEUS_ERROR("Connect to service : " << serviceName << " " << m_uid);
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
								m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "UN-AUTHORIZED-SERVICE");
								return;
							}
							ememory::SharedPtr<appl::ServiceInterface> srv = m_gatewayInterface->get(serviceName);
							if (srv != nullptr) {
								zeus::Future<bool> futLink = srv->m_interfaceClient.call(m_uid, ZEUS_ID_SERVICE_ROOT, "_new", m_userConnectionName, m_clientName, m_clientgroups);
								futLink.wait(); // TODO: Set timeout ...
								if (futLink.hasError() == true) {
									APPL_ERROR("Get error from the service ... LINK");
									m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "ERROR-CREATE-SERVICE-INSTANCE");
									return;
								}
								m_listConnectedService.push_back(srv);
								ZEUS_ERROR("      ==> get ID : " << m_uid);
								m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), m_listConnectedService.size());
								return;
							}
							m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "CAN-NOT-CONNECT-SERVICE");
							return;
						}
						m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "SERVICE-ALREADY-CONNECTED");;
						return;
					}
					if (callFunction == "unlink") {
						ZEUS_ERROR("Disconnnect from service : " << m_uid);
						// first param: the service we want to unconnect ...
						int64_t localServiceID = callObj->getParameter<int64_t>(0)-1;
						// Check if service already link:
						if (localServiceID >= m_listConnectedService.size()) {
							m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "NOT-CONNECTED-SERVICE");
							return;
						}
						zeus::Future<bool> futUnLink = m_listConnectedService[localServiceID]->m_interfaceClient.call(m_uid, ZEUS_ID_SERVICE_ROOT, "_delete");
						futUnLink.wait(); // TODO: Set timeout ...
						if (futUnLink.hasError() == true) {
							APPL_ERROR("Get error from the service ... UNLINK");
							m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "ERROR-CREATE-SERVICE-INSTANCE");
							return;
						}
						m_listConnectedService[localServiceID] = nullptr;
						m_interfaceWeb->answerValue(transactionId, _value->getClientId(), _value->getServiceId(), true);
						return;
					}
					APPL_ERROR("Function does not exist ... '" << callFunction << "'");
					m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "CALL-UNEXISTING");
					return;
				}
				// decrease service ID ...
				serviceId -= 1;
				if (serviceId >= m_listConnectedService.size()) {
					m_interfaceWeb->answerError(transactionId, _value->getClientId(), _value->getServiceId(), "NOT-CONNECTED-SERVICE");
					return;
				} else {
					if (m_listConnectedService[serviceId] == nullptr) {
						// TODO ...
						APPL_ERROR("TODO : Manage this case ...");
						return;
					}
					auto fut = m_listConnectedService[serviceId]->m_interfaceClient.callForward(m_uid, _value, (uint64_t(m_uid) << 32) + uint64_t(transactionId));
					fut.andAll([=](zeus::FutureBase _ret) {
					           		ememory::SharedPtr<zeus::Message> tmpp = _ret.getRaw();
					           		if (tmpp == nullptr) {
					           			return true;
					           		}
					           		APPL_DEBUG("    ==> transmit : " << tmpp->getTransactionId() << " -> " << transactionId);
					           		APPL_DEBUG("    msg=" << tmpp);
					           		tmpp->setTransactionId(transactionId);
					           		tmpp->setClientId(m_routeurUID);
					           		tmpp->setServiceId(serviceId+1);
					           		APPL_DEBUG("transmit=" << tmpp);
					           		m_interfaceWeb->writeBinary(tmpp);
					           		// multiple send element ...
					           		return tmpp->getPartFinish();
					           });
				}
			}
	}
}
#endif


appl::RouterInterface::RouterInterface(const std::string& _ip, uint16_t _port, std::string _userName, appl::GateWay* _gateway) :
  m_state(appl::clientState::unconnect),
  m_gateway(_gateway),
  m_interfaceWeb() {
	APPL_INFO("----------------------------------------");
	APPL_INFO("-- NEW Connection to the ROUTER       --");
	APPL_INFO("----------------------------------------");
	enet::Tcp connection = std::move(enet::connectTcpClient(_ip, _port));
	if (connection.getConnectionStatus() != enet::Tcp::status::link) {
		APPL_ERROR("Can not connect the GateWay-front-end");
		return;
	}
	m_interfaceWeb.setInterface(std::move(connection), false, _userName);
	m_interfaceWeb.connect(this, &appl::RouterInterface::onClientData);
	m_interfaceWeb.connect(true);
	m_interfaceWeb.setInterfaceName("cli-GW-to-router");
	// TODO : Check if user name is accepted ...
}

appl::RouterInterface::~RouterInterface() {
	APPL_TODO("Call All unlink ...");
	stop();
	APPL_INFO("-------------------------------------------");
	APPL_INFO("-- DELETE Connection to the ROUTER       --");
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
	m_interfaceWeb.disconnect();
}

bool appl::RouterInterface::isAlive() {
	return m_interfaceWeb.isActive();
}



void appl::RouterInterface::onClientData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	// Get client ID:
	uint16_t sourceId = _value->getSourceId();
	//APPL_ERROR("[" << clientId << "] get message from front-end gateWay: " << _value);
	for (auto &it : m_listClients) {
		if (it->checkId(sourceId) == true) {
			it->receive(_value);
			return;
		}
	}
	m_listClients.push_back(ememory::makeShared<clientSpecificInterface>());
	size_t localId = m_listClients.size()-1;
	bool ret = m_listClients[localId]->start(m_gateway, &m_interfaceWeb, sourceId);
	if (ret == false) {
		return;
	}
	m_listClients[localId]->receive(_value);
}

void appl::RouterInterface::send(const ememory::SharedPtr<zeus::Message>& _data) {
	m_interfaceWeb.writeBinary(_data);
}

void appl::RouterInterface::clean() {
	auto it = m_listClients.begin();
	while (it != m_listClients.end()) {
		if ((*it)->m_state == appl::clientState::disconnect) {
			it = m_listClients.erase(it);
			continue;
		}
		++it;
	}
}

echrono::Steady appl::RouterInterface::getLastTransmission() {
	auto receive = m_interfaceWeb.getLastTimeReceive();
	auto send = m_interfaceWeb.getLastTimeSend();
	if (receive >= send) {
		if (receive == 0) {
			return echrono::Steady::now();
		}
		return receive;
	}
	if (send == 0) {
		return echrono::Steady::now();
	}
	return send;
}

