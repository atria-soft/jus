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
		class BufferEvent :
		  public BufferParameter {
			friend class zeus::Buffer;
			protected:
				/ **
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferEvent::create
				 * /
				BufferEvent() {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::event);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				void appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) override;
			public:
				/ **
				 * @brief Create a shared pointer on the BufferEvent
				 * @return Allocated Buffer.
				 * /
				static ememory::SharedPtr<zeus::BufferEvent> create();
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::event;
				}
				
		};
		*/
	}
}

