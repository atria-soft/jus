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


ememory::SharedPtr<zeus::File> zeus::File::create(etk::String _fileNameReal) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal);
}

ememory::SharedPtr<zeus::File> zeus::File::create(etk::String _fileNameReal, etk::String _sha512) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _sha512);
}

ememory::SharedPtr<zeus::File> zeus::File::create(etk::String _fileNameReal, etk::String _fileNameShow, etk::String _mineType) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _fileNameShow, _mineType);
}
ememory::SharedPtr<zeus::File> zeus::File::create(etk::String _fileNameReal, etk::String _fileNameShow, etk::String _mineType, etk::String _sha512) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _fileNameShow, _mineType, _sha512);
}

zeus::FileImpl::FileImpl(etk::String _fileNameReal, etk::String _sha512) :
  m_filename(_fileNameReal),
  m_node(_fileNameReal),
  m_gettedData(0),
  m_sha512(_sha512) {
	m_size = m_node.fileSize();
	etk::String extention;
	if (    _fileNameReal.rfind('.') != etk::String::npos
	     && _fileNameReal.rfind('.') != 0) {
		extention = etk::String(_fileNameReal.begin()+_fileNameReal.rfind('.')+1, _fileNameReal.end());
	}
	m_mineType = zeus::getMineType(extention);
	if (    _sha512.size() > 0
	     && _sha512.size() != 128) {
		ZEUS_ERROR("Set a wrong sha512 file type");
		_sha512.clear();
	}
}

// sha 512 example: 6134b4a4b5b116cf1b1b757c5aa48bd8b3482b86c6d3fee389a0a3232f74e7331e5f8af6ad516d2ca92eda0a475f44e1291618562ce6f9e54634ba052650dcd7
//                  000000000100000000020000000003000000000400000000050000000006000000000700000000080000000009000000000A000000000B000000000C00000000
zeus::FileImpl::FileImpl(etk::String _fileNameReal, etk::String _fileNameShow, etk::String _mineType, etk::String _sha512) :
  m_filename(_fileNameShow),
  m_node(_fileNameReal),
  m_gettedData(0),
  m_mineType(_mineType),
  m_sha512(_sha512) {
	m_size = m_node.fileSize();
	if (    _sha512.size() > 0
	     && _sha512.size() != 128) {
		ZEUS_ERROR("Set a wrong sha512 file type");
		_sha512.clear();
	}
}

zeus::FileImpl::~FileImpl() {
	if (m_node.fileIsOpen() == true) {
		m_node.fileClose();
	}
}

uint64_t zeus::FileImpl::getSize() {
	return m_size;
}

etk::String zeus::FileImpl::getName() {
	return m_filename;
}

etk::String zeus::FileImpl::getSha512() {
	if (m_sha512 == "") {
		ZEUS_INFO("calculation of sha 512 (start)");
		m_sha512 = algue::stringConvert(algue::sha512::encodeFromFile(m_node.getFileSystemName()));
		ZEUS_INFO("calculation of sha 512 (stop)");
	}
	return m_sha512;
}

etk::String zeus::FileImpl::getMineType() {
	return m_mineType;
}

zeus::Raw zeus::FileImpl::getPart(uint64_t _start, uint64_t _stop) {
	if ((_stop - _start) > 25*1024*1024) {
		ZEUS_ERROR("REQUEST more that 25 Mo in a part file ...");
		throw std::invalid_argument("REQUEST more that 25 Mo in a part file ..." + etk::toString(_stop - _start) + " Bytes");
		return zeus::Raw();
	}
	if (_start >= m_size) {
		throw std::invalid_argument("REQUEST start position out of file size" + etk::toString(_start) + " > " + etk::toString(m_size));
	}
	if (m_node.fileIsOpen() == false) {
		m_node.fileOpenRead();
	}
	m_gettedData += (_stop - _start);
	//ZEUS_PRINT("Reading file : " << m_gettedData << "/" << m_size << " ==> " << float(m_gettedData)/float(m_size)*100.0f << "%");
	std::cout << "Reading file : " << m_gettedData << "/" << m_size << " ==> " << float(m_gettedData)/float(m_size)*100.0f << "%                                      \r";
	zeus::Raw tmp(_stop - _start);
	if (m_node.fileSeek(_start, etk::seekNode_start) == false) {
		ZEUS_ERROR("REQUEST seek error ...");
		throw std::runtime_error("Seek in the file error");
		return zeus::Raw();
	}
	int64_t sizeCopy = m_node.fileRead(tmp.writeData(), 1, _stop-_start);
	if (m_size <= _stop) {
		m_node.fileClose();
	}
	// TODO : Check if copy is correct ...
	return etk::move(tmp);
}

etk::String zeus::storeInFile(zeus::ProxyFile _file, etk::String _filename) {
	zeus::ActionNotification<etk::String> tmp;
	return zeus::storeInFileNotify(_file, _filename, tmp);
}

etk::String zeus::storeInFileNotify(zeus::ProxyFile _file, etk::String _filename, zeus::ActionNotification<etk::String> _notification) {
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
		_notification.notify("{\"pourcent\":" + etk::toString(float(offset)/float(buffer.size())) + ", \"comment\":\"download\"");
	}
	nodeFile.fileClose();
	// get the final sha512 of the file:
	etk::String sha512String = algue::stringConvert(shaCtx.finalize());
	futSha.wait();
	if (sha512String != futSha.get()) {
		ZEUS_ERROR("get wrong Sha512 local : '" << sha512String << "'");
		ZEUS_ERROR("get wrong Sha512 remote: '" << futSha.get() << "'");
		throw std::runtime_error("SHA-512 error check");
	}
	return sha512String;
}

etk::Vector<uint8_t> zeus::storeInMemory(zeus::ProxyFile _file) {
	etk::Vector<uint8_t> out;
	auto futSize = _file.getSize();
	auto futSha = _file.getSha512();
	futSize.wait();
	int64_t retSize = futSize.get();
	int64_t offset = 0;
	
	algue::Sha512 shaCtx;
	out.resize(retSize);
	int64_t currentOffset = 0;
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
		memcpy(&out[currentOffset], buffer.data(), buffer.size());
		currentOffset += buffer.size();
		offset += nbElement;
		retSize -= nbElement;
		ZEUS_VERBOSE("read: " << offset << "/" << futSize.get() << "    " << buffer.size());
	}
	// get the final sha512 of the file:
	etk::String sha512String = algue::stringConvert(shaCtx.finalize());
	futSha.wait();
	if (sha512String != futSha.get()) {
		ZEUS_ERROR("get wrong Sha512 local : '" << sha512String << "'");
		ZEUS_ERROR("get wrong Sha512 remote: '" << futSha.get() << "'");
	}
	return out;
}