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
#include <zeus/AbstractFunction.hpp>
#include <climits>
#include <zeus/BufferAnswer.hpp>
#include <zeus/BufferCtrl.hpp>
#include <zeus/BufferCall.hpp>
#include <zeus/BufferData.hpp>
#include <zeus/BufferFlow.hpp>
#include <zeus/BufferEvent.hpp>
#include <zeus/WebServer.hpp>


namespace etk {
	template<> std::string to_string<enum zeus::Buffer::typeMessage>(const enum zeus::Buffer::typeMessage& _value) {
		switch (_value) {
			case zeus::Buffer::typeMessage::unknow:
				return "unknow";
			case zeus::Buffer::typeMessage::ctrl:
				return "ctrl";
			case zeus::Buffer::typeMessage::call:
				return "call";
			case zeus::Buffer::typeMessage::answer:
				return "answer";
			case zeus::Buffer::typeMessage::event:
				return "event";
			case zeus::Buffer::typeMessage::data:
				return "data";
		}
		return "???";
	}
}
std::ostream& zeus::operator <<(std::ostream& _os, enum zeus::Buffer::typeMessage _value) {
	_os << etk::to_string(_value);
	return _os;
}

static enum zeus::Buffer::typeMessage getTypeType(uint16_t _value) {
	switch (_value) {
		case 0:
			return zeus::Buffer::typeMessage::unknow;
		case 1:
			return zeus::Buffer::typeMessage::ctrl;
		case 2:
			return zeus::Buffer::typeMessage::call;
		case 3:
			return zeus::Buffer::typeMessage::answer;
		case 4:
			return zeus::Buffer::typeMessage::data;
		case 5:
			return zeus::Buffer::typeMessage::event;
	}
	return zeus::Buffer::typeMessage::unknow;
}

zeus::Buffer::Buffer(ememory::SharedPtr<zeus::WebServer> _iface):
  m_iface(_iface) {
	clear();
}

void zeus::Buffer::appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) {
	ZEUS_ERROR("Can not append datas ... Not managed");
}

void zeus::Buffer::appendBuffer(ememory::SharedPtr<zeus::Buffer> _obj) {
	if (_obj == nullptr) {
		return;
	}
	if (_obj->getType() != zeus::Buffer::typeMessage::data) {
		ZEUS_ERROR("try to add data with a wrong buffer: " << _obj->getType() << " ==> set the buffer finish ...");
		// close the connection ...
		setPartFinish(true);
		// TODO : Add an error ...
		return;
	}
	setPartFinish(_obj->getPartFinish());
	appendBufferData(ememory::staticPointerCast<zeus::BufferData>(_obj));
}

bool zeus::Buffer::writeOn(enet::WebSocket& _interface) {
	if (_interface.configHeader(false) == false) {
		return false;
	}
	_interface.writeData((uint8_t*)&m_header, sizeof(headerBin));
	return true;
}


void zeus::Buffer::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// impossible case
}

void zeus::Buffer::clear() {
	m_header.transactionId = 1;
	m_header.sourceId = 0;
	m_header.sourceObjectId = 0;
	m_header.destinationId = 0;
	m_header.destinationObjectId = 0;
	m_header.flags = ZEUS_BUFFER_FLAG_FINISH;
}

std::ostream& zeus::operator <<(std::ostream& _os, ememory::SharedPtr<zeus::Buffer> _obj) {
	_os << "zeus::Buffer: ";
	if (_obj == nullptr) {
		_os << "nullptr";
	} else {
		_obj->generateDisplay(_os);
	}
	return _os;
}
void zeus::Buffer::generateDisplay(std::ostream& _os) const {
	//out += " v=" + etk::to_string(m_header.versionProtocol); // set it in the websocket
	_os << " if=" << etk::to_string(getInterfaceId());
	_os << " tr-id=" << etk::to_string(getTransactionId());
	_os << " src=" << etk::to_string(getSourceId()) << "/" << etk::to_string(getSourceObjectId());
	_os << " dst=" << etk::to_string(getDestinationId()) << "/" << etk::to_string(getDestinationObjectId());
	if (getPartFinish() == true) {
		_os << " finish";
	}
	enum zeus::Buffer::typeMessage type = getType();
	switch (type) {
		case zeus::Buffer::typeMessage::unknow:
			_os << " -UNKNOW-";
			break;
		case zeus::Buffer::typeMessage::ctrl:
			_os << " -CTRL-";
			break;
		case zeus::Buffer::typeMessage::call:
			_os << " -CALL-";
			break;
		case zeus::Buffer::typeMessage::answer:
			_os << " -ANSWER-";
			break;
		case zeus::Buffer::typeMessage::event:
			_os << " -EVENT-";
			break;
		case zeus::Buffer::typeMessage::data:
			_os << " -DATA-";
			break;
	}
}

uint32_t zeus::Buffer::getInterfaceId() const {
	return m_interfaceID;
}

void zeus::Buffer::setInterfaceId(uint32_t _value) {
	m_interfaceID = _value;
}

uint32_t zeus::Buffer::getTransactionId() const {
	return m_header.transactionId;
}

void zeus::Buffer::setTransactionId(uint32_t _value) {
	m_header.transactionId = _value;
}

uint32_t zeus::Buffer::getSource() const {
	return (uint32_t(m_header.sourceId) << 16) + m_header.sourceObjectId;
}

void zeus::Buffer::setSource(uint32_t _value) {
	m_header.sourceId = _value >> 16;
	m_header.sourceObjectId = _value & 0xFFFF;
}

uint16_t zeus::Buffer::getSourceId() const {
	return m_header.sourceId;
}

void zeus::Buffer::setSourceId(uint16_t _value) {
	m_header.sourceId = _value;
}

uint16_t zeus::Buffer::getSourceObjectId() const {
	return m_header.sourceObjectId;
}

void zeus::Buffer::setSourceObjectId(uint16_t _value) {
	m_header.sourceObjectId = _value;
}

uint32_t zeus::Buffer::getDestination() const {
	return (uint32_t(m_header.destinationId) << 16) + m_header.destinationObjectId;
}

void zeus::Buffer::setDestination(uint32_t _value) {
	m_header.destinationId = _value >> 16;
	m_header.destinationObjectId = _value & 0xFFFF;
}

uint16_t zeus::Buffer::getDestinationId() const {
	return m_header.destinationId;
}

void zeus::Buffer::setDestinationId(uint16_t _value) {
	m_header.destinationId = _value;
}

uint16_t zeus::Buffer::getDestinationObjectId() const {
	return m_header.destinationObjectId;
}

void zeus::Buffer::setDestinationObjectId(uint16_t _value) {
	m_header.destinationObjectId = _value;
}

bool zeus::Buffer::getPartFinish() const {
	return (m_header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0;
}

void zeus::Buffer::setPartFinish(bool _value) {
	if (_value == true) {
		m_header.flags = (m_header.flags & 0x7F) | ZEUS_BUFFER_FLAG_FINISH;
	} else {
		m_header.flags = m_header.flags & 0x7F;
	}
}

enum zeus::Buffer::typeMessage zeus::Buffer::getType() const {
	return zeus::Buffer::typeMessage::unknow;
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------
ememory::SharedPtr<zeus::Buffer> zeus::Buffer::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::Buffer>(new zeus::Buffer(_iface));
}

ememory::SharedPtr<zeus::Buffer> zeus::Buffer::create(ememory::SharedPtr<zeus::WebServer> _iface, const std::vector<uint8_t>& _buffer) {
	headerBin header;
	if (_buffer.size() < sizeof(headerBin)) {
		ZEUS_ERROR("wrong size of the buffer");
		return nullptr;
	}
	memcpy(reinterpret_cast<char*>(&header), &_buffer[0], sizeof(headerBin));
	enum zeus::Buffer::typeMessage type = getTypeType(uint16_t(header.flags & 0x07));
	switch (type) {
		case zeus::Buffer::typeMessage::unknow:
			return nullptr;
		case zeus::Buffer::typeMessage::ctrl: {
				ememory::SharedPtr<zeus::BufferCtrl> value = zeus::BufferCtrl::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::call: {
				ememory::SharedPtr<zeus::BufferCall> value = zeus::BufferCall::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::answer: {
				ememory::SharedPtr<zeus::BufferAnswer> value = zeus::BufferAnswer::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::data: {
				ememory::SharedPtr<zeus::BufferData> value = zeus::BufferData::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::event:
			
			break;
	}
	return nullptr;
}

