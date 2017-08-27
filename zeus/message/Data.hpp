/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <zeus/WebServer.hpp>
#include <zeus/message/ParamType.hpp>
#include <zeus/message/Message.hpp>

namespace zeus {
	class WebServer;
	namespace message {
		class Data: public zeus::Message {
			friend class zeus::Message;
			protected:
				uint32_t m_partId;
				uint16_t m_parameterId;
				etk::Vector<uint8_t> m_data;
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::message::Data::create
				 */
				Data(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::Message(_iface),
				  m_partId(0) {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::data);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				// TODO :... void appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) override;
				bool writeOn(enet::WebSocket& _interface) override;
				void generateDisplay(etk::Stream& _os) const override;
			public:
				/**
				 * @brief Create a shared pointer on the MessageData
				 * @return Allocated Message.
				 */
				static ememory::SharedPtr<zeus::message::Data> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::data;
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
				const etk::Vector<uint8_t>& getData() const {
					return m_data;
				}
				
		};
	}
}
