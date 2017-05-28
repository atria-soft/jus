/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/debug.hpp>
#include <etk/stdTools.hpp>
#include <zeus/message/Progress.hpp>

void zeus::message::Progress::generateDisplay(std::ostream& _os) const {
	zeus::Message::generateDisplay(_os);
	_os << m_data;
}

const std::string& zeus::message::Progress::getData() const {
	return m_data;
}

void zeus::message::Progress::setData(const std::string& _data) {
	m_data = _data;
}

bool zeus::message::Progress::writeOn(enet::WebSocket& _interface) {
	std::unique_lock<std::mutex> lock = _interface.getScopeLock();
	zeus::Message::writeOn(_interface);
	_interface.writeData((uint8_t*)m_data.c_str(), m_data.size() + 1);
	int32_t count = _interface.send();
	return count > 0;
}

void zeus::message::Progress::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// First element iw the call name, after, this is the parameters...
	// parse the string: (call name)
	uint32_t pos = 0;
	m_data.clear();
	while(    pos < _lenght
	       && _buffer[pos] != '\0') {
		m_data += _buffer[pos];
		pos++;
	}
}

void zeus::message::Progress::appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) {
	ZEUS_ERROR("can not append data at a progress message ...");
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------

ememory::SharedPtr<zeus::message::Progress> zeus::message::Progress::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::message::Progress>(new zeus::message::Progress(_iface));
}

