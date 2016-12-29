/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <appl/debug.hpp>
#include <appl/widget/ListViewer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>


appl::widget::ListViewer::ListViewer() {
	addObjectType("appl::widget::ListViewer");
	
}

void appl::widget::ListViewer::init() {
	ewol::Widget::init();
	markToRedraw();
	m_compImageVideo.setSource("DATA:Video.svg", 128);
	m_compImageAudio.setSource("DATA:MusicNote.svg", 128);
}

appl::widget::ListViewer::~ListViewer() {
	
}


void appl::widget::ListViewer::onDraw() {
	m_draw.draw();
	m_compImageVideo.draw();
	m_compImageAudio.draw();
	m_text.draw();
	
}

void appl::widget::ListViewer::onRegenerateDisplay() {
	//!< Check if we really need to redraw the display, if not needed, we redraw the previous data ...
	if (needRedraw() == false) {
		return;
	}
	m_draw.clear();
	m_draw.setPos(vec2(10,10));
	m_draw.setColor(etk::color::blue);
	m_draw.rectangleWidth(m_size-vec2(20,20));
	
	std::u32string errorString = U"No element Availlable";
	/*
	vec3 curentTextSize = m_text.calculateSizeDecorated(errorString);
	
	// clean the element
	m_text.reset();
	m_text.setDefaultColorBg(etk::color::red);
	m_text.setDefaultColorFg(etk::color::green);
	vec2 origin = m_size - vec2(curentTextSize.x(),curentTextSize.y());
	origin *= 0.5f;
	APPL_INFO("Regenerate display : " << origin << "  " << m_origin << "  " << m_size);
	m_text.setPos(origin);
	m_text.setTextAlignement(origin.x(), origin.x(), ewol::compositing::alignLeft);
	m_text.setClipping(origin, m_size);
	m_text.printDecorated(errorString);
	*/
	
	
	
	
	
	
	m_text.clear();
	int32_t paddingSize = 2;
	
	vec2 tmpMax = propertyMaxSize->getPixel();
	// to know the size of one line : 
	vec3 minSize = m_text.calculateSize(char32_t('A'));
	
	/*
	if (tmpMax.x() <= 999999) {
		m_text.setTextAlignement(0, tmpMax.x()-2*paddingSize, ewol::compositing::alignLeft);
	}
	*/
	vec3 curentTextSize = m_text.calculateSizeDecorated(errorString);
	
	ivec2 localSize = m_minSize;
	
	// no change for the text orogin : 
	vec3 tmpTextOrigin((m_size.x() - m_minSize.x()) / 2.0,
	                   (m_size.y() - m_minSize.y()) / 2.0,
	                   0);
	
	if (propertyFill->x() == true) {
		localSize.setX(m_size.x());
		tmpTextOrigin.setX(0);
	}
	if (propertyFill->y() == true) {
		localSize.setY(m_size.y());
		tmpTextOrigin.setY(m_size.y() - 2*paddingSize - curentTextSize.y());
	}
	tmpTextOrigin += vec3(paddingSize, paddingSize, 0);
	localSize -= vec2(2*paddingSize,2*paddingSize);
	
	tmpTextOrigin.setY( tmpTextOrigin.y() + (m_minSize.y()-2*paddingSize) - minSize.y());
	
	vec2 textPos(tmpTextOrigin.x(), tmpTextOrigin.y());
	
	vec3 drawClippingPos(paddingSize, paddingSize, -0.5);
	vec3 drawClippingSize((m_size.x() - paddingSize),
	                      (m_size.y() - paddingSize),
	                      1);
	
	// clean the element
	m_text.reset();
	m_text.setDefaultColorFg(etk::color::red);
	m_text.setPos(tmpTextOrigin);
	APPL_INFO("Regenerate display : " << tmpTextOrigin << "  " << m_origin << "  " << m_size);
	//APPL_VERBOSE("[" << getId() << "] {" << errorString << "} display at pos : " << tmpTextOrigin);
	m_text.setTextAlignement(tmpTextOrigin.x(), tmpTextOrigin.x()+localSize.x(), ewol::compositing::alignLeft);
	m_text.setClipping(drawClippingPos, drawClippingSize);
	m_text.printDecorated(errorString);
	
	
	
	
}

