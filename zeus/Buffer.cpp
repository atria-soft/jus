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
			case zeus::Buffer::typeMessage::call:
				return "call";
			case zeus::Buffer::typeMessage::answer:
				return "answer";
			case zeus::Buffer::typeMessage::event:
				return "event";
			case zeus::Buffer::typeMessage::data:
				return "event";
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
		case 1:
			return zeus::Buffer::typeMessage::call;
		case 2:
			return zeus::Buffer::typeMessage::answer;
		case 3:
			return zeus::Buffer::typeMessage::data;
		case 4:
			return zeus::Buffer::typeMessage::event;
	}
	return zeus::Buffer::typeMessage::call;
}

zeus::Buffer::Buffer() {
	clear();
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
		uint16_t sizeParam;
		memcpy(&sizeParam, &_buffer[offset], sizeof(uint16_t));
		offset += sizeof(uint16_t);
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
		size += sizeof(uint16_t); // parameter size
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
		uint16_t paramSize = it.second.size();
		size = _interface.writeData((uint8_t*)&paramSize, sizeof(uint16_t));
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
	if (    getPartId() != 0
	     || getPartFinish() == false) {
		out += " part=" + etk::to_string(getPartId());
		if (getPartFinish() == true) {
			out += "/finish";
		}
	}
	enum zeus::Buffer::typeMessage type = getTypeType(m_header.typeMessage);
	switch (type) {
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

void zeus::Buffer::addData(void* _data, uint32_t _size) {
	m_parameter.clear();
	setType(zeus::Buffer::typeMessage::data);
	//m_data.resize(_size);
	//memcpy(&m_data[0], _data, _size);
	ZEUS_TODO("Must implement the add data ...");
}

uint32_t zeus::Buffer::getParameterSize(int32_t _id) const {
	return internalGetParameterSize(_id + 1);
}

static void addType(std::vector<uint8_t>& _data, zeus::ParamType _type) {
	_data.push_back(uint8_t(_type.getId()>>8));
	_data.push_back(uint8_t(_type.getId()));
}

static void addTypeObject(std::vector<uint8_t>& _data, const std::string _type) {
	_data.push_back(uint8_t(zeus::paramTypeObject>>8));
	_data.push_back(uint8_t(zeus::paramTypeObject));
	for (auto &it : _type) {
		_data.push_back(uint8_t(it));
	}
	_data.push_back(0);
}

void zeus::Buffer::addParameter() {
	std::vector<uint8_t> data;
	addType(data, createType<void>());
	m_parameter.push_back(std::make_pair(2,data));
}
void zeus::Buffer::addParameterEmptyVector() {
	// special case of json change mode
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<void>>());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::string>(const std::string& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::string>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], _value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<std::string>>(const std::vector<std::string>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<std::string>>());
	// count all datas:
	uint32_t size = 0;
	for (auto &it : _value) {
		size+=it.size()+1;
	}
	uint16_t nb = _value.size();
	int32_t currentOffset = data.size();
	data.resize(data.size()+size+2);
	memcpy(&data[currentOffset], &nb, sizeof(uint16_t));
	currentOffset += sizeof(uint16_t);
	for (auto &it : _value) {
		memcpy(&data[currentOffset], &it[0], it.size());
		currentOffset += it.size();
		data[currentOffset] = '\0';
		currentOffset++;
	}
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<std::vector<bool>>(const std::vector<bool>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<bool>>());
	// add size:
	uint16_t nb = _value.size();
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	for (const auto &it : _value) {
		if (it == true) {
			data[currentOffset] = 'T';
		} else {
			data[currentOffset] = 'F';
		}
		currentOffset++;
	}
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<std::vector<int8_t>>(const std::vector<int8_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int8_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int8_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<int16_t>>(const std::vector<int16_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int16_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int16_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<int32_t>>(const std::vector<int32_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int32_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int32_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<int64_t>>(const std::vector<int64_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int64_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int64_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<uint8_t>>(const std::vector<uint8_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint8_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint8_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<uint16_t>>(const std::vector<uint16_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint16_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint16_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<uint32_t>>(const std::vector<uint32_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint32_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint32_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<std::vector<uint64_t>>(const std::vector<uint64_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint64_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint64_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<std::vector<float>>(const std::vector<float>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<float>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(float)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<std::vector<double>>(const std::vector<double>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<double>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(double)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<int8_t>(const int8_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int8_t>());
	data.push_back(uint8_t(_value));
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<uint8_t>(const uint8_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint8_t>());
	data.push_back(_value);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<int16_t>(const int16_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int16_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+2);
	memcpy(&data[currentOffset], &_value, 2);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<uint16_t>(const uint16_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint16_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+2);
	memcpy(&data[currentOffset], &_value, 2);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<int32_t>(const int32_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int32_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<uint32_t>(const uint32_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint32_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<int64_t>(const int64_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int64_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<uint64_t>(const uint64_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint64_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<float>(const float& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<float>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<double>(const double& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<double>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::addParameter<bool>(const bool& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<bool>());
	if (_value == true) {
		data.push_back('T');
	} else {
		data.push_back('F');
	}
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::addParameter<zeus::File>(const zeus::File& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<zeus::File>());
	ZEUS_TODO("Send file in output ...");
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
bool zeus::Buffer::internalGetParameter<bool>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	if (createType<bool>() != type) {
		return 0;
	}
	const char* pointer2 = reinterpret_cast<const char*>(pointer);
	if (    *pointer2 == 'T'
	     || *pointer2 == '1'
	     || *pointer2 == 1) {
		return true;
	}
	return false;
}

template<>
std::string zeus::Buffer::internalGetParameter<std::string>(int32_t _id) const {
	std::string out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	out.resize(dataSize, 0);
	memcpy(&out[0], pointer, out.size());
	return out;
}


template<>
uint8_t zeus::Buffer::internalGetParameter<uint8_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return std::min(*tmp, uint16_t(UCHAR_MAX));
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return std::min(*tmp, uint32_t(UCHAR_MAX));
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(UCHAR_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return std::max(int8_t(0), *tmp);
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return etk::avg(int16_t(0), *tmp, int16_t(UCHAR_MAX));
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return etk::avg(int32_t(0), *tmp, int32_t(UCHAR_MAX));
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(0), *tmp, int64_t(UCHAR_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return uint8_t(etk::avg(float(0), *tmp, float(UCHAR_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return uint8_t(etk::avg(double(0), *tmp, double(UCHAR_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}
template<>
uint16_t zeus::Buffer::internalGetParameter<uint16_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return std::min(*tmp, uint32_t(USHRT_MAX));
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(USHRT_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return std::max(int8_t(0), *tmp);
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return std::max(int16_t(0), *tmp);
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return etk::avg(int32_t(0), *tmp, int32_t(USHRT_MAX));
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(0), *tmp, int64_t(USHRT_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return uint16_t(etk::avg(float(0), *tmp, float(USHRT_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return uint16_t(etk::avg(double(0), *tmp, double(USHRT_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
uint32_t zeus::Buffer::internalGetParameter<uint32_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return *tmp;
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(ULONG_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return std::max(int8_t(0), *tmp);
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return std::max(int16_t(0), *tmp);
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return std::max(int32_t(0), *tmp);
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(0), *tmp, int64_t(ULONG_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return uint32_t(etk::avg(float(0), *tmp, float(ULONG_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return uint32_t(etk::avg(double(0), *tmp, double(ULONG_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
uint64_t zeus::Buffer::internalGetParameter<uint64_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return *tmp;
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return *tmp;
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return std::max(int8_t(0), *tmp);
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return std::max(int16_t(0), *tmp);
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return std::max(int32_t(0), *tmp);
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return std::max(int64_t(0), *tmp);
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return uint64_t(etk::avg(float(0), *tmp, float(ULONG_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return uint64_t(etk::avg(double(0), *tmp, double(ULONG_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
int8_t zeus::Buffer::internalGetParameter<int8_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return std::min(*tmp, uint8_t(SCHAR_MAX));
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return std::min(*tmp, uint16_t(SCHAR_MAX));
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return std::min(*tmp, uint32_t(SCHAR_MAX));
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(SCHAR_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return etk::avg(int16_t(SCHAR_MIN), *tmp, int16_t(SCHAR_MAX));
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return etk::avg(int32_t(SCHAR_MIN), *tmp, int32_t(SCHAR_MAX));
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(SCHAR_MIN), *tmp, int64_t(SCHAR_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return int8_t(etk::avg(float(SCHAR_MIN), *tmp, float(SCHAR_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return int8_t(etk::avg(double(SCHAR_MIN), *tmp, double(SCHAR_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
int16_t zeus::Buffer::internalGetParameter<int16_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return std::min(*tmp, uint16_t(SHRT_MAX));
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return std::min(*tmp, uint32_t(SHRT_MAX));
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(SHRT_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return *tmp;
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return etk::avg(int32_t(SHRT_MIN), *tmp, int32_t(SHRT_MAX));
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(SHRT_MIN), *tmp, int64_t(SHRT_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return int16_t(etk::avg(float(SHRT_MIN), *tmp, float(SHRT_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return int16_t(etk::avg(double(SHRT_MIN), *tmp, double(SHRT_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
int32_t zeus::Buffer::internalGetParameter<int32_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return std::min(*tmp, uint32_t(LONG_MAX));
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(LONG_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return *tmp;
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return *tmp;
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return etk::avg(int64_t(LONG_MIN), *tmp, int64_t(LONG_MAX));
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return int32_t(etk::avg(float(LONG_MIN), *tmp, float(LONG_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return int32_t(etk::avg(double(LONG_MIN), *tmp, double(LONG_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
int64_t zeus::Buffer::internalGetParameter<int64_t>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return *tmp;
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return std::min(*tmp, uint64_t(LLONG_MAX));
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return *tmp;
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return *tmp;
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return *tmp;
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return int64_t(etk::avg(float(LLONG_MIN), *tmp, float(LLONG_MAX)));
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return int64_t(etk::avg(double(LLONG_MIN), *tmp, double(LLONG_MAX)));
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0;
}

template<>
float zeus::Buffer::internalGetParameter<float>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return *tmp;
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return *tmp;
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return *tmp;
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return *tmp;
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return *tmp;
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return *tmp;
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return *tmp;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0.0f;
}
template<>
double zeus::Buffer::internalGetParameter<double>(int32_t _id) const {
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<uint8_t>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		return *tmp;
	} else if (createType<uint16_t>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		return *tmp;
	} else if (createType<uint32_t>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		return *tmp;
	} else if (createType<uint64_t>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		return *tmp;
	} else if (createType<int8_t>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		return *tmp;
	} else if (createType<int16_t>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		return *tmp;
	} else if (createType<int32_t>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		return *tmp;
	} else if (createType<int64_t>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		return *tmp;
	} else if (createType<float>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		return *tmp;
	} else if (createType<double>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		return *tmp;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return 0.0;
}
























template<>
std::vector<uint8_t> zeus::Buffer::internalGetParameter<std::vector<uint8_t>>(int32_t _id) const {
	std::vector<uint8_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(uint8_t));
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint16_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint32_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int8_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int16_t(0), tmp[iii], int16_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int32_t(0), tmp[iii], int32_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(UCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint8_t(etk::avg(float(0), tmp[iii], float(UCHAR_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint8_t(etk::avg(double(0), tmp[iii], double(UCHAR_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}
template<>
std::vector<uint16_t> zeus::Buffer::internalGetParameter<std::vector<uint16_t>>(int32_t _id) const {
	std::vector<uint16_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(uint16_t));
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint32_t(USHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(USHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int8_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int16_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int32_t(0), tmp[iii], int32_t(USHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(USHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint16_t(etk::avg(float(0), tmp[iii], float(USHRT_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint16_t(etk::avg(double(0), tmp[iii], double(USHRT_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<uint32_t> zeus::Buffer::internalGetParameter<std::vector<uint32_t>>(int32_t _id) const {
	std::vector<uint32_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(uint32_t));
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(ULONG_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int8_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int16_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int32_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(ULONG_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint32_t(etk::avg(float(0), tmp[iii], float(ULONG_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint32_t(etk::avg(double(0), tmp[iii], double(ULONG_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<uint64_t> zeus::Buffer::internalGetParameter<std::vector<uint64_t>>(int32_t _id) const {
	std::vector<uint64_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(uint64_t));
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int8_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int16_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int32_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::max(int64_t(0), tmp[iii]);
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint64_t(etk::avg(float(0), tmp[iii], float(ULONG_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = uint64_t(etk::avg(double(0), tmp[iii], double(ULONG_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<int8_t> zeus::Buffer::internalGetParameter<std::vector<int8_t>>(int32_t _id) const {
	std::vector<int8_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint8_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint16_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint32_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(int8_t));
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int16_t(SCHAR_MIN), tmp[iii], int16_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int32_t(SCHAR_MIN), tmp[iii], int32_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(SCHAR_MIN), tmp[iii], int64_t(SCHAR_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int8_t(etk::avg(float(SCHAR_MIN), tmp[iii], float(SCHAR_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int8_t(etk::avg(double(SCHAR_MIN), tmp[iii], double(SCHAR_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<int16_t> zeus::Buffer::internalGetParameter<std::vector<int16_t>>(int32_t _id) const {
	std::vector<int16_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint16_t(SHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint32_t(SHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(SHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(int16_t));
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int32_t(SHRT_MIN), tmp[iii], int32_t(SHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(SHRT_MIN), tmp[iii], int64_t(SHRT_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int16_t(etk::avg(float(SHRT_MIN), tmp[iii], float(SHRT_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int16_t(etk::avg(double(SHRT_MIN), tmp[iii], double(SHRT_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<int32_t> zeus::Buffer::internalGetParameter<std::vector<int32_t>>(int32_t _id) const {
	std::vector<int32_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint32_t(LONG_MAX));
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(LONG_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(int32_t));
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = etk::avg(int64_t(LONG_MIN), tmp[iii], int64_t(LONG_MAX));
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int32_t(etk::avg(float(LONG_MIN), tmp[iii], float(LONG_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int32_t(etk::avg(double(LONG_MIN), tmp[iii], double(LONG_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<int64_t> zeus::Buffer::internalGetParameter<std::vector<int64_t>>(int32_t _id) const {
	std::vector<int64_t> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = std::min(tmp[iii], uint64_t(LLONG_MAX));
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(int64_t));
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int64_t(etk::avg(float(LLONG_MIN), tmp[iii], float(LLONG_MAX)));
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = int64_t(etk::avg(double(LLONG_MIN), tmp[iii], double(LLONG_MAX)));
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<float> zeus::Buffer::internalGetParameter<std::vector<float>>(int32_t _id) const {
	std::vector<float> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(float));
		return out;
	} else if (createType<std::vector<double>>() == type) {
		const double* tmp = reinterpret_cast<const double*>(pointer);
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<double> zeus::Buffer::internalGetParameter<std::vector<double>>(int32_t _id) const {
	std::vector<double> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<uint8_t>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint16_t>>() == type) {
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint32_t>>() == type) {
		const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<uint64_t>>() == type) {
		const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int8_t>>() == type) {
		const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int16_t>>() == type) {
		const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int16_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int32_t>>() == type) {
		const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int32_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<int64_t>>() == type) {
		const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(int64_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<float>>() == type) {
		const float* tmp = reinterpret_cast<const float*>(pointer);
		int32_t nbElement = dataSize / sizeof(float);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii];
		}
		return out;
	} else if (createType<std::vector<double>>() == type) {
		int32_t nbElement = dataSize / sizeof(double);
		out.resize(nbElement);
		memcpy(&out, pointer, nbElement * sizeof(double));
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<bool> zeus::Buffer::internalGetParameter<std::vector<bool>>(int32_t _id) const {
	std::vector<bool> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<bool>>() == type) {
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii] == 'T';
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
std::vector<std::string> zeus::Buffer::internalGetParameter<std::vector<std::string>>(int32_t _id) const {
	std::vector<std::string> out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	if (type == createType<std::vector<void>>()) {
		return out;
	} else if (createType<std::vector<std::string>>() == type) {
		// first element is the number of elements:
		const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
		out.resize(*tmp);
		pointer += sizeof(uint16_t);
		ZEUS_DEBUG("Parse list of string: Find " << out.size() << " elements");
		//each string is separated with a \0:
		for (int32_t iii=0; iii<out.size(); ++iii) {
			const char* tmp2 = reinterpret_cast<const char*>(pointer);
			out[iii] = tmp2;
			pointer += out[iii].size() + 1;
			ZEUS_DEBUG("    value: '" << out[iii] << "'");
		}
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
}

template<>
zeus::File zeus::Buffer::internalGetParameter<zeus::File>(int32_t _id) const {
	zeus::File out;
	zeus::ParamType type = internalGetParameterType(_id);
	const uint8_t* pointer = internalGetParameterPointer(_id);
	uint32_t dataSize = internalGetParameterSize(_id);
	// TODO : Check size ...
	if (createType<zeus::File>() == type) {
		/*
		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
		int32_t nbElement = dataSize / sizeof(uint8_t);
		out.resize(nbElement);
		for (size_t iii=0; iii<nbElement; ++iii) {
			out[iii] = tmp[iii] == 'T';
		}
		*/
		ZEUS_TODO("Generate retrun of file...");
		return out;
	}
	ZEUS_ERROR("Can not get type from '" << type << "'");
	return out;
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
		case zeus::Buffer::typeMessage::event:
			ZEUS_WARNING("get 'call' with an input type: 'event'");
			break;
		default:
			ZEUS_ERROR("unknow type: " << uint16_t(getType()));
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
		return internalGetParameter<std::string>(_id);
	}
	return paramType.getName();
}

