/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/ParamType.hpp>
#include <zeus/Buffer.hpp>

namespace zeus {
	class BufferCtrl :
	  public Buffer {
		friend class zeus::Buffer;
		protected:
			std::string m_ctrlValue;
		protected:
			/**
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferCall::create
			 */
			BufferCtrl(ememory::SharedPtr<zeus::WebServer> _iface):
			  zeus::Buffer(_iface) {
				m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::Buffer::typeMessage::ctrl);
			};
			void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
			void appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) override;
			bool writeOn(enet::WebSocket& _interface) override;
			void generateDisplay(std::ostream& _os) const override;
		public:
			/**
			 * @brief Create a shared pointer on the BufferCall
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::BufferCtrl> create(ememory::SharedPtr<zeus::WebServer> _iface);
		public:
			enum zeus::Buffer::typeMessage getType() const override {
				return zeus::Buffer::typeMessage::ctrl;
			}
			/**
			 * @brief get the call value of the buffer
			 * @return string of the function to call
			 */
			const std::string& getCtrl() const;
			/**
			 * @brief Set the call value of the buffer
			 * @param[in] _value Function to call
			 */
			void setCtrl(const std::string& _value);
			
	};
}

