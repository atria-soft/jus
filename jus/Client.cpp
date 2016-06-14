/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/TcpClient.h>
#include <jus/Client.h>
#include <jus/debug.h>

#include <unistd.h>

jus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &jus::Client::onPropertyChangePort),
  m_id(1) {
	m_interfaceClient.connect(this, &jus::Client::onClientData);
}

jus::Client::~Client() {
	
}

void jus::Client::onClientData(jus::Buffer& _value) {
	JUS_DEBUG("Get answer RAW : "/* << _value*/);
	jus::FutureBase future;
	uint64_t tid = _value.getTransactionId();
	if (tid == 0) {
		JUS_ERROR("Get a Protocol error ... No ID ...");
		/*
		if (obj["error"].toString().get() == "PROTOCOL-ERROR") {
			JUS_ERROR("Get a Protocol error ...");
			std::unique_lock<std::mutex> lock(m_mutex);
			for (auto &it : m_pendingCall) {
				if (it.isValid() == false) {
					continue;
				}
				it.setAnswer(obj);
			}
			m_pendingCall.clear();
		} else {
			JUS_ERROR("call with no ID ==> error ...");
		}
		*/
		return;
	}
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_pendingCall.begin();
		while (it != m_pendingCall.end()) {
			if (it->isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->getTransactionId() != tid) {
				++it;
				continue;
			}
			future = *it;
			break;
		}
	}
	if (future.isValid() == false) {
		JUS_TODO("manage this event better ...");
		//m_newData.push_back(std::move(_value));
		return;
	}
	bool ret = future.setAnswer(_value);
	if (ret == true) {
		std::unique_lock<std::mutex> lock(m_mutex);
		auto it = m_pendingCall.begin();
		while (it != m_pendingCall.end()) {
			if (it->isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->getTransactionId() != tid) {
				++it;
				continue;
			}
			it = m_pendingCall.erase(it);
			break;
		}
	}
}

jus::ServiceRemote jus::Client::getService(const std::string& _name) {
	return jus::ServiceRemote(this, _name);
}

int32_t jus::Client::link(const std::string& _serviceName) {
	// TODO : Check the number of connection of this service ...
	jus::Future<int32_t> ret = call("link", _serviceName);
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not link with the service named: '" << _serviceName << "' ==> link error");
		return false;
	}
	return ret.get();
}

bool jus::Client::unlink(const uint32_t& _serviceId) {
	jus::Future<bool> ret = call("unlink", _serviceId);
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not unlink with the service id: '" << _serviceId << "' ==> link error");
		return false;
	}
	return ret.get();
}

void jus::Client::onPropertyChangeIp() {
	disconnect();
}

void jus::Client::onPropertyChangePort(){
	disconnect();
}


bool jus::Client::connect(const std::string& _remoteUserToConnect){
	disconnect();
	JUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient.setInterface(std::move(connection));
	m_interfaceClient.connect();
	// Force mode binary:
	JUS_WARNING("Request change in mode Binary");
	jus::Future<bool> retBin = call("setMode", "BIN").wait();
	if (retBin.get() == true) {
		JUS_WARNING("    ==> accepted binary");
		m_interfaceClient.setMode(jus::connectionMode::modeBinary);
		JUS_INFO("Connection jump in BINARY ...");
	} else {
		// stay in JSON
	}
	
	JUS_WARNING("Request connect user " << _remoteUserToConnect);
	jus::Future<bool> ret = call("connectToUser", _remoteUserToConnect, "jus-client");
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not connect to user named: '" << _remoteUserToConnect << "' ==> return error");
		return false;
	}
	if (ret.get() == true) {
		JUS_WARNING("    ==> accepted connection");
	} else {
		JUS_WARNING("    ==> Refuse connection");
	}
	return ret.get();
}

void jus::Client::disconnect() {
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

uint64_t jus::Client::getId() {
	return m_id++;
}



