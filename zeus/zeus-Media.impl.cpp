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

ememory::SharedPtr<zeus::Media> zeus::Media::create(etk::String _fileNameReal) {
	return ememory::makeShared<zeus::MediaImpl>(0, _fileNameReal);
}

ejson::Object zeus::MediaImpl::getJson() {
	ejson::Object out;
	out.add("id", ejson::Number(m_id));
	out.add("file-name", ejson::String(m_fileName));
	if (m_metadata.size() != 0) {
		ejson::Object listMetadata;
		out.add("meta", listMetadata);
		for (auto &it : m_metadata) {
			listMetadata.add(it.first, ejson::String(it.second));
		}
	}
	return out;
}

zeus::MediaImpl::MediaImpl(const etk::String& _basePath, ejson::Object _property) :
  m_basePath(_basePath) {
	m_id = _property["id"].toNumber().getU64();
	ZEUS_INFO("get ID : " << m_id);
	m_fileName = _property["file-name"].toString().get();
	ejson::Object tmpObj = _property["meta"].toObject();
	if (tmpObj.exist() == true) {
		for (auto itValue = tmpObj.begin();
		     itValue != tmpObj.end();
		     ++itValue) {
			m_metadata.insert(etk::makePair(itValue.getKey(), (*itValue).toString().get()));
		}
	}
}

uint64_t zeus::MediaImpl::getUniqueId() {
	return m_id;
}

zeus::MediaImpl::MediaImpl(uint64_t _id, const etk::String& _fileNameReal, const etk::String& _basePath):
  m_id(_id),
  m_basePath(_basePath),
  m_fileName(_fileNameReal) {
	etk::String extention;
	if (    m_fileName.rfind('.') != etk::String::npos
	     && m_fileName.rfind('.') != 0) {
		extention = etk::tolower(etk::String(m_fileName.begin()+m_fileName.rfind('.')+1, m_fileName.end()));
		m_fileName = etk::String(m_fileName.begin(), m_fileName.begin()+m_fileName.rfind('.'));
	}
	if (extention != "") {
		setMetadata("mime-type", zeus::getMineType(extention));
		//m_creationData = echrono::Time::now();
	}
}

zeus::MediaImpl::~MediaImpl() {
	
}

etk::String zeus::MediaImpl::getMineType() {
	return getMetadata("mime-type");
}


etk::String zeus::MediaImpl::getSha512() {
	try {
		return getMetadata("sha512");
	} catch (std::invalid_argument _eee) {
		// Nothing to do ...
	}
	etk::String sha512;
	auto it = m_metadata.find("mime-type");
	if (it != m_metadata.end()) {
		sha512 = algue::stringConvert(algue::sha512::encodeFromFile(m_basePath + m_fileName + "." + zeus::getExtention(it->second)));
	} else {
		sha512 = algue::stringConvert(algue::sha512::encodeFromFile(m_basePath + m_fileName));
	}
	setMetadata("sha512", sha512);
	return sha512;
}


void zeus::MediaImpl::forceUpdateDecoratedName() {
	// force the new calculation ..
	etk::String value = getDecoratedNameFrom(m_metadata);
	setMetadata("decorated-name", value);
}

etk::String zeus::MediaImpl::getDecoratedName() {
	try {
		etk::String out = getMetadata("decorated-name");
		if (out != "") {
			return out;
		}
	} catch (std::invalid_argument _eee) {
		// Nothing to do ...
	}
	// Store the metadat to not calculated it all the time ...
	etk::String value = getDecoratedNameFrom(m_metadata);
	setMetadata("decorated-name", value);
	return value;
}

ememory::SharedPtr<zeus::File> zeus::MediaImpl::getFile() {
	auto it = m_metadata.find("mime-type");
	if (it != m_metadata.end()) {
		return zeus::File::create(m_basePath + m_fileName + "." + zeus::getExtention(it->second), "", it->second, getSha512());
	}
	// no mimetype specify ... ==> theoric impossible case ...
	return zeus::File::create(m_basePath + m_fileName, "", "");
}

etk::Vector<etk::String> zeus::MediaImpl::getMetadataKeys() {
	etk::Vector<etk::String> out;
	for (auto &it : m_metadata) {
		out.pushBack(it.first);
	}
	return out;
}

etk::String zeus::MediaImpl::getMetadata(etk::String _key) {
	auto it = m_metadata.find(_key);
	if (it != m_metadata.end()) {
		return it->second;
	}
	throw std::invalid_argument("KEY '" + _key + "' Does not exist");
}

void zeus::MediaImpl::setMetadata(etk::String _key, etk::String _value) {
	ZEUS_INFO("metadataSetKey: '" << _key << "' value='" << _value << "'");
	auto it = m_metadata.find(_key);
	if (it != m_metadata.end()) {
		if (_value == "") {
			m_metadata.erase(it);
		} else if (it->second == _value) {
			return;
		} else {
			it->second = _value;
		}
	} else {
		m_metadata.insert(etk::makePair(_key, _value));
	}
	// hook to remove some case that does not call the callback ==> can change many times ...
	if (    _key == "sha512"
	     || _key == "mime-type"
	     || _key == "decorated-name") {
		return;
	}
	if (m_callback != nullptr) {
		m_callback(this, _key);
	}
}


bool zeus::MediaImpl::erase() {
	auto it = m_metadata.find("mime-type");
	if (it != m_metadata.end()) {
		return etk::FSNodeRemove(m_basePath + m_fileName + "." + zeus::getExtention(it->second));
	}
	return etk::FSNodeRemove(m_basePath + m_fileName);
}

bool zeus::MediaImpl::move(const etk::String& _newOffsetFile) {
	ZEUS_INFO("move file : '" << m_basePath + m_fileName << "' ==> " << m_basePath + _newOffsetFile << "'");
	if (_newOffsetFile == m_fileName) {
		// nothing to do ...
		return true;
	}
	auto it = m_metadata.find("mime-type");
	if (it != m_metadata.end()) {
		bool ret = etk::FSNodeMove(m_basePath + m_fileName + "." + zeus::getExtention(it->second), m_basePath + _newOffsetFile + "." + zeus::getExtention(it->second));
		if (ret == true) {
			m_fileName = _newOffsetFile;
		}
		return ret;
	}
	bool ret = etk::FSNodeMove(m_basePath + m_fileName, m_basePath + _newOffsetFile);
	if (ret == true) {
		m_fileName = _newOffsetFile;
	}
	return ret;
}



etk::String zeus::MediaImpl::getMetadataFrom(const etk::Map<etk::String, etk::String>& _metadata, etk::String _key) {
	auto it = _metadata.find(_key);
	if (it != _metadata.end()) {
		return it->second;
	}
	return "";
}

etk::String zeus::MediaImpl::getDecoratedNameFrom(const etk::Map<etk::String, etk::String>& _metadata) {
	etk::String basePath;
	etk::String type = getMetadataFrom(_metadata, "type");
	etk::String title = getMetadataFrom(_metadata, "title");
	/*
	ZEUS_INFO("---- : " << type << "  " << title << "   ");
	for (auto &it: _metadata) {
		ZEUS_INFO("         " << it.first << "  " << it.second);
	}
	*/
	etk::String out;
	if (type == "film") {
		etk::String productionMethode = getMetadataFrom(_metadata, "production-methode");
		etk::String serie = getMetadataFrom(_metadata, "series-name");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		if (productionMethode == "picture") {
			// real film with real human
			basePath += "film/";
		} else if (productionMethode == "draw") {
			// annimation films
			basePath += "film-annimation/";
		} else if (productionMethode == "short") {
			// annimation films
			basePath += "film-short/";
		} else {
			basePath += "film-other/";
		}
		if (serie != "") {
			out += serie + "-";
		}
		if (episode != "") {
			out += "e" + episode + "-";
		}
	} else if (type == "tv-show") {
		etk::String productionMethode = getMetadataFrom(_metadata, "production-methode");
		etk::String serie = getMetadataFrom(_metadata, "series-name");
		etk::String saison = getMetadataFrom(_metadata, "saison");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		if (productionMethode == "picture") {
			// real film with real human
			basePath += "tv-show/";
		} else if (productionMethode == "draw") {
			// annimation films
			basePath += "tv-show-annimation/";
		} else {
			basePath += "tv-show-other/";
		}
		if (serie != "") {
			basePath += serie + "/";
			out += serie + "-";
		}
		if (saison != "") {
			basePath += "saison_" + saison + "/";
			if (saison.size() < 2) {
				out += "s0" + saison + "-";
			} else {
				out += "s" + saison + "-";
			}
		}
		if (episode != "") {
			if (episode.size() < 2) {
				out += "e0" + episode + "-";
			} else {
				out += "e" + episode + "-";
			}
		}
	} else if (type == "theater") {
		etk::String serie = getMetadataFrom(_metadata, "series-name");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		basePath += type + "/";
		if (serie != "") {
			basePath += serie + "/";
			out += serie + "-";
		}
		if (episode != "") {
			if (episode.size() < 2) {
				out += "e0" + episode + "-";
			} else {
				out += "e" + episode + "-";
			}
		}
	} else if (type == "one-man") {
		etk::String author = getMetadataFrom(_metadata, "author");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		basePath += type + "/";
		if (author != "") {
			basePath += author + "/";
			out += author + "-";
		}
		if (episode != "") {
			if (episode.size() < 2) {
				out += "e0" + episode + "-";
			} else {
				out += "e" + episode + "-";
			}
		}
	} else if (    type == "sound-track"
	            || type == "album") {
		// soundtrack: Original sound track from films
		// album: general audio track
		etk::String author = getMetadataFrom(_metadata, "author");
		etk::String album = getMetadataFrom(_metadata, "album");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		basePath += type + "/";
		if (author != "") {
			basePath += author + "/";
			out += author + "-";
		}
		if (album != "") {
			basePath += album + "/";
			out += album + "-";
		}
		if (episode != "") {
			if (episode.size() < 2) {
				out += "e0" + episode + "-";
			} else {
				out += "e" + episode + "-";
			}
		}
	} else if (type == "file") {
		// generic file that is provided to the generic cloud system ==> simple sharing of files
		etk::String path = getMetadataFrom(_metadata, "path");
		basePath += type + "/";
		if (path != "") {
			basePath += path + "/";
		}
	} else {
		etk::String author = getMetadataFrom(_metadata, "author");
		etk::String group = getMetadataFrom(_metadata, "group");
		etk::String episode = getMetadataFrom(_metadata, "episode");
		etk::String serie = getMetadataFrom(_metadata, "series-name");
		basePath += "unknow/";
		if (author != "") {
			out += author + "-";
		}
		if (group != "") {
			out += group + "-";
		}
		if (serie != "") {
			out += serie + "-";
		}
		if (episode != "") {
			out += "e" + episode + "-";
		}
	}
	out += title;
	etk::String mimeType = getMetadataFrom(_metadata, "mime-type");
	if (mimeType != "") {
		return basePath + out + "." + zeus::getExtention(mimeType);
	}
	return basePath + out;
}