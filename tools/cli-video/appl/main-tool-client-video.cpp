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
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ProxyMedia.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>

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

bool pushVideoFile(zeus::service::ProxyVideo& _srv, std::string _path, std::map<std::string,std::string> _basicKey = std::map<std::string,std::string>()) {
	APPL_PRINT("Add media : '" << _path << "'");
	std::string extention;
	if (    _path.rfind('.') != std::string::npos
	     && _path.rfind('.') != 0) {
		extention = etk::tolower(std::string(_path.begin()+_path.rfind('.')+1, _path.end()));
	}
	if (    extention != "avi"
	     && extention != "mkv"
	     && extention != "mov"
	     && extention != "mp4"
	     && extention != "ts") {
		APPL_ERROR("Sot send file : " << _path << " Not manage extention...");
		return false;
	}
	
	uint32_t mediaId = _srv.add(zeus::File::create(_path)).waitFor(echrono::seconds(20000)).get();
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
	// Remove Date (XXXX)
	std::vector<std::string> dates;
	fileName = extractAndRemove(fileName, '(', ')', dates);
	if (dates.size() > 1) {
		APPL_INFO("                '" << fileName << "'");
		APPL_ERROR("Parse Date error : () : " << dates);
	} else if (dates.size() == 1) {
		APPL_INFO("                '" << fileName << "'");
		_basicKey.insert(std::pair<std::string,std::string>("date", dates[0]));
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
			APPL_INFO("find '" << lastPathName << "' " << basicKeyTmp.size());
			if (lastPathName == "films") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "film"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			} else if (lastPathName == "films-annimation") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "film"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "draw"));
			} else if (lastPathName == "tv-show") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "tv-show"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			} else if (lastPathName == "anim") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "tv-show"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "draw"));
			} else if (lastPathName == "courses") { // short films
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "courses"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture")); // TODO : Check "draw"
			} else if (lastPathName == "theater") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "theater"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			} else if (lastPathName == "one-man-show") {
				basicKeyTmp.insert(std::pair<std::string,std::string>("type", "one-man show"));
				basicKeyTmp.insert(std::pair<std::string,std::string>("production-methode", "picture"));
			}
		} else {
			APPL_INFO("find '" << lastPathName << "' " << basicKeyTmp.size());
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
			APPL_PRINT("        ACTION        action to execute: clear, push, pushPath, list");
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
	std::string serviceName = "video";
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- connect service '" << serviceName << "'");
	APPL_INFO("    ----------------------------------");
	if (client1.waitForService(serviceName) == false) {
		APPL_ERROR("==> Service not availlable '" << serviceName << "'");
		client1.disconnect();
		return -1;
	}
	zeus::service::ProxyVideo remoteServiceVideo = client1.getService("video");
	// remove all media (for test)
	if (remoteServiceVideo.exist() == false) {
		APPL_ERROR("==> can not connect to the service proxy '" << serviceName << "'");
		client1.disconnect();
		return -1;
	}
	/* ****************************************************************************************
	   **   Clear All the data base ...
	   ****************************************************************************************/
	if (requestAction == "clear") {
		APPL_PRINT("============================================");
		APPL_PRINT("== Clear data base: ");
		APPL_PRINT("============================================");
		// TODO : Do it :
		APPL_ERROR("NEED to add check in cmd line to execute it ...");
		return -1;
		uint32_t count = remoteServiceVideo.count().wait().get();
		APPL_DEBUG("have " << count << " medias");
		for (uint32_t iii=0; iii<count ; iii += 1024) {
			uint32_t tmpMax = std::min(iii + 1024, count);
			APPL_DEBUG("read section " << iii << " -> " << tmpMax);
			std::vector<uint32_t> list = remoteServiceVideo.getIds(iii,tmpMax).wait().get();
			zeus::FutureGroup groupWait;
			for (auto& it : list) {
				APPL_PRINT("remove ELEMENT : " << it);
				groupWait.add(remoteServiceVideo.remove(it));
			}
			groupWait.waitFor(echrono::seconds(2000));
		}
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else if (requestAction == "list") {
		APPL_PRINT("============================================");
		APPL_PRINT("== list files: ");
		APPL_PRINT("============================================");
		uint32_t count = remoteServiceVideo.count().wait().get();
		APPL_DEBUG("have " << count << " medias");
		for (uint32_t iii=0; iii<count ; iii += 1024) {
			uint32_t tmpMax = std::min(iii + 1024, count);
			APPL_DEBUG("read section " << iii << " -> " << tmpMax);
			std::vector<uint32_t> list = remoteServiceVideo.getIds(iii, tmpMax).wait().get();
			for (auto& it : list) {
				// Get the media
				zeus::ProxyMedia media = remoteServiceVideo.get(it).waitFor(echrono::seconds(2000)).get();
				if (media.exist() == false) {
					APPL_ERROR("get media error");
					return false;
				}
				std::string name    = media.getMetadata("title").wait().get();
				std::string serie   = media.getMetadata("series-name").wait().get();
				std::string episode = media.getMetadata("episode").wait().get();
				std::string saison  = media.getMetadata("saison").wait().get();
				std::string outputDesc = "";
				if (serie != "") {
					outputDesc += serie + "-";
				}
				if (saison != "") {
					outputDesc += "s" + saison + "-";
				}
				if (episode != "") {
					outputDesc += "e" + episode + "-";
				}
				outputDesc += name;
				APPL_PRINT("[" << it << "] " << outputDesc);
			}
		}
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else if (requestAction == "push") {
		APPL_PRINT("============================================");
		APPL_PRINT("== push file: ");
		APPL_PRINT("============================================");
		if (args.size() == 0) {
			APPL_ERROR("Need to specify some file to push ...");
		}
		for (auto &it: args) {
			// TODO : Check if already exist ...
			pushVideoFile(remoteServiceVideo, it);
		}
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else if (requestAction == "pushPath") {
		APPL_PRINT("============================================");
		APPL_PRINT("== push path: ");
		APPL_PRINT("============================================");
		// Send a full path:
		// installVideoPath(remoteServiceVideo, "testVideo");
		APPL_PRINT("============================================");
		APPL_PRINT("==              DONE                      ==");
		APPL_PRINT("============================================");
	} else {
		APPL_PRINT("============================================");
		APPL_ERROR("== Unknow action: '" << requestAction << "'");
		APPL_PRINT("============================================");
	}
	client1.disconnect();
	return 0;
}
