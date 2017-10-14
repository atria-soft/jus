/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <zeus/RemoteProcessCall.hpp>


#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::RemoteProcessCall);

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

void zeus::RemoteProcessCall::setDescription(const etk::String& _desc) {
	m_description = _desc;
}

etk::String zeus::RemoteProcessCall::getDescription() {
	return m_description;
}

void zeus::RemoteProcessCall::setVersion(const etk::String& _desc) {
	m_version = _desc;
}

etk::String zeus::RemoteProcessCall::getVersion() {
	return m_version;
}

void zeus::RemoteProcessCall::setVersionImplementation(const etk::String& _desc) {
	m_versionImplement = _desc;
}

etk::String zeus::RemoteProcessCall::getVersionImplementation() {
	return m_versionImplement;
}

void zeus::RemoteProcessCall::addAuthor(const etk::String& _name, const etk::String& _email) {
	m_authors.pushBack(etk::makePair(_name, _email));
}

const etk::Vector<etk::Pair<etk::String,etk::String>>& zeus::RemoteProcessCall::getAuthors() const {
	return m_authors;
}

etk::Vector<etk::String> zeus::RemoteProcessCall::getAuthors2() {
	etk::Vector<etk::String> out;
	for (auto &it : m_authors) {
		out.pushBack(it.first + "<" + it.second + ">");
	}
	return out;
}

zeus::AbstractFunction* zeus::RemoteProcessCall::getFunction(etk::String _funcName) {
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

etk::String zeus::RemoteProcessCall::getType() {
	return m_type;
}

void zeus::RemoteProcessCall::setType(const etk::String& _type) {
	m_type = _type;
}


etk::Vector<etk::String> zeus::RemoteProcessCall::getFunctions() {
	etk::Vector<etk::String> out;
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		/*
		if (isFunctionAuthorized(it->getName()) == false) {
			continue;
		}
		*/
		out.pushBack(it->getName());
	}
	return out;
}

etk::Vector<etk::String> zeus::RemoteProcessCall::getFunctionSignature(etk::String _funcName) {
	/*
	if (isFunctionAuthorized(_funcName) == false) {
		return etk::Vector<etk::String>();
	}
	*/
	for (auto &it: m_listFunction) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() != _funcName) {
			continue;
		}
		etk::Vector<etk::String> out;
		out = it->getPrototypeParam();
		out.insert(out.begin(), it->getPrototypeReturn());
		return out;
	}
	return etk::Vector<etk::String>();
}

etk::String zeus::RemoteProcessCall::getFunctionPrototype(etk::String _funcName) {
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

etk::String zeus::RemoteProcessCall::getFunctionDescription(etk::String _funcName) {
	/*
	if (isFunctionAuthorized(_funcName) == false) {
		return etk::String("UNKNOW Function: ") + _funcName;
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

bool zeus::RemoteProcessCall::isFunctionAuthorized(uint64_t _clientSessionID, const etk::String& _funcName) {
	ZEUS_ERROR("lllllllllllllllllllllllllllllllllllllllllllllllllllllllllll");
	return true;
}
