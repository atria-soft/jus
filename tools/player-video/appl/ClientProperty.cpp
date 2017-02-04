/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <appl/ClientProperty.hpp>
#include <appl/debug.hpp>
#include <appl/widget/ListViewer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>

#include <egami/egami.hpp>
#include <zeus/zeus.hpp>
#include <zeus/Client.hpp>
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <etk/stdTools.hpp>
#include <ejson/ejson.hpp>

void appl::ClientProperty::connect() {
	// Generate IP and Port in the client interface
	connection.propertyIp.set(address);
	connection.propertyPort.set(port);
	// Connection depending on the mode requested
	if (fromUser == toUser) {
		bool ret = connection.connect(fromUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with '" << toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Authentify with '" << toUser << "'");
		}
	} else if (fromUser != "") {
		bool ret = connection.connect(fromUser, toUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to '" << toUser << "' with '" << fromUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with '" << toUser << "'");
		}
	} else {
		bool ret = connection.connect(toUser);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous' to '" << toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous' to '" << toUser << "'");
		}
	}
}


#include <esignal/details/Signal.hxx>
ESIGNAL_DECLARE_SIGNAL(ememory::SharedPtr<appl::ClientProperty>);

