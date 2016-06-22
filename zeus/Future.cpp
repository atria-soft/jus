/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/Future.h>
#include <zeus/debug.h>
#include <unistd.h>
#include <zeus/File.h>

namespace zeus {
	template<>
	bool zeus::Future<bool>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return false;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return false;
		}
		return m_data->m_returnData->getAnswer<bool>();
	}
	template<>
	int64_t zeus::Future<int64_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<bool>();
	}
	template<>
	int32_t zeus::Future<int32_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<int32_t>();
	}
	template<>
	int16_t zeus::Future<int16_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<int16_t>();
	}
	template<>
	int8_t zeus::Future<int8_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<int8_t>();
	}
	template<>
	uint64_t zeus::Future<uint64_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<int64_t>();
	}
	template<>
	uint32_t zeus::Future<uint32_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<uint32_t>();
	}
	template<>
	uint16_t zeus::Future<uint16_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<uint16_t>();
	}
	template<>
	uint8_t zeus::Future<uint8_t>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return m_data->m_returnData->getAnswer<uint8_t>();
	}
	template<>
	double zeus::Future<double>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0.0;
		}
		return m_data->m_returnData->getAnswer<double>();
	}
	template<>
	float zeus::Future<float>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return 0.0f;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0.0f;
		}
		return m_data->m_returnData->getAnswer<float>();
	}
	template<>
	std::string zeus::Future<std::string>::get() {
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return "";
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return "";
		}
		return m_data->m_returnData->getAnswer<std::string>();
	}
	template<>
	std::vector<int64_t> zeus::Future<std::vector<int64_t>>::get() {
		std::vector<int64_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<int64_t>>();
		return out;
	}
	template<>
	std::vector<int32_t> zeus::Future<std::vector<int32_t>>::get() {
		std::vector<int32_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<int32_t>>();
		return out;
	}
	template<>
	std::vector<int16_t> zeus::Future<std::vector<int16_t>>::get() {
		std::vector<int16_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<int16_t>>();
		return out;
	}
	template<>
	std::vector<int8_t> zeus::Future<std::vector<int8_t>>::get() {
		std::vector<int8_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<int8_t>>();
		return out;
	}
	template<>
	std::vector<uint64_t> zeus::Future<std::vector<uint64_t>>::get() {
		std::vector<uint64_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<uint64_t>>();
		return out;
	}
	template<>
	std::vector<uint32_t> zeus::Future<std::vector<uint32_t>>::get() {
		std::vector<uint32_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<uint32_t>>();
		return out;
	}
	template<>
	std::vector<uint16_t> zeus::Future<std::vector<uint16_t>>::get() {
		std::vector<uint16_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<uint16_t>>();
		return out;
	}
	template<>
	std::vector<uint8_t> zeus::Future<std::vector<uint8_t>>::get() {
		std::vector<uint8_t> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<uint8_t>>();
		return out;
	}
	template<>
	std::vector<double> zeus::Future<std::vector<double>>::get() {
		std::vector<double> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<double>>();
		return out;
	}
	template<>
	std::vector<float> zeus::Future<std::vector<float>>::get() {
		std::vector<float> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<float>>();
		return out;
	}
	template<>
	std::vector<std::string> zeus::Future<std::vector<std::string>>::get() {
		std::vector<std::string> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<std::string>>();
		return out;
	}
	template<>
	std::vector<bool> zeus::Future<std::vector<bool>>::get() {
		std::vector<bool> out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<std::vector<bool>>();
		return out;
	}
	template<>
	zeus::File zeus::Future<zeus::File>::get() {
		zeus::File out;
		if (    m_data == nullptr
		     || m_data->m_returnData == nullptr) {
			return out;
		}
		if (m_data->m_returnData->getType() != zeus::Buffer::typeMessage::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = m_data->m_returnData->getAnswer<zeus::File>();
		return out;
	}
}


