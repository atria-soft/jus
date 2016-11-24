#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools


def get_type():
	return "PACKAGE"

def get_desc():
	return "ZEUS package group to set it usable"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def configure(target, my_module):
	my_module.add_depend([
	    'zeus-router',
	    'zeus-gateway',
	    'zeus-service-user-impl',
	    'zeus-service-picture-impl',
	    'zeus-service-video-impl',
	    'zeus-launcher',
	    ])
	return True



