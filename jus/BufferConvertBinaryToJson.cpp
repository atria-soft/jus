/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <etk/types.h>
#include <jus/Buffer.h>
#include <jus/debug.h>
#include <jus/ParamType.h>




namespace jus {
	// ----------------------------------------------------------------------------------------------------
	
	template<> ejson::Value convertBinaryToJson<bool>(const uint8_t* _data, uint32_t _size) {
		const bool* value = reinterpret_cast<const bool*>(_data);
		return ejson::Boolean(*value);
	}
	template<> ejson::Value convertBinaryToJson<std::vector<bool>>(const uint8_t* _data, uint32_t _size) {
		const bool* value = reinterpret_cast<const bool*>(_data);
		ejson::Array out;
		for (size_t iii=0; iii<_size; ++iii) {
			out.add(ejson::Boolean(value[iii]));
		}
		return out;
	}
	template<> ejson::Value convertBinaryToJson<float>(const uint8_t* _data, uint32_t _size) {
		const float* value = reinterpret_cast<const float*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<double>(const uint8_t* _data, uint32_t _size) {
		const double* value = reinterpret_cast<const double*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<int64_t>(const uint8_t* _data, uint32_t _size) {
		const int64_t* value = reinterpret_cast<const int64_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<int32_t>(const uint8_t* _data, uint32_t _size) {
		const int32_t* value = reinterpret_cast<const int32_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<int16_t>(const uint8_t* _data, uint32_t _size) {
		const int16_t* value = reinterpret_cast<const int16_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<int8_t>(const uint8_t* _data, uint32_t _size) {
		const int8_t* value = reinterpret_cast<const int8_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<uint64_t>(const uint8_t* _data, uint32_t _size) {
		const uint64_t* value = reinterpret_cast<const uint64_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<uint32_t>(const uint8_t* _data, uint32_t _size) {
		const uint32_t* value = reinterpret_cast<const uint32_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<uint16_t>(const uint8_t* _data, uint32_t _size) {
		const uint16_t* value = reinterpret_cast<const uint16_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<uint8_t>(const uint8_t* _data, uint32_t _size) {
		const uint8_t* value = reinterpret_cast<const uint8_t*>(_data);
		return ejson::Number(*value);
	}
	template<> ejson::Value convertBinaryToJson<std::string>(const uint8_t* _data, uint32_t _size) {
		std::string val;
		val.resize(_size, '\0');
		memcpy(&val[0], _data, _size);
		return ejson::String(val);
	}
	template<> ejson::Value convertBinaryToJson<std::vector<std::string>>(const uint8_t* _data, uint32_t _size) {
		/*
		 Little hard ... TODO : ...
		const bool* value = reinterpret_cast<const bool*>(_data);
		ejson::Array out;
		for (size_t iii=0; iii<_size; ++iii) {
			out.add(ejson::Boolean(value[iii]));
		}
		return out;
		*/
		ejson::Array out;
		return out;
	}
	/*
	template<> ejson::Value convertBinaryToJson<jus::File>(const uint8_t* _data, uint32_t _size) {
		ejson::Object out;
		out.add("type", ejson::String("file"));
		out.add("mine-type", ejson::String(_value.getMineType()));
		out.add("size", ejson::Number(_value.getData().size()));
		if (_value.getData().size() != 0) {
			_asyncAction.push_back(SenderJusFile(_value, _paramId));
		}
		return out;
	}
	*/
	
	// ----------------------------------------------------------------------------------------------------
	
}



