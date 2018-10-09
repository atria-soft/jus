/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


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
#include <etk/Vector.hpp>
#include <etk/tool.hpp>
#include <appl/debug.hpp>

#include <ewol/ewol.hpp>
#include <etk/stdTools.hpp>
#include <ewol/tools/message.hpp>

appl::widget::UpBar::UpBar() :
  propertyType(this, "bar-type",
                      "sound",
                      "type of the element",
                      &appl::widget::UpBar::onChangePropertyType),
  propertyValue(this, "value",
                      0,
                      "value of the bar",
                      &appl::widget::UpBar::onChangePropertyValue),
  signalChange(this, "value", "Value of the upbar") {
	addObjectType("appl::widget::UpBar");
}

void appl::widget::UpBar::init() {
	ewol::widget::Composer::init();
	if (*propertySubFile == "") {
		if (*propertyType == "volume") {
			propertySubFile.set("DATA:///gui-volume.xml");
		} else if (*propertyType == "light") {
			propertySubFile.set("DATA:///gui-light.xml");
		} else {
			APPL_ERROR("can not set the mode of upBar");
		}
	}
	if (*propertyType == "volume") {
		subBind(appl::widget::VolumeBar, "[" + etk::toString(getId()) + "]appl-upbar-range", signalChange, sharedFromThis(), &appl::widget::UpBar::onCallbackVolumeRequest);
	} else {
		subBind(appl::widget::VolumeBar, "[" + etk::toString(getId()) + "]appl-upbar-range", signalChange, sharedFromThis(), &appl::widget::UpBar::onCallbackLightRequest);
	}
	subBind(appl::widget::VolumeBar, "[" + etk::toString(getId()) + "]appl-upbar-range", signalHide, sharedFromThis(), &appl::widget::UpBar::onCallbackHide);
	markToRedraw();
}

appl::widget::UpBar::~UpBar() {
	
}

void appl::widget::UpBar::onCallbackVolumeRequest(const float& _value) {
	APPL_DEBUG("===========================================================================");
	APPL_DEBUG("volume change value=" << _value << " dB");
	APPL_DEBUG("===========================================================================");
	signalChange.emit(_value);
	etk::String display = etk::toString(int32_t(_value)) + "." + etk::toString(etk::abs(int32_t(_value*10.0f)-int32_t(_value)*10));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-label", "value", display + " dB");
}

void appl::widget::UpBar::onCallbackLightRequest(const float& _value) {
	APPL_DEBUG("===========================================================================");
	APPL_DEBUG("Light change value=" << _value << " %");
	APPL_DEBUG("===========================================================================");
	signalChange.emit(float(int32_t(_value))*0.01f);
	etk::String display = etk::toString(int32_t(_value));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-label", "value", display + " %");
}

void appl::widget::UpBar::onCallbackHide(const float& _value) {
	if (_value == 0.0f) {
		propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-label", "hide", etk::toString(true));
		propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-image", "hide", etk::toString(true));
	} else {
		propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-label", "hide", etk::toString(false));
		propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-image", "hide", etk::toString(false));
	}
}


void appl::widget::UpBar::onChangePropertyType() {
	
}

void appl::widget::UpBar::onChangePropertyValue() {
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-upbar-range", "value", etk::toString(*propertyValue));
	if (*propertyType == "volume") {
		onCallbackVolumeRequest(*propertyValue);
	} else {
		onCallbackLightRequest(*propertyValue);
	}
	return;
}