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

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

namespace appl {
	class Decoder : public gale::Thread {
		private:
			AVFormatContext* m_formatContext;
			AVCodecContext* m_videoDecoderContext;
			AVCodecContext* m_audioDecoderContext;
			ivec2 m_size;
			enum AVPixelFormat m_pixelFormat;
			AVStream *m_videoStream;
			AVStream *m_audioStream;
			std::string m_sourceFilename;
			
			uint8_t *m_videoDestinationData[4];
			int32_t m_videoDestinationLineSize[4];
			int32_t m_videoDestinationBufferSize;
			
			uint8_t *m_videoDestinationRGBData[4];
			int32_t m_videoDestinationRGBLineSize[4];
			int32_t m_videoDestinationRGBBufferSize;
			
			int32_t m_videoStream_idx;
			int32_t m_audioStream_idx;
			AVFrame *m_frame;
			AVPacket m_packet;
			int32_t m_videoFrameCount;
			int32_t m_audioFrameCount;
			
			// output format convertion:
			SwsContext* m_convertContext;
			
			// Enable or disable frame reference counting.
			// You are not supposed to support both paths in your application but pick the one most appropriate to your needs.
			// Look for the use of refcount in this example to see what are the differences of API usage between them.
			bool m_refCount;
		public:
			Decoder();
			
			int decode_packet(int *_gotFrame, int _cached);
			int open_codec_context(int *_streamId, AVFormatContext *_formatContext, enum AVMediaType _type);
			double getFps(AVCodecContext *_avctx);
			void init(const std::string& _filename);
			bool onThreadCall() override;
			void uninit();
	};
}
namespace appl {
	namespace widget {
		class VideoDisplay : public ewol::Widget {
			public:
				esignal::Signal<int32_t> signalFps;
			private:
				mat4 m_matrixApply;
				appl::Decoder m_decoder;
				ivec2 m_videoSize;
				ivec2 m_imageSize;
				echrono::Duration m_LastResetCounter;
				int32_t m_nbFramePushed;
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
			public:
				void periodicEvent(const ewol::event::Time& _event);
			private:
				void printPart(const vec2& _size, const vec2& _sourcePosStart, const vec2& _sourcePosStop);
				void loadProgram();
			public:
				void setRawData(const ivec2& _size, void* _dataPointer);
		};
	}
}




