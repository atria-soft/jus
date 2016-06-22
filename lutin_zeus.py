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
	return [0,1,"dev"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_module_depend(['etk', 'enet', 'ememory', 'eproperty', 'esignal'])
	my_module.add_src_file([
	    'zeus/debug.cpp'
	    ])
	my_module.add_path(tools.get_current_path(__file__))
	my_module.add_src_file([
	    'zeus/AbstractFunction.cpp',
	    'zeus/AbstractFunctionTypeDirect.cpp',
	    'zeus/AbstractFunctionTypeClass.cpp',
	    'zeus/FutureBase.cpp',
	    'zeus/Future.cpp',
	    'zeus/File.cpp',
	    'zeus/Buffer.cpp',
	    'zeus/Buffer_addParameter.cpp',
	    'zeus/Buffer_getParameter.cpp',
	    'zeus/ParamType.cpp',
	    'zeus/Client.cpp',
	    'zeus/GateWay.cpp',
	    'zeus/GateWayService.cpp',
	    'zeus/GateWayClient.cpp',
	    'zeus/RemoteProcessCall.cpp',
	    'zeus/Service.cpp',
	    'zeus/ServiceRemote.cpp',
	    'zeus/TcpString.cpp',
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
	    'zeus/ParamType.h',
	    'zeus/debug.h',
	    'zeus/Client.h',
	    'zeus/GateWay.h',
	    'zeus/GateWayService.h',
	    'zeus/GateWayClient.h',
	    'zeus/RemoteProcessCall.h',
	    'zeus/Service.h',
	    'zeus/ServiceRemote.h',
	    'zeus/TcpString.h',
	    'zeus/mineType.h',
	    ])
	if target.config["compilator"] == "clang":
		my_module.add_export_flag('c++', "-Wno-unsequenced")
	# build in C++ mode
	my_module.compile_version("c++", 2011)
	return my_module







