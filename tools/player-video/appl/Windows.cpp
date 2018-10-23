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
#include <appl/widget/VideoPlayer.hpp>
#include <ewol/tools/message.hpp>

#include <zeus/zeus.hpp>
#include <zeus/Client.hpp>
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <etk/stdTools.hpp>
#include <ejson/ejson.hpp>
#include <appl/widget/Connection.hpp>
#include <ewol/context/Context.hpp>
#include <appl/widget/Player.hpp>


static etk::Uri g_baseDBName = "CONFIG:///config.json";

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


appl::Windows::Windows():
  m_id(0),
  m_fullScreen(false) {
	addObjectType("appl::Windows");
	propertyTitle.setDirectCheck(etk::String("sample ") + PROJECT_NAME);
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
	
	drawWidgetTree();
	
	m_listViewer = ememory::dynamicPointerCast<appl::widget::ListViewer>(m_composer->getSubObjectNamed("ws-name-list-viewer"));
	if (m_listViewer != null) {
		m_listViewer->signalSelect.connect(sharedFromThis(), &appl::Windows::onCallbackSelectMedia);
	}
	
	subBind(ewol::widget::Button, "access-fast-back", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectBack);
	subBind(ewol::widget::Button, "access-fast-home", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectHome);
	subBind(ewol::widget::Button, "access-fast-group", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectGroup);
	// Direct display list:
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	subBind(ewol::widget::Menu, "menu-bar", signalSelect, sharedFromThis(), &appl::Windows::onCallbackMenuEvent);
	shortCutAdd("alt+F4",       "menu:exit");
	shortCutAdd("F12",          "menu:reload-shader");
	shortCutAdd("F9",           "menu:connect");
	shortCutAdd("F11",          "menu:full-screen");
	signalShortcut.connect(sharedFromThis(), &appl::Windows::onCallbackShortCut);
	// TODO: try to connect the last connection availlable ...
	if (m_clientProp == null) {
		onCallbackMenuEvent("menu:connect");
	} else {
		m_clientProp->connect();
		if (m_clientProp->getConnection().isAlive() == false) {
			onCallbackMenuEvent("menu:connect");
		} else {
			if (m_listViewer != null) {
				m_listViewer->setClientProperty(m_clientProp);
				m_listViewer->searchElements();
			}
		}
	}
	m_player = ememory::dynamicPointerCast<appl::widget::Player>(m_composer->getSubObjectNamed("ws-name-player"));
	if (m_player != null) {
		m_player->signalPrevious.connect(sharedFromThis(), &appl::Windows::onCallbackPlayerPrevious);
		m_player->signalNext.connect(sharedFromThis(), &appl::Windows::onCallbackPlayerNext);
		m_player->signalFinished.connect(sharedFromThis(), &appl::Windows::onCallbackPlayerFinished);
	}
}


void appl::Windows::onCallbackShortCut(const etk::String& _value) {
	APPL_WARNING("Event from ShortCut : " << _value);
	onCallbackMenuEvent(_value);
}

void appl::Windows::onCallbackMenuEvent(const etk::String& _value) {
	APPL_WARNING("Event from Menu : " << _value);
	if (m_player != null) {
		m_player->stop();
	}
	if (_value == "menu:connect") {
		appl::widget::ConnectionShared tmpWidget = appl::widget::Connection::create();
		if (tmpWidget == null) {
			APPL_ERROR("Can not open File chooser !!! ");
			return;
		}
		tmpWidget->setProperty(m_clientProp);
		// register on the Validate event:
		tmpWidget->signalValidate.connect(sharedFromThis(), &appl::Windows::onCallbackConnectionValidate);
		tmpWidget->signalConnectionError.connect(sharedFromThis(), &appl::Windows::onCallbackConnectionError);
		// no need of this event watching ...
		tmpWidget->signalCancel.connect(sharedFromThis(), &appl::Windows::onCallbackConnectionCancel);
		// add the widget as windows pop-up ...
		popUpWidgetPush(tmpWidget);
	} else if (_value == "menu:full-screen") {
		if (m_fullScreen == false) {
			m_fullScreen = true;
		} else {
			m_fullScreen = false;
		}
		gale::getContext().setFullScreen(m_fullScreen);
	} else if (_value == "menu:exit") {
		gale::getContext().stop();
	} else if (_value == "menu:back") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->backHistory();
	} else if (_value == "menu:home") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("");
	} else if (_value == "menu:group") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("group");
	} else if (_value == "menu:tv") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-tv");
	
	} else if (_value == "menu:films") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("film");
	} else if (_value == "menu:animation-films") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("annimation");
	} else if (_value == "menu:tv-show") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("tv-show");
	} else if (_value == "menu:animation-tv-show") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("tv-annimation");
	} else if (_value == "menu:teather") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("teather");
	} else if (_value == "menu:one-man-show") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("one-man");
	} else if (_value == "menu:courses") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("courses");
	} else if (_value == "menu:TV-child") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("");
		// TODO: ...
	} else if (_value == "menu:TV-adult") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("");
		// TODO: ...
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
	// Update viewer to show all ...
	if (m_listViewer != null) {
		m_listViewer->setClientProperty(m_clientProp);
		m_listViewer->searchElements();
	}
}
void appl::Windows::onCallbackConnectionError(const ememory::SharedPtr<ClientProperty>& _prop) {
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

void appl::Windows::onCallbackPrevious() {
	m_id--;
	if (m_id < 0) {
		m_id = m_list.size()-1;
	}
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != null) {
		// stop previous (if needed)
		tmpDisp->stop();
		// Set new file:
		tmpDisp->setFile(m_list[m_id]);
		tmpDisp->play();
		echrono::Duration time = tmpDisp->getDuration();
		APPL_DEBUG("duration = " << time << "  " << etk::toString(time.toSeconds()));
		propertySetOnWidgetNamed("progress-bar", "value", "0");
		propertySetOnWidgetNamed("progress-bar", "max", etk::toString(time.toSeconds()));
	}
}

void appl::Windows::onCallbackBack() {
	
}




void appl::Windows::onCallbackNext() {
	m_id++;
	if (m_id >= m_list.size()) {
		m_id = 0;
	}
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != null) {
		// stop previous (if needed)
		tmpDisp->stop();
		// Set new file:
		tmpDisp->setFile(m_list[m_id]);
		tmpDisp->play();
		echrono::Duration time = tmpDisp->getDuration();
		APPL_DEBUG("duration = " << time << "  " << etk::toString(time.toSeconds()));
		propertySetOnWidgetNamed("progress-bar", "value", "0");
		propertySetOnWidgetNamed("progress-bar", "max", etk::toString(time.toSeconds()));
	}
}



void appl::Windows::addFile(const etk::String& _file) {
	APPL_DEBUG("Add file : " << _file);
	m_list.pushBack(_file);
	if (m_list.size() == 1) {
		m_id = 0;
		ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
		if (tmpDisp != null) {
			tmpDisp->setFile(m_list[m_id]);
			echrono::Duration time = tmpDisp->getDuration();
			APPL_DEBUG("duration = " << time << "  " << etk::toString(time.toSeconds()));
			propertySetOnWidgetNamed("progress-bar", "value", "0");
			propertySetOnWidgetNamed("progress-bar", "max", etk::toString(time.toSeconds()));
		}
	}
}

void appl::Windows::onCallbackSelectMedia(const uint32_t& _value) {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-player");
	if (m_player != null) {
		m_player->playStream(m_clientProp, _value);
	}
}

void appl::Windows::onCallbackPlayerPrevious() {
	if (m_player != null) {
		m_player->stop();
	}
	if (m_listViewer != null) {
		if (m_listViewer->previous() == false) {
			ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		}
	}
}

void appl::Windows::onCallbackPlayerNext() {
	if (m_player != null) {
		m_player->stop();
	}
	if (m_listViewer != null) {
		if (m_listViewer->next() == false) {
			ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		}
	}
}

void appl::Windows::onCallbackPlayerFinished() {
	if (m_player != null) {
		m_player->stop();
	}
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
}

