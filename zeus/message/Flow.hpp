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
		class BufferFlow:
		  public Buffer {
			friend class zeus::Buffer;
			protected:
				/ **
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferFlow::create
				 * /
				BufferFlow() {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::flow);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
			public:
				/ **
				 * @brief Create a shared pointer on the BufferFlow
				 * @return Allocated Buffer.
				 * /
				static ememory::SharedPtr<zeus::BufferFlow> create();
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::flow;
				}
				
		};
		*/
	}
}
