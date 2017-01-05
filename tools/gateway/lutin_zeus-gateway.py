#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools


def get_type():
	return "BINARY"

def get_sub_type():
	return "TOOLS"

def get_desc():
	return "ZEUS generic gateway"

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
	my_module.add_depend(['zeus'])
	my_module.add_src_file([
	    'appl/debug.cpp',
	    'appl/IOInterface.cpp',
	    'appl/RouterInterface.cpp',
	    #'appl/ServiceInterface.cpp',
	    'appl/DirectInterface.cpp',
	    'appl/GateWay.cpp',
	    'appl/main-gateway.cpp'
	    ])
	return True



