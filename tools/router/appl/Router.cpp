/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/Router.hpp>
#include <appl/debug.hpp>
#include <enet/TcpServer.hpp>


namespace appl {
	class TcpServerInput {
		private:
			enet::TcpServer m_interface;
			std::thread* m_thread;
			bool m_threadRunning;
			appl::Router* m_router;
			bool m_service;
		public:
			TcpServerInput(appl::Router* _router, bool _service) :
			  m_thread(nullptr),
			  m_threadRunning(false),
			  m_router(_router),
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
						m_router->newClientGateWay(std::move(data));
					} else {
						m_router->newClient(std::move(data));
					}
				}
			}
	};
}

void appl::Router::newClientGateWay(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (service)");
	ememory::SharedPtr<appl::GateWayInterface> tmp = ememory::makeShared<appl::GateWayInterface>(std::move(_connection), this);
	tmp->start();
	m_GateWayList.push_back(tmp);
}

void appl::Router::newClient(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (client)");
	ememory::SharedPtr<appl::ClientInterface> tmp = ememory::makeShared<appl::ClientInterface>(std::move(_connection), this);
	tmp->start();
	m_clientList.push_back(tmp);
}

appl::Router::Router() :
  m_clientUID(2),
  propertyClientIp(this, "client-ip", "127.0.0.1", "Ip to listen client", &appl::Router::onPropertyChangeClientIp),
  propertyClientPort(this, "client-port", 1983, "Port to listen client", &appl::Router::onPropertyChangeClientPort),
  propertyClientMax(this, "client-max", 8000, "Maximum of client at the same time", &appl::Router::onPropertyChangeClientMax),
  propertyGateWayIp(this, "gw-ip", "127.0.0.1", "Ip to listen Gateway", &appl::Router::onPropertyChangeGateWayIp),
  propertyGateWayPort(this, "gw-port", 1984, "Port to listen Gateway", &appl::Router::onPropertyChangeGateWayPort),
  propertyGateWayMax(this, "gw-max", 8000, "Maximum of Gateway at the same time", &appl::Router::onPropertyChangeGateWayMax) {
	m_interfaceClientServer = ememory::makeShared<appl::TcpServerInput>(this, false);
	m_interfaceGateWayServer = ememory::makeShared<appl::TcpServerInput>(this, true);
}

appl::Router::~Router() {
	
}

void appl::Router::start() {
	m_interfaceClientServer->start(*propertyClientIp, *propertyClientPort);
	m_interfaceGateWayServer->start(*propertyGateWayIp, *propertyGateWayPort);
}

void appl::Router::stop() {
	// TODO : Stop all server ...
	
}

ememory::SharedPtr<appl::GateWayInterface> appl::Router::get(const std::string& _userName) {
	// TODO : Start USer only when needed, not get it all time started...
	for (auto &it : m_GateWayList) {
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


std::vector<std::string> appl::Router::getAllUserName() {
	std::vector<std::string> out;
	/*
	for (auto &it : m_GateWayList) {
		if (it == nullptr) {
			continue;
		}
		out.push_back(it->getName());
	}
	*/
	return out;
}



void appl::Router::cleanIO() {
	
	auto it = m_GateWayList.begin();
	while (it != m_GateWayList.end()) {
		if (*it != nullptr) {
			if ((*it)->isAlive() == false) {
				it = m_GateWayList.erase(it);
				continue;
			}
		} else {
			it = m_GateWayList.erase(it);
			continue;
		}
		++it;
	}
	
	auto it2 = m_clientList.begin();
	while (it2 != m_clientList.end()) {
		if (*it2 != nullptr) {
			if ((*it2)->isAlive() == false) {
				(*it2)->stop();
				it2 = m_clientList.erase(it2);
				APPL_INFO("remove DONE ... ");
				continue;
			}
		} else {
			it2 = m_clientList.erase(it2);
			continue;
		}
		++it2;
	}
}

void appl::Router::onClientConnect(const bool& _value) {
	ZEUS_TODO("Client connection: " << _value);
}

void appl::Router::onServiceConnect(const bool& _value) {
	ZEUS_TODO("Service connection: " << _value);
}

void appl::Router::onPropertyChangeClientIp() {
	
}

void appl::Router::onPropertyChangeClientPort() {
	
}

void appl::Router::onPropertyChangeClientMax() {
	
}

void appl::Router::onPropertyChangeGateWayIp() {
	
}

void appl::Router::onPropertyChangeGateWayPort() {
	
}

void appl::Router::onPropertyChangeGateWayMax() {
	
}
