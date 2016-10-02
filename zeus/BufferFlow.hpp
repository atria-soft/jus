/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/ParamType.hpp>

namespace zeus {

	/*
	class BufferFlow:
	  public Buffer {
		friend class zeus::Buffer;
		protected:
			/ **
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferFlow::create
			 * /
			BufferFlow() {
				m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::Buffer::typeMessage::flow);
			};
			void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
		public:
			/ **
			 * @brief Create a shared pointer on the BufferFlow
			 * @return Allocated Buffer.
			 * /
			static ememory::SharedPtr<zeus::BufferFlow> create();
		public:
			enum zeus::Buffer::typeMessage getType() const override {
				return zeus::Buffer::typeMessage::flow;
			}
			
	};
	*/
}
