/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ObjectRemote.hpp>
#include <zeus/Client.hpp>



zeus::ObjectRemoteBase::ObjectRemoteBase(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _localId, uint16_t _localObjectId, uint32_t _address, const std::string& _type):
  zeus::WebObj(_iface, _localId, _localObjectId),
  m_type(_type),
  m_remoteAddress(_address),
  m_isLinked(false) {
	m_isLinked = true;
}

zeus::ObjectRemoteBase::~ObjectRemoteBase() {
	if (m_isLinked == true) {
		uint32_t tmpLocalService = m_remoteAddress;
		// little hack : Call the service manager with the service ID=0 ...
		m_remoteAddress = 0;
		zeus::Future<bool> ret = m_interfaceWeb->call(getFullId(), m_remoteAddress, "unlink", tmpLocalService);
		ret.wait();
		if (ret.hasError() == true) {
			ZEUS_WARNING("Can not unlink with the service id: '" << tmpLocalService << "' ==> link error");
			m_remoteAddress = tmpLocalService;
			return;
		}
		if (ret.get() == true) {
			m_isLinked = false;
		} else {
			ZEUS_ERROR("Can not unlink with this service ....");
			m_remoteAddress = tmpLocalService;
		}
	}
}

bool zeus::ObjectRemoteBase::exist() const {
	return m_isLinked;
}

const std::string& zeus::ObjectRemoteBase::getName() const {
	return m_type;
}

zeus::ObjectRemote::ObjectRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _interface):
  m_interface(_interface) {
	if (m_interface == nullptr) {
		return;
	}
}

zeus::ObjectRemote::~ObjectRemote() {
	
}

bool zeus::ObjectRemote::exist() const {
	if (m_interface == nullptr) {
		return false;
	}
	return m_interface->m_isLinked;
}

