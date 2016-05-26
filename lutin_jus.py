#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "Json micro-service"

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
	my_module.add_module_depend(['etk', 'enet', 'ememory', 'eproperty', 'esignal', 'ejson'])
	my_module.add_src_file([
	    'jus/debug.cpp'
	    ])
	my_module.add_path(tools.get_current_path(__file__))
	my_module.add_src_file([
	    'jus/AbstractFunction.cpp',
	    'jus/AbstractFunctionTypeDirect.cpp',
	    'jus/AbstractFunctionTypeClass.cpp',
	    'jus/FutureBase.cpp',
	    'jus/Future.cpp',
	    'jus/ParamType.cpp',
	    'jus/Client.cpp',
	    'jus/GateWay.cpp',
	    'jus/GateWayService.cpp',
	    'jus/GateWayClient.cpp',
	    'jus/RemoteProcessCall.cpp',
	    'jus/Service.cpp',
	    'jus/ServiceRemote.cpp',
	    'jus/TcpString.cpp',
	    ])
	my_module.add_header_file([
	    'jus/AbstractFunction.h',
	    'jus/AbstractFunctionTypeDirect.h',
	    'jus/AbstractFunctionTypeClass.h',
	    'jus/FutureData.h',
	    'jus/FutureBase.h',
	    'jus/Future.h',
	    'jus/ParamType.h',
	    'jus/debug.h',
	    'jus/Client.h',
	    'jus/GateWay.h',
	    'jus/GateWayService.h',
	    'jus/GateWayClient.h',
	    'jus/RemoteProcessCall.h',
	    'jus/Service.h',
	    'jus/ServiceRemote.h',
	    'jus/TcpString.h',
	    ])
	if target.config["compilator"] == "clang":
		my_module.add_export_flag('c++', "-Wno-unsequenced")
	# build in C++ mode
	my_module.compile_version("c++", 2011)
	return my_module







