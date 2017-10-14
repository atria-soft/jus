/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/ObjectIntrospect.hpp>


#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::ObjectIntrospect);

zeus::ObjectIntrospect::ObjectIntrospect(zeus::ObjectRemote& _obj):
  m_obj(_obj) {
	
}

zeus::Future<etk::String> zeus::ObjectIntrospect::getDescription() {
	return m_obj.call("sys.getDescription");
}

zeus::Future<etk::String> zeus::ObjectIntrospect::getVersion() {
	return m_obj.call("sys.getVersion");
}

zeus::Future<etk::String> zeus::ObjectIntrospect::getType() {
	return m_obj.call("sys.getType");
}

zeus::Future<etk::Vector<etk::String>> zeus::ObjectIntrospect::getAuthors() {
	return m_obj.call("sys.getAuthors");
}

zeus::Future<etk::Vector<etk::String>> zeus::ObjectIntrospect::getFunctions() {
	return m_obj.call("sys.getFunctions");
}

zeus::Future<etk::String> zeus::ObjectIntrospect::getFunctionPrototype(etk::String _functionName) {
	return m_obj.call("sys.getFunctionPrototype", _functionName);
}

zeus::Future<etk::String> zeus::ObjectIntrospect::getFunctionDescription(etk::String _functionName) {
	return m_obj.call("sys.getFunctionDescription", _functionName);
}

