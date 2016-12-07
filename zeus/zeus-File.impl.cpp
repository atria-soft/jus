/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/zeus-File.impl.hpp>
#include <zeus/mineType.hpp>
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
	m_size = m_node.fileSize();
	m_node.fileOpenRead();
	std::string extention = std::string(_filename.begin()+_filename.size() -3, _filename.end());
	m_mineType = zeus::getMineType(extention);
}

zeus::FileImpl::~FileImpl() {
	m_node.fileClose();
}

uint64_t zeus::FileImpl::getSize() {
	return m_size;
}

std::string zeus::FileImpl::getName() {
	return m_filename;
}

std::string zeus::FileImpl::getMineType() {
	return m_mineType;
}

zeus::Raw zeus::FileImpl::getPart(uint64_t _start, uint64_t _stop) {
	if ((_stop - _start) > 25*1024*1024) {
		ZEUS_ERROR("REQUEST more that 25 Mo in a part file ...");
		// TODO : throw an error ...
		return zeus::Raw();
	}
	zeus::Raw tmp(_stop - _start);
	if (m_node.fileSeek(_start, etk::seekNode_start) == false) {
		ZEUS_ERROR("REQUEST seek error ...");
		// TODO : throw an error ...
		return zeus::Raw();
	}
	int64_t sizeCopy = m_node.fileRead(tmp.writeData(), 1, _stop-_start);
	// TODO : Check if copy is correct ...
	return std::move(tmp);
}



