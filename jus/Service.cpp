/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <jus/Service.h>
#include <jus/debug.h>

#include <unistd.h>

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
	_value.display();
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
	_value.display();
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





jus::Service::Service() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &jus::Service::onPropertyChangeIp),
  propertyPort(this, "port", 1984, "Port to connect server", &jus::Service::onPropertyChangePort),
  m_id(0) {
	m_interfaceClient.propertyIp.set(*propertyIp);
	m_interfaceClient.propertyPort.set(*propertyPort);
	m_interfaceClient.propertyServer.set(false);
	m_dataCallback = m_interfaceClient.signalData.connect(this, &jus::Service::onClientData);
}

jus::Service::~Service() {
	
}

void jus::Service::onClientData(const std::string& _value) {
	ejson::Object request(_value);
	JUS_INFO("Request : " << _value);
	m_interfaceClient.write("{ \"error\": \"NOT-IMPLEMENTED\"}");
}

void jus::Service::onPropertyChangeIp() {
	m_interfaceClient.propertyIp.set(*propertyIp);
}

void jus::Service::onPropertyChangePort(){
	m_interfaceClient.propertyPort.set(*propertyPort);
}


void jus::Service::connect(){
	JUS_DEBUG("connect [START]");
	m_interfaceClient.connect();
	JUS_DEBUG("connect [STOP]");
}

void jus::Service::disconnect(){
	JUS_DEBUG("disconnect [START]");
	m_interfaceClient.disconnect();
	JUS_DEBUG("disconnect [STOP]");
}

ejson::Object jus::Service::callJson(const ejson::Object& _obj) {
	/*
	JUS_VERBOSE("Call JSON [START] ");
	std::string tmpVal = _obj.generate();
	JUS_DEBUG("Call JSON '" << tmpVal << "'");
	m_interfaceClient.write(_obj.generate());
	std::string ret = asyncRead();
	JUS_VERBOSE("Call JSON [STOP]");
	*/
	return ejson::Object();
}
