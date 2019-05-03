#!/usr/bin/python
import realog.debug as debug
import lutin.tools as tools


def get_type():
	return "LIBRARY_DYNAMIC"
	#return "BINARY"

def get_sub_type():
	return "TOOLS"

def get_desc():
	return "ZEUS test service1"

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def configure(target, my_module):
	my_module.add_path(".")
	my_module.add_depend([
	    'zeus',
	    'ejson',
	    'zeus-service-test-service1'
	    ])
	my_module.add_src_file([
	    'appl/debug.cpp',
	    'appl/main-service-test-service1.cpp'
	    ])
	my_module.add_flag('c++', "-DSERVICE_NAME=\"\\\"test-service1\\\"\"")
	return True



