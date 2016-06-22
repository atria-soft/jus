/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>

namespace zeus {
	class File {
		private:
			std::string m_mineType;
			int32_t m_fileSize;
			std::vector<uint8_t> m_data;
		public:
			File();
			File(const std::string& _filename);
			File(const std::string& _mineType, std::vector<uint8_t> _data, int32_t _fileSize = -1);
			int32_t getTheoricFileSize() const {
				return m_fileSize;
			}
			void storeIn(const std::string& _filename) const;
			const std::string& getMineType() const {
				return m_mineType;
			}
			void setMineType(const std::string& _type) {
				m_mineType = _type;
			}
			void preSetDataSize(uint64_t _size) {
				m_data.resize(_size, 0);
			}
			const std::vector<uint8_t>& getData() const {
				return m_data;
			}
			void setData(uint64_t _offset, const std::vector<uint8_t>& _data);
			void setData(uint64_t _offset, const uint8_t* _data, uint32_t _size);
	};
	class FileServer {
		private:
			std::string m_name;
		public:
			FileServer();
			FileServer(const std::string& _filename);
			const std::string& getFileName() const {
				return m_name;
			}
	};
}

