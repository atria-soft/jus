/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/Buffer.hpp>
#include <zeus/debug.hpp>
#include <zeus/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/BufferCall.hpp>

void zeus::BufferCall::generateDisplay(std::ostream& _os) const {
	zeus::Buffer::generateDisplay(_os);
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

const std::string& zeus::BufferCall::getCall() const {
	return m_callName;
}

void zeus::BufferCall::setCall(const std::string& _value) {
	m_callName = _value;
}

bool zeus::BufferCall::writeOn(enet::WebSocket& _interface) {
	zeus::Buffer::writeOn(_interface);
	_interface.writeData((uint8_t*)m_callName.c_str(), m_callName.size() + 1);
	return BufferParameter::writeOn(_interface);
}

void zeus::BufferCall::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
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
	BufferParameter::composeWith(&_buffer[pos], _lenght-pos);
}

void zeus::BufferCall::appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) {
	parameterAppendBufferData(_obj);
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------


ememory::SharedPtr<zeus::BufferCall> zeus::BufferCall::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::BufferCall>(new zeus::BufferCall(_iface));
}
