/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ServiceProxy.hpp>

zeus::ServiceProxy::ServiceProxy(zeus::ServiceRemote& _srv):
  m_srv(_srv) {
	
}
zeus::Future<std::vector<std::string>> zeus::ServiceProxy::getExtention() {
	return m_srv.call("srv.getExtention");
}

