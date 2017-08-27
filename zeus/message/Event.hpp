/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/message/ParamType.hpp>
#include <zeus/message/Parameter.hpp>

namespace zeus {
	class WebServer;
	namespace message {
		class Event :
		  public message::Parameter {
			friend class zeus::Message;
			protected:
				uint64_t m_uid; //!< Signal is sended in an active call ==> then we need to identify all the type ==> need an uniqueId ... the bigger bit mean that the message has no more data ...
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::message::Event::create
				 */
				Event(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::message::Parameter(_iface) {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::event);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				bool writeOn(enet::WebSocket& _interface) override;
				void generateDisplay(etk::Stream& _os) const override;
			public:
				/**
				 * @brief Create a shared pointer on the Event
				 * @return Allocated Message.
				 */
				static ememory::SharedPtr<zeus::message::Event> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::event;
				}
				/**
				 * @brief set the Event of the call
				 * @param[in] _value Value to add
				 */
				template<class ZEUS_TYPE_DATA>
				void addEvent(const ZEUS_TYPE_DATA& _value) {
					addParameter(_value);
				}
				/**
				 * @brief get the Event value
				 * @param[in] Data of the Event
				 */
				template<class ZEUS_TYPE_DATA>
				ZEUS_TYPE_DATA getEvent() const {
					return getParameter<ZEUS_TYPE_DATA>(0);
				}
		};
	}
}
