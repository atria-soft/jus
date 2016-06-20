/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/RemoteProcessCall.h>

zeus::RemoteProcessCall::RemoteProcessCall() :
  m_type("UNKNOW"){
	advertise("getDescription", &zeus::RemoteProcessCall::getDescription);
	setLastFuncDesc("Get description");
	addLastFuncReturn("String version of the service number separate with dot with -dev at the end if developpement version, and '-number' or integration version uipdate : 1.5.2 or 1.8-dev 1.5.2-55");
	
	advertise("getVersion", &zeus::RemoteProcessCall::getVersion);
	setLastFuncDesc("Get version");
	addLastFuncReturn("String version of the service number separate with dot with -dev at the end if developpement version, and '-number' or integration version uipdate : 1.5.2 or 1.8-dev 1.5.2-55");
	
	advertise("getType", &zeus::RemoteProcessCall::getType);
	setLastFuncDesc("Get type");
	addLastFuncReturn("String of generic type of the service base on TYPE-ENTERPRISE.ENTERPRISE-NAME.SERVICE-TYPE/VERSION_PROTOCOL");
	
	advertise("getAuthors", &zeus::RemoteProcessCall::getAuthors2);
	setLastFuncDesc("Get List of developper/maintainer");
	addLastFuncReturn("list of personnes: 'NAME surname <email@xxx.yyy>'");
	
	advertise("getFunctions", &zeus::RemoteProcessCall::getFunctions);
	setLastFuncDesc("Get List of function availlable (filter with right)");
	addLastFuncReturn("list of function name");
	
	advertise("getFunctionSignature", &zeus::RemoteProcessCall::getFunctionSignature);
	addLastFuncParam("func", "function name");
	setLastFuncDesc("Get List type of return and after the parameters");
	addLastFuncReturn("list of element type");
	
	advertise("getFunctionPrototype", &zeus::RemoteProcessCall::getFunctionPrototype);
	addLastFuncParam("func", "function name");
	setLastFuncDesc("Get List type of return and after the parameters");
	addLastFuncReturn("list of element type");
	
	advertise("getFunctionDescription", &zeus::RemoteProcessCall::getFunctionDescription);
	addLastFuncParam("func", "function name");
	setLastFuncDesc("get function description");
	addLastFuncReturn("generic string");
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

void zeus::RemoteProcessCall::setLastFuncDesc(const std::string& _desc) {
	if (m_listFunction.size() == 0) {
		ZEUS_ERROR("Can not set description to a function with no function advertise before ...");
		return;
	}
	if (m_listFunction[m_listFunction.size()-1] == nullptr) {
		ZEUS_ERROR("Last element is nullptr ... ==> what are you doing??");
		return;
	}
	m_listFunction[m_listFunction.size()-1]->setDescription(_desc);
}

void zeus::RemoteProcessCall::setFuncDesc(const std::string& _funcName, const std::string& _desc) {
	for (auto &it : m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		it->setDescription(_desc);
		return;
	}
	ZEUS_ERROR("function '" << _funcName << "' des not exist");
}

void zeus::RemoteProcessCall::addLastFuncParam(const std::string& _name, const std::string& _desc) {
	if (m_listFunction.size() == 0) {
		ZEUS_ERROR("Can not set description to a function with no function advertise before ...");
		return;
	}
	if (m_listFunction[m_listFunction.size()-1] == nullptr) {
		ZEUS_ERROR("Last element is nullptr ... ==> what are you doing??");
		return;
	}
	m_listFunction[m_listFunction.size()-1]->addParam(_name, _desc);
}

void zeus::RemoteProcessCall::setFuncParam(const std::string& _funcName, int32_t _idParam, const std::string& _name, const std::string& _desc) {
	for (auto &it : m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		it->setParam(_idParam, _name, _desc);
		return;
	}
	ZEUS_ERROR("function '" << _funcName << "' des not exist");
}

void zeus::RemoteProcessCall::addLastFuncReturn(const std::string& _desc) {
	if (m_listFunction.size() == 0) {
		ZEUS_ERROR("Can not set return to a function with no function advertise before ...");
		return;
	}
	if (m_listFunction[m_listFunction.size()-1] == nullptr) {
		ZEUS_ERROR("Last element is nullptr ... ==> what are you doing??");
		return;
	}
	m_listFunction[m_listFunction.size()-1]->setReturn(_desc);
}

void zeus::RemoteProcessCall::setFuncReturn(const std::string& _funcName, const std::string& _desc) {
	for (auto &it : m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		it->setReturn(_desc);
		return;
	}
	ZEUS_ERROR("function '" << _funcName << "' des not exist");
}


std::string zeus::RemoteProcessCall::getType() {
	return m_type;
}
void zeus::RemoteProcessCall::setType(const std::string& _type, uint16_t _version) {
	m_type = _type + "/" + etk::to_string(_version);
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
		return it->getPrototypeFull();
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
	return true;
}
