/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/FutureBase.hpp>
#include <zeus/BufferAnswer.hpp>
#include <zeus/debug.hpp>


zeus::FutureBase::FutureBase(const zeus::FutureBase& _base):
  m_data(_base.m_data) {
	
}

zeus::FutureBase::FutureBase() {
	m_data = nullptr;
}

zeus::FutureBase::FutureBase(uint32_t _transactionId, uint32_t _source) {
	m_data = ememory::makeShared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_source = _source;
	m_data->m_isSynchronous = false;
}

ememory::SharedPtr<zeus::Buffer> zeus::FutureBase::getRaw() {
	if (m_data == nullptr) {
		return nullptr;
	}
	return m_data->m_returnData;
}

zeus::FutureBase::FutureBase(uint32_t _transactionId, ememory::SharedPtr<zeus::Buffer> _returnData, uint32_t _source) {
	m_data = ememory::makeShared<zeus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_source = _source;
	m_data->m_isSynchronous = false;
	m_data->m_returnData = _returnData;
	if (isFinished() == true) {
		m_data->m_receiveTime = std::chrono::steady_clock::now();
	}
}

void zeus::FutureBase::andAll(zeus::FutureData::Observer _callback) {
	if (m_data == nullptr) {
		return;
	}
	// TODO : Lock ...
	m_data->m_callbackThen = _callback;
	m_data->m_callbackElse = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		if (m_data->m_callbackThen != nullptr) {
			m_data->m_callbackThen(*this);
		}
	} else {
		if (m_data->m_callbackElse != nullptr) {
			m_data->m_callbackElse(*this);
		}
	}
}

void zeus::FutureBase::andThen(zeus::FutureData::Observer _callback) {
	if (m_data == nullptr) {
		return;
	}
	// TODO : Lock ...
	m_data->m_callbackThen = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == true) {
		return;
	}
	if (m_data->m_callbackThen == nullptr) {
		return;
	}
	m_data->m_callbackThen(*this);
}

void zeus::FutureBase::andElse(zeus::FutureData::Observer _callback) {
	if (m_data == nullptr) {
		return;
	}
	// TODO : Lock ...
	m_data->m_callbackElse = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		return;
	}
	if (m_data->m_callbackElse == nullptr) {
		return;
	}
	m_data->m_callbackElse(*this);
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

bool zeus::FutureBase::appendData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (m_data == nullptr) {
		ZEUS_ERROR(" Not a valid future ...");
		return true;
	}
	m_data->m_receiveTime = std::chrono::steady_clock::now();
	if (m_data->m_isSynchronous == true) {
		m_data->m_returnData = _value;
		if (hasError() == false) {
			if (m_data->m_callbackThen != nullptr) {
				return m_data->m_callbackThen(*this);
			}
		} else {
			if (m_data->m_callbackElse != nullptr) {
				return m_data->m_callbackElse(*this);
			}
		}
		return true;
	}
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		if (m_data->m_returnData != nullptr) {
			m_data->m_returnData->appendBuffer(_value);
		}
	} else {
		m_data->m_returnData = _value;
	}
	if (m_data->m_returnData == nullptr) {
		return true;
	}
	if (m_data->m_returnData->getPartFinish() == true) {
		if (hasError() == false) {
			if (m_data->m_callbackThen != nullptr) {
				return m_data->m_callbackThen(*this);
			}
		} else {
			if (m_data->m_callbackElse != nullptr) {
				return m_data->m_callbackElse(*this);
			}
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

uint32_t zeus::FutureBase::getSource() const {
	if (m_data == nullptr) {
		return 0;
	}
	return m_data->m_source;
}

bool zeus::FutureBase::hasError() const {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return true;
	}
	if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
		return true;
	}
	return static_cast<const zeus::BufferAnswer*>(m_data->m_returnData.get())->hasError();
}

std::string zeus::FutureBase::getErrorType() const {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return "NULL_PTR";
	}
	if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
		return "NOT_ANSWER_MESSAGE";
	}
	return static_cast<const zeus::BufferAnswer*>(m_data->m_returnData.get())->getError();
}

std::string zeus::FutureBase::getErrorHelp() const {
	if (    m_data == nullptr
	     || m_data->m_returnData == nullptr) {
		return "This is a nullptr future";
	}
	if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
		return "This answer is not a anwser type";
	}
	return static_cast<const zeus::BufferAnswer*>(m_data->m_returnData.get())->getErrorHelp();
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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitFor(std::chrono::microseconds _delta) const {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (    std::chrono::steady_clock::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		start = std::chrono::steady_clock::now();
	}
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitUntil(std::chrono::steady_clock::time_point _endTime) const {
	while (    std::chrono::steady_clock::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return *this;
}

