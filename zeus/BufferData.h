/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>
#include <enet/WebSocket.h>
#include <zeus/ParamType.h>
#include <zeus/Buffer.h>

namespace zeus {

	class BufferData:
	  public Buffer {
		friend class zeus::Buffer;
		protected:
			uint32_t m_partId;
			uint16_t m_parameterId;
			std::vector<uint8_t> m_data;
		protected:
			/**
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferData::create
			 */
			BufferData():
			  m_partId(0) {
				m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::Buffer::typeMessage::data);
			};
			void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
			// TODO :... void appendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj) override;
			bool writeOn(enet::WebSocket& _interface) override;
			void generateDisplay(std::ostream& _os) const override;
		public:
			/**
			 * @brief Create a shared pointer on the BufferData
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::BufferData> create();
		public:
			enum zeus::Buffer::typeMessage getType() const override {
				return zeus::Buffer::typeMessage::data;
			}
			/**
			 * @brief Get the parameter Id of the buffer
			 * @return Part Identifier
			 */
			uint16_t getParameterId() const {
				return m_parameterId;
			}
			/**
			 * @brief Get the part Id of the buffer
			 * @return Part Identifier
			 */
			uint32_t getPartId() const;
			/**
			 * @brief Set the part Id of the buffer
			 * @param[in] _value New Part ID
			 */
			void setPartId(uint32_t _value);
			/**
			 * @brief add a raw data on the buffer
			 * @param[in] _parameterId Parameter id of the destination of the data
			 * @param[in] _data Pointer on the data
			 * @param[in] _size size of the data to add
			 */
			void addData(uint16_t _parameterId, void* _data, uint32_t _size);
			/**
			 * @brief Get data reference
			 */
			const std::vector<uint8_t>& getData() const {
				return m_data;
			}
			
	};
}
