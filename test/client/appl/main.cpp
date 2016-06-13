/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.h>
#include <jus/Client.h>
#include <jus/ServiceRemote.h>
#include <jus/mineType.h>
#include <etk/etk.h>
#include <etk/os/FSNode.h>
#include <unistd.h>

#include <etk/stdTools.h>

int main(int _argc, const char *_argv[]) {
	etk::init(_argc, _argv);
	jus::Client client1;
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
	APPL_INFO("== JUS test client start        ==");
	APPL_INFO("==================================");
	client1.connect("test1#atria-soft.com");
	if (false) {
		jus::Future<bool> retIdentify = client1.call("identify", "clientTest1#atria-soft.com", "QSDQSDGQSF54HSXWVCSQDJ654URTDJ654NBXCDFDGAEZ51968");
		retIdentify.wait();
		if (retIdentify.get() == false) {
			APPL_ERROR("    ==> NOT Connected with 'clientTest1#atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Connected with 'clientTest1#atria-soft.com'");
		}
	} else if (true) {
		jus::Future<bool> retIdentify = client1.call("auth", "coucou");
		retIdentify.wait();
		if (retIdentify.get() == false) {
			APPL_ERROR("    ==> NOT Authentify with 'test1#atria-soft.com'");
			return -1;
		} else {
			APPL_INFO("    ==> Authentify with 'test1#atria-soft.com'");
		}
	} else {
		jus::Future<bool> retIdentify = client1.call("anonymous");
		retIdentify.wait();
		if (retIdentify.get() == false) {
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
	jus::Future<int32_t> retNbService = client1.call("getServiceCount");
	retNbService.wait();
	APPL_INFO("Nb services = " << retNbService.get());
	jus::Future<std::vector<std::string>> retServiceList = client1.call("getServiceList");
	retServiceList.wait();
	APPL_INFO("List services:");
	for (auto &it: retServiceList.get()) {
		APPL_INFO("    - " << it);
	}
	/*
	jus::ServiceRemote localService = client1.getService("serviceTest1");
	if (localService.exist() == true) {
		double retCall = localService.call_d("mul", 13.1, 2.0);
		APPL_INFO("serviceTest1.mul = " << retCall);
	}
	*/
	
	if (true) {
		APPL_INFO("    ----------------------------------");
		APPL_INFO("    -- Get service system-user");
		APPL_INFO("    ----------------------------------");
		
		jus::ServiceRemote remoteServiceUser = client1.getService("system-user");
		if (remoteServiceUser.exist() == true) {
			jus::Future<std::vector<std::string>> retCall = remoteServiceUser.call("getGroups", "clientTest1#atria-soft.com");
			retCall.wait();
			APPL_INFO("system-user.getGroups() = " << retCall.get());
			jus::Future<std::string> retDesc = remoteServiceUser.call("sys.getDescription");
			jus::Future<std::string> retVersion = remoteServiceUser.call("sys.getVersion");
			jus::Future<std::string> retType = remoteServiceUser.call("sys.getType");
			jus::Future<std::vector<std::string>> retExtention = remoteServiceUser.call("srv.getExtention");
			jus::Future<std::vector<std::string>> retMaintainer = remoteServiceUser.call("sys.getAuthors");
			retDesc.wait();
			retVersion.wait();
			retType.wait();
			retExtention.wait();
			retMaintainer.wait();
			APPL_INFO("Service: system-user");
			APPL_INFO("    version   : " << retVersion.get());
			APPL_INFO("    type      : " << retType.get());
			APPL_INFO("    Extention : " << retExtention.get().size());
			for (auto &it : retExtention.get()) {
				APPL_INFO("        - " << it);
			}
			APPL_INFO("    maintainer: " << retMaintainer.get().size());
			for (auto &it : retMaintainer.get()) {
				APPL_INFO("        - " << it);
			}
			APPL_INFO("    description:");
			APPL_INFO("        " << retDesc.get());
			APPL_INFO("    Function List:");
			jus::Future<std::vector<std::string>> retFuctions = remoteServiceUser.call("sys.getFunctions").wait();
			for (auto it : retFuctions.get()) {
				std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
				jus::Future<std::string> retFunctionPrototype = remoteServiceUser.call("sys.getFunctionPrototype", it);
				jus::Future<std::string> retFunctionHelp = remoteServiceUser.call("sys.getFunctionDescription", it);
				retFunctionPrototype.wait();
				retFunctionHelp.wait();
				std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
				APPL_INFO("        - " << retFunctionPrototype.get());
				APPL_INFO("            " << retFunctionHelp.get());
				APPL_INFO("          IO1=" << int64_t(retFunctionPrototype.getTransmitionTime().count()/1000)/1000.0 << " ms");
				APPL_INFO("          IO2=" << int64_t(retFunctionHelp.getTransmitionTime().count()/1000)/1000.0 << " ms");
				APPL_INFO("          IO*=" << int64_t((stop-start).count()/1000)/1000.0 << " ms");
			}
		}
	}
	APPL_INFO("    ----------------------------------");
	APPL_INFO("    -- Get service picture");
	APPL_INFO("    ----------------------------------");
	if (false) {
		jus::ServiceRemote remoteServicePicture = client1.getService("picture");
		if (remoteServicePicture.exist() == true) {
			/*
			jus::Future<std::vector<std::string>> retCall = remoteServicePicture.call("getAlbums").wait();
			APPL_INFO("    album list: ");
			for (auto &it : retCall.get()) {
				jus::Future<uint32_t> retCount = remoteServicePicture.call("getAlbumCount", it).wait();
				if (retCount.get() != 0) {
					APPL_INFO("        - " << it << " / " << retCount.get() << " images");
					jus::Future<std::vector<std::string>> retListImage = remoteServicePicture.call("getAlbumListPicture", it).wait();
					for (auto &it3 : retListImage.get()) {
						APPL_INFO("                - " << it3);
					}
				} else {
					APPL_INFO("        - " << it);
				}
				jus::Future<std::vector<std::string>> retCall2 = remoteServicePicture.call("getSubAlbums", it).wait();
				for (auto &it2 : retCall2.get()) {
					jus::Future<uint32_t> retCount2 = remoteServicePicture.call("getAlbumCount", it2).wait();
					if (retCount2.get() != 0) {
						APPL_INFO("            - " << it2 << " / " << retCount2.get() << " images");
						jus::Future<std::vector<std::string>> retListImage = remoteServicePicture.call("getAlbumListPicture", it2).wait();
						for (auto &it3 : retListImage.get()) {
							APPL_INFO("                - " << it3);
							jus::Future<jus::File> retListImage = remoteServicePicture.call("getAlbumPicture", it3).wait();
							jus::File tmpFile = retListImage.get();
							APPL_INFO("                    mine-type: " << tmpFile.getMineType());
							APPL_INFO("                    size: " << tmpFile.getData().size());
							APPL_INFO("                    receive in =" << int64_t(retListImage.getTransmitionTime().count()/1000)/1000.0 << " ms");
							std::string tmpFileName = std::string("./out/") + it + "_" + it2 + "_" + it3 + "." + jus::getExtention(tmpFile.getMineType());
							APPL_INFO("                    store in: " << tmpFileName);
							etk::FSNode node(tmpFileName);
							node.fileOpenWrite();
							node.fileWrite(&tmpFile.getData()[0], 1, tmpFile.getData().size());
							node.fileClose();
						}
					} else {
						APPL_INFO("            - " << it2);
					}
				}
			}
			*/
			#if 0
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
			jus::File tmp("./photo_2016_33913.bmp");//"image/jpg", {0,5,2,6,7,5,8,4,5,2,1,5,65,5,2,6,85,4,6,6,54,65,88,64,14,6,4,64,51,3,16,4});
			int32_t size = tmp.getData().size();
			jus::FutureBase retSendImage = remoteServicePicture.call("addFile", tmp).wait();
			std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
			APPL_WARNING("          IO*=" << int64_t((stop-start).count()/1000)/1000.0 << " ms");
			double megaParSec = double(size)/(double((stop-start).count())/1000000000.0);
			APPL_WARNING("          speed=" << int64_t(megaParSec/1024.0)/1024.0 << " Mo/s");
			#endif
		}
	}
	int32_t iii=0;
	while (iii < 3) {
		usleep(500000);
		APPL_INFO("Appl in waiting ... " << iii << "/3");
		iii++;
	}
	client1.disconnect();
	APPL_INFO("==================================");
	APPL_INFO("== JUS test client stop");
	APPL_INFO("==================================");
	return 0;
}
