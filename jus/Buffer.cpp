/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.h>
#include <jus/Buffer.h>
#include <jus/debug.h>
#include <jus/ParamType.h>

namespace etk {
	template<> std::string to_string<enum jus::Buffer::typeMessage>(const enum jus::Buffer::typeMessage& _value) {
		switch (_value) {
			case jus::Buffer::typeMessage::call:
				return "call";
			case jus::Buffer::typeMessage::answer:
				return "answer";
			case jus::Buffer::typeMessage::event:
				return "event";
		}
		return "???";
	}
}
std::ostream& jus::operator <<(std::ostream& _os, enum jus::Buffer::typeMessage _value) {
	_os << etk::to_string(_value);
	return _os;
}

static enum jus::Buffer::typeMessage getTypeType(uint16_t _value) {
	switch (_value) {
		case 1:
			return jus::Buffer::typeMessage::call;
		case 2:
			return jus::Buffer::typeMessage::answer;
		case 4:
			return jus::Buffer::typeMessage::event;
	}
	return jus::Buffer::typeMessage::call;
}

jus::Buffer::Buffer() {
	clear();
}

void jus::Buffer::composeWith(const std::vector<uint8_t>& _buffer) {
	clear();
	m_header.lenght = _buffer.size();
	uint32_t offset = 0;
	memcpy(reinterpret_cast<char*>(&m_header) + sizeof(uint32_t), &_buffer[offset], sizeof(headerBin)-sizeof(uint32_t));
	offset += sizeof(headerBin)-sizeof(uint32_t);
	if (m_header.numberOfParameter != 0) {
		m_paramOffset.resize(m_header.numberOfParameter);
		memcpy(&m_paramOffset[0], &_buffer[offset], m_header.numberOfParameter * sizeof(uint16_t));
		offset += m_header.numberOfParameter * sizeof(uint16_t);
		m_data.resize(_buffer.size() - offset);
		memcpy(&m_data[0], &_buffer[offset], m_data.size());
	} else {
		// TODO : check size ...
	}
	JUS_INFO("Get binary messages " << generateHumanString());
}

void jus::Buffer::clear() {
	JUS_WARNING("clear buffer");
	m_data.clear();
	m_paramOffset.clear();
	m_header.lenght = 0;
	m_header.versionProtocol = 1;
	m_header.transactionID = 1;
	m_header.clientID = 0;
	m_header.partID = 0x8000;
	m_header.typeMessage = 1;
	m_header.numberOfParameter = 1;
}
std::string jus::Buffer::generateHumanString() {
	std::string out = "jus::Buffer Lenght=: ";
	out += etk::to_string(m_header.lenght);
	out += " v=" + etk::to_string(m_header.versionProtocol);
	out += " id=" + etk::to_string(m_header.transactionID);
	out += " cId=" + etk::to_string(m_header.clientID);
	out += " pId=" + etk::to_string(getPartId());
	out += " finish=" + etk::to_string(getPartFinish());
	enum jus::Buffer::typeMessage type = getTypeType(m_header.typeMessage);
	out += " type=" + etk::to_string(type);
	switch (type) {
		case jus::Buffer::typeMessage::call:
			out += " nbParam=" + etk::to_string(getNumberParameter());
			break;
		case jus::Buffer::typeMessage::answer:
			if (getNumberParameter() == 1) {
				out += " mode=Value";
			} else if (getNumberParameter() == 2) {
				out += " mode=Error";
			} else if (getNumberParameter() == 3) {
				out += " mode=Value+Error";
			} else {
				out += " mode=???";
			}
			break;
		case jus::Buffer::typeMessage::event:
			
			break;
	}
	if (getNumberParameter() != 0) {
		out += " paramType(";
		for (int32_t iii=0; iii< getNumberParameter(); ++iii) {
			if (iii != 0) {
				out += ",";
			}
			out += internalGetParameterType(iii);
		}
		out += ")";
	}
	return out;
}

uint16_t jus::Buffer::getProtocalVersion() const {
	return m_header.versionProtocol;
}

void jus::Buffer::setProtocolVersion(uint16_t _value) {
	JUS_WARNING("setProtocolVersion :" << _value);
	m_header.versionProtocol = _value;
}

uint32_t jus::Buffer::getTransactionId() const {
	return m_header.transactionID;
}

void jus::Buffer::setTransactionId(uint32_t _value) {
	JUS_WARNING("setTransactionId :" << _value);
	m_header.transactionID = _value;
}

uint32_t jus::Buffer::getClientId() const {
	return m_header.clientID;
}

void jus::Buffer::setClientId(uint32_t _value) {
	JUS_WARNING("setClientId :" << _value);
	m_header.clientID = _value;
}

// note limited 15 bits
uint16_t jus::Buffer::getPartId() const {
	return uint16_t(m_header.partID & 0x7FFF);
}

void jus::Buffer::setPartId(uint16_t _value) {
	JUS_WARNING("setPartId :" << _value);
	m_header.partID = (m_header.partID&0x8000) | (_value & 0x7FFF);
}

bool jus::Buffer::getPartFinish() const {
	return m_header.partID<0;
}

void jus::Buffer::setPartFinish(bool _value) {
	JUS_WARNING("setPartFinish :" << _value);
	if (_value == true) {
		m_header.partID = (m_header.partID & 0x7FFF) | 0x8000;
	} else {
		m_header.partID = m_header.partID & 0x7FFF;
	}
}

enum jus::Buffer::typeMessage jus::Buffer::getType() const {
	return (enum jus::Buffer::typeMessage)m_header.typeMessage;
}

void jus::Buffer::setType(enum typeMessage _value) {
	JUS_WARNING("setType :" << _value);
	m_header.typeMessage = uint16_t(_value);
}

uint16_t jus::Buffer::getNumberParameter() const {
	return m_paramOffset.size()-1;
}
std::string jus::Buffer::internalGetParameterType(int32_t _id) const {
	std::string out;
	if (m_paramOffset.size() <= _id) {
		JUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_paramOffset.size());
		return out;
	}
	out = reinterpret_cast<const char*>(&m_data[m_paramOffset[_id]]);
	return out;
}
std::string jus::Buffer::getParameterType(int32_t _id) const {
	return internalGetParameterType(_id + 1);
}

const uint8_t* jus::Buffer::internalGetParameterPointer(int32_t _id) const {
	const uint8_t* out = nullptr;
	if (m_paramOffset.size() <= _id) {
		JUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_paramOffset.size());
		return out;
	}
	out = reinterpret_cast<const uint8_t*>(&m_data[m_paramOffset[_id]]);
	if (out == nullptr) {
		return out;
	}
	// TODO : unlock if > 1024
	while (*out != 0) {
		out++;
	}
	out++;
	return out;
}

const uint8_t* jus::Buffer::getParameterPointer(int32_t _id) const {
	return internalGetParameterPointer(_id + 1);
}

uint32_t jus::Buffer::internalGetParameterSize(int32_t _id) const {
	int32_t out = 0;
	if (m_paramOffset.size() <= _id) {
		JUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_paramOffset.size());
		return out;
	}
	int32_t startPos = m_paramOffset[_id];
	int32_t endPos = m_data.size();
	if (m_paramOffset.size() > _id+1) {
		endPos = m_paramOffset[_id+1];
	}
	// First get type:
	const char* type = reinterpret_cast<const char*>(&m_data[startPos]); // Will be stop with \0 ...
	// move in the buffer pos
	startPos += strlen(type) + 1;
	// get real data size
	out = endPos - startPos;
	if (out < 0) {
		JUS_ERROR("Get size < 0 : " << out);
		out = 0;
	}
	return out;
}

uint32_t jus::Buffer::getParameterSize(int32_t _id) const {
	return internalGetParameterSize(_id + 1);
}

void jus::Buffer::addParameter() {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('v');
	m_data.push_back('o');
	m_data.push_back('i');
	m_data.push_back('d');
	m_data.push_back('\0');
}
template<>
void jus::Buffer::addParameter<std::string>(const std::string& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('s');
	m_data.push_back('t');
	m_data.push_back('r');
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('g');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+_value.size()+1);
	memcpy(&m_data[currentOffset], &_value[0], _value.size());
}
template<>
void jus::Buffer::addParameter<int8_t>(const int8_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('8');
	m_data.push_back('\0');
	m_data.push_back(uint8_t(_value));
}
template<>
void jus::Buffer::addParameter<uint8_t>(const uint8_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('u');
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('8');
	m_data.push_back('\0');
	m_data.push_back(_value);
}
template<>
void jus::Buffer::addParameter<int16_t>(const int16_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('1');
	m_data.push_back('6');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+2);
	memcpy(&m_data[currentOffset], &_value, 2);
}
template<>
void jus::Buffer::addParameter<uint16_t>(const uint16_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('u');
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('1');
	m_data.push_back('6');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+2);
	memcpy(&m_data[currentOffset], &_value, 2);
}
template<>
void jus::Buffer::addParameter<int32_t>(const int32_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('3');
	m_data.push_back('2');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+4);
	memcpy(&m_data[currentOffset], &_value, 4);
}
template<>
void jus::Buffer::addParameter<uint32_t>(const uint32_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('u');
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('3');
	m_data.push_back('2');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+4);
	memcpy(&m_data[currentOffset], &_value, 4);
}
template<>
void jus::Buffer::addParameter<int64_t>(const int64_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('3');
	m_data.push_back('2');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+8);
	memcpy(&m_data[currentOffset], &_value, 8);
}
template<>
void jus::Buffer::addParameter<uint64_t>(const uint64_t& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('u');
	m_data.push_back('i');
	m_data.push_back('n');
	m_data.push_back('t');
	m_data.push_back('6');
	m_data.push_back('4');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+8);
	memcpy(&m_data[currentOffset], &_value, 8);
}
template<>
void jus::Buffer::addParameter<float>(const float& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('f');
	m_data.push_back('l');
	m_data.push_back('o');
	m_data.push_back('a');
	m_data.push_back('t');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+4);
	memcpy(&m_data[currentOffset], &_value, 4);
}
template<>
void jus::Buffer::addParameter<double>(const double& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('d');
	m_data.push_back('o');
	m_data.push_back('u');
	m_data.push_back('b');
	m_data.push_back('l');
	m_data.push_back('e');
	m_data.push_back('\0');
	currentOffset = m_data.size();
	m_data.resize(m_data.size()+8);
	memcpy(&m_data[currentOffset], &_value, 8);
}
template<>
void jus::Buffer::addParameter<bool>(const bool& _value) {
	int32_t currentOffset = m_data.size();
	m_paramOffset.push_back(currentOffset);
	m_data.push_back('b');
	m_data.push_back('o');
	m_data.push_back('o');
	m_data.push_back('l');
	m_data.push_back('\0');
	if (_value == true) {
		m_data.push_back('T');
	} else {
		m_data.push_back('F');
	}
}

template<>
std::string jus::Buffer::internalGetParameter<std::string>(int32_t _id) const {
	std::string out;
	if (m_paramOffset.size() <= _id) {
		JUS_ERROR("out of range Id for parameter ... " << _id << " have " << m_paramOffset.size());
		return out;
	}
	int32_t startPos = m_paramOffset[_id];
	int32_t endPos = m_data.size();
	if (m_paramOffset.size() > _id+1) {
		endPos = m_paramOffset[_id+1];
	}
	// First get type:
	const char* type = reinterpret_cast<const char*>(&m_data[startPos]); // Will be stop with \0 ...
	if (strcmp(type, "string") == 0) {
		// OK
		// move in the buffer pos
		startPos += strlen(type) + 1;
		// get real data size
		int32_t dataSize = endPos - startPos;
		if (dataSize < 0) {
			JUS_ERROR("Get size < 0 : " << dataSize);
		} else if (dataSize < 0) {
			// nothing to do ...
		} else {
			// Allocate data
			out.resize(dataSize, ' ');
			memcpy(&out[0], &m_data[startPos], dataSize);
		}
	} else {
		//wrong type ...
		JUS_ERROR("Can not convert '" << type << "' into 'string'");
	}
	
	return out;
}


template<>
int8_t jus::Buffer::internalGetParameter<int8_t>(int32_t _id) const {
	std::string type = getParameterType(_id);
	const uint8_t* pointer = getParameterPointer(_id);
	uint32_t dataSize = getParameterSize(_id);
	if (createType<int8_t>() != type) {
		return 0;
	}
	const int8_t* pointer2 = reinterpret_cast<const int8_t*>(pointer);
	return *pointer2;
}

template<>
int32_t jus::Buffer::internalGetParameter<int32_t>(int32_t _id) const {
	std::string type = getParameterType(_id);
	const uint8_t* pointer = getParameterPointer(_id);
	uint32_t dataSize = getParameterSize(_id);
	if (createType<int32_t>() != type) {
		return 0;
	}
	const int32_t* pointer2 = reinterpret_cast<const int32_t*>(pointer);
	return *pointer2;
}


void jus::Buffer::addError(const std::string& _value, const std::string& _comment) {
	addParameter(_value);
	addParameter(_comment);
}

std::string jus::Buffer::getCall() const {
	std::string out;
	switch(getType()) {
		case jus::Buffer::typeMessage::call:
			return internalGetParameter<std::string>(0);
			break;
		case jus::Buffer::typeMessage::answer:
			JUS_WARNING("get 'call' with an input type: 'answer'");
			break;
		case jus::Buffer::typeMessage::event:
			JUS_WARNING("get 'call' with an input type: 'event'");
			break;
		default:
			JUS_ERROR("unknow type: " << uint16_t(getType()));
			break;
	}
	return "";
}

void jus::Buffer::setCall(std::string _value) {
	if (m_paramOffset.size() != 0) {
		JUS_ERROR("Clear Buffer of parameter ==> set the call type in first ...");
		m_paramOffset.clear();
		m_data.clear();
	}
	addParameter(_value);
}

void jus::Buffer::prepare() {
	m_header.numberOfParameter = m_paramOffset.size();
	m_header.lenght = sizeof(headerBin) - sizeof(uint32_t);
	m_header.lenght += m_paramOffset.size() * sizeof(uint16_t); // param list
	m_header.lenght += m_data.size();
}

ejson::Object jus::Buffer::toJson() const {
	ejson::Object out;
	out.add("id", ejson::Number(getTransactionId()));
	out.add("client-id", ejson::Number(getClientId()));
	uint16_t partId = getPartId();
	bool partFinish = getPartFinish();
	if (    partId != 0
	     || partFinish == false) {
		out.add("part", ejson::Number(partId));
		if (partFinish == true) {
			out.add("finish", ejson::Boolean(partFinish));
		}
	}
	if (getType() == jus::Buffer::typeMessage::call) {
		out.add("call", ejson::String(getCall()));
		ejson::Array listParam;
		out.add("param", listParam);
		for (int32_t iii=0; iii<getNumberParameter(); ++iii) {
			std::string type = getParameterType(iii);
			const uint8_t* dataPointer = getParameterPointer(iii);
			uint32_t dataLenght = getParameterSize(iii);
			ejson::Value param;
			if (createType<bool>() == type) {
				param = convertBinaryToJson<bool>(dataPointer, dataLenght);
			} else if (createType<int8_t>() == type) {
				param = convertBinaryToJson<int8_t>(dataPointer, dataLenght);
			} else if (createType<int16_t>() == type) {
				param = convertBinaryToJson<int16_t>(dataPointer, dataLenght);
			} else if (createType<int32_t>() == type) {
				param = convertBinaryToJson<int32_t>(dataPointer, dataLenght);
			} else if (createType<int64_t>() == type) {
				param = convertBinaryToJson<int64_t>(dataPointer, dataLenght);
			} else if (createType<uint8_t>() == type) {
				param = convertBinaryToJson<uint8_t>(dataPointer, dataLenght);
			} else if (createType<uint16_t>() == type) {
				param = convertBinaryToJson<uint16_t>(dataPointer, dataLenght);
			} else if (createType<uint32_t>() == type) {
				param = convertBinaryToJson<uint32_t>(dataPointer, dataLenght);
			} else if (createType<uint64_t>() == type) {
				param = convertBinaryToJson<uint64_t>(dataPointer, dataLenght);
			} else if (createType<float>() == type) {
				param = convertBinaryToJson<float>(dataPointer, dataLenght);
			} else if (createType<double>() == type) {
				param = convertBinaryToJson<double>(dataPointer, dataLenght);
			} else if (createType<std::string>() == type) {
				param = convertBinaryToJson<std::string>(dataPointer, dataLenght);
			} else if (createType<std::vector<bool>>() == type) {
				//param = convertBinaryToJson<std::vector<bool>>(dataPointer, dataLenght);
			} else if (createType<std::vector<int8_t>>() == type) {
				//param = convertBinaryToJson<std::vector<int8_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<int16_t>>() == type) {
				//param = convertBinaryToJson<std::vector<int16_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<int32_t>>() == type) {
				//param = convertBinaryToJson<std::vector<int32_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<int64_t>>() == type) {
				//param = convertBinaryToJson<std::vector<int64_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<uint8_t>>() == type) {
				//param = convertBinaryToJson<std::vector<uint8_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<uint16_t>>() == type) {
				//param = convertBinaryToJson<std::vector<uint16_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<uint32_t>>() == type) {
				//param = convertBinaryToJson<std::vector<uint32_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<uint64_t>>() == type) {
				//param = convertBinaryToJson<std::vector<uint64_t>>(dataPointer, dataLenght);
			} else if (createType<std::vector<float>>() == type) {
				//param = convertBinaryToJson<std::vector<float>>(dataPointer, dataLenght);
			} else if (createType<std::vector<double>>() == type) {
				//param = convertBinaryToJson<std::vector<double>>(dataPointer, dataLenght);
			} else if (createType<std::vector<std::string>>() == type) {
				param = convertBinaryToJson<std::vector<std::string>>(dataPointer, dataLenght);
			} else {
				JUS_ERROR("Unknow param ==> can not convert ...");
			}
			listParam.add(param);
		}
	} else if (getType() == jus::Buffer::typeMessage::answer) {
		
	} else if (getType() == jus::Buffer::typeMessage::event) {
		JUS_ERROR(" NOT managed ...");
	} else {
		JUS_ERROR("Unknow TYPE ...");
	}
	return out;
}
