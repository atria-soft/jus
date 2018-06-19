/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/ObjectRemote.hpp>
#include <zeus/Client.hpp>

#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::ObjectRemoteBase);

zeus::ObjectRemoteBase::ObjectRemoteBase(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _localId, uint16_t _localObjectId, uint32_t _address, const etk::String& _type):
  zeus::WebObj(_iface, _localId, _localObjectId),
  m_type(_type),
  m_remoteAddress(_address),
  m_isLinked(false) {
	m_isLinked = true;
	ZEUS_VERBOSE("[" << m_id << "/" << m_objectId << "] create => to remote [" << (m_remoteAddress>>16) << "/" << (m_remoteAddress&0xFFFF) << "]");
}

void zeus::ObjectRemoteBase::display() {
	ZEUS_VERBOSE("    - [" << m_id << "/" << m_objectId << "] => [" << (m_remoteAddress>>16) << "/" << (m_remoteAddress&0xFFFF) << "]");
	for (auto &it : m_listRemoteConnected) {
		ZEUS_INFO("        * [" << (it>>16) << "/" << (it&0xFFFF) << "]");
	}
}

zeus::ObjectRemoteBase::~ObjectRemoteBase() {
	ZEUS_VERBOSE("[" << m_id << "/" << m_objectId << "] DESTROY => to remote [" << (m_remoteAddress>>16) << "/" << (m_remoteAddress&0xFFFF) << "]");
	if (m_isLinked == true) {
		zeus::Future<bool> ret = m_interfaceWeb->call(getFullId(), m_remoteAddress&0xFFFF0000, "unlink", m_remoteAddress);
		ret.waitFor(echrono::seconds(1));
		if (ret.hasError() == true) {
			ZEUS_WARNING("return call error: " << ret.getErrorType() << " help:" << ret.getErrorHelp());
			ZEUS_WARNING("Can not unlink with the object id: " << (m_remoteAddress>>16) << "/" << (m_remoteAddress&0xFFFF) << " ==> link error");
			return;
		}
		m_isLinked = false;
	}
}

bool zeus::ObjectRemoteBase::exist() const {
	return m_isLinked;
}

const etk::String& zeus::ObjectRemoteBase::getName() const {
	return m_type;
}

zeus::ObjectRemote::ObjectRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _interface):
  m_interface(_interface) {
	if (m_interface == null) {
		return;
	}
}

bool zeus::ObjectRemote::exist() const {
	if (m_interface == null) {
		return false;
	}
	return m_interface->m_isLinked;
}

