/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Client.hpp>
#include <zeus/ObjectRemote.hpp>
#include <zeus/mineType.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>
#include <etk/os/FSNode.hpp>
#include <elog/elog.hpp>
#include <algorithm>

#include <etk/stdTools.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ProxyMedia.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <algue/sha512.hpp>

static std::string extractAndRemove(const std::string& _inputValue, const char _startMark, const char _stopMark, std::vector<std::string>& _values) {
	_values.clear();
	std::string out;
	bool inside=false;
	std::string insideData;
	for (auto &it : _inputValue) {
		if (    inside == false
		     && it == _startMark) {
			inside = true;
		} else if (    inside == true
		            && it == _stopMark) {
			inside = false;
			_values.push_back(insideData);
			insideData.clear();
		} else if (inside == true) {
			insideData += it;
		} else {
			out += it;
		}
	}
	return out;
}
bool progressCall(const std::string& _value) {
	return false;
}

void progressCallback(const std::string& _value) {
	APPL_PRINT("plop " << _value);
}
#if 0
bool pushVideoFile(zeus::service::ProxyVideo& _srv, std::string _path, std::map<std::string,std::string> _basicKey = std::map<std::string,std::string>()) {
	std::string extention;
	if (    _path.rfind('.') != std::string::npos
	     && _path.rfind('.') != 0) {
		extention = etk::tolower(std::string(_path.begin()+_path.rfind('.')+1, _path.end()));
	}
	// internal extention ....
	if (extention == "sha512") {
		return true;
	}
	APPL_PRINT("Add media : '" << _path << "'");
	if (    extention != "avi"
	     && extention != "mkv"
	     && extention != "mov"
	     && extention != "mp4"
	     && extention != "ts") {
		APPL_ERROR("Sot send file : " << _path << " Not manage extention...");
		return false;
	}
	std::string storedSha512;
	if (etk::FSNodeExist(_path + ".sha512") == true) {
		uint64_t time_sha512 = etk::FSNodeGetTimeModified(_path + ".sha512");
		uint64_t time_elem = etk::FSNodeGetTimeModified(_path);
		std::string storedSha512_file = etk::FSNodeReadAllData(_path + ".sha512");
		if (time_elem > time_sha512) {
			// check the current sha512 
			storedSha512 = algue::stringConvert(algue::sha512::encodeFromFile(_path));
			if (storedSha512_file != storedSha512) {
				//need to remove the old sha file
				auto idFileToRemove_fut = _srv.getId(storedSha512_file).waitFor(echrono::seconds(2));
				if (idFileToRemove_fut.hasError() == true) {
					APPL_ERROR("can not remove the remote file with sha " + storedSha512_file);
				} else {
					APPL_INFO("Remove old deprecated file: " + storedSha512_file);
					_srv.remove(idFileToRemove_fut.get());
					// note, no need to wait the call is async ... and the user does not interested with the result ...
				}
			}
			// store new sha512 ==> this update tile too ...
			etk::FSNodeWriteAllData(_path + ".sha512", storedSha512);
		} else {
			// store new sha512
			storedSha512 = etk::FSNodeReadAllData(_path + ".sha512");
		}
	} else {
		storedSha512 = algue::stringConvert(algue::sha512::encodeFromFile(_path));
		etk::FSNodeWriteAllData(_path + ".sha512", storedSha512);
	}
	// push only if the file exist
	// TODO : Check the metadata updating ...
	auto idFile_fut = _srv.getId(storedSha512).waitFor(echrono::seconds(2));
	if (idFile_fut.hasError() == false) {
		// media already exit ==> stop here ...
		return true;
	}
	// TODO: Do it better ==> add the calback to know the push progression ...
	auto sending = _srv.add(zeus::File::create(_path, storedSha512));
	sending.onSignal(progressCallback);
	uint32_t mediaId = sending.waitFor(echrono::seconds(20000)).get();
	if (mediaId == 0) {
		APPL_ERROR("Get media ID = 0 With no error");
		return false;
	}
	// Get the media
	zeus::ProxyMedia media = _srv.get(mediaId).waitFor(echrono::seconds(2000)).get();
	if (media.exist() == false) {
		APPL_ERROR("get media error");
		return false;
	}
	
	// TODO: if the media have meta data ==> this mean that the media already added before ...
	
	// Parse file name:
	std::string fileName = etk::split(_path, '/').back();
	APPL_INFO("Find fileName : '" << fileName << "'");
	// Remove Date (XXXX) or other title
	std::vector<std::string> dates;
	fileName = extractAndRemove(fileName, '(', ')', dates);
	bool haveDate = false;
	bool haveTitle = false;
	for (auto &it: dates) {
		if (it.size() == 0) {
			continue;
		}
		if (    it[0] == '0'
		     || it[0] == '1'
		     || it[0] == '2'
		     || it[0] == '3'
		     || it[0] == '4'
		     || it[0] == '5'
		     || it[0] == '6'
		     || it[0] == '7'
		     || it[0] == '8'
		     || it[0] == '9') {
			// find a date ...
			if (haveDate == true) {
				APPL_INFO("                '" << fileName << "'");
				APPL_ERROR("Parse Date error : () : " << it << " ==> multiple date");
				continue;
			}
			haveDate = true;
			_basicKey.insert(std::pair<std::string,std::string>("date", it));
		} else {
			if (haveTitle == true) {
				APPL_INFO("                '" << fileName << "'");
				APPL_ERROR("Parse Title error : () : " << it << " ==> multiple title");
				continue;
			}
			haveTitle = true;
			// Other title
			_basicKey.insert(std::pair<std::string,std::string>("title2", it));
		}
	}
	// remove unneeded date
	if (haveDate == false) {
		_basicKey.insert(std::pair<std::string,std::string>("date", ""));
	}
	// remove unneeded title 2
	if (haveTitle == false) {
		_basicKey.insert(std::pair<std::string,std::string>("title2", ""));
	}
	// Remove the actors [XXX YYY][EEE TTT]...
	std::vector<std::string> acthors;
	fileName = extractAndRemove(fileName, '[', ']', acthors);
	if (acthors.size() > 0) {
		APPL_INFO("                '" << fileName << "'");
		std::string actorList;
		for (auto &itActor : acthors) {
			if (actorList != "") {
				actorList += ";";
			}
			actorList += itActor;
		}
		_basicKey.insert(std::pair<std::string,std::string>("acthors", actorList));
	}
	
	// remove extention
	fileName = std::string(fileName.begin(), fileName.begin() + fileName.size() - (extention.size()+1));
	
	std::vector<std::string> listElementBase = etk::split(fileName, '-');
	
	std::vector<std::string> listElement;
	std::string tmpStartString;
	for (size_t iii=0; iii<listElementBase.size(); ++iii) {
		if (    listElementBase[iii][0] != 's'
		     && listElementBase[iii][0] != 'e') {
			if (tmpStartString != "") {
				tmpStartString += '-';
			}
			tmpStartString += listElementBase[iii];
		} else {
			listElement.push_back(tmpStartString);
			tmpStartString = "";
			for (/* nothing to do */; iii<listElementBase.size(); ++iii) {
				listElement.push_back(listElementBase[iii]);
			}
		}
		
	}
	if (tmpStartString != "") {
		listElement.push_back(tmpStartString);
	}
	if (listElement.size() == 1) {
		// nothing to do , it might be a film ...
		_basicKey.insert(std::pair<std::string,std::string>("title", etk::to_string(listElement[0])));
	} else {
		/*
		for (auto &itt : listElement) {
			APPL_INFO("    " << itt);
		}
		*/
		if (    listElement.size() > 3
		     && listElement[1][0] == 's'
		     && listElement[2][0] == 'e') {
			// internal formalisme ...
			int32_t saison = -1;
			int32_t episode = -1;
			std::string seriesName = listElement[0];
			
			_basicKey.insert(std::pair<std::string,std::string>("series-name", etk::to_string(seriesName)));
			std::string fullEpisodeName = listElement[3];
			for (int32_t yyy=4; yyy<listElement.size(); ++yyy) {
				fullEpisodeName += "-" + listElement[yyy];
			}
			_basicKey.insert(std::pair<std::string,std::string>("title", etk::to_string(fullEpisodeName)));
			if (std::string(&listElement[1][1]) == "XX") {
				// saison unknow ... ==> nothing to do ...
			} else {
				saison = etk::string_to_int32_t(&listElement[1][1]);
			}
			if (std::string(&listElement[2][1]) == "XX") {
				// episode unknow ... ==> nothing to do ...
			} else {
				episode = etk::string_to_int32_t(&listElement[2][1]);
				
				_basicKey.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
			}
			APPL_INFO("Find a internal mode series: :");
			APPL_INFO("    origin       : '" << fileName << "'");
			std::string saisonPrint = "XX";
			std::string episodePrint = "XX";
			if (saison < 0) {
				// nothing to do
			} else if(saison < 10) {
				saisonPrint = "0" + etk::to_string(saison);
				_basicKey.insert(std::pair<std::string,std::string>("saison", etk::to_string(saison)));
			} else {
				saisonPrint = etk::to_string(saison);
				_basicKey.insert(std::pair<std::string,std::string>("saison", etk::to_string(saison)));
			}
			if (episode < 0) {
				// nothing to do
			} else if(episode < 10) {
				episodePrint = "0" + etk::to_string(episode);
				_basicKey.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
			} else {
				episodePrint = etk::to_string(episode);
				_basicKey.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
			}
			APPL_PRINT("     ==> '" << seriesName << "-s" << saisonPrint << "-e" << episodePrint << "-" << fullEpisodeName << "'");
		}
	}
	// send all meta data:
	zeus::FutureGroup group;
	for (auto &itKey : _basicKey) {
		if (itKey.second != "") {
			APPL_WARNING("Set metaData: " << itKey.first << " : " << itKey.second);
		}
		group.add(media.setMetadata(itKey.first, itKey.second));
	}
	group.wait();
	return true;
}

void installVideoPath(zeus::service::ProxyVideo& _srv, std::string _path, std::map<std::string,std::string> _basicKey = std::map<std::string,std::string>()) {
	etk::FSNode node(_path);
	APPL_INFO("Parse : '" << _path << "'");
	std::vector<std::string> listSubPath = node.folderGetSub(true, false, "*");
	for (auto &itPath : listSubPath) {
		std::map<std::string,std::string> basicKeyTmp = _basicKey;
		APPL_INFO("Add Sub path: '" << itPath << "'");
		std::string lastPathName = etk::split(itPath, '/').back();
		if (basicKeyTmp.size() == 0) {
			APPL_INFO("find A '" << lastPathName << "' " << basicKeyTmp.size());
			if (lastPathName == "film") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "film"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			} else if (lastPathName == "film-annimation") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "film"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "draw"));
			} else if (lastPathName == "film-short") { // short films
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "film"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "short"));
			} else if (lastPathName == "tv-show") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "tv-show"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			} else if (lastPathName == "tv-show-annimation") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "tv-show"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "draw"));
			} else if (lastPathName == "theater") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "theater"));
			} else if (lastPathName == "one-man") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "one-man"));
			}
		} else {
			APPL_INFO("find B '" << lastPathName << "' " << basicKeyTmp.size());
			if (lastPathName == "saison_01") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "1"));
			} else if (lastPathName == "saison_02") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "2"));
			} else if (lastPathName == "saison_03") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "3"));
			} else if (lastPathName == "saison_04") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "4"));
			} else if (lastPathName == "saison_05") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "5"));
			} else if (lastPathName == "saison_06") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "6"));
			} else if (lastPathName == "saison_07") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "7"));
			} else if (lastPathName == "saison_08") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "8"));
			} else if (lastPathName == "saison_09") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "9"));
			} else if (lastPathName == "saison_10") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "10"));
			} else if (lastPathName == "saison_11") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "11"));
			} else if (lastPathName == "saison_12") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "12"));
			} else if (lastPathName == "saison_13") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "13"));
			} else if (lastPathName == "saison_14") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "14"));
			} else if (lastPathName == "saison_15") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "15"));
			} else if (lastPathName == "saison_16") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "16"));
			} else if (lastPathName == "saison_17") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "17"));
			} else if (lastPathName == "saison_18") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "18"));
			} else if (lastPathName == "saison_19") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "19"));
			} else if (lastPathName == "saison_20") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "20"));
			} else if (lastPathName == "saison_21") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "21"));
			} else if (lastPathName == "saison_22") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "22"));
			} else if (lastPathName == "saison_23") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "23"));
			} else if (lastPathName == "saison_24") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "24"));
			} else if (lastPathName == "saison_25") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "25"));
			} else if (lastPathName == "saison_26") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "26"));
			} else if (lastPathName == "saison_27") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "27"));
			} else if (lastPathName == "saison_28") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "28"));
			} else if (lastPathName == "saison_29") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("saison", "29"));
			} else {
				basicKeyTmp.insert(std::pair<std::string,std::string>("series-name", lastPathName));
			}
		}
		installVideoPath(_srv, itPath, basicKeyTmp);
	}
	// Add files :
	std::vector<std::string> listSubFile = node.folderGetSub(false, true, "*");
	for (auto &itFile : listSubFile) {
		
		std::map<std::string,std::string> basicKeyTmp = _basicKey;
		pushVideoFile(_srv, itFile, _basicKey);
		
	}
}
#endif

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	elog::init(_argc, _argv);
	zeus::init(_argc, _argv);
	zeus::Client client1;
	std::string login = "test1";
	std::string address = "";
	uint32_t port = 0;
	std::string pass = "coucou";
	std::string requestAction = "";
	std::vector<std::string> args;
	for (int32_t iii=1; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--login=") == true) {
			// separate loggin and IP adress ...
			std::vector<std::string> listElem = etk::split(&data[8], '~');
			if (listElem.size() == 0) {
				APPL_ERROR("Not enouth element in the login ... need use a XXX~SERVER.org:zzz");
				return -1;
			}
			login = listElem[0];
			if (listElem.size() == 1) {
				// connnect on local host ... nothing to do
			} else {
				std::vector<std::string> listElem2 = etk::split(listElem[1], ':');
				if (listElem2.size() >= 1) {
					address = listElem2[0];
				}
				if (listElem2.size() >= 2) {
					port = etk::string_to_uint32_t(listElem2[1]);
				}
			}
		} else if (etk::start_with(data, "--pass=") == true) {
			pass = &data[7];
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options] ACTION [arguments]");
			APPL_PRINT("        --login=XXX   user login");
			APPL_PRINT("        --pass=XXX    pass to connect to the user");
			APPL_PRINT("        ACTION        action to execute: info / list");
			APPL_PRINT("        [arguments]   argument depending on the action");
			return -1;
		} else if (    etk::start_with(data, "--elog") == true
		       || etk::start_with(data, "--etk") == true
		       || etk::start_with(data, "--zeus") == true
		       || etk::start_with(data, "--enet") == true) {
			// disable option ...
		} else {
			if (requestAction == "") {
				requestAction = data;
			} else {
				args.push_back(data);
			}
		}
	}
	if (requestAction == "" ) {
		requestAction = "list";
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client start        ==");
	APPL_INFO("==================================");
	// Generate IP and Port in the client interface
	if (address != "") {
		client1.propertyIp.set(address);
	}
	if (port != 0) {
		client1.propertyPort.set(port);
	}
	bool ret = client1.connect(login, pass);
	if (ret == false) {
		APPL_ERROR("    ==> NOT Authentify with '" << login << "'");
		client1.disconnect();
		return -1;
	}
	if (requestAction == "list") {
		APPL_PRINT("============================================");
		APPL_PRINT("== List: ");
		APPL_PRINT("============================================");
		zeus::Future<int32_t> retNbService = client1.getServiceCount();
		zeus::Future<std::vector<std::string>> retServiceList = client1.getServiceList();
		retNbService.wait();
		APPL_INFO("Nb services = " << retNbService.get());
		retServiceList.wait();
		APPL_INFO("List services:");
		for (auto &it: retServiceList.get()) {
			APPL_INFO("    - " << it);
		}
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else if (requestAction == "info") {
		APPL_PRINT("============================================");
		APPL_PRINT("== info: ");
		APPL_PRINT("============================================");
		// Send a full path:
		if (args.size() == 0) {
			zeus::Future<std::vector<std::string>> retServiceList = client1.getServiceList();
			retServiceList.wait();
			for (auto &it: retServiceList.get()) {
				zeus::Proxy proxy = client1.getService(it);
				if (proxy.exist() == false) {
					APPL_ERROR("[" << it << "] ==> can not connect ..." );
				} else {
					std::string desc = proxy.sys.getDescription().wait().get();
					APPL_PRINT("[" << it << "] " << desc);
				}
			}
		} else if (args.size() != 1) {
			APPL_PRINT("============================================");
			APPL_ERROR("== action: '" << requestAction << "' can have 0 or 1 arguments");
			APPL_PRINT("============================================");
		} else {
			zeus::Future<std::vector<std::string>> retServiceList = client1.getServiceList();
			retServiceList.wait();
			bool exist = false;
			for (auto &it: retServiceList.get()) {
				if (it == args[0]) {
					exist = true;
				}
			}
			if (exist == true) {
				zeus::Proxy proxy = client1.getService(args[0]);
				if (proxy.exist() == false) {
					APPL_ERROR("[" << args[0] << "] ==> can not connect ..." );
				} else {
					APPL_PRINT("[" << args[0] << "] " );
					APPL_PRINT("    desc=" << proxy.sys.getDescription().wait().get());
					APPL_PRINT("    version=" << proxy.sys.getVersion().wait().get());
					APPL_PRINT("    type=" << proxy.sys.getType().wait().get());
					APPL_PRINT("    authors=" << proxy.sys.getAuthors().wait().get());
					auto listFunctions = proxy.sys.getFunctions().wait().get();
					APPL_PRINT("    functions: " << listFunctions.size());
					for (auto &it: listFunctions) {
						#if 0
							APPL_PRINT("        " << it);
							APPL_PRINT("            prototype=" << proxy.sys.getFunctionPrototype(it).wait().get());
							APPL_PRINT("            desc=" << proxy.sys.getFunctionDescription(it).wait().get());
						#else
							APPL_PRINT("        " << proxy.sys.getFunctionPrototype(it).wait().get());
							APPL_PRINT("            " << proxy.sys.getFunctionDescription(it).wait().get());
						#endif
					}
				}
			} else {
				APPL_PRINT("service does not exist: [" << args[0] << "] " );
			}
		}
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else {
		APPL_PRINT("============================================");
		APPL_ERROR("== Unknow action: '" << requestAction << "'");
		APPL_PRINT("============================================");
	}
	client1.disconnect();
	return -1;
}
