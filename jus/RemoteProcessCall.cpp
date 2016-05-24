/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/RemoteProcessCall.h>


std::vector<jus::AbstractFunction*> m_listFunction;


void jus::RemoteProcessCall::setDescription(const std::string& _desc) {
	m_description = _desc;
}

const std::string& jus::RemoteProcessCall::getDescription() const {
	return m_description;
}

void jus::RemoteProcessCall::setVersion(const std::string& _desc) {
	m_version = _desc;
}

const std::string& jus::RemoteProcessCall::getVersion() const {
	return m_version;
}

void jus::RemoteProcessCall::addAuthor(const std::string& _name, const std::string& _email) {
	m_authors.push_back(std::make_pair(_name, _email));
}

const std::vector<std::pair<std::string,std::string>>& jus::RemoteProcessCall::getAuthors() const {
	return m_authors;
}

void jus::RemoteProcessCall::setLastFuncDesc(const std::string& _desc) {
	if (m_listFunction.size() == 0) {
		JUS_ERROR("Can not set description to a function with no function advertise before ...");
		return;
	}
	if (m_listFunction[m_listFunction.size()-1] == nullptr) {
		JUS_ERROR("Last element is nullptr ... ==> what are you doing??");
		return;
	}
	m_listFunction[m_listFunction.size()-1]->setDescription(_desc);
}

void jus::RemoteProcessCall::setFuncDesc(const std::string& _funcName, const std::string& _desc) {
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
	JUS_ERROR("function '" << _funcName << "' des not exist");
}

void jus::RemoteProcessCall::addLastFuncParam(const std::string& _name, const std::string& _desc) {
	if (m_listFunction.size() == 0) {
		JUS_ERROR("Can not set description to a function with no function advertise before ...");
		return;
	}
	if (m_listFunction[m_listFunction.size()-1] == nullptr) {
		JUS_ERROR("Last element is nullptr ... ==> what are you doing??");
		return;
	}
	m_listFunction[m_listFunction.size()-1]->addParam(_name, _desc);
}

void jus::RemoteProcessCall::setFuncParam(const std::string& _funcName, int32_t _idParam, const std::string& _name, const std::string& _desc) {
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
	JUS_ERROR("function '" << _funcName << "' des not exist");
}
