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
			std::vector<uint8_t> m_dataInternal;
		public:
			/**
			 * @brief generic constructor
			 */
			Raw();
			/**
			 * @brief generic constructor
			 */
			Raw(uint32_t _size);
			
			Raw(uint32_t _size, const uint8_t* _data);
			
			~Raw();
			
			
			uint32_t size() const;
			
			const uint8_t* data() const;
			uint8_t* writeData();
	};
}

