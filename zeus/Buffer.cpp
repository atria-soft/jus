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

ememory::SharedPtr<zeus::Buffer> zeus::Buffer::create() {
	return ememory::SharedPtr<zeus::Buffer>(new zeus::Buffer);
}

void zeus::Buffer::appendBufferData(const ememory::SharedPtr<zeus::Buffer>& _obj) {
	if (_obj == nullptr) {
		return;
	}
	if (_obj->getType() != zeus::Buffer::typeMessage::data) {
		ZEUS_ERROR("try to add data with a wrong buffer: " << _obj->getType());
		return;
	}
	if (_obj->m_parameter.size() <= 1) {
		// normal end frame with no data ==> no problem ...
		return;
	}
	// At this point we just add data at the parameter value:
	//It contain 2 things : 
	//    - param 0 : parameter ID (int16_t)
	//    - param 1 : Raw data (local direct acces ...)
	uint16_t parameterID = _obj->internalGetParameter<uint16_t>(0);
	
	zeus::ParamType type = _obj->internalGetParameterType(1);
	const uint8_t* pointer = _obj->internalGetParameterPointer(1);
	uint32_t dataSize = _obj->internalGetParameterSize(1);
	if (    type.getId() != zeus::paramTypeRaw
	     || pointer == nullptr) {
		ZEUS_ERROR("Can not get parameter Raw ==> stop add");
		return;
	}
	if (dataSize == 0) {
		// no data (normal case)
		return;
	}
	// now, we will add datas:
	if (parameterID >= m_parameter.size()) {
		ZEUS_ERROR("Try to add data on an unexisting parameter...");
		return;
	}
	int32_t offset = m_parameter[parameterID].second.size();
	m_parameter[parameterID].second.resize(offset + dataSize);
	memcpy(&m_parameter[parameterID].second[offset], pointer, dataSize);
}


void zeus::Buffer::internalComposeWith(const uint8_t* _buffer, uint32_t _lenght) {
	clear();
	if (_lenght < sizeof(headerBin)) {
		ZEUS_ERROR("wrong size of the buffer");
		return;
	}
	uint32_t offset = 0;
	memcpy(reinterpret_cast<char*>(&m_header), &_buffer[offset], sizeof(headerBin));
	offset += sizeof(headerBin);
	m_parameter.clear();
	m_parameter.resize(m_header.numberOfParameter, std::make_pair(-1, std::vector<uint8_t>()));
	// Load all Parameters
	int32_t nbParameters = 0;
	while (    nbParameters < m_header.numberOfParameter
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

bool zeus::Buffer::writeOn(enet::WebSocket& _interface) {
	m_header.numberOfParameter = m_parameter.size();
	uint64_t size = sizeof(headerBin);
	for (auto &it : m_parameter) {
		size += sizeof(uint32_t); // parameter size
		size += it.second.size();
	}
	ZEUS_DEBUG("Send BINARY " << size << " bytes '" << generateHumanString() << "'");
	
	if (_interface.writeHeader(size, false) == false) {
		return false;
	}
	uint8_t* data = nullptr;
	uint32_t dataSize = 0;
	m_header.numberOfParameter = m_parameter.size();
	size = _interface.writeData((uint8_t*)&m_header, sizeof(headerBin));
	for (auto &it : m_parameter) {
		uint32_t paramSize = it.second.size();
		size = _interface.writeData((uint8_t*)&paramSize, sizeof(uint32_t));
		size = _interface.writeData(&it.second[0], it.second.size() * sizeof(uint8_t));
	}
	return true;
}

void zeus::Buffer::composeWith(const std::vector<uint8_t>& _buffer) {
	internalComposeWith(&_buffer[0], _buffer.size());
}

void zeus::Buffer::clear() {
	m_parameter.clear();
	m_header.transactionID = 1;
	m_header.clientID = 0;
	m_header.partID = 0x8000;
	m_header.typeMessage = 1;
	m_header.numberOfParameter = 1;
}

std::string zeus::Buffer::generateHumanString() {
	std::string out = "zeus::Buffer: ";
	//out += " v=" + etk::to_string(m_header.versionProtocol); // se it in the websocket
	out += " id=" + etk::to_string(m_header.transactionID);
	out += " cId=" + etk::to_string(m_header.clientID);
	if (    getPartFinish() == false
	     || getPartId() != 0) {
		out += " part=" + etk::to_string(getPartId());
		if (getPartFinish() == true) {
			out += "/finish";
		}
	}
	enum zeus::Buffer::typeMessage type = getTypeType(m_header.typeMessage);
	switch (type) {
		case zeus::Buffer::typeMessage::unknow:
			out += " -UNKNOW-";
			break;
		case zeus::Buffer::typeMessage::call:
			out += " nbParam=" + etk::to_string(getNumberParameter());
			out += " -CALL-:'" + getCall() + "'";
			break;
		case zeus::Buffer::typeMessage::answer:
			out += " -ANSWER-:";
			if (m_parameter.size() == 1) {
				out += "Value:" + simpleStringParam(0);
			} else if (m_parameter.size() == 2) {
				out += "Error";
				out += "Error:" + simpleStringParam(0);
			} else if (m_parameter.size() == 3) {
				out += "Value:" + simpleStringParam(0);
				out += "+Error:" + simpleStringParam(1);
			} else {
				out += "???";
			}
			break;
		case zeus::Buffer::typeMessage::event:
			out += " -EVENT-";
			
			break;
		case zeus::Buffer::typeMessage::data:
			out += " -DATA-";
			
			break;
	}
	if (getNumberParameter() != 0) {
		out += " paramType(";
		if (getNumberParameter() >256) {
			out += " !!!!!!!";
			return out;
		}
		for (int32_t iii=0; iii< getNumberParameter(); ++iii) {
			if (iii != 0) {
				out += ",";
			}
			out += internalGetParameterType(iii).getName();
		}
		out += ")";
	}
	return out;
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

// note limited 15 bits
uint16_t zeus::Buffer::getPartId() const {
	return uint16_t(m_header.partID & 0x7FFF);
}

void zeus::Buffer::setPartId(uint16_t _value) {
	m_header.partID = (m_header.partID&0x8000) | (_value & 0x7FFF);
}

bool zeus::Buffer::getPartFinish() const {
	return m_header.partID<0;
}

void zeus::Buffer::setPartFinish(bool _value) {
	if (_value == true) {
		m_header.partID = (m_header.partID & 0x7FFF) | 0x8000;
	} else {
		m_header.partID = m_header.partID & 0x7FFF;
	}
}

enum zeus::Buffer::typeMessage zeus::Buffer::getType() const {
	return (enum zeus::Buffer::typeMessage)m_header.typeMessage;
}

void zeus::Buffer::setType(enum typeMessage _value) {
	m_header.typeMessage = uint16_t(_value);
}

uint16_t zeus::Buffer::getNumberParameter() const {
	if (m_parameter.size() <= 1) {
		return 0;
	}
	return m_parameter.size()-1;
}
zeus::ParamType zeus::Buffer::internalGetParameterType(int32_t _id) const {
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
zeus::ParamType zeus::Buffer::getParameterType(int32_t _id) const {
	return internalGetParameterType(_id + 1);
}

const uint8_t* zeus::Buffer::internalGetParameterPointer(int32_t _id) const {
	const uint8_t* out = nullptr;
	if (m_parameter.size() <= _id) {
		ZEUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_parameter.size());
		return out;
	}
	if (m_parameter[_id].first <= -1) {
		internalGetParameterType(_id); // this function initialize this parameter if needed ...
	}
	out = reinterpret_cast<const uint8_t*>(&m_parameter[_id].second[m_parameter[_id].first]);
	return out;
}

const uint8_t* zeus::Buffer::getParameterPointer(int32_t _id) const {
	return internalGetParameterPointer(_id + 1);
}

uint32_t zeus::Buffer::internalGetParameterSize(int32_t _id) const {
	int32_t out = 0;
	if (m_parameter.size() <= _id) {
		ZEUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_parameter.size());
		return 0;
	}
	if (m_parameter[_id].first <= -1) {
		internalGetParameterType(_id); // this function initialize this parameter if needed ...
	}
	return m_parameter[_id].second.size() - m_parameter[_id].first;
}

void zeus::Buffer::addData(uint16_t _parameterId, void* _data, uint32_t _size) {
	m_parameter.clear();
	addParameter(_parameterId);
	std::vector<uint8_t> data;
	// set parameter type in case of wrong interpretation
	zeus::addTypeRaw(data);
	int32_t offset = data.size();
	data.resize(offset + _size);
	memcpy(&data[offset], _data, _size);
	m_parameter.push_back(std::make_pair(2,data));
}

uint32_t zeus::Buffer::getParameterSize(int32_t _id) const {
	return internalGetParameterSize(_id + 1);
}


void zeus::Buffer::addError(const std::string& _value, const std::string& _comment) {
	addParameter(_value);
	addParameter(_comment);
}

std::string zeus::Buffer::getCall() const {
	std::string out;
	switch(getType()) {
		case zeus::Buffer::typeMessage::call:
			return internalGetParameter<std::string>(0);
			break;
		case zeus::Buffer::typeMessage::answer:
			ZEUS_WARNING("get 'call' with an input type: 'answer'");
			break;
		default:
			ZEUS_WARNING("get 'call' with an input type: '" << getType() << "'");
			break;
	}
	return "";
}

void zeus::Buffer::setCall(std::string _value) {
	if (m_parameter.size() != 0) {
		ZEUS_ERROR("Clear Buffer of parameter ==> set the call type in first ...");
		m_parameter.clear();
	}
	addParameter(_value);
}


bool zeus::Buffer::hasError() {
	if (getType() != zeus::Buffer::typeMessage::answer) {
		return false;
	}
	if (m_parameter.size() == 2) {
		return true;
	} else if (m_parameter.size() == 3) {
		return true;
	}
	return false;
}

std::string zeus::Buffer::getError() {
	if (getType() != zeus::Buffer::typeMessage::answer) {
		return "";
	}
	if (m_parameter.size() == 2) {
		return getParameter<std::string>(0);
	} else if (m_parameter.size() == 3) {
		return getParameter<std::string>(1);
	}
	return "";
}

std::string zeus::Buffer::getErrorHelp() {
	if (getType() != zeus::Buffer::typeMessage::answer) {
		return "";
	}
	if (m_parameter.size() == 2) {
		return getParameter<std::string>(1);
	} else if (m_parameter.size() == 3) {
		return getParameter<std::string>(2);
	}
	return "";
}


std::string zeus::Buffer::simpleStringParam(uint32_t _id) const {
	zeus::ParamType paramType = internalGetParameterType(_id);
	if (paramType.isVector() == false) {
		if (paramType.isNumber() == true) {
			return etk::to_string(internalGetParameter<int64_t>(_id));
		}
	}
	if (paramType == createType<bool>()) {
		return etk::to_string(internalGetParameter<bool>(_id));
	}
	if (paramType == createType<std::string>()) {
		return "{" + internalGetParameter<std::string>(_id) + "}";
	}
	if (paramType == createType<std::vector<std::string>>()) {
		return "[" + etk::to_string(internalGetParameter<std::vector<std::string>>(_id)) + "]";
	}
	return paramType.getName();
}

