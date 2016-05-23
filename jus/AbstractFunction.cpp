/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/AbstractFunction.h>
#include <jus/debug.h>

template<> bool convertJsonTo<bool>(const ejson::Value& _value) {
	return _value.toBoolean().get();
}
template<> float convertJsonTo<float>(const ejson::Value& _value) {
	return _value.toNumber().get();
}
template<> double convertJsonTo<double>(const ejson::Value& _value) {
	return _value.toNumber().get();
}
template<> int64_t convertJsonTo<int64_t>(const ejson::Value& _value) {
	return int64_t(_value.toNumber().get());
}
template<> int32_t convertJsonTo<int32_t>(const ejson::Value& _value) {
	//_value.display();
	return int32_t(_value.toNumber().get());
}
template<> int16_t convertJsonTo<int16_t>(const ejson::Value& _value) {
	return int16_t(_value.toNumber().get());
}
template<> int8_t convertJsonTo<int8_t>(const ejson::Value& _value) {
	return int8_t(_value.toNumber().get());
}
template<> uint64_t convertJsonTo<uint64_t>(const ejson::Value& _value) {
	return uint64_t(_value.toNumber().get());
}
template<> uint32_t convertJsonTo<uint32_t>(const ejson::Value& _value) {
	return uint32_t(_value.toNumber().get());
}
template<> uint16_t convertJsonTo<uint16_t>(const ejson::Value& _value) {
	return uint16_t(_value.toNumber().get());
}
template<> uint8_t convertJsonTo<uint8_t>(const ejson::Value& _value) {
	return uint8_t(_value.toNumber().get());
}
template<> std::string convertJsonTo<std::string>(const ejson::Value& _value) {
	//_value.display();
	return _value.toString().get();
}

template<> ejson::Value convertToJson<bool>(const bool& _value) {
	return ejson::Boolean(_value);
}
template<> ejson::Value convertToJson<float>(const float& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<double>(const double& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<int64_t>(const int64_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<int32_t>(const int32_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<int16_t>(const int16_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<int8_t>(const int8_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<uint64_t>(const uint64_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<uint32_t>(const uint32_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<uint16_t>(const uint16_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<uint8_t>(const uint8_t& _value) {
	return ejson::Number(_value);
}
template<> ejson::Value convertToJson<std::string>(const std::string& _value) {
	return ejson::String(_value);
}

template<> bool convertStringTo<bool>(const std::string& _value) {
	return etk::string_to_bool(_value);
}
template<> float convertStringTo<float>(const std::string& _value) {
	return etk::string_to_float(_value);
}
template<> double convertStringTo<double>(const std::string& _value) {
	return etk::string_to_double(_value);
}
template<> int64_t convertStringTo<int64_t>(const std::string& _value) {
	return etk::string_to_int64_t(_value);
}
template<> int32_t convertStringTo<int32_t>(const std::string& _value) {
	return etk::string_to_int32_t(_value);
}
template<> int16_t convertStringTo<int16_t>(const std::string& _value) {
	return etk::string_to_int16_t(_value);
}
template<> int8_t convertStringTo<int8_t>(const std::string& _value) {
	return etk::string_to_int8_t(_value);
}
template<> uint64_t convertStringTo<uint64_t>(const std::string& _value) {
	return etk::string_to_uint64_t(_value);
}
template<> uint32_t convertStringTo<uint32_t>(const std::string& _value) {
	return etk::string_to_uint32_t(_value);
}
template<> uint16_t convertStringTo<uint16_t>(const std::string& _value) {
	return etk::string_to_uint16_t(_value);
}
template<> uint8_t convertStringTo<uint8_t>(const std::string& _value) {
	return etk::string_to_uint8_t(_value);
}
template<> std::string convertStringTo<std::string>(const std::string& _value) {
	return _value;
}

bool AbstractFunction::checkCompatibility(const ParamType& _type, const ejson::Value& _params) {
	if (createType<bool>() == _type) {
		return _params.isBoolean();
	}
	if (    createType<int64_t>() == _type
	     || createType<int32_t>() == _type
	     || createType<int16_t>() == _type
	     || createType<int8_t>() == _type
	     || createType<uint64_t>() == _type
	     || createType<uint32_t>() == _type
	     || createType<uint16_t>() == _type
	     || createType<uint8_t>() == _type
	     || createType<float>() == _type
	     || createType<double>() == _type) {
		return _params.isNumber();
	}
	if (createType<std::string>() == _type) {
		return _params.isString();
	}
	return false;
}
bool AbstractFunction::checkCompatibility(const ParamType& _type, const std::string& _params) {
	return false;
}