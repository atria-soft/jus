/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureBase.h>

namespace zeus {
	template<class ZEUS_RETURN>
	class Future : public zeus::FutureBase {
		public:
			Future(const zeus::FutureBase& _base):
			  zeus::FutureBase(_base) {
				
			}
			zeus::Future<ZEUS_RETURN>& operator= (const zeus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
			ZEUS_RETURN get();
	};
	template<>
	class Future<void> : public zeus::FutureBase {
		public:
			Future(const zeus::FutureBase& _base):
			  zeus::FutureBase(_base) {
				
			}
			zeus::Future<void>& operator= (const zeus::FutureBase& _base) {
				m_data = _base.m_data;
				return *this;
			}
	};
}
