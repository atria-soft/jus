/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/zeus-File.impl.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/mineType.hpp>
#include <algue/sha512.hpp>
#include <etk/os/FSNode.hpp>
#include "debug.hpp"


ememory::SharedPtr<zeus::File> zeus::File::create(std::string _fileNameReal) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal);
}

ememory::SharedPtr<zeus::File> zeus::File::create(std::string _fileNameReal, std::string _fileNameShow, std::string _mineType) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _fileNameShow, _mineType);
}

zeus::FileImpl::FileImpl(std::string _fileNameReal) :
  m_filename(_fileNameReal),
  m_node(_fileNameReal) {
	m_size = m_node.fileSize();
	m_node.fileOpenRead();
	std::string extention = std::string(_fileNameReal.begin()+_fileNameReal.size() -3, _fileNameReal.end());
	m_mineType = zeus::getMineType(extention);
	m_sha512 = algue::stringConvert(algue::sha512::encodeFromFile(_fileNameReal));
}

zeus::FileImpl::FileImpl(std::string _fileNameReal, std::string _fileNameShow, std::string _mineType) :
  m_filename(_fileNameShow),
  m_node(_fileNameReal),
  m_mineType(_mineType) {
	m_size = m_node.fileSize();
	m_node.fileOpenRead();
	m_sha512 = algue::stringConvert(algue::sha512::encodeFromFile(_fileNameReal));
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

std::string zeus::FileImpl::getSha512() {
	return m_sha512;
}

std::string zeus::FileImpl::getMineType() {
	return m_mineType;
}

zeus::Raw zeus::FileImpl::getPart(uint64_t _start, uint64_t _stop) {
	if ((_stop - _start) > 25*1024*1024) {
		ZEUS_ERROR("REQUEST more that 25 Mo in a part file ...");
		throw std::invalid_argument("REQUEST more that 25 Mo in a part file ..." + etk::to_string(_stop - _start) + " Bytes");
		return zeus::Raw();
	}
	if (_start >= m_size) {
		throw std::invalid_argument("REQUEST start position out of file size" + etk::to_string(_start) + " > " + etk::to_string(m_size));
	}
	zeus::Raw tmp(_stop - _start);
	if (m_node.fileSeek(_start, etk::seekNode_start) == false) {
		ZEUS_ERROR("REQUEST seek error ...");
		throw std::runtime_error("Seek in the file error");
		return zeus::Raw();
	}
	int64_t sizeCopy = m_node.fileRead(tmp.writeData(), 1, _stop-_start);
	// TODO : Check if copy is correct ...
	return std::move(tmp);
}

std::string zeus::storeInFile(zeus::ProxyFile _file, std::string _filename) {
	auto futSize = _file.getSize();
	auto futSha = _file.getSha512();
	futSize.wait();
	int64_t retSize = futSize.get();
	int64_t offset = 0;
	
	algue::Sha512 shaCtx;
	etk::FSNode nodeFile(_filename);
	nodeFile.fileOpenWrite();
	while (retSize > 0) {
		// get by batch of 1 MB
		int32_t nbElement = 1*1024*1024;
		if (retSize<nbElement) {
			nbElement = retSize;
		}
		auto futData = _file.getPart(offset, offset + nbElement);
		futData.wait();
		if (futData.hasError() == true) {
			throw std::runtime_error("Error when loading data");
		}
		zeus::Raw buffer = futData.get();
		shaCtx.update(buffer.data(), buffer.size());
		nodeFile.fileWrite(buffer.data(), 1, buffer.size());
		offset += nbElement;
		retSize -= nbElement;
		ZEUS_VERBOSE("read: " << offset << "/" << futSize.get() << "    " << buffer.size());
	}
	nodeFile.fileClose();
	// get the final sha512 of the file:
	std::string sha512String = algue::stringConvert(shaCtx.finalize());
	futSha.wait();
	if (sha512String != futSha.get()) {
		ZEUS_ERROR("get wrong Sha512 local : '" << sha512String << "'");
		ZEUS_ERROR("get wrong Sha512 remote: '" << futSha.get() << "'");
		throw std::runtime_error("SHA-512 error check");
	}
	return sha512String;
}


