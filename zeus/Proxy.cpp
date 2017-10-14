/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/Proxy.hpp>
#include <zeus/debug.hpp>


#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::Proxy);

const zeus::Proxy& zeus::Proxy::operator= (const zeus::ObjectRemote& _obj) {
	m_obj = _obj;
	return *this;
}

zeus::Proxy::Proxy():
  sys(m_obj) {
	
}

zeus::Proxy::Proxy(const zeus::ObjectRemote& _obj):
  m_obj(_obj),
  sys(m_obj) {
	
}

bool zeus::Proxy::exist() const {
	return m_obj.exist();
}
