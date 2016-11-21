/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/GateWay.hpp>
#include <appl/debug.hpp>
#include <enet/TcpServer.hpp>


namespace appl {
	class TcpServerInput {
		private:
			enet::TcpServer m_interface;
			std::thread* m_thread;
			bool m_threadRunning;
			appl::GateWay* m_gateway;
			bool m_service;
		public:
			TcpServerInput(appl::GateWay* _gateway, bool _service) :
			  m_thread(nullptr),
			  m_threadRunning(false),
			  m_gateway(_gateway),
			  m_service(_service) {
				
			}
			virtual ~TcpServerInput() {}
			void start(const std::string& _host, uint16_t _port) {
				m_interface.setHostNane(_host);
				m_interface.setPort(_port);
				m_interface.link();
				m_threadRunning = true;
				m_thread = new std::thread([&](void *){ this->threadCallback();}, nullptr);
				if (m_thread == nullptr) {
					m_threadRunning = false;
					ZEUS_ERROR("creating callback thread!");
					return;
				}
			}
			void stop() {
				if (m_thread != nullptr) {
					m_threadRunning = false;
				}
				m_interface.unlink();
				if (m_thread != nullptr) {
					m_thread->join();
					delete m_thread;
					m_thread = nullptr;
				}
			}
			void threadCallback() {
				// get datas:
				while (m_threadRunning == true) {
					// READ section data:
					enet::Tcp data = std::move(m_interface.waitNext());
					ZEUS_VERBOSE("New connection");
					if (m_service == true) {
						m_gateway->newClientGateWayBackEnd(std::move(data));
					} else {
						m_gateway->newClient(std::move(data));
					}
				}
			}
	};
}

void appl::GateWay::newClientGateWayBackEnd(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (service)");
	ememory::SharedPtr<appl::GateWayInterface> tmp = ememory::makeShared<appl::GateWayInterface>(std::move(_connection), this);
	tmp->start();
	m_gatewayBackEndList.push_back(tmp);
}

void appl::GateWay::newClient(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (client)");
	ememory::SharedPtr<appl::ClientInterface> tmp = ememory::makeShared<appl::ClientInterface>(std::move(_connection), this);
	tmp->start(m_clientUID++, m_clientUID++);
	m_clientList.push_back(tmp);
}

appl::GateWay::GateWay() :
  m_clientUID(1),
  propertyClientIp(this, "client-ip", "127.0.0.1", "Ip to listen client", &appl::GateWay::onPropertyChangeClientIp),
  propertyClientPort(this, "client-port", 1983, "Port to listen client", &appl::GateWay::onPropertyChangeClientPort),
  propertyClientMax(this, "client-max", 80, "Maximum of client at the same time", &appl::GateWay::onPropertyChangeClientMax),
  propertyGatewayBackEndIp(this, "gw-ip", "127.0.0.1", "Ip to listen client", &appl::GateWay::onPropertyChangeGateWayIp),
  propertyGatewayBackEndPort(this, "gw-port", 1984, "Port to listen client", &appl::GateWay::onPropertyChangeGateWayPort),
  propertyGatewayBackEndMax(this, "gw-max", 80, "Maximum of client at the same time", &appl::GateWay::onPropertyChangeGateWayMax) {
	m_interfaceClientServer = ememory::makeShared<appl::TcpServerInput>(this, false);
	m_interfaceGatewayBackEndServer = ememory::makeShared<appl::TcpServerInput>(this, true);
}

appl::GateWay::~GateWay() {
	
}

void appl::GateWay::start() {
	m_interfaceClientServer->start(*propertyClientIp, *propertyClientPort);
	m_interfaceGatewayBackEndServer->start(*propertyGatewayBackEndIp, *propertyGatewayBackEndPort);
}

void appl::GateWay::stop() {
	// TODO : Stop all server ...
	
}

ememory::SharedPtr<appl::GateWayInterface> appl::GateWay::get(const std::string& _userName) {
	// TODO : Start USer only when needed, not get it all time started...
	for (auto &it : m_gatewayBackEndList) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _userName) {
			continue;
		}
		return it;
	}
	return nullptr;
}


std::vector<std::string> appl::GateWay::getAllUserName() {
	std::vector<std::string> out;
	/*
	for (auto &it : m_gatewayBackEndList) {
		if (it == nullptr) {
			continue;
		}
		out.push_back(it->getName());
	}
	*/
	return out;
}


void appl::GateWay::answer(uint64_t _userSessionId, const ememory::SharedPtr<zeus::Buffer>& _data) {
	for (auto &it : m_clientList) {
		if (it == nullptr) {
			continue;
		}
		if (it->checkId(_userSessionId) == false) {
			continue;
		}
		it->returnMessage(_data);
		return;
	}
}

void appl::GateWay::cleanIO() {
	
	auto it = m_gatewayBackEndList.begin();
	while (it != m_gatewayBackEndList.end()) {
		if (*it != nullptr) {
			if ((*it)->isAlive() == false) {
				it = m_gatewayBackEndList.erase(it);
				continue;
			}
		} else {
			it = m_gatewayBackEndList.erase(it);
			continue;
		}
		++it;
	}
	
	auto it2 = m_clientList.begin();
	while (it2 != m_clientList.end()) {
		if (*it2 != nullptr) {
			if ((*it2)->isAlive() == false) {
				it2 = m_clientList.erase(it2);
				continue;
			}
		} else {
			it2 = m_clientList.erase(it2);
			continue;
		}
		++it2;
	}
}

void appl::GateWay::onClientConnect(const bool& _value) {
	ZEUS_TODO("Client connection: " << _value);
}

void appl::GateWay::onServiceConnect(const bool& _value) {
	ZEUS_TODO("Service connection: " << _value);
}

void appl::GateWay::onPropertyChangeClientIp() {
	
}

void appl::GateWay::onPropertyChangeClientPort() {
	
}

void appl::GateWay::onPropertyChangeClientMax() {
	
}

void appl::GateWay::onPropertyChangeGateWayIp() {
	
}

void appl::GateWay::onPropertyChangeGateWayPort() {
	
}

void appl::GateWay::onPropertyChangeGateWayMax() {
	
}
