/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <appl/debug.hpp>
#include <appl/widget/VideoPlayer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>

// VBO table property:
const int32_t appl::widget::VideoDisplay::m_vboIdCoord(0);
const int32_t appl::widget::VideoDisplay::m_vboIdCoordTex(1);
const int32_t appl::widget::VideoDisplay::m_vboIdColor(2);
#define NB_VBO (3)

appl::widget::VideoDisplay::VideoDisplay() {
	addObjectType("appl::widget::VideoDisplay");
	m_color = etk::color::white;
	m_nbFramePushed = 0;
	m_isPalying = false;
}

void appl::widget::VideoDisplay::init() {
	ewol::Widget::init();
	markToRedraw();
	// set call all time (sample ...).
	getObjectManager().periodicCall.connect(sharedFromThis(), &appl::widget::VideoDisplay::periodicEvent);
	// Create the VBO:
	m_VBO = gale::resource::VirtualBufferObject::create(NB_VBO);
	if (m_VBO == nullptr) {
		APPL_ERROR("can not instanciate VBO ...");
		return;
	}
	m_VBO->setName("[VBO] of appl::widget::VideoDisplay");
	loadProgram();
	m_matrixApply.identity();
	// By default we load a graphic resource ...
	if (m_resource == nullptr) {
		m_resource = ewol::resource::Texture::create();
		if (m_resource == nullptr) {
			EWOL_ERROR("Can not CREATE Image resource");
			return;
		}
		// All time need to configure in RGB, By default it is in RGBA ...
		m_resource->get().configure(ivec2(128,128), egami::colorType::RGB8);
	}
	// Create the River manager for tha application or part of the application.
	m_audioManager = audio::river::Manager::create("river_sample_read");
	//! [audio_river_sample_create_write_interface]
}

appl::widget::VideoDisplay::~VideoDisplay() {
	
}

void appl::widget::VideoDisplay::loadProgram() {
	// get the shader resource:
	m_GLPosition = 0;
	m_GLprogram.reset();
	m_GLprogram = gale::resource::Program::create("{ewol}DATA:textured3D.prog");
	if (m_GLprogram != nullptr) {
		m_GLPosition = m_GLprogram->getAttribute("EW_coord3d");
		m_GLColor    = m_GLprogram->getAttribute("EW_color");
		m_GLtexture  = m_GLprogram->getAttribute("EW_texture2d");
		m_GLMatrix   = m_GLprogram->getUniform("EW_MatrixTransformation");
		m_GLtexID    = m_GLprogram->getUniform("EW_texID");
	}
}
void appl::widget::VideoDisplay::setFile(const std::string& _filename) {
	// Stop playing in all case...
	stop();
	// Clear the old interface
	m_decoder.reset();
	// Create a new interface
	m_decoder = ememory::makeShared<appl::MediaDecoder>();
	if (m_decoder == nullptr) {
		APPL_ERROR("Can not create sharedPtr on decoder ...");
		return;
	}
	m_decoder->init(_filename);
	markToRedraw();
}

void appl::widget::VideoDisplay::setZeusMedia(ememory::SharedPtr<ClientProperty> _property, uint32_t _mediaId) {
	// Stop playing in all case...
	stop();
	// Clear the old interface
	m_decoder.reset();
	// Create a new interface
	m_decoder = ememory::makeShared<appl::MediaDecoder>();
	if (m_decoder == nullptr) {
		APPL_ERROR("Can not create sharedPtr on decoder ...");
		return;
	}
	m_decoder->init(_property, _mediaId);
	markToRedraw();
}

bool appl::widget::VideoDisplay::isPlaying() {
	return m_isPalying;
}

void appl::widget::VideoDisplay::play() {
	if (m_decoder == nullptr) {
		APPL_WARNING("Request play with no associated decoder");
		return;
	}
	m_isPalying = true;
	if (m_decoder->getState() != gale::Thread::state::stop) {
		// The thread is already active ==> then it is just in pause ...
		APPL_DEBUG("Already started");
		return;
	}
	if (m_decoder->haveAudio() == true) {
		m_audioInterface = m_audioManager->createOutput(m_decoder->audioGetSampleRate(),
		                                                m_decoder->audioGetChannelMap(),
		                                                m_decoder->audioGetFormat(),
		                                                "speaker");
		if(m_audioInterface == nullptr) {
			APPL_ERROR("Can not creata Audio interface");
		}
		m_audioInterface->setReadwrite();
		m_audioInterface->start();
	}
	// Start decoder, this is maybe not the good point, but if we configure a decoder, it is to use it ...
	m_decoder->start();
	//TODO: Set an option to river to auto-generate dot: m_audioManager->generateDotAll("out/local_player_flow.dot");
}

void appl::widget::VideoDisplay::pause() {
	m_isPalying = false;
}

void appl::widget::VideoDisplay::stop() {
	m_isPalying = false;
	if (    m_decoder != nullptr
	     && m_decoder->getState() != gale::Thread::state::stop) {
		APPL_ERROR("Stop Decoder");
		// stop it ... and request seet at 0 position ...
		m_decoder->seek(echrono::Duration(0));
		m_decoder->stop();
	}
	if (m_audioInterface != nullptr) {
		APPL_ERROR("Stop audio interface");
		// Stop audio interface
		m_audioInterface->stop();
		// wait a little to be sure it is done correctly:
		// TODO : Set this in an asynchronous loop ...
		m_audioInterface.reset();
	}
}


void appl::widget::VideoDisplay::onDraw() {
	if (m_VBO->bufferSize(m_vboIdCoord) <= 0) {
		APPL_WARNING("Nothink to draw...");
		return;
	}
	if (m_resource == nullptr) {
		// this is a normale case ... the user can choice to have no image ...
		return;
	}
	if (m_GLprogram == nullptr) {
		APPL_ERROR("No shader ...");
		return;
	}
	gale::openGL::disable(gale::openGL::flag_depthTest);
	mat4 tmpMatrix = gale::openGL::getMatrix()*m_matrixApply;
	m_GLprogram->use();
	m_GLprogram->uniformMatrix(m_GLMatrix, tmpMatrix);
	// TextureID
	if (m_resource != nullptr) {
		m_GLprogram->setTexture0(m_GLtexID, m_resource->getRendererId());
	}
	// position:
	m_GLprogram->sendAttributePointer(m_GLPosition, m_VBO, m_vboIdCoord);
	// Texture:
	m_GLprogram->sendAttributePointer(m_GLtexture, m_VBO, m_vboIdCoordTex);
	// color:
	m_GLprogram->sendAttributePointer(m_GLColor, m_VBO, m_vboIdColor);
	// Request the draw of the elements:
	gale::openGL::drawArrays(gale::openGL::renderMode::triangle, 0, m_VBO->bufferSize(m_vboIdCoord));
	m_GLprogram->unUse();
}

void appl::widget::VideoDisplay::printPart(const vec2& _size,
                                           const vec2& _sourcePosStart,
                                           const vec2& _sourcePosStop) {
	//EWOL_ERROR("Debug image " << m_filename << "  ==> " << m_position << " " << _size << " " << _sourcePosStart << " " << _sourcePosStop);
	vec3 point = m_position;
	vec2 tex(_sourcePosStart.x(),_sourcePosStop.y());
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	
	tex.setValue(_sourcePosStop.x(),_sourcePosStop.y());
	point.setX(m_position.x() + _size.x());
	point.setY(m_position.y());
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	
	tex.setValue(_sourcePosStop.x(),_sourcePosStart.y());
	point.setX(m_position.x() + _size.x());
	point.setY(m_position.y() + _size.y());
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	
	tex.setValue(_sourcePosStart.x(),_sourcePosStart.y());
	point.setX(m_position.x());
	point.setY(m_position.y() + _size.y());
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	
	tex.setValue(_sourcePosStart.x(),_sourcePosStop.y());
	point.setX(m_position.x());
	point.setY(m_position.y());
	m_VBO->pushOnBuffer(m_vboIdCoord, point);
	m_VBO->pushOnBuffer(m_vboIdCoordTex, tex);
	m_VBO->pushOnBuffer(m_vboIdColor, m_color);
	m_VBO->flush();
}

void appl::widget::VideoDisplay::onRegenerateDisplay() {
	//!< Check if we really need to redraw the display, if not needed, we redraw the previous data ...
	if (needRedraw() == false) {
		return;
	}
	// remove previous data
	m_VBO->clear();
	// set the somposition properties :
	m_position = vec3(0,0,0);
	printPart(m_size, vec2(0,0), vec2(float(m_videoSize.x())/float(m_imageSize.x()), float(m_videoSize.y())/float(m_imageSize.y())));
}

void appl::widget::VideoDisplay::periodicEvent(const ewol::event::Time& _event) {
	if (m_isPalying == true) {
		APPL_VERBOSE("tick: " << _event);
		m_currentTime += _event.getDeltaCallDuration();
	}
	if (m_decoder == nullptr) {
		return;
	}
	if (m_decoder->m_seekApply >= echrono::Duration(0)) {
		m_currentTime = m_decoder->m_seekApply;
		m_decoder->m_seekApply = echrono::Duration(-1);
		if (m_audioInterface != nullptr) {
			m_audioInterface->clearInternalBuffer();
		}
	}
	// SET AUDIO:
	int32_t idSlot = m_decoder->audioGetOlderSlot();
	if (    idSlot != -1
	     && m_currentTime > m_decoder->m_audioPool[idSlot].m_time) {
		if (m_audioInterface != nullptr) {
			int32_t nbSample =   m_decoder->m_audioPool[idSlot].m_buffer.size()
			                   / audio::getFormatBytes(m_decoder->m_audioPool[idSlot].m_format)
			                   / m_decoder->m_audioPool[idSlot].m_map.size();
			m_audioInterface->write(&m_decoder->m_audioPool[idSlot].m_buffer[0], nbSample);
		}
		m_decoder->m_audioPool[idSlot].m_isUsed = false;
	}
	// SET VIDEO:
	idSlot = m_decoder->videoGetOlderSlot();
	// check the slot is valid and check display time of the element:
	if (    idSlot != -1
	     && m_currentTime > m_decoder->m_videoPool[idSlot].m_time) {
		m_resource->get().swap(m_decoder->m_videoPool[idSlot].m_image);
		m_imageSize = m_resource->get().getSize();
		ivec2 tmpSize = m_decoder->m_videoPool[idSlot].m_imagerealSize;
		m_decoder->m_videoPool[idSlot].m_imagerealSize = m_videoSize;
		m_videoSize = tmpSize;
		m_decoder->m_videoPool[idSlot].m_isUsed = false;
		m_resource->flush();
		m_nbFramePushed++;
	}
	// Display FPS ...
	m_LastResetCounter += _event.getDeltaCallDuration();
	if (m_LastResetCounter > echrono::seconds(1)) {
		m_LastResetCounter.reset();
		signalFps.emit(m_nbFramePushed);
		m_nbFramePushed = 0;
	}
	signalPosition.emit(m_currentTime);
	// TODO : Chek if this is needed, the display configuration not change too much ...
	markToRedraw();
}

void appl::widget::VideoDisplay::seek(const echrono::Duration& _time) {
	APPL_PRINT("seek request = " << _time);
	if (m_decoder == nullptr) {
		return;
	}
	m_decoder->seek(_time);
}
