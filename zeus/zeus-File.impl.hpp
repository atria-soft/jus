/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/File.hpp>
#include <etk/os/FSNode.hpp>

namespace zeus {
	class FileImpl : public zeus::File {
		private:
			std::string m_filename; //!< Name of the file
			etk::FSNode m_node; //!< File node access
			size_t m_size; //!< Size of the file
			size_t m_gettedData; //!< Number of byte loaded by the remote client
			std::string m_mineType; //!< Mine Type of the file
			std::string m_sha512; //!< Global file sha-512
		public:
			FileImpl(std::string _fileNameReal, std::string _fileNameShow, std::string _mineType);
			FileImpl(std::string _fileNameReal);
			~FileImpl();
			uint64_t getSize() override;
			std::string getName() override;
			std::string getSha512() override;
			std::string getMineType() override;
			zeus::Raw getPart(uint64_t _start, uint64_t _stop) override;
		
	};
}

