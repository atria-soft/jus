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
#include <algue/sha512.hpp>

static etk::String extractAndRemove(const etk::String& _inputValue, const char _startMark, const char _stopMark, etk::Vector<etk::String>& _values) {
	_values.clear();
	etk::String out;
	bool inside=false;
	etk::String insideData;
	for (auto &it : _inputValue) {
		if (    inside == false
		     && it == _startMark) {
			inside = true;
		} else if (    inside == true
		            && it == _stopMark) {
			inside = false;
			_values.pushBack(insideData);
			insideData.clear();
		} else if (inside == true) {
			insideData += it;
		} else {
			out += it;
		}
	}
	return out;
}
bool progressCall(const etk::String& _value) {
	return false;
}

void progressCallback(const etk::String& _value) {
	APPL_PRINT("plop " << _value);
}
bool pushVideoFile(zeus::service::ProxyVideo& _srv, etk::String _path, etk::Map<etk::String,etk::String> _basicKey = etk::Map<etk::String,etk::String>()) {
	etk::String extention;
	if (    _path.rfind('.') != etk::String::npos
	     && _path.rfind('.') != 0) {
		extention = etk::tolower(etk::String(_path.begin()+_path.rfind('.')+1, _path.end()));
	}
	etk::String fileName = etk::split(_path, '/').back();
	// internal extention ...
	if (extention == "sha512") {
		return true;
	}
	APPL_PRINT("Add media : '" << _path << "'");
	if (    extention != "avi"
	     && extention != "mkv"
	     && extention != "mov"
	     && extention != "mp4"
	     && extention != "ts"
	     && fileName != "cover_1.jpg"
	     && fileName != "cover_1.png"
	     && fileName != "cover_1.till"
	     && fileName != "cover_1.bmp"
	     && fileName != "cover_1.tga") {
		APPL_ERROR("Sot send file : " << _path << " Not manage extention...");
		return false;
	}
	if (    fileName == "cover_1.jpg"
	     || fileName == "cover_1.png"
	     || fileName == "cover_1.till"
	     || fileName == "cover_1.bmp"
	     || fileName == "cover_1.tga") {
		// find a cover...
		APPL_INFO("Send cover for: " << _basicKey["series-name"] << " " << _basicKey["saison"]);
		if (_basicKey["series-name"] == "") {
			APPL_ERROR("    ==> can not asociate at a specific seri");
			return false;
		}
		etk::String groupName = _basicKey["series-name"];
		if (_basicKey["saison"] != "") {
			groupName += ":" + _basicKey["saison"];
		}
		auto sending = _srv.setGroupCover(zeus::File::create(_path, ""), groupName);
		sending.onSignal(progressCallback);
		sending.waitFor(echrono::seconds(20000));
		return true;
	}
	etk::String storedSha512;
	if (etk::FSNodeExist(_path + ".sha512") == true) {
		uint64_t time_sha512 = etk::FSNodeGetTimeModified(_path + ".sha512");
		uint64_t time_elem = etk::FSNodeGetTimeModified(_path);
		etk::String storedSha512_file = etk::FSNodeReadAllData(_path + ".sha512");
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
	APPL_INFO("Find fileName : '" << fileName << "'");
	// Remove Date (XXXX) or other title
	etk::Vector<etk::String> dates;
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
			_basicKey.insert(etk::Pair<etk::String,etk::String>("date", it));
		} else {
			if (haveTitle == true) {
				APPL_INFO("                '" << fileName << "'");
				APPL_ERROR("Parse Title error : () : " << it << " ==> multiple title");
				continue;
			}
			haveTitle = true;
			// Other title
			_basicKey.insert(etk::Pair<etk::String,etk::String>("title2", it));
		}
	}
	// remove unneeded date
	if (haveDate == false) {
		_basicKey.insert(etk::Pair<etk::String,etk::String>("date", ""));
	}
	// remove unneeded title 2
	if (haveTitle == false) {
		_basicKey.insert(etk::Pair<etk::String,etk::String>("title2", ""));
	}
	// Remove the actors [XXX YYY][EEE TTT]...
	etk::Vector<etk::String> acthors;
	fileName = extractAndRemove(fileName, '[', ']', acthors);
	if (acthors.size() > 0) {
		APPL_INFO("                '" << fileName << "'");
		etk::String actorList;
		for (auto &itActor : acthors) {
			if (actorList != "") {
				actorList += ";";
			}
			actorList += itActor;
		}
		_basicKey.insert(etk::Pair<etk::String,etk::String>("acthors", actorList));
	}
	
	// remove extention
	fileName = etk::String(fileName.begin(), fileName.begin() + fileName.size() - (extention.size()+1));
	
	etk::Vector<etk::String> listElementBase = etk::split(fileName, '-');
	
	etk::Vector<etk::String> listElement;
	etk::String tmpStartString;
	for (size_t iii=0; iii<listElementBase.size(); ++iii) {
		if (    listElementBase[iii][0] != 's'
		     && listElementBase[iii][0] != 'e') {
			if (tmpStartString != "") {
				tmpStartString += '-';
			}
			tmpStartString += listElementBase[iii];
		} else {
			listElement.pushBack(tmpStartString);
			tmpStartString = "";
			for (/* nothing to do */; iii<listElementBase.size(); ++iii) {
				listElement.pushBack(listElementBase[iii]);
			}
		}
		
	}
	if (tmpStartString != "") {
		listElement.pushBack(tmpStartString);
	}
	if (listElement.size() == 1) {
		// nothing to do , it might be a film ...
		_basicKey.insert(etk::Pair<etk::String,etk::String>("title", etk::toString(listElement[0])));
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
			etk::String seriesName = listElement[0];
			
			_basicKey.insert(etk::Pair<etk::String,etk::String>("series-name", etk::toString(seriesName)));
			etk::String fullEpisodeName = listElement[3];
			for (int32_t yyy=4; yyy<listElement.size(); ++yyy) {
				fullEpisodeName += "-" + listElement[yyy];
			}
			_basicKey.insert(etk::Pair<etk::String,etk::String>("title", etk::toString(fullEpisodeName)));
			if (etk::String(&listElement[1][1]) == "XX") {
				// saison unknow ... ==> nothing to do ...
			} else {
				saison = etk::string_to_int32_t(&listElement[1][1]);
			}
			if (etk::String(&listElement[2][1]) == "XX") {
				// episode unknow ... ==> nothing to do ...
			} else {
				episode = etk::string_to_int32_t(&listElement[2][1]);
				
				_basicKey.insert(etk::Pair<etk::String,etk::String>("episode", etk::toString(episode)));
			}
			APPL_INFO("Find a internal mode series: :");
			APPL_INFO("    origin       : '" << fileName << "'");
			etk::String saisonPrint = "XX";
			etk::String episodePrint = "XX";
			if (saison < 0) {
				// nothing to do
			} else if(saison < 10) {
				saisonPrint = "0" + etk::toString(saison);
				_basicKey.insert(etk::Pair<etk::String,etk::String>("saison", etk::toString(saison)));
			} else {
				saisonPrint = etk::toString(saison);
				_basicKey.insert(etk::Pair<etk::String,etk::String>("saison", etk::toString(saison)));
			}
			if (episode < 0) {
				// nothing to do
			} else if(episode < 10) {
				episodePrint = "0" + etk::toString(episode);
				_basicKey.insert(etk::Pair<etk::String,etk::String>("episode", etk::toString(episode)));
			} else {
				episodePrint = etk::toString(episode);
				_basicKey.insert(etk::Pair<etk::String,etk::String>("episode", etk::toString(episode)));
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

void installVideoPath(zeus::service::ProxyVideo& _srv, etk::String _path, etk::Map<etk::String,etk::String> _basicKey = etk::Map<etk::String,etk::String>()) {
	etk::FSNode node(_path);
	APPL_INFO("Parse : '" << _path << "'");
	etk::Vector<etk::String> listSubPath = node.folderGetSub(true, false, "*");
	for (auto &itPath : listSubPath) {
		etk::Map<etk::String,etk::String> basicKeyTmp = _basicKey;
		APPL_INFO("Add Sub path: '" << itPath << "'");
		etk::String lastPathName = etk::split(itPath, '/').back();
		if (basicKeyTmp.size() == 0) {
			APPL_INFO("find A '" << lastPathName << "' " << basicKeyTmp.size());
			if (lastPathName == "film") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "film"));
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("production-methode", "picture"));
			} else if (lastPathName == "film-annimation") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "film"));
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("production-methode", "draw"));
			} else if (lastPathName == "film-short") { // short films
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "film"));
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("production-methode", "short"));
			} else if (lastPathName == "tv-show") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "tv-show"));
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("production-methode", "picture"));
			} else if (lastPathName == "tv-show-annimation") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "tv-show"));
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("production-methode", "draw"));
			} else if (lastPathName == "theater") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "theater"));
			} else if (lastPathName == "one-man") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("type", "one-man"));
			}
		} else {
			APPL_INFO("find B '" << lastPathName << "' " << basicKeyTmp.size());
			if (lastPathName == "saison_01") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "1"));
			} else if (lastPathName == "saison_02") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "2"));
			} else if (lastPathName == "saison_03") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "3"));
			} else if (lastPathName == "saison_04") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "4"));
			} else if (lastPathName == "saison_05") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "5"));
			} else if (lastPathName == "saison_06") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "6"));
			} else if (lastPathName == "saison_07") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "7"));
			} else if (lastPathName == "saison_08") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "8"));
			} else if (lastPathName == "saison_09") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "9"));
			} else if (lastPathName == "saison_10") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "10"));
			} else if (lastPathName == "saison_11") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "11"));
			} else if (lastPathName == "saison_12") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "12"));
			} else if (lastPathName == "saison_13") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "13"));
			} else if (lastPathName == "saison_14") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "14"));
			} else if (lastPathName == "saison_15") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "15"));
			} else if (lastPathName == "saison_16") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "16"));
			} else if (lastPathName == "saison_17") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "17"));
			} else if (lastPathName == "saison_18") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "18"));
			} else if (lastPathName == "saison_19") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "19"));
			} else if (lastPathName == "saison_20") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "20"));
			} else if (lastPathName == "saison_21") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "21"));
			} else if (lastPathName == "saison_22") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "22"));
			} else if (lastPathName == "saison_23") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "23"));
			} else if (lastPathName == "saison_24") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "24"));
			} else if (lastPathName == "saison_25") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "25"));
			} else if (lastPathName == "saison_26") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "26"));
			} else if (lastPathName == "saison_27") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "27"));
			} else if (lastPathName == "saison_28") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "28"));
			} else if (lastPathName == "saison_29") {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("saison", "29"));
			} else {
				basicKeyTmp.insert(etk::Pair<etk::String,etk::String>("series-name", lastPathName));
			}
		}
		installVideoPath(_srv, itPath, basicKeyTmp);
	}
	// Add files :
	etk::Vector<etk::String> listSubFile = node.folderGetSub(false, true, "*");
	for (auto &itFile : listSubFile) {
		
		etk::Map<etk::String,etk::String> basicKeyTmp = _basicKey;
		pushVideoFile(_srv, itFile, _basicKey);
		
	}
}

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	elog::init(_argc, _argv);
	zeus::init(_argc, _argv);
	zeus::Client client1;
	etk::String login = "test1";
	etk::String address = "";
	uint32_t port = 0;
	etk::String pass = "coucou";
	etk::String requestAction = "";
	etk::Vector<etk::String> args;
	for (int32_t iii=1; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
		if (etk::start_with(data, "--login=") == true) {
			// separate loggin and IP adress ...
			etk::Vector<etk::String> listElem = etk::split(&data[8], '~');
			if (listElem.size() == 0) {
				APPL_ERROR("Not enouth element in the login ... need use a XXX~SERVER.org:zzz");
				return -1;
			}
			login = listElem[0];
			if (listElem.size() == 1) {
				// connnect on local host ... nothing to do
			} else {
				etk::Vector<etk::String> listElem2 = etk::split(listElem[1], ':');
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
				args.pushBack(data);
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
	etk::String serviceName = "video";
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
			uint32_t tmpMax = etk::min(iii + 1024, count);
			APPL_DEBUG("read section " << iii << " -> " << tmpMax);
			etk::Vector<uint32_t> list = remoteServiceVideo.getIds(iii,tmpMax).wait().get();
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
			uint32_t tmpMax = etk::min(iii + 1024, count);
			APPL_DEBUG("read section " << iii << " -> " << tmpMax);
			etk::Vector<uint32_t> list = remoteServiceVideo.getIds(iii, tmpMax).wait().get();
			for (auto& it : list) {
				// Get the media
				zeus::ProxyMedia media = remoteServiceVideo.get(it).waitFor(echrono::seconds(2000)).get();
				if (media.exist() == false) {
					APPL_ERROR("get media error");
					return -1;
				}
				etk::String name    = media.getMetadata("title").wait().get();
				etk::String serie   = media.getMetadata("series-name").wait().get();
				etk::String episode = media.getMetadata("episode").wait().get();
				etk::String saison  = media.getMetadata("saison").wait().get();
				etk::String outputDesc = "";
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
		for (auto &it: args) {
			installVideoPath(remoteServiceVideo, it);
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
	return 0;
}
