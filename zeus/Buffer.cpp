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
#include <zeus/AbstractFunction.h>
#include <climits>

namespace etk {
	template<> std::string to_string<enum zeus::Buffer::typeMessage>(const enum zeus::Buffer::typeMessage& _value) {
		switch (_value) {
			case zeus::Buffer::typeMessage::unknow:
				return "unknow";
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
			return zeus::Buffer::typeMessage::call;
		case 2:
			return zeus::Buffer::typeMessage::answer;
		case 3:
			return zeus::Buffer::typeMessage::data;
		case 4:
			return zeus::Buffer::typeMessage::event;
	}
	return zeus::Buffer::typeMessage::unknow;
}

zeus::Buffer::Buffer() {
	clear();
}

void zeus::Buffer::appendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj) {
	ZEUS_ERROR("Can not append datas ... Not managed");
}

void zeus::Buffer::appendBuffer(const ememory::SharedPtr<zeus::Buffer>& _obj) {
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
	appendBufferData(std::static_pointer_cast<zeus::BufferData>(_obj));
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
	m_header.transactionID = 1;
	m_header.clientID = 0;
	m_header.flags = ZEUS_BUFFER_FLAG_FINISH;
}

std::ostream& zeus::operator <<(std::ostream& _os, zeus::Buffer* _obj) {
	_os << "zeus::Buffer: ";
	if (_obj == nullptr) {
		_os << "nullptr";
	} else {
		_obj->generateDisplay(_os);
	}
	return _os;
}
void zeus::Buffer::generateDisplay(std::ostream& _os) const {
	//out += " v=" + etk::to_string(m_header.versionProtocol); // se it in the websocket
	_os << " id=" << etk::to_string(getTransactionId());
	_os << " cId=" << etk::to_string(getClientId());
	if (getPartFinish() == true) {
		_os << " finish";
	}
	enum zeus::Buffer::typeMessage type = getType();
	switch (type) {
		case zeus::Buffer::typeMessage::unknow:
			_os << " -UNKNOW-";
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

void zeus::BufferData::generateDisplay(std::ostream& _os) const {
	zeus::Buffer::generateDisplay(_os);
	_os << " paramId=" << etk::to_string(m_parameterId);
	_os << " part=" << etk::to_string(m_partId);
	_os << " nbData=" << etk::to_string(m_data.size());
}

uint32_t zeus::Buffer::getTransactionId() const {
	return m_header.transactionID;
}

void zeus::Buffer::setTransactionId(uint32_t _value) {
	m_header.transactionID = _value;
}

uint32_t zeus::Buffer::getClientId() const {
	return m_header.clientID;
}

void zeus::Buffer::setClientId(uint32_t _value) {
	m_header.clientID = _value;
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
// -- Multiple parameter
// ------------------------------------------------------------------------------------
bool zeus::BufferParameter::writeOn(enet::WebSocket& _interface) {
	uint8_t* data = nullptr;
	uint32_t dataSize = 0;
	uint16_t nbParameters = m_parameter.size();
	size_t size = _interface.writeData((uint8_t*)&nbParameters, sizeof(uint16_t));
	for (auto &it : m_parameter) {
		uint32_t paramSize = it.second.size();
		size = _interface.writeData((uint8_t*)&paramSize, sizeof(uint32_t));
		size += _interface.writeData(&it.second[0], it.second.size() * sizeof(uint8_t));
	}
	return true;
}

void zeus::BufferParameter::composeWith(const uint8_t* _buffer, uint32_t _lenght) {
	m_parameter.clear();
	uint16_t nbParameters = 0;
	if (_lenght < sizeof(uint16_t)) {
		ZEUS_ERROR("wrong size of the buffer ==> missing parameter count");
		return;
	}
	uint32_t offset = 0;
	memcpy(&nbParameters, &_buffer[offset], sizeof(uint16_t));
	offset += sizeof(uint16_t);
	m_parameter.resize(nbParameters, std::make_pair(-1, std::vector<uint8_t>()));
	// Load all Parameters
	nbParameters = 0;
	while (    nbParameters < m_parameter.size()
	        && offset < _lenght) {
		// parameters is contituated with size + data assiciated
		uint32_t sizeParam;
		memcpy(&sizeParam, &_buffer[offset], sizeof(uint32_t));
		offset += sizeof(uint32_t);
		if (offset+sizeParam > _lenght) {
			ZEUS_ERROR("Wrong parameter size : " << sizeParam << " / availlable=" << _lenght-offset);
			return;
		}
		std::vector<uint8_t> data;
		data.resize(sizeParam);
		memcpy(&data[0], &_buffer[offset], data.size() * sizeof(uint8_t));
		offset += data.size() * sizeof(uint8_t);
		m_parameter[nbParameters].second = data;
		nbParameters++;
	}
}

zeus::ParamType zeus::BufferParameter::getParameterType(int32_t _id) const {
	if (m_parameter.size() <= _id) {
		ZEUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_parameter.size());
		return createType<void>();
	}
	if (m_parameter[_id].second.size() < 2) {
		ZEUS_ERROR("try to get type with no ID value Type (not enouth data)");
		return createType<void>();
	}
	uint16_t typeId = (uint16_t(m_parameter[_id].second[0]) << 8) + uint16_t(m_parameter[_id].second[1]);
	if (typeId == createType<void>().getId()) { m_parameter[_id].first = 2; return createType<void>(); }
	if (typeId == createType<bool>().getId()) { m_parameter[_id].first = 2; return createType<bool>(); }
	if (typeId == createType<float>().getId()) { m_parameter[_id].first = 2; return createType<float>(); }
	if (typeId == createType<double>().getId()) { m_parameter[_id].first = 2; return createType<double>(); }
	if (typeId == createType<int64_t>().getId()) { m_parameter[_id].first = 2; return createType<int64_t>(); }
	if (typeId == createType<int32_t>().getId()) { m_parameter[_id].first = 2; return createType<int32_t>(); }
	if (typeId == createType<int16_t>().getId()) { m_parameter[_id].first = 2; return createType<int16_t>(); }
	if (typeId == createType<int8_t>().getId()) { m_parameter[_id].first = 2; return createType<int8_t>(); }
	if (typeId == createType<uint64_t>().getId()) { m_parameter[_id].first = 2; return createType<uint64_t>(); }
	if (typeId == createType<uint32_t>().getId()) { m_parameter[_id].first = 2; return createType<uint32_t>(); }
	if (typeId == createType<uint16_t>().getId()) { m_parameter[_id].first = 2; return createType<uint16_t>(); }
	if (typeId == createType<uint8_t>().getId()) { m_parameter[_id].first = 2; return createType<uint8_t>(); }
	if (typeId == createType<std::string>().getId()) { m_parameter[_id].first = 2; return createType<std::string>(); }
	if (typeId == createType<std::vector<void>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<void>>(); }
	if (typeId == createType<std::vector<bool>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<bool>>(); }
	if (typeId == createType<std::vector<float>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<float>>(); }
	if (typeId == createType<std::vector<double>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<double>>(); }
	if (typeId == createType<std::vector<int64_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<int64_t>>(); }
	if (typeId == createType<std::vector<int32_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<int32_t>>(); }
	if (typeId == createType<std::vector<int16_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<int16_t>>(); }
	if (typeId == createType<std::vector<int8_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<int8_t>>(); }
	if (typeId == createType<std::vector<uint64_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<uint64_t>>(); }
	if (typeId == createType<std::vector<uint32_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<uint32_t>>(); }
	if (typeId == createType<std::vector<uint16_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<uint16_t>>(); }
	if (typeId == createType<std::vector<uint8_t>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<uint8_t>>(); }
	if (typeId == createType<std::vector<std::string>>().getId()) { m_parameter[_id].first = 2; return createType<std::vector<std::string>>(); }
	if (typeId == createType<zeus::File>().getId()) { m_parameter[_id].first = 2; return createType<zeus::File>(); }
	if (typeId == paramTypeRaw) {
		m_parameter[_id].first = sizeof(uint16_t);
		return zeus::ParamType("raw", paramTypeRaw);
	}
	if (typeId == paramTypeObject) {
		std::string type = reinterpret_cast<const char*>(&m_parameter[_id].second[2]);
		m_parameter[_id].first = type.size() + sizeof(uint16_t);
		// TODO : Check error of \0 ==> limit at 256 char ...
		return zeus::ParamType(type, paramTypeObject);
	}
	ZEUS_ERROR("Can not get type of parameter ... ");
	return createType<void>();
}

const uint8_t* zeus::BufferParameter::getParameterPointer(int32_t _id) const {
	const uint8_t* out = nullptr;
	if (m_parameter.size() <= _id) {
		ZEUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_parameter.size());
		return out;
	}
	if (m_parameter[_id].first <= -1) {
		getParameterType(_id); // this function initialize this parameter if needed ...
	}
	out = reinterpret_cast<const uint8_t*>(&m_parameter[_id].second[m_parameter[_id].first]);
	return out;
}

uint32_t zeus::BufferParameter::getParameterSize(int32_t _id) const {
	int32_t out = 0;
	if (m_parameter.size() <= _id) {
		ZEUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_parameter.size());
		return 0;
	}
	if (m_parameter[_id].first <= -1) {
		getParameterType(_id); // this function initialize this parameter if needed ...
	}
	return m_parameter[_id].second.size() - m_parameter[_id].first;
}

uint16_t zeus::BufferParameter::getNumberParameter() const {
	return m_parameter.size();
}

std::string zeus::BufferParameter::simpleStringParam(uint32_t _id) const {
	zeus::ParamType paramType = getParameterType(_id);
	if (paramType.isVector() == false) {
		if (paramType.isNumber() == true) {
			return etk::to_string(getParameter<int64_t>(_id));
		}
	}
	if (paramType == createType<bool>()) {
		return etk::to_string(getParameter<bool>(_id));
	}
	if (paramType == createType<std::string>()) {
		return "{" + getParameter<std::string>(_id) + "}";
	}
	if (paramType == createType<std::vector<std::string>>()) {
		return "[" + etk::to_string(getParameter<std::vector<std::string>>(_id)) + "]";
	}
	return paramType.getName();
}

void zeus::BufferParameter::parameterAppendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj) {
	// At this point we just add data at the parameter value:
	uint16_t parameterID = _obj->getParameterId();
	
	if (_obj->getData().size() == 0) {
		// no data (normal case)
		return;
	}
	// now, we will add datas:
	if (parameterID >= m_parameter.size()) {
		ZEUS_ERROR("Try to add data on an unexisting parameter...");
		return;
	}
	int32_t offset = m_parameter[parameterID].second.size();
	m_parameter[parameterID].second.resize(offset + _obj->getData().size());
	memcpy(&m_parameter[parameterID].second[offset], &_obj->getData()[0], _obj->getData().size());
}

// ------------------------------------------------------------------------------------
// -- Call
// ------------------------------------------------------------------------------------

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

void zeus::BufferCall::appendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj) {
	parameterAppendBufferData(_obj);
}

// ------------------------------------------------------------------------------------
// -- Answer
// ------------------------------------------------------------------------------------

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
// -- Data
// ------------------------------------------------------------------------------------

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
ememory::SharedPtr<zeus::Buffer> zeus::Buffer::create() {
	return ememory::SharedPtr<zeus::Buffer>(new zeus::Buffer);
}

ememory::SharedPtr<zeus::Buffer> zeus::Buffer::create(const std::vector<uint8_t>& _buffer) {
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
		case zeus::Buffer::typeMessage::call: {
				ememory::SharedPtr<zeus::BufferCall> value = zeus::BufferCall::create();
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionID);
				value->setClientId(header.clientID);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::answer: {
				ememory::SharedPtr<zeus::BufferAnswer> value = zeus::BufferAnswer::create();
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionID);
				value->setClientId(header.clientID);
				value->setPartFinish((header.flags & ZEUS_BUFFER_FLAG_FINISH) != 0);
				value->composeWith(&_buffer[sizeof(headerBin)],
				                    _buffer.size() - sizeof(headerBin));
				return value;
			}
			break;
		case zeus::Buffer::typeMessage::data: {
				ememory::SharedPtr<zeus::BufferData> value = zeus::BufferData::create();
				if (value == nullptr) {
					return nullptr;
				}
				value->setTransactionId(header.transactionID);
				value->setClientId(header.clientID);
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

ememory::SharedPtr<zeus::BufferCall> zeus::BufferCall::create() {
	return ememory::SharedPtr<zeus::BufferCall>(new zeus::BufferCall);
}

ememory::SharedPtr<zeus::BufferAnswer> zeus::BufferAnswer::create() {
	return ememory::SharedPtr<zeus::BufferAnswer>(new zeus::BufferAnswer);
}

ememory::SharedPtr<zeus::BufferData> zeus::BufferData::create() {
	return ememory::SharedPtr<zeus::BufferData>(new zeus::BufferData);
}
/*
ememory::SharedPtr<zeus::BufferEvent> zeus::BufferEvent::create() {
	return ememory::SharedPtr<zeus::BufferEvent>(new zeus::BufferEvent);
}

ememory::SharedPtr<zeus::BufferFlow> zeus::BufferFlow::create() {
	return ememory::SharedPtr<zeus::BufferFlow>(new zeus::BufferFlow);
}
*/

