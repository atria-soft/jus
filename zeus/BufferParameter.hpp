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

	class BufferParameter:
	  public Buffer {
		protected:
			mutable std::vector<std::pair<int32_t,std::vector<uint8_t>>> m_parameter; //!< list of the parameter (offset of start data and buffer of data (subprotocol...)
		public:
			/**
			 * @brief Template to get a parameter with a specific type
			 * @param[in] _id Number of the parameter
			 * @return Converted type of the parameter (or empty value)
			 */
			template<class ZEUS_TYPE_DATA>
			ZEUS_TYPE_DATA getParameter(int32_t _id) const;
			/**
			 * @brief Get the type of a parameter.
			 * @param[in] _id Number of the parameter
			 * @return type of the parameter
			 */
			zeus::ParamType getParameterType(int32_t _id) const;
			/**
			 * @brief Get the start pointer of the parameter
			 * @param[in] _id Number of the parameter
			 * @return pointer of the parameter or nullptr
			 */
			const uint8_t* getParameterPointer(int32_t _id) const;
			/**
			 * @brief Get the size of the parameter availlable in the parameter pointer
			 * @param[in] _id Number of the parameter
			 * @return size of the parameter buffer
			 */
			uint32_t getParameterSize(int32_t _id) const;
			/**
			 * @brief Get the number of parameter availlable
			 * @return number of parameter
			 */
			uint16_t getNumberParameter() const;
			/**
			 * @brief Add an empty vector with no type
			 */
			void addParameterEmptyVector();
			/**
			 * @brief Add parameter (or not.. this is the end of an auto resursive add parameter)
			 */
			void addParameter();
			/**
			 * @brief Convert the parameter in a simple human readable string
			 * @param[in] _id Number of the parameter
			 * @return readable string
			 */
			std::string simpleStringParam(uint32_t _id) const;
			void composeWith(const uint8_t* _buffer, uint32_t _lenght) override;
			bool writeOn(enet::WebSocket& _interface) override;
		protected:
			/**
			 * @brief Add a parameter at a specific position
			 * @param[in] _paramId Id of the parameter (needed for the multiple packet sending)
			 * @param[in] _value Value to add in parameter
			 */
			template<class ZEUS_TYPE_DATA>
			void addParameter(uint16_t _paramId, const ZEUS_TYPE_DATA& _value);
		public:
			template<class ZEUS_TYPE_DATA>
			void addParameter(const ZEUS_TYPE_DATA& _value) {
				addParameter(m_parameter.size(), _value);
			}
			void parameterAppendBufferData(ememory::SharedPtr<zeus::BufferData> _obj);
	};
}
