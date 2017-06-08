/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <zeus/Promise.hpp>
#include <zeus/FutureBase.hpp>
#include <zeus/message/Answer.hpp>
#include <zeus/message/Event.hpp>
#include <zeus/debug.hpp>
#include <zeus/WebServer.hpp>


zeus::Promise::Promise(uint32_t _transactionId, uint32_t _source) {
	m_sendTime = echrono::Steady::now();
	m_transactionId = _transactionId;
	m_source = _source;
}

ememory::SharedPtr<zeus::Message> zeus::Promise::getRaw() {
	return m_message;
}

zeus::Promise::Promise(uint32_t _transactionId, ememory::SharedPtr<zeus::Message> _returnData, uint32_t _source) {
	m_sendTime = echrono::Steady::now();
	m_transactionId = _transactionId;
	m_source = _source;
	m_message = _returnData;
	if (isFinished() == true) {
		m_receiveTime = echrono::Steady::now();
	}
}

void zeus::Promise::remoteObjectDestroyed() {
	auto answer = zeus::message::Answer::create(nullptr);
	if (answer == nullptr) {
		return;
	}
	answer->setTransactionId(m_transactionId);
	answer->setSource(m_source);
	answer->setDestination(0);
	answer->setPartFinish(true);
	answer->addError("REMOTE-OBJECT-REMOVE", "The remote interface ot the Object has been destroyed");
	setMessage(answer);
}

void zeus::Promise::setAction() {
	m_isAction = true;
}

void zeus::Promise::andAll(zeus::Promise::Observer _callback) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_callbackThen = _callback;
		m_callbackElse = _callback;
	}
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_callbackThen != nullptr) {
			m_callbackThen(zeus::FutureBase(sharedFromThis()));
		}
	} else {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_callbackElse != nullptr) {
			m_callbackElse(zeus::FutureBase(sharedFromThis()));
		}
	}
}

void zeus::Promise::andThen(zeus::Promise::Observer _callback) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_callbackThen = _callback;
	}
	if (isFinished() == false) {
		return;
	}
	if (hasError() == true) {
		return;
	}
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_callbackThen == nullptr) {
		return;
	}
	m_callbackThen(zeus::FutureBase(sharedFromThis()));
}

void zeus::Promise::andElse(zeus::Promise::Observer _callback) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_callbackElse = _callback;
	}
	if (isFinished() == false) {
		return;
	}
	if (hasError() == false) {
		return;
	}
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_callbackElse == nullptr) {
		return;
	}
	m_callbackElse(zeus::FutureBase(sharedFromThis()));
}

void zeus::Promise::onEvent(zeus::Promise::ObserverEvent _callback) {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_isAction == false) {
		ZEUS_ERROR("Request a Event calback on a simple function call");
	}
	m_callbackEvent = _callback;
}

echrono::Duration zeus::Promise::getTransmitionTime() const {
	if (isFinished() == false) {
		return echrono::nanoseconds(0);
	}
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_receiveTime - m_sendTime;
}

bool zeus::Promise::setMessage(ememory::SharedPtr<zeus::Message> _value) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_receiveTime = echrono::Steady::now();
	}
	if (_value->getType() == zeus::message::type::event) {
		std::unique_lock<std::mutex> lock(m_mutex);
		// notification of a progresion ...
		if (m_callbackEvent != nullptr) {
			if (_value == nullptr) {
				return true;
			}
			m_callbackEvent(ememory::staticPointerCast<zeus::message::Event>(_value));
			return false; // no error
		}
		return false;
	}
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_message = _value;
		if (m_message == nullptr) {
			return true;
		}
		if (m_message->getPartFinish() == false) {
			ZEUS_ERROR("set buffer that is not finished ...");
			return false;
		}
	}
	if (hasError() == false) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_callbackThen != nullptr) {
			return m_callbackThen(zeus::FutureBase(sharedFromThis()));
		}
	} else {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_callbackElse != nullptr) {
			return m_callbackElse(zeus::FutureBase(sharedFromThis()));
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
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_message == nullptr) {
		return true;
	}
	if (m_message->getType() != zeus::message::type::answer) {
		return true;
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->hasError();
}

std::string zeus::Promise::getErrorType() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_message == nullptr) {
		return "NULL_PTR";
	}
	if (m_message->getType() != zeus::message::type::answer) {
		return "NOT_ANSWER_MESSAGE";
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->getError();
}

std::string zeus::Promise::getErrorHelp() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_message == nullptr) {
		return "This is a nullptr future";
	}
	if (m_message->getType() != zeus::message::type::answer) {
		return "This answer is not a anwser type";
	}
	return static_cast<const zeus::message::Answer*>(m_message.get())->getErrorHelp();
}


bool zeus::Promise::isFinished() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_message == nullptr) {
		// in this case, we are waiting for an answer that the first packet is not arrived
		return false;
	}
	return m_message->getPartFinish();
}

void zeus::Promise::wait() const {
	while (isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
	}
}

void zeus::Promise::waitFor(echrono::Duration _delta) const {
	echrono::Steady start = echrono::Steady::now();
	while (    echrono::Steady::now() - start < _delta
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
	}
	if (isFinished() == false) {
		ZEUS_WARNING("Wait timeout ... " << _delta);
		//elog::displayBacktrace();
	}
}

void zeus::Promise::waitUntil(echrono::Steady _endTime) const {
	while (    echrono::Steady::now() < _endTime
	        && isFinished() == false) {
		// TODO : Do it better ... like messaging/mutex_locked ...
		std::this_thread::sleep_for(echrono::milliseconds(10));
	}
	if (isFinished() == false) {
		ZEUS_WARNING("Wait timeout ..." << _endTime);
		//elog::displayBacktrace();
	}
}

