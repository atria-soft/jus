/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/message/ParamType.hpp>

namespace zeus {
	namespace message {
		/**
		 * @brief Type of the massage send or receive
		 */
		enum class type {
			unknow = 0x0000, //!< Init value
			call = 0x0001, //!< Remote call on a service ID
			answer = 0x0002, //!< Answer from a previous call
			data = 0x0003, //!< data message happend when partId > 0 it compleate the data of a parameter or an answer or an event
			event = 0x0004, //!< Message in one way (no return is waiting and the message has no garenty...)
		};
		/**
		 * @brief generate a display of the typemessage
		 * @param[in] _os stream to add data
		 * @value[in] _value Value to add
		 * @return a reference of the stream
		 */
		std::ostream& operator <<(std::ostream& _os, enum zeus::message::type _value);
		
		enum zeus::message::type getTypeFromInt(uint16_t _value);
	}
}


