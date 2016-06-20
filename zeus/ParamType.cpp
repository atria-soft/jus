/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/ParamType.h>
#include <zeus/debug.h>
#include <zeus/File.h>


zeus::ParamType::ParamType(const char* _name):
  m_typeName(_name) {
	
}

const char* zeus::ParamType::getName() const {
	return m_typeName;
}

bool zeus::ParamType::operator == (const ParamType& _obj) const {
	return m_typeName == _obj.m_typeName;
}

bool zeus::ParamType::operator == (const std::string& _value) const {
	return _value == m_typeName;
}
bool zeus::ParamType::operator != (const std::string& _value) const {
	return _value != m_typeName;
}

#define generate_basic_type(_type, _name) \
namespace zeus { \
	template<> zeus::ParamType createType<_type>() {\
		return zeus::ParamType(_name); \
	} \
}

generate_basic_type(void, "void");
generate_basic_type(bool, "bool");
generate_basic_type(float, "float");
generate_basic_type(double, "double");
generate_basic_type(int64_t, "int64");
generate_basic_type(int32_t, "int32");
generate_basic_type(int16_t, "int16");
generate_basic_type(int8_t, "int8");
generate_basic_type(uint64_t, "uint64");
generate_basic_type(uint32_t, "uint32");
generate_basic_type(uint16_t, "uint16");
generate_basic_type(uint8_t, "uint8");
generate_basic_type(std::string, "string");


generate_basic_type(std::vector<bool>, "vector:bool");
generate_basic_type(std::vector<float>, "vector:float");
generate_basic_type(std::vector<double>, "vector:double");
generate_basic_type(std::vector<int64_t>, "vector:int64");
generate_basic_type(std::vector<int32_t>, "vector:int32");
generate_basic_type(std::vector<int16_t>, "vector:int16");
generate_basic_type(std::vector<int8_t>, "vector:int8");
generate_basic_type(std::vector<uint64_t>, "vector:uint64");
generate_basic_type(std::vector<uint32_t>, "vector:uint32");
generate_basic_type(std::vector<uint16_t>, "vector:uint16");
generate_basic_type(std::vector<uint8_t>, "vector:uint8");
generate_basic_type(std::vector<std::string>, "vector:string");


generate_basic_type(zeus::File, "file");
generate_basic_type(zeus::FileServer, "file");


