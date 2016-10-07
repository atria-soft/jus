#!/usr/bin/python
import lutin.debug as debug
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
	return "authors.txt"

def get_version():
	return "version.txt"

def configure(target, my_module):
	my_module.add_depend([
	    'etk',
	    'enet',
	    'ememory',
	    'eproperty'])
	my_module.add_src_file([
	    'zeus/debug.cpp'
	    ])
	my_module.add_path(".")
	my_module.add_src_file([
	    'zeus/zeus.cpp',
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
	    'zeus/zeus.hpp',
	    'zeus/AbstractFunction.hpp',
	    'zeus/AbstractFunctionTypeDirect.hpp',
	    'zeus/AbstractFunctionTypeClass.hpp',
	    'zeus/FutureData.hpp',
	    'zeus/FutureBase.hpp',
	    'zeus/Future.hpp',
	    'zeus/File.hpp',
	    'zeus/Buffer.hpp',
	    'zeus/BufferParameter.hpp',
	    'zeus/BufferCall.hpp',
	    'zeus/BufferAnswer.hpp',
	    'zeus/BufferData.hpp',
	    'zeus/BufferEvent.hpp',
	    'zeus/BufferFlow.hpp',
	    'zeus/ParamType.hpp',
	    'zeus/debug.hpp',
	    'zeus/Client.hpp',
	    'zeus/RemoteProcessCall.hpp',
	    'zeus/Service.hpp',
	    'zeus/ServiceRemote.hpp',
	    'zeus/WebServer.hpp',
	    'zeus/mineType.hpp',
	    ])
	if target.config["compilator"] == "clang":
		my_module.add_flag('c++', "-Wno-unsequenced", export=True)
	# build in C++ mode
	my_module.compile_version("c++", 2011)
	return True







