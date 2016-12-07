/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/debug.hpp>
#include <zeus/message/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/message/Answer.hpp>

void zeus::message::Answer::generateDisplay(std::ostream& _os) const {
	zeus::Message::generateDisplay(_os);
	if (getNumberParameter() != 0) {
		_os << " '" + simpleStringParam(0) + "'";
	}
	if (m_errorType.size() != 0) {
		_os << " Error='" + m_errorType + "'";
	}if (m_errorHelp.size() != 0) {
		_os << " Help='" + m_errorHelp + "'";
	}
}

bool zeus::message::Answer::hasError() const {
	return m_errorType.size() != 0;
}

const std::string& zeus::message::Answer::getError() const {
	return m_errorType;
}

const std::string& zeus::message::Answer::getErrorHelp() const {
	return m_errorHelp;
}

void zeus::message::Answer::addError(const std::string& _value, const std::string& _comment) {
	m_errorType = _value;
	m_errorHelp = _comment;
}

bool zeus::message::Answer::writeOn(enet::WebSocket& _interface) {
	zeus::Message::writeOn(_interface);
	_interface.writeData((uint8_t*)m_errorType.c_str(), m_errorType.size() + 1);
	if (m_errorType.size() != 0) {
		_interface.writeData((uint8_t*)m_errorHelp.c_str(), m_errorHelp.size() + 1);
	}
	return message::Parameter::writeOn(_interface);
}

void zeus::message::Answer::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
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
	message::Parameter::composeWith(&_buffer[pos], _lenght-pos);
}

void zeus::message::Answer::appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) {
	parameterAppendMessageData(_obj);
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------

ememory::SharedPtr<zeus::message::Answer> zeus::message::Answer::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::message::Answer>(new zeus::message::Answer(_iface));
}

