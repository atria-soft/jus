/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ServiceRemote.hpp>
#include <zeus/Client.hpp>



zeus::ServiceRemoteBase::ServiceRemoteBase(ememory::SharedPtr<zeus::WebServer> _clientLink, const std::string& _name, uint16_t _localId, uint16_t _localObjectId):
  zeus::WebObj(_clientLink, _localId, _localObjectId),
  m_name(_name),
  m_serviceId(0),
  m_isLinked(false) {
	if (m_interfaceWeb == nullptr) {
		return;
	}
	// little hack : Call the service manager with the service ID=0 ...
	zeus::Future<uint32_t> ret = m_interfaceWeb->call(getFullId(), ZEUS_GATEWAY_ADDRESS, "link", _name);
	ret.wait();
	if (ret.hasError() == true) {
		ZEUS_WARNING("Can not link with the service named: '" << _name << "' ==> link error");
		return;
	}
	m_isLinked = true;
	m_serviceId = ret.get();
}

zeus::ServiceRemoteBase::~ServiceRemoteBase() {
	if (m_isLinked == true) {
		uint32_t tmpLocalService = m_serviceId;
		// little hack : Call the service manager with the service ID=0 ...
		m_serviceId = 0;
		zeus::Future<bool> ret = m_interfaceWeb->call(getFullId(), m_serviceId, "unlink", tmpLocalService);
		ret.wait();
		if (ret.hasError() == true) {
			ZEUS_WARNING("Can not unlink with the service id: '" << tmpLocalService << "' ==> link error");
			m_serviceId = tmpLocalService;
			return;
		}
		if (ret.get() == true) {
			m_isLinked = false;
		} else {
			ZEUS_ERROR("Can not unlink with this service ....");
			m_serviceId = tmpLocalService;
		}
	}
}

bool zeus::ServiceRemoteBase::exist() const {
	return m_isLinked;
}

const std::string& zeus::ServiceRemoteBase::getName() const {
	return m_name;
}

zeus::ServiceRemote::ServiceRemote(ememory::SharedPtr<zeus::ServiceRemoteBase> _interface):
  m_interface(_interface) {
	if (m_interface == nullptr) {
		return;
	}
}

zeus::ServiceRemote::~ServiceRemote() {
	
}

bool zeus::ServiceRemote::exist() const {
	if (m_interface == nullptr) {
		return false;
	}
	return m_interface->m_isLinked;
}

