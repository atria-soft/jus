/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#if 0
#include <zeus/File.hpp>
#include <zeus/debug.hpp>
#include <etk/types.hpp>
#include <etk/stdTools.hpp>
#include <zeus/mineType.hpp>
#include <etk/os/FSNode.hpp>
#include <cstring>


zeus::File::File() {
	
}

zeus::File::File(const std::string& _filename) {
	m_data = etk::FSNodeReadAllDataType<uint8_t>(_filename);
	std::string extention = std::string(_filename.begin()+_filename.size() -3, _filename.end());
	m_mineType = zeus::getMineType(extention);
}

void zeus::File::storeIn(const std::string& _filename) const {
	etk::FSNodeWriteAllDataType(_filename, m_data);
}

zeus::File::File(const std::string& _mineType, std::vector<uint8_t> _data, int32_t _fileSize):
  m_mineType(_mineType),
  m_data(_data) {
	if (_fileSize < 0){
		m_fileSize = m_data.size();
	} else {
		m_fileSize = _fileSize;
	}
}

zeus::File::File(const std::string& _mineType, int32_t _size) :
  m_mineType(_mineType),
  m_data() {
	m_data.resize(_size);
	m_fileSize = _size;
}

void zeus::File::setData(uint64_t _offset, const std::vector<uint8_t>& _data) {
	setData(_offset, &_data[0], _data.size());
}

void zeus::File::setData(uint64_t _offset, const uint8_t* _data, uint32_t _size) {
	if (_size+_offset > m_data.size()) {
		ZEUS_ERROR("Need Resize file buffer ... need=" << _size+_offset << "  have=" << m_data.size());
		m_data.resize(_size+_offset);
	}
	memcpy(&m_data[_offset], _data, _size);
}


zeus::FileServer::FileServer() {
	
}

zeus::FileServer::FileServer(const std::string& _filename) :
  m_name(_filename) {
	
}


namespace etk {
	template<>
	std::string to_string<zeus::FileServer>(zeus::FileServer const& _obj) {
		return "";
	}
}

#endif

