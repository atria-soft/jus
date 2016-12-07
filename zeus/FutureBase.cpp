/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/FutureBase.hpp>
#include <zeus/message/Answer.hpp>
#include <zeus/debug.hpp>


zeus::FutureBase::FutureBase(const zeus::FutureBase& _base):
  m_promise(_base.m_promise) {
	
}

zeus::FutureBase::FutureBase() {
	m_promise = nullptr;
}

zeus::FutureBase::FutureBase(uint32_t _transactionId, uint32_t _source) {
	m_promise = ememory::makeShared<zeus::Promise>(_transactionId, _source);
}

ememory::SharedPtr<zeus::message::Message> zeus::FutureBase::getRaw() {
	if (m_promise == nullptr) {
		return nullptr;
	}
	return m_promise->getRaw();
}

zeus::FutureBase::FutureBase(uint32_t _transactionId, ememory::SharedPtr<zeus::message::Message> _returnData, uint32_t _source) {
	m_promise = ememory::makeShared<zeus::Promise>(_transactionId, _returnData, _source);
}

void zeus::FutureBase::andAll(zeus::Promise::Observer _callback) {
	if (m_promise == nullptr) {
		return;
	}
	m_promise->andAll(_callback);
}

void zeus::FutureBase::andThen(zeus::Promise::Observer _callback) {
	if (m_promise == nullptr) {
		return;
	}
	m_promise->andThen(_callback);
}

void zeus::FutureBase::andElse(zeus::Promise::Observer _callback) {
	if (m_promise == nullptr) {
		return;
	}
	m_promise->andElse(_callback);
}


echrono::Duration zeus::FutureBase::getTransmitionTime() const {
	if (m_promise == nullptr) {
		return echrono::nanoseconds(0);
	}
	return m_promise->getTransmitionTime();
}

zeus::FutureBase zeus::FutureBase::operator= (const zeus::FutureBase& _base) {
	m_promise = _base.m_promise;
	return *this;
}

bool zeus::FutureBase::setBuffer(ememory::SharedPtr<zeus::message::Message> _value) {
	if (m_promise == nullptr) {
		ZEUS_ERROR(" Not a valid future ...");
		return true;
	}
	return m_promise->setBuffer(_value);
}

uint32_t zeus::FutureBase::getTransactionId() const {
	if (m_promise == nullptr) {
		return 0;
	}
	return m_promise->getTransactionId();
}

uint32_t zeus::FutureBase::getSource() const {
	if (m_promise == nullptr) {
		return 0;
	}
	return m_promise->getSource();
}

bool zeus::FutureBase::hasError() const {
	if (m_promise == nullptr) {
		return true;
	}
	return m_promise->hasError();
}

std::string zeus::FutureBase::getErrorType() const {
	if (m_promise == nullptr) {
		return "NULL_PTR";
	}
	return m_promise->getErrorType();
}

std::string zeus::FutureBase::getErrorHelp() const {
	if (m_promise == nullptr) {
		return "This is a nullptr future";
	}
	m_promise->getErrorHelp();
}

bool zeus::FutureBase::isValid() const {
	return m_promise != nullptr;
}

bool zeus::FutureBase::isFinished() const {
	if (m_promise == nullptr) {
		return true;
	}
	return m_promise->isFinished();
}

const zeus::FutureBase& zeus::FutureBase::wait() const {
	if (m_promise == nullptr) {
		return *this;
	}
	m_promise.waitFor(echrono::seconds(5));
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitFor(echrono::Duration _delta) const {
	if (m_promise == nullptr) {
		return *this;
	}
	m_promise.waitFor(_delta);
	return *this;
}

const zeus::FutureBase& zeus::FutureBase::waitUntil(echrono::Steady _endTime) const {
	if (m_promise == nullptr) {
		return *this;
	}
	m_promise.waitUntil(_endTime);
	return *this;
}

