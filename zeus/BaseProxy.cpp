/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/BaseProxy.hpp>
#include <zeus/debug.hpp>

const zeus::BaseProxy& zeus::BaseProxy::operator= (const zeus::ServiceRemote& _srv) {
	m_srv = _srv;
	return *this;
}
zeus::BaseProxy::BaseProxy():
  sys(m_srv),
  srv(m_srv) {
	
}
zeus::BaseProxy::BaseProxy(const zeus::ServiceRemote& _srv):
  m_srv(_srv),
  sys(m_srv),
  srv(m_srv) {
	
}
bool zeus::BaseProxy::exist() const {
	return m_srv.exist();
}
