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
#include <zeus/BufferCtrl.hpp>

void zeus::BufferCtrl::generateDisplay(std::ostream& _os) const {
	zeus::Buffer::generateDisplay(_os);
	_os << " '" + m_ctrlValue + "'";
}

const std::string& zeus::BufferCtrl::getCtrl() const {
	return m_ctrlValue;
}

void zeus::BufferCtrl::setCtrl(const std::string& _value) {
	m_ctrlValue = _value;
}

bool zeus::BufferCtrl::writeOn(enet::WebSocket& _interface) {
	zeus::Buffer::writeOn(_interface);
	_interface.writeData((uint8_t*)m_ctrlValue.c_str(), m_ctrlValue.size() + 1);
	return true;
}

void zeus::BufferCtrl::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// First element iw the call name, after, this is the parameters...
	// parse the string: (call name)
	uint32_t pos = 0;
	m_ctrlValue.clear();
	while(    pos < _lenght
	       && (char)_buffer[pos] != '\0') {
		m_ctrlValue += _buffer[pos];
		pos++;
	}
	pos++;
	// TODO : Check if some parameter are present ... ==> must create an error
}

void zeus::BufferCtrl::appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) {
	ZEUS_ERROR("A ctrl message can not have data ...");
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------


ememory::SharedPtr<zeus::BufferCtrl> zeus::BufferCtrl::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::BufferCtrl>(new zeus::BufferCtrl(_iface));
}
