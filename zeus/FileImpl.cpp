/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/FileImpl.hpp>
#include "debug.hpp"

namespace zeus {
	template<>
	ememory::SharedPtr<zeus::File> File::create<std::string>(std::string _filename) {
		return ememory::makeShared<zeus::FileImpl>(_filename);
	}
	template<>
	ememory::SharedPtr<zeus::File> File::create<const char*>(const char* _filename) {
		return ememory::makeShared<zeus::FileImpl>(_filename);
	}
}

zeus::FileImpl::FileImpl(std::string _filename) :
  m_filename(_filename),
  m_node(_filename) {
	m_node.fileOpenRead();
}

zeus::FileImpl::~FileImpl() {
	m_node.fileClose();
}

uint64_t zeus::FileImpl::size() {
	return 4092;
}

std::string zeus::FileImpl::name() {
	return m_filename;
}

std::string zeus::FileImpl::getMineType() {
	return "audio/x-matroska";
}

zeus::Raw zeus::FileImpl::getPart(uint64_t _start, uint64_t _stop) {
	return zeus::Raw(4092);
}



