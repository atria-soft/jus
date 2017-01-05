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
    ["raw", "zeus::Raw"],
    ]


def get_list_type():
	out = []
	for elem in list_of_known_type:
		out.append(elem[0])
	return out

def validate_type(data):
	if data in get_list_type():
		return True
	val = data.split(":")
	if val[0] == "obj":
		return True
	return False

def convert_type_in_cpp(data, proxy=False, argument=False):
	for elem in list_of_known_type:
		if data == elem[0]:
			return elem[1]
	val = data.split(":")
	if val[0] == "obj":
		# this is a generated object:
		listElem = val[1].split("-")
		out = ""
		for elem in listElem[:-1]:
			out += elem + "::"
		if proxy == True:
			if argument == False:
				out += "Proxy" + listElem[-1]
			else:
				out = "ememory::SharedPtr<" + out + listElem[-1] + ">"
		else:
			if argument == True:
				out += "Proxy" + listElem[-1]
			else:
				out = "ememory::SharedPtr<" + out + listElem[-1] + ">"
		return out
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
		self.brief = remove_start_stop_spacer(desc).replace("\"", "\\\"")
		self.type = "";
	
	def set_type(self, type):
		self.type = remove_start_stop_spacer(type);
	
	def display(self):
		debug.info("       BRIEF: " + self.brief)
		debug.info("       " + self.type + " " + self.name + ";")
	
	def generate_cpp(self, space):
		out = "";
		out += space + "eproperty::Value<" + convert_type_in_cpp(self.type) + "> " + self.name + "; //!<" + self.brief + "\n"
		out += space + "//! Internal interface to call property\n"
		out += space + "virtual " + convert_type_in_cpp(self.type) + " _internalWrapperProperty_get" + capital_first(self.name) + "() {\n"
		out += space + "	return " + self.name + ".get();\n"
		out += space + "}\n"
		out += space + "//! Internal interface to call property\n"
		out += space + "virtual void _internalWrapperProperty_set" + capital_first(self.name) + "(" + convert_type_in_cpp(self.type) + " _value) {\n"
		out += space + "	" + self.name + ".set(_value);\n"
		out += space + "}\n"
		return out;
		
	def generate_hpp_proxy(self, space):
		out = "";
		out += space + "zeus::RemoteProperty<" + convert_type_in_cpp(self.type) + "> " + self.name + "; //!<" + self.brief + "\n"
		return out;
	
	def generate_cpp_proxy(self, space, class_name):
		out = "";
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
		self.brief = remove_start_stop_spacer(desc).replace("\"", "\\\"")
		self.return_type = "";
		self.return_brief = "";
		self.parameters = []
	
	def add_param_comment(self, name, desc):
		for elem in self.parameters:
			if     elem["name"] == "" \
			   and elem["brief"] == "":
				elem["name"] = remove_start_stop_spacer(name)
				elem["brief"] = remove_start_stop_spacer(desc).replace("\"", "\\\"")
				return;
		self.parameters.append({
		    "type":"",
		    "name":remove_start_stop_spacer(name),
		    "brief":remove_start_stop_spacer(desc)
		    })
	
	def set_return_comment(self, desc):
		self.return_brief = remove_start_stop_spacer(desc)
	
	def set_return_type(self, type):
		self.return_type = remove_start_stop_spacer(type)
	
	def add_parameter_type(self, type):
		for elem in self.parameters:
			if elem["type"] == "":
				elem["type"] = remove_start_stop_spacer(type)
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
	
	def generate_doxy(self, space):
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
	
	def generate_cpp(self, space, class_name="", virtual=True):
		out = "";
		out += self.generate_doxy(space)
		out += space
		if self.return_type != "":
			if virtual == True:
				out += "virtual "
			out += convert_type_in_cpp(self.return_type, False, False) + " "
		else:
			out += "static ememory::SharedPtr<" + class_name + "> "
		out += self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += convert_type_in_cpp(elem["type"], False, True) + " _"
			if elem["name"] == "":
				param_data += "no_name_param_" + str(id_parameter)
			else:
				param_data += elem["name"]
		out += param_data
		out += ")"
		if     self.return_type != "" \
		   and virtual == True:
			out += " = 0"
		out += ";\n"
		return out;
		
	def generate_hpp_proxy(self, space):
		out = "";
		out += self.generate_doxy(space)
		out += space + "virtual zeus::Future<" + convert_type_in_cpp(self.return_type, True, False) + "> " + self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += "const " + convert_type_in_cpp(elem["type"], True, True) + "& _"
			if elem["name"] == "":
				param_data += "no_name_param_" + str(id_parameter)
			else:
				param_data += elem["name"]
		out += param_data
		out += ");\n"
		return out;
	def generate_cpp_proxy(self, space, class_name):
		out = "";
		out += space + "zeus::Future<" + convert_type_in_cpp(self.return_type, True, False) + "> " + class_name + "::" + self.name + "("
		param_data = ""
		id_parameter = 0
		for elem in self.parameters:
			id_parameter += 1
			if len(param_data) != 0:
				param_data += ", "
			param_data += "const " + convert_type_in_cpp(elem["type"], True, True) + "& _"
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
		self.factories = []
		self.tools = []
		self.imports = []
	
	def set_name(self, value):
		self.name = value
		# TODO : Check range ...
		self.name[-1] = capital_first(self.name[-1])
	
	def set_brief(self, value):
		self.brief = remove_start_stop_spacer(value).replace("\"", "\\\"")
	
	def set_version(self, value):
		self.version = remove_start_stop_spacer(value)
	
	def set_api(self, value):
		self.api = remove_start_stop_spacer(value)
	
	def add_author(self, value):
		self.authors.append(remove_start_stop_spacer(value).replace("\"", "\\\""))
	
	def add_factory(self, value):
		# TODO : Check if function already exist
		self.factories.append(value)
	
	def add_tool(self, value):
		# TODO : Check if function already exist
		self.tools.append(value)
	
	def add_function(self, value):
		# TODO : Check if function already exist
		self.functions.append(value)
	
	def add_attribute(self, value):
		# TODO : Check if attribute already exist
		self.attributes.append(value)
	
	def add_import(self, value):
		self.imports.append(value)
	
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
	
	def generate_header(self):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += self.name[-1] + ".hpp";
		out = ""
		# TODO: add global header:
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license MPL-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <eproperty/Value.hpp>\n"
		out += "#include <zeus/Raw.hpp>\n"
		out += "#include <string>\n"
		out += "#include <vector>\n"
		out += "#include <ememory/memory.hpp>\n"
		for elem in self.imports:
			out += "#include <" + elem.replace("-","/") + ".hpp>\n"
			out += "#include <" + elem.replace("-","/Proxy") + ".hpp>\n"
		out += "\n"
		space = ""
		class_name = ""
		for elem in self.name[:-1]:
			out += space + "namespace " + elem + " {\n"
			space += "	"
			class_name += elem + "::"
		class_name += self.name[-1]
		
		out += space + "class Proxy" + self.name[-1] + ";\n"
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
		if len(self.factories) == 0:
			out += space + "/**\n"
			out += space + " * @brief generic factory, pay attention when set arguments...\n"
			out += space + " */\n"
			out += space + "template<typename ... ZEUS_OBJECT_CREATE>\n"
			out += space + "static ememory::SharedPtr<" + class_name + "> create(ZEUS_OBJECT_CREATE ...);\n"
		else:
			for elem in self.factories:
				out += elem.generate_cpp(space, class_name)
		
		out += space + "/**\n"
		out += space + " * @brief Generic virtual destructor\n"
		out += space + " */\n"
		out += space + "virtual ~" + self.name[-1] + "() = default;\n"
		
		for elem in self.attributes:
			out += elem.generate_cpp(space)
		
		for elem in self.functions:
			out += elem.generate_cpp(space)
		
		space = space[:-2]
		out += space + "};\n"
		# now we simply add tools provided:
		for elem in self.tools:
			out += elem.generate_cpp(space, virtual=False)
		
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		return [filename, out]
	
	def generate_source(self):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		register_filename = filename + "register" + self.name[-1] + ".hpp";
		register_filename_proxy = filename + "Proxy" + self.name[-1] + ".hpp";
		filename += self.name[-1] + ".cpp";
		out = ""
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name_proxy = class_name + "Proxy" + self.name[-1];
		class_name += self.name[-1];
		
		namespace = ""
		for elem in self.name[:-1]:
			namespace += elem + "::"
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license MPL-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp",".hpp") + ">\n"
		out += "#include <" + register_filename + ">\n"
		out += "#include <" + register_filename_proxy + ">\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <zeus/debug.hpp>\n"
		out += "#include <zeus/message/Message.hpp>\n"
		out += "#include <zeus/message/Data.hpp>\n"
		out += "#include <zeus/message/ParamType.hpp>\n"
		out += "#include <zeus/message/Parameter.hpp>\n"
		out += "#include <etk/stdTools.hpp>\n"
		out += "#include <zeus/AbstractFunction.hpp>\n"
		out += "#include <climits>\n"
		out += "#include <etk/os/FSNode.hpp>\n"
		out += "#include <zeus/WebServer.hpp>\n"
		out += "#include <zeus/Object.hpp>\n"
		out += "\n"
		
		# now gebnerate the get and set parameter object ...
		out += "namespace zeus {\n"
		out += "	namespace message {\n"
		out += "		template<> const zeus::message::ParamType& createType<ememory::SharedPtr<" + class_name + ">>() {\n"
		out += "			static zeus::message::ParamType type(\"obj:" + class_name + "\", zeus::message::paramTypeObject, false, false);\n"
		out += "			return type;\n"
		out += "		}\n"
		out += "		\n"
		out += "		template<> const zeus::message::ParamType& createType<" + class_name_proxy + ">() {\n"
		out += "			static zeus::message::ParamType type(\"obj:" + class_name + "\", zeus::message::paramTypeObject, false, false);\n"
		out += "			return type;\n"
		out += "		}\n"
		out += "		\n"
		out += "		template<>\n"
		out += "		void Parameter::addParameter<ememory::SharedPtr<" + class_name + ">>(uint16_t _paramId, const ememory::SharedPtr<" + class_name + ">& _value) {\n"
		out += "			std::vector<uint8_t> data;\n"
		"""
		out += "			addType(data, createType<" + class_name + ">());\n"
		"""
		out += "			addTypeObject(data, \"obj:" + class_name + "\");\n"
		out += "			int32_t currentOffset = data.size();\n"
		out += "			int32_t startOffset = data.size();\n"
		out += "			data.resize(data.size()+4);\n"
		out += "			uint32_t fullId = 0;\n"
		# convert the object in a real System Object ....
		out += "			if (m_iface != nullptr) {\n"
		out += "				uint16_t id    = m_iface->getAddress();\n"
		out += "				uint16_t idObj = m_iface->getNewObjectId();\n"
		out += "				ememory::SharedPtr<zeus::ObjectType<" + class_name + ">> obj = ememory::makeShared<zeus::ObjectType<" + class_name + ">>(m_iface, idObj, _value);\n"
		out += "				" + namespace + "register" + self.name[-1] + "(*obj);\n"
		out += "				obj->addRemote(getDestination());\n"
		out += "				m_iface->addWebObj(obj);\n"
		out += "				ZEUS_DEBUG(\"Create object ID : \" << idObj);\n"
		out += "				fullId = (uint32_t(id)<<16)+idObj;\n"
		out += "			}\n"
		# return Object ID and interface adress
		out += "			memcpy(&data[currentOffset], &fullId, 4);\n"
		out += "			m_parameter.push_back(std::make_pair(startOffset,data));\n"
		out += "		}\n"
		out += "		\n"
		out += "		template<>\n"
		out += "		" + class_name_proxy + " Parameter::getParameter<" + class_name_proxy + ">(int32_t _id) const {\n"
		out += "			ememory::SharedPtr<zeus::ObjectRemoteBase> out;\n"
		out += "			out = zeus::message::Parameter::getParameter<ememory::SharedPtr<zeus::ObjectRemoteBase>>(_id);\n"
		out += "			return zeus::ObjectRemote(out);\n"
		out += "		}\n"
		out += "	}\n"
		out += "}\n"
		
		return [filename, out]
	
	def generate_register_header(self):
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
		out += " * @license MPL-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <zeus/Object.hpp>\n"
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
		out += space + "void register" + self.name[-1] + "(zeus::ObjectType<" + class_name + ">& _interface);\n"
		out += space + "\n"
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		out += space + "\n"
		out += space + "#define " + MACRO_BASE_NAME + "DECLARE(type) \\\n"
		out += space + "	ETK_EXPORT_API void SERVICE_IO_instanciate(uint32_t _transactionId, ememory::SharedPtr<zeus::WebServer>& _iface, uint32_t _destination) { \\\n"
		out += space + "		ememory::SharedPtr<type> tmp; \\\n"
		out += space + "		tmp = ememory::makeShared<type>(_destination>>16); \\\n"
		out += space + "		ememory::SharedPtr<" + class_name + "> tmp2 = tmp; \\\n"
		out += space + "		_iface->answerValue(_transactionId, uint32_t(_iface->getAddress())<<16, _destination, tmp2); \\\n"
		out += space + "	}\n"
		out += space + "\n"
		
		
		return [filename, out]
	
	def generate_register_code(self):
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
		out += " * @license MPL-2\n"
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
		out += space + "void " + function_name + "(zeus::ObjectType<" + class_name + ">& _interface) {\n"
		
		space += "	"
		
		out += space + 'ZEUS_VERBOSE("===========================================================");\n';
		out += space + 'ZEUS_VERBOSE("== Instanciate service: ' + self.name[-1] + '");\n';
		out += space + 'ZEUS_VERBOSE("===========================================================");\n';
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
			out += space + 'func = _interface.advertise("' + elem.name + '.set", &' + class_name + '::_internalWrapperProperty_set' + capital_first(elem.name) + ');\n'
			out += space + 'if (func != nullptr) {\n'
			if elem.brief != "":
				out += space + '	func->setDescription("Set parameter ' + elem.brief + '");\n'
			out += space + '}\n'
			out += space + 'func = _interface.advertise("' + elem.name + '.get", &' + class_name + '::_internalWrapperProperty_get' + capital_first(elem.name) + ');\n'
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
		out += space + 'ZEUS_VERBOSE("===========================================================");\n';
		out += space + 'ZEUS_VERBOSE("== Instanciate service: ' + self.name[-1] + ' [DONE]");\n';
		out += space + 'ZEUS_VERBOSE("===========================================================");\n';
		
		out += "}\n"
		out += "\n"
		return [filename, out]
	
	def generate_proxy_header(self):
		filename = ""
		for elem in self.name[:-1]:
			filename += elem + "/"
		filename += "Proxy" + self.name[-1] + ".hpp";
		out = ""
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license MPL-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <zeus/ObjectRemote.hpp>\n"
		out += "#include <zeus/Proxy.hpp>\n"
		out += "#include <zeus/RemoteProperty.hpp>\n"
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
		out += space + "class Proxy" + self.name[-1] + " :public zeus::Proxy {\n"
		space += "	"
		out += space + "public:\n"
		out += space + "	const Proxy" + self.name[-1] + "& operator= (const zeus::ObjectRemote& _srv) {\n"
		out += space + "		m_obj = _srv;\n"
		out += space + "		return *this;\n"
		out += space + "	}\n"
		out += space + "	~Proxy" + self.name[-1] + "() = default;\n"
		out += space + "	Proxy" + self.name[-1] + "()"
		if len(self.attributes) != 0:
			out += ": \n"
			first = True
			for elem in self.attributes:
				if first == False:
					out += ",\n"
				out += space + "	  " + elem.name + "(m_obj, \"" + elem.name + "\")"
				first = False
		out += " {}\n"
		
		out += space + "	Proxy" + self.name[-1] + "(const zeus::ObjectRemote& _srv) :\n"
		out += space + "	  zeus::Proxy(_srv)"
		for elem in self.attributes:
			out += ",\n"
			out += space + "	  " + elem.name + "(m_obj, \"" + elem.name + "\")"
			first = False
		out += " {\n"
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
			out += elem.generate_hpp_proxy(space)
		for elem in self.functions:
			out += elem.generate_hpp_proxy(space)
		
		space = space[:-2]
		out += space + "};\n"
		
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		return [filename, out]
	
	def generate_proxy_code(self):
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
		out += " * @license MPL-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp",".hpp") + ">\n"
		out += "\n"
		
		for elem in self.attributes:
			out += elem.generate_cpp_proxy("", proxy_class_name)
		
		for elem in self.functions:
			out += elem.generate_cpp_proxy("", proxy_class_name)
		
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
		if line[:7] == "import ":
			debug.debug("find import : " + line)
			# TODO : Add check ...
			service_def.add_import(line.split(" ")[1])
		elif line[-1] == ")":
			# Find a fundtion ==> parse it
			#debug.error("line " + str(id_line) + " Can not parse function the line dos not ended by a ')'")
			#get first part (befor '('):
			list_elems = line.replace("[tool-remote] ", "[tool-remote]").split("(")
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
			debug.extreme_verbose("        Parse of function done :")
			current_def.set_function_name(function_name)
			type_function = "normal"
			if return_value[:13] == "[tool-remote]":
				type_function = "tool"
				current_def.set_return_type(return_value[13:])
				debug.extreme_verbose("            return:" + return_value[13:])
				if validate_type(return_value[13:]) == False:
					debug.error("line " + str(id_line) + " fucntion return type unknow : '" + return_value + "' not in " + str(get_list_type()))
			elif return_value == "[factory]":
				type_function = "factory"
				if function_name != "create":
					debug.error("line " + str(id_line) + " factory function name must be 'create' not '" + function_name + "'")
				debug.extreme_verbose("            return: --- ")
			elif validate_type(return_value) == False:
				debug.error("line " + str(id_line) + " fucntion return type unknow : '" + return_value + "' not in " + str(get_list_type()))
			else:
				current_def.set_return_type(return_value)
				debug.extreme_verbose("            return:" + return_value)
			
			for elem in argument_list:
				if validate_type(elem) == False:
					debug.error("line " + str(id_line) + " fucntion argument type unknow : '" + elem + "' not in " + str(get_list_type()))
			debug.extreme_verbose("            name:" + function_name)
			debug.extreme_verbose("            arguments:" + str(argument_list))
			for elem in argument_list:
				current_def.add_parameter_type(elem)
			if type_function == "normal":
				service_def.add_function(current_def)
			elif type_function == "factory":
				service_def.add_factory(current_def)
			else:
				service_def.add_tool(current_def)
		else:
			# if must be a simple element separate with a space
			if len(line.split("(")) != 1:
				debug.error("line " + str(id_line) + " Can not parse function the line does not ended by a ')'")
			elem = line.split(" ")
			if len(elem) != 2:
				debug.error("line " + str(id_line) + " Can not parse attribute must be constituated with the type and the name")
			if validate_type(elem[0]) == False:
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
	
	service_header = service_def.generate_header()
	service_source = service_def.generate_source()
	register_header = service_def.generate_register_header()
	register_code = service_def.generate_register_code()
	proxy_header = service_def.generate_proxy_header()
	proxy_code = service_def.generate_proxy_code()
	
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

def parse_object_idl(module, idl_path):
	module.add_action(tool_generate_idl, data={"path":idl_path, "type":"object"})

def parse_struct_idl(module, idl_path):
	module.add_action(tool_generate_idl, data={"path":idl_path, "type":"struct"})



