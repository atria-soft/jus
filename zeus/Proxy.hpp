/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/ObjectRemote.hpp>
#include <zeus/ObjectIntrospect.hpp>

namespace zeus {
	/**
	 * @brief Proxy base interface
	 */
	class Proxy {
		protected:
			zeus::ObjectRemote m_obj; //!< Object instance handle
		public:
			zeus::ObjectIntrospect sys; //!< publish introspection function call
		public:
			/**
			 * @brief a simple proxy constructor
			 */
			Proxy();
			/** 
			 * @brief Contructor of a proxy
			 * @param[in] _obj an the object remote handle
			 */
			
			Proxy(const zeus::ObjectRemote& _obj);
			/** 
			 * @brief Set operator
			 * @param[in] _obj an other service remote
			 * @return A reference on this proxy.
			 */
			const Proxy& operator= (const zeus::ObjectRemote& _obj);
			/**
			 * @brief set destructor VIRTUAL
			 */
			virtual ~Proxy() = default;
			/**
			 * @brief Permit to check if the remote hss been corectly created
			 * @return true The object exist
			 * @return false The object is NOT accessible
			 */
			bool exist() const;
	};
}
