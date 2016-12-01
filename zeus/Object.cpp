/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/Object.hpp>
#include <zeus/debug.hpp>
#include <etk/stdTools.hpp>
#include <enet/TcpClient.hpp>
#include <zeus/Client.hpp>



/*zeus::Object::Object(zeus::Client* _client, uint16_t _objectId) :
  zeus::RemoteProcessCall(_client->getWebInterface(), _client->m_localAddress, _objectId),
  m_clientId(_client->m_localAddress),
  m_objectId(_objectId) {
	/ *
	zeus::AbstractFunction* func = advertise("getExtention", &zeus::Object::getExtention);
	if (func != nullptr) {
		func->setDescription("Get List of availlable extention of this Object");
		func->setReturn("A list of extention register in the Object");
	}
	* /
}
*/
zeus::Object::Object(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId) :
  zeus::RemoteProcessCall(_iface, _iface->getAddress(), _objectId),
  m_clientId(_iface->getAddress()),
  m_objectId(_objectId) {
	
}


zeus::Object::~Object() {
	
}


void zeus::Object::receive(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	ZEUS_WARNING("BUFFER" << _value);
	uint32_t tmpID = _value->getTransactionId();
	uint32_t source = _value->getSource();
	if (_value->getType() == zeus::Buffer::typeMessage::data) {
		auto it = m_callMultiData.begin();
		while (it != m_callMultiData.end()) {
			if (    it->getTransactionId() == tmpID
			     && it->getSource() == source) {
				ZEUS_WARNING("Append data ... " << tmpID);
				it->appendData(_value);
				if (it->isFinished() == true) {
					ZEUS_WARNING("CALL Function ...");
					callBinary(it->getRaw());
					it = m_callMultiData.erase(it);
				}
				return;
			}
			++it;
		}
		ZEUS_ERROR("Un-associated data ...");
		return;
	}
	ZEUS_WARNING("direct call");
	zeus::FutureBase futData(tmpID, _value, source);
	if (futData.isFinished() == true) {
		ZEUS_INFO("Call Binary ..");
		callBinary(futData.getRaw());
	} else {
		ZEUS_INFO("ADD ...");
		m_callMultiData.push_back(futData);
	}
}

void zeus::Object::callBinary(ememory::SharedPtr<zeus::Buffer> _obj) {
	ZEUS_INFO("plop 1 ...");
	if (_obj == nullptr) {
		return;
	}
	ZEUS_INFO("plop 2 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::event) {
		ZEUS_ERROR("Unknow event: '...'");
		return;
	}
	ZEUS_INFO("plop 3 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::answer) {
		ZEUS_ERROR("Local Answer: '...'");
		return;
	}
	ZEUS_INFO("plop 4 ...");
	if (_obj->getType() == zeus::Buffer::typeMessage::call) {
		ZEUS_INFO("plop 5 ... ");
		ememory::SharedPtr<zeus::BufferCall> callObj = ememory::staticPointerCast<zeus::BufferCall>(_obj);
		uint32_t source = callObj->getSource();
		uint32_t sourceId = callObj->getSourceId();
		std::string callFunction = callObj->getCall();
		ZEUS_INFO("plop - ... " << callFunction);
		/*
		if (callFunction[0] == '_') {
			if (callFunction == "_new") {
				std::string userName = callObj->getParameter<std::string>(0);
				std::string clientName = callObj->getParameter<std::string>(1);
				std::vector<std::string> clientGroup = callObj->getParameter<std::vector<std::string>>(2);
				clientConnect(sourceId, userName, clientName, clientGroup);
			} else if (callFunction == "_delete") {
				clientDisconnect(sourceId);
			}
			m_interfaceWeb->answerValue(callObj->getTransactionId(), uint32_t(m_id)<<16, source, true);
			return;
		} else */if (isFunctionAuthorized(sourceId, callFunction) == true) {
			ZEUS_INFO("plop 6 ...");
			callBinary2(callFunction, callObj);
			return;
		} else {
			ZEUS_INFO("plop 7 ...");
			m_interfaceWeb->answerError(callObj->getTransactionId(), (uint32_t(m_clientId)<<16) + m_objectId, source, "NOT-AUTHORIZED-FUNCTION", "");
			return;
		}
	}
}


