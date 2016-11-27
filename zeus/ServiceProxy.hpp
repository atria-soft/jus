/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/ServiceRemote.hpp>

namespace zeus {
	class ServiceProxy {
		protected:
			zeus::ServiceRemote& m_srv; //!< Service instance handle
		public:
			ServiceProxy(zeus::ServiceRemote& _srv);
			/**
			 * @brief Get the list of extention of this service
			 * @return List of all extention availlable
			 */
			virtual zeus::Future<std::vector<std::string>> getExtention();
	};
}
