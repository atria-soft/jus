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
	composition += "		<button name='bt-play'>\n";
	composition += "			<label>\n";
	composition += "				play/pause\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "		<button name='bt-next'>\n";
	composition += "			<label>\n";
	composition += "				Next\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "		<label name='lb-fps'/>\n";
	composition += "	</sizer>\n";
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
	subBind(ewol::widget::Button, "bt-play", signalPressed, sharedFromThis(), &appl::Windows::onCallbackPlay);
	subBind(ewol::widget::Button, "bt-next", signalPressed, sharedFromThis(), &appl::Windows::onCallbackNext);
	subBind(appl::widget::VideoDisplay, "displayer", signalFps, sharedFromThis(), &appl::Windows::onCallbackFPS);
}



void appl::Windows::onCallbackPrevious() {
	m_id--;
	if (m_id < 0) {
		m_id = m_list.size()-1;
	}
	onCallbackPlay();
}

void appl::Windows::onCallbackPlay() {
	ememory::SharedPtr<appl::widget::VideoDisplay> tmpDisp = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(getSubObjectNamed("displayer"));
	if (tmpDisp != nullptr) {
		tmpDisp->setFile(m_list[m_id]);
	}
}

void appl::Windows::onCallbackNext() {
	m_id++;
	if (m_id >= m_list.size()) {
		m_id = 0;
	}
	onCallbackPlay();
}


void appl::Windows::onCallbackFPS(const int32_t& _fps) {
	APPL_PRINT("FPS = " << _fps);
	propertySetOnWidgetNamed("lb-fps", "value", "FPS=<font color='orangered'>" + etk::to_string(_fps) + "</font>");
}

void appl::Windows::addFile(const std::string& _file) {
	APPL_PRINT("Add file : " << _file);
	m_list.push_back(_file);
}