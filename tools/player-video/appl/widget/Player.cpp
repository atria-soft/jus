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
//#include <vector>
#include <vector>
#include <etk/tool.hpp>
#include <appl/debug.hpp>

#include <ewol/ewol.hpp>
#include <ewol/tools/message.hpp>

appl::widget::Player::Player() {
	addObjectType("appl::widget::Player");
}

void appl::widget::Player::init() {
	ewol::widget::Composer::init();
	if (loadFromFile("DATA:gui-player.xml", getId()) == false) {
		APPL_ERROR("Can not load Player GUI from file ...");
		return;
	}
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-previous", signalPressed, sharedFromThis(), &appl::widget::Player::onCallbackButtonPrevious);
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-play", signalValue, sharedFromThis(), &appl::widget::Player::onCallbackButtonPlay);
	subBind(ewol::widget::Button, "[" + etk::to_string(getId()) + "]appl-player-bt-next", signalPressed, sharedFromThis(), &appl::widget::Player::onCallbackButtonNext);
	propertyCanFocus.set(true);
	markToRedraw();
}


void appl::widget::Player::onGetFocus() {
	// transfert focus on a specific widget...
	propertySetOnWidgetNamed("[" + etk::to_string(getId()) + "]appl-player-bt-play", "focus", "true");
}

appl::widget::Player::~Player() {
	
}

void appl::widget::Player::onCallbackButtonPrevious() {
	
}

void appl::widget::Player::onCallbackButtonPlay(const bool& _value) {
	
}

void appl::widget::Player::onCallbackButtonNext() {
	
}


