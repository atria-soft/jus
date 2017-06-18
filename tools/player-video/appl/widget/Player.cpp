/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


#include <appl/widget/Player.hpp>
#include <appl/widget/UpBar.hpp>
#include <ewol/widget/Sizer.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/CheckBox.hpp>
#include <ewol/widget/ListFileSystem.hpp>
#include <ewol/widget/Entry.hpp>
#include <ewol/widget/Spacer.hpp>
#include <ewol/widget/Slider.hpp>
#include <ewol/widget/Image.hpp>
#include <ewol/widget/Composer.hpp>
#include <ewol/widget/Manager.hpp>
#include <vector>
#include <etk/tool.hpp>
#include <appl/debug.hpp>

#include <ewol/ewol.hpp>
#include <ewol/tools/message.hpp>

appl::widget::Player::Player() :
  signalFinished(this, "finish", "The playing of the stream is finished"),
  signalNext(this, "next", "User request the next stream"),
  signalPrevious(this, "previous", "User request the previous stream") {
	addObjectType("appl::widget::Player");
}

void appl::widget::Player::init() {
	ewol::widget::Composer::init();
	if (*propertySubFile == "") {
		propertySubFile.set("DATA:gui-player.xml");
	}
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-previous", signalPressed, sharedFromThis(), &appl::widget::Player::onCallbackButtonPrevious);
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-play", signalValue, sharedFromThis(), &appl::widget::Player::onCallbackButtonPlay);
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-next", signalPressed, sharedFromThis(), &appl::widget::Player::onCallbackButtonNext);
	
	subBind(appl::widget::VideoDisplay, "[" + etk::to_string(getId()) + "]appl-player-display", signalOther, sharedFromThis(), &appl::widget::Player::onCallbackOther);
	subBind(appl::widget::VideoDisplay, "[" + etk::to_string(getId()) + "]appl-player-display", signalPosition, sharedFromThis(), &appl::widget::Player::onCallbackPosition);
	subBind(appl::widget::VideoDisplay, "[" + etk::to_string(getId()) + "]appl-player-display", signalDuration, sharedFromThis(), &appl::widget::Player::onCallbackDuration);
	subBind(appl::widget::VideoDisplay, "[" + etk::to_string(getId()) + "]appl-player-display", signalFinish, sharedFromThis(), &appl::widget::Player::onCallbackFinished);
	
	subBind(appl::widget::VideoDisplay, "[" + etk::to_string(getId()) + "]appl-player-display", signalFps, sharedFromThis(), &appl::widget::Player::onCallbackFPS);
	subBind(appl::widget::ProgressBar, "[" + etk::to_string(getId()) + "]appl-player-progress-bar", signalChange, sharedFromThis(), &appl::widget::Player::onCallbackSeekRequest);
	subBind(appl::widget::UpBar, "[" + etk::to_string(getId()) + "]appl-player-volume", signalChange, sharedFromThis(), &appl::widget::Player::onCallbackVolumeRequest);
	subBind(appl::widget::UpBar, "[" + etk::to_string(getId()) + "]appl-player-light", signalChange, sharedFromThis(), &appl::widget::Player::onCallbackLightRequest);
	
	m_display = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("[" + etk::to_string(getId()) + "]appl-player-display"));
	m_progress = ememory::dynamicPointerCast<appl::widget::ProgressBar>(getSubObjectNamed("[" + etk::to_string(getId()) + "]appl-player-progress-bar"));
	propertyCanFocus.set(true);
	markToRedraw();
}


void appl::widget::Player::onGetFocus() {
	// transfert focus on a specific widget...
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "focus", "true");
}

appl::widget::Player::~Player() {
	
}

void appl::widget::Player::playStream(ememory::SharedPtr<appl::ClientProperty> _property, uint32_t _mediaId) {
	if (m_display == nullptr) {
		return;
	}
	// stop previous (if needed)
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "value", "false");
	m_display->stop();
	// Set new file:
	m_display->setZeusMedia(_property, _mediaId);
	m_display->play();
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "value", "true");
}


static std::string timeToStaticString(const echrono::Duration& _time) {
	float sec = _time.toSeconds();
	int32_t millisecond = int32_t(sec*1000.0f)%999;
	int32_t seconds = int32_t(sec)%60;
	int32_t minutes = int32_t(sec/60)%60;
	int32_t hours = sec/3600;
	std::string out;
	if (hours!=0) {
		out += etk::to_string(hours) + ":";
	}
	if (minutes<10) {
		out += " " + etk::to_string(minutes) + "'";
	} else {
		out += etk::to_string(minutes) + "'";
	}
	if (seconds<10) {
		out += " " + etk::to_string(seconds) + "\"";
	} else {
		out += etk::to_string(seconds) + "\"";
	}
	if (millisecond<10) {
		out += " 00" + etk::to_string(millisecond);
	} else if (millisecond<100) {
		out += " 0" + etk::to_string(millisecond);
	} else {
		out += etk::to_string(millisecond);
	}
	return out;
}

void appl::widget::Player::onCallbackFinished() {
	if (m_progress != nullptr) {
		std::vector<std::pair<float,float>> tmp;
		m_progress->setRangeAvaillable(tmp);
		m_progress->propertyValue.set(0);
		m_progress->propertyMaximum.set(0);
	}
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-time", "value", "<font color='black'>--:--</font>");
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-duration", "value", "<font color='black'>--:--</font>");
	signalFinished.emit();
}


void appl::widget::Player::onCallbackDuration(const echrono::Duration& _time) {
	//APPL_ERROR("duration = " << _time);
	if (m_progress != nullptr) {
		m_progress->propertyValue.set(0);
		m_progress->propertyMaximum.set(_time.toSeconds());
	}
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-duration", "value", "<font color='black'>" + timeToStaticString(_time) + "</font>");
}

void appl::widget::Player::onCallbackPosition(const echrono::Duration& _time) {
	//APPL_ERROR("time = " << _time);
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-time", "value", "<font color='black'>" + timeToStaticString(_time) + "</font>");
	if (m_progress != nullptr) {
		m_progress->propertyValue.set(_time.toSeconds());
	}
	if (m_display != nullptr) {
		std::vector<std::pair<float,float>> tmp = m_display->getDownloadPart();
		if (m_progress != nullptr) {
			m_progress->setRangeAvaillable(tmp);
		}
	}
}

void appl::widget::Player::onCallbackSeekRequest(const float& _value) {
	APPL_DEBUG("===========================================================================");
	APPL_DEBUG("seek at = " << echrono::Duration(_value) << "  from value=" << _value);
	APPL_DEBUG("===========================================================================");
	if (m_display != nullptr) {
		m_display->seek(echrono::Duration(_value));
	}
}

void appl::widget::Player::onCallbackVolumeRequest(const float& _value) {
	APPL_DEBUG("===========================================================================");
	APPL_DEBUG("volume change value=" << _value << " dB");
	APPL_DEBUG("===========================================================================");
	if (m_display != nullptr) {
		m_display->changeVolume(_value);
	}
}

void appl::widget::Player::onCallbackLightRequest(const float& _value) {
	APPL_DEBUG("===========================================================================");
	APPL_DEBUG("volume change value=" << _value << " %");
	APPL_DEBUG("===========================================================================");
	if (m_display != nullptr) {
		m_display->changeLight(_value);
	}
}


void appl::widget::Player::onCallbackOther(const std::string& _plop) {
	APPL_DEBUG("FPS = " << _plop);
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-fps", "value", "FPS=<font color='orangered'>" + _plop + "</font>");
}

void appl::widget::Player::onCallbackFPS(const int32_t& _fps) {
	APPL_DEBUG("FPS = " << _fps);
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-label-fps", "value", "FPS=<font color='orangered'>" + etk::to_string(_fps) + "</font>");
}


void appl::widget::Player::stop() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "value", "false");
	if (m_display == nullptr) {
		return;
	}
	m_display->stop();
}

void appl::widget::Player::suspend() {
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "value", "false");
	if (m_display == nullptr) {
		return;
	}
	m_display->pause();
}


void appl::widget::Player::onCallbackButtonPrevious() {
	signalPrevious.emit();
}

void appl::widget::Player::onCallbackButtonPlay(const bool& _value) {
	if (m_display == nullptr) {
		return;
	}
	if (_value == true) {
		m_display->play();
	} else {
		m_display->pause();
	}
}

void appl::widget::Player::onCallbackButtonNext() {
	signalNext.emit();
}


