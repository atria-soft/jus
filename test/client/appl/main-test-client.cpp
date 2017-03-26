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


#include <etk/stdTools.hpp>
#include <zeus/service/ProxyUser.hpp>
#include <zeus/service/ProxyPicture.hpp>
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>

void installPath(zeus::service::ProxyPicture& _srv, std::string _path, uint32_t _albumID) {
	etk::FSNode node(_path);
	APPL_INFO("Parse : '" << _path << "'");
	std::vector<std::string> listSubPath = node.folderGetSub(true, false, "*");
	for (auto &itPath : listSubPath) {
		APPL_INFO("Create Album : '" << itPath << "'");
		std::string albumName = etk::split(itPath, '/').back();
		uint32_t albumId = _srv.albumCreate(albumName).wait().get();
		_srv.albumDescriptionSet(albumId, itPath).wait();
		if (_albumID != 0) {
			_srv.albumParentSet(albumId, _albumID).wait();
		}
		installPath(_srv, itPath, albumId);
	}
	
	std::vector<std::string> listSubFile = node.folderGetSub(false, true, "*");
	for (auto &itFile : listSubFile) {
		APPL_INFO("Add media : '" << itFile << "' in " << _albumID);
		std::string extention = etk::tolower(std::string(itFile.begin()+itFile.size() -3, itFile.end()));
		if (    extention == "jpg"
		     || extention == "png"
		     || extention == "gif"
		     || extention == "bmp"
		     || extention == "avi"
		     || extention == "ogg"
		     || extention == "mp3"
		     || extention == "mkv"
		     || extention == "mka"
		     || extention == "tga"
		     || extention == "mp2"
		     || extention == "mov") {
			uint32_t mediaId = _srv.mediaAdd(zeus::File::create(itFile)).wait().get();
			if (mediaId == 0) {
				APPL_ERROR("Get media ID = 0 With no error");
			}
			if (_albumID != 0) {
				APPL_INFO("Add media '" << mediaId << "'in ALbum: " << _albumID);
				_srv.albumMediaAdd(_albumID, mediaId).wait();
			}
		} else {
			APPL_ERROR("Sot send file : " << itFile << "  Not manage extention...");
		}
	}
}

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
		APPL_INFO("Add media : '" << itFile << "'");
		std::string extention = etk::tolower(std::string(itFile.begin()+itFile.size() -3, itFile.end()));
		if (    extention == "avi"
		     || extention == "mkv"
		     || extention == "mov"
		     || extention == "mp4") {
			
			uint32_t mediaId = _srv.mediaAdd(zeus::File::create(itFile)).waitFor(echrono::seconds(20000)).get();
			if (mediaId == 0) {
				APPL_ERROR("Get media ID = 0 With no error");
				continue;
			}
			
			// Parse file name:
			std::string fileName = etk::split(itFile, '/').back();
			APPL_INFO("Find fileName : '" << fileName << "'");
			// Remove Date (XXXX)
			std::vector<std::string> dates;
			fileName = extractAndRemove(fileName, '(', ')', dates);
			if (dates.size() > 1) {
				APPL_INFO("                '" << fileName << "'");
				APPL_ERROR("Parse Date error : () : " << dates);
			} else if (dates.size() == 1) {
				APPL_INFO("                '" << fileName << "'");
				basicKeyTmp.insert(std::pair<std::string,std::string>("date", dates[0]));
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
				basicKeyTmp.insert(std::pair<std::string,std::string>("acthors", actorList));
			}
			
			// remove extention
			fileName = std::string(fileName.begin(), fileName.begin() + fileName.size() - 4);
			
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
				basicKeyTmp.insert(std::pair<std::string,std::string>("title", etk::to_string(listElement[0])));
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
					
					basicKeyTmp.insert(std::pair<std::string,std::string>("series-name", etk::to_string(seriesName)));
					basicKeyTmp.insert(std::pair<std::string,std::string>("title", etk::to_string(listElement[3])));
					if (std::string(&listElement[1][1]) == "XX") {
						// saison unknow ... ==> nothing to do ...
					} else {
						saison = etk::string_to_int32_t(&listElement[1][1]);
					}
					if (std::string(&listElement[2][1]) == "XX") {
						// episode unknow ... ==> nothing to do ...
					} else {
						episode = etk::string_to_int32_t(&listElement[2][1]);
						
						basicKeyTmp.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
					}
					APPL_INFO("Find a internal mode series: :");
					APPL_INFO("    origin       : '" << fileName << "'");
					std::string saisonPrint = "XX";
					std::string episodePrint = "XX";
					if (saison < 0) {
						// nothing to do
					} else if(saison < 10) {
						saisonPrint = "0" + etk::to_string(saison);
						basicKeyTmp.insert(std::pair<std::string,std::string>("saison", etk::to_string(saison)));
					} else {
						saisonPrint = etk::to_string(saison);
						basicKeyTmp.insert(std::pair<std::string,std::string>("saison", etk::to_string(saison)));
					}
					if (episode < 0) {
						// nothing to do
					} else if(episode < 10) {
						episodePrint = "0" + etk::to_string(episode);
						basicKeyTmp.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
					} else {
						episodePrint = etk::to_string(episode);
						basicKeyTmp.insert(std::pair<std::string,std::string>("episode", etk::to_string(episode)));
					}
					APPL_INFO("    recontituated: '" << seriesName << "-s" << saisonPrint << "-e" << episodePrint << "-" << listElement[3] << "'");
				}
			}
			
			// send all meta data:
			zeus::FutureGroup group;
			for (auto &itKey : basicKeyTmp) {
				group.add(_srv.mediaMetadataSetKey(mediaId, itKey.first, itKey.second));
			}
			group.wait();
			
		} else {
			APPL_ERROR("Sot send file : " << itFile << "  Not manage extention...");
		}
	}
}
int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	elog::init(_argc, _argv);
	zeus::init(_argc, _argv);
	zeus::Client client1;
	std::string fromUser = "test1";
	std::string toUser = "test1";
	std::string pass = "coucou";
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			client1.propertyIp.set(std::string(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			client1.propertyPort.set(etk::string_to_uint16_t(std::string(&data[7])));
		} else if (etk::start_with(data, "--from=") == true) {
			fromUser = &data[7];
		} else if (etk::start_with(data, "--to=") == true) {
			toUser = &data[7];
		} else if (etk::start_with(data, "--pass=") == true) {
			pass = &data[7];
		} else if (etk::start_with(data, "--tocken=") == true) {
			pass = &data[9];
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --ip=XXX      Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX    Server connection PORT (default: 1983)");
			APPL_PRINT("        --from=XXX    user that request connection");
			APPL_PRINT("        --to=XXX      destination user that we want to connect");
			APPL_PRINT("        --pass=XXX    pass or tocker to connect to the user");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client start        ==");
	APPL_INFO("==================================");
	
	if (fromUser == toUser) {
		bool ret = client1.connect(fromUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with '" << toUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Authentify with '" << toUser << "'");
		}
	} else if (fromUser != "") {
		bool ret = client1.connect(fromUser, toUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to '" << toUser << "' with '" << fromUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with '" << toUser << "'");
		}
	} else {
		bool ret = client1.connect(toUser);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous' to '" << toUser << "'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous' to '" << toUser << "'");
		}
	}
	// Connect to ourself:
	//client1.authentificate("coucou");
	//bool retAuthentify = client1.call_b("authentify", "coucou");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count");
	APPL_INFO("    ----------------------------------");
	zeus::Future<int32_t> retNbService = client1.getServiceCount();
	zeus::Future<std::vector<std::string>> retServiceList = client1.getServiceList();
	retNbService.wait();
	APPL_INFO("Nb services = " << retNbService.get());
	retServiceList.wait();
	APPL_INFO("List services:");
	for (auto &it: retServiceList.get()) {
		APPL_INFO("    - " << it);
	}
	/*
	zeus::ServiceRemote localService = client1.getService("serviceTest1");
	if (localService.exist() == true) {
		double retCall = localService.call_d("mul", 13.1, 2.0);
		APPL_INFO("serviceTest1.mul = " << retCall);
	}
	*/
	
	if (    client1.waitForService("user") == true
	     && false) {
		
		APPL_INFO("    ----------------------------------");
		APPL_INFO("    -- Get service system-user");
		APPL_INFO("    ----------------------------------");
		zeus::service::ProxyUser remoteServiceUser;
		remoteServiceUser = client1.getService("user");
		if (remoteServiceUser.exist() == true) {
			zeus::Future<std::vector<std::string>> retCall = remoteServiceUser.clientGroupsGet("clientTest1#atria-soft.com");
			retCall.wait();
			APPL_INFO("system-user.getGroups() = " << retCall.get());
			// system DOC
			zeus::Future<std::string> retDesc = remoteServiceUser.sys.getDescription();
			zeus::Future<std::string> retVersion = remoteServiceUser.sys.getVersion();
			zeus::Future<std::string> retType = remoteServiceUser.sys.getType();
			zeus::Future<std::vector<std::string>> retMaintainer = remoteServiceUser.sys.getAuthors();
			retDesc.wait();
			retVersion.wait();
			retType.wait();
			retMaintainer.wait();
			APPL_INFO("Service: system-user");
			APPL_INFO("    version   : " << retVersion.get());
			APPL_INFO("    type      : " << retType.get());
			APPL_INFO("    maintainer: " << retMaintainer.get().size());
			for (auto &it : retMaintainer.get()) {
				APPL_INFO("        - " << it);
			}
			APPL_INFO("    description:");
			APPL_INFO("        " << retDesc.get());
			APPL_INFO("    Function List:");
			zeus::Future<std::vector<std::string>> retFuctions = remoteServiceUser.sys.getFunctions().wait();
			for (auto it : retFuctions.get()) {
				echrono::Steady start = echrono::Steady::now();
				zeus::Future<std::string> retFunctionPrototype = remoteServiceUser.sys.getFunctionPrototype(it);
				zeus::Future<std::string> retFunctionHelp = remoteServiceUser.sys.getFunctionDescription(it);
				retFunctionPrototype.wait();
				retFunctionHelp.wait();
				echrono::Steady stop = echrono::Steady::now();
				APPL_INFO("        - " << retFunctionPrototype.get());
				APPL_INFO("            " << retFunctionHelp.get());
				APPL_INFO("          IO1=" << int64_t(retFunctionPrototype.getTransmitionTime().count()/1000)/1000.0 << " ms");
				APPL_INFO("          IO2=" << int64_t(retFunctionHelp.getTransmitionTime().count()/1000)/1000.0 << " ms");
				APPL_INFO("          IO*=" << (stop-start));
			}
		}
	}
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service picture");
	APPL_INFO("    ----------------------------------");
	if (    client1.waitForService("picture") == true
	     && false) {
		zeus::service::ProxyPicture remoteServicePicture = client1.getService("picture");
		if (remoteServicePicture.exist() == true) {
			// Send a full path:
			installPath(remoteServicePicture, "testPhoto", 0);
		} else {
			APPL_ERROR("Can not get service Picture ...");
		}
	}
	if (    client1.waitForService("picture") == true
	     && false) {
		zeus::service::ProxyPicture remoteServicePicture = client1.getService("picture");
		if (remoteServicePicture.exist() == true) {
			zeus::Future<std::vector<uint32_t>> retCall = remoteServicePicture.albumGetList().wait();
			if (retCall.hasError() == true) {
				APPL_INFO("    Get an error when getting list of albums ...");
			} else {
				APPL_INFO("    album list: ");
				for (auto &it : retCall.get()) {
					std::string name = remoteServicePicture.albumNameGet(it).wait().get();
					std::string desc = remoteServicePicture.albumDescriptionGet(it).wait().get();
					uint32_t retCount = remoteServicePicture.albumMediaCount(it).wait().get();
					uint32_t parentId = remoteServicePicture.albumParentGet(it).wait().get();
					
					if (retCount != 0) {
						APPL_INFO("        - [" << it << "] '" << name << "' DESC=" << desc);
						if (parentId != 0) {
							APPL_INFO("                 PARENT : " << parentId);
						}
						APPL_INFO("                 " << retCount << " Medias");
						std::vector<uint32_t> retListImage = remoteServicePicture.albumMediaIdGet(it, 0, retCount).wait().get();
						for (auto &it3 : retListImage) {
							APPL_INFO("                - " << it3);
						}
					} else {
						APPL_INFO("        - " << it);
					}
				}
			}
			#if 1
				echrono::Steady start = echrono::Steady::now();
				//ememory::SharedPtr<zeus::File> tmp = zeus::File::create("./tmpResult.bmp");
				ememory::SharedPtr<zeus::File> tmp = zeus::File::create("./testImage.png");
				//ememory::SharedPtr<zeus::File> tmp = zeus::File::create("./test_log.txt");
				int32_t size = tmp->getSize();
				auto retSendImage = remoteServicePicture.mediaAdd(tmp).wait();
				if (retSendImage.hasError() == true) {
					APPL_ERROR(" get an error while sending the File : '" << retSendImage.getErrorType() << "' help=" << retSendImage.getErrorHelp() );
				} else {
					echrono::Steady stop = echrono::Steady::now();
					APPL_WARNING("          IO*=" << (stop-start) << "                    " << retSendImage.get());
					double megaParSec = double(size)/(double((stop-start).count())/1000000000.0);
					APPL_WARNING("          speed=" << int64_t(megaParSec/1024.0)/1024.0 << " Mo/s");
				}
			#endif
		}
	}
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service video (send DATA)  ");
	APPL_INFO("    ----------------------------------");
	if (    client1.waitForService("video") == true
	     && true) {
		zeus::service::ProxyVideo remoteServiceVideo = client1.getService("video");
		// remove all media (for test)
		if (remoteServiceVideo.exist() == true) {
			uint32_t count = remoteServiceVideo.mediaIdCount().wait().get();
			std::vector<uint32_t> list = remoteServiceVideo.mediaIdGet(0,count).wait().get();
			zeus::FutureGroup groupWait;
			for (auto& it : list) {
				APPL_INFO("remove ELEMENT : " << it);
				groupWait.add(remoteServiceVideo.mediaRemove(it));
			}
			groupWait.waitFor(echrono::seconds(2000));
		}
		
		// Update path:
		if (remoteServiceVideo.exist() == true) {
			// Send a full path:
			installVideoPath(remoteServiceVideo, "testVideo");
		} else {
			APPL_ERROR("Can not get service Picture ...");
		}
		// List all data:
		if (remoteServiceVideo.exist() == true) {
			
			
		}
	}
	
	
	int32_t iii=0;
	while (iii < 3) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		APPL_INFO("Appl in waiting ... " << iii << "/3");
		iii++;
	}
	APPL_INFO("Request disconnect");
	client1.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client stop");
	APPL_INFO("==================================");
	return 0;
}
