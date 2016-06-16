#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "BINARY"

def get_sub_type():
	return "TOOLS"

def get_desc():
	return "JUS test service"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_export_path(tools.get_current_path(__file__))
	my_module.add_module_depend(['jus', 'ejson'])
	my_module.add_src_file([
	    'appl/debug.cpp',
	    'appl/main.cpp'
	    ])
	my_module.add_export_flag('c++', "-DSERVICE_NAME=\"\\\"" + module_name[4:] + "\\\"\"")
	return my_module



