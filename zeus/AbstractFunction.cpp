/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <zeus/AbstractFunction.h>
#include <zeus/debug.h>
#include <etk/os/FSNode.h>

zeus::Buffer zeus::createBinaryCall(uint64_t _transactionId, const std::string& _functionName, const zeus::Buffer& _params) {
	zeus::Buffer callElem = createBinaryBaseCall(_transactionId, _functionName);
	//callElem.add("param", _params);
	return callElem;
}

zeus::Buffer zeus::createBinaryBaseCall(uint64_t _transactionId, const std::string& _functionName, const uint32_t& _serviceId) {
	zeus::Buffer obj;
	obj.setServiceId(_serviceId);
	obj.setCall(_functionName);
	obj.setTransactionId(_transactionId);
	return obj;
}

void zeus::createBinaryParam(std::vector<ActionAsyncClient>& _asyncAction, int32_t _paramId, zeus::Buffer& _obj) {
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


bool zeus::AbstractFunction::checkCompatibility(const ParamType& _type, const std::string& _params) {
	if (createType<bool>() == _type) {
		return _params == "bool";
	}
	if (    createType<int64_t>() == _type
	     || createType<int32_t>() == _type
	     || createType<int16_t>() == _type
	     || createType<int8_t>() == _type
	     || createType<uint64_t>() == _type
	     || createType<uint32_t>() == _type
	     || createType<uint16_t>() == _type
	     || createType<uint8_t>() == _type
	     || createType<float>() == _type
	     || createType<double>() == _type) {
		return    _params == "int8"
		       || _params == "int16"
		       || _params == "int32"
		       || _params == "int64"
		       || _params == "uint8"
		       || _params == "uint16"
		       || _params == "uint32"
		       || _params == "uint64"
		       || _params == "float"
		       || _params == "double";
	}
	if (createType<std::vector<std::string>>() == _type) {
		return _params == "vector:string";
	}
	if (    createType<std::vector<bool>>() == _type
	     || createType<std::vector<int64_t>>() == _type
	     || createType<std::vector<int32_t>>() == _type
	     || createType<std::vector<int16_t>>() == _type
	     || createType<std::vector<int8_t>>() == _type
	     || createType<std::vector<uint64_t>>() == _type
	     || createType<std::vector<uint32_t>>() == _type
	     || createType<std::vector<uint16_t>>() == _type
	     || createType<std::vector<uint8_t>>() == _type
	     || createType<std::vector<float>>() == _type
	     || createType<std::vector<double>>() == _type) {
		return    _params == "vector:int8"
		       || _params == "vector:int16"
		       || _params == "vector:int32"
		       || _params == "vector:int64"
		       || _params == "vector:uint8"
		       || _params == "vector:uint16"
		       || _params == "vector:uint32"
		       || _params == "vector:uint64"
		       || _params == "vector:float"
		       || _params == "vector:double"
		       || _params == "vector:empty";
	}
	if (createType<zeus::File>() == _type) {
		return _params == "file";
	}
	if (createType<std::string>() == _type) {
		return _params == "string";
	}
	return false;
}
