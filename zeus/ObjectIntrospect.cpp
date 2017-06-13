/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/ObjectIntrospect.hpp>

zeus::ObjectIntrospect::ObjectIntrospect(zeus::ObjectRemote& _obj):
  m_obj(_obj) {
	
}

zeus::Future<std::string> zeus::ObjectIntrospect::getDescription() {
	return m_obj.call("obj.getDescription");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getVersion() {
	return m_obj.call("obj.getVersion");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getType() {
	return m_obj.call("obj.getType");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getExtention() {
	return m_obj.call("obj.getExtention");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getAuthors() {
	return m_obj.call("obj.getAuthors");
}

zeus::Future<std::vector<std::string>> zeus::ObjectIntrospect::getFunctions() {
	return m_obj.call("obj.getFunctions");
}

zeus::Future<std::string> zeus::ObjectIntrospect::getFunctionPrototype(std::string _functionName) {
	return m_obj.call("obj.getFunctionPrototype", _functionName);
}

zeus::Future<std::string> zeus::ObjectIntrospect::getFunctionDescription(std::string _functionName) {
	return m_obj.call("obj.getFunctionDescription", _functionName);
}

