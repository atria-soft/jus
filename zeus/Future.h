/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/FutureBase.h>

namespace jus {
	template<class JUS_RETURN>
	class Future : public jus::FutureBase {
		public:
			Future(const jus::FutureBase& _base):
			  jus::FutureBase(_base) {
				
			}
			jus::Future<JUS_RETURN>& operator= (const jus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
			JUS_RETURN get();
	};
	template<>
	class Future<void> : public jus::FutureBase {
		public:
			Future(const jus::FutureBase& _base):
			  jus::FutureBase(_base) {
				
			}
			jus::Future<void>& operator= (const jus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
	};
}
