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

static void unPlanar(void* _buffer, int32_t _len, audio::format _format, int32_t _nbChannel) {
  if (_nbChannel == 1) {
    // nothing to do only one channel ...
    return;
  }
  std::vector<uint8_t> tmpData;
  tmpData.resize(_len);
  memcpy(&tmpData[0], _buffer, _len);
  // depend on the type of flow:
  switch(_format) {
    case audio::format_int8:
    {
      uint8_t* in = reinterpret_cast<uint8_t*>(&tmpData[0]);
      uint8_t* out = reinterpret_cast<uint8_t*>(_buffer);
      int32_t nbSample = _len/static_cast<int32_t>(sizeof(int8_t)*_nbChannel);
      for (int32_t iii=0; iii<nbSample; ++iii) {
        for (int32_t jjj=0; jjj<_nbChannel; ++jjj) {
          out[iii*_nbChannel + jjj] = in[jjj*nbSample+iii];
        }
      }
      return;
    }
    case audio::format_int16:
    {
      int16_t* in = reinterpret_cast<int16_t*>(&tmpData[0]);
      int16_t* out = reinterpret_cast<int16_t*>(_buffer);
      int32_t nbSample = _len/static_cast<int32_t>(sizeof(int16_t)*_nbChannel);
      for (int32_t iii=0; iii<nbSample; ++iii) {
        for (int32_t jjj=0; jjj<_nbChannel; ++jjj) {
          out[iii*_nbChannel + jjj] = in[jjj*nbSample+iii];
        }
      }
      return;
    }
    case audio::format_int32:
    {
      int32_t* in = reinterpret_cast<int32_t*>(&tmpData[0]);
      int32_t* out = reinterpret_cast<int32_t*>(_buffer);
      int32_t nbSample = _len/static_cast<int32_t>(sizeof(int32_t)*_nbChannel);
      for (int32_t iii=0; iii<nbSample; ++iii) {
        for (int32_t jjj=0; jjj<_nbChannel; ++jjj) {
          out[iii*_nbChannel + jjj] = in[jjj*nbSample+iii];
        }
      }
      return;
    }
    case audio::format_float:
    {
      float* in = reinterpret_cast<float*>(&tmpData[0]);
      float* out = reinterpret_cast<float*>(_buffer);
      int32_t nbSample = _len/static_cast<int32_t>(sizeof(float)*_nbChannel);
      for (int32_t iii=0; iii<nbSample; ++iii) {
        for (int32_t jjj=0; jjj<_nbChannel; ++jjj) {
          out[iii*_nbChannel + jjj] = in[jjj*nbSample+iii];
        }
      }
      return;
    }
    case audio::format_double:
    {
      double* in = reinterpret_cast<double*>(&tmpData[0]);
      double* out = reinterpret_cast<double*>(_buffer);
      int32_t nbSample = _len/static_cast<int32_t>(sizeof(double)*_nbChannel);
      for (int32_t iii=0; iii<nbSample; ++iii) {
        for (int32_t jjj=0; jjj<_nbChannel; ++jjj) {
          out[iii*_nbChannel + jjj] = in[jjj*nbSample+iii];
        }
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

void appl::BufferElementVideo::setSize(const ivec2& _size) {
	if (m_imagerealSize != _size) {
		// Resize the buffer:
		m_imagerealSize = _size;
		m_image.resize(ivec2(nextP2(_size.x()), nextP2(_size.y())));
		m_lineSize = m_image.getSize().x() * 3; // 3 is for RGBA
		//m_image.getSize();
	}
}
void appl::BufferElementAudio::configure(audio::format _format, uint32_t _sampleRate, int32_t _nbChannel, int32_t _nbSample) {
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

appl::Decoder::Decoder() {
	m_formatContext = nullptr;
	m_videoDecoderContext = nullptr;
	m_audioDecoderContext = nullptr;
	m_size = ivec2(0,0);
	m_videoStream = nullptr;
	m_audioStream = nullptr;
	
	m_videoDestinationRGBData[0] = nullptr;
	
	m_videoStream_idx = -1;
	m_audioStream_idx = -1;
	m_frame = nullptr;
	m_videoFrameCount = 0;
	m_audioFrameCount = 0;
	
	// output format convertion:
	m_convertContext = nullptr;
	m_audioPresent = false;
	m_audioFormat = audio::format_unknow;
	// Enable or disable frame reference counting.
	// You are not supposed to support both paths in your application but pick the one most appropriate to your needs.
	// Look for the use of refcount in this example to see what are the differences of API usage between them.
	m_refCount = false;
}

int appl::Decoder::decode_packet(int *_gotFrame, int _cached) {
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
			APPL_VERBOSE("video_frame " << (_cached?"(cached)":"")
			             << " n=" << m_videoFrameCount
			             << " coded_n=" << m_frame->coded_picture_number
			             << " pts=" << av_ts2timestr(m_frame->pts, &m_videoDecoderContext->time_base));
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
			APPL_VERBOSE("audio_frame " << (_cached?"(cached)":"")
			             << " n=" << m_audioFrameCount
			             << " nb_samples=" << m_frame->nb_samples
			             << " pts=" << av_ts2timestr(m_frame->pts, &m_audioDecoderContext->time_base));
			m_audioFrameCount++;
			int32_t slotId = audioGetEmptySlot();
			if (slotId == -1) {
				APPL_ERROR("an error occured when getting an empty slot for audio");
			} else {
				//m_frame->channel_layout
				bool isPlanar = false;
				audio::format format = audio::format_unknow;
				switch(m_frame->format) {
					case AV_SAMPLE_FMT_U8P:  isPlanar = true;
					case AV_SAMPLE_FMT_U8:   format = audio::format_int8;   break;
					case AV_SAMPLE_FMT_S16P: isPlanar = true;
					case AV_SAMPLE_FMT_S16:  format = audio::format_int16;  break;
					case AV_SAMPLE_FMT_S32P: isPlanar = true;
					case AV_SAMPLE_FMT_S32:  format = audio::format_int32;  break;
					case AV_SAMPLE_FMT_FLTP: isPlanar = true;
					case AV_SAMPLE_FMT_FLT:  format = audio::format_float;  break;
					case AV_SAMPLE_FMT_DBLP: isPlanar = true;
					case AV_SAMPLE_FMT_DBL:  format = audio::format_double; break;
				}
				if (format == audio::format_unknow) {
					APPL_ERROR("Unsupported audio format :" << m_frame->format << " ...");
				} else {
					// configure Buffer:
					m_audioPool[slotId].configure(format, m_frame->sample_rate, m_frame->channels, m_frame->nb_samples);
					// TODO : Optimise buffer transfer
					if (isPlanar == true) {
						unPlanar(m_frame->extended_data[0], m_audioPool[slotId].m_buffer.size(), m_audioPool[slotId].m_format, m_frame->channels);
					}
					// inject data in the buffer:
					memcpy(&m_audioPool[slotId].m_buffer[0], m_frame->extended_data[0], m_audioPool[slotId].m_buffer.size());
					m_audioPool[slotId].m_id = m_audioFrameCount;
					m_audioPool[slotId].m_time = m_currentAudioTime;
					m_audioPool[slotId].m_duration = echrono::Duration(0,(1000000000.0*m_frame->nb_samples)/float(m_frame->sample_rate));
					m_currentAudioTime += m_audioPool[slotId].m_duration;
					m_audioPool[slotId].m_isUsed = true;
				}
			}
		}
	}
	// If we use frame reference counting, we own the data and need to de-reference it when we don't use it anymore
	if (*_gotFrame && m_refCount)
		av_frame_unref(m_frame);
	return decoded;
}
int appl::Decoder::open_codec_context(int *_streamId, AVFormatContext *_formatContext, enum AVMediaType _type) {
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
		av_dict_set(&opts, "refcounted_frames", m_refCount ? "1" : "0", 0);
		av_dict_set(&opts, "threads", "auto", 0);
		if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
			APPL_ERROR("Failed to open " << av_get_media_type_string(_type) << " codec");
			return ret;
		}
		*_streamId = stream_index;
	}
	return 0;
}

double appl::Decoder::getFps(AVCodecContext *_avctx) {
	return 1.0 / av_q2d(_avctx->time_base) / FFMAX(_avctx->ticks_per_frame, 1);
}

void appl::Decoder::init(const std::string& _filename) {
	int ret = 0;
	// Enable or disable refcount:
	if (false) {
		m_refCount = true;
	}
	m_sourceFilename = _filename;
	// register all formats and codecs
	av_register_all();
	// open input file, and allocate format context
	if (avformat_open_input(&m_formatContext, m_sourceFilename.c_str(), nullptr, nullptr) < 0) {
		APPL_ERROR("Could not open source file " << m_sourceFilename);
		exit(1);
	}
	// retrieve stream information
	if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
		APPL_ERROR("Could not find stream information");
		exit(1);
	}
	// Open Video decoder:
	if (open_codec_context(&m_videoStream_idx, m_formatContext, AVMEDIA_TYPE_VIDEO) >= 0) {
		m_videoStream = m_formatContext->streams[m_videoStream_idx];
		m_videoDecoderContext = m_videoStream->codec;
		// allocate image where the decoded image will be put
		m_size.setValue(m_videoDecoderContext->width, m_videoDecoderContext->height);
		m_pixelFormat = m_videoDecoderContext->pix_fmt;
		
		m_videoPool.resize(10);
		
		// Create the video buffer for RGB mode:
		ret = av_image_alloc(m_videoDestinationRGBData, m_videoDestinationRGBLineSize, m_size.x(), m_size.y(), AV_PIX_FMT_RGB24, 1);
		if (ret < 0) {
			APPL_ERROR("Could not allocate raw video buffer");
			return; // TODO : An error occured ... !!!!!
		}
		m_videoDestinationRGBBufferSize = ret;
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
		bool isPlanar = false;
		m_audioSampleRate = m_audioDecoderContext->sample_rate;
		m_audioFormat = audio::format_unknow;
		switch(m_audioDecoderContext->sample_fmt) {
			case AV_SAMPLE_FMT_U8P:
			case AV_SAMPLE_FMT_U8:   m_audioFormat = audio::format_int8;   break;
			case AV_SAMPLE_FMT_S16P:
			case AV_SAMPLE_FMT_S16:  m_audioFormat = audio::format_int16;  break;
			case AV_SAMPLE_FMT_S32P:
			case AV_SAMPLE_FMT_S32:  m_audioFormat = audio::format_int32;  break;
			case AV_SAMPLE_FMT_FLTP:
			case AV_SAMPLE_FMT_FLT:  m_audioFormat = audio::format_float;  break;
			case AV_SAMPLE_FMT_DBLP:
			case AV_SAMPLE_FMT_DBL:  m_audioFormat = audio::format_double; break;
			case AV_SAMPLE_FMT_NONE:
			case AV_SAMPLE_FMT_NB:
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
	av_dump_format(m_formatContext, 0, m_sourceFilename.c_str(), 0);
	if (!m_audioStream && !m_videoStream) {
		APPL_ERROR("Could not find audio or video stream in the input, aborting");
		ret = 1;
		return; // TODO : An error occured ... !!!!!
	}
	m_frame = av_frame_alloc();
	if (!m_frame) {
		APPL_ERROR("Could not allocate frame");
		ret = AVERROR(ENOMEM);
		return; // TODO : An error occured ... !!!!!
	}
	// initialize packet, set data to nullptr, let the demuxer fill it
	av_init_packet(&m_packet);
	m_packet.data = nullptr;
	m_packet.size = 0;
}
bool appl::Decoder::onThreadCall() {
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
void appl::Decoder::uninit() {
	// flush cached frames
	m_packet.data = nullptr;
	m_packet.size = 0;
	int gotFrame;
	do {
		decode_packet(&gotFrame, 1);
	} while (gotFrame);
	APPL_PRINT("Demuxing & Decoding succeeded...");
	avcodec_close(m_videoDecoderContext);
	avcodec_close(m_audioDecoderContext);
	avformat_close_input(&m_formatContext);
	av_frame_free(&m_frame);
	av_free(m_videoDestinationRGBData[0]);
}

int32_t appl::Decoder::videoGetEmptySlot() {
	for (int32_t iii=0; iii<m_videoPool.size(); ++iii) {
		if (m_videoPool[iii].m_isUsed == false) {
			return iii;
		}
	}
	return -1;
}

int32_t appl::Decoder::audioGetEmptySlot() {
	for (int32_t iii=0; iii<m_audioPool.size(); ++iii) {
		if (m_audioPool[iii].m_isUsed == false) {
			return iii;
		}
	}
	return -1;
}

int32_t appl::Decoder::videoGetOlderSlot() {
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

int32_t appl::Decoder::audioGetOlderSlot() {
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
	m_decoder.init(_filename);
	if (m_decoder.haveAudio() == true) {
		//Get the generic input:
		m_audioInterface = m_audioManager->createOutput(m_decoder.audioGetSampleRate(),
		                                                m_decoder.audioGetChannelMap(),
		                                                m_decoder.audioGetFormat(),
		                                                "speaker");
		if(m_audioInterface == nullptr) {
			APPL_ERROR("Can not creata Audio interface");
		}
		m_audioInterface->setReadwrite();
		m_audioInterface->start();
	}
	
	m_decoder.start();
	m_audioManager->generateDotAll("out/local_player_flow.dot");
	markToRedraw();
}

bool appl::widget::VideoDisplay::isPlaying() {
	return m_isPalying;
}

void appl::widget::VideoDisplay::play() {
	m_isPalying = true;
}

void appl::widget::VideoDisplay::pause() {
	m_isPalying = false;
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
	//APPL_WARNING("Display image : " << m_VBO->bufferSize(m_vboIdCoord));
	gale::openGL::disable(gale::openGL::flag_depthTest);
	// set Matrix : translation/positionMatrix
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
		m_currentTime += _event.getDeltaCallDuration();
	}
	// SET AUDIO:
	int32_t idSlot = m_decoder.audioGetOlderSlot();
	if (    idSlot != -1
	     && m_currentTime > m_decoder.m_audioPool[idSlot].m_time) {
		int32_t nbSample =   m_decoder.m_audioPool[idSlot].m_buffer.size()
		                   / audio::getFormatBytes(m_decoder.m_audioPool[idSlot].m_format)
		                   / m_decoder.m_audioPool[idSlot].m_map.size();
		m_audioInterface->write(&m_decoder.m_audioPool[idSlot].m_buffer[0], nbSample);
		m_decoder.m_audioPool[idSlot].m_isUsed = false;
	}
	// SET VIDEO:
	idSlot = m_decoder.videoGetOlderSlot();
	// check the slot is valid and check display time of the element:
	if (    idSlot != -1
	     && m_currentTime > m_decoder.m_videoPool[idSlot].m_time) {
		m_resource->get().swap(m_decoder.m_videoPool[idSlot].m_image);
		m_imageSize = m_resource->get().getSize();
		ivec2 tmpSize = m_decoder.m_videoPool[idSlot].m_imagerealSize;
		m_decoder.m_videoPool[idSlot].m_imagerealSize = m_videoSize;
		m_videoSize = tmpSize;
		m_decoder.m_videoPool[idSlot].m_isUsed = false;
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
	markToRedraw();
}

