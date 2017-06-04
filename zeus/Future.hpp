/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureBase.hpp>

// TODO : When do a cast of the type on the future ==> do a wait ...

namespace zeus {
	class void_tmp {};
	template <class ZEUS_RETURN>
	ZEUS_RETURN futureGetValue(ememory::SharedPtr<zeus::Promise>& _promise);
	/**
	 * @brief future template to cast type in a specific type
	 */
	template<class ZEUS_RETURN, class ZEUS_EVENT=void_tmp>
	class Future : public zeus::FutureBase {
		public:
			/**
			 * @brief contructor of the Future with the basic FutureBase
			 * @param[in] _base Generic base Future
			 */
			Future(const zeus::FutureBase& _base):
			  zeus::FutureBase(_base) {
				
			}
			/**
			 * @brief Asignement operator with an other future
			 * @param[in] _base Generic base Future
			 * @return the reference on the local element
			 */
			zeus::Future<ZEUS_RETURN, ZEUS_EVENT>& operator= (const zeus::FutureBase& _base) {
				m_promise = _base.m_promise;
				return *this;
			}
			/**
			 * @brief Get the value Requested with the type
			 * @return requested value
			 */
			ZEUS_RETURN get() {
				return zeus::futureGetValue<ZEUS_RETURN>(m_promise);
			}
			const Future<ZEUS_RETURN, ZEUS_EVENT>& wait() const {
				zeus::FutureBase::wait();
				return *this;
			}
			Future<ZEUS_RETURN, ZEUS_EVENT>& wait() {
				zeus::FutureBase::wait();
				return *this;
			}
			const Future<ZEUS_RETURN, ZEUS_EVENT>& waitFor(echrono::Duration _delta = echrono::seconds(30)) const {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			Future<ZEUS_RETURN, ZEUS_EVENT>& waitFor(echrono::Duration _delta = echrono::seconds(30)) {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			const Future<ZEUS_RETURN, ZEUS_EVENT>& waitUntil(echrono::Steady _endTime) const {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			Future<ZEUS_RETURN, ZEUS_EVENT>& waitUntil(echrono::Steady _endTime) {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			using ObserverFut = std::function<bool(zeus::Future<ZEUS_RETURN, ZEUS_EVENT>)>; //!< Define an Observer: function pointer for the local specific Future
			/**
			 * @brief Attach callback on all return type of value
			 * @param[in] _callback Handle on the function to call in all case
			 */
			Future<ZEUS_RETURN, ZEUS_EVENT>& andAll(ObserverFut _callback) {
				zeus::FutureBase::andAll(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<ZEUS_RETURN>(_fut));
				    });
				return *this;
			}
			/**
			 * @brief Attach callback on a specific return action (SUCESS)
			 * @param[in] _callback Handle on the function to call in case of sucess on the call
			 */
			/*
			note : this is anbigous ???
			Future<ZEUS_RETURN, ZEUS_EVENT>& andThen(std::function<bool(const ZEUS_RETURN&)> _callback) {
				zeus::FutureBase::andThen(
				    [=](zeus::FutureBase _fut) {
				    	zeus::Future<ZEUS_RETURN> tmp(_fut);
				    	return _callback(tmp.get());
				    });
				return *this;
			}
			*/
			Future<ZEUS_RETURN, ZEUS_EVENT>& andThen(std::function<bool(ZEUS_RETURN)> _callback) {
				zeus::FutureBase::andThen(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(std::move(zeus::Future<ZEUS_RETURN>(_fut).get()));
				    });
				return *this;
			}
			/*
			Future<ZEUS_RETURN, ZEUS_EVENT>& andThen(ObserverFut _callback) {
				zeus::FutureBase::andThen(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<ZEUS_RETURN>(_fut));
				    });
				return *this;
			}*/
			/**
			 * @brief Attach callback on a specific return action (ERROR)
			 * @param[in] _callback Handle on the function to call in case of error on the call
			 */
			/*
			Future<ZEUS_RETURN, ZEUS_EVENT>& andElse(ObserverFut _callback) {
				zeus::FutureBase::andElse(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<ZEUS_RETURN>(_fut));
				    });
				return *this;
			}
			*/
			Future<ZEUS_RETURN, ZEUS_EVENT>& andElse(std::function<bool(const std::string&, const std::string&)> _callback) {
				zeus::FutureBase::andElse(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(_fut.getErrorType(), _fut.getErrorHelp());
				    });
				return *this;
			}
			/**
			 * @brief Attach callback on activity of the action if user set some return information
			 * @param[in] _callback Handle on the function to call in progress information
			 */
			// TODO: this is deprecated ...
			Future<ZEUS_RETURN, ZEUS_EVENT>& onProgress(Promise::ObserverEvent _callback) {
				zeus::FutureBase::onEvent(_callback);
				return *this;
			}
			//template<typename = std::enable_if<std::is_void<ZEUS_EVENT>::value, false>>
			/*
			Future<ZEUS_RETURN, ZEUS_EVENT>& onSignal(std::function<void(const ZEUS_EVENT&)> _callback) {
				zeus::FutureBase::onEvent(
				    [=](ememory::SharedPtr<zeus::message::Event> _msg) {
				    	if (_msg == nullptr) {
				    		return;
				    	}
				    	_callback(_msg->getEvent<ZEUS_EVENT>());
				    });
				return *this;
			}
			*/
			//template<typename = std::enable_if<std::is_void<ZEUS_EVENT>::value, false>>
			Future<ZEUS_RETURN, ZEUS_EVENT>& onSignal(std::function<void(ZEUS_EVENT)> _callback) {
				zeus::FutureBase::onEvent(
				    [=](ememory::SharedPtr<zeus::message::Event> _msg) {
				    	if (_msg == nullptr) {
				    		return;
				    	}
				    	_callback(std::move(_msg->getEvent<ZEUS_EVENT>()));
				    });
				return *this;
			}
	};
	/**
	 * @brief future template to cast type in a void methode (fallback)
	 */
	template<class ZEUS_EVENT>
	class Future<void, ZEUS_EVENT> : public zeus::FutureBase {
		public:
			/**
			 * @brief contructor of the Future with the basic FutureBase
			 * @param[in] _base Generic base Future
			 */
			Future(const zeus::FutureBase& _base):
			  zeus::FutureBase(_base) {
				
			}
			/**
			 * @brief Asignement operator with an other future
			 * @param[in] _base Generic base Future
			 * @return the reference on the local element
			 */
			zeus::Future<void, ZEUS_EVENT>& operator= (const zeus::FutureBase& _base) {
				m_promise = _base.m_promise;
				return *this;
			}
			const Future<void, ZEUS_EVENT>& wait() const {
				zeus::FutureBase::wait();
				return *this;
			}
			Future<void, ZEUS_EVENT>& wait() {
				zeus::FutureBase::wait();
				return *this;
			}
			const Future<void, ZEUS_EVENT>& waitFor(echrono::Duration _delta = echrono::seconds(30)) const {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			Future<void, ZEUS_EVENT>& waitFor(echrono::Duration _delta = echrono::seconds(30)) {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			const Future<void, ZEUS_EVENT>& waitUntil(echrono::Steady _endTime) const {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			Future<void, ZEUS_EVENT>& waitUntil(echrono::Steady _endTime) {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			using ObserverFut = std::function<bool(zeus::Future<void, ZEUS_EVENT>)>; //!< Define an Observer: function pointer for the local specific Future
			/**
			 * @brief Attach callback on all return type of value
			 * @param[in] _callback Handle on the function to call in all case
			 */
			Future<void, ZEUS_EVENT>& andAll(ObserverFut _callback) {
				zeus::FutureBase::andAll(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<void>(_fut));
				    });
				return *this;
			}
			/**
			 * @brief Attach callback on a specific return action (SUCESS)
			 * @param[in] _callback Handle on the function to call in case of sucess on the call
			 */
			/*
			Future<void, ZEUS_EVENT>& andThen(ObserverFut _callback) {
				zeus::FutureBase::andThen(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<void>(_fut));
				    });
				return *this;
			}
			*/
			Future<void, ZEUS_EVENT>& andThen(std::function<bool()> _callback) {
				zeus::FutureBase::andThen(
				    [=](zeus::FutureBase _fut) {
				    	return _callback();
				    });
				return *this;
			}
			/**
			 * @brief Attach callback on a specific return action (ERROR)
			 * @param[in] _callback Handle on the function to call in case of error on the call
			 */
			/*
			Future<void, ZEUS_EVENT>& andElse(ObserverFut _callback) {
				zeus::FutureBase::andElse(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(zeus::Future<void>(_fut));
				    });
				return *this;
			}
			*/
			Future<void, ZEUS_EVENT>& andElse(std::function<bool(const std::string&, const std::string&)> _callback) {
				zeus::FutureBase::andElse(
				    [=](zeus::FutureBase _fut) {
				    	return _callback(_fut.getErrorType(), _fut.getErrorHelp());
				    });
				return *this;
			}
			/**
			 * @brief Attach callback on activity of the action if user set some return information
			 * @param[in] _callback Handle on the function to call in progress information
			 */
			/*
			Future<void, ZEUS_EVENT>& onEvent(Promise::ObserverEvent _callback) {
				zeus::FutureBase::onEvent(_callback);
				return *this;
			}
			*/
			Future<void, ZEUS_EVENT>& onSignal(std::function<void(ZEUS_EVENT)> _callback) {
				zeus::FutureBase::onEvent(
				    [=](ememory::SharedPtr<zeus::message::Event> _msg) {
				    	if (_msg == nullptr) {
				    		return;
				    	}
				    	_callback(std::move(_msg->getEvent<ZEUS_EVENT>()));
				    });
				return *this;
			}
	};
}
