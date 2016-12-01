#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import os
import copy

list_of_known_type = [
    ["void", "void"],
    ["bool", "bool"],
    ["string", "std::string"],
    ["int8", "int8_t"],
    ["int16", "int16_t"],
    ["int32", "int32_t"],
    ["int64", "int64_t"],
    ["uint8", "uint8_t"],
    ["uint16", "uint16_t"],
    ["uint32", "uint32_t"],
    ["uint64", "uint64_t"],
    ["float32", "float"],
    ["float64", "double"],
    ["vector:bool", "std::vector<bool>"],
    ["vector:string", "std::vector<std::string>"],
    ["vector:int8", "std::vector<int8_t>"],
    ["vector:int16", "std::vector<int16_t>"],
    ["vector:int32", "std::vector<int32_t>"],
    ["vector:int64", "std::vector<int64_t>"],
    ["vector:uint8", "std::vector<uint8_t>"],
    ["vector:uint16", "std::vector<uint16_t>"],
    ["vector:uint32", "std::vector<uint32_t>"],
    ["vector:uint64", "std::vector<uint64_t>"],
    ["vector:float32", "std::vector<float>"],
    ["vector:float64", "std::vector<double>"],
    ["duration", "echrono::Duration"],
    ["time", "echrono::Time"],
    ["file", "zeus::File"],
    ["stream", "zeus::Stream"],
    ["json", "ejson::Object"],
    ]


def get_list_type():
	out = []
	for elem in list_of_known_type:
		out.append(elem[0])
	return out

def convert_type_in_cpp(data):
	for elem in list_of_known_type:
		if data == elem[0]:
			return elem[1]
	debug.error(" can not find type in IDL : '" + data + "'")


def remove_start_stop_spacer(data):
	dataout = copy.deepcopy(data)
	while     len(dataout) >= 1 \
	      and (    dataout[0] == " " \
	            or dataout[0] == "\t"):
		dataout = dataout[1:]
	while     len(dataout) >= 1 \
	      and (    dataout[-1] == " " \
	            or dataout[-1] == "\t"):
		dataout = dataout[:-1]
	return dataout

def capital_first(data):
	return data[0].upper() + data[1:]

class AttributeDefinition:
	def __init__(self):
		self.name = "";
		self.brief = "";
		self.type = "";
	
	def set_name(self, name):
		self.name = remove_start_stop_spacer(name);
	
	def set_brief(self, desc):
		self.name = "";
		self.brief = remove_start_stop_spacer(desc);
		self.type = "";
	
	def set_type(self, type):
		self.type = remove_start_stop_spacer(type);
	
	def display(self):
		debug.info("       BRIEF: " + self.brief)
		debug.info("       " + self.type + " " + self.name + ";")
	
	def generate_doxy_get(self, space, object):
		out = ""
		out += space + "/**\n"
		out += space + " * @brief Get parameter " + self.brief + "\n"
		out += space + " * @return Requested parameter\n"
		out += space + " */\n"
		return out
	
	def generate_doxy_set(self, space, object):
		out = ""
		out += space + "/**\n"
		out += space + " * @brief Set parameter " + self.brief + "\n"
		out += space + " * @param[in] _value New parameter value\n"
		out += space + " */\n"
		return out
	
	def generate_cpp(self, space, object):
		out = "";
		# TODO : Set it in protected
		out += space + convert_type_in_cpp(self.type) + " m_" + self.name + "; //!<" + self.brief + "\n"
		# TODO: set it in public ...
		out += self.generate_doxy_get(space, object)
		out += space + "virtual " + convert_type_in_cpp(self.type) + " get" + capital_first(self.name) + "() {\n"
		out += space + "	return m_" + self.name + ";\n"
		out += space + "}\n"
		out += self.generate_doxy_set(space, object)
		out += space + "virtual void set" + capital_first(self.name) + "(const " + convert_type_in_cpp(self.type) + "& _value) {\n"
		out += space + "	m_" + self.name + " = _value;\n"
		out += space + "}\n"
		return out;
		
	def generate_hpp_proxy(self, space, object):
		out = "";
		out += self.generate_doxy_get(space, object)
		out += space + "virtual zeus::Future<" + convert_type_in_cpp(self.type) + "> get" + capital_first(self.name) + "();\n"
		out += self.generate_doxy_set(space, object)
		out += space + "virtual zeus::Future<void> set" + capital_first(self.name) + "(const " + convert_type_in_cpp(self.type) + "& _value);\n"
		return out;
	
	def generate_cpp_proxy(self, space, class_name, object):
		out = "";
		out += space + "zeus::Future<" + convert_type_in_cpp(self.type) + "> " + class_name + "::get" + capital_first(self.name) + "() {\n"
		out += space + '	return m_obj.call("' + self.name + '.get");\n'
		out += space + "}\n"
		out += space + "zeus::Future<void> " + class_name + "::set" + capital_first(self.name) + "(const " + convert_type_in_cpp(self.type) + "& _value) {\n"
		out += space + '	return m_obj.call("' + self.name + '.set", _value);\n'
		out += space + "}\n"
		return out;


class FunctionDefinition:
	def __init__(self):
		self.name = "";
		self.brief = "";
		self.return_type = "";
		self.return_brief = "";
		self.parameters = []
	
	def set_function_name(self, name):
		self.name = remove_start_stop_spacer(name);
	
	def set_brief(self, desc):
		self.name = "";
		self.brief = remove_start_stop_spacer(desc);
		self.return_type = "";
		self.return_brief = "";
		self.parameters = []
	
	def add_param_comment(self, name, desc):
		for elem in self.parameters:
			if     elem["name"] == "" \
			   and elem["brief"] == "":
				elem["name"] = remove_start_stop_spacer(name);
				elem["brief"] = remove_start_stop_spacer(desc);
				return;
		self.parameters.append({
		    "type":"",
		    "name":remove_start_stop_spacer(name),
		    "brief":remove_start_stop_spacer(desc)
		    })
	
	def set_return_comment(self, desc):
		self.return_brief = remove_start_stop_spacer(desc);
	
	def set_return_type(self, type):
		self.return_type = remove_start_stop_spacer(type);
	
	def add_parameter_type(self, type):
		for elem in self.parameters:
			if elem["type"] == "":
				elem["type"] = remove_start_stop_spacer(type);
				return;
		self.parameters.append({
		    "type":remove_start_stop_spacer(type),
		    "name":"",
		    "brief":""
		    })
		
	def display(self):
		debug.info("       BRIEF: " + self.brief)
		debug.info("       BRIEF-return: " + self.return_brief)
		debug.info("       " + self.return_type + " " + self.name + "(")
		for elem in self.parameters:
			debug.info("               " + elem["type"] + " " + elem["name"] + ", # " + elem["brief"])
		debug.info("       )")
	
	def generate_doxy(self, space, object):
		# generate doxygen comment:
		out = space + "/**\n"
		if self.brief != "":
			out += space + " * @brief " + self.brief + "\n"
		for elem in self.parameters:
			if     elem["name"] == "" \
			   and elem["brief"] == "":
				continue
			out += space + " * @param[in] "
			if elem["name"] != "":
				out += elem["name"] + " "
			if elem["brief"] != "":
				out += elem["brief"] + " "
			out += "\n"
		if self.return_brief != "":
			out += space + " * @return " + self.return_brief + "\n"
		out += space + " */\n"
		return out
	
	def generate_cpp(self, space, object):
		out = "";
		out += self.generate_doxy(space, object)
		out += space + "virtual "
		out += convert_type_in_cpp(self.return_type) + " " + self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += convert_type_in_cpp(elem["type"]) + " _"
			if elem["name"] == "":
				param_data += "no_name_param_" + str(id_parameter)
			else:
				param_data += elem["name"]
		out += param_data
		out += ") = 0;\n"
		return out;
		
	def generate_hpp_proxy(self, space, object):
		out = "";
		out += self.generate_doxy(space, object)
		out += space + "virtual zeus::Future<" + convert_type_in_cpp(self.return_type) + "> " + self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += "const " + convert_type_in_cpp(elem["type"]) + "& _"
			if elem["name"] == "":
				param_data += "no_name_param_" + str(id_parameter)
			else:
				param_data += elem["name"]
		out += param_data
		out += ");\n"
		return out;
	def generate_cpp_proxy(self, space, class_name, object):
		out = "";
		out += space + "zeus::Future<" + convert_type_in_cpp(self.return_type) + "> " + class_name + "::" + self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += "const " + convert_type_in_cpp(elem["type"]) + "& _"
			if elem["name"] == "":
				param_data += "no_name_param_" + str(id_parameter)
			else:
				param_data += elem["name"]
		out += param_data
		out += ") {\n"
		space += "	"
		out += space + 'return m_obj.call("' + self.name + '"'
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			out += ", "
			out += "_"
			if elem["name"] == "":
				out += "no_name_param_" + str(id_parameter)
			else:
				out += elem["name"]
		out += ');\n'
		out += "}\n"
		space = space[:-1]
		return out;


class ServiceDefinition:
	def __init__(self):
		self.name = [""];
		self.brief = "";
		self.version = "";
		self.api = "";
		self.authors = []
		self.attributes = []
		self.functions = []
	
	def set_name(self, value):
		self.name = value
		# TODO : Check range ...
		self.name[-1] = capital_first(self.name[-1])
	
	def set_brief(self, value):
		self.brief = remove_start_stop_spacer(value)
	
	def set_version(self, value):
		self.version = remove_start_stop_spacer(value)
	
	def set_api(self, value):
		self.api = remove_start_stop_spacer(value)
	
	def add_author(self, value):
		self.authors.append(remove_start_stop_spacer(value))
	
	def add_function(self, value):
		# TODO : Check if function already exist
		self.functions.append(value)
	
	def add_attribute(self, value):
		# TODO : Check if attribute already exist
		self.attributes.append(value)
	
	def display(self):
		debug.info("Display service definition : ")
		debug.info("    name:    " + str(self.name))
		debug.info("    brief:   '" + str(self.brief) + "'")
		debug.info("    version: '" + str(self.version) + "'")
		debug.info("    api:     '" + str(self.api) + "'")
		debug.info("    authors: '" + str(self.authors) + "'")
		debug.info("    functions: ")
		for elem in self.functions:
			elem.display();
	
	def generate_header(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += self.name[-1] + ".hpp";
		out = ""
		# TODO: add global header:
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <string>\n"
		out += "#include <vector>\n"
		out += "\n"
		space = ""
		for elem in self.name[:-1]:
			out += space + "namespace " + elem + " {\n"
			space += "	"
		
		out += space + " /**\n"
		if self.brief != "":
			out += space + " * @brief " + self.brief + " \n"
		if self.version != "":
			out += space + " *     version:" + self.version + "\n"
		if self.api != "":
			out += space + " *     api:" + self.api + "\n"
		for elem in self.authors:
			out += space + " *     authors:" + elem + "\n"
		out += space + " */\n"
		out += space + "class " + self.name[-1] + " {\n"
		space += "	"
		out += space + "public:\n"
		space += "	"
		out += space + "/**\n"
		out += space + " * @brief Generic virtual destructor\n"
		out += space + " */\n"
		out += space + "virtual ~" + self.name[-1] + "() = default;\n"
		
		for elem in self.attributes:
			out += elem.generate_cpp(space, object)
		
		for elem in self.functions:
			out += elem.generate_cpp(space, object)
		
		space = space[:-2]
		out += space + "};\n"
		
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		return [filename, out]
	
	def generate_source(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		register_filename = filename + "register" + self.name[-1] + ".hpp";
		filename += self.name[-1] + ".cpp";
		out = ""
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name += self.name[-1];
		
		namespace = ""
		for elem in self.name[:-1]:
			namespace += elem + "::"
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp",".hpp") + ">\n"
		out += "#include <" + register_filename + ">\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <zeus/Buffer.hpp>\n"
		out += "#include <zeus/BufferData.hpp>\n"
		out += "#include <zeus/debug.hpp>\n"
		out += "#include <zeus/ParamType.hpp>\n"
		out += "#include <etk/stdTools.hpp>\n"
		out += "#include <zeus/AbstractFunction.hpp>\n"
		out += "#include <climits>\n"
		out += "#include <etk/os/FSNode.hpp>\n"
		out += "#include <zeus/BufferParameter.hpp>\n"
		out += "#include <zeus/WebServer.hpp>\n"
		out += "#include <zeus/Object.hpp>\n"
		out += "\n"
		
		# now gebnerate the get and set parameter object ...
		out += "namespace zeus {\n"
		out += "	template<> const zeus::ParamType& createType<ememory::SharedPtr<" + class_name + ">>() {\n"
		if object == True:
			out += "		static zeus::ParamType type(\"obj:" + class_name + "\", zeus::paramTypeObject, false, false);\n"
		else:
			out += "		static zeus::ParamType type(\"srv:" + class_name + "\", zeus::paramTypeService, false, false);\n"
		out += "		return type;\n"
		out += "	}\n"
		out += "	template<>\n"
		out += "	void BufferParameter::addParameter<ememory::SharedPtr<" + class_name + ">>(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _paramId, const ememory::SharedPtr<" + class_name + ">& _value) {\n"
		out += "		std::vector<uint8_t> data;\n"
		"""
		out += "		addType(data, createType<" + class_name + ">());\n"
		"""
		if object == True:
			out += "		addTypeObject(data, \"" + class_name + "\");\n"
		else:
			out += "		addTypeService(data, \"" + class_name + "\");\n"
		out += "		int32_t currentOffset = data.size();\n"
		out += "		int32_t startOffset = data.size();\n"
		out += "		data.resize(data.size()+4);\n"
		out += "		uint32_t fullId = 0;\n"
		# convert the object in a real System Object ....
		out += "		if (_iface != nullptr) {\n"
		out += "			ememory::SharedPtr<zeus::WebServer> _iface2 = _iface;\n"
		out += "			uint16_t id    = _iface2->getAddress();\n"
		out += "			uint16_t idObj = _iface2->getNewObjectId();\n"
		if object == True:
			out += "			ememory::SharedPtr<zeus::ObjectType<" + class_name + ">> obj = ememory::makeShared<zeus::ObjectType<" + class_name + ">>(_iface, idObj, _value);\n"
		else:
			out += "			ememory::SharedPtr<zeus::ServiceType<" + class_name + ">> obj = ememory::makeShared<zeus::ServiceType<" + class_name + ">>(_iface, idObj, _value);\n"
		
		out += "			" + namespace + "register" + self.name[-1] + "(*obj);\n"
		out += "			_iface2->addWebObj(obj);\n"
		out += "			fullId = (uint32_t(id)<<16)+idObj;\n"
		out += "		}\n"
		# return Object ID and interface adress
		out += "		memcpy(&data[currentOffset], &fullId, 4);\n"
		out += "		m_parameter.push_back(std::make_pair(startOffset,data));\n"
		out += "	}\n"
		
		out += "}\n"
		
		return [filename, out]
	
	def generate_register_header(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += "register" + self.name[-1] + ".hpp";
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name += self.name[-1];
		
		out = ""
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <etk/types.hpp>\n"
		if object == True:
			out += "#include <zeus/Object.hpp>\n"
		else:
			out += "#include <zeus/Service.hpp>\n"
		out += "#include <zeus/Client.hpp>\n"
		out += "#include <" + class_name.replace("::","/") + ".hpp>\n"
		out += "#include <string>\n"
		out += "#include <vector>\n"
		out += "\n"
		space = ""
		namespace = ""
		for elem in self.name[:-1]:
			out += space + "namespace " + elem + " {\n"
			namespace += elem + "::"
			space += "	"
		
		MACRO_BASE_NAME = ""
		for elem in self.name:
			MACRO_BASE_NAME += elem.upper() + "_"
		
		out += space + "\n"
		if object == True:
			out += space + "void register" + self.name[-1] + "(zeus::ObjectType<" + class_name + ">& _interface);\n"
		else:
			out += space + "void register" + self.name[-1] + "(zeus::ServiceType<" + class_name + ">& _interface);\n"
		out += space + "\n"
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		out += space + "\n"
		if object == False:
			out += space + "#define " + MACRO_BASE_NAME + "DECLARE(type) \\\n"
			out += space + "	ETK_EXPORT_API void SERVICE_IO_instanciate(uint32_t _transactionId, ememory::SharedPtr<zeus::WebServer>& _iface, uint32_t _destination) { \\\n"
			out += space + "		ememory::SharedPtr<type> tmp; \\\n"
			out += space + "		tmp = ememory::makeShared<type>(_destination>>16); \\\n"
			out += space + "		ememory::SharedPtr<" + class_name + "> tmp2 = tmp; \\\n"
			out += space + "		_iface->answerValue(_transactionId, _destination, uint32_t(_iface->getAddress())<<16, tmp2); \\\n"
			out += space + "	}\n"
			out += space + "\n"
			"""
			out += space + "#define " + MACRO_BASE_NAME + "DECLARE_FACTORY(type, factory) \\\n"
			out += space + "	ETK_EXPORT_API zeus::Service* SERVICE_IO_instanciate() { \\\n"
			out += space + "		return " + namespace + "create" + self.name[-1] + "<type>(factory); \\\n"
			out += space + "	}\n"
			"""
		
		
		return [filename, out]
	
	def generate_register_code(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += "register" + self.name[-1] + ".cpp";
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name += self.name[-1];
		
		out = ""
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp", ".hpp") + ">\n"
		out += "#include <zeus/debug.hpp>\n"
		out += "\n"
		space = ""
		function_name = ""
		for elem in self.name[:-1]:
			function_name += "" + elem + "::"
		function_name += "register" + self.name[-1];
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name += self.name[-1];
		if object == True:
			out += space + "void " + function_name + "(zeus::ObjectType<" + class_name + ">& _interface) {\n"
		else:
			out += space + "void " + function_name + "(zeus::ServiceType<" + class_name + ">& _interface) {\n"
		
		space += "	"
		
		out += space + 'ZEUS_INFO("===========================================================");\n';
		out += space + 'ZEUS_INFO("== Instanciate service: ' + self.name[-1] + '");\n';
		out += space + 'ZEUS_INFO("===========================================================");\n';
		#out += space + '_serviceInterface.propertyNameService.set("' + self.name[-1].lower() + '");\n'
		if self.brief != "":
			out += space + '_interface.setDescription("' + self.brief + '");\n';
		if self.version != "":
			out += space + '_interface.setVersion("' + self.version + '");\n';
		if self.api != "":
			out += space + '_interface.setType("' + self.api + '");\n';
		for elem in self.authors:
			out += space + '_interface.addAuthor("' + elem.split("<")[0] + '", "' + elem.split("<")[1].replace(">","") + '");\n';
		if    len(self.functions) != 0 \
		   or len(self.attributes) != 0:
			out += space + "zeus::AbstractFunction* func = nullptr;\n"
		for elem in self.attributes:
			out += space + 'func = _interface.advertise("' + elem.name + '.set", &' + class_name + '::set' + capital_first(elem.name) + ');\n'
			out += space + 'if (func != nullptr) {\n'
			if elem.brief != "":
				out += space + '	func->setDescription("Set parameter ' + elem.brief + '");\n'
			out += space + '}\n'
			out += space + 'func = _interface.advertise("' + elem.name + '.get", &' + class_name + '::get' + capital_first(elem.name) + ');\n'
			out += space + 'if (func != nullptr) {\n'
			if elem.brief != "":
				out += space + '	func->setDescription("Get parameter ' + elem.brief + '");\n'
			out += space + '}\n'
		for elem in self.functions:
			out += space + 'func = _interface.advertise("' + elem.name + '", &' + class_name + '::' + elem.name + ');\n'
			out += space + 'if (func != nullptr) {\n'
			space += "	"
			if elem.brief != "":
				out += space + 'func->setDescription("' + elem.brief + '");\n'
			for elem_p in elem.parameters:
				if     elem_p["name"] == "" \
				   and elem_p["brief"] == "":
					continue
				out += space + 'func->addParam("'
				if elem_p["name"] != "":
					out += elem_p["name"]
				out += '", "'
				if elem_p["brief"] != "":
					out += elem_p["brief"]
				out += '");\n'
			if elem.return_brief != "":
				out += space + 'func->setReturn("' + elem.return_brief + '");\n'
			space = space[:-1]
			out += space + '}\n'
		out += space + 'ZEUS_INFO("===========================================================");\n';
		out += space + 'ZEUS_INFO("== Instanciate service: ' + self.name[-1] + ' [DONE]");\n';
		out += space + 'ZEUS_INFO("===========================================================");\n';
		
		out += "}\n"
		out += "\n"
		return [filename, out]
	
	def generate_proxy_header(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += "Proxy" + self.name[-1] + ".hpp";
		out = ""
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <zeus/ServiceRemote.hpp>\n"
		out += "#include <zeus/BaseProxy.hpp>\n"
		out += "#include <string>\n"
		out += "#include <vector>\n"
		out += "\n"
		space = ""
		for elem in self.name[:-1]:
			out += space + "namespace " + elem + " {\n"
			space += "	"
		
		out += space + " /**\n"
		if self.brief != "":
			out += space + " * @brief " + self.brief + " \n"
		if self.version != "":
			out += space + " *     version:" + self.version + "\n"
		if self.api != "":
			out += space + " *     api:" + self.api + "\n"
		for elem in self.authors:
			out += space + " *     authors:" + elem + "\n"
		out += space + " */\n"
		#out += space + "class Proxy" + self.name[-1] + " : public " + self.name[-1] + " {\n"
		out += space + "class Proxy" + self.name[-1] + " :public zeus::BaseProxy {\n"
		space += "	"
		out += space + "public:\n"
		out += space + "	const Proxy" + self.name[-1] + "& operator= (const zeus::ServiceRemote& _srv) {\n"
		out += space + "		m_obj = _srv;\n"
		out += space + "		return *this;\n"
		out += space + "	}\n"
		out += space + "	~Proxy" + self.name[-1] + "() = default;\n"
		out += space + "	Proxy" + self.name[-1] + "() = default;\n"
		out += space + "	Proxy" + self.name[-1] + "(const zeus::ServiceRemote& _srv) :\n"
		out += space + "	  zeus::BaseProxy(_srv) {\n"
		out += space + "		\n"
		out += space + "	}\n"
		"""
		out += space + "	bool exist() const {\n"
		out += space + "		return m_obj.exist();\n"
		out += space + "	}\n"
		"""
		out += space + "public:\n"
		space += "	"
		"""
		out += space + "/**\n"
		out += space + " * @brief Generic virtual destructor\n"
		out += space + " */\n"
		out += space + "virtual ~" + self.name[-1] + "() = default;\n"
		"""
		for elem in self.attributes:
			out += elem.generate_hpp_proxy(space, object)
		for elem in self.functions:
			out += elem.generate_hpp_proxy(space, object)
		
		space = space[:-2]
		out += space + "};\n"
		
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		return [filename, out]
	
	def generate_proxy_code(self, object):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += "Proxy" + self.name[-1] + ".cpp";
		out = ""
		
		proxy_class_name = ""
		class_name = ""
		for elem in self.name[:-1]:
			proxy_class_name += "" + elem + "::"
			class_name += "" + elem + "::"
		proxy_class_name += "Proxy" + self.name[-1];
		class_name += self.name[-1];
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp",".hpp") + ">\n"
		out += "\n"
		
		for elem in self.attributes:
			out += elem.generate_cpp_proxy("", proxy_class_name, object)
		
		for elem in self.functions:
			out += elem.generate_cpp_proxy("", proxy_class_name, object)
		
		return [filename, out]
	


def tool_generate_idl(target, module, data_option):
	data_path = data_option["path"]
	debug.debug("Parsing .zeus.idl [start] " + str(data_path))
	name_file = os.path.basename(data_path)
	if     len(name_file) < 9 \
	   and name_file[-9:] != ".zeus.idl":
		debug.error("IDL must have an extention ended with '.zeus.idl' and not with '" + name_file[-9:] + "'")
	elem_name = ""
	if     len(name_file) >= 13 \
	   and name_file[-13:] == ".srv.zeus.idl":
		elem_name = name_file[:-13]
	elif     len(name_file) >= 16 \
	     and name_file[-16:] == ".struct.zeus.idl":
		elem_name = name_file[:-16]
	elif     len(name_file) >= 13 \
	     and name_file[-13:] == ".obj.zeus.idl":
		elem_name = name_file[:-13]
	else:
		debug.error("IDL must have an extention ended with '(struct|obj|srv).zeus.idl' and not with '" + name_file + "'")
	
	service_def = ServiceDefinition()
	service_def.set_name(elem_name.split("-"))
	data = tools.file_read_data(os.path.join(module.get_origin_path(), data_path))
	if len(data) == 0:
		debug.error("Can not parse zeus.idl ==> no data in the file, or no file : " + os.path.join(module.get_origin_path(), data_path))
		return;
	# standardise windows/Mac file in Linux file.
	data = data.replace("\r\n", "\n")
	data = data.replace("\r", "\n")
	id_line = 0
	multi_comment = False
	current_def = FunctionDefinition()
	current_attr = AttributeDefinition()
	for line in data.split("\n"):
		id_line += 1;
		if len(line) == 0:
			# empty line
			debug.extreme_verbose("find line " + str(id_line) + " ==> empty line")
			continue
		if multi_comment == False:
			if     len(line) >= 2 \
			   and line[:2] == "/*":
				# Comment multi-line
				debug.extreme_verbose("find line " + str(id_line) + " ==> comment multi-line [START]")
				if len(line) > 2:
					debug.error("line " + str(id_line) + " ==> /* must be alone in the line (no text after)")
				multi_comment = True
				continue
			if     len(line) >= 2 \
			   and line[:2] == "*/":
				debug.error("line " + str(id_line) + " ==> find '*/' Without a start multiline-comment '/*'")
		else:
			if     len(line) >= 2 \
			   and line[:2] == "*/":
				# Comment multi-line
				debug.extreme_verbose("find line " + str(id_line) + " ==> comment multi-line [STOP]")
				multi_comment = False
				if len(line) > 2:
					debug.error("line " + str(id_line) + " ==> find '/*' must be alone in the line (no text after)")
				continue
			continue
		if     len(line) >= 2 \
		   and line[:2] == "//":
			# Comment line
			debug.extreme_verbose("find line " + str(id_line) + " ==> comment line")
			continue
		if    len(line) >= 1 \
		   and line[0] == "#":
			# Documentation line
			debug.extreme_verbose("find line " + str(id_line) + " ==> documentation line")
			#get keyword:
			list_elems = line.split(":")
			if len(list_elems) < 1:
				debug.error("line " + str(id_line) + " ==> Missing Keyword ... ");
			doc_keyword = list_elems[0] + ":"
			doc_data = line[len(doc_keyword):]
			if doc_keyword == "#brief:":
				debug.extreme_verbose("    BRIEF: '" + doc_data + "'")
				current_def = FunctionDefinition()
				current_def.set_brief(doc_data)
				current_attr.set_brief(doc_data)
			elif doc_keyword == "#param:":
				debug.extreme_verbose("    PARAMETER: '" + doc_data + "'")
				# TODO : Do it better ...
				current_def.add_param_comment(doc_data.split(":")[0], doc_data.split(":")[1])
			elif doc_keyword == "#return:":
				debug.extreme_verbose("    RETURN: '" + doc_data + "'")
				current_def.set_return_comment(doc_data)
			elif doc_keyword == "#elem-brief:":
				debug.extreme_verbose("    SRV-BRIEF: '" + doc_data + "'")
				service_def.set_brief(doc_data)
			elif doc_keyword == "#elem-version:":
				debug.extreme_verbose("    SRV-VERSION: '" + doc_data + "'")
				service_def.set_version(doc_data)
			elif doc_keyword == "#elem-type:":
				debug.extreme_verbose("    SRV-TYPE: '" + doc_data + "'")
				service_def.set_api(doc_data)
			elif doc_keyword == "#elem-author:":
				debug.extreme_verbose("    SRV-AUTHOR: '" + doc_data + "'")
				service_def.add_author(doc_data)
			else:
				debug.warning("line " + str(id_line) + " ==> Unknow: keyword: '" + doc_keyword + "'")
				debug.error("        support only: '#brief:' '#param:' '#return:' '#elem-brief:' '#elem-version:' '#elem-type:' '#elem-author:'")
			continue
		debug.extreme_verbose("Need to parse the fucntion/attribute line:")
		debug.extreme_verbose("    '" + line + "'")
		if line[-1] == ")":
			# Find a fundtion ==> parse it
			#debug.error("line " + str(id_line) + " Can not parse function the line dos not ended by a ')'")
			#get first part (befor '('):
			list_elems = line.split("(")
			if len(list_elems) <= 1:
				debug.error("line " + str(id_line) + " function parsing error missing the '(' element")
			fist_part = list_elems[0].replace("   ", " ").replace("  ", " ").replace("  ", " ")
			argument_list = list_elems[1].replace("   ", "").replace("  ", "").replace(" ", "")[:-1]
			if len(argument_list) != 0:
				argument_list = argument_list.split(",")
			else:
				argument_list = []
			# separate the 
			list_elems = fist_part.split(" ")
			if len(list_elems) <= 1:
				debug.error("line " + str(id_line) + " fucntion return and name is nt parsable")
			return_value = list_elems[0]
			function_name = list_elems[1]
			# check types:
			if return_value not in get_list_type():
				debug.error("line " + str(id_line) + " fucntion return type unknow : '" + return_value + "' not in " + str(get_list_type()))
			for elem in argument_list:
				if elem not in get_list_type():
					debug.error("line " + str(id_line) + " fucntion argument type unknow : '" + elem + "' not in " + str(get_list_type()))
			debug.extreme_verbose("        Parse of function done :")
			debug.extreme_verbose("            return:" + return_value)
			debug.extreme_verbose("            name:" + function_name)
			debug.extreme_verbose("            arguments:" + str(argument_list))
			current_def.set_function_name(function_name)
			current_def.set_return_type(return_value)
			for elem in argument_list:
				current_def.add_parameter_type(elem)
			service_def.add_function(current_def)
		else:
			# if must be a simple element separate with a space
			if len(line.split("(")) != 1:
				debug.error("line " + str(id_line) + " Can not parse function the line does not ended by a ')'")
			elem = line.split(" ")
			if len(elem) != 2:
				debug.error("line " + str(id_line) + " Can not parse attribute must be constituated with the type and the name")
			if elem[0] not in get_list_type():
				debug.error("line " + str(id_line) + " Attribute type unknow : '" + elem[0] + "' not in " + str(get_list_type()))
			current_attr.set_type(elem[0]);
			current_attr.set_name(elem[1]);
			service_def.add_attribute(current_attr)
		# reset it ...
		current_def = FunctionDefinition()
		current_attr = AttributeDefinition()
	if multi_comment == True:
		debug.error("reach end of file and missing end of multi-line comment */")
	debug.verbose("Parsing idl Done (no error ...)")
	
	#service_def.display()
	
	service_header = service_def.generate_header(data_option["type"] == "object")
	service_source = service_def.generate_source(data_option["type"] == "object")
	register_header = service_def.generate_register_header(data_option["type"] == "object")
	register_code = service_def.generate_register_code(data_option["type"] == "object")
	proxy_header = service_def.generate_proxy_header(data_option["type"] == "object")
	proxy_code = service_def.generate_proxy_code(data_option["type"] == "object")
	
	debug.verbose("-----------------  " + service_header[0] + "  -----------------")
	debug.verbose("\n" + service_header[1])
	debug.verbose("-----------------  " + service_source[0] + "  -----------------")
	debug.verbose("\n" + service_source[1])
	debug.verbose("-----------------  " + register_header[0] + "  -----------------")
	debug.verbose("\n" + register_header[1])
	debug.verbose("-----------------  " + register_code[0] + "  -----------------")
	debug.verbose("\n" + register_code[1])
	debug.verbose("-----------------  " + proxy_header[0] + "  -----------------")
	debug.verbose("\n" + proxy_header[1])
	debug.verbose("-----------------  " + proxy_code[0] + "  -----------------")
	debug.verbose("\n" + proxy_code[1])
	
	tmp_path = os.path.join(target.get_build_path_temporary_generate(module.get_name()), "idl_src")
	module.add_generated_header_file(service_header[1], service_header[0], install_element=True)
	module.add_generated_src_file(service_source[1], service_source[0])
	module.add_generated_header_file(register_header[1], register_header[0], install_element=True)
	module.add_generated_src_file(register_code[1], register_code[0])
	module.add_generated_header_file(proxy_header[1], proxy_header[0], install_element=True)
	module.add_generated_src_file(proxy_code[1], proxy_code[0])
	
	debug.debug("Parsing .zeus.idl [DONE]")


def parse_service_idl(module, idl_path):
	module.add_action(tool_generate_idl, data={"path":idl_path, "type":"service"})

def parse_object_idl(module, idl_path):
	module.add_action(tool_generate_idl, data={"path":idl_path, "type":"object"})

def parse_struct_idl(module, idl_path):
	module.add_action(tool_generate_idl, data={"path":idl_path, "type":"struct"})



