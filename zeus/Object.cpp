/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/Object.hpp>
#include <zeus/debug.hpp>
#include <etk/stdTools.hpp>
#include <enet/TcpClient.hpp>
#include <zeus/Client.hpp>

#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::Object);


zeus::Object::Object(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _objectId) :
  zeus::RemoteProcessCall(_iface, _iface->getAddress(), _objectId) {
	
}

void zeus::Object::receive(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == null) {
		return;
	}
	if (_value->getPartFinish() == false) {
		ZEUS_ERROR("call a receive function with a not finished message ...");
		return;
	}
	if (_value->getType() == zeus::message::type::event) {
		ZEUS_ERROR("Unknow event: '...'");
		return;
	}
	if (_value->getType() == zeus::message::type::answer) {
		ZEUS_ERROR("Local Answer: '...'");
		return;
	}
	if (_value->getType() == zeus::message::type::data) {
		ZEUS_ERROR("Local DATA: '...'");
		return;
	}
	if (_value->getType() == zeus::message::type::call) {
		ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
		uint32_t source = callObj->getSource();
		uint32_t sourceId = callObj->getSourceId();
		etk::String callFunction = callObj->getCall();
		if (isFunctionAuthorized(sourceId, callFunction) == true) {
			callBinary(callFunction, callObj);
			return;
		} else {
			m_interfaceWeb->answerError(callObj->getTransactionId(), getFullId(), source, "NOT-AUTHORIZED-FUNCTION", "");
			return;
		}
	}
}
