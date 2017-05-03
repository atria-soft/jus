/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <zeus/AbstractFunction.hpp>
#include <zeus/debug.hpp>
#include <etk/os/FSNode.hpp>

static int32_t firstCall(bool& _value) {
	_value = false;
	return 51;
}

static int32_t secondCall(bool& _value) {
	_value = true;
	return 452;
}

static void unneededCall(int32_t _val1, int32_t _val2) {
	// Nothing to do ...
}

bool zeus::checkOrderFunctionParameter() {
	static bool value = false;
	static bool init = false;
	if (init == true) {
		return value;
	}
	// use a temporary variable to mermit to have multiple first call and like this permit to not need to initialize it while not really needed
	bool valueTmp = false;
	unneededCall(firstCall(valueTmp),secondCall(valueTmp));
	value = valueTmp;
	init = true;
	return value;
}

const std::string zeus::g_threadKeyTransactionId("zeus-transaction-id");
const std::string zeus::g_threadKeyTransactionSource("zeus-transaction-source");
const std::string zeus::g_threadKeyTransactiondestination("zeus-transaction-destination");


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

std::string zeus::AbstractFunction::getPrototype() const {
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


std::string zeus::AbstractFunction::getSignature() const {
	std::string out = getPrototypeReturn();
	out += "(";
	std::vector<std::string> tmp = getPrototypeParam();
	for (size_t iii=0; iii<tmp.size(); ++iii) {
		if (iii != 0) {
			out += ",";
		}
		out += tmp[iii];
	}
	out += ")";
	return out;
}

zeus::AbstractFunction::AbstractFunction(const std::string& _name):
  m_type(zeus::AbstractFunction::type::unknow),
  m_name(_name),
  m_description("") {
	
}
