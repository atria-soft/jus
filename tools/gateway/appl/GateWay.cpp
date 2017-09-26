/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/GateWay.hpp>
#include <appl/debug.hpp>
#include <enet/TcpServer.hpp>
#include <appl/DirectInterface.hpp>
#include <appl/RouterInterface.hpp>
#include <etk/Exception.hpp>


namespace appl {
	class TcpServerInput {
		private:
			enet::TcpServer m_interface;
			ethread::Thread* m_thread;
			bool m_threadRunning;
			appl::GateWay* m_gateway;
		public:
			TcpServerInput(appl::GateWay* _gateway) :
			  m_thread(nullptr),
			  m_threadRunning(false),
			  m_gateway(_gateway) {
				
			}
			virtual ~TcpServerInput() {}
			void start(const etk::String& _host, uint16_t _port) {
				m_interface.setHostNane(_host);
				m_interface.setPort(_port);
				m_interface.link();
				m_threadRunning = true;
				APPL_INFO("Start waiting on " << _host << " " << _port);
				m_thread = new ethread::Thread([&](){ threadCallback();}, "GatewayListening");
				if (m_thread == nullptr) {
					m_threadRunning = false;
					APPL_ERROR("creating callback thread!");
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
					enet::Tcp data = etk::move(m_interface.waitNext());
					if (data.getConnectionStatus() != enet::Tcp::status::link) {
						APPL_CRITICAL("New TCP connection (DEAD ....) ==> gateway is dead ...");
						// TODO: Check interaface: if (m_interface.
						ethread::sleepMilliSeconds((300));
					}
					APPL_VERBOSE("New connection");
					m_gateway->newDirectInterface(etk::move(data));
				}
			}
	};
}

void appl::GateWay::newDirectInterface(enet::Tcp _connection) {
	APPL_WARNING("New TCP connection (service)");
	ememory::SharedPtr<appl::DirectInterface> tmp = ememory::makeShared<appl::DirectInterface>(etk::move(_connection));
	tmp->start(this);
	m_listTemporaryIO.pushBack(tmp);
}

appl::GateWay::GateWay() :
  m_idIncrement(10),
  propertyUserName(this, "user", "no-name", "User name of the interface"), // must be set befor start ...
  propertyRouterNo(this, "no-router", false, "No connection on the router"),
  propertyRouterIp(this, "router-ip", "127.0.0.1", "Ip to listen client", &appl::GateWay::onPropertyChangeClientIp),
  propertyRouterPort(this, "router-port", 1984, "Port to listen client", &appl::GateWay::onPropertyChangeClientPort),
  propertyServiceExtern(this, "service-extern", false, "enable extern service"),
  propertyServiceIp(this, "service-ip", "127.0.0.1", "Ip to listen client", &appl::GateWay::onPropertyChangeServiceIp),
  propertyServicePort(this, "service-port", 1985, "Port to listen client", &appl::GateWay::onPropertyChangeServicePort),
  propertyServiceMax(this, "service-max", 80, "Maximum of client at the same time", &appl::GateWay::onPropertyChangeServiceMax) {
	m_interfaceNewService = ememory::makeShared<appl::TcpServerInput>(this);
}

appl::GateWay::~GateWay() {
	
}

void appl::GateWay::addIO(const ememory::SharedPtr<appl::IOInterface>& _io) {
	m_listIO.pushBack(_io);
	// REMOVE of temporary element in the temporary list:
	auto it = m_listTemporaryIO.begin();
	while (it != m_listTemporaryIO.end()) {
		ememory::SharedPtr<appl::IOInterface> tmp = *it;
		if (tmp.get() == _io.get()) {
			it = m_listTemporaryIO.erase(it);
		} else {
			++it;
		}
	}
}

void appl::GateWay::removeIO(const ememory::SharedPtr<appl::IOInterface>& _io) {
	auto it = m_listIO.begin();
	while (it != m_listIO.end()) {
		if (it->get() == _io.get()) {
			it = m_listIO.erase(it);
		} else {
			++it;
		}
	}
}

uint16_t appl::GateWay::getId() {
	return m_idIncrement++;
}

void appl::GateWay::start() {
	if (*propertyRouterNo == false) {
		m_routerClient = ememory::makeShared<appl::RouterInterface>(*propertyRouterIp, *propertyRouterPort, *propertyUserName, this);
		if (m_routerClient->isAlive() == false) {
			APPL_ERROR("Can not connect the Router (if it is the normal case, use option '--no-router'");
			throw etk::exception::RuntimeError("Can not connect router");
		}
	}
	m_interfaceNewService->start(*propertyServiceIp, *propertyServicePort);
}

void appl::GateWay::stop() {
	m_routerClient.reset();
	m_listTemporaryIO.clear();
	m_listIO.clear();
}

bool appl::GateWay::serviceExist(const etk::String& _service) {
	for (auto &it : m_listIO) {
		if (it == nullptr) {
			continue;
		}
		if (it->isConnected() == false) {
			continue;
		}
		for (auto &srvIt : it->getServiceList()) {
			if (srvIt == _service) {
				return true;
			}
		}
	}
	return false;
}

uint16_t appl::GateWay::serviceClientIdGet(const etk::String& _service) {
	for (auto &it : m_listIO) {
		if (it == nullptr) {
			continue;
		}
		if (it->isConnected() == false) {
			continue;
		}
		for (auto &srvIt : it->getServiceList()) {
			if (srvIt == _service) {
				return it->getId();
			}
		}
	}
	return 0;
}


etk::Vector<etk::String> appl::GateWay::getAllServiceName() {
	etk::Vector<etk::String> out;
	// TODO : Change this it is old and deprecated ...
	for (auto &it : m_listIO) {
		if (it == nullptr) {
			continue;
		}
		if (it->isConnected() == false) {
			continue;
		}
		for (auto &srvIt : it->getServiceList()) {
			out.pushBack(srvIt);
		}
	}
	return out;
}


bool appl::GateWay::send(ememory::SharedPtr<zeus::Message> _data) {
	auto it = m_listIO.begin();
	uint16_t id = _data->getDestinationId();
	while (it != m_listIO.end()) {
		if (*it == nullptr) {
			continue;
		}
		if ((*it)->isConnected() == false) {
			continue;
		}
		if ((*it)->getId() == id) {
			(*it)->send(_data);
			return true;
		}
		++it;
	}
	return false;
}

void appl::GateWay::cleanIO() {
	APPL_VERBOSE("Check if something need to be clean ...");
	etk::Vector<uint16_t> tmpIDToRemove;
	// Clean all IOs...
	{
		auto it = m_listIO.begin();
		while (it != m_listIO.end()) {
			if (*it != nullptr) {
				if ((*it)->isConnected() == false) {
					tmpIDToRemove.pushBack((*it)->getId());
					it = m_listIO.erase(it);
					continue;
				}
			} else {
				it = m_listIO.erase(it);
				continue;
			}
			++it;
		}
	}
	// Clean all temporary connecting IO
	{
		auto it = m_listTemporaryIO.begin();
		while (it != m_listTemporaryIO.end()) {
			if (*it != nullptr) {
				if ((*it)->isConnected() == false) {
					it = m_listTemporaryIO.erase(it);
					continue;
				}
			} else {
				it = m_listTemporaryIO.erase(it);
				continue;
			}
			++it;
		}
	}
	// Clean router IO
	if (m_routerClient != nullptr) {
		m_routerClient->clean();
	}
	if (tmpIDToRemove.size() != 0) {
		APPL_TODO("Remove Ids ... " << tmpIDToRemove);
		for (auto &it : m_listIO) {
			if (it == nullptr) {
				continue;
			}
			if (it->isConnected() == false) {
				continue;
			}
			// send the message the interface has been removed
			zeus::WebServer* iface = it->getInterface();
			uint16_t id = it->getId();
			iface->call(ZEUS_ID_GATEWAY, uint32_t(id)<<16, "removeInterface", tmpIDToRemove);
			APPL_WARNING("Send it to :" << id << "  " << tmpIDToRemove);
		}
	}
	// Simply display All active objkect in all interfaces:
	for( auto &it : m_listIO) {
		if (it == nullptr) {
			continue;
		}
		zeus::WebServer* tmpp = it->getInterface();
		if (tmpp == nullptr) {
			continue;
		}
		tmpp->listObjects();
	}
	
}

void appl::GateWay::onClientConnect(const bool& _value) {
	APPL_TODO("Client connection: " << _value);
}

void appl::GateWay::onServiceConnect(const bool& _value) {
	APPL_TODO("Service connection: " << _value);
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

// Check if it take a long time without activity to kill itself ...
bool appl::GateWay::checkIsAlive(const echrono::Duration& _timeout) {
	// if no roueter, no delay to check
	if (*propertyRouterNo == true) {
		return true;
	}
	// If no router ==> dead
	if (m_routerClient == nullptr) {
		return false;
	}
	// check only for smallest time-out : 1 second.
	if (_timeout > echrono::seconds(1)) {
		echrono::Steady now = echrono::Steady::now();
		echrono::Steady lastTransmission = m_routerClient->getLastTransmission();
		if ((now - lastTransmission) >= _timeout) {
			APPL_INFO("Detect timeout ... last transmission=" << lastTransmission);
			return false;
		}
	}
	return true;
}