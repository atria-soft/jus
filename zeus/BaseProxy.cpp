/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/BaseProxy.hpp>
#include <zeus/debug.hpp>

const zeus::BaseProxy& zeus::BaseProxy::operator= (const zeus::ServiceRemote& _obj) {
	m_obj = _obj;
	return *this;
}
zeus::BaseProxy::BaseProxy():
  sys(m_obj),
  srv(m_obj) {
	
}
zeus::BaseProxy::BaseProxy(const zeus::ServiceRemote& _obj):
  m_obj(_obj),
  sys(m_obj),
  srv(m_obj) {
	
}
bool zeus::BaseProxy::exist() const {
	return m_obj.exist();
}
