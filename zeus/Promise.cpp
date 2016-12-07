/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/Promise.hpp>
#include <zeus/message/Answer.hpp>
#include <zeus/debug.hpp>


zeus::Promise::Promise(uint32_t _transactionId, uint32_t _source) {
	m_sendTime = echrono::Steady::now();
	m_transactionId = _transactionId;
	m_source = _source;
}

ememory::SharedPtr<zeus::message::Message> zeus::Promise::getRaw() {
	return m_message;
}

zeus::Promise::Promise(uint32_t _transactionId, ememory::SharedPtr<zeus::message::Message> _returnData, uint32_t _source) {
	m_sendTime = echrono::Steady::now();
	m_transactionId = _transactionId;
	m_source = _source;
	m_message = _returnData;
	if (isFinished() == true) {
		m_receiveTime = echrono::Steady::now();
	}
}

void zeus::Promise::andAll(zeus::Promise::Observer _callback) {
	// TODO : Lock ...
	m_callbackThen = _callback;
	m_callbackElse = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		if (m_callbackThen != nullptr) {
			m_callbackThen(*this);
		}
	} else {
		if (m_callbackElse != nullptr) {
			m_callbackElse(*this);
		}
	}
}

void zeus::Promise::andThen(zeus::Promise::Observer _callback) {
	// TODO : Lock ...
	m_callbackThen = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == true) {
		return;
	}
	if (m_callbackThen == nullptr) {
		return;
	}
	m_callbackThen(*this);
}

void zeus::Promise::andElse(zeus::Promise::Observer _callback) {
	// TODO : Lock ...
	m_callbackElse = _callback;
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		return;
	}
	if (m_callbackElse == nullptr) {
		return;
	}
	m_callbackElse(*this);
}


echrono::Duration zeus::Promise::getTransmitionTime() const {
	if (isFinished() == false) {
		return echrono::nanoseconds(0);
	}
	return m_receiveTime - m_sendTime;
}

bool zeus::Promise::setBuffer(ememory::SharedPtr<zeus::message::Message> _value) {
	m_receiveTime = echrono::Steady::now();
	m_message = _value;
	if (m_message == nullptr) {
		return true;
	}
	if (m_message->getPartFinish() == false) {
		ZEUS_ERROR("set buffer that is not finished ...");
		return false;
	}
	if (hasError() == false) {
		if (m_callbackThen != nullptr) {
			return m_callbackThen(*this);
		}
	} else {
		if (m_callbackElse != nullptr) {
			return m_callbackElse(*this);
		}
	}
	return true;
}

uint32_t zeus::Promise::getTransactionId() const {
	return m_transactionId;
}

uint32_t zeus::Promise::getSource() const {
	return m_source;
}

bool zeus::Promise::hasError() const {
	if (m_message == nullptr) {
		return true;
	}
	if (m_message->getType() != zeus::message::Message::typeMessage::answer) {
		return true;
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->hasError();
}

std::string zeus::Promise::getErrorType() const {
	if (m_message == nullptr) {
		return "NULL_PTR";
	}
	if (m_message->getType() != zeus::message::Message::typeMessage::answer) {
		return "NOT_ANSWER_MESSAGE";
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->getError();
}

std::string zeus::Promise::getErrorHelp() const {
	if (m_message == nullptr) {
		return "This is a nullptr future";
	}
	if (m_message->getType() != zeus::message::Message::typeMessage::answer) {
		return "This answer is not a anwser type";
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->getErrorHelp();
}


bool zeus::Promise::isFinished() const {
	if (m_message == nullptr) {
		// in this case, we are waiting for an answer that the first packet is not arrived
		return false;
	}
	return m_message->getPartFinish();
}

const zeus::Promise& zeus::Promise::wait() const {
	while (isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
	}
	return *this;
}

const zeus::Promise& zeus::Promise::waitFor(echrono::Duration _delta) const {
	echrono::Steady start = echrono::Steady::now();
	while (    echrono::Steady::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
		start = echrono::Steady::now();
	}
	return *this;
}

const zeus::Promise& zeus::Promise::waitUntil(echrono::Steady _endTime) const {
	while (    echrono::Steady::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
	}
	return *this;
}

