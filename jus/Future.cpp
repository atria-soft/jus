/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/Future.h>
#include <jus/debug.h>
#include <unistd.h>

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
	std::vector<int32_t> jus::Future<std::vector<int32_t>>::get() {
		std::vector<int32_t> out;
		if (m_data == nullptr) {
			return out;
		}
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return std::vector<int32_t>();
		}
		if (val.isArray() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Array'");
			return std::vector<int32_t>();
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
}


