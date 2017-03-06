/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <ewol/widget/Widget.hpp>
#include <ewol/compositing/Image.hpp>
#include <ewol/widget/Manager.hpp>
#include <gale/Thread.hpp>
#include <esignal/Signal.hpp>
#include <audio/channel.hpp>
#include <audio/format.hpp>
#include <audio/river/river.hpp>
#include <audio/river/Manager.hpp>
#include <audio/river/Interface.hpp>
#include <appl/MediaDecoder.hpp>
#include <appl/widget/ListViewer.hpp>

namespace appl {
	namespace widget {
		class VideoDisplay : public ewol::Widget {
			public:
				esignal::Signal<int32_t> signalFps;
				esignal::Signal<echrono::Duration> signalPosition; //!< signal the current duration of the video duration
				esignal::Signal<echrono::Duration> signalDuration; //!< signal the current duration of the video duration
				bool m_haveDuration;
			private:
				mat4 m_matrixApply;
				ememory::SharedPtr<appl::MediaDecoder> m_decoder;
				ivec2 m_videoSize;
				ivec2 m_imageSize;
				echrono::Duration m_LastResetCounter;
				int32_t m_nbFramePushed;
				echrono::Duration m_currentTime;
			private:
				ememory::SharedPtr<gale::resource::Program> m_GLprogram; //!< pointer on the opengl display program
				int32_t m_GLPosition; //!< openGL id on the element (vertex buffer)
				int32_t m_GLMatrix; //!< openGL id on the element (transformation matrix)
				int32_t m_GLColor; //!< openGL id on the element (color buffer)
				int32_t m_GLtexture; //!< openGL id on the element (Texture position)
				int32_t m_GLtexID; //!< openGL id on the element (texture ID)
			protected:
				ememory::SharedPtr<ewol::resource::Texture> m_resource; //!< texture resources
			protected:
				static const int32_t m_vboIdCoord;
				static const int32_t m_vboIdCoordTex;
				static const int32_t m_vboIdColor;
				ememory::SharedPtr<gale::resource::VirtualBufferObject> m_VBO;
			protected:
				etk::Color<float,4> m_color;
				vec3 m_position;
			protected:
				//! @brief constructor
				VideoDisplay();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(VideoDisplay, "VideoDisplay");
				//! @brief destructor
				virtual ~VideoDisplay();
			public:
				void onDraw() override;
				void onRegenerateDisplay() override;
			public:
				void setFile(const std::string& _fileName);
				void setZeusMedia(ememory::SharedPtr<ClientProperty> _property, uint32_t _mediaId);
			protected:
				bool m_isPalying;
			public:
				bool isPlaying();
				void play();
				void pause();
				void stop();
			public:
				void periodicEvent(const ewol::event::Time& _event);
			private:
				void printPart(const vec2& _size, const vec2& _sourcePosStart, const vec2& _sourcePosStop);
				void loadProgram();
			private: // Audio Property:
				ememory::SharedPtr<audio::river::Manager> m_audioManager; //!< River manager interface
				ememory::SharedPtr<audio::river::Interface> m_audioInterface; //!< Play audio interface
			public:
				echrono::Duration getDuration() {
					if (m_decoder != nullptr) {
						return m_decoder->getDuration();
					}
					return echrono::Duration(0);
				}
				void seek(const echrono::Duration& _time);
		};
	}
}




