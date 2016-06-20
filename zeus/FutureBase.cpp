/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/FutureBase.h>
#include <jus/debug.h>
#include <unistd.h>

jus::FutureBase::FutureBase(const jus::FutureBase& _base):
  m_data(_base.m_data) {
	
}

jus::FutureBase::FutureBase() {
	m_data = nullptr;
}

jus::FutureBase::FutureBase(uint64_t _transactionId, jus::FutureData::ObserverFinish _callback) {
	m_data = std::make_shared<jus::FutureData>();
	if (m_data == nullptr) {
		return;
	}
	m_data->m_sendTime = std::chrono::steady_clock::now();
	m_data->m_transactionId = _transactionId;
	m_data->m_isFinished = false;
	m_data->m_isSynchronous = false;
	m_data->m_callbackFinish = _callback;
}

const jus::Buffer& jus::FutureBase::getRaw() {
	if (m_data == nullptr) {
		static jus::Buffer tmpp;
		return tmpp;
	}
	return m_data->m_returnData;
}

jus::FutureBase::FutureBase(uint64_t _transactionId, bool _isFinished, jus::Buffer _returnData, jus::FutureData::ObserverFinish _callback) {
	m_data = std::make_shared<jus::FutureData>();
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
std::chrono::nanoseconds jus::FutureBase::getTransmitionTime() {
	if (m_data == nullptr) {
		return std::chrono::nanoseconds(0);
	}
	if (m_data->m_isFinished == false) {
		return std::chrono::nanoseconds(0);
	}
	return m_data->m_receiveTime - m_data->m_sendTime;
}

jus::FutureBase jus::FutureBase::operator= (const jus::FutureBase& _base) {
	m_data = _base.m_data;
	return *this;
}

bool jus::FutureBase::setAnswer(const jus::Buffer& _returnValue) {
	if (m_data == nullptr) {
		JUS_ERROR(" Not a valid future ...");
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
void jus::FutureBase::setSynchronous() {
	if (m_data == nullptr) {
		return;
	}
	m_data->m_isSynchronous = true;
}

uint64_t jus::FutureBase::getTransactionId() {
	if (m_data == nullptr) {
		return 0;
	}
	return m_data->m_transactionId;
}

bool jus::FutureBase::hasError() {
	if (m_data == nullptr) {
		return true;
	}
	return m_data->m_returnData.hasError();
}

std::string jus::FutureBase::getErrorType() {
	if (m_data == nullptr) {
		return "NULL_PTR";
	}
	return m_data->m_returnData.getError();
}

std::string jus::FutureBase::getErrorHelp() {
	if (m_data == nullptr) {
		return "Thsi is a nullptr future";
	}
	return m_data->m_returnData.getErrorHelp();
}

bool jus::FutureBase::isValid() {
	return m_data != nullptr;
}

bool jus::FutureBase::isFinished() {
	if (m_data == nullptr) {
		return true;
	}
	return m_data->m_isFinished;
}

jus::FutureBase& jus::FutureBase::wait() {
	while (isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}

jus::FutureBase& jus::FutureBase::waitFor(std::chrono::microseconds _delta) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	while (    std::chrono::steady_clock::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
		start = std::chrono::steady_clock::now();
	}
	return *this;
}

jus::FutureBase& jus::FutureBase::waitUntil(std::chrono::steady_clock::time_point _endTime) {
	while (    std::chrono::steady_clock::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		usleep(10000);
	}
	return *this;
}


jus::FutureCall::FutureCall(uint64_t _clientId, uint64_t _transactionId, jus::Buffer& _callValue) :
  m_transactionId(_transactionId),
  m_clientId(_clientId),
  m_isFinished(false) {
	m_data = _callValue;
	m_isFinished = m_data.getPartFinish();
}

void jus::FutureCall::appendData(jus::Buffer& _callValue) {
	m_dataMultiplePack.push_back(_callValue);
	m_isFinished = _callValue.getPartFinish();
}

uint64_t jus::FutureCall::getTransactionId() {
	return m_transactionId;
}

uint64_t jus::FutureCall::getClientId() {
	return m_clientId;
}

bool jus::FutureCall::isFinished() {
	return m_isFinished;
}

jus::Buffer& jus::FutureCall::getRaw() {
	return m_data;
}

std::chrono::nanoseconds jus::FutureCall::getTransmitionTime() {
	return m_answerTime - m_receiveTime;
}
