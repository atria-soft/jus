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
#include <ewol/widget/Slider.hpp>
#include <appl/widget/VideoPlayer.hpp>

appl::Windows::Windows():
  m_id(0) {
	addObjectType("appl::Windows");
	propertyTitle.setDirectCheck(std::string("sample ") + PROJECT_NAME);
}

void appl::Windows::init() {
	ewol::widget::Windows::init();
	std::string composition = std::string("");
	composition += "<sizer mode='vert'>\n";
	composition += "	<sizer mode='hori'>\n";
	composition += "		<button name='bt-previous'>\n";
	composition += "			<label>\n";
	composition += "				previous\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "		<button name='bt-play' toggle='true'>\n";
	composition += "			<label>play</label>\n";
	composition += "			<label>pause</label>\n";
	composition += "		</button>\n";
	composition += "		<button name='bt-next'>\n";
	composition += "			<label>\n";
	composition += "				Next\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "		<label name='lb-fps'/>\n";
	composition += "		<label name='lb-time'/>\n";
	composition += "	</sizer>\n";
	composition += "	<slider name='progress-bar' expand='true,false' fill='true' step='0.01' min='0'/>\n";
	composition += "	<VideoDisplay name='displayer' expand='true' fill='true'/>\n";
	composition += "</sizer>\n";
	
	m_composer = ewol::widget::Composer::create();
	if (m_composer == nullptr) {
		APPL_CRITICAL(" An error occured ... in the windows creatrion ...");
		return;
	}
	m_composer->loadFromString(composition);
	setSubWidget(m_composer);
	subBind(ewol::widget::Button, "bt-previous", signalPressed, sharedFromThis(), &appl::Windows::onCallbackPrevious);
	subBind(ewol::widget::Button, "bt-play", signalValue, sharedFromThis(), &appl::Windows::onCallbackPlay);
	subBind(ewol::widget::Button, "bt-next", signalPressed, sharedFromThis(), &appl::Windows::onCallbackNext);
	subBind(appl::widget::VideoDisplay, "displayer", signalFps, sharedFromThis(), &appl::Windows::onCallbackFPS);
	subBind(appl::widget::VideoDisplay, "displayer", signalPosition, sharedFromThis(), &appl::Windows::onCallbackPosition);
	subBind(ewol::widget::Slider, "progress-bar", signalChange, sharedFromThis(), &appl::Windows::onCallbackSeekRequest);
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


