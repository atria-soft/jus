/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <zeus/Future.hpp>
#include <zeus/message/Answer.hpp>
#include <zeus/debug.hpp>

#include <zeus/Raw.hpp>
#include <zeus/ObjectRemote.hpp>

namespace zeus {
	template<>
	bool futureGetValue<bool>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return false;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return false;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<bool>();
	}
	template<>
	int64_t futureGetValue<int64_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<int64_t>();
	}
	template<>
	int32_t futureGetValue<int32_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<int32_t>();
	}
	template<>
	int16_t futureGetValue<int16_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<int16_t>();
	}
	template<>
	int8_t futureGetValue<int8_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<int8_t>();
	}
	template<>
	uint64_t futureGetValue<uint64_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<uint64_t>();
	}
	template<>
	uint32_t futureGetValue<uint32_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<uint32_t>();
	}
	template<>
	uint16_t futureGetValue<uint16_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<uint16_t>();
	}
	template<>
	uint8_t futureGetValue<uint8_t>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<uint8_t>();
	}
	template<>
	double futureGetValue<double>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0.0;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<double>();
	}
	template<>
	float futureGetValue<float>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return 0.0f;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0.0f;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<float>();
	}
	template<>
	std::string futureGetValue<std::string>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return "";
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return "";
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::string>();
	}
	template<>
	std::vector<int64_t> futureGetValue<std::vector<int64_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<int64_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<int64_t>>();
		return out;
	}
	template<>
	std::vector<int32_t> futureGetValue<std::vector<int32_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<int32_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<int32_t>>();
		return out;
	}
	template<>
	std::vector<int16_t> futureGetValue<std::vector<int16_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<int16_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<int16_t>>();
		return out;
	}
	template<>
	std::vector<int8_t> futureGetValue<std::vector<int8_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<int8_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<int8_t>>();
		return out;
	}
	template<>
	std::vector<uint64_t> futureGetValue<std::vector<uint64_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<uint64_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<uint64_t>>();
		return out;
	}
	template<>
	std::vector<uint32_t> futureGetValue<std::vector<uint32_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<uint32_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<uint32_t>>();
		return out;
	}
	template<>
	std::vector<uint16_t> futureGetValue<std::vector<uint16_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<uint16_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<uint16_t>>();
		return out;
	}
	template<>
	std::vector<uint8_t> futureGetValue<std::vector<uint8_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<uint8_t> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<uint8_t>>();
		return out;
	}
	template<>
	std::vector<double> futureGetValue<std::vector<double>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<double> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<double>>();
		return out;
	}
	template<>
	std::vector<float> futureGetValue<std::vector<float>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<float> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<float>>();
		return out;
	}
	template<>
	std::vector<std::string> futureGetValue<std::vector<std::string>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<std::string> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<std::string>>();
		return out;
	}
	template<>
	std::vector<bool> futureGetValue<std::vector<bool>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		std::vector<bool> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<std::vector<bool>>();
		return out;
	}
	template<>
	zeus::Raw futureGetValue<zeus::Raw>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return zeus::Raw();
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return zeus::Raw();
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<zeus::Raw>();
	}
	template<>
	ememory::SharedPtr<zeus::ObjectRemoteBase> futureGetValue<ememory::SharedPtr<zeus::ObjectRemoteBase>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		ememory::SharedPtr<zeus::ObjectRemoteBase> out;
		if (    _promise == nullptr
		     || _promise->getRaw() == nullptr) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<ememory::SharedPtr<zeus::ObjectRemoteBase>>();
		return out;
	}
}


