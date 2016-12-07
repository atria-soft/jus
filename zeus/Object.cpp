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



zeus::Object::Object(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId) :
  zeus::RemoteProcessCall(_iface, _iface->getAddress(), _objectId) {
	
}


zeus::Object::~Object() {
	
}


void zeus::Object::receive(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	ZEUS_WARNING("BUFFER" << _value);
	uint32_t tmpID = _value->getTransactionId();
	uint32_t source = _value->getSource();
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

void zeus::Object::callBinary(ememory::SharedPtr<zeus::Message> _obj) {
	if (_obj == nullptr) {
		return;
	}
	if (_obj->getType() == zeus::message::type::event) {
		ZEUS_ERROR("Unknow event: '...'");
		return;
	}
	if (_obj->getType() == zeus::message::type::answer) {
		ZEUS_ERROR("Local Answer: '...'");
		return;
	}
	if (_obj->getType() == zeus::message::type::call) {
		ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_obj);
		uint32_t source = callObj->getSource();
		uint32_t sourceId = callObj->getSourceId();
		std::string callFunction = callObj->getCall();
		if (isFunctionAuthorized(sourceId, callFunction) == true) {
			callBinary2(callFunction, callObj);
			return;
		} else {
			m_interfaceWeb->answerError(callObj->getTransactionId(), getFullId(), source, "NOT-AUTHORIZED-FUNCTION", "");
			return;
		}
	}
}


