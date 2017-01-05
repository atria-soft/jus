/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureBase.hpp>

// TODO : When do a cast of the type on the future ==> do a wait ...

namespace zeus {
	/**
	 * @brief future template to cast type in a specific type
	 */
	template<class ZEUS_RETURN>
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
			zeus::Future<ZEUS_RETURN>& operator= (const zeus::FutureBase& _base) {
				m_promise = _base.m_promise;
				return *this;
			}
			/**
			 * @brief Get the value Requested with the type
			 * @return requested value
			 */
			ZEUS_RETURN get();
			const Future<ZEUS_RETURN>& wait() const {
				zeus::FutureBase::wait();
				return *this;
			}
			Future<ZEUS_RETURN>& wait() {
				zeus::FutureBase::wait();
				return *this;
			}
			const Future<ZEUS_RETURN>& waitFor(echrono::Duration _delta = echrono::seconds(30)) const {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			Future<ZEUS_RETURN>& waitFor(echrono::Duration _delta = echrono::seconds(30)) {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			const Future<ZEUS_RETURN>& waitUntil(echrono::Steady _endTime) const {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			Future<ZEUS_RETURN>& waitUntil(echrono::Steady _endTime) {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
	};
	/**
	 * @brief future template to cast type in a void methode (fallback)
	 */
	template<>
	class Future<void> : public zeus::FutureBase {
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
			zeus::Future<void>& operator= (const zeus::FutureBase& _base) {
				m_promise = _base.m_promise;
				return *this;
			}
			const Future<void>& wait() const {
				zeus::FutureBase::wait();
				return *this;
			}
			Future<void>& wait() {
				zeus::FutureBase::wait();
				return *this;
			}
			const Future<void>& waitFor(echrono::Duration _delta = echrono::seconds(30)) const {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			Future<void>& waitFor(echrono::Duration _delta = echrono::seconds(30)) {
				zeus::FutureBase::waitFor(_delta);
				return *this;
			}
			const Future<void>& waitUntil(echrono::Steady _endTime) const {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
			Future<void>& waitUntil(echrono::Steady _endTime) {
				zeus::FutureBase::waitUntil(_endTime);
				return *this;
			}
	};
}
