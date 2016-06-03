/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/ServiceRemote.h>
#include <jus/Client.h>

jus::ServiceRemote::ServiceRemote(jus::Client* _clientInterface, const std::string& _name):
  m_clientInterface(_clientInterface),
  m_name(_name) {
	m_isLinked = m_clientInterface->link(_name);
}

jus::ServiceRemote::~ServiceRemote() {
	if (m_isLinked == true) {
		m_clientInterface->unlink(m_name);
		m_isLinked = false;
	}
}

bool jus::ServiceRemote::exist() {
	return m_isLinked;
}

uint64_t jus::ServiceRemote::getId() {
	return m_clientInterface->getId();
}

jus::FutureBase jus::ServiceRemote::callJson(uint64_t _transactionId,
                                             const ejson::Object& _obj,
                                             const std::vector<ActionAsyncClient>& _async,
                                             jus::FutureData::ObserverFinish _callback) {
	return m_clientInterface->callJson(_transactionId, _obj, _async, _callback, m_name);
}

