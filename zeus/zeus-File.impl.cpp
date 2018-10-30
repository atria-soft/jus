/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/zeus-File.impl.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/mineType.hpp>
#include <algue/sha512.hpp>
#include <etk/path/fileSystem.hpp>
#include <etk/Exception.hpp>
#include "debug.hpp"


#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::FileImpl);


ememory::SharedPtr<zeus::File> zeus::File::create(etk::Uri _fileNameReal) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal);
}

ememory::SharedPtr<zeus::File> zeus::File::create(etk::Uri _fileNameReal, etk::String _sha512) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _sha512);
}

ememory::SharedPtr<zeus::File> zeus::File::create(etk::Uri _fileNameReal, etk::String _fileNameShow, etk::String _mineType) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _fileNameShow, _mineType);
}
ememory::SharedPtr<zeus::File> zeus::File::create(etk::Uri _fileNameReal, etk::String _fileNameShow, etk::String _mineType, etk::String _sha512) {
	return ememory::makeShared<zeus::FileImpl>(_fileNameReal, _fileNameShow, _mineType, _sha512);
}

ememory::SharedPtr<zeus::File> zeus::File::create(etk::Vector<uint8_t> _fileData, etk::String _virtualName, etk::String _mineType) {
	return ememory::makeShared<zeus::FileImpl>(etk::move(_fileData), _virtualName, _mineType);
}


zeus::FileImpl::FileImpl(etk::Vector<uint8_t> _value, etk::String _virtualName, etk::String _mineType) :
  m_filename(_virtualName),
  m_gettedData(0),
  m_mineType(_mineType),
  m_sha512("") {
	ZEUS_ERROR("    ==============>>>>>>>>>>>>>>     CREATE  FILE 1 " << _virtualName);
	m_dataRaw = true;
	m_data = _value;
	m_size = m_data.size();
}
zeus::FileImpl::FileImpl(etk::Uri _fileNameReal, etk::String _sha512) :
  m_filename(_fileNameReal.getPath().getFileName()),
  m_file(etk::uri::get(_fileNameReal)),
  m_gettedData(0),
  m_sha512(_sha512) {
	ZEUS_ERROR("    ==============>>>>>>>>>>>>>>     CREATE  FILE 3 " << _fileNameReal << "  '" << _sha512 << "' size=" << m_file->size());
	m_size = m_file->size();
	etk::String extention = _fileNameReal.getPath().getExtention();
	m_mineType = zeus::getMineType(extention);
	if (    _sha512.size() > 0
	     && _sha512.size() != 128) {
		ZEUS_ERROR("Set a wrong sha512 file type");
		_sha512.clear();
	}
}

// sha 512 example: 6134b4a4b5b116cf1b1b757c5aa48bd8b3482b86c6d3fee389a0a3232f74e7331e5f8af6ad516d2ca92eda0a475f44e1291618562ce6f9e54634ba052650dcd7
//                  000000000100000000020000000003000000000400000000050000000006000000000700000000080000000009000000000A000000000B000000000C00000000
zeus::FileImpl::FileImpl(etk::Uri _fileNameReal, etk::String _fileNameShow, etk::String _mineType, etk::String _sha512) :
  m_filename(_fileNameShow),
  m_file(etk::uri::get(_fileNameReal)),
  m_gettedData(0),
  m_mineType(_mineType),
  m_sha512(_sha512) {
	ZEUS_ERROR("    ==============>>>>>>>>>>>>>>     CREATE  FILE 2 " << _fileNameReal);
	m_size = m_file->size();
	if (    _sha512.size() > 0
	     && _sha512.size() != 128) {
		ZEUS_ERROR("Set a wrong sha512 file type");
		_sha512.clear();
	}
}

zeus::FileImpl::~FileImpl() {
	ZEUS_ERROR("    <<<<<<<<<<<<<<==============     DESTROY FILE");
	if (m_file->isOpen() == true) {
		m_file->close();
	}
}

uint64_t zeus::FileImpl::getSize() {
	return m_size;
}

etk::String zeus::FileImpl::getName() {
	return m_filename;
}

etk::String zeus::FileImpl::getSha512() {
	ZEUS_VERBOSE("Get SHA 512 ... " << m_sha512.size() << "  '" << m_sha512 << "'");
	if (m_sha512 == "") {
		ZEUS_INFO("calculation of sha 512 (start)");
		if (m_dataRaw == false) {
			m_sha512 = algue::stringConvert(algue::sha512::encodeFromFile(m_file));
		} else {
			m_sha512 = algue::stringConvert(algue::sha512::encode(m_data));
		}
		ZEUS_INFO("calculation of sha 512 (stop)");
	}
	ZEUS_VERBOSE("return sha512 : '" << m_sha512 << "'");
	return m_sha512;
}

etk::String zeus::FileImpl::getMineType() {
	return m_mineType;
}

zeus::Raw zeus::FileImpl::getPart(uint64_t _start, uint64_t _stop) {
	ZEUS_VERBOSE("REQUEST Get part ... " << _start << "   " << _stop);
	if ((_stop - _start) > 25*1024*1024) {
		ZEUS_ERROR("REQUEST more that 25 Mo in a part file ...");
		throw etk::exception::InvalidArgument("REQUEST more that 25 Mo in a part file ..." + etk::toString(_stop - _start) + " Bytes");
		return zeus::Raw();
	}
	if (_start >= m_size) {
		throw etk::exception::InvalidArgument("REQUEST start position out of file size" + etk::toString(_start) + " > " + etk::toString(m_size));
	}
	if (m_dataRaw == false) {
		if (m_file->isOpen() == false) {
			m_file->open(etk::io::OpenMode::Read);
		}
		m_gettedData += (_stop - _start);
		//ZEUS_PRINT("Reading file : " << m_gettedData << "/" << m_size << " ==> " << float(m_gettedData)/float(m_size)*100.0f << "%");
		printf("Reading file : %d/%d ==> %f                                                                          \r", int(m_gettedData), int(m_size), float(m_gettedData)/float(m_size)*100.0f);
		zeus::Raw tmp(_stop - _start);
		if (m_file->seek(_start, etk::io::SeekMode::Start) == false) {
			ZEUS_ERROR("REQUEST seek error ...");
			throw etk::exception::RuntimeError("Seek in the file error");
			return zeus::Raw();
		}
		int64_t sizeCopy = m_file->read(tmp.writeData(), 1, _stop-_start);
		if (m_size <= _stop) {
			m_file->close();
		}
		// TODO : Check if copy is correct ...
		return etk::move(tmp);
	} else {
		m_gettedData += (_stop - _start);
		//ZEUS_PRINT("Reading file : " << m_gettedData << "/" << m_size << " ==> " << float(m_gettedData)/float(m_size)*100.0f << "%");
		printf("Reading file : %d/%d ==> %f                                                                          \r", int(m_gettedData), int(m_size), float(m_gettedData)/float(m_size)*100.0f);
		zeus::Raw tmp(_stop - _start);
		memcpy(tmp.writeData(), &m_data[_start], _stop-_start);
		return etk::move(tmp);
	}
}

etk::String zeus::storeInFile(zeus::ProxyFile _file, etk::Uri _uri) {
	zeus::ActionNotification<etk::String> tmp;
	return zeus::storeInFileNotify(_file, _uri, tmp);
}

etk::String zeus::storeInFileNotify(zeus::ProxyFile _file, etk::Uri _uri, zeus::ActionNotification<etk::String> _notification) {
	auto futSize = _file.getSize();
	auto futSha = _file.getSha512();
	futSize.wait();
	int64_t retSize = futSize.get();
	int64_t offset = 0;
	
	algue::Sha512 shaCtx;
	ememory::SharedPtr<etk::io::Interface> file = etk::uri::get(_uri);
	file->open(etk::io::OpenMode::Write);
	while (retSize > 0) {
		// get by batch of 1 MB
		int32_t nbElement = 1*1024*1024;
		if (retSize<nbElement) {
			nbElement = retSize;
		}
		auto futData = _file.getPart(offset, offset + nbElement);
		// TODO: set a timeout
		futData.wait();
		if (futData.hasError() == true) {
			ZEUS_DEBUG("read: ==> exception when read data ...");
			throw etk::exception::RuntimeError("Error when loading data");
		}
		zeus::Raw buffer = futData.get();
		shaCtx.update(buffer.data(), buffer.size());
		file->write(buffer.data(), 1, buffer.size());
		offset += nbElement;
		retSize -= nbElement;
		ZEUS_VERBOSE("read: " << offset << "/" << futSize.get() << "    " << buffer.size());
		_notification.notify("{\"pourcent\":" + etk::toString(float(offset)/float(buffer.size())) + ", \"comment\":\"download\"");
	}
	file->close();
	// get the final sha512 of the file:
	etk::String sha512String = algue::stringConvert(shaCtx.finalize());
	futSha.wait();
	if (sha512String != futSha.get()) {
		ZEUS_ERROR("get wrong Sha512 local : '" << sha512String << "'");
		ZEUS_ERROR("get wrong Sha512 remote: '" << futSha.get() << "'");
		throw etk::exception::RuntimeError("SHA-512 error check");
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
			throw etk::exception::RuntimeError("Error when loading data");
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
