/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/AbstractFunction.h>
#include <zeus/debug.h>
#include <etk/os/FSNode.h>

ememory::SharedPtr<zeus::Buffer> zeus::createBinaryBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId) {
	ememory::SharedPtr<zeus::Buffer> obj = zeus::Buffer::create();
	if (obj == nullptr) {
		return nullptr;
	}
	obj->setServiceId(_serviceId);
	obj->setCall(_functionName);
	obj->setTransactionId(_transactionId);
	return obj;
}

void zeus::createBinaryParam(int32_t _paramId, const ememory::SharedPtr<zeus::Buffer>& _obj) {
	// Finish recursive parse ...
}

enum zeus::AbstractFunction::type zeus::AbstractFunction::getType() const {
	return m_type;
}

void zeus::AbstractFunction::setType(enum zeus::AbstractFunction::type _type) {
	m_type = _type;
}

const std::string& zeus::AbstractFunction::getName() const {
	return m_name;
}

const std::string& zeus::AbstractFunction::getDescription() const {
	return m_description;
}

void zeus::AbstractFunction::setDescription(const std::string& _desc) {
	m_description = _desc;
}

void zeus::AbstractFunction::setParam(int32_t _idParam, const std::string& _name, const std::string& _desc) {
	ZEUS_TODO("not implemented set param ... '" << _name << "'");
}

void zeus::AbstractFunction::addParam(const std::string& _name, const std::string& _desc) {
	m_paramsDescription.push_back(std::make_pair(_name, _desc));
}

void zeus::AbstractFunction::setReturn(const std::string& _desc) {
	m_returnDescription = _desc;
}

std::string zeus::AbstractFunction::getPrototypeFull() const {
	std::string out = getPrototypeReturn();
	out += " ";
	out += m_name;
	out += "(";
	std::vector<std::string> tmp = getPrototypeParam();
	for (size_t iii=0; iii<tmp.size(); ++iii) {
		if (iii != 0) {
			out += ", ";
		}
		out += tmp[iii];
		if (iii < m_paramsDescription.size()) {
			out += " " + m_paramsDescription[iii].first;
		}
	}
	out += ");";
	return out;
}


zeus::AbstractFunction::AbstractFunction(const std::string& _name,
                                        const std::string& _desc):
  m_type(zeus::AbstractFunction::type::unknow),
  m_name(_name),
  m_description(_desc) {
	
}


bool zeus::AbstractFunction::checkCompatibility(const ParamType& _type, const ParamType& _params) {
	if (_params == _type) {
		return true;
	}
	// check cross compatibilité (All number are convertible (with min/max) and all number vector are convertible (min/max too)
	if (    _type.isNumber() == true
	     && _params.isNumber() == true) {
		return _type.isVector() == _params.isVector();
	}
	return false;
}
