/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
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
			zeus::ObjectRemote m_obj; //!< Service instance handle
		public:
			zeus::ObjectIntrospect sys;
		public:
			const Proxy& operator= (const zeus::ObjectRemote& _srv);
			Proxy();
			virtual ~Proxy() = default;
			Proxy(const zeus::ObjectRemote& _srv);
			bool exist() const;
	};
}
