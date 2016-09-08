#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "Zeus ewol micro-service"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def get_version():
	return [0,2,"dev"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_depend(['etk', 'enet', 'ememory', 'eproperty'])
	my_module.add_src_file([
	    'zeus/debug.cpp'
	    ])
	my_module.add_path(tools.get_current_path(__file__))
	my_module.add_src_file([
	    'zeus/AbstractFunction.cpp',
	    'zeus/FutureBase.cpp',
	    'zeus/Future.cpp',
	    'zeus/File.cpp',
	    'zeus/Buffer.cpp',
	    'zeus/BufferParameter.cpp',
	    'zeus/BufferCall.cpp',
	    'zeus/BufferAnswer.cpp',
	    'zeus/BufferData.cpp',
	    'zeus/BufferEvent.cpp',
	    'zeus/BufferFlow.cpp',
	    'zeus/BufferParameter_addParameter.cpp',
	    'zeus/BufferParameter_getParameter.cpp',
	    'zeus/ParamType.cpp',
	    'zeus/Client.cpp',
	    'zeus/RemoteProcessCall.cpp',
	    'zeus/Service.cpp',
	    'zeus/ServiceRemote.cpp',
	    'zeus/WebServer.cpp',
	    'zeus/mineType.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/AbstractFunction.h',
	    'zeus/AbstractFunctionTypeDirect.h',
	    'zeus/AbstractFunctionTypeClass.h',
	    'zeus/FutureData.h',
	    'zeus/FutureBase.h',
	    'zeus/Future.h',
	    'zeus/File.h',
	    'zeus/Buffer.h',
	    'zeus/BufferParameter.h',
	    'zeus/BufferCall.h',
	    'zeus/BufferAnswer.h',
	    'zeus/BufferData.h',
	    'zeus/BufferEvent.h',
	    'zeus/BufferFlow.h',
	    'zeus/ParamType.h',
	    'zeus/debug.h',
	    'zeus/Client.h',
	    'zeus/RemoteProcessCall.h',
	    'zeus/Service.h',
	    'zeus/ServiceRemote.h',
	    'zeus/WebServer.h',
	    'zeus/mineType.h',
	    ])
	if target.config["compilator"] == "clang":
		my_module.add_export_flag('c++', "-Wno-unsequenced")
	# build in C++ mode
	my_module.compile_version("c++", 2011)
	return my_module







