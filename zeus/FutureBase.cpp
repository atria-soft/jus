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

zeus::FutureBase::FutureBase(uint32_t _transactionId, zeus::FutureData::ObserverFinish _callback, uint32_t _clientId) {
	m_data = std::make_shared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_clientId = _clientId;
	m_data->m_isSynchronous = false;
	m_data->m_callbackFinish = _callback;
}

ememory::SharedPtr<zeus::Buffer> zeus::FutureBase::getRaw() {
	if (m_data == nullptr) {
		return nullptr;
	}
	return m_data->m_returnData;
}

zeus::FutureBase::FutureBase(uint32_t _transactionId, const ememory::SharedPtr<zeus::Buffer>& _returnData, zeus::FutureData::ObserverFinish _callback, uint32_t _clientId) {
	m_data = std::make_shared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_isSynchronous = false;
	m_data->m_returnData = _returnData;
	m_data->m_callbackFinish = _callback;
	if (isFinished() == true) {
		m_data->m_receiveTime = std::chrono::steady_clock::now();
		if (m_data->m_callbackFinish != nullptr) {
			m_data->m_callbackFinish(*this);
		}
	}
}
std::chrono::nanoseconds zeus::FutureBase::getTransmitionTime() const {
	if (m_data == nullptr) {
		return std::chrono::nanoseconds(0);
	}
	if (isFinished() == false) {
		return std::chrono::nanoseconds(0);
	}
	return m_data->m_receiveTime - m_data->m_sendTime;
}

zeus::FutureBase zeus::FutureBase::operator= (const zeus::FutureBase& _base) {
	m_data = _base.m_data;
	return *this;
}

bool zeus::FutureBase::appendData(const ememory::SharedPtr<zeus::Buffer>& _value) {
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
	if (m_data->m_returnData == nullptr) {
		return true;
	}
	if (m_data->m_returnData->getPartFinish() == true) {
		if (m_data->m_callbackFinish != nullptr) {
			return m_data->m_callbackFinish(*this);
		}
		return true;
	}
	return false;
}
void zeus::FutureBase::setSynchronous() {
	if (m_data == nullptr) {
		return;
	}
	m_data->m_isSynchronous = true;
}

uint32_t zeus::FutureBase::getTransactionId() const {
	if (m_data == nullptr) {
		return 0;
	}
	return m_data->m_transactionId;
}

uint32_t zeus::FutureBase::getClientId() const {
	if (m_data == nullptr) {
		return 0;
	}
	return m_data->m_clientId;
}

bool zeus::FutureBase::hasError() const {
	if (m_data == nullptr) {
		return true;
	}
	return m_data->m_returnData->hasError();
}

std::string zeus::FutureBase::getErrorType() const {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return "NULL_PTR";
	}
	return m_data->m_returnData->getError();
}

std::string zeus::FutureBase::getErrorHelp() const {
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
	if (m_data->m_returnData == nullptr) {
		// in this case, we are waiting for an answer that the first packet is not arrived
		return false;
	}
	return m_data->m_returnData->getPartFinish();
}

const zeus::FutureBase& zeus::FutureBase::wait() const {
	while (isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitFor(std::chrono::microseconds _delta) const {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (    std::chrono::steady_clock::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
		start = std::chrono::steady_clock::now();
	}
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitUntil(std::chrono::steady_clock::time_point _endTime) const {
	while (    std::chrono::steady_clock::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}

