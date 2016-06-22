/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/FutureBase.h>
#include <zeus/debug.h>
#include <unistd.h>

zeus::FutureBase::FutureBase(const zeus::FutureBase& _base):
  m_data(_base.m_data) {
	
}

zeus::FutureBase::FutureBase() {
	m_data = nullptr;
}

zeus::FutureBase::FutureBase(uint64_t _transactionId, zeus::FutureData::ObserverFinish _callback) {
	m_data = std::make_shared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_isFinished = false;
	m_data->m_isSynchronous = false;
	m_data->m_callbackFinish = _callback;
}

ememory::SharedPtr<zeus::Buffer> zeus::FutureBase::getRaw() {
	if (m_data == nullptr) {
		return nullptr;
	}
	return m_data->m_returnData;
}

zeus::FutureBase::FutureBase(uint64_t _transactionId, bool _isFinished, const ememory::SharedPtr<zeus::Buffer>& _returnData, zeus::FutureData::ObserverFinish _callback) {
	m_data = std::make_shared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_isFinished = _isFinished;
	m_data->m_isSynchronous = false;
	m_data->m_returnData = _returnData;
	m_data->m_callbackFinish = _callback;
	if (m_data->m_isFinished == true) {
		m_data->m_receiveTime = std::chrono::steady_clock::now();
		if (m_data->m_callbackFinish != nullptr) {
			m_data->m_callbackFinish(*this);
		}
	}
}
std::chrono::nanoseconds zeus::FutureBase::getTransmitionTime() {
	if (m_data == nullptr) {
		return std::chrono::nanoseconds(0);
	}
	if (m_data->m_isFinished == false) {
		return std::chrono::nanoseconds(0);
	}
	return m_data->m_receiveTime - m_data->m_sendTime;
}

zeus::FutureBase zeus::FutureBase::operator= (const zeus::FutureBase& _base) {
	m_data = _base.m_data;
	return *this;
}

bool zeus::FutureBase::setAnswer(const ememory::SharedPtr<zeus::Buffer>& _value) {
	if (m_data == nullptr) {
		ZEUS_ERROR(" Not a valid future ...");
		return true;
	}
	m_data->m_receiveTime = std::chrono::steady_clock::now();
	if (m_data->m_isSynchronous == true) {
		m_data->m_returnData = _value;
		if (m_data->m_callbackFinish != nullptr) {
			return m_data->m_callbackFinish(*this);
		}
		return true;
	}
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		if (m_data->m_returnData != nullptr) {
			m_data->m_returnData->appendBufferData(_value);
		}
	} else {
		m_data->m_returnData = _value;
	}
	m_data->m_isFinished = _value->getPartFinish();
	if (m_data->m_callbackFinish != nullptr) {
		return m_data->m_callbackFinish(*this);
	}
	return m_data->m_isFinished;
}
void zeus::FutureBase::setSynchronous() {
	if (m_data == nullptr) {
		return;
	}
	m_data->m_isSynchronous = true;
}

uint64_t zeus::FutureBase::getTransactionId() {
	if (m_data == nullptr) {
		return 0;
	}
	return m_data->m_transactionId;
}

bool zeus::FutureBase::hasError() {
	if (m_data == nullptr) {
		return true;
	}
	return m_data->m_returnData->hasError();
}

std::string zeus::FutureBase::getErrorType() {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return "NULL_PTR";
	}
	return m_data->m_returnData->getError();
}

std::string zeus::FutureBase::getErrorHelp() {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return "Thsi is a nullptr future";
	}
	return m_data->m_returnData->getErrorHelp();
}

bool zeus::FutureBase::isValid() const {
	return m_data != nullptr;
}

bool zeus::FutureBase::isFinished() const {
	if (m_data == nullptr) {
		return true;
	}
	return m_data->m_isFinished;
}

zeus::FutureBase& zeus::FutureBase::wait() {
	while (isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}

zeus::FutureBase& zeus::FutureBase::waitFor(std::chrono::microseconds _delta) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (    std::chrono::steady_clock::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
		start = std::chrono::steady_clock::now();
	}
	return *this;
}

zeus::FutureBase& zeus::FutureBase::waitUntil(std::chrono::steady_clock::time_point _endTime) {
	while (    std::chrono::steady_clock::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}


zeus::FutureCall::FutureCall(uint64_t _clientId, uint64_t _transactionId, const ememory::SharedPtr<zeus::Buffer>& _callValue) :
  m_transactionId(_transactionId),
  m_clientId(_clientId),
  m_isFinished(false) {
	m_data = _callValue;
	m_isFinished = m_data->getPartFinish();
}

void zeus::FutureCall::appendData(const ememory::SharedPtr<zeus::Buffer>& _value) {
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		if (m_data == nullptr) {
			return;
		}
		m_data->appendBufferData(_value);
	} else {
		m_data = _value;
	}
	m_dataMultiplePack.push_back(_value);
	m_isFinished = _value->getPartFinish();
}

uint64_t zeus::FutureCall::getTransactionId() const {
	return m_transactionId;
}

uint64_t zeus::FutureCall::getClientId() const {
	return m_clientId;
}

bool zeus::FutureCall::isFinished() const {
	return m_isFinished;
}

ememory::SharedPtr<zeus::Buffer> zeus::FutureCall::getRaw() const {
	return m_data;
}

std::chrono::nanoseconds zeus::FutureCall::getTransmitionTime() const {
	return m_answerTime - m_receiveTime;
}
