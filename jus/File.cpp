/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/File.h>
#include <jus/debug.h>
#include <etk/types.h>
#include <etk/stdTools.h>
#include <jus/mineType.h>
#include <etk/os/FSNode.h>



jus::File::File() {
	
}

jus::File::File(const std::string& _filename) {
	m_data = etk::FSNodeReadAllDataType<uint8_t>(_filename);
	std::string extention = std::string(_filename.begin()+_filename.size() -3, _filename.end());
	JUS_WARNING("send file: '" << _filename << "' with extention: '" << extention << "'");
	m_mineType = jus::getMineType(extention);
}

void jus::File::storeIn(const std::string& _filename) const {
	etk::FSNodeWriteAllDataType(_filename, m_data);
}

jus::File::File(const std::string& _mineType, std::vector<uint8_t> _data):
  m_mineType(_mineType),
  m_data(_data) {
	
}

void jus::File::setData(uint64_t _offset, const std::vector<uint8_t>& _data) {
	// TODO : Check size/offset before set
	memcpy(&m_data[_offset], &_data[0], _data.size());
}


jus::FileServer::FileServer() {
	
}
jus::FileServer::FileServer(const std::string& _filename) :
  m_name(_filename) {
	
}


namespace etk {
	template<>
	std::string to_string<jus::FileServer>(jus::FileServer const& _obj) {
		return "";
	}
}


