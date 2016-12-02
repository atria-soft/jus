/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>


namespace zeus {
	/**
	 * @brief Raw data Buffer
	 */
	class Raw {
		private:
			uint32_t m_size;
			const uint8_t* m_dataExternal;
			uint8_t* m_dataInternal;
		public:
			/**
			 * @brief generic constructor
			 */
			Raw() :
			  m_size(0),
			  m_dataExternal(nullptr),
			  m_dataInternal(nullptr) {
				
			}
			/**
			 * @brief generic constructor
			 */
			Raw(uint32_t _size) :
			  m_size(_size),
			  m_dataExternal(nullptr),
			  m_dataInternal(new uint8_t[_size]) {
				
			}
			
			Raw(uint32_t _size, const uint8_t* _data) :
			  m_size(_size),
			  m_dataExternal(_data),
			  m_dataInternal(nullptr) {
				
			}
			
			~Raw() {
				if (m_dataInternal != nullptr) {
					delete[] m_dataInternal;
					m_dataInternal = nullptr;
				}
			}
			
			
			uint32_t size() const {
				return m_size;
			}
			
			const uint8_t* data() const {
				if (m_dataExternal != nullptr) {
					return m_dataExternal;
				}
				return m_dataInternal;
			}
	};
}

