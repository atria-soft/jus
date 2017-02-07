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
#include <zeus/message/Call.hpp>

void zeus::message::Call::generateDisplay(std::ostream& _os) const {
	zeus::Message::generateDisplay(_os);
	_os << " '" + m_callName + "'";
	_os << " nbParam=" + etk::to_string(getNumberParameter());
	if (getNumberParameter() != 0) {
		_os << " paramType(";
		for (int32_t iii=0; iii<getNumberParameter(); ++iii) {
			if (iii != 0) {
				_os << ",";
			}
			_os << getParameterType(iii).getName();
		}
		_os << ")";
	}
}

const std::string& zeus::message::Call::getCall() const {
	return m_callName;
}

void zeus::message::Call::setCall(const std::string& _value) {
	m_callName = _value;
}

bool zeus::message::Call::writeOn(enet::WebSocket& _interface) {
	std::unique_lock<std::mutex> lock = _interface.getScopeLock();
	zeus::Message::writeOn(_interface);
	_interface.writeData((uint8_t*)m_callName.c_str(), m_callName.size() + 1);
	return message::Parameter::writeOn(_interface);
}

void zeus::message::Call::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// First element iw the call name, after, this is the parameters...
	// parse the string: (call name)
	uint32_t pos = 0;
	m_callName.clear();
	while(    pos < _lenght
	       && (char)_buffer[pos] != '\0') {
		m_callName += _buffer[pos];
		pos++;
	}
	pos++;
	// parse parameters:
	message::Parameter::composeWith(&_buffer[pos], _lenght-pos);
}

void zeus::message::Call::appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) {
	parameterAppendMessageData(_obj);
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------


ememory::SharedPtr<zeus::message::Call> zeus::message::Call::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::message::Call>(new zeus::message::Call(_iface));
}
