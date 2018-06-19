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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return 0.0f;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return 0.0f;
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<float>();
	}
	template<>
	etk::String futureGetValue<etk::String>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return "";
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return "";
		}
		return static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::String>();
	}
	template<>
	etk::Vector<int64_t> futureGetValue<etk::Vector<int64_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<int64_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<int64_t>>();
		return out;
	}
	template<>
	etk::Vector<int32_t> futureGetValue<etk::Vector<int32_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<int32_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<int32_t>>();
		return out;
	}
	template<>
	etk::Vector<int16_t> futureGetValue<etk::Vector<int16_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<int16_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<int16_t>>();
		return out;
	}
	template<>
	etk::Vector<int8_t> futureGetValue<etk::Vector<int8_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<int8_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<int8_t>>();
		return out;
	}
	template<>
	etk::Vector<uint64_t> futureGetValue<etk::Vector<uint64_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<uint64_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<uint64_t>>();
		return out;
	}
	template<>
	etk::Vector<uint32_t> futureGetValue<etk::Vector<uint32_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<uint32_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<uint32_t>>();
		return out;
	}
	template<>
	etk::Vector<uint16_t> futureGetValue<etk::Vector<uint16_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<uint16_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<uint16_t>>();
		return out;
	}
	template<>
	etk::Vector<uint8_t> futureGetValue<etk::Vector<uint8_t>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<uint8_t> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<uint8_t>>();
		return out;
	}
	template<>
	etk::Vector<double> futureGetValue<etk::Vector<double>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<double> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<double>>();
		return out;
	}
	template<>
	etk::Vector<float> futureGetValue<etk::Vector<float>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<float> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<float>>();
		return out;
	}
	template<>
	etk::Vector<etk::String> futureGetValue<etk::Vector<etk::String>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<etk::String> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<etk::String>>();
		return out;
	}
	template<>
	etk::Vector<bool> futureGetValue<etk::Vector<bool>>(ememory::SharedPtr<zeus::Promise>& _promise) {
		etk::Vector<bool> out;
		if (    _promise == null
		     || _promise->getRaw() == null) {
			return out;
		}
		if (_promise->getRaw()->getType() != zeus::message::type::answer) {
			ZEUS_WARNING("No Return value ...");
			return out;
		}
		out = static_cast<zeus::message::Answer*>(_promise->getRaw().get())->getAnswer<etk::Vector<bool>>();
		return out;
	}
	template<>
	zeus::Raw futureGetValue<zeus::Raw>(ememory::SharedPtr<zeus::Promise>& _promise) {
		if (    _promise == null
		     || _promise->getRaw() == null) {
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
		if (    _promise == null
		     || _promise->getRaw() == null) {
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


