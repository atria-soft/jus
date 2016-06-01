/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/Future.h>
#include <jus/debug.h>
#include <unistd.h>
#include <jus/File.h>

namespace jus {
	template<>
	bool jus::Future<bool>::get() {
		if (m_data == nullptr) {
			return false;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return false;
		}
		if (val.isBoolean() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Boolean'");
			return false;
		}
		return val.toBoolean().get();
	}
	template<>
	int64_t jus::Future<int64_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return int64_t(val.toNumber().get());
	}
	template<>
	int32_t jus::Future<int32_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return int32_t(val.toNumber().get());
	}
	template<>
	int16_t jus::Future<int16_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return int16_t(val.toNumber().get());
	}
	template<>
	int8_t jus::Future<int8_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return int8_t(val.toNumber().get());
	}
	template<>
	uint64_t jus::Future<uint64_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return uint64_t(val.toNumber().get());
	}
	template<>
	uint32_t jus::Future<uint32_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return uint32_t(val.toNumber().get());
	}
	template<>
	uint16_t jus::Future<uint16_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return uint16_t(val.toNumber().get());
	}
	template<>
	uint8_t jus::Future<uint8_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0;
		}
		return uint8_t(val.toNumber().get());
	}
	template<>
	double jus::Future<double>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0.0;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0.0;
		}
		return val.toNumber().get();
	}
	template<>
	float jus::Future<float>::get() {
		if (m_data == nullptr) {
			return 0.0f;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return 0.0f;
		}
		if (val.isNumber() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Number'");
			return 0.0f;
		}
		return val.toNumber().get();
	}
	template<>
	std::string jus::Future<std::string>::get() {
		if (m_data == nullptr) {
			return "";
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return "";
		}
		if (val.isString() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'String'");
			return "";
		}
		return val.toString().get();
	}
	template<>
	std::vector<int64_t> jus::Future<std::vector<int64_t>>::get() {
		std::vector<int64_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(int64_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<int32_t> jus::Future<std::vector<int32_t>>::get() {
		std::vector<int32_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(int32_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<int16_t> jus::Future<std::vector<int16_t>>::get() {
		std::vector<int16_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(int16_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<int8_t> jus::Future<std::vector<int8_t>>::get() {
		std::vector<int8_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(int8_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<uint64_t> jus::Future<std::vector<uint64_t>>::get() {
		std::vector<uint64_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(uint64_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<uint32_t> jus::Future<std::vector<uint32_t>>::get() {
		std::vector<uint32_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(uint32_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<uint16_t> jus::Future<std::vector<uint16_t>>::get() {
		std::vector<uint16_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(int16_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<uint8_t> jus::Future<std::vector<uint8_t>>::get() {
		std::vector<uint8_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (val.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(uint8_t(val.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<double> jus::Future<std::vector<double>>::get() {
		std::vector<double> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (it.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(it.toNumber().get());
		}
		return out;
	}
	template<>
	std::vector<float> jus::Future<std::vector<float>>::get() {
		std::vector<float> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (it.isNumber() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Number'");
				continue;
			}
			out.push_back(float(it.toNumber().get()));
		}
		return out;
	}
	template<>
	std::vector<std::string> jus::Future<std::vector<std::string>>::get() {
		std::vector<std::string> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (it.isString() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'String'");
				continue;
			}
			out.push_back(it.toString().get());
		}
		return out;
	}
	template<>
	std::vector<bool> jus::Future<std::vector<bool>>::get() {
		std::vector<bool> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (it.isBoolean() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Boolean'");
				continue;
			}
			out.push_back(it.toBoolean().get());
		}
		return out;
	}
	template<>
	jus::File jus::Future<jus::File>::get() {
		jus::File out;
		if (m_data == nullptr) {
			return out;
		}
		// TODO :...
		/*
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return out;
		}
		for (auto it : val.toArray()) {
			if (it.isBoolean() == false) {
				JUS_WARNING("Wrong return Type (part of array) get '" << it.getType() << " instead of 'Boolean'");
				continue;
			}
			out.push_back(it.toBoolean().get());
		}
		*/
		return out;
	}
}


