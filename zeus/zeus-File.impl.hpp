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
			etk::String m_filename; //!< Name of the file
			etk::FSNode m_node; //!< File node access
			etk::Vector<uint8_t> m_data; //!< data of the file to transfer;
			bool m_dataRaw = false;
			size_t m_size; //!< Size of the file
			size_t m_gettedData; //!< Number of byte loaded by the remote client
			etk::String m_mineType; //!< Mine Type of the file
			etk::String m_sha512; //!< Global file sha-512
		public:
			FileImpl(etk::String _fileNameReal, etk::String _fileNameShow, etk::String _mineType, etk::String _sha512="");
			FileImpl(etk::String _fileNameReal, etk::String _sha512="");
			FileImpl(const etk::Vector<uint8_t>& _value, etk::String _virtualName, etk::String _mineType);
			~FileImpl();
			uint64_t getSize() override;
			etk::String getName() override;
			etk::String getSha512() override;
			etk::String getMineType() override;
			zeus::Raw getPart(uint64_t _start, uint64_t _stop) override;
		
	};
}

