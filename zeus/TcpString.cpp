/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/TcpString.h>
#include <zeus/debug.h>
#include <ethread/tools.h>
#include <unistd.h>

zeus::TcpString::TcpString() :
  m_connection(),
  m_observerElement(nullptr),
  m_threadAsync(nullptr) {
	m_threadAsyncRunning = false;
	m_transmissionId = 1;
}

zeus::TcpString::TcpString(enet::Tcp _connection, bool _isServer) :
  m_connection(),
  m_observerElement(nullptr),
  m_threadAsync(nullptr) {
	m_threadAsyncRunning = false;
	m_transmissionId = 1;
	setInterface(std::move(_connection), _isServer);
}

void zeus::TcpString::setInterface(enet::Tcp _connection, bool _isServer) {
	m_connection.setInterface(std::move(_connection), _isServer);
	m_connection.connect(this, &zeus::TcpString::onReceiveData);
	if (_isServer == true) {
		m_connection.connectUri(this, &zeus::TcpString::onReceiveUri);
		m_connection.start();
	} else {
		m_connection.start("/stupidName");
	}
}

zeus::TcpString::~TcpString() {
	disconnect();
}

void zeus::TcpString::setInterfaceName(const std::string& _name) {
	//ethread::setName(*m_thread, "Tcp-" + _name);
}


bool zeus::TcpString::isActive() const {
	return m_connection.isAlive();
}

void zeus::TcpString::connect(bool _async){
	ZEUS_DEBUG("connect [START]");
	m_threadAsyncRunning = true;
	m_threadAsync = new std::thread([&](void *){ this->threadAsyncCallback();}, nullptr);
	if (m_threadAsync == nullptr) {
		m_threadAsyncRunning = false;
		ZEUS_ERROR("creating async sender thread!");
		return;
	}
	
	while (    _async == false
	        && m_threadAsyncRunning == true
	        && m_connection.isAlive() != true) {
		usleep(50000);
	}
	//ethread::setPriority(*m_receiveThread, -6);
	if (_async == true) {
		ZEUS_DEBUG("connect [STOP] async mode");
	} else {
		ZEUS_DEBUG("connect [STOP]");
	}
}

void zeus::TcpString::disconnect(bool _inThreadStop){
	ZEUS_DEBUG("disconnect [START]");
	m_threadAsyncRunning = false;
	if (m_connection.isAlive() == true) {
		m_connection.controlClose();
	}
	m_connection.stop(_inThreadStop);
	if (m_threadAsync != nullptr) {
		m_threadAsync->join();
		delete m_threadAsync;
		m_threadAsync = nullptr;
	}
	ZEUS_DEBUG("disconnect [STOP]");
}

int32_t zeus::TcpString::writeBinary(zeus::Buffer& _data) {
	uint64_t size = _data.prepare();
	ZEUS_DEBUG("Send BINARY " << size << " bytes '" << _data.generateHumanString() << "'");
	if (m_connection.isAlive() == false) {
		return -2;
	}
	if (m_connection.writeHeader(size, false) == false) {
		return -1;
	}
	uint8_t* data = nullptr;
	uint32_t dataSize = 0;
	data = (uint8_t*)_data.getHeader();
	dataSize = _data.getHeaderSize();
	size = m_connection.writeData(data, dataSize);
	data = (uint8_t*)_data.getParam();
	dataSize = _data.getParamSize();
	size += m_connection.writeData(data, dataSize);
	data = (uint8_t*)_data.getData();
	dataSize = _data.getDataSize();
	size += m_connection.writeData(data, dataSize);
	return size;
}

bool zeus::TcpString::onReceiveUri(const std::string& _uri) {
	ZEUS_INFO("Receive Header uri: " << _uri);
	if (_uri == "/stupidName") {
		return true;
	}
	return false;
}

void zeus::TcpString::onReceiveData(std::vector<uint8_t>& _frame, bool _isBinary) {
	ZEUS_VERBOSE("Receive Frame ... " << _frame.size());
	zeus::Buffer dataRaw;
	if (_isBinary == true) {
		ZEUS_ERROR("Receive non binary frame ...");
		disconnect(true);
		return;
	}
	dataRaw.composeWith(_frame);
	newBuffer(dataRaw);
}

void zeus::TcpString::ping() {
	m_connection.controlPing();
}

void zeus::TcpString::newBuffer(zeus::Buffer& _buffer) {
	ZEUS_VERBOSE("Receive Binary :" << _buffer.generateHumanString());
	zeus::FutureBase future;
	uint64_t tid = _buffer.getTransactionId();
	if (tid == 0) {
		ZEUS_ERROR("Get a Protocol error ... No ID ...");
		/*
		if (obj["error"].toString().get() == "PROTOCOL-ERROR") {
			ZEUS_ERROR("Get a Protocol error ...");
			std::unique_lock<std::mutex> lock(m_mutex);
			for (auto &it : m_pendingCall) {
				if (it.isValid() == false) {
					continue;
				}
				it.setAnswer(obj);
			}
			m_pendingCall.clear();
		} else {
			ZEUS_ERROR("call with no ID ==> error ...");
		}
		*/
		return;
	}
	{
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
		auto it = m_pendingCall.begin();
		while (it != m_pendingCall.end()) {
			if (it->second.isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->second.getTransactionId() != tid) {
				++it;
				continue;
			}
			future = it->second;
			break;
		}
	}
	if (future.isValid() == false) {
		// not a pending call ==> simple event or call ...
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
			if (it->second.isValid() == false) {
				it = m_pendingCall.erase(it);
				continue;
			}
			if (it->second.getTransactionId() != tid) {
				++it;
				continue;
			}
			it = m_pendingCall.erase(it);
			break;
		}
	}
}

void zeus::TcpString::threadAsyncCallback() {
	ethread::setName("Async-sender");
	// get datas:
	while (    m_threadAsyncRunning == true
	        && m_connection.isAlive() == true) {
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
	m_threadAsyncRunning = false;
	ZEUS_DEBUG("End of thread");
}

class SendAsyncBinary {
	private:
		std::vector<zeus::ActionAsyncClient> m_async;
		uint64_t m_transactionId;
		uint32_t m_serviceId;
		uint32_t m_partId;
	public:
		SendAsyncBinary(uint64_t _transactionId, const uint32_t& _serviceId, const std::vector<zeus::ActionAsyncClient>& _async) :
		  m_async(_async),
		  m_transactionId(_transactionId),
		  m_serviceId(_serviceId),
		  m_partId(1) {
			
		}
		bool operator() (zeus::TcpString* _interface){
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
				zeus::Buffer obj;
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

zeus::FutureBase zeus::TcpString::callBinary(uint64_t _transactionId,
                                           zeus::Buffer& _obj,
                                           const std::vector<ActionAsyncClient>& _async,
                                           zeus::FutureData::ObserverFinish _callback,
                                           const uint32_t& _serviceId) {
	ZEUS_VERBOSE("Send Binary [START] ");
	if (isActive() == false) {
		zeus::Buffer obj;
		obj.setType(zeus::Buffer::typeMessage::answer);
		obj.addError("NOT-CONNECTED", "Client interface not connected (no TCP)");
		return zeus::FutureBase(_transactionId, true, obj, _callback);
	}
	zeus::FutureBase tmpFuture(_transactionId, _callback);
	{
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
		m_pendingCall.push_back(std::make_pair(uint64_t(0), tmpFuture));
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
	ZEUS_VERBOSE("Send Binary [STOP]");
	return tmpFuture;
}

zeus::FutureBase zeus::TcpString::callForward(uint32_t _clientId,
                                            zeus::Buffer& _buffer,
                                            uint64_t _singleReferenceId,
                                            zeus::FutureData::ObserverFinish _callback) {
	ZEUS_VERBOSE("Call Forward [START]");
	//zeus::FutureBase ret = callBinary(id, _Buffer, async, _callback);
	//ret.setSynchronous();
	
	if (isActive() == false) {
		zeus::Buffer obj;
		obj.setType(zeus::Buffer::typeMessage::answer);
		obj.addError("NOT-CONNECTED", "Client interface not connected (no TCP)");
		return zeus::FutureBase(0, true, obj, _callback);
	}
	uint64_t id = getId();
	_buffer.setTransactionId(id);
	_buffer.setClientId(_clientId);
	zeus::FutureBase tmpFuture(id, _callback);
	tmpFuture.setSynchronous();
	{
		std::unique_lock<std::mutex> lock(m_pendingCallMutex);
		m_pendingCall.push_back(std::make_pair(_singleReferenceId, tmpFuture));
	}
	writeBinary(_buffer);
	ZEUS_VERBOSE("Send Forward [STOP]");
	return tmpFuture;
}

void zeus::TcpString::callForwardMultiple(uint32_t _clientId,
                                         zeus::Buffer& _buffer,
                                         uint64_t _singleReferenceId){
	// subMessage ... ==> try to forward message:
	std::unique_lock<std::mutex> lock(m_pendingCallMutex);
	for (auto &itCall : m_pendingCall) {
		ZEUS_INFO(" compare : " << itCall.first << " =?= " << _singleReferenceId);
		if (itCall.first == _singleReferenceId) {
			// Find element ==> transit it ...
			_buffer.setTransactionId(itCall.second.getTransactionId());
			_buffer.setClientId(_clientId);
			writeBinary(_buffer);
			return;
		}
	}
	ZEUS_ERROR("Can not transfer part of a message ...");
}

void zeus::TcpString::answerError(uint64_t _clientTransactionId, const std::string& _errorValue, const std::string& _errorHelp, uint32_t _clientId) {
	zeus::Buffer answer;
	answer.setType(zeus::Buffer::typeMessage::answer);
	answer.setTransactionId(_clientTransactionId);
	answer.setClientId(_clientId);
	answer.addError(_errorValue, _errorHelp);
	writeBinary(answer);
}


void zeus::TcpString::answerVoid(uint64_t _clientTransactionId, uint32_t _clientId) {
	zeus::Buffer answer;
	answer.setType(zeus::Buffer::typeMessage::answer);
	answer.setTransactionId(_clientTransactionId);
	answer.setClientId(_clientId);
	answer.addParameter();
	writeBinary(answer);
}