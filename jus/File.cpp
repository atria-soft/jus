/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/File.h>
#include <etk/types.h>
#include <etk/stdTools.h>



jus::File::File() {
	
}

jus::File::File(const std::string& _filename) {
	
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


