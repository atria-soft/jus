/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/debug.hpp>
#include <zeus/message/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/message/Event.hpp>

void zeus::message::Event::generateDisplay(etk::Stream& _os) const {
	zeus::Message::generateDisplay(_os);
	if (getNumberParameter() != 0) {
		_os << " '" + simpleStringParam(0) + "'";
	}
}

bool zeus::message::Event::writeOn(enet::WebSocket& _interface) {
	ethread::UniqueLock lock = _interface.getScopeLock();
	zeus::Message::writeOn(_interface);
	_interface.writeData((uint8_t*)(&m_uid), sizeof(m_uid));
	if (message::Parameter::writeOn(_interface) == false) {
		return false;
	}
	int32_t count = _interface.send();
	return count > 0;
}

void zeus::message::Event::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	uint64_t* uuid = (uint64_t*)_buffer;
	m_uid = *uuid;
	// parse parameters:
	message::Parameter::composeWith(&_buffer[sizeof(m_uid)], _lenght-sizeof(m_uid));
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------

ememory::SharedPtr<zeus::message::Event> zeus::message::Event::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::message::Event>(new zeus::message::Event(_iface));
}

