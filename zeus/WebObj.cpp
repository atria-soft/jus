/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/WebObj.hpp>
#include <zeus/debug.hpp>

zeus::WebObj::WebObj(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId) :
  m_interfaceWeb(_iface),
  m_id(_id),
  m_objectId(_objectId) {
	ZEUS_VERBOSE("[" << m_id << "/" << m_objectId << "] Create");
}

zeus::WebObj::~WebObj() {
	ZEUS_VERBOSE("[" << m_id << "/" << m_objectId << "] Delete");
}

uint16_t zeus::WebObj::getInterfaceId() {
	return m_id;
}

uint16_t zeus::WebObj::getObjectId() {
	return m_objectId;
}

uint32_t zeus::WebObj::getFullId() {
	return (uint32_t(m_id) << 16 ) + m_objectId;
}

void zeus::WebObj::receive(ememory::SharedPtr<zeus::Message> _value) {
	ZEUS_ERROR("Receive a message ==> not implemented messaging ..." << _value);
}

void zeus::WebObj::display() {
	ZEUS_DEBUG("    - [" << m_id << "/" << m_objectId << "]");
	for (auto &it : m_listRemoteConnected) {
		ZEUS_DEBUG("        * [" << (it>>16) << "/" << (it&0xFFFF) << "]");
	}
}

void zeus::WebObj::addRemote(uint32_t _id) {
	m_listRemoteConnected.pushBack(_id);
}


bool zeus::WebObj::removeOwnership(uint32_t _id) {
	for (auto it = m_listRemoteConnected.begin();
	     it != m_listRemoteConnected.end();
	     ) {
		ZEUS_VERBOSE("    check ID : " << (*it>>16) << "/" << (*it&0xFFFF) << " == " << (_id>>16) << "/" << (_id&0xFFFF));
		if (*it == _id) {
			it = m_listRemoteConnected.erase(it);
			ZEUS_VERBOSE("        REMOVE ...");
			return true;
		}
		++it;
	}
	return false;
}
void zeus::WebObj::rmRemoteInterface(uint16_t _id) {
	for (auto it = m_listRemoteConnected.begin();
	     it != m_listRemoteConnected.end();
	     ) {
		if ((*it>>16) == _id) {
			it = m_listRemoteConnected.erase(it);
		} else {
			++it;
		}
	}
}

bool zeus::WebObj::transferOwnership(uint32_t _sourceAddress, uint32_t _destinataireAddress) {
	ZEUS_VERBOSE("    transfer onership : " << (_sourceAddress>>16) << "/" << (_sourceAddress&0xFFFF) << " ==> " << (_destinataireAddress>>16) << "/" << (_destinataireAddress&0xFFFF));
	for (auto &it : m_listRemoteConnected) {
		//ZEUS_INFO("      " << (it>>16) << "/" << (it&0xFFFF));
		if (it == _sourceAddress) {
			it = _destinataireAddress;
			return true;
		}
	}
	return false;
}

bool zeus::WebObj::haveRemoteConnected() const {
	return m_listRemoteConnected.size() != 0;
}

