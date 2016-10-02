/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/ParamType.hpp>
#include <zeus/BufferParameter.hpp>

namespace zeus {

	class BufferAnswer :
	  public BufferParameter {
		friend class zeus::Buffer;
		protected:
			std::string m_errorType;
			std::string m_errorHelp;
		protected:
			/**
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::BufferAnswer::create
			 */
			BufferAnswer() {
				m_header.flags = ZEUS_BUFFER_FLAG_FINISH + uint8_t(zeus::Buffer::typeMessage::answer);
			};
			void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
			void appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj) override;
			bool writeOn(enet::WebSocket& _interface) override;
			void generateDisplay(std::ostream& _os) const override;
		public:
			/**
			 * @brief Create a shared pointer on the BufferAnswer
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::BufferAnswer> create();
		public:
			enum zeus::Buffer::typeMessage getType() const override {
				return zeus::Buffer::typeMessage::answer;
			}
			/**
			 * @brief set the answer of the call
			 * @param[in] _value Value to add
			 */
			template<class ZEUS_TYPE_DATA>
			void addAnswer(const ZEUS_TYPE_DATA& _value) {
				addParameter(_value);
			}
			/**
			 * @brief get the answer value
			 * @param[in] Data of the answer
			 */
			// TODO : Do it better check error ... ==> can be good ...
			template<class ZEUS_TYPE_DATA>
			ZEUS_TYPE_DATA getAnswer() const {
				return getParameter<ZEUS_TYPE_DATA>(0);
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
