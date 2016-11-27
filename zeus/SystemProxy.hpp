/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/ServiceRemote.hpp>

namespace zeus {
	class SystemProxy {
		protected:
			zeus::ServiceRemote& m_srv; //!< Service instance handle
		public:
			SystemProxy(zeus::ServiceRemote& _srv);
			zeus::Future<std::string> getDescription();
			zeus::Future<std::string> getVersion();
			zeus::Future<std::string> getType();
			zeus::Future<std::vector<std::string>> getExtention();
			zeus::Future<std::vector<std::string>> getAuthors();
			zeus::Future<std::vector<std::string>> getFunctions();
			zeus::Future<std::string> getFunctionPrototype(std::string _functionName);
			zeus::Future<std::string> getFunctionDescription(std::string _functionName);
			
	};
}
