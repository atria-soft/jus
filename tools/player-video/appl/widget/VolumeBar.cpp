/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/widget/VolumeBar.hpp>

#include <ewol/widget/Manager.hpp>
#include <ewol/object/Manager.hpp>

appl::widget::VolumeBar::VolumeBar() :
  signalChange(this, "change", ""),
  signalHide(this, "hide", ""),
  propertyValue(this, "value",
                      0.0f,
                      "Value of the VolumeBar",
                      &appl::widget::VolumeBar::onChangePropertyValue),
  propertyStep(this, "step",
                      1.0f,
                      "step value when change"),
  propertyMinimum(this, "min",
                        -10.0f,
                        "Minimum value",
                        &appl::widget::VolumeBar::onChangePropertyMinimum),
  propertyMaximum(this, "max",
                        5.0f,
                        "Maximum value",
                        &appl::widget::VolumeBar::onChangePropertyMaximum),
  propertyDanger(this, "danger",
                        4.0f,
                        "Danger value"),
  m_isHidden(false),
  m_lastEventTime(echrono::Time::now()) {
	addObjectType("appl::widget::VolumeBar");
	
	m_textColorFg = etk::color::orange;
	m_textColorLoaded = etk::color::red;
	m_textColorDone = etk::color::green;
	
	m_textColorBg = etk::color::black;
	m_textColorBg.setA(0x3F);
	
	propertyCanFocus.setDirectCheck(true);
	
	m_PCH = getObjectManager().periodicCall.connect(this, &appl::widget::VolumeBar::periodicCall);
	
	// Limit event at 1:
	setMouseLimit(1);
}


void appl::widget::VolumeBar::periodicCall(const ewol::event::Time& _event) {
	if (echrono::Time::now() - m_lastEventTime > echrono::seconds(3)) {
		m_isHidden = true;
		signalHide.emit(0.0f);
	}
	if (m_isHidden == true) {
		m_PCH.disconnect();
	}
	markToRedraw();
}

appl::widget::VolumeBar::~VolumeBar() {
	
}

void appl::widget::VolumeBar::calculateMinMaxSize() {
	vec2 minTmp = propertyMinSize->getPixel();
	m_minSize.setValue(std::max(minTmp.x(), 40.0f),
	                   std::max(minTmp.y(), std::max(minTmp.x(), 40.0f)) );
	markToRedraw();
}

void appl::widget::VolumeBar::onDraw() {
	if (m_isHidden == true) {
		return;
	}
	m_draw.draw();
}

void appl::widget::VolumeBar::onRegenerateDisplay() {
	if (needRedraw() == false) {
		return;
	}
	// clean the object list ...
	m_draw.clear();
	m_draw.setColor(m_textColorFg);
	
	// draw a line:
	#if 0
		m_draw.setPos(vec2(0.0f, 0.0f));
		m_draw.rectangleWidth(vec2(m_size.x(), m_size.y()));
	#else
		m_draw.setThickness(4.0f);
		m_draw.setPos(vec2(0.0f, 0.0f));
		m_draw.lineRel(vec2(m_size.x(), 0.0f));
		m_draw.lineRel(vec2(0.0f, m_size.y()));
		m_draw.lineRel(vec2(-m_size.x(), 0.0f));
		m_draw.lineRel(vec2(0.0f, -m_size.y()));
	#endif
	// chaneg color whe soud became louder ...
	if (*propertyValue > *propertyDanger) {
		m_draw.setColor(m_textColorLoaded);
	} else {
		m_draw.setColor(m_textColorDone);
	}
	m_draw.setPos(vec3(m_size.x()*0.1f, m_size.x()*0.1f, 0.0f));
	
	float offset = (*propertyValue-*propertyMinimum)/(*propertyMaximum-*propertyMinimum);
	m_draw.rectangleWidth(vec3(m_size.x()*0.8f, offset*(m_size.y()-m_size.x()*0.2f), 0.0f) );
	
}

bool appl::widget::VolumeBar::onEventInput(const ewol::event::Input& _event) {
	m_lastEventTime = echrono::Time::now();
	if (m_isHidden == true) {
		m_isHidden = false;
		signalHide.emit(1.0f);
		m_PCH = getObjectManager().periodicCall.connect(this, &appl::widget::VolumeBar::periodicCall);
	}
	vec2 relativePos = relativePosition(_event.getPos());
	//EWOL_DEBUG("Event on VolumeBar ..." << _event);
	if (_event.getId() == 1) {
		if(    _event.getStatus() == gale::key::status::pressSingle
		    || _event.getStatus() == gale::key::status::move) {
			// get the new position :
			EWOL_VERBOSE("Event on VolumeBar " << relativePos);
			float oldValue = *propertyValue;
			updateValue((float)(relativePos.y() - m_size.x()*0.1f) / (m_size.y()-m_size.x()*0.2f) * (*propertyMaximum-*propertyMinimum)+*propertyMinimum);
			if (oldValue != *propertyValue) {
				EWOL_VERBOSE(" new value : " << *propertyValue << " in [" << *propertyMinimum << ".." << *propertyMaximum << "]");
				signalChange.emit(*propertyValue);
			}
			return true;
		}
	} else if (_event.getId() == 4) {
		if(_event.getStatus() == gale::key::status::pressSingle) {
			float oldValue = *propertyValue;
			updateValue(*propertyValue + *propertyStep);
			if (oldValue != *propertyValue) {
				EWOL_VERBOSE(" new value : " << *propertyValue << " in [" << *propertyMinimum << ".." << *propertyMaximum << "]");
				signalChange.emit(*propertyValue);
			}
			return true;
		}
	} else if (_event.getId() == 5) {
		if(_event.getStatus() == gale::key::status::pressSingle) {
			float oldValue = *propertyValue;
			updateValue(*propertyValue - *propertyStep);
			if (oldValue != *propertyValue) {
				EWOL_VERBOSE(" new value : " << *propertyValue << " in [" << *propertyMinimum << ".." << *propertyMaximum << "]");
				signalChange.emit(*propertyValue);
			}
			return true;
		}
	}
	return false;
}

void appl::widget::VolumeBar::updateValue(float _newValue) {
	_newValue = std::max(std::min(_newValue, *propertyMaximum), *propertyMinimum);
	propertyValue.setDirect(_newValue);
	markToRedraw();
}


void appl::widget::VolumeBar::onChangePropertyValue() {
	updateValue(*propertyValue);
	return;
}

void appl::widget::VolumeBar::onChangePropertyMaximum() {
	updateValue(*propertyValue);
	return;
}

void appl::widget::VolumeBar::onChangePropertyMinimum() {
	updateValue(*propertyValue);
	return;
}


