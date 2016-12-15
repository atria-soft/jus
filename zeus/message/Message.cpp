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
#include <zeus/AbstractFunction.hpp>
#include <climits>
#include <zeus/message/Answer.hpp>
#include <zeus/message/Call.hpp>
#include <zeus/message/Data.hpp>
#include <zeus/message/Flow.hpp>
#include <zeus/message/Event.hpp>
#include <zeus/WebServer.hpp>


zeus::Message::Message(ememory::SharedPtr<zeus::WebServer> _iface):
  m_iface(_iface) {
	clear();
}

void zeus::Message::appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) {
	ZEUS_ERROR("Can not append datas ... Not managed");
}

void zeus::Message::appendMessage(ememory::SharedPtr<zeus::Message> _obj) {
	if (_obj == nullptr) {
		return;
	}
	if (_obj->getType() != zeus::message::type::data) {
		ZEUS_ERROR("try to add data with a wrong buffer: " << _obj->getType() << " ==> set the buffer finish ...");
		// close the connection ...
		setPartFinish(true);
		// TODO : Add an error ...
		return;
	}
	setPartFinish(_obj->getPartFinish());
	appendMessageData(ememory::staticPointerCast<zeus::message::Data>(_obj));
}

bool zeus::Message::writeOn(enet::WebSocket& _interface) {
	if (_interface.configHeader(false) == false) {
		return false;
	}
	_interface.writeData((uint8_t*)&m_header, sizeof(zeus::message::headerBin));
	return true;
}


void zeus::Message::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	// impossible case
}

void zeus::Message::clear() {
	m_header.transactionId = 1;
	m_header.sourceId = 0;
	m_header.sourceObjectId = 0;
	m_header.destinationId = 0;
	m_header.destinationObjectId = 0;
	m_header.flags = ZEUS_BUFFER_FLAG_FINISH;
}

std::ostream& zeus::operator <<(std::ostream& _os, ememory::SharedPtr<zeus::Message> _obj) {
	_os << "zeus::Message: ";
	if (_obj == nullptr) {
		_os << "nullptr";
	} else {
		_obj->generateDisplay(_os);
	}
	return _os;
}
void zeus::Message::generateDisplay(std::ostream& _os) const {
	//out += " v=" + etk::to_string(m_header.versionProtocol); // set it in the websocket
	_os << " if=" << etk::to_string(getInterfaceId());
	_os << " tr-id=" << etk::to_string(getTransactionId());
	_os << " src=" << etk::to_string(getSourceId()) << "/" << etk::to_string(getSourceObjectId());
	_os << " dst=" << etk::to_string(getDestinationId()) << "/" << etk::to_string(getDestinationObjectId());
	if (getPartFinish() == true) {
		_os << " finish";
	}
	enum zeus::message::type type = getType();
	switch (type) {
		case zeus::message::type::unknow:
			_os << " -UNKNOW-";
			break;
		case zeus::message::type::call:
			_os << " -CALL-";
			break;
		case zeus::message::type::answer:
			_os << " -ANSWER-";
			break;
		case zeus::message::type::event:
			_os << " -EVENT-";
			break;
		case zeus::message::type::data:
			_os << " -DATA-";
			break;
	}
}

uint32_t zeus::Message::getInterfaceId() const {
	return m_interfaceID;
}

void zeus::Message::setInterfaceId(uint32_t _value) {
	m_interfaceID = _value;
}

uint32_t zeus::Message::getTransactionId() const {
	return m_header.transactionId;
}

void zeus::Message::setTransactionId(uint32_t _value) {
	m_header.transactionId = _value;
}

uint32_t zeus::Message::getSource() const {
	return (uint32_t(m_header.sourceId) << 16) + m_header.sourceObjectId;
}

void zeus::Message::setSource(uint32_t _value) {
	m_header.sourceId = _value >> 16;
	m_header.sourceObjectId = _value & 0xFFFF;
}

uint16_t zeus::Message::getSourceId() const {
	return m_header.sourceId;
}

void zeus::Message::setSourceId(uint16_t _value) {
	m_header.sourceId = _value;
}

uint16_t zeus::Message::getSourceObjectId() const {
	return m_header.sourceObjectId;
}

void zeus::Message::setSourceObjectId(uint16_t _value) {
	m_header.sourceObjectId = _value;
}

uint32_t zeus::Message::getDestination() const {
	return (uint32_t(m_header.destinationId) << 16) + m_header.destinationObjectId;
}

void zeus::Message::setDestination(uint32_t _value) {
	m_header.destinationId = _value >> 16;
	m_header.destinationObjectId = _value & 0xFFFF;
}

uint16_t zeus::Message::getDestinationId() const {
	return m_header.destinationId;
}

void zeus::Message::setDestinationId(uint16_t _value) {
	m_header.destinationId = _value;
}

uint16_t zeus::Message::getDestinationObjectId() const {
	return m_header.destinationObjectId;
}

void zeus::Message::setDestinationObjectId(uint16_t _value) {
	m_header.destinationObjectId = _value;
}

bool zeus::Message::getPartFinish() const {
	return (m_header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0;
}

void zeus::Message::setPartFinish(bool _value) {
	if (_value == true) {
		m_header.flags = (m_header.flags & 0x7F) | ZEUS_BUFFER_FLAG_FINISH;
	} else {
		m_header.flags = m_header.flags & 0x7F;
	}
}

enum zeus::message::type zeus::Message::getType() const {
	return zeus::message::type::unknow;
}

// ------------------------------------------------------------------------------------
// -- Factory
// ------------------------------------------------------------------------------------
ememory::SharedPtr<zeus::Message> zeus::Message::create(ememory::SharedPtr<zeus::WebServer> _iface) {
	return ememory::SharedPtr<zeus::Message>(new zeus::Message(_iface));
}

ememory::SharedPtr<zeus::Message> zeus::Message::create(ememory::SharedPtr<zeus::WebServer> _iface, const std::vector<uint8_t>& _buffer) {
	zeus::message::headerBin header;
	if (_buffer.size() < sizeof(zeus::message::headerBin)) {
		ZEUS_ERROR("wrong size of the buffer: " << _buffer.size() << " must be " << sizeof(zeus::message::headerBin));
		return nullptr;
	}
	memcpy(reinterpret_cast<char*>(&header), &_buffer[0], sizeof(zeus::message::headerBin));
	enum zeus::message::type type = zeus::message::getTypeFromInt(uint16_t(header.flags & 0x07));
	switch (type) {
		case zeus::message::type::unknow:
			return nullptr;
		case zeus::message::type::call: {
				ememory::SharedPtr<zeus::message::Call> value = zeus::message::Call::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(zeus::message::headerBin)],
				                    _buffer.size() - sizeof(zeus::message::headerBin));
				return value;
			}
			break;
		case zeus::message::type::answer: {
				ememory::SharedPtr<zeus::message::Answer> value = zeus::message::Answer::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(zeus::message::headerBin)],
				                    _buffer.size() - sizeof(zeus::message::headerBin));
				return value;
			}
			break;
		case zeus::message::type::data: {
				ememory::SharedPtr<zeus::message::Data> value = zeus::message::Data::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(zeus::message::headerBin)],
				                    _buffer.size() - sizeof(zeus::message::headerBin));
				return value;
			}
			break;
		case zeus::message::type::event: {
				ememory::SharedPtr<zeus::message::Event> value = zeus::message::Event::create(_iface);
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionId);
				value->setSourceId(header.sourceId);
				value->setSourceObjectId(header.sourceObjectId);
				value->setDestinationId(header.destinationId);
				value->setDestinationObjectId(header.destinationObjectId);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(zeus::message::headerBin)],
				                    _buffer.size() - sizeof(zeus::message::headerBin));
				return value;
			}
			break;
	}
	return nullptr;
}

