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


static std::string g_baseDBName = "USERDATA:config.json";

void appl::Windows::store_db() {
	APPL_DEBUG("Store database [START]");
	ejson::Document database;
	if (m_clientProp != nullptr) {
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
	if (m_clientProp == nullptr) {
		m_clientProp = ememory::makeShared<appl::ClientProperty>();
		if (m_clientProp == nullptr) {
			APPL_ERROR(" can not allocate the pointer of data ==> must auto kill");
			autoDestroy();
			return;
		}
	}
	if (m_clientProp != nullptr) {
		m_clientProp->fromJson(database["access"].toObject());
	}
}


appl::Windows::Windows():
  m_id(0) {
	addObjectType("appl::Windows");
	propertyTitle.setDirectCheck(std::string("sample ") + PROJECT_NAME);
}

void appl::Windows::init() {
	ewol::widget::Windows::init();
	load_db();
	m_composer = ewol::widget::Composer::create();
	if (m_composer == nullptr) {
		APPL_CRITICAL(" An error occured ... in the windows creatrion ...");
		return;
	}
	m_composer->loadFromFile("DATA:gui.xml");
	setSubWidget(m_composer);
	
	m_listViewer = ememory::dynamicPointerCast<appl::widget::ListViewer>(m_composer->getSubObjectNamed("ws-name-list-viewer"));
	m_listViewer->signalSelect.connect(sharedFromThis(), &appl::Windows::onCallbackSelectMedia);
	
	subBind(ewol::widget::Button, "bt-previous", signalPressed, sharedFromThis(), &appl::Windows::onCallbackPrevious);
	subBind(ewol::widget::Button, "bt-play", signalValue, sharedFromThis(), &appl::Windows::onCallbackPlay);
	subBind(ewol::widget::Button, "bt-next", signalPressed, sharedFromThis(), &appl::Windows::onCallbackNext);
	subBind(ewol::widget::Button, "bt-back", signalPressed, sharedFromThis(), &appl::Windows::onCallbackBack);
	subBind(appl::widget::VideoDisplay, "displayer", signalFps, sharedFromThis(), &appl::Windows::onCallbackFPS);
	subBind(appl::widget::VideoDisplay, "displayer", signalPosition, sharedFromThis(), &appl::Windows::onCallbackPosition);
	subBind(ewol::widget::Slider, "progress-bar", signalChange, sharedFromThis(), &appl::Windows::onCallbackSeekRequest);
	
	
	subBind(ewol::widget::Button, "bt-film-picture", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectFilms);
	subBind(ewol::widget::Button, "bt-film-draw", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectAnnimation);
	subBind(ewol::widget::Button, "bt-tv-picture", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTVShow);
	subBind(ewol::widget::Button, "bt-tv-draw", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTvAnnimation);
	subBind(ewol::widget::Button, "bt-theater", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTeather);
	subBind(ewol::widget::Button, "bt-one-man-show", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectOneManShow);
	subBind(ewol::widget::Button, "bt-courses", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectSourses);
	
	subBind(ewol::widget::Button, "access-fast-home", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectHome);
	subBind(ewol::widget::Button, "access-fast-group", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectGroup);
	
	// Direct display list:
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	subBind(ewol::widget::Menu, "menu-bar", signalSelect, sharedFromThis(), &appl::Windows::onCallbackMenuEvent);
	shortCutAdd("alt+F4",       "menu:exit");
	shortCutAdd("F12",          "menu:reload-shader");
	shortCutAdd("F11",          "menu:connect");
	signalShortcut.connect(sharedFromThis(), &appl::Windows::onCallbackShortCut);
	// TODO: try to connect the last connection availlable ...
	if (m_clientProp == nullptr) {
		onCallbackMenuEvent("menu:connect");
	} else {
		m_clientProp->connect();
		if (m_clientProp->connection.isAlive() == false) {
			onCallbackMenuEvent("menu:connect");
		} else {
			if (m_listViewer != nullptr) {
				m_listViewer->setClientProperty(m_clientProp);
				m_listViewer->searchElements();
			}
		}
	}
}


void appl::Windows::onCallbackShortCut(const std::string& _value) {
	APPL_WARNING("Event from ShortCut : " << _value);
	onCallbackMenuEvent(_value);
}

void appl::Windows::onCallbackMenuEvent(const std::string& _value) {
	APPL_WARNING("Event from Menu : " << _value);
	if (_value == "menu:connect") {
		appl::widget::ConnectionShared tmpWidget = appl::widget::Connection::create();
		if (tmpWidget == nullptr) {
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
	} else if (_value == "menu:home") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
		m_listViewer->searchElements("");
	} else if (_value == "menu:group") {
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-group");
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
	if (m_clientProp == nullptr) {
		// TODO: set back in public mode ...
		return;
	}
	store_db();
	// Update viewer to show all ...
	if (m_listViewer != nullptr) {
		m_listViewer->setClientProperty(m_clientProp);
		m_listViewer->searchElements();
	}
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
	if (tmpDisp != nullptr) {
		// stop previous (if needed)
		tmpDisp->stop();
		// Set new file:
		tmpDisp->setFile(m_list[m_id]);
		tmpDisp->play();
		echrono::Duration time = tmpDisp->getDuration();
		APPL_DEBUG("duration = " << time << "  " << etk::to_string(time.toSeconds()));
		propertySetOnWidgetNamed("progress-bar", "value", "0");
		propertySetOnWidgetNamed("progress-bar", "max", etk::to_string(time.toSeconds()));
	}
}

void appl::Windows::onCallbackBack() {
	
}

void appl::Windows::onCallbackPlay(const bool& _isPressed) {
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp == nullptr) {
		return;
	}
	if (_isPressed == true) {
		tmpDisp->play();
	} else {
		tmpDisp->pause();
	}
}

void appl::Windows::onCallbackNext() {
	m_id++;
	if (m_id >= m_list.size()) {
		m_id = 0;
	}
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != nullptr) {
		// stop previous (if needed)
		tmpDisp->stop();
		// Set new file:
		tmpDisp->setFile(m_list[m_id]);
		tmpDisp->play();
		echrono::Duration time = tmpDisp->getDuration();
		APPL_DEBUG("duration = " << time << "  " << etk::to_string(time.toSeconds()));
		propertySetOnWidgetNamed("progress-bar", "value", "0");
		propertySetOnWidgetNamed("progress-bar", "max", etk::to_string(time.toSeconds()));
	}
}


void appl::Windows::onCallbackFPS(const int32_t& _fps) {
	APPL_DEBUG("FPS = " << _fps);
	propertySetOnWidgetNamed("lb-fps", "value", "FPS=<font color='orangered'>" + etk::to_string(_fps) + "</font>");
}

void appl::Windows::addFile(const std::string& _file) {
	APPL_DEBUG("Add file : " << _file);
	m_list.push_back(_file);
	if (m_list.size() == 1) {
		m_id = 0;
		ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
		if (tmpDisp != nullptr) {
			tmpDisp->setFile(m_list[m_id]);
			echrono::Duration time = tmpDisp->getDuration();
			APPL_DEBUG("duration = " << time << "  " << etk::to_string(time.toSeconds()));
			propertySetOnWidgetNamed("progress-bar", "value", "0");
			propertySetOnWidgetNamed("progress-bar", "max", etk::to_string(time.toSeconds()));
		}
	}
}

void appl::Windows::onCallbackPosition(const echrono::Duration& _time) {
	APPL_DEBUG("time = " << _time);
	propertySetOnWidgetNamed("lb-time", "value", "<font color='green'>" + etk::to_string(_time) + "</font>");
	propertySetOnWidgetNamed("progress-bar", "value", etk::to_string(_time.toSeconds()));
}

void appl::Windows::onCallbackSeekRequest(const float& _value) {
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != nullptr) {
		tmpDisp->seek(echrono::Duration(_value));
	}
}

void appl::Windows::onCallbackSelectFilms() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("film");
}
void appl::Windows::onCallbackSelectAnnimation() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("annimation");
}
void appl::Windows::onCallbackSelectTVShow() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("tv-show");
}
void appl::Windows::onCallbackSelectTvAnnimation() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("tv-annimation");
}
void appl::Windows::onCallbackSelectTeather() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("theater");
}
void appl::Windows::onCallbackSelectOneManShow() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("one-man");
}
void appl::Windows::onCallbackSelectSourses() {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list-viewer");
	m_listViewer->searchElements("courses");
}

void appl::Windows::onCallbackSelectMedia(const uint32_t& _value) {
	ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-player");
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != nullptr) {
		// stop previous (if needed)
		tmpDisp->stop();
		// Set new file:
		tmpDisp->setZeusMedia(m_clientProp, _value);
		tmpDisp->play();
		echrono::Duration time = tmpDisp->getDuration();
		APPL_DEBUG("duration = " << time << "  " << etk::to_string(time.toSeconds()));
		propertySetOnWidgetNamed("progress-bar", "value", "0");
		propertySetOnWidgetNamed("progress-bar", "max", etk::to_string(time.toSeconds()));
	}
}

