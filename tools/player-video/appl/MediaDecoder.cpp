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
#include <egami/egami.hpp>

static void init_ffmpeg() {
	static bool isInit = false;
	if (isInit == false) {
		isInit = true;
		av_register_all();
	}
}


static void unPlanar(void* _bufferOut, const void* _bufferIn, int32_t _nbSample, audio::format _format, int32_t _channelId, int32_t _nbChannel) {
	switch(_format) {
		case audio::format_int8: {
			const uint8_t* in = reinterpret_cast<const uint8_t*>(_bufferIn);
			uint8_t* out = reinterpret_cast<uint8_t*>(_bufferOut);
			for (int32_t sss=0; sss<_nbSample; ++sss) {
				out[sss*_nbChannel + _channelId] = in[sss];
			}
			return;
		}
		case audio::format_int16: {
			const int16_t* in = reinterpret_cast<const int16_t*>(_bufferIn);
			int16_t* out = reinterpret_cast<int16_t*>(_bufferOut);
			for (int32_t sss=0; sss<_nbSample; ++sss) {
				out[sss*_nbChannel + _channelId] = in[sss];
			}
			return;
		}
		case audio::format_int32: {
			const int32_t* in = reinterpret_cast<const int32_t*>(_bufferIn);
			int32_t* out = reinterpret_cast<int32_t*>(_bufferOut);
			for (int32_t sss=0; sss<_nbSample; ++sss) {
				out[sss*_nbChannel + _channelId] = in[sss];
			}
			return;
		}
		case audio::format_float: {
			const float* in = reinterpret_cast<const float*>(_bufferIn);
			float* out = reinterpret_cast<float*>(_bufferOut);
			for (int32_t sss=0; sss<_nbSample; ++sss) {
				out[sss*_nbChannel + _channelId] = in[sss];
			}
			return;
		}
		case audio::format_double: {
			const double* in = reinterpret_cast<const double*>(_bufferIn);
			double* out = reinterpret_cast<double*>(_bufferOut);
			for (int32_t sss=0; sss<_nbSample; ++sss) {
				out[sss*_nbChannel + _channelId] = in[sss];
			}
			return;
		}
		default:
			break;
	}
}

/**
 * @brief get the next power 2 if the input
 * @param[in] value Value that we want the next power of 2
 * @return result value
 */
static int32_t nextP2(int32_t _value) {
	int32_t val=1;
	for (int32_t iii=1; iii<31; iii++) {
		if (_value <= val) {
			return val;
		}
		val *=2;
	}
	EWOL_CRITICAL("impossible CASE....");
	return val;
}

void appl::MessageElementVideo::setSize(const ivec2& _size) {
	if (m_imagerealSize != _size) {
		// Resize the buffer:
		m_imagerealSize = _size;
		m_image.resize(ivec2(nextP2(_size.x()), nextP2(_size.y())));
		m_lineSize = m_image.getSize().x() * 3; // 3 is for RGBA
		//m_image.getSize();
	}
}
void appl::MessageElementAudio::configure(audio::format _format, uint32_t _sampleRate, int32_t _nbChannel, int32_t _nbSample) {
	// resize the buffer:
	m_buffer.resize(_nbSample*_nbChannel*audio::getFormatBytes(_format));
	m_format = _format;
	m_sampleRate = _sampleRate;
	m_map.resize(_nbChannel);
	switch(_nbChannel) {
		case 1:
			m_map[0] = audio::channel_frontCenter;
			break;
		case 2:
			m_map[0] = audio::channel_frontLeft;
			m_map[1] = audio::channel_frontRight;
			break;
		case 3:
			m_map[0] = audio::channel_frontLeft;
			m_map[1] = audio::channel_lfe;
			m_map[2] = audio::channel_frontRight;
			break;
		case 4:
			m_map[0] = audio::channel_frontLeft;
			m_map[1] = audio::channel_frontRight;
			m_map[2] = audio::channel_rearLeft;
			m_map[3] = audio::channel_rearRight;
			break;
		default:
			APPL_ERROR("not supportef nbChannel" << _nbChannel);
	}
}

appl::MediaDecoder::MediaDecoder() {
	init_ffmpeg();
	m_formatContext = nullptr;
	m_videoDecoderContext = nullptr;
	m_audioDecoderContext = nullptr;
	m_size = ivec2(0,0);
	m_videoStream = nullptr;
	m_audioStream = nullptr;
	
	m_videoStream_idx = -1;
	m_audioStream_idx = -1;
	m_frame = nullptr;
	m_videoFrameCount = 0;
	m_audioFrameCount = 0;
	m_seek = -1;
	// output format convertion:
	m_convertContext = nullptr;
	m_audioPresent = false;
	m_audioFormat = audio::format_unknow;
	m_isInit = false;
	m_stopRequested = false;
}

appl::MediaDecoder::~MediaDecoder() {
	uninit();
}

int appl::MediaDecoder::decode_packet(int *_gotFrame, int _cached) {
	int ret = 0;
	int decoded = m_packet.size;
	*_gotFrame = 0;
	if (m_packet.stream_index == m_videoStream_idx) {
		// decode video frame
		ret = avcodec_decode_video2(m_videoDecoderContext, m_frame, _gotFrame, &m_packet);
		if (ret < 0) {
			APPL_ERROR("Error decoding video frame (" << av_err2str(ret) << ")");
			return ret;
		}
		if (*_gotFrame) {
			if (    m_frame->width != m_size.x()
			     || m_frame->height != m_size.y() ||
				m_frame->format != m_pixelFormat) {
				// To handle this change, one could call av_image_alloc again and decode the following frames into another rawvideo file.
				APPL_ERROR("Width, height and pixel format have to be constant in a rawvideo file, but the width, height or pixel format of the input video changed:");
				APPL_ERROR("old: size=" << m_size << " format=" << av_get_pix_fmt_name(m_pixelFormat));
				APPL_ERROR("new: size=" << ivec2(m_frame->width,m_frame->height) << " format=" << av_get_pix_fmt_name((enum AVPixelFormat)m_frame->format));
				return -1;
			}
			if (m_updateVideoTimeStampAfterSeek == true) {
				m_currentVideoTime = m_currentAudioTime;
				m_updateVideoTimeStampAfterSeek = false;
				m_seekApply = m_currentVideoTime; // => ready to display
			}
			echrono::Duration packetTime(double(m_frame->pkt_pts) * double(m_videoDecoderContext->time_base.num) / double(m_videoDecoderContext->time_base.den));
			APPL_INFO("video_frame " << (_cached?"(cached)":"")
			             << " n=" << m_videoFrameCount
			             << " coded_n=" << m_frame->coded_picture_number
			             << " pts=" << av_ts2timestr(m_frame->pkt_pts, &m_videoDecoderContext->time_base) << "  " << packetTime);
			m_videoFrameCount++;
			int32_t slotId = videoGetEmptySlot();
			if (slotId == -1) {
				APPL_ERROR("an error occured when getting an empty slot for video");
			} else {
				m_videoPool[slotId].setSize(ivec2(m_frame->width, m_frame->height));
				uint8_t* dataPointer = (uint8_t*)(m_videoPool[slotId].m_image.getTextureDataPointer());
				// Convert Image in RGB:
				sws_scale(m_convertContext,
				          (const uint8_t **)(m_frame->data),
				          m_frame->linesize,
				          0, m_frame->height,
				          &dataPointer,
				          &m_videoPool[slotId].m_lineSize);
				m_videoPool[slotId].m_id = m_videoFrameCount;
				m_videoPool[slotId].m_time = m_currentVideoTime;
				m_videoPool[slotId].m_duration = echrono::Duration(0, 1000000000.0/float(getFps(m_videoDecoderContext)));
				m_currentVideoTime += m_videoPool[slotId].m_duration;
				m_videoPool[slotId].m_isUsed = true;
			}
		}
	} else if (m_packet.stream_index == m_audioStream_idx) {
		// decode audio frame
		ret = avcodec_decode_audio4(m_audioDecoderContext, m_frame, _gotFrame, &m_packet);
		if (ret < 0) {
			APPL_ERROR("Error decoding audio frame (" << av_err2str(ret) << ")");
			return ret;
		}
		// Some audio decoders decode only part of the packet, and have to be called again with the remainder of the packet data.
		decoded = FFMIN(ret, m_packet.size);
		if (*_gotFrame) {
			echrono::Duration packetTime(double(m_frame->pkt_pts) * double(m_audioDecoderContext->time_base.num) / double(m_audioDecoderContext->time_base.den));
			if (m_updateVideoTimeStampAfterSeek == true) {
				// seek specific usecase ==> drop frame to have fast display
				m_currentAudioTime = packetTime;
			} else {
				APPL_INFO("audio_frame " << (_cached?"(cached)":"")
				             << " n=" << m_audioFrameCount
				             << " nb_samples=" << m_frame->nb_samples
				             << " pts=" << packetTime);
				m_audioFrameCount++;
				int32_t slotId = audioGetEmptySlot();
				if (slotId == -1) {
					APPL_ERROR("an error occured when getting an empty slot for audio");
				} else {
					//m_frame->channel_layout
					audio::format format = audio::format_unknow;
					switch(av_get_packed_sample_fmt((enum AVSampleFormat)m_frame->format)) {
						case AV_SAMPLE_FMT_U8:   format = audio::format_int8;   break;
						case AV_SAMPLE_FMT_S16:  format = audio::format_int16;  break;
						case AV_SAMPLE_FMT_S32:  format = audio::format_int32;  break;
						case AV_SAMPLE_FMT_FLT:  format = audio::format_float;  break;
						case AV_SAMPLE_FMT_DBL:  format = audio::format_double; break;
						default: break;
					}
					if (format == audio::format_unknow) {
						APPL_ERROR("Unsupported audio format :" << m_frame->format << " ...");
					} else {
						// configure Message:
						m_audioPool[slotId].configure(format, m_frame->sample_rate, m_frame->channels, m_frame->nb_samples);
						if (av_sample_fmt_is_planar((enum AVSampleFormat)m_frame->format) == 1) {
							for (int32_t ccc=0; ccc<m_frame->channels; ++ccc) {
								unPlanar(&m_audioPool[slotId].m_buffer[0],
								         m_frame->extended_data[ccc],
								         m_frame->nb_samples,
								         m_audioPool[slotId].m_format,
								         ccc,
								         m_frame->channels);
							}
						} else {
							// inject data in the buffer:
							memcpy(&m_audioPool[slotId].m_buffer[0], m_frame->extended_data[0], m_audioPool[slotId].m_buffer.size());
						}
						// We use the Time of the packet ==> better synchronisation when seeking
						m_currentAudioTime = packetTime;
						m_audioPool[slotId].m_id = m_audioFrameCount;
						m_audioPool[slotId].m_time = m_currentAudioTime;
						m_audioPool[slotId].m_duration = echrono::Duration(0,(1000000000.0*m_frame->nb_samples)/float(m_frame->sample_rate));
						m_currentAudioTime += m_audioPool[slotId].m_duration;
						m_audioPool[slotId].m_isUsed = true;
					}
				}
			}
		}
	}
	return decoded;
}

int appl::MediaDecoder::open_codec_context(int *_streamId, AVFormatContext *_formatContext, enum AVMediaType _type) {
	int ret = 0;
	int stream_index = 0;
	AVStream *st = nullptr;
	AVCodecContext *dec_ctx = nullptr;
	AVCodec *dec = nullptr;
	AVDictionary *opts = nullptr;
	ret = av_find_best_stream(_formatContext, _type, -1, -1, nullptr, 0);
	if (ret < 0) {
		APPL_ERROR("Could not find " << av_get_media_type_string(_type) << " stream in input file '" << m_sourceFilename << "'");
		return ret;
	} else {
		stream_index = ret;
		st = _formatContext->streams[stream_index];
		// find decoder for the stream
		dec_ctx = st->codec;
		dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			APPL_ERROR("Failed to find " << av_get_media_type_string(_type) << " codec");
			return AVERROR(EINVAL);
		}
		// Init the decoders, with or without reference counting
		av_dict_set(&opts, "refcounted_frames", "0", 0);
		av_dict_set(&opts, "threads", "auto", 0);
		if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
			APPL_ERROR("Failed to open " << av_get_media_type_string(_type) << " codec");
			return ret;
		}
		*_streamId = stream_index;
	}
	return 0;
}

double appl::MediaDecoder::getFps(AVCodecContext *_avctx) {
	return 1.0 / av_q2d(_avctx->time_base) / FFMAX(_avctx->ticks_per_frame, 1);
}

void appl::MediaDecoder::init(const std::string& _filename) {
	m_updateVideoTimeStampAfterSeek = false;
	m_sourceFilename = _filename;
	// open input file, and allocate format context
	if (avformat_open_input(&m_formatContext, m_sourceFilename.c_str(), nullptr, nullptr) < 0) {
		APPL_ERROR("Could not open source file " << m_sourceFilename);
		exit(1);
	}
	// retrieve stream information
	if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
		APPL_ERROR("Could not find stream information");
		// TODO : check this, this will create a memeory leak
		return;;
	}
	m_duration = echrono::Duration(double(m_formatContext->duration)/double(AV_TIME_BASE));
	APPL_INFO("Stream duration : " << m_duration);
	// Open Video decoder:
	if (open_codec_context(&m_videoStream_idx, m_formatContext, AVMEDIA_TYPE_VIDEO) >= 0) {
		m_videoStream = m_formatContext->streams[m_videoStream_idx];
		m_videoDecoderContext = m_videoStream->codec;
		// allocate image where the decoded image will be put
		m_size.setValue(m_videoDecoderContext->width, m_videoDecoderContext->height);
		m_pixelFormat = m_videoDecoderContext->pix_fmt;
		
		m_videoPool.resize(10);
		APPL_INFO("Open video stream with property: size=" << m_size << " pixel format=" << av_get_pix_fmt_name(m_pixelFormat) << " fps=" << getFps(m_videoDecoderContext) << " tick/frame=" << m_videoDecoderContext->ticks_per_frame);
		// convert the image format:
		m_convertContext = sws_getContext(m_size.x(), m_size.y(), m_pixelFormat,
		                                  m_size.x(), m_size.y(), AV_PIX_FMT_RGB24,
		                                  0, 0, 0, 0);
	}
	// Open Audio Decoder:
	if (open_codec_context(&m_audioStream_idx, m_formatContext, AVMEDIA_TYPE_AUDIO) >= 0) {
		m_audioPresent = true;
		m_audioStream = m_formatContext->streams[m_audioStream_idx];
		m_audioDecoderContext = m_audioStream->codec;
		
		m_audioPool.resize(80);
		
		// Number of channels: m_audioDecoderContext->channels
		// Framerate:          m_audioDecoderContext->sample_rate
		APPL_INFO("Open audio stream with audio property: " << int32_t(m_audioDecoderContext->channels) << " channel(s) & samplerate=" << m_audioDecoderContext->sample_rate << " Hz");
		
		//m_frame->channel_layout
		m_audioSampleRate = m_audioDecoderContext->sample_rate;
		m_audioFormat = audio::format_unknow;
		switch(av_get_packed_sample_fmt(m_audioDecoderContext->sample_fmt)) {
			case AV_SAMPLE_FMT_U8:   m_audioFormat = audio::format_int8;   break;
			case AV_SAMPLE_FMT_S16:  m_audioFormat = audio::format_int16;  break;
			case AV_SAMPLE_FMT_S32:  m_audioFormat = audio::format_int32;  break;
			case AV_SAMPLE_FMT_FLT:  m_audioFormat = audio::format_float;  break;
			case AV_SAMPLE_FMT_DBL:  m_audioFormat = audio::format_double; break;
			default:
				m_audioFormat = audio::format_unknow;
				break;
		}
		// TODO : Do it better:
		m_audioMap.resize(m_audioDecoderContext->channels);
		switch(m_audioDecoderContext->channels) {
			case 1:
				m_audioMap[0] = audio::channel_frontCenter;
				break;
			case 2:
				m_audioMap[0] = audio::channel_frontLeft;
				m_audioMap[1] = audio::channel_frontRight;
				break;
			case 3:
				m_audioMap[0] = audio::channel_frontLeft;
				m_audioMap[1] = audio::channel_lfe;
				m_audioMap[2] = audio::channel_frontRight;
				break;
			case 4:
				m_audioMap[0] = audio::channel_frontLeft;
				m_audioMap[1] = audio::channel_frontRight;
				m_audioMap[2] = audio::channel_rearLeft;
				m_audioMap[3] = audio::channel_rearRight;
				break;
			default:
				APPL_ERROR("not supportef nbChannel " << m_audioDecoderContext->channels);
		}
		APPL_PRINT("Audio configuration : " << m_audioMap << " " << m_audioFormat << " sampleRate=" <<m_audioSampleRate);
	}
	// dump input information to stderr
	// For test only: av_dump_format(m_formatContext, 0, m_sourceFilename.c_str(), 0);
	
	if (    m_audioStream == nullptr
	     && m_videoStream == nullptr) {
		APPL_ERROR("Could not find audio or video stream in the input, aborting");
		return; // TODO : An error occured ... !!!!!
	}
	m_frame = av_frame_alloc();
	if (!m_frame) {
		int ret = AVERROR(ENOMEM);
		APPL_ERROR("Could not allocate frame ret=" << ret);
		return; // TODO : An error occured ... !!!!!
	}
	// initialize packet, set data to nullptr, let the demuxer fill it
	av_init_packet(&m_packet);
	m_packet.data = nullptr;
	m_packet.size = 0;
	m_isInit = true;
}

bool appl::MediaDecoder::onThreadCall() {
	if (m_stopRequested == true) {
		return true;
	}
	if (m_seek >= echrono::Duration(0)) {
		// seek requested (create a copy to permit to update it in background):
		echrono::Duration tmpSeek = m_seek;
		m_seek = echrono::Duration(-1);
		applySeek(tmpSeek);
	}
	// check if we have space to decode data
	if (    (    m_videoPool.size() != 0
	          && videoGetEmptySlot() == -1)
	     || (    m_audioPool.size() != 0
	          && audioGetEmptySlot() == -1)
	   ) {
		// take some time to sleep the decoding ...
		std::this_thread::sleep_for(std::chrono::milliseconds(60/25));
		return false;
	}
	APPL_VERBOSE("Work on decoding");
	int gotFrame;
	// read frames from the file
	int ret = av_read_frame(m_formatContext, &m_packet);
	if (ret >= 0) {
		AVPacket orig_pkt = m_packet;
		do {
			ret = decode_packet(&gotFrame, 0);
			if (ret < 0) {
				break;
			}
			m_packet.data += ret;
			m_packet.size -= ret;
		} while (m_packet.size > 0);
		av_packet_unref(&orig_pkt);
	}
	return (ret < 0);
}
void appl::MediaDecoder::uninit() {
	if (m_isInit == false) {
		return;
	}
	APPL_PRINT("Demuxing & Decoding succeeded...");
	avcodec_close(m_videoDecoderContext);
	avcodec_close(m_audioDecoderContext);
	avformat_close_input(&m_formatContext);
	av_frame_free(&m_frame);
	m_isInit = false;
}

void appl::MediaDecoder::stop() {
	m_stopRequested = true;
}

void appl::MediaDecoder::flushMessage() {
	// flush all decoders ...
	avcodec_flush_buffers(m_audioStream->codec);
	avcodec_flush_buffers(m_videoStream->codec);
	// TODO : Protect this ...
	// Disable use of all buffer
	for (int32_t iii=0; iii<m_videoPool.size(); ++iii) {
		m_videoPool[iii].m_isUsed = false;
	}
	for (int32_t iii=0; iii<m_audioPool.size(); ++iii) {
		m_audioPool[iii].m_isUsed = false;
	}
}

void appl::MediaDecoder::applySeek(echrono::Duration _time) {
	APPL_INFO("Apply seek : " << _time);
	flushMessage();
	int64_t seekPos = int64_t(_time.toSeconds() * double(AV_TIME_BASE));
	
	int32_t id = -1;
	echrono::Duration tmpPos;
	if (m_audioStream_idx>=0) {
		id = m_audioStream_idx;
		tmpPos = m_currentAudioTime;
	} else if (m_videoStream_idx>=0) {
		id = m_videoStream_idx;
		tmpPos = m_currentVideoTime;
	}
	
	int64_t seekTarget = av_rescale_q(seekPos, AV_TIME_BASE_Q, m_formatContext->streams[id]->time_base);
	APPL_INFO("request seek at: " << seekPos << " and get position: " << seekTarget);
	int flags = _time < tmpPos ? AVSEEK_FLAG_BACKWARD : 0; // AVSEEK_FLAG_ANY
	if (av_seek_frame(m_formatContext, id, seekTarget, flags) < 0) {
		APPL_ERROR(" Unable to seek");
		return;
	}
	m_currentVideoTime = _time;
	m_currentAudioTime = _time;
	m_updateVideoTimeStampAfterSeek = true;
	APPL_INFO("Request seeking : " << _time << " done");
}

int32_t appl::MediaDecoder::videoGetEmptySlot() {
	for (int32_t iii=0; iii<m_videoPool.size(); ++iii) {
		if (m_videoPool[iii].m_isUsed == false) {
			return iii;
		}
	}
	return -1;
}

int32_t appl::MediaDecoder::audioGetEmptySlot() {
	for (int32_t iii=0; iii<m_audioPool.size(); ++iii) {
		if (m_audioPool[iii].m_isUsed == false) {
			return iii;
		}
	}
	return -1;
}

int32_t appl::MediaDecoder::videoGetOlderSlot() {
	int32_t smallerId = 0x7FFFFFFE;
	int32_t findId = -1;
	for (int32_t iii=0; iii<m_videoPool.size(); ++iii) {
		if (    m_videoPool[iii].m_isUsed == true
		     && smallerId > m_videoPool[iii].m_id) {
			smallerId = m_videoPool[iii].m_id;
			findId = iii;
		}
	}
	return findId;
}

int32_t appl::MediaDecoder::audioGetOlderSlot() {
	int32_t smallerId = 0x7FFFFFFF;
	int32_t findId = -1;
	for (int32_t iii=0; iii<m_audioPool.size(); ++iii) {
		if (    m_audioPool[iii].m_isUsed == true
		     && smallerId > m_audioPool[iii].m_id) {
			smallerId = m_audioPool[iii].m_id;
			findId = iii;
		}
	}
	return findId;
}
