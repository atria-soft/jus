/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>

namespace jus {
	class File {
		private:
			std::string m_mineType;
			std::vector<uint8_t> m_data;
		public:
			File();
			File(const std::string& _filename);
			File(const std::string& _mineType, std::vector<uint8_t> _data);
			const std::string& getMineType() {
				return m_mineType;
			}
			const std::vector<uint8_t>& getData() {
				return m_data;
			}
	};
	class FileServer {
		private:
			std::string m_name;
		public:
			FileServer();
			FileServer(const std::string& _filename);
	};
}

