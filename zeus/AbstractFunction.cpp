/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <zeus/AbstractFunction.hpp>
#include <zeus/debug.hpp>
#include <etk/os/FSNode.hpp>

#include <etk/typeInfo.hpp>
ETK_DECLARE_TYPE(zeus::AbstractFunction);

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

const etk::String zeus::g_threadKeyTransactionId("zeus-transaction-id");
const etk::String zeus::g_threadKeyTransactionSource("zeus-transaction-source");
const etk::String zeus::g_threadKeyTransactionDestination("zeus-transaction-destination");


enum zeus::AbstractFunction::type zeus::AbstractFunction::getType() const {
	return m_type;
}

void zeus::AbstractFunction::setType(enum zeus::AbstractFunction::type _type) {
	m_type = _type;
}

const etk::String& zeus::AbstractFunction::getName() const {
	return m_name;
}

const etk::String& zeus::AbstractFunction::getDescription() const {
	return m_description;
}

void zeus::AbstractFunction::setDescription(const etk::String& _desc) {
	m_description = _desc;
}

void zeus::AbstractFunction::setParam(int32_t _idParam, const etk::String& _name, const etk::String& _desc) {
	ZEUS_TODO("not implemented set param ... '" << _name << "'");
}

void zeus::AbstractFunction::addParam(const etk::String& _name, const etk::String& _desc) {
	m_paramsDescription.pushBack(etk::makePair(_name, _desc));
}

void zeus::AbstractFunction::setReturn(const etk::String& _desc) {
	m_returnDescription = _desc;
}

etk::String zeus::AbstractFunction::getPrototype() const {
	etk::String out = getPrototypeReturn();
	out += " ";
	out += m_name;
	out += "(";
	etk::Vector<etk::String> tmp = getPrototypeParam();
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


etk::String zeus::AbstractFunction::getSignature() const {
	etk::String out = getPrototypeReturn();
	out += "(";
	etk::Vector<etk::String> tmp = getPrototypeParam();
	for (size_t iii=0; iii<tmp.size(); ++iii) {
		if (iii != 0) {
			out += ",";
		}
		out += tmp[iii];
	}
	out += ")";
	return out;
}

zeus::AbstractFunction::AbstractFunction(const etk::String& _name):
  m_type(zeus::AbstractFunction::type::unknow),
  m_name(_name),
  m_description("") {
	
}
