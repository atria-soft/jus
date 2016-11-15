/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureBase.hpp>

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
				m_data = _base.m_data;
				return *this;
			}
			/**
			 * @brief Get the value Requested with the type
			 * @return requested value
			 */
			ZEUS_RETURN get();
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
				m_data = _base.m_data;
				return *this;
			}
	};
}