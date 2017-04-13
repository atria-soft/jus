/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/widget/ProgressBar.hpp>

#include <ewol/widget/Manager.hpp>

const int32_t dotRadius = 6;

appl::widget::ProgressBar::ProgressBar() :
  signalChange(this, "change", ""),
  propertyValue(this, "value",
                      0.0f,
                      "Value of the ProgressBar",
                      &appl::widget::ProgressBar::onChangePropertyValue),
  propertyMaximum(this, "max",
                        10.0f,
                        "Maximum value",
                        &appl::widget::ProgressBar::onChangePropertyMaximum) {
	addObjectType("appl::widget::ProgressBar");
	
	m_textColorFg = etk::color::orange;
	m_textColorLoaded = etk::color::red;
	m_textColorDone = etk::color::green;
	
	m_textColorBg = etk::color::black;
	m_textColorBg.setA(0x3F);
	
	propertyCanFocus.setDirectCheck(true);
	// Limit event at 1:
	setMouseLimit(1);
}

appl::widget::ProgressBar::~ProgressBar() {
	
}

void appl::widget::ProgressBar::calculateMinMaxSize() {
	vec2 minTmp = propertyMinSize->getPixel();
	m_minSize.setValue(std::max(minTmp.x(), 40.0f),
	                   std::max(minTmp.y(), dotRadius*2.0f) );
	markToRedraw();
}

void appl::widget::ProgressBar::onDraw() {
	m_draw.draw();
}

void appl::widget::ProgressBar::onRegenerateDisplay() {
	if (needRedraw() == false) {
		return;
	}
	// clean the object list ...
	m_draw.clear();
	m_draw.setColor(m_textColorFg);
	// draw a line:
	m_draw.setPos(vec3(dotRadius, 0, 0));
	m_draw.rectangleWidth(vec3(m_size.x()-dotRadius*2.0, m_size.y(), 0));
	
	// draw all availlable section ...
	m_draw.setColor(m_textColorLoaded);
	for (auto &it: m_listAvaillable) {
		//APPL_INFO("plop        " << it.first << " " << it.second);
		m_draw.setPos(vec3(dotRadius+(it.first/propertyMaximum)*(m_size.x()-2*dotRadius), m_size.y()*0.1, 0));
		m_draw.rectangleWidth(vec3(std::min((m_size.x()-2*dotRadius), (it.second/propertyMaximum)*(m_size.x()-2*dotRadius)), m_size.y()*0.8, 0) );
	}
	
	m_draw.setColor(m_textColorDone);
	m_draw.setPos(vec3(dotRadius, m_size.y()*0.3, 0));
	m_draw.rectangleWidth(vec3((propertyValue/propertyMaximum)*(m_size.x()-2*dotRadius), m_size.y()*0.4, 0) );
	
	
	
	
	etk::Color<> borderDot = m_textColorFg;
	borderDot.setA(borderDot.a()/2);
	m_draw.setPos(vec3(4+(propertyValue/propertyMaximum)*(m_size.x()-2*dotRadius), m_size.y()/2, 0) );
	m_draw.setColorBg(borderDot);
	m_draw.circle(dotRadius);
	m_draw.setColorBg(m_textColorFg);
	m_draw.circle(dotRadius/1.6);
}

bool appl::widget::ProgressBar::onEventInput(const ewol::event::Input& _event) {
	vec2 relativePos = relativePosition(_event.getPos());
	//EWOL_DEBUG("Event on ProgressBar ..." << _event);
	if (1 == _event.getId()) {
		if(    gale::key::status::pressSingle == _event.getStatus()
		    || gale::key::status::move   == _event.getStatus()) {
			// get the new position :
			EWOL_VERBOSE("Event on ProgressBar " << relativePos);
			float oldValue = *propertyValue;
			updateValue((float)(relativePos.x() - dotRadius) / (m_size.x()-2*dotRadius) * (*propertyMaximum));
			if (oldValue != *propertyValue) {
				EWOL_VERBOSE(" new value : " << *propertyValue << " in [0.." << *propertyMaximum << "]");
				signalChange.emit(*propertyValue);
			}
			return true;
		}
	}
	return false;
}

void appl::widget::ProgressBar::updateValue(float _newValue) {
	_newValue = std::max(std::min(_newValue, *propertyMaximum), 0.0f);
	propertyValue.setDirect(_newValue);
	markToRedraw();
}


void appl::widget::ProgressBar::onChangePropertyValue() {
	updateValue(*propertyValue);
	return;
}

void appl::widget::ProgressBar::onChangePropertyMaximum() {
	updateValue(*propertyValue);
	return;
}


