/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/Buffer.hpp>
#include <zeus/BufferData.hpp>
#include <zeus/debug.hpp>
#include <zeus/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/BufferParameter.hpp>
#include <zeus/File.hpp>

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
	if (    typeId == paramTypeObject
	     || typeId == paramTypeService) {
		const char* tmp = reinterpret_cast<const char*>(&m_parameter[_id].second[2]);
		bool find = false;
		for (int32_t iii=0; iii<1024; ++iii) {
			if (tmp[iii] == 0) {
				find = true;
				break;
			}
		}
		if (find == false) {
			ZEUS_ERROR("Request object with a name too big ==> error ...");
			m_parameter[_id].first = 0;
			return zeus::ParamType("no-name", typeId);
		}
		std::string type(tmp);
		m_parameter[_id].first = type.size() + sizeof(uint16_t) + 1; // add \0
		return zeus::ParamType(type, typeId);
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

void zeus::BufferParameter::parameterAppendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) {
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
