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

static std::string g_baseDBName = "USERDATA:config.json";

void appl::Windows::store_db() {
	APPL_DEBUG("Store database [START]");
	ejson::Document database;
	database.add("login", ejson::String(m_login));
	database.add("pass", ejson::String(m_password));
	bool retGenerate = database.storeSafe(g_baseDBName);
	APPL_ERROR("Store database [STOP] : " << (g_baseDBName) << " ret = " << retGenerate);
}

void appl::Windows::load_db() {
	ejson::Document database;
	bool ret = database.load(g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	m_login = database["login"].toString().get();
	m_password = database["pass"].toString().get();
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
	
	
	subBind(ewol::widget::Entry, "connect-login", signalModify, sharedFromThis(), &appl::Windows::onCallbackConnectLogin);
	subBind(ewol::widget::Entry, "connect-password", signalModify, sharedFromThis(), &appl::Windows::onCallbackConnectPassword);
	subBind(ewol::widget::Button, "connect-bt", signalPressed, sharedFromThis(), &appl::Windows::onCallbackConnectConnect);
	
	
	subBind(ewol::widget::Button, "bt-film-picture", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectFilms);
	subBind(ewol::widget::Button, "bt-film-draw", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectAnnimation);
	subBind(ewol::widget::Button, "bt-tv-picture", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTVShow);
	subBind(ewol::widget::Button, "bt-tv-draw", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTvAnnimation);
	subBind(ewol::widget::Button, "bt-theater", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectTeather);
	subBind(ewol::widget::Button, "bt-one-man-show", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectOneManShow);
	subBind(ewol::widget::Button, "bt-courses", signalPressed, sharedFromThis(), &appl::Windows::onCallbackSelectSourses);
	
	
	propertySetOnWidgetNamed("connect-login", "value", m_login);
	propertySetOnWidgetNamed("connect-password", "value", m_password);
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


void appl::Windows::onCallbackConnectLogin(const std::string& _value) {
	m_login = _value;
	store_db();
}

void appl::Windows::onCallbackConnectPassword(const std::string& _value) {
	m_password = _value;
	store_db();
}

void appl::Windows::onCallbackConnectConnect() {
	APPL_INFO("Connect with : '" << m_login << "' ... '" << m_password << "'");
	
	m_clientProp = ememory::makeShared<ClientProperty>();
	if (m_clientProp == nullptr) {
		APPL_ERROR("Can not create property");
		return;
	}
	m_clientProp->address = "127.0.0.1";
	m_clientProp->port = 1983;
	
	// check connection is correct:
	zeus::Client client1;
	// separate loggin and IP adress ...
	std::string login;
	std::vector<std::string> listElem = etk::split(m_login, '~');
	login = listElem[0];
	if (listElem.size() == 1) {
		// connnect on local host ... nothing to do
	} else {
		std::vector<std::string> listElem2 = etk::split(listElem[0], ':');
		client1.propertyIp.set(listElem2[0]);
		m_clientProp->address = listElem2[0];
		if (listElem2.size() >= 1) {
			client1.propertyPort.set(etk::string_to_uint32_t(listElem2[1]));
			m_clientProp->port = etk::string_to_uint32_t(listElem2[1]);
		}
	}
	m_clientProp->fromUser = login;
	m_clientProp->toUser = login;
	m_clientProp->pass = m_password;
	m_clientProp->connect();
	
	if (m_clientProp->connection.isAlive() == false) {
		APPL_ERROR("    ==> NOT Authentify to '" << login << "'");
		ewol::tools::message::displayError("Can not connect the server with <br/>'" + login + "'");
	} else {
		APPL_INFO("    ==> Authentify with '" << login << "'");
		ewol::propertySetOnObjectNamed("view-selection", "select", "ws-name-list");
		// set the client property interface:
		if (m_listViewer == nullptr) {
			APPL_ERROR("Nullptr in the viewer ...");
		} else {
			m_listViewer->setClientProperty(m_clientProp);
		}
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

