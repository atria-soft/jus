/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.h>
#include <jus/Buffer.h>
#include <jus/debug.h>

jus::Buffer::Buffer() {
	clear();
}

void jus::Buffer::clear() {
	m_data.clear();
	m_paramOffset.clear();
	m_header.clear();
}
std::string jus::Buffer::generateHumanString() {
	return "jus::Buffer ...";
}

uint16_t jus::Buffer::getProtocalVersion() {
	return m_header.versionProtocol;
}

void jus::Buffer::setProtocolVersion(uint16_t _value) {
	m_header.versionProtocol = _value;
}

uint32_t jus::Buffer::getTransactionId() {
	return m_header.transactionID;
}

void jus::Buffer::setTransactionId(uint32_t _value) {
	m_header.transactionID = _value;
}

uint32_t jus::Buffer::getClientId() {
	return m_header.clientID;
}

void jus::Buffer::setClientId(uint32_t _value) {
	m_header.clientID = _value;
}

// note limited 15 bits
uint16_t jus::Buffer::getPartId() {
	return uint16_t(m_header.partID & 0x7FFF);
}

void jus::Buffer::setPartId(uint16_t _value) {
	m_header.partID = (m_header.partID&0x8000) | (_value & 0x7FFF);
}

bool jus::Buffer::getPartFinish() {
	return m_header.versionProtocol<0;
}

void jus::Buffer::setPartFinish(bool _value) {
	if (_value == true) {
		m_header.versionProtocol = (m_header.versionProtocol & 0x7FFF) | 0x8000;
	} else {
		m_header.versionProtocol = m_header.versionProtocol & 0x7FFF;
	}
}

enum jus::Buffer::typeMessage jus::Buffer::getType() {
	return (enum jus::Buffer::typeMessage)m_header.typeMessage;
}

void jus::Buffer::setType(enum typeMessage _value) {
	m_header.typeMessage = uint16_t(_value);
}

uint16_t jus::Buffer::getNumberParameter() {
	return m_paramOffset.size()-1;
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
	m_data.resize(m_data.size()+_value.size()+1);
	memcpy(&m_data[currentOffset], &_value[0], m_data.size());
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
	m_data.resize(m_data.size()+8);
	memcpy(&m_data[currentOffset], &_value, 8);
}

template<>
std::string jus::Buffer::internalGetParameter<std::string>(int32_t _id) {
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
	char* type = reinterpret_cast<char*>(&m_data[startPos]); // Will be stop with \0 ...
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
int8_t jus::Buffer::internalGetParameter<int8_t>(int32_t _id) {
	int8_t out;
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
	char* type = reinterpret_cast<char*>(&m_data[startPos]); // Will be stop with \0 ...
	if (strcmp(type, "int8") == 0) {
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
			if (dataSize != 1) {
				JUS_ERROR("Get size > 1 : " << dataSize << " ==> limit 1");
				dataSize = 1;
			}
			// Allocate data
			memcpy(&out, &m_data[startPos], dataSize);
		}
	} else {
		//wrong type ...
		JUS_ERROR("Can not convert '" << type << "' into 'string'");
	}
	return out;
}




std::string jus::Buffer::getCall() {
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