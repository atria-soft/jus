/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ServiceRemote.h>
#include <zeus/Client.h>

zeus::ServiceRemote::ServiceRemote(ememory::SharedPtr<zeus::WebServer> _clientLink, const std::string& _name):
  m_interfaceClient(_clientLink),
  m_name(_name),
  m_serviceId(0),
  m_isLinked(false) {
	if (m_interfaceClient == nullptr) {
		return;
	}
	// little hack : Call the service manager with the service ID=0 ...
	zeus::Future<uint32_t> ret = call("link", _name);
	ret.wait();
	if (ret.hasError() == true) {
		ZEUS_WARNING("Can not link with the service named: '" << _name << "' ==> link error");
		return;
	}
	m_isLinked = true;
	m_serviceId = ret.get();
}

zeus::ServiceRemote::~ServiceRemote() {
	if (m_isLinked == true) {
		uint32_t tmpLocalService = m_serviceId;
		// little hack : Call the service manager with the service ID=0 ...
		m_serviceId = 0;
		zeus::Future<bool> ret = call("unlink", tmpLocalService);
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

bool zeus::ServiceRemote::exist() {
	return m_isLinked;
}

