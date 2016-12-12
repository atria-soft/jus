/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/mineType.hpp>
#include <zeus/debug.hpp>


static std::vector<std::pair<std::string, std::string>> mineList = {
  /* Video files */
  { "webm",  "video/webm"},
  { "asf",   "video/x-ms-asf"},
  { "avi",   "video/avi"},
  { "avc",   "video/avi"},
  { "dv",    "video/x-dv"},
  { "divx",  "video/avi"},
  { "wmv",   "video/x-ms-wmv"},
  { "mjpg",  "video/x-motion-jpeg"},
  { "mjpeg", "video/x-motion-jpeg"},
  { "mpeg",  "video/mpeg"},
  { "mpg",   "video/mpeg"},
  { "mpe",   "video/mpeg"},
  { "mp2p",  "video/mp2p"},
  { "vob",   "video/mp2p"},
  { "mp2t",  "video/mp2t"},
  { "m1v",   "video/mpeg"},
  { "m2v",   "video/mpeg2"},
  { "mpg2",  "video/mpeg2"},
  { "mpeg2", "video/mpeg2"},
  { "m4v",   "video/mp4"},
  { "m4p",   "video/mp4"},
  { "mp4ps", "video/x-nerodigital-ps"},
  { "ts",    "video/mpeg2"},
  { "ogm",   "video/mpeg"},
  { "mkv",   "video/x-matroska"},
  { "rmvb",  "video/mpeg"},
  { "mov",   "video/quicktime"},
  { "hdmov", "video/quicktime"},
  { "qt",    "video/quicktime"},

  /* Audio files */
  { "weba", "audio/webm"},
  { "3gp",  "audio/3gpp"},
  { "aac",  "audio/x-aac"},
  { "ac3",  "audio/x-ac3"},
  { "aif",  "audio/aiff"},
  { "aiff", "audio/aiff"},
  { "at3p", "audio/x-atrac3"},
  { "au",   "audio/basic"},
  { "snd",  "audio/basic"},
  { "dts",  "audio/x-dts"},
  { "rmi",  "audio/midi"},
  { "mid",  "audio/midi"},
  { "mp1",  "audio/mp1"},
  { "mp2",  "audio/mp2"},
  { "mp3",  "audio/mpeg"},
  { "mp4",  "audio/mp4"},
  { "m4a",  "audio/mp4"},
  { "mka",  "audio/x-matroska"},
  { "ogg",  "audio/x-ogg"},
  { "wav",  "audio/wav"},
  { "pcm",  "audio/l16"},
  { "lpcm", "audio/l16"},
  { "l16",  "audio/l16"},
  { "wma",  "audio/x-ms-wma"},
  { "mka",  "audio/x-matroska"},
  { "ra",   "audio/x-pn-realaudio"},
  { "rm",   "audio/x-pn-realaudio"},
  { "ram",  "audio/x-pn-realaudio"},
  { "flac", "audio/x-flac"},

  /* Images files */
  { "webp", "image/webp"},
  { "bmp",  "image/bmp"},
  { "ico",  "image/x-icon"},
  { "gif",  "image/gif"},
  { "jpg",  "image/jpeg"},
  { "jpeg", "image/jpeg"},
  { "jpe",  "image/jpeg"},
  { "pcd",  "image/x-ms-bmp"},
  { "png",  "image/png"},
  { "pnm",  "image/x-portable-anymap"},
  { "ppm",  "image/x-portable-pixmap"},
  { "qti",  "image/x-quicktime"},
  { "qtf",  "image/x-quicktime"},
  { "qtif", "image/x-quicktime"},
  { "tif",  "image/tiff"},
  { "tiff", "image/tiff"},

  /* Playlist files */
  { "pls", "audio/x-scpls"},
  { "m3u", "audio/mpegurl"},
  { "asx", "video/x-ms-asf"},

  /* Subtitle Text files */
  { "srt", "text/srt"}, /* SubRip */
  { "ssa", "text/ssa"}, /* SubStation Alpha */
  { "stl", "text/srt"}, /* Spruce */
  { "psb", "text/psb"}, /* PowerDivX */
  { "pjs", "text/pjs"}, /* Phoenix Japanim */
  { "sub", "text/sub"}, /* MicroDVD */
  { "idx", "text/idx"}, /* VOBsub */
  { "dks", "text/dks"}, /* DKS */
  { "scr", "text/scr"}, /* MACsub */
  { "tts", "text/tts"}, /* TurboTitler */
  { "vsf", "text/vsf"}, /* ViPlay */
  { "zeg", "text/zeg"}, /* ZeroG */
  { "mpl", "text/mpl"}, /* MPL */

  /* Miscellaneous text files */
  { "bup", "text/bup"}, /* DVD backup */
  { "ifo", "text/ifo"}, /* DVD information */
  
  /* Some Raw format for images */
  { "yuv422",  "image/x-raw/yuv422"},
  { "yuv420",  "image/x-raw/yuv420"},
  { "yuv411",  "image/x-raw/yuv411"},
  { "rgb",     "image/x-raw/r8g8b8"},
  { "rgba",    "image/x-raw/r8g8b8a8"},
  
  { "js",      "application/javascript"},
  { "raw",     "application/octet-stream"},
  { "ogg",     "application/ogg"},
  { "pdf",     "application/pdf"},
  { "xhtml",   "application/xhtml+xml"},
  { "flw",     "application/x-shockwave-flash"},
  { "json",    "application/json"},
  { "xml",     "application/xml"},
  { "zip",     "application/zip"},
  { "gz",      "application/gzip"},
  { "rar",     "application/rar"},
  
  { "css",     "text/css"},
  { "csv",     "text/csv"},
  { "html",    "text/html"},
  { "js",      "text/javascript"}, // DEPRECATED application/javascript.
  { "txt",     "text/plain"},
  { "xml",     "text/xml"},
  { "json",    "text/json"},
  { "yml",     "text/yml"},
  
  { "c",       "code/c"},
  { "h",       "header/c"},
  { "cpp",     "code/c++"},
  { "hpp",     "header/c++"},
  { "c#",      "code/c#"},
  { "py",      "code/python"},
  { "java",    "code/java"},
  { "js",      "code/javascript"},
};

std::string zeus::getMineType(const std::string& _extention) {
	for (auto &it : mineList) {
		if (it.first == _extention) {
			return it.second;
		}
	}
	ZEUS_ERROR(" try to convert extention in mine type: '" << _extention << "'");
	return "";
}

std::string zeus::getExtention(const std::string& _mineType) {
	for (auto &it : mineList) {
		if (it.second == _mineType) {
			return it.first;
		}
	}
	ZEUS_ERROR(" try to convert mine-type in extention: '" << _mineType << "'");
	return "";
}
