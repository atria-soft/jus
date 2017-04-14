/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/zeus-Media.impl.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/mineType.hpp>
#include <algue/sha512.hpp>
#include <etk/os/FSNode.hpp>
#include "debug.hpp"

ememory::SharedPtr<zeus::Media> zeus::Media::create(std::string _fileNameReal) {
	return ememory::makeShared<zeus::MediaImpl>(0, _fileNameReal);
}

ejson::Object zeus::MediaImpl::getJson() {
	ejson::Object out;
	out.add("id", ejson::Number(m_id));
	out.add("file-name", ejson::String(m_fileName));
	out.add("name", ejson::String(m_name));
	out.add("mine-type", ejson::String(m_mineType));
	out.add("add-date", ejson::Number(m_creationData.count()));
	if (m_metadata.size() != 0) {
		ejson::Object listMetadata;
		out.add("meta", listMetadata);
		for (auto &it : m_metadata) {
			listMetadata.add(it.first, ejson::String(it.second));
		}
	}
	return out;
}

zeus::MediaImpl::MediaImpl(const std::string& _basePath, ejson::Object _property) :
  m_basePath(_basePath) {
	m_id = _property["id"].toNumber().getU64();
	ZEUS_INFO("get ID : " << m_id);
	m_fileName = _property["file-name"].toString().get();
	m_name = _property["name"].toString().get();
	m_mineType = _property["mine-type"].toString().get();
	m_creationData = echrono::Time(_property["add-date"].toNumber().getU64()*1000);
	
	ejson::Object tmpObj = _property["meta"].toObject();
	if (tmpObj.exist() == true) {
		for (auto itValue = tmpObj.begin();
		     itValue != tmpObj.end();
		     ++itValue) {
			m_metadata.insert(std::make_pair(itValue.getKey(), (*itValue).toString().get()));
		}
	}
}

uint64_t zeus::MediaImpl::getUniqueId() {
	return m_id;
}

zeus::MediaImpl::MediaImpl(uint64_t _id, const std::string& _fileNameReal):
  m_id(_id),
  m_fileName(_fileNameReal) {
	std::string extention;
	if (    m_fileName.rfind('.') != std::string::npos
	     && m_fileName.rfind('.') != 0) {
		extention = etk::tolower(std::string(m_fileName.begin()+m_fileName.rfind('.')+1, m_fileName.end()));
		m_fileName = std::string(m_fileName.begin(), m_fileName.begin()+m_fileName.rfind('.'));
	}
	m_mineType = zeus::getMineType(extention);
	m_creationData = echrono::Time::now();
}

zeus::MediaImpl::~MediaImpl() {
	
}

std::string zeus::MediaImpl::getMineType() {
	return m_mineType;
}

ememory::SharedPtr<zeus::File> zeus::MediaImpl::GetFile() {
	return zeus::File::create(m_basePath + m_fileName + "." + zeus::getExtention(m_mineType), "", m_mineType);
}

std::vector<std::string> zeus::MediaImpl::getMetadataKeys() {
	std::vector<std::string> out;
	for (auto &it : m_metadata) {
		out.push_back(it.first);
	}
	return out;
}

std::string zeus::MediaImpl::getMetadata(std::string _key) {
	auto it = m_metadata.find(_key);
	if (it != m_metadata.end()) {
		return it->second;
	}
	throw std::invalid_argument("KEY '" + _key + "' Does not exist");
}

void zeus::MediaImpl::setMetadata(std::string _key, std::string _value) {
	ZEUS_INFO("metadataSetKey: '" << _key << "' value='" << _value << "'");
	auto it = m_metadata.find(_key);
	if (it != m_metadata.end()) {
		it->second = _value;
	} else {
		m_metadata.insert(std::make_pair(_key, _value));
	}
}


bool zeus::MediaImpl::erase() {
	return etk::FSNodeRemove(m_basePath + m_fileName + "." + zeus::getExtention(m_mineType));
}


