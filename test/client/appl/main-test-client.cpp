/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
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
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>

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
				_srv.albumMediaAdd(_albumID, mediaId).wait();
			}
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
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data, "--ip=") == true) {
			client1.propertyIp.set(std::string(&data[5]));
		} else if (etk::start_with(data, "--port=") == true) {
			client1.propertyPort.set(etk::string_to_uint16_t(std::string(&data[7])));
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT(etk::getApplicationName() << " - help : ");
			APPL_PRINT("    " << _argv[0] << " [options]");
			APPL_PRINT("        --ip=XXX      Server connection IP (default: 1.7.0.0.1)");
			APPL_PRINT("        --port=XXX    Server connection PORT (default: 1983)");
			return -1;
		}
	}
	APPL_INFO("==================================");
	APPL_INFO("== ZEUS test client start        ==");
	APPL_INFO("==================================");
	
	if (false) {
		bool ret = client1.connect("test1~atria-soft.com", "clientTest1~atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to 'test1~atria-soft.com' with 'clientTest1~atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'clientTest1~atria-soft.com'");
		}
	} else if (true) {
		bool ret = client1.connect("test1~atria-soft.com", "coucou");
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with 'test1~atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Authentify with 'test1~atria-soft.com'");
		}
	} else {
		bool ret = client1.connect("test1~atria-soft.com");
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous'");
		}
	}
	// Connect to ourself:
	//client1.authentificate("coucou");
	//bool retAuthentify = client1.call_b("authentify", "coucou");
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service count");
	APPL_INFO("    ----------------------------------");
	zeus::Future<int32_t> retNbService = client1.call(ZEUS_NO_ID_OBJECT, ZEUS_ID_GATEWAY, "getServiceCount");
	retNbService.wait();
	APPL_INFO("Nb services = " << retNbService.get());
	zeus::Future<std::vector<std::string>> retServiceList = client1.call(ZEUS_NO_ID_OBJECT, ZEUS_ID_GATEWAY, "getServiceList");
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
	
	if (false) {
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
	if (true) {
		zeus::service::ProxyPicture remoteServicePicture = client1.getService("picture");
		if (remoteServicePicture.exist() == true) {
			// Send a full path:
			installPath(remoteServicePicture, "testPhoto", 0);
		} else {
			APPL_ERROR("Can not get service Picture ...");
		}
	}
	if (true) {
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