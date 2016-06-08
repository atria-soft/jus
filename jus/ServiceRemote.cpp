/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/ServiceRemote.h>
#include <jus/Client.h>

jus::ServiceRemote::ServiceRemote(jus::Client* _clientInterface, const std::string& _name):
  m_clientInterface(_clientInterface),
  m_name(_name),
  m_serviceId(0) {
	int32_t val = m_clientInterface->link(_name);
	if (val >= 0) {
		m_isLinked = true;
		m_serviceId = val;
	} else {
		m_isLinked = false;
		m_serviceId = 0;
	}
}

jus::ServiceRemote::~ServiceRemote() {
	if (m_isLinked == true) {
		m_clientInterface->unlink(m_serviceId);
		m_isLinked = false;
	}
}

bool jus::ServiceRemote::exist() {
	return m_isLinked;
}

uint64_t jus::ServiceRemote::getId() {
	return m_clientInterface->getId();
}

enum jus::connectionMode jus::ServiceRemote::getMode() {
	return m_clientInterface->getMode();
}

jus::FutureBase jus::ServiceRemote::callJson(uint64_t _transactionId,
                                             const ejson::Object& _obj,
                                             const std::vector<ActionAsyncClient>& _async,
                                             jus::FutureData::ObserverFinish _callback) {
	return m_clientInterface->callJson(_transactionId, _obj, _async, _callback, m_serviceId);
}

jus::FutureBase jus::ServiceRemote::callBinary(uint64_t _transactionId,
                                               const jus::Buffer& _obj,
                                               const std::vector<ActionAsyncClient>& _async,
                                               jus::FutureData::ObserverFinish _callback) {
	return m_clientInterface->callBinary(_transactionId, _obj, _async, _callback, m_serviceId);
}


