/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/SystemProxy.hpp>

zeus::SystemProxy::SystemProxy(zeus::ServiceRemote& _srv):
  m_srv(_srv) {
	
}

zeus::Future<std::string> zeus::SystemProxy::getDescription() {
	return m_srv.call("sys.getDescription");
}

zeus::Future<std::string> zeus::SystemProxy::getVersion() {
	return m_srv.call("sys.getVersion");
}

zeus::Future<std::string> zeus::SystemProxy::getType() {
	return m_srv.call("sys.getType");
}

zeus::Future<std::vector<std::string>> zeus::SystemProxy::getExtention() {
	return m_srv.call("sys.getExtention");
}

zeus::Future<std::vector<std::string>> zeus::SystemProxy::getAuthors() {
	return m_srv.call("sys.getAuthors");
}

zeus::Future<std::vector<std::string>> zeus::SystemProxy::getFunctions() {
	return m_srv.call("sys.getFunctions");
}

zeus::Future<std::string> zeus::SystemProxy::getFunctionPrototype(std::string _functionName) {
	return m_srv.call("sys.getFunctionPrototype", _functionName);
}

zeus::Future<std::string> zeus::SystemProxy::getFunctionDescription(std::string _functionName) {
	return m_srv.call("sys.getFunctionDescription", _functionName);
}

