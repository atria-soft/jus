/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.h>
#include <zeus/Buffer.h>
#include <zeus/debug.h>
#include <zeus/ParamType.h>
#include <etk/stdTools.h>
#include <zeus/BufferAnswer.h>

void zeus::BufferAnswer::generateDisplay(std::ostream& _os) const {
	zeus::Buffer::generateDisplay(_os);
	if (getNumberParameter() != 0) {
		_os << " '" + simpleStringParam(0) + "'";
	}
	if (m_errorType.size() != 0) {
		_os << " Error='" + m_errorType + "'";
	}if (m_errorHelp.size() != 0) {
		_os << " Help='" + m_errorHelp + "'";
	}
}

bool zeus::BufferAnswer::hasError() {
	return m_errorType.size() != 0;
}

const std::string& zeus::BufferAnswer::getError() {
	return m_errorType;
}

const std::string& zeus::BufferAnswer::getErrorHelp() {
	return m_errorHelp;
}

void zeus::BufferAnswer::addError(const std::string& _value, const std::string& _comment) {
	m_errorType = _value;
	m_errorHelp = _comment;
}

bool zeus::BufferAnswer::writeOn(enet::WebSocket& _interface) {
	zeus::Buffer::writeOn(_interface);
	_interface.writeData((uint8_t*)m_errorType.c_str(), m_errorType.size() + 1);
	if (m_errorType.size() != 0) {
		_interface.writeData((uint8_t*)m_errorHelp.c_str(), m_errorHelp.size() + 1);
	}
	return BufferParameter::writeOn(_interface);
}

void zeus::BufferAnswer::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// First element iw the call name, after, this is the parameters...
	// parse the string: (call name)
	uint32_t pos = 0;
	m_errorType.clear();
	m_errorHelp.clear();
	while(    pos < _lenght
	       && _buffer[pos] != '\0') {
		m_errorType += _buffer[pos];
		pos++;
	}
	pos++;
	if (m_errorType.size() != 0) {
		while(    pos < _lenght
		       && _buffer[pos] != '\0') {
			m_errorHelp += _buffer[pos];
			pos++;
		}
		pos++;
	}
	// parse parameters:
	BufferParameter::composeWith(&_buffer[pos], _lenght-pos);
}

void zeus::BufferAnswer::appendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj) {
	parameterAppendBufferData(_obj);
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------

ememory::SharedPtr<zeus::BufferAnswer> zeus::BufferAnswer::create() {
	return ememory::SharedPtr<zeus::BufferAnswer>(new zeus::BufferAnswer);
}

