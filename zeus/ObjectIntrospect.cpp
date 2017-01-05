/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/ObjectIntrospect.hpp>

zeus::ObjectIntrospect::ObjectIntrospect(zeus::ObjectRemote& _srv):
  m_srv(_srv) {
	
}

zeus::Future<std::string> zeus::ObjectIntrospect::getDescription() {
	return m_srv.call("sys.getDescription");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getVersion() {
	return m_srv.call("sys.getVersion");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getType() {
	return m_srv.call("sys.getType");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getExtention() {
	return m_srv.call("sys.getExtention");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getAuthors() {
	return m_srv.call("sys.getAuthors");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getFunctions() {
	return m_srv.call("sys.getFunctions");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getFunctionPrototype(std::string _functionName) {
	return m_srv.call("sys.getFunctionPrototype", _functionName);
}

zeus::Future<std::string> zeus::ObjectIntrospect::getFunctionDescription(std::string _functionName) {
	return m_srv.call("sys.getFunctionDescription", _functionName);
}

