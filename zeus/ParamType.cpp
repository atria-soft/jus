/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ParamType.h>
#include <zeus/debug.h>
#include <zeus/File.h>


bool zeus::checkCompatibility(const ParamType& _type, const ParamType& _params) {
	if (_params == _type) {
		return true;
	}
	// check cross compatibilité (All number are convertible (with min/max) and all number vector are convertible (min/max too)
	if (    _type.isNumber() == true
	     && _params.isNumber() == true) {
		return _type.isVector() == _params.isVector();
	}
	return false;
}


std::ostream& zeus::operator <<(std::ostream& _os, const zeus::ParamType& _obj) {
	_os << "{" << _obj.getId() << ":" << _obj.getName() << "}";
	return _os;
}

zeus::ParamType::ParamType(const char* _name, uint16_t _id, bool _isNumber, bool _isVector):
  m_typeName(_name),
  m_id(_id),
  m_isNumber(_isNumber),
  m_isVector(_isVector) {
	
}
zeus::ParamType::ParamType(const std::string& _name, uint16_t _id, bool _isNumber, bool _isVector):
  m_typeName(_name),
  m_id(_id),
  m_isNumber(_isNumber),
  m_isVector(_isVector) {
	
}

bool zeus::ParamType::isNumber() const {
	return m_isNumber;
}

bool zeus::ParamType::isVector() const {
	return m_isVector;
}
const std::string& zeus::ParamType::getName() const {
	return m_typeName;
}

uint16_t zeus::ParamType::getId() const {
	return m_id;
}

bool zeus::ParamType::operator == (const ParamType& _obj) const {
	if (m_id != _obj.m_id) {
		return false;
	}
	if (m_id == paramTypeObject) {
		return m_typeName == _obj.m_typeName;
	}
	return true;
}
bool zeus::ParamType::operator != (const ParamType& _obj) const {
	if (m_id != _obj.m_id) {
		return true;
	}
	if (m_id == paramTypeObject) {
		return m_typeName != _obj.m_typeName;
	}
	return false;
}

bool zeus::ParamType::operator == (const std::string& _value) const {
	return _value == m_typeName;
}
bool zeus::ParamType::operator != (const std::string& _value) const {
	return _value != m_typeName;
}

bool zeus::ParamType::operator == (const uint16_t& _value) const {
	return _value == m_id;
}

bool zeus::ParamType::operator != (const uint16_t& _value) const {
	return _value != m_id;
}

#define generate_basic_type(_type, _name, _id, _num, _vect) \
namespace zeus { \
	template<> zeus::ParamType createType<_type>() {\
		return zeus::ParamType(_name, _id, _num, _vect); \
	} \
}

generate_basic_type(void, "void", 0x1, false, false);
generate_basic_type(bool, "bool", 0x2, false, false);
generate_basic_type(float, "float", 0x3, true, false);
generate_basic_type(double, "double", 0x4, true, false);
generate_basic_type(int64_t, "int64", 0x5, true, false);
generate_basic_type(int32_t, "int32", 0x6, true, false);
generate_basic_type(int16_t, "int16", 0x7, true, false);
generate_basic_type(int8_t, "int8", 0x8, true, false);
generate_basic_type(uint64_t, "uint64", 0x9, true, false);
generate_basic_type(uint32_t, "uint32", 0xA, true, false);
generate_basic_type(uint16_t, "uint16", 0xB, true, false);
generate_basic_type(uint8_t, "uint8", 0xC, true, false);
generate_basic_type(std::string, "string", 0xD, false, false);


generate_basic_type(std::vector<void>, "vector:empty", 0x0102, true, true);
generate_basic_type(std::vector<bool>, "vector:bool", 0x0102, false, true);
generate_basic_type(std::vector<float>, "vector:float", 0x0103, true, true);
generate_basic_type(std::vector<double>, "vector:double", 0x0104, true, true);
generate_basic_type(std::vector<int64_t>, "vector:int64", 0x0105, true, true);
generate_basic_type(std::vector<int32_t>, "vector:int32", 0x0106, true, true);
generate_basic_type(std::vector<int16_t>, "vector:int16", 0x0107, true, true);
generate_basic_type(std::vector<int8_t>, "vector:int8", 0x0108, true, true);
generate_basic_type(std::vector<uint64_t>, "vector:uint64", 0x0109, true, true);
generate_basic_type(std::vector<uint32_t>, "vector:uint32", 0x010A, true, true);
generate_basic_type(std::vector<uint16_t>, "vector:uint16", 0x010B, true, true);
generate_basic_type(std::vector<uint8_t>, "vector:uint8", 0x010C, true, true);
generate_basic_type(std::vector<std::string>, "vector:string", 0x010D, false, true);

generate_basic_type(zeus::File, "file", 0x000E, false, false);
generate_basic_type(zeus::FileServer, "file", 0x000E, false, false);

const uint16_t zeus::paramTypeObject = 0xFFFF;
const uint16_t zeus::paramTypeRaw = 0xFFFE;

