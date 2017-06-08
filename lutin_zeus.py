#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import lutin.macro as macro


def get_type():
	return "LIBRARY"

def get_desc():
	return "Zeus ewol micro-service"

def get_licence():
	return "MPL-2"

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
	    'eproperty',
	    'echrono'
	    ])
	my_module.add_src_file([
	    'zeus/debug.cpp'
	    ])
	my_module.add_path(".")
	# Future and promise interface
	my_module.add_src_file([
	    'zeus/FutureBase.cpp',
	    'zeus/Future.cpp',
	    'zeus/Promise.cpp',
	    'zeus/FutureGroup.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/Promise.hpp',
	    'zeus/FutureBase.hpp',
	    'zeus/Future.hpp',
	    'zeus/FutureGroup.hpp',
	    ])
	
	# messaging interface
	my_module.add_src_file([
	    'zeus/message/Message.cpp',
	    'zeus/message/Parameter.cpp',
	    'zeus/message/Call.cpp',
	    'zeus/message/Answer.cpp',
	    'zeus/message/Data.cpp',
	    'zeus/message/Event.cpp',
	    'zeus/message/Flow.cpp',
	    'zeus/message/Parameter_addParameter.cpp',
	    'zeus/message/Parameter_getParameter.cpp',
	    'zeus/message/ParamType.cpp',
	    'zeus/message/type.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/message/Message.hpp',
	    'zeus/message/Parameter.hpp',
	    'zeus/message/Call.hpp',
	    'zeus/message/Answer.hpp',
	    'zeus/message/Data.hpp',
	    'zeus/message/Event.hpp',
	    'zeus/message/Flow.hpp',
	    'zeus/message/ParamType.hpp',
	    'zeus/message/type.hpp',
	    ])
	# complexe object interface
	my_module.add_src_file([
	    'zeus/Object.cpp',
	    'zeus/ObjectIntrospect.cpp',
	    'zeus/ObjectRemote.cpp',
	    'zeus/Proxy.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/Object.hpp',
	    'zeus/ObjectIntrospect.hpp',
	    'zeus/ObjectRemote.hpp',
	    'zeus/Proxy.hpp',
	    ])
	# other ...
	my_module.add_src_file([
	    'zeus/zeus.cpp',
	    'zeus/AbstractFunction.cpp',
	    'zeus/Raw.cpp',
	    'zeus/Client.cpp',
	    'zeus/RemoteProcessCall.cpp',
	    'zeus/WebObj.cpp',
	    'zeus/WebServer.cpp',
	    'zeus/mineType.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/Raw.hpp',
	    'zeus/zeus.hpp',
	    'zeus/ActionNotification.hpp',
	    'zeus/AbstractFunction.hpp',
	    'zeus/AbstractFunctionTypeDirect.hpp',
	    'zeus/AbstractFunctionTypeClass.hpp',
	    'zeus/debug.hpp',
	    'zeus/Client.hpp',
	    'zeus/RemoteProcessCall.hpp',
	    'zeus/WebObj.hpp',
	    'zeus/WebServer.hpp',
	    'zeus/mineType.hpp',
	    'zeus/RemoteProperty.hpp',
	    ])
	
	# add basic object:
	zeus_macro = macro.load_macro('zeus')
	zeus_macro.parse_object_idl(my_module, 'zeus/zeus-File.obj.zeus.idl')
	my_module.add_src_file([
	    'zeus/zeus-File.impl.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/zeus-File.impl.hpp',
	    ])
	
	my_module.add_depend('ejson')
	zeus_macro.parse_object_idl(my_module, 'zeus/zeus-Media.obj.zeus.idl')
	my_module.add_src_file([
	    'zeus/zeus-Media.impl.cpp',
	    ])
	my_module.add_header_file([
	    'zeus/zeus-Media.impl.hpp',
	    ])
	
	if target.config["compilator"] == "clang":
		my_module.add_flag('c++', "-Wno-unsequenced", export=True)
	# build in C++ mode
	my_module.compile_version("c++", 2011)
	return True


