/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/Router.hpp>
#include <appl/debug.hpp>
#include <etk/path/fileSystem.hpp>
#include <enet/TcpServer.hpp>

static etk::Uri g_pathDBName = "USER_DATA:///router-database.json";

class UserAvaillable {
	public:
		etk::String m_name;
		etk::String m_basePath;
		bool m_accessMediaCenter;
		bool m_enableDirectAccess;
		FILE* m_subProcess;
};
etk::Vector<UserAvaillable> g_listUserAvaillable;
bool g_needToStore = false;

static void store_db() {
	if (g_needToStore == false) {
		return;
	}
	APPL_ERROR("Store database [START]");
	ejson::Document database;
	ejson::Array listUserArray;
	database.add("users", listUserArray);
	for (auto &it : g_listUserAvaillable) {
		ejson::Object propObject;
		listUserArray.add(propObject);
		propObject.add("name", ejson::String(it.m_name));
		propObject.add("path", ejson::String(it.m_basePath));
		propObject.add("access-media-center", ejson::Boolean(it.m_accessMediaCenter));
		propObject.add("access-direct", ejson::Boolean(it.m_enableDirectAccess));
	}
	bool retGenerate = database.storeSafe(g_pathDBName);
	APPL_ERROR("Store database [STOP] : " << g_pathDBName << " ret = " << retGenerate);
	g_needToStore = false;
}

static void load_db() {
	ejson::Document database;
	bool ret = database.load(g_pathDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	g_listUserAvaillable.clear();
	ejson::Array listUserArray = database["users"].toArray();
	for (const auto itArray: listUserArray) {
		ejson::Object userElement = itArray.toObject();
		UserAvaillable userProperty;
		userProperty.m_name = userElement["name"].toString().get();
		userProperty.m_basePath = userElement["path"].toString().get();
		userProperty.m_accessMediaCenter = userElement["access-media-center"].toBoolean().get();
		userProperty.m_enableDirectAccess = userElement["access-direct"].toBoolean().get();
		APPL_INFO("find USER: '" << userProperty.m_name << "'");
		g_listUserAvaillable.pushBack(userProperty);
	}
	g_needToStore = false;
}


namespace appl {
	class TcpServerInput {
		private:
			enet::TcpServer m_interface;
			ethread::Thread* m_thread;
			bool m_threadRunning;
			appl::Router* m_router;
			bool m_service;
		public:
			TcpServerInput(appl::Router* _router, bool _service) :
			  m_thread(null),
			  m_threadRunning(false),
			  m_router(_router),
			  m_service(_service) {
				
			}
			virtual ~TcpServerInput() {}
			void start(const etk::String& _host, uint16_t _port) {
				m_interface.setHostNane(_host);
				m_interface.setPort(_port);
				m_interface.link();
				m_threadRunning = true;
				m_thread = ETK_NEW(ethread::Thread, [&](){ threadCallback();}, "routerListener");
				if (m_thread == null) {
					m_threadRunning = false;
					ZEUS_ERROR("creating callback thread!");
					return;
				}
			}
			void stop() {
				if (m_thread != null) {
					m_threadRunning = false;
				}
				m_interface.unlink();
				if (m_thread != null) {
					m_thread->join();
					ETK_DELETE(ethread::Thread, m_thread);
					m_thread = null;
				}
			}
			void threadCallback() {
				// get datas:
				while (m_threadRunning == true) {
					// READ section data:
					enet::Tcp data = etk::move(m_interface.waitNext());
					ZEUS_VERBOSE("New connection");
					if (m_service == true) {
						m_router->newClientGateWay(etk::move(data));
					} else {
						m_router->newClient(etk::move(data));
					}
				}
			}
	};
}

void appl::Router::newClientGateWay(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (service)");
	ememory::SharedPtr<appl::GateWayInterface> tmp = ememory::makeShared<appl::GateWayInterface>(etk::move(_connection), this);
	tmp->start();
	m_GateWayList.pushBack(tmp);
}

void appl::Router::newClient(enet::Tcp _connection) {
	ZEUS_WARNING("New TCP connection (client)");
	ememory::SharedPtr<appl::ClientInterface> tmp = ememory::makeShared<appl::ClientInterface>(etk::move(_connection), this);
	tmp->start();
	m_clientList.pushBack(tmp);
}

appl::Router::Router() :
  m_clientUID(2),
  propertyStdOut(this, "stdout", false, "Set the Log of the output"),
  propertyClientIp(this, "client-ip", "127.0.0.1", "Ip to listen client", &appl::Router::onPropertyChangeClientIp),
  propertyClientPort(this, "client-port", 1983, "Port to listen client", &appl::Router::onPropertyChangeClientPort),
  propertyClientMax(this, "client-max", 8000, "Maximum of client at the same time", &appl::Router::onPropertyChangeClientMax),
  propertyGateWayIp(this, "gw-ip", "127.0.0.1", "Ip to listen Gateway", &appl::Router::onPropertyChangeGateWayIp),
  propertyGateWayPort(this, "gw-port", 1984, "Port to listen Gateway", &appl::Router::onPropertyChangeGateWayPort),
  propertyGateWayMax(this, "gw-max", 8000, "Maximum of Gateway at the same time", &appl::Router::onPropertyChangeGateWayMax),
  propertyDelayToStop(this, "delay-to-stop", 0, "Delay before the client stop the connection in second (default: 0=automatic set by the gateway; -1=never disconnect; other the time )"),
  propertyGateWayDirectPortMin(this, "gw-direct-port-min", 12000, "Minimum of Gateway at the same time"),
  propertyGateWayDirectPortMax(this, "gw-direct-port-max", 13000, "Maximum of Gateway at the same time") {
	m_interfaceClientServer = ememory::makeShared<appl::TcpServerInput>(this, false);
	m_interfaceGateWayServer = ememory::makeShared<appl::TcpServerInput>(this, true);
	load_db();
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

bool appl::Router::userIsConnected(const etk::String& _userName) {
	for (auto &it : m_GateWayList) {
		if (it == null) {
			continue;
		}
		if (it->getName() != _userName) {
			continue;
		}
		return true;
	}
	return false;
}

extern "C" {
	#include <stdio.h>
	#include <unistd.h>
}

ememory::SharedPtr<appl::GateWayInterface> appl::Router::get(const etk::String& _userName) {
	// TODO : Start USer only when needed, not get it all time started...
	APPL_ERROR("Get user : " << _userName);
	for (auto &it : m_GateWayList) {
		if (it == null) {
			continue;
		}
		if (it->getName() != _userName) {
			continue;
		}
		APPL_ERROR("    ==> find IT ... already started...");
		return it;
	}
	APPL_ERROR("    ==> Not found ==> start it ...");
	// we not find the user ==> check if it is availlable ...
	for (auto &it : g_listUserAvaillable) {
		APPL_ERROR("    Check: " << it.m_name << " != " << _userName);
		if (it.m_name != _userName) {
			continue;
		}
		APPL_ERROR("        ==> find ...");
		#if 0
			// start interface:
			etk::String cmd = "~/dev/perso/out/Linux_x86_64/debug/staging/clang/zeus-package-base/zeus-package-base.app/bin/zeus-gateway";
			cmd += " --user=" + it.m_name + " ";
			cmd += " --srv=user";
			cmd += " --srv=picture";
			cmd += " --srv=video";
			cmd += " --base-path=" + it.m_basePath;
			cmd += " --elog-file=\"/tmp/zeus.gateway." + it.m_name + ".log\"";
			cmd += "&";
			APPL_ERROR("Start " << cmd);
			it.m_subProcess = popen(cmd.c_str(), "r");
			if (it.m_subProcess == null) {
				perror("popen");
				return null;
			}
			// just trash IO ...
			//pclose(it.m_subProcess);
		#else
			pid_t pid = fork();
			if (pid > 0) {
				// We're in the parent here.
				// nothing to do ...
				APPL_ERROR("Parent Execution ...");
			} else if (pid == 0) {
				// We're in the child here.
				APPL_ERROR("Child Execution ...");
				etk::Path binary = etk::path::getBinaryDirectory() / "zeus-gateway";
				etk::String userConf = "--user=" + it.m_name;
				etk::String basePath = "--base-path=" + it.m_basePath;
				etk::String logFile;
				if (*propertyStdOut == false) {
					etk::Path tmp = etk::Path(it.m_basePath) / "log" / "gateway.log";
					if (    tmp.isEmpty() == false
					     && tmp.getString()[0] == '~') {
						tmp = etk::path::getHomePath() / &logFile[1];
					}
					logFile = "--elog-file=" + tmp.getString();
					//etk::String logFile = "--elog-file=/home/heero/.local/share/zeus-DATA/SDFGHTHBSDFGSQDHZSRDFGSDFGSDFGSDFG/log/gateway.log";
					//etk::String logFile = " ";
					APPL_INFO("New Child log in = " << logFile);
				}
				etk::String directAccess;
				if (it.m_enableDirectAccess == true) {
					directAccess = "--direct-ip=" + *propertyClientIp;
					directAccess += " --direct-port=" + etk::toString(*propertyGateWayDirectPortMin);
				}
				etk::String delay = "--router-delay=" + etk::toString(*propertyDelayToStop);
				//etk::String delay = "--router-delay=-1";
				APPL_INFO("execute: " << binary << " " << userConf << " --srv=all " << delay << " " << basePath << " " << logFile << " " << directAccess);
				int ret = execlp( binary.getNative().c_str(),
				                  binary.getNative().c_str(), // must repeate the binary name to have the name as first argument ...
				                  userConf.c_str(),
				                  "--srv=all",
				                  /*"--service-extern=false",*/
				                  delay.c_str(),
				                  basePath.c_str(),
				                  logFile.c_str(),
				                  directAccess.c_str(),
				                  NULL);
				APPL_ERROR("Child Execution ret = " << ret);
				exit (-1);
				APPL_ERROR("Must never appear ... child of fork killed ...");
			} else {
				// fork failed
				APPL_ERROR("fork() failed!\n");
			}
		#endif
		int32_t nbCheckDelayMax = 24;
		while (nbCheckDelayMax-- > 0) {
			ethread::sleepMilliSeconds((25));
			for (auto &it : m_GateWayList) {
				if (it == null) {
					continue;
				}
				if (it->getName() != _userName) {
					continue;
				}
				return it;
			}
		}
		APPL_ERROR("must be connected ==> and it is not ...");
		break;
	}
	return null;
}


etk::Vector<etk::String> appl::Router::getAllUserName() {
	etk::Vector<etk::String> out;
	/*
	for (auto &it : m_GateWayList) {
		if (it == null) {
			continue;
		}
		out.pushBack(it->getName());
	}
	*/
	return out;
}



void appl::Router::cleanIO() {
	store_db();
	auto it = m_GateWayList.begin();
	while (it != m_GateWayList.end()) {
		if (*it != null) {
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
		if (*it2 != null) {
			if ((*it2)->isAlive() == false) {
				(*it2)->stop();
				APPL_ERROR("count = " << (*it2).useCount() << " list.size()=" << m_clientList.size());
				it2 = m_clientList.erase(it2);
				APPL_ERROR("             list.size()=" << m_clientList.size());
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
