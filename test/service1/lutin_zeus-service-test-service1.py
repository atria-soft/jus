#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import lutin.macro as macro


def get_type():
	return "LIBRARY"
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
	my_module.add_depend([
	    'zeus'
	    ])
	
	zeus_macro = macro.load_macro('zeus')
	zeus_macro.parse_object_idl(my_module, 'appl/zeus-service-test-service1.srv.zeus.idl')
	
	my_module.add_flag('c++', "-DSERVICE_NAME=\"\\\"test-service1\\\"\"")
	return True



