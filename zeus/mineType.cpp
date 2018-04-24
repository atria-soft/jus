/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <zeus/mineType.hpp>
#include <zeus/debug.hpp>
#include <etk/Pair.hpp>

static etk::Vector<etk::Pair<etk::String, etk::String>> getMimeList() {
	static etk::Vector<etk::Pair<etk::String, etk::String>> mineList;
	if (mineList.size() != 0) {
		return mineList;
	}
	/* Video files */
	mineList.pushBack(etk::makePair("webm",  "video/webm"));
	mineList.pushBack(etk::makePair("asf",   "video/x-ms-asf"));
	mineList.pushBack(etk::makePair("avi",   "video/avi"));
	mineList.pushBack(etk::makePair("avc",   "video/avi"));
	mineList.pushBack(etk::makePair("dv",    "video/x-dv"));
	mineList.pushBack(etk::makePair("divx",  "video/avi"));
	mineList.pushBack(etk::makePair("wmv",   "video/x-ms-wmv"));
	mineList.pushBack(etk::makePair("mjpg",  "video/x-motion-jpeg"));
	mineList.pushBack(etk::makePair("mjpeg", "video/x-motion-jpeg"));
	mineList.pushBack(etk::makePair("mpeg",  "video/mpeg"));
	mineList.pushBack(etk::makePair("mpg",   "video/mpeg"));
	mineList.pushBack(etk::makePair("mpe",   "video/mpeg"));
	mineList.pushBack(etk::makePair("mp2p",  "video/mp2p"));
	mineList.pushBack(etk::makePair("vob",   "video/mp2p"));
	mineList.pushBack(etk::makePair("mp2t",  "video/mp2t"));
	mineList.pushBack(etk::makePair("m1v",   "video/mpeg"));
	mineList.pushBack(etk::makePair("m2v",   "video/mpeg2"));
	mineList.pushBack(etk::makePair("mpg2",  "video/mpeg2"));
	mineList.pushBack(etk::makePair("mpeg2", "video/mpeg2"));
	mineList.pushBack(etk::makePair("m4v",   "video/mp4"));
	mineList.pushBack(etk::makePair("m4p",   "video/mp4"));
	mineList.pushBack(etk::makePair("mp4ps", "video/x-nerodigital-ps"));
	mineList.pushBack(etk::makePair("ts",    "video/mpeg2-ts"));
	mineList.pushBack(etk::makePair("ogm",   "video/mpeg"));
	mineList.pushBack(etk::makePair("mkv",   "video/x-matroska"));
	mineList.pushBack(etk::makePair("rmvb",  "video/mpeg"));
	mineList.pushBack(etk::makePair("mov",   "video/quicktime"));
	mineList.pushBack(etk::makePair("hdmov", "video/quicktime"));
	mineList.pushBack(etk::makePair("qt",    "video/quicktime"));
	/* Audio files */
	mineList.pushBack(etk::makePair("weba", "audio/webm"));
	mineList.pushBack(etk::makePair("3gp",  "audio/3gpp"));
	mineList.pushBack(etk::makePair("aac",  "audio/x-aac"));
	mineList.pushBack(etk::makePair("ac3",  "audio/x-ac3"));
	mineList.pushBack(etk::makePair("aif",  "audio/aiff"));
	mineList.pushBack(etk::makePair("aiff", "audio/aiff"));
	mineList.pushBack(etk::makePair("at3p", "audio/x-atrac3"));
	mineList.pushBack(etk::makePair("au",   "audio/basic"));
	mineList.pushBack(etk::makePair("snd",  "audio/basic"));
	mineList.pushBack(etk::makePair("dts",  "audio/x-dts"));
	mineList.pushBack(etk::makePair("rmi",  "audio/midi"));
	mineList.pushBack(etk::makePair("mid",  "audio/midi"));
	mineList.pushBack(etk::makePair("mp1",  "audio/mp1"));
	mineList.pushBack(etk::makePair("mp2",  "audio/mp2"));
	mineList.pushBack(etk::makePair("mp3",  "audio/mpeg"));
	mineList.pushBack(etk::makePair("mp4",  "audio/mp4"));
	mineList.pushBack(etk::makePair("m4a",  "audio/mp4"));
	mineList.pushBack(etk::makePair("mka",  "audio/x-matroska"));
	mineList.pushBack(etk::makePair("ogg",  "audio/x-ogg"));
	mineList.pushBack(etk::makePair("wav",  "audio/wav"));
	mineList.pushBack(etk::makePair("pcm",  "audio/l16"));
	mineList.pushBack(etk::makePair("lpcm", "audio/l16"));
	mineList.pushBack(etk::makePair("l16",  "audio/l16"));
	mineList.pushBack(etk::makePair("wma",  "audio/x-ms-wma"));
	mineList.pushBack(etk::makePair("mka",  "audio/x-matroska"));
	mineList.pushBack(etk::makePair("ra",   "audio/x-pn-realaudio"));
	mineList.pushBack(etk::makePair("rm",   "audio/x-pn-realaudio"));
	mineList.pushBack(etk::makePair("ram",  "audio/x-pn-realaudio"));
	mineList.pushBack(etk::makePair("flac", "audio/x-flac"));
	/* Images files */
	mineList.pushBack(etk::makePair("webp", "image/webp"));
	mineList.pushBack(etk::makePair("bmp",  "image/bmp"));
	mineList.pushBack(etk::makePair("ico",  "image/x-icon"));
	mineList.pushBack(etk::makePair("gif",  "image/gif"));
	mineList.pushBack(etk::makePair("jpg",  "image/jpeg"));
	mineList.pushBack(etk::makePair("jpeg", "image/jpeg"));
	mineList.pushBack(etk::makePair("jpe",  "image/jpeg"));
	mineList.pushBack(etk::makePair("pcd",  "image/x-ms-bmp"));
	mineList.pushBack(etk::makePair("png",  "image/png"));
	mineList.pushBack(etk::makePair("pnm",  "image/x-portable-anymap"));
	mineList.pushBack(etk::makePair("ppm",  "image/x-portable-pixmap"));
	mineList.pushBack(etk::makePair("qti",  "image/x-quicktime"));
	mineList.pushBack(etk::makePair("qtf",  "image/x-quicktime"));
	mineList.pushBack(etk::makePair("qtif", "image/x-quicktime"));
	mineList.pushBack(etk::makePair("tif",  "image/tiff"));
	mineList.pushBack(etk::makePair("tiff", "image/tiff"));
	/* Playlist files */
	mineList.pushBack(etk::makePair("pls", "audio/x-scpls"));
	mineList.pushBack(etk::makePair("m3u", "audio/mpegurl"));
	mineList.pushBack(etk::makePair("asx", "video/x-ms-asf"));
	/* Subtitle Text files */
	mineList.pushBack(etk::makePair("srt", "text/srt")); /* SubRip */
	mineList.pushBack(etk::makePair("ssa", "text/ssa")); /* SubStation Alpha */
	mineList.pushBack(etk::makePair("stl", "text/srt")); /* Spruce */
	mineList.pushBack(etk::makePair("psb", "text/psb")); /* PowerDivX */
	mineList.pushBack(etk::makePair("pjs", "text/pjs")); /* Phoenix Japanim */
	mineList.pushBack(etk::makePair("sub", "text/sub")); /* MicroDVD */
	mineList.pushBack(etk::makePair("idx", "text/idx")); /* VOBsub */
	mineList.pushBack(etk::makePair("dks", "text/dks")); /* DKS */
	mineList.pushBack(etk::makePair("scr", "text/scr")); /* MACsub */
	mineList.pushBack(etk::makePair("tts", "text/tts")); /* TurboTitler */
	mineList.pushBack(etk::makePair("vsf", "text/vsf")); /* ViPlay */
	mineList.pushBack(etk::makePair("zeg", "text/zeg")); /* ZeroG */
	mineList.pushBack(etk::makePair("mpl", "text/mpl")); /* MPL */
	/* Miscellaneous text files */
	mineList.pushBack(etk::makePair("bup", "text/bup")); /* DVD backup */
	mineList.pushBack(etk::makePair("ifo", "text/ifo")); /* DVD information */
	/* Some Raw format for images */
	mineList.pushBack(etk::makePair("yuv422",  "image/x-raw/yuv422"));
	mineList.pushBack(etk::makePair("yuv420",  "image/x-raw/yuv420"));
	mineList.pushBack(etk::makePair("yuv411",  "image/x-raw/yuv411"));
	mineList.pushBack(etk::makePair("rgb",     "image/x-raw/r8g8b8"));
	mineList.pushBack(etk::makePair("rgba",    "image/x-raw/r8g8b8a8"));
	
	mineList.pushBack(etk::makePair("js",      "application/javascript"));
	mineList.pushBack(etk::makePair("raw",     "application/octet-stream"));
	mineList.pushBack(etk::makePair("ogg",     "application/ogg"));
	mineList.pushBack(etk::makePair("pdf",     "application/pdf"));
	mineList.pushBack(etk::makePair("xhtml",   "application/xhtml+xml"));
	mineList.pushBack(etk::makePair("flw",     "application/x-shockwave-flash"));
	mineList.pushBack(etk::makePair("json",    "application/json"));
	mineList.pushBack(etk::makePair("xml",     "application/xml"));
	mineList.pushBack(etk::makePair("zip",     "application/zip"));
	mineList.pushBack(etk::makePair("gz",      "application/gzip"));
	mineList.pushBack(etk::makePair("rar",     "application/rar"));
	
	mineList.pushBack(etk::makePair("css",     "text/css"));
	mineList.pushBack(etk::makePair("csv",     "text/csv"));
	mineList.pushBack(etk::makePair("html",    "text/html"));
	mineList.pushBack(etk::makePair("js",      "text/javascript")); // DEPRECATED application/javascript.
	mineList.pushBack(etk::makePair("txt",     "text/plain"));
	mineList.pushBack(etk::makePair("xml",     "text/xml"));
	mineList.pushBack(etk::makePair("json",    "text/json"));
	mineList.pushBack(etk::makePair("yml",     "text/yml"));
	
	mineList.pushBack(etk::makePair("c",       "code/c"));
	mineList.pushBack(etk::makePair("h",       "header/c"));
	mineList.pushBack(etk::makePair("cpp",     "code/c++"));
	mineList.pushBack(etk::makePair("hpp",     "header/c++"));
	mineList.pushBack(etk::makePair("c#",      "code/c#"));
	mineList.pushBack(etk::makePair("py",      "code/python"));
	mineList.pushBack(etk::makePair("java",    "code/java"));
	mineList.pushBack(etk::makePair("js",      "code/javascript"));
	
	// microsoft mime type:
	mineList.pushBack(etk::makePair("doc",     "application/msword"));
	mineList.pushBack(etk::makePair("dot",     "application/msword"));
	
	mineList.pushBack(etk::makePair("docx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
	mineList.pushBack(etk::makePair("dotx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.template"));
	mineList.pushBack(etk::makePair("docm",    "application/vnd.ms-word.document.macroEnabled.12"));
	mineList.pushBack(etk::makePair("dotm",    "application/vnd.ms-word.template.macroEnabled.12"));
	
	mineList.pushBack(etk::makePair("xls",     "application/vnd.ms-excel"));
	mineList.pushBack(etk::makePair("xlt",     "application/vnd.ms-excel"));
	mineList.pushBack(etk::makePair("xla",     "application/vnd.ms-excel"));
	
	mineList.pushBack(etk::makePair("xlsx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
	mineList.pushBack(etk::makePair("xltx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.template"));
	mineList.pushBack(etk::makePair("xlsm",    "application/vnd.ms-excel.sheet.macroEnabled.12"));
	mineList.pushBack(etk::makePair("xltm",    "application/vnd.ms-excel.template.macroEnabled.12"));
	mineList.pushBack(etk::makePair("xlam",    "application/vnd.ms-excel.addin.macroEnabled.12"));
	mineList.pushBack(etk::makePair("xlsb",    "application/vnd.ms-excel.sheet.binary.macroEnabled.12"));
	
	mineList.pushBack(etk::makePair("ppt",     "application/vnd.ms-powerpoint"));
	mineList.pushBack(etk::makePair("pot",     "application/vnd.ms-powerpoint"));
	mineList.pushBack(etk::makePair("pps",     "application/vnd.ms-powerpoint"));
	mineList.pushBack(etk::makePair("ppa",     "application/vnd.ms-powerpoint"));
	
	mineList.pushBack(etk::makePair("pptx",    "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
	mineList.pushBack(etk::makePair("potx",    "application/vnd.openxmlformats-officedocument.presentationml.template"));
	mineList.pushBack(etk::makePair("ppsx",    "application/vnd.openxmlformats-officedocument.presentationml.slideshow"));
	mineList.pushBack(etk::makePair("ppam",    "application/vnd.ms-powerpoint.addin.macroEnabled.12"));
	mineList.pushBack(etk::makePair("pptm",    "application/vnd.ms-powerpoint.presentation.macroEnabled.12"));
	mineList.pushBack(etk::makePair("potm",    "application/vnd.ms-powerpoint.template.macroEnabled.12"));
	mineList.pushBack(etk::makePair("ppsm",    "application/vnd.ms-powerpoint.slideshow.macroEnabled.12"));
	
	mineList.pushBack(etk::makePair("mdb",     "application/vnd.ms-access"));
	
	
	return mineList;
};

etk::String zeus::getMineType(etk::String _extention) {
	_extention = etk::tolower(_extention);
	for (auto &it : getMimeList()) {
		if (it.first == _extention) {
			return it.second;
		}
	}
	if (_extention == "") {
		return "";
	}
	return "unknow/" + _extention;
}

etk::String zeus::getExtention(etk::String _mineType) {
	_mineType = etk::tolower(_mineType);
	for (auto &it : getMimeList()) {
		if (it.second == _mineType) {
			return it.first;
		}
	}
	// special internal case to support all type of extention ...
	if (etk::start_with(_mineType, "unknow/") == true) {
		return &_mineType[7];
	}
	return "";
}
