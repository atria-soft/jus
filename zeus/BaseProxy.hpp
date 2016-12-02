/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/ObjectRemote.hpp>
#include <zeus/SystemProxy.hpp>

namespace zeus {
	/**
	 * @brief Proxy base interface
	 */
	class BaseProxy {
		protected:
			zeus::ObjectRemote m_obj; //!< Service instance handle
		public:
			zeus::SystemProxy sys;
		public:
			const BaseProxy& operator= (const zeus::ObjectRemote& _srv);
			BaseProxy();
			virtual ~BaseProxy() = default;
			BaseProxy(const zeus::ObjectRemote& _srv);
			bool exist() const;
	};
}
