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

const zeus::Buffer& zeus::FutureBase::getRaw() {
	if (m_data == nullptr) {
		static zeus::Buffer tmpp;
		return tmpp;
	}
	return m_data->m_returnData;
}

zeus::FutureBase::FutureBase(uint64_t _transactionId, bool _isFinished, zeus::Buffer _returnData, zeus::FutureData::ObserverFinish _callback) {
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

bool zeus::FutureBase::setAnswer(const zeus::Buffer& _returnValue) {
	if (m_data == nullptr) {
		ZEUS_ERROR(" Not a valid future ...");
		return true;
	}
	m_data->m_receiveTime = std::chrono::steady_clock::now();
	if (m_data->m_isSynchronous == true) {
		m_data->m_returnData = _returnValue;
		if (m_data->m_callbackFinish != nullptr) {
			return m_data->m_callbackFinish(*this);
		}
		return true;
	}
	/* TODO : ...
	if (_returnValue.valueExist("part") == true) {
		uint64_t idPart = _returnValue["part"].toNumber().getU64();
		if (idPart == 0) {
			m_data->m_returnData = _returnValue;
		} else {
			m_data->m_returnDataPart.push_back(_returnValue["data"]);
		}
		if (_returnValue.valueExist("finish") == true) {
			if (_returnValue["finish"].toBoolean().get() == true) {
				m_data->m_isFinished = true;
				if (m_data->m_callbackFinish != nullptr) {
					return m_data->m_callbackFinish(*this);
				}
				return true;
			}
			// finish is false ==> normal case ...
		}
		return false;
	}*/
	m_data->m_returnData = _returnValue;
	m_data->m_isFinished = true;
	if (m_data->m_callbackFinish != nullptr) {
		return m_data->m_callbackFinish(*this);
	}
	return true;
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
	return m_data->m_returnData.hasError();
}

std::string zeus::FutureBase::getErrorType() {
	if (m_data == nullptr) {
		return "NULL_PTR";
	}
	return m_data->m_returnData.getError();
}

std::string zeus::FutureBase::getErrorHelp() {
	if (m_data == nullptr) {
		return "Thsi is a nullptr future";
	}
	return m_data->m_returnData.getErrorHelp();
}

bool zeus::FutureBase::isValid() {
	return m_data != nullptr;
}

bool zeus::FutureBase::isFinished() {
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


zeus::FutureCall::FutureCall(uint64_t _clientId, uint64_t _transactionId, zeus::Buffer& _callValue) :
  m_transactionId(_transactionId),
  m_clientId(_clientId),
  m_isFinished(false) {
	m_data = _callValue;
	m_isFinished = m_data.getPartFinish();
}

void zeus::FutureCall::appendData(zeus::Buffer& _callValue) {
	m_dataMultiplePack.push_back(_callValue);
	m_isFinished = _callValue.getPartFinish();
}

uint64_t zeus::FutureCall::getTransactionId() {
	return m_transactionId;
}

uint64_t zeus::FutureCall::getClientId() {
	return m_clientId;
}

bool zeus::FutureCall::isFinished() {
	return m_isFinished;
}

zeus::Buffer& zeus::FutureCall::getRaw() {
	return m_data;
}

std::chrono::nanoseconds zeus::FutureCall::getTransmitionTime() {
	return m_answerTime - m_receiveTime;
}
