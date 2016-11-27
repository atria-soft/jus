/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/ServiceRemote.hpp>
#include <zeus/ServiceProxy.hpp>
#include <zeus/SystemProxy.hpp>

namespace zeus {
	/**
	 * @brief Proxy base interface
	 */
	class BaseProxy {
		protected:
			zeus::ServiceRemote m_srv; //!< Service instance handle
		public:
			zeus::SystemProxy sys;
			zeus::ServiceProxy srv;
		public:
			const BaseProxy& operator= (const zeus::ServiceRemote& _srv);
			BaseProxy();
			virtual ~BaseProxy() = default;
			BaseProxy(const zeus::ServiceRemote& _srv);
			bool exist() const;
	};
}
