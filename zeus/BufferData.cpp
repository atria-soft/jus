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
#include <zeus/BufferData.h>

void zeus::BufferData::generateDisplay(std::ostream& _os) const {
	zeus::Buffer::generateDisplay(_os);
	_os << " paramId=" << etk::to_string(m_parameterId);
	_os << " part=" << etk::to_string(m_partId);
	_os << " nbData=" << etk::to_string(m_data.size());
}

void zeus::BufferData::addData(uint16_t _parameterId, void* _data, uint32_t _size) {
	m_parameterId = _parameterId;
	m_data.resize(_size);
	memcpy(&m_data[0], _data, _size);
}

// note limited 15 bits
uint32_t zeus::BufferData::getPartId() const {
	return m_partId;
}

void zeus::BufferData::setPartId(uint32_t _value) {
	if (_value == 0) {
		ZEUS_ERROR("Part ID must be != of 0");
		return;
	}
	m_partId = _value;
}

bool zeus::BufferData::writeOn(enet::WebSocket& _interface) {
	zeus::Buffer::writeOn(_interface);
	_interface.writeData((uint8_t*)&m_partId, sizeof(uint32_t));
	_interface.writeData((uint8_t*)&m_parameterId, sizeof(uint16_t));
	_interface.writeData((uint8_t*)&m_data[0], m_data.size());
	return true;
}

void zeus::BufferData::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// First element iw the call name, after, this is the parameters...
	// parse the string: (call name)
	uint32_t pos = 0;
	m_partId = 0;
	m_parameterId = 0;
	memcpy(reinterpret_cast<char*>(&m_partId), &_buffer[pos], sizeof(uint32_t));
	pos += sizeof(uint32_t);
	memcpy(reinterpret_cast<char*>(&m_parameterId), &_buffer[pos], sizeof(uint16_t));
	pos += sizeof(uint16_t);
	m_data.resize(_lenght - pos);
	memcpy(&m_data[0], &_buffer[pos], m_data.size());
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------

ememory::SharedPtr<zeus::BufferData> zeus::BufferData::create() {
	return ememory::SharedPtr<zeus::BufferData>(new zeus::BufferData);
}
