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
#include <etk/os/FSNode.h>
#include <zeus/mineType.h>


void zeus::addType(std::vector<uint8_t>& _data, zeus::ParamType _type) {
	_data.push_back(uint8_t(_type.getId()>>8));
	_data.push_back(uint8_t(_type.getId()));
}

void zeus::addTypeObject(std::vector<uint8_t>& _data, const std::string _type) {
	_data.push_back(uint8_t(zeus::paramTypeObject>>8));
	_data.push_back(uint8_t(zeus::paramTypeObject));
	for (auto &it : _type) {
		_data.push_back(uint8_t(it));
	}
	_data.push_back(0);
}

void zeus::addTypeRaw(std::vector<uint8_t>& _data) {
	_data.push_back(uint8_t(zeus::paramTypeRaw>>8));
	_data.push_back(uint8_t(zeus::paramTypeRaw));
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
void zeus::Buffer::internalAddParameter<std::string>(uint16_t _paramId, const std::string& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::string>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], _value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<std::string>>(uint16_t _paramId, const std::vector<std::string>& _value) {
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
void zeus::Buffer::internalAddParameter<std::vector<bool>>(uint16_t _paramId, const std::vector<bool>& _value) {
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
void zeus::Buffer::internalAddParameter<std::vector<int8_t>>(uint16_t _paramId, const std::vector<int8_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int8_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int8_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<int16_t>>(uint16_t _paramId, const std::vector<int16_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int16_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int16_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<int32_t>>(uint16_t _paramId, const std::vector<int32_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int32_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int32_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<int64_t>>(uint16_t _paramId, const std::vector<int64_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<int64_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(int64_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<uint8_t>>(uint16_t _paramId, const std::vector<uint8_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint8_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint8_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<uint16_t>>(uint16_t _paramId, const std::vector<uint16_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint16_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint16_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<uint32_t>>(uint16_t _paramId, const std::vector<uint32_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint32_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint32_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<std::vector<uint64_t>>(uint16_t _paramId, const std::vector<uint64_t>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<uint64_t>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(uint64_t)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::internalAddParameter<std::vector<float>>(uint16_t _paramId, const std::vector<float>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<float>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(float)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::internalAddParameter<std::vector<double>>(uint16_t _paramId, const std::vector<double>& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<std::vector<double>>());
	// add size:
	int32_t currentOffset = data.size();
	data.resize(data.size()+_value.size());
	memcpy(&data[currentOffset], &_value[0], sizeof(double)*_value.size());
	m_parameter.push_back(std::make_pair(2,data));
}

template<>
void zeus::Buffer::internalAddParameter<int8_t>(uint16_t _paramId, const int8_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int8_t>());
	data.push_back(uint8_t(_value));
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<uint8_t>(uint16_t _paramId, const uint8_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint8_t>());
	data.push_back(_value);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<int16_t>(uint16_t _paramId, const int16_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int16_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+2);
	memcpy(&data[currentOffset], &_value, 2);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<uint16_t>(uint16_t _paramId, const uint16_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint16_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+2);
	memcpy(&data[currentOffset], &_value, 2);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<int32_t>(uint16_t _paramId, const int32_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int32_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<uint32_t>(uint16_t _paramId, const uint32_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint32_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<int64_t>(uint16_t _paramId, const int64_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<int64_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<uint64_t>(uint16_t _paramId, const uint64_t& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<uint64_t>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<float>(uint16_t _paramId, const float& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<float>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+4);
	memcpy(&data[currentOffset], &_value, 4);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<double>(uint16_t _paramId, const double& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<double>());
	int32_t currentOffset = data.size();
	data.resize(data.size()+8);
	memcpy(&data[currentOffset], &_value, 8);
	m_parameter.push_back(std::make_pair(2,data));
}
template<>
void zeus::Buffer::internalAddParameter<bool>(uint16_t _paramId, const bool& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<bool>());
	if (_value == true) {
		data.push_back('T');
	} else {
		data.push_back('F');
	}
	m_parameter.push_back(std::make_pair(2,data));
}
#define ZEUS_MINIMUM_SIZE_MULTIPLE (1024*50)

namespace zeus {
	class SendData {
		private:
			std::vector<uint8_t> m_data;
			uint16_t m_parameterId;
			int64_t m_size;
			uint64_t m_offset;
		public:
			SendData(const std::vector<uint8_t>& _data, uint16_t _parameterId) :
			  m_data(_data),
			  m_parameterId(_parameterId),
			  m_size(_data.size()-1),
			  m_offset(0) {
				
			}
			~SendData() {
				
			}
			bool operator() (zeus::WebServer* _interface,
			                 uint32_t _clientId,
			                 uint32_t _transactionId,
			                 uint32_t _partId) {
				ememory::SharedPtr<zeus::Buffer> answer = zeus::Buffer::create();
				answer->setTransactionId(_transactionId);
				answer->setClientId(_clientId);
				answer->setPartId(_partId);
				answer->setPartFinish(false);
				answer->setType(zeus::Buffer::typeMessage::data);
				int32_t tmpSize = ZEUS_MINIMUM_SIZE_MULTIPLE;
				if (m_size < ZEUS_MINIMUM_SIZE_MULTIPLE) {
					tmpSize = m_size;
				}
				answer->addData(m_parameterId, &m_data[m_offset], tmpSize);
				m_size -= tmpSize;
				m_offset += tmpSize;
				_interface->writeBinary(answer);;
				if (m_size <= 0) {
					return true;
				}
				return false;
			}
	};
}
template<>
void zeus::Buffer::internalAddParameter<zeus::File>(uint16_t _paramId, const zeus::File& _value) {
	std::vector<uint8_t> data;
	addType(data, createType<zeus::File>());
	// set mine type in string:
	std::string name = _value.getMineType();
	int32_t currentOffset = data.size();
	data.resize(data.size()+name.size()+1);
	memcpy(&data[currentOffset], &name[0], name.size());
	// finish with '\0'
	currentOffset = data.size()-1;
	data[currentOffset] = 0;
	// set size if the file in int32_t
	int32_t size = _value.getTheoricFileSize();
	currentOffset = data.size();
	data.resize(data.size()+sizeof(int32_t));
	memcpy(&data[currentOffset], &size, sizeof(int32_t));
	// and now the data (can be none ...):
	const std::vector<uint8_t>& dataFile = _value.getData();
	if (dataFile.size() != 0) {
		currentOffset = data.size();
		if (dataFile.size() < ZEUS_MINIMUM_SIZE_MULTIPLE) {
			data.resize(data.size()+dataFile.size());
			memcpy(&data[currentOffset], &dataFile[0], dataFile.size());
		} else {
			m_multipleSend.push_back(zeus::SendData(dataFile, _paramId));
		}
	}
	m_parameter.push_back(std::make_pair(2,data));
}
namespace zeus {
	class SendFile {
		private:
			etk::FSNode m_node;
			uint16_t m_parameterId;
			int64_t m_size;
		public:
			SendFile(const std::string& _data, uint16_t _parameterId, uint32_t _size) :
			  m_node(_data),
			  m_parameterId(_parameterId),
			  m_size(_size) {
				
			}
			~SendFile() {
				
			}
			bool operator() (zeus::WebServer* _interface,
			                 uint32_t _clientId,
			                 uint32_t _transactionId,
			                 uint32_t _partId) {
				if (m_node.fileIsOpen() == false) {
					m_node.fileOpenRead();
				}
				ememory::SharedPtr<zeus::Buffer> answer = zeus::Buffer::create();
				answer->setTransactionId(_transactionId);
				answer->setClientId(_clientId);
				answer->setPartId(_partId);
				answer->setPartFinish(false);
				answer->setType(zeus::Buffer::typeMessage::data);
				int32_t tmpSize = ZEUS_MINIMUM_SIZE_MULTIPLE;
				if (m_size < ZEUS_MINIMUM_SIZE_MULTIPLE) {
					tmpSize = m_size;
				}
				uint8_t tmpData[ZEUS_MINIMUM_SIZE_MULTIPLE];
				m_node.fileRead(tmpData, 1, tmpSize);
				answer->addData(m_parameterId, tmpData, tmpSize);
				m_size -= tmpSize;
				_interface->writeBinary(answer);;
				if (m_size <= 0) {
					m_node.fileClose();
					return true;
				}
				return false;
			}
	};
}

template<>
void zeus::Buffer::internalAddParameter<zeus::FileServer>(uint16_t _paramId, const zeus::FileServer& _value) {
	etk::FSNode node(_value.getFileName());
	node.fileOpenRead();
	std::string extention = std::string(_value.getFileName().begin()+_value.getFileName().size() -3, _value.getFileName().end());
	ZEUS_WARNING("send file: '" << _value.getFileName() << "' with extention: '" << extention << "'");
	uint64_t size = node.fileSize();
	std::vector<uint8_t> fileData;
	if (size < ZEUS_MINIMUM_SIZE_MULTIPLE) {
		// if the file is small ==> send directly ...
		fileData.resize(size);
		node.fileRead(&fileData[0], 1, size);
	}
	zeus::File tmpFile(zeus::getMineType(extention), fileData, size);
	internalAddParameter(_paramId, tmpFile);
	node.fileClose();
	if (size >= ZEUS_MINIMUM_SIZE_MULTIPLE) {
		m_multipleSend.push_back(zeus::SendFile(_value.getFileName(), _paramId, size));
	}
}
