/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <etk/stdTools.hpp>
#include <zeus/message/type.hpp>

namespace etk {
	template<> etk::String toString<enum zeus::message::type>(const enum zeus::message::type& _value) {
		switch (_value) {
			case zeus::message::type::unknow:
				return "unknow";
			case zeus::message::type::call:
				return "call";
			case zeus::message::type::answer:
				return "answer";
			case zeus::message::type::event:
				return "event";
			case zeus::message::type::data:
				return "data";
		}
		return "???";
	}
}
etk::Stream& zeus::message::operator <<(etk::Stream& _os, enum zeus::message::type _value) {
	_os << etk::toString(_value);
	return _os;
}

enum zeus::message::type zeus::message::getTypeFromInt(uint16_t _value) {
	switch (_value) {
		case 0:
			return zeus::message::type::unknow;
		case 1:
			return zeus::message::type::call;
		case 2:
			return zeus::message::type::answer;
		case 3:
			return zeus::message::type::data;
		case 4:
			return zeus::message::type::event;
	}
	return zeus::message::type::unknow;
}
