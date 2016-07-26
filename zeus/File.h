/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>

namespace zeus {
	/**
	 * @brief File inetrface
	 */
	class File {
		private:
			std::string m_mineType;
			int32_t m_fileSize;
			std::vector<uint8_t> m_data;
		public:
			/**
			 * @brief Contructor of an empty file
			 */
			File();
			/**
			 * @brief Contructor with the name of a file (load all data)
			 * @param[in] _filename path of the file
			 */
			File(const std::string& _filename);
			/**
			 * @brief Constructor of a typed file
			 * @param[in] _mineType mine type of the file
			 * @param[in] _data data of the file
			 * @param[in] _fileSize theoric file size
			 */
			File(const std::string& _mineType, std::vector<uint8_t> _data, int32_t _fileSize = -1);
			/**
			 * @brief Constructor of a typed file
			 * @param[in] _mineType mine type of the file
			 * @param[in] _fileSize preallocation size (data will be set after ...)
			 */
			File(const std::string& _mineType, int32_t _size);
			/**
			 * @brief get the théoric file size
			 * @return size of the file
			 */
			int32_t getTheoricFileSize() const {
				return m_fileSize;
			}
			/**
			 * @brief Store data in the specify file
			 * @param[in] _filename name of the file
			 */
			void storeIn(const std::string& _filename) const;
			/**
			 * @brief Get the mine type of the file
			 * @return generic mine type
			 */
			const std::string& getMineType() const {
				return m_mineType;
			}
			/**
			 * @brief Set the mine type of the file
			 * @param[in] _type New type of the file
			 */
			void setMineType(const std::string& _type) {
				m_mineType = _type;
			}
			/**
			 * @brief préallocate the data
			 * @param[in] _size New size of the buffer
			 */
			void preSetDataSize(uint64_t _size) {
				m_data.resize(_size, 0);
			}
			/**
			 * @brief get a rérérence of the data
			 * @return requested data
			 */
			const std::vector<uint8_t>& getData() const {
				return m_data;
			}
			/**
			 * @brief Set data on the buffer
			 * @param[in] _offset offset of the write data
			 * @param[in] _data data to write
			 */
			void setData(uint64_t _offset, const std::vector<uint8_t>& _data);
			/**
			 * @brief Set data on the buffer
			 * @param[in] _offset offset of the write data
			 * @param[in] _data pointer on the data to write
			 * @param[in] _size Number of Octet to write
			 */
			void setData(uint64_t _offset, const uint8_t* _data, uint32_t _size);
	};
	/**
	 * @brief File server interface
	 * @note This class permit to send file without opening all data befor sending
	 */
	class FileServer {
		private:
			std::string m_name;
		public:
			/**
			 * @brief generic constructor
			 */
			FileServer();
			/**
			 * @brief contructor with a file
			 * @param[in] _filename Name of the file to send
			 */
			FileServer(const std::string& _filename);
			/**
			 * @brief Get the filename registered
			 * @return path of the file
			 */
			const std::string& getFileName() const {
				return m_name;
			}
	};
}

