/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/RemoteProcessCall.hpp>

zeus::RemoteProcessCall::RemoteProcessCall(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId) :
  zeus::WebObj(_iface, _id, _objectId),
  m_type("UNKNOW") {
	zeus::AbstractFunction* func = advertise("getDescription", &zeus::RemoteProcessCall::getDescription);
	if (func != nullptr) {
		func->setDescription("Get description");
		func->setReturn("String version of the service number separate with dot with -dev at the end if developpement version, and '-number' or integration version uipdate : 1.5.2 or 1.8-dev 1.5.2-55");
	}
	func = advertise("getVersion", &zeus::RemoteProcessCall::getVersion);
	if (func != nullptr) {
		func->setDescription("Get version");
		func->setReturn("String version of the service number separate with dot with -dev at the end if developpement version, and '-number' or integration version uipdate : 1.5.2 or 1.8-dev 1.5.2-55");
	}
	func = advertise("getType", &zeus::RemoteProcessCall::getType);
	if (func != nullptr) {
		func->setDescription("Get type");
		func->setReturn("String of generic type of the service base on TYPE-ENTERPRISE.ENTERPRISE-NAME.SERVICE-TYPE/VERSION_PROTOCOL");
	}
	func = advertise("getAuthors", &zeus::RemoteProcessCall::getAuthors2);
	if (func != nullptr) {
		func->setDescription("Get List of developper/maintainer");
		func->setReturn("list of personnes: 'NAME surname <email@xxx.yyy>'");
	}
	func = advertise("getFunctions", &zeus::RemoteProcessCall::getFunctions);
	if (func != nullptr) {
		func->setDescription("Get List of function availlable (filter with right)");
		func->setReturn("list of function name");
	}
	func = advertise("getFunctionSignature", &zeus::RemoteProcessCall::getFunctionSignature);
	if (func != nullptr) {
		func->setDescription("Get List type of return and after the parameters");
		func->addParam("func", "function name");
		func->setReturn("list of element type");
	}
	func = advertise("getFunctionPrototype", &zeus::RemoteProcessCall::getFunctionPrototype);
	if (func != nullptr) {
		func->setDescription("Get List type of return and after the parameters");
		func->addParam("func", "function name");
		func->setReturn("list of element type");
	}
	func = advertise("getFunctionDescription", &zeus::RemoteProcessCall::getFunctionDescription);
	if (func != nullptr) {
		func->setDescription("get function description");
		func->addParam("func", "function name");
		func->setReturn("generic string");
	}
}


void zeus::RemoteProcessCall::setDescription(const std::string& _desc) {
	m_description = _desc;
}

std::string zeus::RemoteProcessCall::getDescription() {
	return m_description;
}

void zeus::RemoteProcessCall::setVersion(const std::string& _desc) {
	m_version = _desc;
}

std::string zeus::RemoteProcessCall::getVersion() {
	return m_version;
}

void zeus::RemoteProcessCall::setVersionImplementation(const std::string& _desc) {
	m_versionImplement = _desc;
}

std::string zeus::RemoteProcessCall::getVersionImplementation() {
	return m_versionImplement;
}

void zeus::RemoteProcessCall::addAuthor(const std::string& _name, const std::string& _email) {
	m_authors.push_back(std::make_pair(_name, _email));
}

const std::vector<std::pair<std::string,std::string>>& zeus::RemoteProcessCall::getAuthors() const {
	return m_authors;
}

std::vector<std::string> zeus::RemoteProcessCall::getAuthors2() {
	std::vector<std::string> out;
	for (auto &it : m_authors) {
		out.push_back(it.first + "<" + it.second + ">");
	}
	return out;
}

zeus::AbstractFunction* zeus::RemoteProcessCall::getFunction(std::string _funcName) {
	for (auto &it : m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		return it;
	}
	return nullptr;
}

std::string zeus::RemoteProcessCall::getType() {
	return m_type;
}

void zeus::RemoteProcessCall::setType(const std::string& _type) {
	m_type = _type;
}


std::vector<std::string> zeus::RemoteProcessCall::getFunctions() {
	std::vector<std::string> out;
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		/*
		if (isFunctionAuthorized(it->getName()) == false) {
			continue;
		}
		*/
		out.push_back(it->getName());
	}
	return out;
}

std::vector<std::string> zeus::RemoteProcessCall::getFunctionSignature(std::string _funcName) {
	/*
	if (isFunctionAuthorized(_funcName) == false) {
		return std::vector<std::string>();
	}
	*/
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		std::vector<std::string> out;
		out = it->getPrototypeParam();
		out.insert(out.begin(), it->getPrototypeReturn());
		return out;
	}
	return std::vector<std::string>();
}

std::string zeus::RemoteProcessCall::getFunctionPrototype(std::string _funcName) {
	/*
	if (isFunctionAuthorized(_funcName) == false) {
		return "";
	}
	*/
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		return it->getPrototype();
	}
	return "";
}

std::string zeus::RemoteProcessCall::getFunctionDescription(std::string _funcName) {
	/*
	if (isFunctionAuthorized(_funcName) == false) {
		return std::string("UNKNOW Function: ") + _funcName;
	}
	*/
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		return it->getDescription();
	}
	return "";

}

bool zeus::RemoteProcessCall::isFunctionAuthorized(uint64_t _clientSessionID, const std::string& _funcName) {
	ZEUS_ERROR("lllllllllllllllllllllllllllllllllllllllllllllllllllllllllll");
	return true;
}
