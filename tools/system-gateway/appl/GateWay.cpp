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
						m_gateway->newService(std::move(data));
					} else {
						m_gateway->newClient(std::move(data));
					}
				}
			}
	};
}

void appl::GateWay::newService(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (service)");
	ememory::SharedPtr<appl::ServiceInterface> tmp = ememory::makeShared<appl::ServiceInterface>(std::move(_connection), this);
	tmp->start();
	m_serviceList.push_back(tmp);
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
  propertyServiceIp(this, "service-ip", "127.0.0.1", "Ip to listen client", &appl::GateWay::onPropertyChangeServiceIp),
  propertyServicePort(this, "service-port", 1982, "Port to listen client", &appl::GateWay::onPropertyChangeServicePort),
  propertyServiceMax(this, "service-max", 80, "Maximum of client at the same time", &appl::GateWay::onPropertyChangeServiceMax) {
	m_interfaceClientServer = ememory::makeShared<appl::TcpServerInput>(this, false);
	m_interfaceServiceServer = ememory::makeShared<appl::TcpServerInput>(this, true);
}

appl::GateWay::~GateWay() {
	
}

void appl::GateWay::start() {
	m_interfaceClientServer->start(*propertyClientIp, *propertyClientPort);
	m_interfaceServiceServer->start(*propertyServiceIp, *propertyServicePort);
}

void appl::GateWay::stop() {
	
}

ememory::SharedPtr<appl::ServiceInterface> appl::GateWay::get(const std::string& _serviceName) {
	for (auto &it : m_serviceList) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _serviceName) {
			continue;
		}
		return it;
	}
	return nullptr;
}

std::vector<std::string> appl::GateWay::getAllServiceName() {
	std::vector<std::string> out;
	for (auto &it : m_serviceList) {
		if (it == nullptr) {
			continue;
		}
		out.push_back(it->getName());
	}
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
	
	auto it = m_serviceList.begin();
	while (it != m_serviceList.end()) {
		if (*it != nullptr) {
			if ((*it)->isAlive() == false) {
				it = m_serviceList.erase(it);
				continue;
			}
		} else {
			it = m_serviceList.erase(it);
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

void appl::GateWay::onPropertyChangeServiceIp() {
	
}

void appl::GateWay::onPropertyChangeServicePort() {
	
}

void appl::GateWay::onPropertyChangeServiceMax() {
	
}
