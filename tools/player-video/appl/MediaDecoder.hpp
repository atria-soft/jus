/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <gale/Thread.hpp>
#include <audio/channel.hpp>
#include <audio/format.hpp>
#include <appl/debug.hpp>
#include <appl/ClientProperty.hpp>
#include <zeus/ProxyFile.hpp>

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libavformat/avio.h>
	#include <libswscale/swscale.h>
}

namespace appl {
	class MessageElement {
		public:
			uint64_t m_id; //!< Id of the current image (must be unique)
			echrono::Duration m_time; //!< Current time of the Message Element
			echrono::Duration m_duration; //!< if the FPS is static ==> the duration can be set otherwise (0)
			bool m_isUsed; //!< This buffer is used
			MessageElement():
			  m_id(0),
			  m_isUsed(false) {
				
			}
			virtual ~MessageElement() = default;
	};
	// class that contain all the element needed for a buffer image transfert:
	class MessageElementVideo : public appl::MessageElement {
		public:
			egami::Image m_image; //!< Image to manage internal data
			ivec2 m_imagerealSize; //!< Real size of the image, in OpenGL we need power of 2 border size.
			int32_t m_lineSize; //!< Size of a single line (in byte)
			void setSize(const ivec2& _newSize);
			MessageElementVideo():
			  m_image(ivec2(32,32), egami::colorType::RGB8) {
				
			}
	};
	class MessageElementAudio : public appl::MessageElement {
		public:
			std::vector<uint8_t> m_buffer; //!< raw audio data
			audio::format m_format; //!< Audio format buffer
			uint32_t m_sampleRate; //!< sample rate of the buffer
			std::vector<audio::channel> m_map; //!< Channel map of the buffer
			void configure(audio::format _format, uint32_t _sampleRate, int32_t _nbChannel, int32_t _nbSample);
	};
	class StreamBuffering : public ememory::EnableSharedFromThis<StreamBuffering> {
		public:
			StreamBuffering();
			std::mutex                                m_mutex; //!< local Lock Data protection
			ememory::SharedPtr<appl::ClientProperty>  m_property; //!< Remote interface that must get data
			uint32_t                                  m_mediaId; //!< remote media ID that need to get data
			zeus::ProxyFile                           m_fileHandle; //!< Reference on the remote file
			std::vector<uint8_t>                      m_buffer; //!< preallocated with all needed data
			int32_t                                   m_bufferReadPosition; //!< Current position that is read
			std::vector<std::pair<uint32_t,uint32_t>> m_bufferFillSection; //!< List of <start-stop> position that contain data
			bool m_callInProgress;
			bool m_stopRequested;
		public:
			bool addDataCallback(zeus::Future<zeus::Raw> _fut, int64_t _positionRequest);
			void checkIfWeNeedMoreDataFromNetwork();
			uint64_t getSize() {
				std::unique_lock<std::mutex> lock(m_mutex);
				return m_buffer.size();
			}
			std::vector<std::pair<uint32_t,uint32_t>> getDownloadPart() {
				std::unique_lock<std::mutex> lock(m_mutex);
				return m_bufferFillSection;
			}
			int32_t sizeReadable();
			void stopStream();
			void startStream();
	};
	class MediaDecoder : public gale::Thread {
		bool m_stopRequested;
		public:
			bool m_fistFrameSended;
			echrono::Duration m_seekApply;
		private:
			echrono::Duration m_seek;
			void applySeek(echrono::Duration _time);
			echrono::Duration m_duration;
		public:
			echrono::Duration getDuration() {
				return m_duration;
			}
		public:
			std::vector<MessageElementAudio> m_audioPool;
			echrono::Duration m_currentAudioTime;
			std::vector<MessageElementVideo> m_videoPool;
			echrono::Duration m_currentVideoTime;
			bool m_updateVideoTimeStampAfterSeek;
			bool getSeekDone() {
				return m_updateVideoTimeStampAfterSeek;
			}
			int32_t audioGetOlderSlot();
			int32_t videoGetOlderSlot();
		private:
			int32_t videoGetEmptySlot();
			int32_t audioGetEmptySlot();
		private:
			AVIOContext* m_IOContext;
			AVFormatContext* m_formatContext;
			AVCodecContext* m_videoDecoderContext;
			AVCodecContext* m_audioDecoderContext;
			ivec2 m_size;
			enum AVPixelFormat m_pixelFormat;
			AVStream *m_videoStream;
			AVStream *m_audioStream;
			std::string m_sourceFilename;
			
			int32_t m_videoStream_idx;
			int32_t m_audioStream_idx;
			AVFrame *m_frame;
			AVPacket m_packet;
			int32_t m_videoFrameCount;
			int32_t m_audioFrameCount;
			
			// output format convertion:
			SwsContext* m_convertContext;
			bool m_isInit;
		public:
			MediaDecoder();
			~MediaDecoder();
			
			int decode_packet(int *_gotFrame, int _cached);
			int open_codec_context(int *_streamId, AVFormatContext *_formatContext, enum AVMediaType _type);
			double getFps(AVCodecContext *_avctx);
		protected:
			void init();
		public:
			void init(ememory::SharedPtr<appl::ClientProperty> _property, uint32_t _mediaId);
			bool onThreadCall() override;
			void uninit();
			
			bool m_audioPresent;
			audio::format m_audioFormat; //!< Audio format buffer
			uint32_t m_audioSampleRate; //!< sample rate of the buffer
			std::vector<audio::channel> m_audioMap; //!< Channel map of the buffer
			bool haveAudio() {
				return m_audioPresent;
			}
			uint32_t audioGetSampleRate() {
				return m_audioSampleRate;
			}
			std::vector<audio::channel> audioGetChannelMap() {
				return m_audioMap;
			}
			audio::format audioGetFormat() {
				return m_audioFormat;
			}
			void seek(const echrono::Duration& _time) {
				m_seek = _time;
			}
			void flushMessage();
			
			void stop() override;
			/* ***********************************************
			   ** Section temporary buffer
			   ***********************************************/
			ememory::SharedPtr<appl::StreamBuffering> m_remote;
		public:
			// @brief INTERNAL read callback
			int readFunc(uint8_t* _buf, int _bufSize);
			// @brief INTERNAL write callback
			int writeFunc(uint8_t* _buf, int _bufSize);
			// @brief INTERNAL seek callback
			int64_t seekFunc(int64_t _offset, int _whence);
			
			std::vector<std::pair<float,float>> getDownloadPart() {
				std::vector<std::pair<float,float>> out;
				if (m_remote == nullptr) {
					return out;
				}
				std::vector<std::pair<uint32_t,uint32_t>> vals = m_remote->getDownloadPart();
				echrono::Duration totalTime = getDuration();
				float size = totalTime.toSeconds()/float(m_remote->getSize());
				//APPL_ERROR(" duration in sec : " << totalTime << " => " << totalTime.toSeconds());
				for (auto &it : vals) {
					out.push_back(std::pair<float,float>(float(it.first)*size, float(it.second)*size));
				}
				return out;
			}
			
		
	};
}


