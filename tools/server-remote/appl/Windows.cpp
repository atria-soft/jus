/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <ewol/ewol.hpp>
#include <appl/debug.hpp>
#include <appl/Windows.hpp>
#include <ewol/widget/Label.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/Entry.hpp>
#include <ewol/widget/Slider.hpp>
#include <ewol/widget/Menu.hpp>
#include <ewol/tools/message.hpp>

#include <zeus/zeus.hpp>
#include <zeus/Client.hpp>
#include <zeus/service/ProxyServer.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <etk/stdTools.hpp>
#include <ejson/ejson.hpp>
#include <appl/widget/Connection.hpp>
#include <ewol/context/Context.hpp>
#include <appl/widget/meta/StdPopUp.hpp>


static etk::String g_baseDBName = "USERDATA:config.json";

void appl::Windows::store_db() {
	APPL_DEBUG("Store database [START]");
	ejson::Document database;
	if (m_clientProp != null) {
		database.add("access", m_clientProp->toJson());
	}
	bool retGenerate = database.storeSafe(g_baseDBName);
	APPL_ERROR("Store database [STOP] : " << (g_baseDBName) << " ret = " << retGenerate);
}

void appl::Windows::load_db() {
	ejson::Document database;
	bool ret = database.load(g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	if (m_clientProp == null) {
		m_clientProp = ememory::makeShared<appl::ClientProperty>();
		if (m_clientProp == null) {
			APPL_ERROR(" can not allocate the pointer of data ==> must auto kill");
			autoDestroy();
			return;
		}
	}
	if (m_clientProp != null) {
		m_clientProp->fromJson(database["access"].toObject());
	}
}


appl::Windows::Windows() {
	addObjectType("appl::Windows");
	propertyTitle.setDirectCheck(PROJECT_NAME);
}

void appl::Windows::init() {
	ewol::widget::Windows::init();
	load_db();
	m_composer = ewol::widget::Composer::create();
	if (m_composer == null) {
		APPL_CRITICAL(" An error occured ... in the windows creatrion ...");
		return;
	}
	m_composer->loadFromFile("DATA:///gui.xml");
	setSubWidget(m_composer);
	
	subBind(ewol::widget::Button, "appl-shutdown", signalPressed, sharedFromThis(), &appl::Windows::onCallbackShutdown);
	subBind(ewol::widget::Button, "appl-reboot", signalPressed, sharedFromThis(), &appl::Windows::onCallbackReboot);
	
	// Direct display list:
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	subBind(ewol::widget::Menu, "menu-bar", signalSelect, sharedFromThis(), &appl::Windows::onCallbackMenuEvent);
	shortCutAdd("alt+F4",       "menu:exit");
	shortCutAdd("F12",          "menu:reload-shader");
	shortCutAdd("F11",          "menu:connect");
	signalShortcut.connect(sharedFromThis(), &appl::Windows::onCallbackShortCut);
	// TODO: try to connect the last connection availlable ...
	if (m_clientProp == null) {
		onCallbackMenuEvent("menu:connect");
	} else {
		m_clientProp->connect();
		if (m_clientProp->connection.isAlive() == false) {
			onCallbackMenuEvent("menu:connect");
		}
	}
}


void appl::Windows::onCallbackShortCut(const etk::String& _value) {
	APPL_WARNING("Event from ShortCut : " << _value);
	onCallbackMenuEvent(_value);
}

void appl::Windows::onCallbackMenuEvent(const etk::String& _value) {
	APPL_WARNING("Event from Menu : " << _value);
	if (_value == "menu:connect") {
		appl::widget::ConnectionShared tmpWidget = appl::widget::Connection::create();
		if (tmpWidget == null) {
			APPL_ERROR("Can not open File chooser !!! ");
			return;
		}
		tmpWidget->setProperty(m_clientProp);
		// register on the Validate event:
		tmpWidget->signalValidate.connect(sharedFromThis(), &appl::Windows::onCallbackConnectionValidate);
		// no need of this event watching ...
		tmpWidget->signalCancel.connect(sharedFromThis(), &appl::Windows::onCallbackConnectionCancel);
		// add the widget as windows pop-up ...
		popUpWidgetPush(tmpWidget);
	} else if (_value == "menu:exit") {
		gale::getContext().stop();
	} else if (_value == "menu:reload-shader") {
		ewol::getContext().getResourcesManager().reLoadResources();
		ewol::getContext().forceRedrawAll();
	} else {
		APPL_ERROR("Event from Menu UNKNOW : '" << _value << "'");
	}
}

void appl::Windows::onCallbackConnectionValidate(const ememory::SharedPtr<ClientProperty>& _prop) {
	m_clientProp = _prop;
	if (m_clientProp == null) {
		// TODO: set back in public mode ...
		return;
	}
	store_db();
}

void appl::Windows::onCallbackConnectionCancel() {
	// TODO: set back in public mode ...
	
}

void appl::Windows::onCallbackReboot() {
	if (m_clientProp == null) {
		onCallbackMenuEvent("menu:connect");
		return;
	}
	m_clientProp->connect();
	if (m_clientProp->connection.isAlive() == false) {
		onCallbackMenuEvent("menu:connect");
		return;
	}
	
	bool retSrv = m_clientProp->connection.waitForService("server");
	if (retSrv == false) {
		APPL_ERROR(" ==> SERVICE not availlable or not started");
		return;
	}
	// get all the data:
	zeus::service::ProxyServer remoteService = m_clientProp->connection.getService("server");
	// remove all media (for test)
	if (remoteService.exist() == false) {
		APPL_ERROR("    ==> Service does not exist : 'server'");
		return;
	}
	zeus::Future<void> listElem = remoteService.reboot().wait();
	if (listElem.hasError() == true) {
		popUpWidgetPush(ewol::widget::StdPopUp::create("title", etk::String("Error occured"),
		                                               "comment", etk::String("Reboot can not be done")));
	}
}

void appl::Windows::onCallbackShutdown() {
	if (m_clientProp == null) {
		onCallbackMenuEvent("menu:connect");
		return;
	}
	m_clientProp->connect();
	if (m_clientProp->connection.isAlive() == false) {
		onCallbackMenuEvent("menu:connect");
		return;
	}
	
	bool retSrv = m_clientProp->connection.waitForService("server");
	if (retSrv == false) {
		APPL_ERROR(" ==> SERVICE not availlable or not started");
		return;
	}
	// get all the data:
	zeus::service::ProxyServer remoteService = m_clientProp->connection.getService("server");
	// remove all media (for test)
	if (remoteService.exist() == false) {
		APPL_ERROR("    ==> Service does not exist : 'server'");
		return;
	}
	zeus::Future<void> listElem = remoteService.shutdown().wait();
	if (listElem.hasError() == true) {
		popUpWidgetPush(ewol::widget::StdPopUp::create("title", etk::String("Error occured"),
		                                               "comment", etk::String("Reboot can not be done")));
	}
}



