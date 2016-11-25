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
	
	def generate_cpp(self, space):
		out = "";
		out += self.generate_doxy(space)
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
		
	def generate_hpp_proxy(self, space):
		out = "";
		out += self.generate_doxy(space)
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
	def generate_cpp_proxy(self, space, class_name):
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
		out += space + 'return m_srv.call("' + self.name + '"'
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
		self.functions = []
	
	def set_name(self, value):
		self.name = value
		# TODO : Check range ...
		self.name[-1] = self.name[-1].title()
	
	def set_brief(self, value):
		self.brief = remove_start_stop_spacer(value)
	
	def set_version(self, value):
		self.version = remove_start_stop_spacer(value)
	
	def set_api(self, value):
		self.api = remove_start_stop_spacer(value)
	
	def add_author(self, value):
		self.authors.append(remove_start_stop_spacer(value))
	
	def add_function(self, value):
		self.functions.append(value)
	
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
		
		for elem in self.functions:
			out += elem.generate_cpp(space)
		
		space = space[:-2]
		out += space + "};\n"
		
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
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
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "#pragma once\n"
		out += "\n"
		out += "#include <etk/types.hpp>\n"
		out += "#include <zeus/Service.hpp>\n"
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
		out += space + "void register" + self.name[-1] + "(zeus::ServiceType<" + class_name + ">& _serviceInterface);\n"
		out += space + "\n"
		out += space + "template<class " + MACRO_BASE_NAME + "TYPE>\n"
		out += space + "zeus::Service* create" + self.name[-1] + "(std::function<ememory::SharedPtr<" + MACRO_BASE_NAME + "TYPE>(ememory::SharedPtr<ClientProperty>, const std::string& _userName)> _factory) {\n"
		out += space + "	zeus::ServiceType<" + class_name + ">* tmp = nullptr;\n"
		out += space + "	tmp = new zeus::ServiceType<" + class_name + ">(_factory);\n"
		out += space + "	zeus::service::register" + self.name[-1] + "(*tmp);\n"
		out += space + "	return tmp;\n"
		out += space + "}\n"
		out += space + "\n"
		for elem in self.name[:-1]:
			space = space[:-1]
			out += space + "}\n"
		out += space + "\n"
		
		out += space + "#define " + MACRO_BASE_NAME + "DECLARE_DEFAULT(type) \\\n"
		out += space + "	ETK_EXPORT_API zeus::Service* SERVICE_IO_instanciate() { \\\n"
		out += space + "		return " + namespace + "create" + self.name[-1] + "<type>([](ememory::SharedPtr<zeus::ClientProperty> _client, const std::string& _userName){ \\\n"
		out += space + "		                                 	return ememory::makeShared<type>(_client, _userName); \\\n"
		out += space + "		                                 }); \\\n"
		out += space + "	}\n"
		out += space + "\n"
		out += space + "#define " + MACRO_BASE_NAME + "DECLARE(type, factory) \\\n"
		out += space + "	ETK_EXPORT_API zeus::Service* SERVICE_IO_instanciate() { \\\n"
		out += space + "		return " + namespace + "create" + self.name[-1] + "<type>(factory); \\\n"
		out += space + "	}\n"
		
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
		
		out += space + "void " + function_name + "(zeus::ServiceType<" + class_name + ">& _serviceInterface) {\n"
		space += "	"
		
		
		
		
		out += space + 'ZEUS_INFO("===========================================================");\n';
		out += space + 'ZEUS_INFO("== Instanciate service: ' + self.name[-1] + '");\n';
		out += space + 'ZEUS_INFO("===========================================================");\n';
		"""
		zeus::ServiceType<appl::SystemService> _serviceInterface([](ememory::SharedPtr<zeus::ClientProperty> _client){
		                                                        	return ememory::makeShared<appl::SystemService>(_client);
		                                                        });
		if (_ip != "") {
			_serviceInterface.propertyIp.set(_ip);
		}
		if (_port != 0) {
			_serviceInterface.propertyPort.set(_port);
		}
		"""
		out += space + '_serviceInterface.propertyNameService.set("' + self.name[-1].lower() + '");\n'
		if self.brief != "":
			out += space + '_serviceInterface.setDescription("' + self.brief + '");\n';
		if self.version != "":
			out += space + '_serviceInterface.setVersion("' + self.version + '");\n';
		if self.api != "":
			out += space + '_serviceInterface.setType("' + self.api + '");\n';
		for elem in self.authors:
			out += space + '_serviceInterface.addAuthor("' + elem.split("<")[0] + '", "' + elem.split("<")[1].replace(">","") + '");\n';
		if len(self.functions) != 0:
			out += space + "zeus::AbstractFunction* func = nullptr;\n"
		for elem in self.functions:
			out += space + 'func = _serviceInterface.advertise("' + elem.name + '", &' + class_name + '::' + elem.name + ');\n'
			out += space + 'if (func != nullptr) {\n'
			space += "	"
			if elem.brief != "":
				out += space + 'func->setDescription("' + elem.name + '");\n'
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
	
	def generate_proxy_header(self):
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
		out += space + "class Proxy" + self.name[-1] + " {\n"
		space += "	"
		out += space + "protected:\n"
		out += space + "	zeus::ServiceRemote m_srv; //!< Service instance handle\n"
		out += space + "public:\n"
		out += space + "	const Proxy" + self.name[-1] + "& operator= (const zeus::ServiceRemote& _srv) {\n"
		out += space + "		m_srv = _srv;\n"
		out += space + "		return *this;\n"
		out += space + "	}\n"
		out += space + "public:\n"
		space += "	"
		"""
		out += space + "/**\n"
		out += space + " * @brief Generic virtual destructor\n"
		out += space + " */\n"
		out += space + "virtual ~" + self.name[-1] + "() = default;\n"
		"""
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
		
		class_name = ""
		for elem in self.name[:-1]:
			class_name += "" + elem + "::"
		class_name += "Proxy" + self.name[-1];
		
		out += "/** @file\n"
		out += " * @note Generated file !!! Do not modify !!!\n"
		out += " * @license APACHE-2\n"
		out += " * @copyright none\n"
		out += " */\n"
		out += "\n"
		out += "#include <" + filename.replace(".cpp",".hpp") + ">\n"
		out += "\n"
		
		for elem in self.functions:
			out += elem.generate_cpp_proxy("", class_name)
		return [filename, out]
	


def tool_generate_idl(target, module, data_path):
	debug.debug("Parsing .zeus.idl [start] " + str(data_path))
	name_file = os.path.basename(data_path)
	if len(name_file) < 9 \
	   and name_file[-9:] != ".zeus.idl":
		debug.error("IDL must have an extention ended with '.zeus.idl' and not with '" + name_file[-9:] + "'")
	
	service_def = ServiceDefinition()
	service_def.set_name(name_file[:-9].split("-"))
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
			elif doc_keyword == "#param:":
				debug.extreme_verbose("    PARAMETER: '" + doc_data + "'")
				# TODO : Do it better ...
				current_def.add_param_comment(doc_data.split(":")[0], doc_data.split(":")[1])
			elif doc_keyword == "#return:":
				debug.extreme_verbose("    RETURN: '" + doc_data + "'")
				current_def.set_return_comment(doc_data)
			elif doc_keyword == "#srv-brief:":
				debug.extreme_verbose("    SRV-BRIEF: '" + doc_data + "'")
				service_def.set_brief(doc_data)
			elif doc_keyword == "#srv-version:":
				debug.extreme_verbose("    SRV-VERSION: '" + doc_data + "'")
				service_def.set_version(doc_data)
			elif doc_keyword == "#srv-type:":
				debug.extreme_verbose("    SRV-TYPE: '" + doc_data + "'")
				service_def.set_api(doc_data)
			elif doc_keyword == "#srv-author:":
				debug.extreme_verbose("    SRV-AUTHOR: '" + doc_data + "'")
				service_def.add_author(doc_data)
			else:
				debug.warning("line " + str(id_line) + " ==> Unknow: keyword: '" + doc_keyword + "'")
				debug.error("        support only: '#brief:' '#param:' '#return:' '#srv-brief:' '#srv-version:' '#srv-type:' '#srv-author:'")
			continue
		debug.extreme_verbose("Need to parse the fucntion line:")
		debug.extreme_verbose("    '" + line + "'")
		if True:
			if line[-1] != ")":
				debug.error("line " + str(id_line) + " Can not parse function the line dos not ended by a ')'")
			#get first part (befor '('):
			list_elems = line.split("(")
			if len(list_elems) <= 1:
				debug.error("line " + str(id_line) + " fucntion parsing error missing the '(' element")
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
			current_def = FunctionDefinition()
	if multi_comment == True:
		debug.error("reach end of file and missing end of multi-line comment */")
	debug.verbose("Parsing idl Done (no error ...)")
	
	#service_def.display()
	
	service_header = service_def.generate_header()
	register_header = service_def.generate_register_header()
	register_code = service_def.generate_register_code()
	proxy_header = service_def.generate_proxy_header()
	proxy_code = service_def.generate_proxy_code()
	
	debug.verbose("-----------------  " + service_header[0] + "  -----------------")
	debug.verbose("\n" + service_header[1])
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
	module.add_generated_header_file(register_header[1], register_header[0], install_element=True)
	module.add_generated_src_file(register_code[1], register_code[0])
	module.add_generated_header_file(proxy_header[1], proxy_header[0], install_element=True)
	module.add_generated_src_file(proxy_code[1], proxy_code[0])
	
	debug.debug("Parsing .zeus.idl [DONE]")


def parse_service_idl(module, idl_path):
	module.add_action(tool_generate_idl, data=idl_path)



