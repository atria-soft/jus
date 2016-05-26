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

void jus::Client::onClientData(std::string _value) {
	ejson::Object obj(_value);
	jus::FutureBase future;
	{
		uint64_t tid = etk::string_to_uint64_t(obj["id"].toString().get());
		if (tid == 0) {
			JUS_ERROR("call with no ID ==> error ...");
			return;
		}
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
			it = m_pendingCall.erase(it);
			return;
		}
	}
	if (future.isValid() == false) {
		JUS_TODO("manage this event better ...");
		m_newData.push_back(std::move(_value));
		return;
	}
	// TODO : Call future ...
	
}

jus::ServiceRemote jus::Client::getService(const std::string& _name) {
	return jus::ServiceRemote(this, _name);
}

bool jus::Client::link(const std::string& _serviceName) {
	// TODO : Check the number of connection of this service ...
	bool ret = call_b("link", _serviceName);
	if (ret == false) {
		JUS_ERROR("Can not link with the service named: '" << _serviceName << "'");
	}
	return ret;
}

void jus::Client::unlink(const std::string& _serviceName) {
	bool ret = call_b("unlink", _serviceName);
	if (ret == false) {
		JUS_ERROR("Can not unlink with the service named: '" << _serviceName << "'");
	}
}

std::string jus::Client::asyncRead() {
	if (m_interfaceClient.isActive() == false) {
		return "";
	}
	int32_t iii = 5000;
	while (iii>0) {
		usleep(10000);
		if (m_newData.size() != 0) {
			break;
		}
		--iii;
	}
	if (iii == 0) {
		// Time-out ...
		return "";
	}
	std::string out;
	out = std::move(m_newData[0]);
	m_newData.erase(m_newData.begin());
	JUS_DEBUG("get async data: " << out);
	return out;
}

void jus::Client::onPropertyChangeIp() {
	disconnect();
}

void jus::Client::onPropertyChangePort(){
	disconnect();
}


void jus::Client::connect(const std::string& _remoteUserToConnect){
	disconnect();
	JUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient.setInterface(std::move(connection));
	m_interfaceClient.connect();
	bool ret = call_b("connectToUser", _remoteUserToConnect, "jus-client");
	if (ret == false) {
		JUS_ERROR("Connection error");
	}
	JUS_DEBUG("connect [STOP]");
}

void jus::Client::disconnect() {
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

uint64_t jus::Client::getId() {
	return m_id++;
}

ejson::Object jus::Client::callJson(const ejson::Object& _obj) {
	JUS_VERBOSE("Call JSON [START] ");
	if (m_interfaceClient.isActive() == false) {
		return ejson::Object();
	}
	JUS_DEBUG("Call JSON '" << _obj.generateHumanString() << "'");
	m_interfaceClient.write(_obj.generateMachineString());
	std::string ret = asyncRead();
	JUS_VERBOSE("Call JSON [STOP]");
	return ejson::Object(ret);
}


jus::FutureBase jus::Client::sendJson(uint64_t _transactionId, const ejson::Object& _obj) {
	JUS_VERBOSE("Send JSON [START] ");
	if (m_interfaceClient.isActive() == false) {
		ejson::Object obj;
		obj.add("error", ejson::String("NOT-CONNECTED"));
		obj.add("error-help", ejson::String("Client interface not connected (no TCP)"));
		return jus::FutureBase(_transactionId, true, obj);
	}
	jus::FutureBase tmpFuture(_transactionId);
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_pendingCall.push_back(tmpFuture);
	}
	JUS_DEBUG("Send JSON '" << _obj.generateHumanString() << "'");
	m_interfaceClient.write(_obj.generateMachineString());
	JUS_VERBOSE("Send JSON [STOP]");
	return tmpFuture;
}
namespace jus {
	template<>
	bool jus::Future<bool>::get() {
		if (m_data == nullptr) {
			return false;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return false;
		}
		if (val.isBoolean() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Boolean'");
			return false;
		}
		return val.toBoolean().get();
	}
}

