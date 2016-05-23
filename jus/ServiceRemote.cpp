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
	m_clientInterface->link(_name);
}

jus::ServiceRemote::~ServiceRemote() {
	m_clientInterface->unlink(m_name);
}


ejson::Object jus::ServiceRemote::createBaseCall(const std::string& _functionName) {
	return m_clientInterface->createBaseCall(_functionName, m_name);
}

ejson::Object jus::ServiceRemote::callJson(const ejson::Object& _obj) {
	return m_clientInterface->callJson(_obj);
}
