#!/usr/bin/python
import lutin.tools as tools
import lutin.debug as debug
import os
import lutinLib_ffmpegCommon

def get_type():
	#return "BINARY_SHARED"
	return "BINARY"

def get_desc():
	return "FFMPEG main application"

def get_licence():
	return "GPL-3"

def get_compagny_type():
	return "org"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return "authors.txt"

def get_version():
	return "version.txt"

def configure(target, my_module):
	# add the file to compile:
	my_module.add_src_file([
	    'appl/debug.cpp',
	    'appl/Main.cpp',
	    'appl/Windows.cpp',
	    'appl/widget/VideoPlayer.cpp',
	    ])
	my_module.add_depend([
	    'ffmpeg-libs',
	    'ewol',
	    'audio-river'
	    ])
	my_module.add_flag('c++', [
	    "-DPROJECT_NAME=\"\\\""+my_module.get_name()+"\\\"\"",
	    "-DAPPL_VERSION=\"\\\"" + tools.version_to_string(get_version()) + "\\\"\"",
	    "-Wno-deprecated-declarations"
	    ])
	my_module.add_path(".")
	return True


