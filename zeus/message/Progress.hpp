/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/message/Message.hpp>

namespace zeus {
	class WebServer;
	namespace message {
		class Progress :
		  public zeus::Message {
			friend class zeus::Message;
			protected:
				std::string m_data;
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::message::Answer::create
				 */
				Progress(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::Message(_iface) {
					m_header.flags = uint8_t(zeus::message::type::progress);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				void appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) override;
				bool writeOn(enet::WebSocket& _interface) override;
				void generateDisplay(std::ostream& _os) const override;
			public:
				/**
				 * @brief Create a shared pointer on the Answer
				 * @return Allocated Message.
				 */
				static ememory::SharedPtr<zeus::message::Progress> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::progress;
				}
				/**
				 * @brief progress message answer
				 * @param[in] _data Data of the progress
				 */
				void setData(const std::string& _data);
				/**
				 * @brief get the error value (if exist)
				 * @return string of the error
				 */
				const std::string& getData() const;
		};
	}
}
