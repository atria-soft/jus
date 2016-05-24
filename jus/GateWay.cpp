/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/GateWay.h>
#include <jus/debug.h>
#include <enet/TcpServer.h>


namespace jus {
	class TcpServerInput {
		private:
			enet::TcpServer m_interface;
			std::thread* m_thread;
			bool m_threadRunning;
			jus::GateWay* m_gateway;
			bool m_service;
		public:
			TcpServerInput(jus::GateWay* _gateway, bool _service) :
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
					JUS_ERROR("creating callback thread!");
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
					JUS_VERBOSE("New connection");
					if (m_service == true) {
						m_gateway->newService(std::move(data));
					} else {
						m_gateway->newClient(std::move(data));
					}
				}
			}
	};
}

void jus::GateWay::newService(enet::Tcp _connection) {
	JUS_WARNING("New TCP connection (service)");
	ememory::SharedPtr<jus::GateWayService> tmp = std::make_shared<jus::GateWayService>(std::move(_connection), this);
	tmp->start();
	m_serviceList.push_back(tmp);
}

void jus::GateWay::newClient(enet::Tcp _connection) {
	JUS_WARNING("New TCP connection (client)");
	ememory::SharedPtr<jus::GateWayClient> tmp = std::make_shared<jus::GateWayClient>(std::move(_connection), this);
	tmp->start(m_clientUID++);
	m_clientList.push_back(tmp);
}

jus::GateWay::GateWay() :
  m_clientUID(1),
  propertyClientIp(this, "client-ip", "127.0.0.1", "Ip to listen client", &jus::GateWay::onPropertyChangeClientIp),
  propertyClientPort(this, "client-port", 1983, "Port to listen client", &jus::GateWay::onPropertyChangeClientPort),
  propertyClientMax(this, "client-max", 80, "Maximum of client at the same time", &jus::GateWay::onPropertyChangeClientMax),
  propertyServiceIp(this, "service-ip", "127.0.0.1", "Ip to listen client", &jus::GateWay::onPropertyChangeServiceIp),
  propertyServicePort(this, "service-port", 1982, "Port to listen client", &jus::GateWay::onPropertyChangeServicePort),
  propertyServiceMax(this, "service-max", 80, "Maximum of client at the same time", &jus::GateWay::onPropertyChangeServiceMax) {
	m_interfaceClientServer = std::make_shared<jus::TcpServerInput>(this, false);
	m_interfaceServiceServer = std::make_shared<jus::TcpServerInput>(this, true);
}

jus::GateWay::~GateWay() {
	
}

void jus::GateWay::start() {
	m_interfaceClientServer->start(*propertyClientIp, *propertyClientPort);
	m_interfaceServiceServer->start(*propertyServiceIp, *propertyServicePort);
}

void jus::GateWay::stop() {
	
}

ememory::SharedPtr<jus::GateWayService> jus::GateWay::get(const std::string& _serviceName) {
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

void jus::GateWay::answer(uint64_t _userSessionId, ejson::Object _data) {
	for (auto &it : m_clientList) {
		if (it == nullptr) {
			continue;
		}
		if (it->getId() != _userSessionId) {
			continue;
		}
		it->returnMessage(_data);
		return;
	}
}

void jus::GateWay::cleanIO() {
	
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

void jus::GateWay::onClientConnect(const bool& _value) {
	JUS_TODO("Client connection: " << _value);
}

void jus::GateWay::onServiceConnect(const bool& _value) {
	JUS_TODO("Service connection: " << _value);
}

void jus::GateWay::onPropertyChangeClientIp() {
	
}

void jus::GateWay::onPropertyChangeClientPort() {
	
}

void jus::GateWay::onPropertyChangeClientMax() {
	
}

void jus::GateWay::onPropertyChangeServiceIp() {
	
}

void jus::GateWay::onPropertyChangeServicePort() {
	
}

void jus::GateWay::onPropertyChangeServiceMax() {
	
}
