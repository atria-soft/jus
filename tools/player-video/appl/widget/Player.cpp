/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


#include <appl/widget/Player.hpp>
#include <ewol/widget/Sizer.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/CheckBox.hpp>
#include <ewol/widget/ListFileSystem.hpp>
#include <ewol/widget/Entry.hpp>
#include <ewol/widget/Spacer.hpp>
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
	
	m_display = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("[" + etk::to_string(getId()) + "]appl-player-display"));
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
	//echrono::Duration time = tmpDisp->getDuration();
	//APPL_DEBUG("duration = " << time << "  " << etk::to_string(time.toSeconds()));
	//propertySetOnWidgetNamed("progress-bar", "value", "0");
	//propertySetOnWidgetNamed("progress-bar", "max", etk::to_string(time.toSeconds()));
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "value", "true");
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


