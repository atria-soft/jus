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
	JUS_DEBUG("Get answer : " << _value);
	ejson::Object obj(_value);
	jus::FutureBase future;
	uint64_t tid = obj["id"].toNumber().get();
	if (tid == 0) {
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
	bool ret = future.setAnswer(obj);
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

bool jus::Client::link(const std::string& _serviceName) {
	// TODO : Check the number of connection of this service ...
	jus::Future<bool> ret = call("link", _serviceName);
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not link with the service named: '" << _serviceName << "' ==> link error");
		return false;
	}
	return ret.get();
}

bool jus::Client::unlink(const std::string& _serviceName) {
	jus::Future<bool> ret = call("unlink", _serviceName);
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not unlink with the service named: '" << _serviceName << "' ==> link error");
		return false;
	}
	return ret.get();
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


bool jus::Client::connect(const std::string& _remoteUserToConnect){
	disconnect();
	JUS_DEBUG("connect [START]");
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient.setInterface(std::move(connection));
	m_interfaceClient.connect();
	jus::Future<bool> ret = call("connectToUser",  _remoteUserToConnect, "jus-client");
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not connect to user named: '" << _remoteUserToConnect << "' ==> return error");
		return false;
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

class SendAsync {
	private:
		std::vector<jus::ActionAsyncClient> m_async;
		uint64_t m_transactionId;
		std::string m_service;
		uint32_t m_partId;
	public:
		SendAsync(uint64_t _transactionId, const std::string& _service, const std::vector<jus::ActionAsyncClient>& _async) :
		  m_async(_async),
		  m_transactionId(_transactionId),
		  m_service(_service),
		  m_partId(1) {
			
		}
		bool operator() (jus::TcpString* _interface){
			auto it = m_async.begin();
			while (it != m_async.end()) {
				bool ret = (*it)(_interface, m_service, m_transactionId, m_partId);
				if (ret == true) {
					// Remove it ...
					it = m_async.erase(it);
				} else {
					++it;
				}
				m_partId++;
			}
			if (m_async.size() == 0) {
				ejson::Object obj;
				if (m_service != "") {
					obj.add("service", ejson::String(m_service));
				}
				obj.add("id", ejson::Number(m_transactionId));
				obj.add("part", ejson::Number(m_partId));
				obj.add("finish", ejson::Boolean(true));
				JUS_DEBUG("Send JSON '" << obj.generateHumanString() << "'");
				_interface->write(obj.generateMachineString());
				return true;
			}
			return false;
		}
};

jus::FutureBase jus::Client::callJson(uint64_t _transactionId,
                                      ejson::Object _obj,
                                      const std::vector<ActionAsyncClient>& _async,
                                      jus::FutureData::ObserverFinish _callback,
                                      const std::string& _service) {
	JUS_VERBOSE("Send JSON [START] ");
	if (m_interfaceClient.isActive() == false) {
		ejson::Object obj;
		obj.add("error", ejson::String("NOT-CONNECTED"));
		obj.add("error-help", ejson::String("Client interface not connected (no TCP)"));
		return jus::FutureBase(_transactionId, true, obj, _callback);
	}
	jus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_pendingCall.push_back(tmpFuture);
	}
	if (_async.size() != 0) {
		_obj.add("part", ejson::Number(0));
	}
	JUS_DEBUG("Send JSON '" << _obj.generateHumanString() << "'");
	m_interfaceClient.write(_obj.generateMachineString());
	
	if (_async.size() != 0) {
		m_interfaceClient.addAsync(SendAsync(_transactionId, _service, _async));
	}
	JUS_VERBOSE("Send JSON [STOP]");
	return tmpFuture;
}

