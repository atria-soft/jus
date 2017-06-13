/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>


namespace zeus {
	/**
	 * @brief Raw data Message
	 */
	class Raw {
		private:
			uint32_t m_size; //!< Size of the DATA
			const uint8_t* m_dataExternal; //!< The raw buffer is contituated with ecternal static data
			std::vector<uint8_t> m_dataInternal; //!< the data is allocated in local.
		public:
			/**
			 * @brief generic constructor
			 */
			Raw();
			/**
			 * @brief generic constructor
			 * @param[in] _size Data size that might be allocated by default
			 */
			Raw(uint32_t _size);
			/**
			 * @brief generic constructor (ecternal data)
			 * @param[in] _size Data size of the external data
			 * @param[in] _data Pointer of the externaldata
			 */
			Raw(uint32_t _size, const uint8_t* _data);
			/**
			 * @breif Get size of the data
			 * @return the size of the data >=0
			 */
			uint32_t size() const;
			/**
			 * @brief Get a pointer on the data
			 * @return a pointer on the data (or nullptr)
			 */
			const uint8_t* data() const;
			/**
			 * @brief Get a writable pointer on the data
			 * @return a pointer on the data (or nullptr)
			 */
			uint8_t* writeData();
	};
}
