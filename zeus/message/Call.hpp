/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/message/ParamType.hpp>
#include <zeus/message/Parameter.hpp>

namespace zeus {
	namespace message {
		class Call :
		  public message::Parameter {
			friend class zeus::Message;
			protected:
				std::string m_callName;
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::message::Call::create
				 */
				Call(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::message::Parameter(_iface) {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::call);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				void appendMessageData(ememory::SharedPtr<zeus::message::Data> _obj) override;
				bool writeOn(enet::WebSocket& _interface) override;
				void generateDisplay(std::ostream& _os) const override;
			public:
				/**
				 * @brief Create a shared pointer on the MessageCall
				 * @return Allocated Message.
				 */
				static ememory::SharedPtr<zeus::message::Call> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::call;
				}
				/**
				 * @brief get the call value of the buffer
				 * @return string of the function to call
				 */
				const std::string& getCall() const;
				/**
				 * @brief Set the call value of the buffer
				 * @param[in] _value Function to call
				 */
				void setCall(const std::string& _value);
				
		};
	}
}
