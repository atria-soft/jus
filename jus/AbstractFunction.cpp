/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/AbstractFunction.h>
#include <jus/debug.h>
#include <etk/os/FSNode.h>
#include <ejson/base64.h>
namespace jus {
	template<> bool convertJsonTo<bool>(const ejson::Value& _value) {
		return _value.toBoolean().get();
	}
	template<> std::vector<bool> convertJsonTo<std::vector<bool>>(const ejson::Value& _value) {
		std::vector<bool> out;
		for (const auto it : _value.toArray()) {
			out.push_back(convertJsonTo<bool>(it));
		}
		return out;
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
		return _value.toString().get();
	}
	template<> std::vector<std::string> convertJsonTo<std::vector<std::string>>(const ejson::Value& _value) {
		std::vector<std::string> out;
		for (const auto it : _value.toArray()) {
			out.push_back(convertJsonTo<std::string>(it));
		}
		return out;
	}
	template<> jus::File convertJsonTo<jus::File>(const ejson::Value& _value) {
		ejson::Object obj = _value.toObject();
		jus::File out;
		out.setMineType(obj["mine-type"].toString().get());
		out.preSetDataSize(obj["size"].toNumber().getU64());
		//out.add("type", ejson::String("file"));
		
		uint64_t offset = 0;
		for (auto it : obj["data"].toArray()) {
			ejson::String valData = it.toString();
			if (valData.get().size() != 0) {
				std::vector<uint8_t> tmpData = ejson::base64::decode(valData.get());
				out.setData(offset, tmpData);
				offset += tmpData.size();
			}
		}
		return out;
	}
	
	// ----------------------------------------------------------------------------------------------------
	
	template<> ejson::Value convertToJson<bool>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const bool& _value) {
		return ejson::Boolean(_value);
	}
	template<> ejson::Value convertToJson<std::vector<bool>>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const std::vector<bool>& _value) {
		ejson::Array out;
		for (const auto &it : _value) {
			out.add(ejson::Boolean(it));
		}
		return out;
	}
	template<> ejson::Value convertToJson<float>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const float& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<double>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const double& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<int64_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const int64_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<int32_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const int32_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<int16_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const int16_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<int8_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const int8_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<uint64_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const uint64_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<uint32_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const uint32_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<uint16_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const uint16_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<uint8_t>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const uint8_t& _value) {
		return ejson::Number(_value);
	}
	template<> ejson::Value convertToJson<std::string>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const std::string& _value) {
		return ejson::String(_value);
	}
	template<> ejson::Value convertToJson<char*>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, char* const & _value) {
		if (_value == nullptr) {
			return ejson::String();
		}
		return ejson::String(_value);
	}
	template<> ejson::Value convertToJson<std::vector<std::string>>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const std::vector<std::string>& _value) {
		ejson::Array out;
		for (auto &it : _value) {
			out.add(ejson::String(it));
		}
		return out;
	}
	template<> ejson::Value convertToJson<jus::FileServer>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const jus::FileServer& _value) {
		ejson::Array out;
		/*
		for (auto &it : _value) {
			out.add(ejson::String(it));
		}
		*/
		return out;
	}
	
	static const int32_t BASE_SIZE_TRANSFER = 4096;
	class SenderJusFile {
		private:
			jus::File m_data;
			uint64_t m_size;
			uint64_t m_offset;
			int32_t m_paramID;
		public:
			SenderJusFile(jus::File _data, int32_t _paramID) :
			  m_data(_data),
			  m_size(m_data.getData().size()),
			  m_offset(0),
			  m_paramID(_paramID) {
				
			}
			~SenderJusFile() {
				
			}
			bool operator() (TcpString* _interface, const uint32_t& _serviceId, uint64_t _transactionId, uint64_t _part) {
				ejson::Object answer;
				if (_serviceId != 0) {
					answer.add("service", ejson::Number(_serviceId));
				}
				answer.add("id", ejson::Number(_transactionId));
				answer.add("part", ejson::Number(_part));
				if (m_paramID >= 0) {
					answer.add("param-id", ejson::Number(m_paramID));
				}
				int32_t tmpSize = BASE_SIZE_TRANSFER;
				if (m_size < BASE_SIZE_TRANSFER) {
					tmpSize = m_size;
				}
				uint8_t tmpData[BASE_SIZE_TRANSFER];
				answer.add("data", ejson::String(ejson::base64::encode(&m_data.getData()[m_offset], tmpSize)));
				m_offset += tmpSize;
				m_size -= tmpSize;
				_interface->writeJson(answer);
				if (m_size <= 0) {
					return true;
				}
				return false;
			}
	};
	template<> ejson::Value convertToJson<jus::File>(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, const jus::File& _value) {
		ejson::Object out;
		out.add("type", ejson::String("file"));
		out.add("mine-type", ejson::String(_value.getMineType()));
		out.add("size", ejson::Number(_value.getData().size()));
		if (_value.getData().size() != 0) {
			_asyncAction.push_back(SenderJusFile(_value, _paramId));
		}
		return out;
	}
	
	// ----------------------------------------------------------------------------------------------------
	
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
	template<> std::vector<std::string> convertStringTo<std::vector<std::string>>(const std::string& _value) {
		std::vector<std::string> out;
		JUS_TODO("Convert string to vs");
		return out;
	}
	template<> jus::FileServer convertStringTo<jus::FileServer>(const std::string& _value) {
		return jus::FileServer();
	}
	template<> jus::File convertStringTo<jus::File>(const std::string& _value) {
		return jus::File();
	}
	
}
// ============================================================
// == JSON
// ============================================================

ejson::Object jus::createCallJson(uint64_t _transactionId, const std::string& _functionName, ejson::Array _params) {
	ejson::Object callElem = createBaseCall(_transactionId, _functionName);
	callElem.add("param", _params);
	return callElem;
}

ejson::Object jus::createBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId) {
	ejson::Object obj;
	if (_serviceId != 0) {
		obj.add("service", ejson::Number(_serviceId));
	}
	obj.add("call", ejson::String(_functionName));
	obj.add("id", ejson::Number(_transactionId));
	return obj;
}
void jus::createParam(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, ejson::Object& _obj) {
	// Finish recursive parse ...
}


// ============================================================
// == Binary
// ============================================================
jus::Buffer jus::createBinaryCall(uint64_t _transactionId, const std::string& _functionName, const jus::Buffer& _params) {
	jus::Buffer callElem = createBinaryBaseCall(_transactionId, _functionName);
	//callElem.add("param", _params);
	return callElem;
}

jus::Buffer jus::createBinaryBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId) {
	jus::Buffer obj;
	obj.setServiceId(_serviceId);
	obj.setCall(_functionName);
	obj.setTransactionId(_transactionId);
	return obj;
}

void jus::createBinaryParam(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, jus::Buffer& _obj) {
	// Finish recursive parse ...
}

enum jus::AbstractFunction::type jus::AbstractFunction::getType() const {
	return m_type;
}

void jus::AbstractFunction::setType(enum jus::AbstractFunction::type _type) {
	m_type = _type;
}

const std::string& jus::AbstractFunction::getName() const {
	return m_name;
}

const std::string& jus::AbstractFunction::getDescription() const {
	return m_description;
}

void jus::AbstractFunction::setDescription(const std::string& _desc) {
	m_description = _desc;
}

void jus::AbstractFunction::setParam(int32_t _idParam, const std::string& _name, const std::string& _desc) {
	JUS_TODO("not implemented set param ... '" << _name << "'");
}

void jus::AbstractFunction::addParam(const std::string& _name, const std::string& _desc) {
	m_paramsDescription.push_back(std::make_pair(_name, _desc));
}

void jus::AbstractFunction::setReturn(const std::string& _desc) {
	m_returnDescription = _desc;
}

std::string jus::AbstractFunction::getPrototypeFull() const {
	std::string out = getPrototypeReturn();
	out += " ";
	out += m_name;
	out += "(";
	std::vector<std::string> tmp = getPrototypeParam();
	for (size_t iii=0; iii<tmp.size(); ++iii) {
		if (iii != 0) {
			out += ", ";
		}
		out += tmp[iii];
		if (iii < m_paramsDescription.size()) {
			out += " " + m_paramsDescription[iii].first;
		}
	}
	out += ");";
	return out;
}


jus::AbstractFunction::AbstractFunction(const std::string& _name,
                                        const std::string& _desc):
  m_type(jus::AbstractFunction::type::unknow),
  m_name(_name),
  m_description(_desc) {
	
}


bool jus::AbstractFunction::checkCompatibility(const ParamType& _type, const std::string& _params) {
	if (createType<bool>() == _type) {
		return _params == "bool";
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
		return    _params == "int8"
		       || _params == "int16"
		       || _params == "int32"
		       || _params == "int64"
		       || _params == "uint8"
		       || _params == "uint16"
		       || _params == "uint32"
		       || _params == "uint64"
		       || _params == "float"
		       || _params == "double";
	}
	if (createType<std::vector<std::string>>() == _type) {
		return _params == "vector:string";
	}
	if (    createType<std::vector<bool>>() == _type
	     || createType<std::vector<int64_t>>() == _type
	     || createType<std::vector<int32_t>>() == _type
	     || createType<std::vector<int16_t>>() == _type
	     || createType<std::vector<int8_t>>() == _type
	     || createType<std::vector<uint64_t>>() == _type
	     || createType<std::vector<uint32_t>>() == _type
	     || createType<std::vector<uint16_t>>() == _type
	     || createType<std::vector<uint8_t>>() == _type
	     || createType<std::vector<float>>() == _type
	     || createType<std::vector<double>>() == _type) {
		return    _params == "vector:int8"
		       || _params == "vector:int16"
		       || _params == "vector:int32"
		       || _params == "vector:int64"
		       || _params == "vector:uint8"
		       || _params == "vector:uint16"
		       || _params == "vector:uint32"
		       || _params == "vector:uint64"
		       || _params == "vector:float"
		       || _params == "vector:double"
		       || _params == "vector:empty";
	}
	if (createType<jus::File>() == _type) {
		/*
		if (_params.isObject()) {
			if (_params.toObject()["type"].toString().get() == "file") {
				return true;
			}
		}
		*/
		return false;
	}
	if (createType<std::string>() == _type) {
		return _params == "string";
	}
	return false;
}
