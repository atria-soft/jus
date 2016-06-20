/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/ServiceRemote.h>
#include <jus/Client.h>

jus::ServiceRemote::ServiceRemote(ememory::SharedPtr<jus::TcpString> _clientLink, const std::string& _name):
  m_interfaceClient(_clientLink),
  m_name(_name),
  m_serviceId(0),
  m_isLinked(false) {
	if (m_interfaceClient == nullptr) {
		return;
	}
	// little hack : Call the service manager with the service ID=0 ...
	jus::Future<uint32_t> ret = call("link", _name);
	ret.wait();
	if (ret.hasError() == true) {
		JUS_WARNING("Can not link with the service named: '" << _name << "' ==> link error");
		return;
	}
	m_isLinked = true;
	m_serviceId = ret.get();
}

jus::ServiceRemote::~ServiceRemote() {
	if (m_isLinked == true) {
		uint32_t tmpLocalService = m_serviceId;
		// little hack : Call the service manager with the service ID=0 ...
		m_serviceId = 0;
		jus::Future<bool> ret = call("unlink", tmpLocalService);
		ret.wait();
		if (ret.hasError() == true) {
			JUS_WARNING("Can not unlink with the service id: '" << tmpLocalService << "' ==> link error");
			m_serviceId = tmpLocalService;
			return;
		}
		if (ret.get() == true) {
			m_isLinked = false;
		} else {
			JUS_ERROR("Can not unlink with this service ....");
			m_serviceId = tmpLocalService;
		}
	}
}

bool jus::ServiceRemote::exist() {
	return m_isLinked;
}

