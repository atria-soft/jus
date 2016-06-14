/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/TcpString.h>
#include <jus/debug.h>
#include <ethread/tools.h>
#include <unistd.h>

jus::TcpString::TcpString(enet::Tcp _connection) :
  m_connection(std::move(_connection)),
  m_thread(nullptr),
  m_observerElement(nullptr),
  m_threadAsync(nullptr) {
	m_threadRunning = false;
	m_threadAsyncRunning = false;
	m_interfaceMode = jus::connectionMode::modeJson;
	m_transmissionId = 0;
}

jus::TcpString::TcpString() :
  m_connection(),
  m_thread(nullptr),
  m_observerElement(nullptr),
  m_threadAsync(nullptr) {
	m_threadRunning = false;
	m_threadAsyncRunning = false;
	m_interfaceMode = jus::connectionMode::modeJson;
	m_transmissionId = 0;
}

void jus::TcpString::setInterface(enet::Tcp _connection) {
	m_connection = std::move(_connection);
}

jus::TcpString::~TcpString() {
	disconnect();
}

void jus::TcpString::setInterfaceName(const std::string& _name) {
	ethread::setName(*m_thread, "Tcp-" + _name);
}

void jus::TcpString::threadCallback() {
	ethread::setName("TcpString-input");
	// get datas:
	while (    m_threadRunning == true
	        && m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		// READ section data:
		read();
	}
	m_threadRunning = false;
	JUS_DEBUG("End of thread");
}

bool jus::TcpString::isActive() const {
	return m_threadRunning;
}

void jus::TcpString::connect(bool _async){
	JUS_DEBUG("connect [START]");
	m_threadRunning = true;
	m_thread = new std::thread([&](void *){ this->threadCallback();}, nullptr);
	if (m_thread == nullptr) {
		m_threadRunning = false;
		JUS_ERROR("creating callback thread!");
		return;
	}
	m_threadAsyncRunning = true;
	m_threadAsync = new std::thread([&](void *){ this->threadAsyncCallback();}, nullptr);
	if (m_threadAsync == nullptr) {
		m_threadAsyncRunning = false;
		JUS_ERROR("creating async sender thread!");
		return;
	}
	
	while (    _async == false
	        && m_threadRunning == true
	        && m_connection.getConnectionStatus() != enet::Tcp::status::link) {
		usleep(50000);
	}
	//ethread::setPriority(*m_receiveThread, -6);
	if (_async == true) {
		JUS_DEBUG("connect [STOP] async mode");
	} else {
		JUS_DEBUG("connect [STOP]");
	}
}

void jus::TcpString::disconnect(bool _inThreadStop){
	JUS_DEBUG("disconnect [START]");
	m_threadRunning = false;
	m_threadAsyncRunning = false;
	if (m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		uint32_t size = 0xFFFFFFFF;
		m_connection.write(&size, 4);
	}
	if (m_connection.getConnectionStatus() != enet::Tcp::status::unlink) {
		m_connection.unlink();
	}
	if (m_threadAsync != nullptr) {
		m_threadAsync->join();
		delete m_threadAsync;
		m_threadAsync = nullptr;
	}
	if (_inThreadStop == false) {
		if (m_thread != nullptr) {
			m_thread->join();
			delete m_thread;
			m_thread = nullptr;
		}
	}
	JUS_DEBUG("disconnect [STOP]");
}

int32_t jus::TcpString::writeJson(ejson::Object& _data) {
	JUS_DEBUG("Send JSON '" << _data.generateHumanString() << "'");
	if (m_threadRunning == false) {
		return -2;
	}
	m_lastSend = std::chrono::steady_clock::now();
	m_connection.write("J", 1);
	std::string tmpData = _data.generateMachineString();
	uint32_t dataSize = tmpData.size();
	m_connection.write(&dataSize, sizeof(uint32_t));
	m_connection.write(&tmpData[0], tmpData.size());
	return 1;
}

int32_t jus::TcpString::writeBinary(jus::Buffer& _data) {
	_data.prepare();
	JUS_DEBUG("Send BINARY '" << _data.toJson().generateHumanString() << "'");
	if (m_threadRunning == false) {
		return -2;
	}
	m_lastSend = std::chrono::steady_clock::now();
	m_connection.write("B", 1);
	const uint8_t* data = nullptr;
	uint32_t dataSize = 0;
	data = _data.getHeader();
	dataSize = _data.getHeaderSize();
	m_connection.write(data, dataSize);
	data = _data.getParam();
	dataSize = _data.getParamSize();
	m_connection.write(data, dataSize);
	data = _data.getData();
	dataSize = _data.getDataSize();
	m_connection.write(data, dataSize);
	return 1;
}

void jus::TcpString::read() {
	JUS_VERBOSE("Read [START]");
	if (m_threadRunning == false) {
		JUS_DEBUG("Read [END] Disconected");
		return;
	}
	// TODO : Do it better with a correct way to check data size ...
	JUS_VERBOSE("Read [START]");
	uint8_t type = 0;
	int32_t len = m_connection.read(&type, 1);
	if (len == 0) {
		JUS_ERROR("Protocol error occured ==> No datas ...");
	} else {
		if (type == 'B') { // binary
			// Binary mode ... start with the lenght of the stream
			JUS_VERBOSE("Read Binary [START]");
			uint32_t size = 0;
			len = m_connection.read(&size, 4);
			if (len != 4) {
				JUS_ERROR("Protocol error occured ...");
			} else {
				if (size == -1) {
					JUS_WARNING("Remote close connection");
					m_threadRunning = false;
					//m_connection.unlink();
				} else {
					int64_t offset = 0;
					m_buffer.resize(size);
					while (offset != size) {
						len = m_connection.read(&m_buffer[offset], size-offset);
						offset += len;
						if (len == 0) {
							JUS_WARNING("Read No data");
						}
					}
					jus::Buffer dataRaw;
					dataRaw.composeWith(m_buffer);
					newBuffer(dataRaw);
				}
			}
			JUS_VERBOSE("ReadRaw [STOP]");
		} else if (    type == 'X' // XML
		            || type == 'J' // JSON
		          ) {
			JUS_VERBOSE("Read sized String [START]");
			// XML/JSON mode ... start with the lenght of the stream
			std::string out;
			uint32_t size = 0;
			len = m_connection.read(&size, 4);
			if (len != 4) {
				JUS_ERROR("Protocol error occured ...");
			} else {
				if (size == -1) {
					JUS_WARNING("Remote close connection");
					m_threadRunning = false;
					//m_connection.unlink();
				} else {
					int64_t offset = 0;
					out.resize(size);
					while (offset != size) {
						len = m_connection.read(&out[offset], size-offset);
						offset += len;
						if (len == 0) {
							JUS_WARNING("Read No data");
						}
					}
					jus::Buffer dataRaw;
					dataRaw.composeWith(out);
					newBuffer(dataRaw);
				}
			}
			JUS_VERBOSE("Read sized String [STOP]");
		} else if (type == '{') {
			// JSON Raw mode ... Finish with a \0
			// TODO: m_dataBuffer
		} else if (type == '<') {
			// XML Raw mode ... Finish with a \0
			// TODO : m_dataBuffer
		}
	}
}

void jus::TcpString::newBuffer(jus::Buffer& _buffer) {
	JUS_VERBOSE("Receive Binary :" << _buffer.toJson().generateHumanString());
	jus::FutureBase future;
	uint64_t tid = _buffer.getTransactionId();
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
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
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
		if (m_observerElement != nullptr) {
			m_observerElement(_buffer);
		}
		return;
	}
	bool ret = future.setAnswer(_buffer);
	if (ret == true) {
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
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

void jus::TcpString::threadAsyncCallback() {
	ethread::setName("Async-sender");
	// get datas:
	while (    m_threadAsyncRunning == true
	        && m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		if (m_threadAsyncList.size() == 0) {
			usleep(10000);
			continue;
		}
		std::unique_lock<std::mutex> lock(m_threadAsyncMutex);
		auto it = m_threadAsyncList.begin();
		while (it != m_threadAsyncList.end()) {
			bool ret = (*it)(this);
			if (ret == true) {
				// Remove it ...
				it = m_threadAsyncList.erase(it);
			} else {
				++it;
			}
		}
	}
	m_threadRunning = false;
	JUS_DEBUG("End of thread");
}




class SendAsyncJson {
	private:
		std::vector<jus::ActionAsyncClient> m_async;
		uint64_t m_transactionId;
		uint32_t m_serviceId;
		uint32_t m_partId;
	public:
		SendAsyncJson(uint64_t _transactionId, const uint32_t& _serviceId, const std::vector<jus::ActionAsyncClient>& _async) :
		  m_async(_async),
		  m_transactionId(_transactionId),
		  m_serviceId(_serviceId),
		  m_partId(1) {
			
		}
		bool operator() (jus::TcpString* _interface){
			auto it = m_async.begin();
			while (it != m_async.end()) {
				bool ret = (*it)(_interface, m_serviceId, m_transactionId, m_partId);
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
				if (m_serviceId != 0) {
					obj.add("service", ejson::Number(m_serviceId));
				}
				obj.add("id", ejson::Number(m_transactionId));
				obj.add("part", ejson::Number(m_partId));
				obj.add("finish", ejson::Boolean(true));
				_interface->writeJson(obj);
				return true;
			}
			return false;
		}
};

jus::FutureBase jus::TcpString::callJson(uint64_t _transactionId,
                                         ejson::Object _obj,
                                         const std::vector<ActionAsyncClient>& _async,
                                         jus::FutureData::ObserverFinish _callback,
                                         const uint32_t& _serviceId) {
	JUS_VERBOSE("Send JSON [START] ");
	if (m_interfaceClient.isActive() == false) {
		jus::Buffer obj;
		obj.setType(jus::Buffer::typeMessage::answer) {
		obj.addError("NOT-CONNECTED", "Client interface not connected (no TCP)");
		return jus::FutureBase(_transactionId, true, obj, _callback);
	}
	jus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
		m_pendingCall.push_back(tmpFuture);
	}
	if (_async.size() != 0) {
		_obj.add("part", ejson::Number(0));
	}
	writeJson(_obj);
	
	if (_async.size() != 0) {
		addAsync(SendAsyncJson(_transactionId, _serviceId, _async));
	}
	JUS_VERBOSE("Send JSON [STOP]");
	return tmpFuture;
}


class SendAsyncBinary {
	private:
		std::vector<jus::ActionAsyncClient> m_async;
		uint64_t m_transactionId;
		uint32_t m_serviceId;
		uint32_t m_partId;
	public:
		SendAsyncBinary(uint64_t _transactionId, const uint32_t& _serviceId, const std::vector<jus::ActionAsyncClient>& _async) :
		  m_async(_async),
		  m_transactionId(_transactionId),
		  m_serviceId(_serviceId),
		  m_partId(1) {
			
		}
		bool operator() (jus::TcpString* _interface){
			auto it = m_async.begin();
			while (it != m_async.end()) {
				bool ret = (*it)(_interface, m_serviceId, m_transactionId, m_partId);
				if (ret == true) {
					// Remove it ...
					it = m_async.erase(it);
				} else {
					++it;
				}
				m_partId++;
			}
			if (m_async.size() == 0) {
				jus::Buffer obj;
				obj.setServiceId(m_serviceId);
				obj.setTransactionId(m_transactionId);
				obj.setPartId(m_partId);
				obj.setPartFinish(true);
				_interface->writeBinary(obj);
				return true;
			}
			return false;
		}
};

jus::FutureBase jus::TcpString::callBinary(uint64_t _transactionId,
                                           jus::Buffer& _obj,
                                           const std::vector<ActionAsyncClient>& _async,
                                           jus::FutureData::ObserverFinish _callback,
                                           const uint32_t& _serviceId) {
	JUS_VERBOSE("Send Binary [START] ");
	if (m_interfaceClient.isActive() == false) {
		jus::Buffer obj;
		obj.setType(jus::Buffer::typeMessage::answer) {
		obj.addError("NOT-CONNECTED", "Client interface not connected (no TCP)");
		return jus::FutureBase(_transactionId, true, obj, _callback);
	}
	jus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
		m_pendingCall.push_back(tmpFuture);
	}
	if (_async.size() != 0) {
		_obj.setPartFinish(false);
	} else {
		_obj.setPartFinish(true);
	}
	writeBinary(_obj);
	
	if (_async.size() != 0) {
		addAsync(SendAsyncBinary(_transactionId, _serviceId, _async));
	}
	JUS_VERBOSE("Send Binary [STOP]");
	return tmpFuture;
}






void jus::TcpString::answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorHelp, uint32_t _clientId) {
	if (m_interfaceMode == jus::connectionMode::modeJson) {
		ejson::Object answer;
		answer.add("error", ejson::String(_errorValue));
		answer.add("id", ejson::Number(_clientTransactionId));
		if (_clientId != 0) {
			answer.add("client-id", ejson::Number(_clientId));
		}
		answer.add("error-help", ejson::String(_errorHelp));
		writeJson(answer);
	} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
		jus::Buffer answer;
		answer.setType(jus::Buffer::typeMessage::answer);
		answer.setTransactionId(_clientTransactionId);
		answer.setClientId(_clientId);
		answer.addError(_errorValue, _errorHelp);
		writeBinary(answer);
	} else if (m_interfaceMode == jus::connectionMode::modeXml) {
		JUS_ERROR("TODO ... ");
	} else {
		JUS_ERROR("wrong type of communication");
	}
}


void jus::TcpString::answerVoid(uint64_t _clientTransactionId, uint32_t _clientId) {
	if (m_interfaceMode == jus::connectionMode::modeJson) {
		ejson::Object answer;
		answer.add("id", ejson::Number(_clientTransactionId));
		if (_clientId != 0) {
			answer.add("client-id", ejson::Number(_clientId));
		}
		answer.add("return", ejson::Null());
		writeJson(answer);
	} else if (m_interfaceMode == jus::connectionMode::modeBinary) {
		jus::Buffer answer;
		answer.setType(jus::Buffer::typeMessage::answer);
		answer.setTransactionId(_clientTransactionId);
		answer.setClientId(_clientId);
		answer.addParameter();
		writeBinary(answer);
	} else if (m_interfaceMode == jus::connectionMode::modeXml) {
		JUS_ERROR("TODO ... ");
	} else {
		JUS_ERROR("wrong type of communication");
	}
}