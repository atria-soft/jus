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
	class WebServer;
	namespace message {
		class Answer :
		  public message::Parameter {
			friend class zeus::message::Message;
			protected:
				std::string m_errorType;
				std::string m_errorHelp;
			protected:
				/**
				 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::Answer::create
				 */
				Answer(ememory::SharedPtr<zeus::WebServer> _iface):
				  zeus::message::Parameter(_iface) {
					m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::message::type::answer);
				};
				void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
				void appendBufferData(ememory::SharedPtr<zeus::message::Data> _obj) override;
				bool writeOn(enet::WebSocket& _interface) override;
				void generateDisplay(std::ostream& _os) const override;
			public:
				/**
				 * @brief Create a shared pointer on the Answer
				 * @return Allocated Buffer.
				 */
				static ememory::SharedPtr<zeus::Answer> create(ememory::SharedPtr<zeus::WebServer> _iface);
			public:
				enum zeus::message::type getType() const override {
					return zeus::message::type::answer;
				}
				/**
				 * @brief set the answer of the call
				 * @param[in] _value Value to add
				 */
				template<class ZEUS_TYPE_DATA>
				void addAnswer(const ememory::SharedPtr<zeus::WebServer>& _iface, const ZEUS_TYPE_DATA& _value) {
					addParameter(_iface, _value);
				}
				/**
				 * @brief get the answer value
				 * @param[in] Data of the answer
				 */
				template<class ZEUS_TYPE_DATA>
				ZEUS_TYPE_DATA getAnswer() const {
					return getParameter<ZEUS_TYPE_DATA>(m_iface, 0);
				}
				/**
				 * @brief Ann an error on the message answer
				 * @param[in] _value error value (simple string with no space with all capital letter separaete with '_'
				 * @param[in] _comment Human readable help to understand the error
				 */
				void addError(const std::string& _value, const std::string& _comment);
				/**
				 * @brief Check if the answer have an error
				 * @return status of the error.
				 */
				bool hasError() const;
				/**
				 * @brief get the error value (if exist)
				 * @return string of the error
				 */
				const std::string& getError() const;
				/**
				 * @brief get the error help (if exist)
				 * @return string of the error help
				 */
				const std::string& getErrorHelp() const;
		};
	}
}
