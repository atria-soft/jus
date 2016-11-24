#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import os


def get_type():
	return "LIBRARY_DYNAMIC"
	#return "BINARY"

def get_sub_type():
	return "TOOLS"

def get_desc():
	return "ZEUS service user"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

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

class FunctionDefinition:
	def __init__(self):
		self.name = "";
		self.brief = "";
		self.return_type = "";
		self.return_brief = "";
		self.parameters = []
	
	def set_function_name(self, name):
		self.name = name;
	
	def set_brief(self, desc):
		self.name = "";
		self.brief = desc;
		self.return_type = "";
		self.return_brief = "";
		self.parameters = []
	
	def add_param_comment(self, name, desc):
		for elem in self.parameters:
			if     elem["name"] == "" \
			   and elem["brief"] == "":
				elem["name"] = name;
				elem["brief"] = desc;
				return;
		self.parameters.append({
		    "type":"",
		    "name":name,
		    "brief":desc
		    })
	
	def set_return_comment(self, desc):
		self.return_brief = desc;
	
	def set_return_type(self, type):
		self.return_type = type;
	
	def add_parameter_type(self, type):
		for elem in self.parameters:
			if elem["type"] == "":
				elem["type"] = type;
				return;
		self.parameters.append({
		    "type":type,
		    "name":"",
		    "brief":""
		    })
		
	def display(self):
		debug.info("   BRIEF: " + self.brief)
		debug.info("   BRIEF-return: " + self.return_brief)
		debug.info("   " + self.return_type + " " + self.name + "(")
		for elem in self.parameters:
			debug.info("           " + elem["type"] + " " + elem["name"] + ", # " + elem["brief"])
		debug.info("   )")
	
	def generate_cpp(self, space):
		out = "";
		# generate doxygen comment:
		out += space + "/**\n"
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
		
		out += space + "virtual "
		out += convert_type_in_cpp(self.return_type) + " " + self.name + "("
		param_data = ""
		for elem in self.parameters:
			if len(param_data) != 0:
				param_data += ", "
			param_data += convert_type_in_cpp(elem["type"]) + " _" + elem["name"]
		out += param_data
		out += ") = 0;\n"
		return out;

def parse_service_idl(path):
	debug.info("Parsing .zeus.idl [start] " + str(path))
	name_file = os.path.basename(path)
	if len(name_file) < 9 \
	   and name_file[-9:] != ".zeus.idl":
		debug.error("IDL must have an extention ended with '.zeus.idl' and not with '" + name_file[-9:] + "'")
	
	# TODO : Get from filename the namespace and the service name
	service_base = name_file[:-9].split("-")
	data = tools.file_read_data(os.path.join(os.path.dirname(__file__), path))
	if len(data) == 0:
		debug.error("Can not parse zeus.idl ==> no data in the file, or no file.")
		return;
	# standardise windows/Mac file in Linux file.
	data = data.replace("\r\n", "\n")
	data = data.replace("\r", "\n")
	id_line = 0
	multi_comment = False
	current_def = FunctionDefinition()
	list_all_function = []
	for line in data.split("\n"):
		id_line += 1;
		if len(line) == 0:
			# empty line
			debug.info("find line " + str(id_line) + " ==> empty line")
			continue
		if multi_comment == False:
			if     len(line) >= 2 \
			   and line[:2] == "/*":
				# Comment multi-line
				debug.info("find line " + str(id_line) + " ==> comment multi-line [START]")
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
				debug.info("find line " + str(id_line) + " ==> comment multi-line [STOP]")
				multi_comment = False
				if len(line) > 2:
					debug.error("line " + str(id_line) + " ==> find '/*' must be alone in the line (no text after)")
				continue
			continue
		if     len(line) >= 2 \
		   and line[:2] == "//":
			# Comment line
			debug.info("find line " + str(id_line) + " ==> comment line")
			continue
		if    len(line) >= 1 \
		   and line[0] == "#":
			# Documentation line
			debug.info("find line " + str(id_line) + " ==> documentation line")
			#get keyword:
			list_elems = line.split(":")
			if len(list_elems) < 1:
				debug.error("line " + str(id_line) + " ==> Missing Keyword ... ");
			doc_keyword = list_elems[0] + ":"
			doc_data = line[len(doc_keyword):]
			if doc_keyword == "#brief:":
				debug.info("    BRIEF: '" + doc_data + "'")
				current_def = FunctionDefinition()
				current_def.set_brief(doc_data)
			elif doc_keyword == "#param:":
				debug.info("    PARAMETER: '" + doc_data + "'")
				# TODO : Do it better ...
				current_def.add_param_comment(doc_data.split(":")[0], doc_data.split(":")[1])
			elif doc_keyword == "#return:":
				debug.info("    RETURN: '" + doc_data + "'")
				current_def.set_return_comment(doc_data)
			elif doc_keyword == "#srv-brief:":
				debug.info("    SRV-BRIEF: '" + doc_data + "'")
				# TODO: ...
			elif doc_keyword == "#srv-version:":
				debug.info("    SRV-VERSION: '" + doc_data + "'")
				# TODO: ...
			elif doc_keyword == "#srv-type-api:":
				debug.info("    SRV-TYPE-API: '" + doc_data + "'")
				# TODO: ...
			elif doc_keyword == "#srv-author:":
				debug.info("    SRV-AUTHOR: '" + doc_data + "'")
				# TODO: ...
			else:
				debug.warning("line " + str(id_line) + " ==> Unknow: keyword: '" + doc_keyword + "'")
				debug.error("        support only: '#brief:' '#param:' '#return:' '#srv-brief:' '#srv-version:' '#srv-type-api:' '#srv-author:'")
			continue
		debug.info("Need to parse the fucntion line:")
		debug.info("    '" + line + "'")
		if True:
			if line[-1] != ")":
				debug.error("line " + str(id_line) + " Can not parse function the line dos not ended by a ')'")
			#get first part (befor '('):
			list_elems = line.split("(")
			if len(list_elems) <= 1:
				debug.error("line " + str(id_line) + " fucntion parsing error missing the '(' element")
			fist_part = list_elems[0].replace("   ", " ").replace("  ", " ").replace("  ", " ")
			argument_list = list_elems[1].replace("   ", "").replace("  ", "").replace(" ", "")[:-1].split(",")
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
			debug.info("        Parse of function done :")
			debug.info("            return:" + return_value)
			debug.info("            name:" + function_name)
			debug.info("            arguments:" + str(argument_list))
			current_def.set_function_name(function_name)
			current_def.set_return_type(return_value)
			for elem in argument_list:
				current_def.add_parameter_type(elem)
			list_all_function.append(current_def)
			current_def = FunctionDefinition()
	if multi_comment == True:
		debug.error("reach end of file and missing end of multi-line comment */")
	debug.warning("Parsing Done")
	for elem in list_all_function:
		elem.display();
	debug.warning("Display Done")
	cpp_header = ""
	# TODO: add global header:
	cpp_header += "/** @file\n"
	cpp_header += " * @note Generated file !!! Do not modify !!!\n"
	cpp_header += " */\n"
	cpp_header += "#pragma once"
	cpp_header += "\n"
	space = ""
	for elem in service_base[:-1]:
		cpp_header += space + "namespace " + elem + " {\n"
		space += "	"
	
	cpp_header += space + "class " + service_base[-1] + " {\n"
	space += "	"
	cpp_header += space + "public:\n"
	space += "	"
	
	for elem in list_all_function:
		cpp_header += elem.generate_cpp(space)
	
	space = space[:-2]
	cpp_header += space + "};\n"
	
	for elem in service_base[:-1]:
		space = space[:-1]
		cpp_header += space + "}\n"
	debug.info(cpp_header)
	debug.warning("Generate C++ Done")
	
	debug.error("Parsing .zeus.idl [DONE]")

def configure(target, my_module):
	my_module.add_path(".")
	my_module.add_depend([
	    'zeus',
	    'ejson'
	    ])
	my_module.add_src_file([
	    'appl/debug.cpp',
	    'appl/main.cpp'
	    ])
	
	parse_service_idl('appl/service-user.zeus.idl')
	
	my_module.add_flag('c++', "-DSERVICE_NAME=\"\\\"" + my_module.get_name()[13:] + "\\\"\"")
	
	return True



