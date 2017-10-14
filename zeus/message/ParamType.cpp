/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/message/ParamType.hpp>
#include <zeus/debug.hpp>
#include <zeus/File.hpp>

#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::message::ParamType);

bool zeus::message::checkCompatibility(const ParamType& _type, const ParamType& _params) {
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


etk::Stream& zeus::message::operator <<(etk::Stream& _os, const zeus::message::ParamType& _obj) {
	_os << "{" << _obj.getId() << ":" << _obj.getName() << "}";
	return _os;
}

zeus::message::ParamType::ParamType(const char* _name, uint16_t _id, bool _isNumber, bool _isVector):
  m_typeName(_name),
  m_id(_id),
  m_isNumber(_isNumber),
  m_isVector(_isVector) {
	
}
zeus::message::ParamType::ParamType(const etk::String& _name, uint16_t _id, bool _isNumber, bool _isVector):
  m_typeName(_name),
  m_id(_id),
  m_isNumber(_isNumber),
  m_isVector(_isVector) {
	
}

bool zeus::message::ParamType::isNumber() const {
	return m_isNumber;
}

bool zeus::message::ParamType::isVector() const {
	return m_isVector;
}
const etk::String& zeus::message::ParamType::getName() const {
	return m_typeName;
}

uint16_t zeus::message::ParamType::getId() const {
	return m_id;
}

bool zeus::message::ParamType::operator == (const ParamType& _obj) const {
	if (m_id != _obj.m_id) {
		return false;
	}
	if (m_id == paramTypeObject) {
		return m_typeName == _obj.m_typeName;
	}
	return true;
}
bool zeus::message::ParamType::operator != (const ParamType& _obj) const {
	if (m_id != _obj.m_id) {
		return true;
	}
	if (m_id == paramTypeObject) {
		return m_typeName != _obj.m_typeName;
	}
	return false;
}

bool zeus::message::ParamType::operator == (const etk::String& _value) const {
	return _value == m_typeName;
}
bool zeus::message::ParamType::operator != (const etk::String& _value) const {
	return _value != m_typeName;
}

bool zeus::message::ParamType::operator == (const uint16_t& _value) const {
	return _value == m_id;
}

bool zeus::message::ParamType::operator != (const uint16_t& _value) const {
	return _value != m_id;
}

#define generate_basic_type(_type, _name, _id, _num, _vect) \
namespace zeus { \
	namespace message { \
		template<> const zeus::message::ParamType& createType<_type>() {\
			static zeus::message::ParamType type(_name, _id, _num, _vect); \
			return type; \
		} \
	} \
}

generate_basic_type(void, "void", 0x0001, false, false);
generate_basic_type(bool, "bool", 0x0002, false, false);
generate_basic_type(float, "float", 0x0003, true, false);
generate_basic_type(double, "double", 0x0004, true, false);
generate_basic_type(int64_t, "int64", 0x0005, true, false);
generate_basic_type(int32_t, "int32", 0x0006, true, false);
generate_basic_type(int16_t, "int16", 0x0007, true, false);
generate_basic_type(int8_t, "int8", 0x0008, true, false);
generate_basic_type(uint64_t, "uint64", 0x0009, true, false);
generate_basic_type(uint32_t, "uint32", 0x000A, true, false);
generate_basic_type(uint16_t, "uint16", 0x000B, true, false);
generate_basic_type(uint8_t, "uint8", 0x000C, true, false);
generate_basic_type(etk::String, "string", 0x000D, false, false);
generate_basic_type(zeus::Raw, "raw", 0x000E, false, false);

generate_basic_type(etk::Vector<void>, "vector:empty", 0x0102, true, true);
generate_basic_type(etk::Vector<bool>, "vector:bool", 0x0102, false, true);
generate_basic_type(etk::Vector<float>, "vector:float", 0x0103, true, true);
generate_basic_type(etk::Vector<double>, "vector:double", 0x0104, true, true);
generate_basic_type(etk::Vector<int64_t>, "vector:int64", 0x0105, true, true);
generate_basic_type(etk::Vector<int32_t>, "vector:int32", 0x0106, true, true);
generate_basic_type(etk::Vector<int16_t>, "vector:int16", 0x0107, true, true);
generate_basic_type(etk::Vector<int8_t>, "vector:int8", 0x0108, true, true);
generate_basic_type(etk::Vector<uint64_t>, "vector:uint64", 0x0109, true, true);
generate_basic_type(etk::Vector<uint32_t>, "vector:uint32", 0x010A, true, true);
generate_basic_type(etk::Vector<uint16_t>, "vector:uint16", 0x010B, true, true);
generate_basic_type(etk::Vector<uint8_t>, "vector:uint8", 0x010C, true, true);
generate_basic_type(etk::Vector<etk::String>, "vector:string", 0x010D, false, true);

const uint16_t zeus::message::paramTypeObject = 0xFFFF;

