/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/message/ParamType.hpp>

namespace zeus {
	namespace message {
		/*
		class MessageEvent :
		  public message::Parameter {
			friend class zeus::Message;
			protected:
				/ **
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::MessageEvent::create
				 * /
				MessageEvent() {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::event);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				void appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) override;
			public:
				/ **
				 * @brief Create a shared pointer on the MessageEvent
				 * @return Allocated Message.
				 * /
				static ememory::SharedPtr<zeus::MessageEvent> create();
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::event;
				}
				
		};
		*/
	}
}

