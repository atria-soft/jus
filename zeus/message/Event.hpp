/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <zeus/message/Call.hpp>

namespace zeus {
	namespace message {
		class Event :
		  public message::Call {
			friend class zeus::Message;
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::message::Call::create
				 */
				Event(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::message::Call(_iface) {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::event);
				};
			public:
				/**
				 * @brief Create a shared pointer on the MessageCall
				 * @return Allocated Message.
				 */
				static ememory::SharedPtr<zeus::message::Event> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::event;
				}
		};
	}
}

