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
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return false;
		}
		return m_data->m_returnData.getAnswer<bool>();
	}
	template<>
	int64_t jus::Future<int64_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<bool>();
	}
	template<>
	int32_t jus::Future<int32_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<int32_t>();
	}
	template<>
	int16_t jus::Future<int16_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<int16_t>();
	}
	template<>
	int8_t jus::Future<int8_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<int8_t>();
	}
	template<>
	uint64_t jus::Future<uint64_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<int64_t>();
	}
	template<>
	uint32_t jus::Future<uint32_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<uint32_t>();
	}
	template<>
	uint16_t jus::Future<uint16_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<uint16_t>();
	}
	template<>
	uint8_t jus::Future<uint8_t>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData.getAnswer<uint8_t>();
	}
	template<>
	double jus::Future<double>::get() {
		if (m_data == nullptr) {
			return 0;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0.0;
		}
		return m_data->m_returnData.getAnswer<double>();
	}
	template<>
	float jus::Future<float>::get() {
		if (m_data == nullptr) {
			return 0.0f;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return 0.0f;
		}
		return m_data->m_returnData.getAnswer<float>();
	}
	template<>
	std::string jus::Future<std::string>::get() {
		if (m_data == nullptr) {
			return "";
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return "";
		}
		return m_data->m_returnData.getAnswer<std::string>();
	}
	template<>
	std::vector<int64_t> jus::Future<std::vector<int64_t>>::get() {
		std::vector<int64_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<int64_t>>();
		return out;
	}
	template<>
	std::vector<int32_t> jus::Future<std::vector<int32_t>>::get() {
		std::vector<int32_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<int32_t>>();
		return out;
	}
	template<>
	std::vector<int16_t> jus::Future<std::vector<int16_t>>::get() {
		std::vector<int16_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<int16_t>>();
		return out;
	}
	template<>
	std::vector<int8_t> jus::Future<std::vector<int8_t>>::get() {
		std::vector<int8_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<int8_t>>();
		return out;
	}
	template<>
	std::vector<uint64_t> jus::Future<std::vector<uint64_t>>::get() {
		std::vector<uint64_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<uint64_t>>();
		return out;
	}
	template<>
	std::vector<uint32_t> jus::Future<std::vector<uint32_t>>::get() {
		std::vector<uint32_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<uint32_t>>();
		return out;
	}
	template<>
	std::vector<uint16_t> jus::Future<std::vector<uint16_t>>::get() {
		std::vector<uint16_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<uint16_t>>();
		return out;
	}
	template<>
	std::vector<uint8_t> jus::Future<std::vector<uint8_t>>::get() {
		std::vector<uint8_t> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<uint8_t>>();
		return out;
	}
	template<>
	std::vector<double> jus::Future<std::vector<double>>::get() {
		std::vector<double> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<double>>();
		return out;
	}
	template<>
	std::vector<float> jus::Future<std::vector<float>>::get() {
		std::vector<float> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<float>>();
		return out;
	}
	template<>
	std::vector<std::string> jus::Future<std::vector<std::string>>::get() {
		std::vector<std::string> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<std::string>>();
		return out;
	}
	template<>
	std::vector<bool> jus::Future<std::vector<bool>>::get() {
		std::vector<bool> out;
		if (m_data == nullptr) {
			return out;
		}
		if (m_data->m_returnData.getType() != jus::Buffer::typeMessage::answer) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData.getAnswer<std::vector<bool>>();
		return out;
	}
	template<>
	jus::File jus::Future<jus::File>::get() {
		jus::File out;
		if (m_data == nullptr) {
			return out;
		}
		/* TODO : ...
		ejson::Value val = m_data->m_returnData["return"];
		if (val.exist() == false) {
			JUS_WARNING("No Return value ...");
			return out;
		}
		if (val.isObject() == false) {
			JUS_WARNING("Wrong return Type get '" << val.getType() << " instead of 'Object'");
			return out;
		}
		ejson::Object retVal = val.toObject();
		if (retVal["type"].toString().get() != "file") {
			JUS_WARNING("Wrong return object-type get '" << retVal["type"].toString().get() << " instead of 'file'");
			return out;
		}
		out.setMineType(retVal["mine-type"].toString().get());
		out.preSetDataSize(retVal["size"].toNumber().getU64());
		// no data might be stored in the first packet ...
		
		uint64_t offset = 0;
		// TODO: check part ID
		for (auto &it : m_data->m_returnDataPart) {
			ejson::String valData = it.toString();
			std::vector<uint8_t> tmpData = ejson::base64::decode(valData.get());
			out.setData(offset, tmpData);
			offset += tmpData.size();
		}
		*/
		return out;
	}
}


