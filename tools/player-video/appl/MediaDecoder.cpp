/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>

#include <appl/debug.hpp>
#include <appl/widget/VideoPlayer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>
#include <egami/egami.hpp>
#include <utility>

#define BUFFER_SIZE_GET_SLOT (1024*512)


static int g_readFunc(void* _opaque, uint8_t* _buf, int _bufSize) {
	if (_opaque == nullptr) {
		return 0;
	}
	return static_cast<appl::MediaDecoder*>(_opaque)->readFunc(_buf, _bufSize);
}

static int g_writeFunc(void* _opaque, uint8_t* _buf, int _bufSize) {
	if (_opaque == nullptr) {
		return 0;
	}
	return static_cast<appl::MediaDecoder*>(_opaque)->writeFunc(_buf, _bufSize);
}

static int64_t g_seekFunc(void* _opaque, int64_t _offset, int _whence) {
	if (_opaque == nullptr) {
		return 0;
	}
	return static_cast<appl::MediaDecoder*>(_opaque)->seekFunc(_offset, _whence);
}



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
	m_IOContext = nullptr;
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
			APPL_VERBOSE("video_frame " << (_cached?"(cached)":"")
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
				APPL_VERBOSE("audio_frame " << (_cached?"(cached)":"")
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

void appl::MediaDecoder::init(ememory::SharedPtr<ClientProperty> _property, uint32_t _mediaId) {
	// TODO : Correct this later ... We first download the media and after we play it
	// TODO : We need to download only a small part ...
	// get the requested node:
	if (_property == nullptr) {
		APPL_ERROR("Request play of not handle property ==> nullptr");
		return;
	}
	if (_property->connection.isAlive() == false) {
		APPL_ERROR("Request play of not connected handle ==> 'not alive'");
		return;
	}
	zeus::service::ProxyVideo remoteServiceVideo = _property->connection.getService("video");
	// remove all media (for test)
	if (remoteServiceVideo.exist() == false) {
		APPL_ERROR("Video service is ==> 'not alive'");
		return;
	}
	m_remote = ememory::makeShared<appl::StreamBuffering>();
	m_remote->m_bufferReadPosition = 0;
	m_remote->m_property = _property;
	m_remote->m_mediaId = _mediaId;
	remoteServiceVideo.mediaGet(_mediaId).andThen(
	    [=](zeus::Future<zeus::ProxyFile> _fut) mutable {
	    	APPL_INFO("Receive ProxyFile");
	    	m_remote->m_fileHandle = _fut.get();
	    	m_remote->m_fileHandle.getSize().andThen(
	    	    [=](zeus::Future<uint64_t> _fut) mutable {
	    	    	APPL_INFO("Receive FileSize to index property");
	    	    	m_remote->m_buffer.resize(_fut.get(), 0);
	    	    	m_remote->checkIfWeNeedMoreDataFromNetwork();
	    	    	return true;
	    	    });
	    	return true;
	    });
}

int appl::MediaDecoder::readFunc(uint8_t* _buf, int _bufSize) {
	APPL_INFO("call read ... " << m_remote->m_bufferReadPosition << "  size=" << _bufSize);
	// check if enought data:
	int64_t readableSize = m_remote->sizeReadable();
	if (_bufSize > readableSize) {
		_bufSize = readableSize;
	}
	if (_bufSize == 0) {
		// No data in the buffer
		return 0;
	}
	{
		std::unique_lock<std::mutex> lock(m_remote->m_mutex);
		memcpy(_buf, &m_remote->m_buffer[m_remote->m_bufferReadPosition], _bufSize);
		m_remote->m_bufferReadPosition += _bufSize;
	}
	m_remote->startStream();
	m_remote->checkIfWeNeedMoreDataFromNetwork();
	return _bufSize;
}

int32_t appl::StreamBuffering::sizeReadable() {
	std::unique_lock<std::mutex> lock(m_mutex);
	for (auto &it : m_bufferFillSection) {
		if (    m_bufferReadPosition >= it.first
		     && m_bufferReadPosition < it.second) {
			return it.second - m_bufferReadPosition;
		}
	}
	// No data in the buffer
	return 0;
}


int appl::MediaDecoder::writeFunc(uint8_t* _buf, int _bufSize) {
	APPL_ERROR("call write ...");
	return _bufSize;
}

int64_t appl::MediaDecoder::seekFunc(int64_t _offset, int _whence) {
	int64_t lastPosition = m_remote->m_bufferReadPosition;
	switch (_whence) {
		case AVSEEK_SIZE:
			APPL_ERROR("call seek 'SIZE' ... " << m_remote->m_buffer.size());
			return m_remote->m_buffer.size();
		case AVSEEK_FORCE:
			APPL_ERROR("call seek 'FORCE' ... pos=" << _offset << " size=" << m_remote->m_buffer.size());
			m_remote->m_bufferReadPosition = _offset;
			break;
		case SEEK_SET:
			APPL_ERROR("call seek 'SET' ... pos=" << _offset << " size=" << m_remote->m_buffer.size());
			m_remote->m_bufferReadPosition = _offset;
			break;
		case SEEK_CUR:
			APPL_ERROR("call seek 'CUR' ... _offset=" << _offset << " size=" << m_remote->m_buffer.size());
			m_remote->m_bufferReadPosition += _offset;
			break;
		case SEEK_END:
			APPL_ERROR("call seek 'END' ... _end=" << _offset << " size=" << m_remote->m_buffer.size());
			m_remote->m_bufferReadPosition = m_remote->m_buffer.size()-_offset;
			break;
		default:
			APPL_ERROR("Unknow the _whence=" << _whence);
			return AVERROR(EINVAL);
	}
	if (m_remote->m_bufferReadPosition < 0 ) {
		APPL_WARNING("Request seek before start of the File");
		m_remote->m_bufferReadPosition = 0;
	}
	if (m_remote->m_bufferReadPosition > m_remote->m_buffer.size()) {
		APPL_WARNING("Request seek after end of the File");
		m_remote->m_bufferReadPosition = m_remote->m_buffer.size()-1;
	}
	if (lastPosition != m_remote->m_bufferReadPosition) {
		{
			// Force the get of new data ==> this is bad TODO : Update this when possible
			std::unique_lock<std::mutex> lock(m_remote->m_mutex);
			m_remote->m_callInProgress = false;
		}
		m_remote->checkIfWeNeedMoreDataFromNetwork();
	}
	return m_remote->m_bufferReadPosition;
}

bool appl::StreamBuffering::addDataCallback(zeus::Future<zeus::Raw> _fut, int64_t _positionRequest) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		bool find = false;
		m_callInProgress = false;
		if (_fut.hasError() == true) {
			APPL_ERROR("Error when loading data (1 MB)");
			return true;
		}
		// TODO : Check buffer size ...
		zeus::Raw buffer = _fut.get();
		APPL_INFO("    ==> receive DATA : " << _positionRequest << " size=" << buffer.size());
		// copy data
		memcpy(&m_buffer[_positionRequest], buffer.data(), buffer.size());
		// Update the buffer data and positionning
		// find if the position correspond at a last positioning:
		
		auto it = m_bufferFillSection.begin();
		while (it != m_bufferFillSection.end()) {
			if (    _positionRequest >= it->first
			     && _positionRequest < it->second) {
				if (_positionRequest + buffer.size() > it->second){
					it->second = _positionRequest + buffer.size();
				}
				find = true;
				break;
			} else if (it->second == _positionRequest) {
				it->second += buffer.size();
				find = true;
				break;
			}
			auto it2 = it;
			++it2;
			if (    it2 != m_bufferFillSection.end()
			     && _positionRequest + buffer.size() >= it2->first) {
				it2->first = _positionRequest;
				find = true;
				break;
			} else {
				find = true;
				break;
			}
			++it;
		}
		if (find == false) {
			m_bufferFillSection.insert(it, std::pair<uint32_t,uint32_t>(_positionRequest, _positionRequest + buffer.size()));
		}
	}
	checkIfWeNeedMoreDataFromNetwork();
	return true;
}


appl::StreamBuffering::StreamBuffering() {
	m_callInProgress = false;
	m_stopRequested = false;
	m_mediaId = 0;
	m_bufferReadPosition = 0;
}
void appl::StreamBuffering::stopStream() {
	m_stopRequested = true;
}
void appl::StreamBuffering::startStream() {
	m_stopRequested = false;
}


void appl::StreamBuffering::checkIfWeNeedMoreDataFromNetwork() {
	std::unique_lock<std::mutex> lock(m_mutex);
	// check if enought data:
	bool find = false;
	if (m_callInProgress == true) {
		return;
	}
	if (m_stopRequested == true) {
		return;
	}
	int32_t preDownloadBufferSlot = BUFFER_SIZE_GET_SLOT*3;
	// When file is < 200Mo ==> just download all...
	if (m_buffer.size() < 300*1024*1024) {
		preDownloadBufferSlot = m_buffer.size()+10;
	}
	APPL_VERBOSE("Request DATA ...");
	auto it = m_bufferFillSection.begin();
	if (it == m_bufferFillSection.end()) {
		// no data in the buffer...
		//Get some
		// start with loading of 1 Mo
		APPL_VERBOSE("Request DATA : " << 0 << " size=" << BUFFER_SIZE_GET_SLOT);
		auto futData = m_fileHandle.getPart(0, BUFFER_SIZE_GET_SLOT);
		auto localShared = ememory::dynamicPointerCast<appl::StreamBuffering>(sharedFromThis());
		futData.andThen([=](zeus::Future<zeus::Raw> _fut) mutable {
		                	return localShared->addDataCallback(_fut, 0);
		                });
		m_callInProgress = true;
		return;
	}
	while (it != m_bufferFillSection.end()) {
		APPL_VERBOSE("Check : " << it->first << " -> " << it->second << "        read-pos=" << m_bufferReadPosition);
		if (    m_bufferReadPosition >= it->first
		     && m_bufferReadPosition < it->second) {
			find = true;
			// part already download... ==> check if we need more data after end position
			if (it->second == m_buffer.size()) {
				// need no more data ...
				return;
			}
			if (it->second - m_bufferReadPosition < preDownloadBufferSlot) {
				int32_t sizeRequest = BUFFER_SIZE_GET_SLOT;
				if (it->second + sizeRequest >= m_buffer.size()) {
					sizeRequest = m_buffer.size() - it->second;
				}
				auto it2 = it;
				++it2;
				if (    it2 != m_bufferFillSection.end()
				     && it->second + sizeRequest >= it2->first) {
					sizeRequest = it2->first - it->second;
				}
				APPL_VERBOSE("Request DATA : " << it->second << " size=" << sizeRequest);
				auto futData = m_fileHandle.getPart(it->second, it->second + sizeRequest);
				auto localShared = ememory::dynamicPointerCast<appl::StreamBuffering>(sharedFromThis());
				futData.andThen([=](zeus::Future<zeus::Raw> _fut) mutable {
				                	return localShared->addDataCallback(_fut, it->second);
				                });
				m_callInProgress = true;
			}
			// nothing more to do ...
			return;
		} else if (m_bufferReadPosition < it->first) {
			int32_t sizeRequest = BUFFER_SIZE_GET_SLOT;
			if (m_bufferReadPosition + sizeRequest >= it->first) {
				sizeRequest = it->first - m_bufferReadPosition;
			}
			APPL_VERBOSE("Request DATA : " << m_bufferReadPosition << " size=" << sizeRequest);
			auto futData = m_fileHandle.getPart(m_bufferReadPosition, m_bufferReadPosition+sizeRequest);
			auto localShared = ememory::dynamicPointerCast<appl::StreamBuffering>(sharedFromThis());
			futData.andThen([=](zeus::Future<zeus::Raw> _fut) mutable {
			                	return localShared->addDataCallback(_fut, m_bufferReadPosition);
			                });
			m_callInProgress = true;
			// nothing more to do ...
			return;
		}
		++it;
	}
	APPL_VERBOSE("Request DATA : " << m_bufferReadPosition << " size=" << BUFFER_SIZE_GET_SLOT);
	auto futData = m_fileHandle.getPart(m_bufferReadPosition, m_bufferReadPosition + BUFFER_SIZE_GET_SLOT);
	auto localShared = ememory::dynamicPointerCast<appl::StreamBuffering>(sharedFromThis());
	futData.andThen([=](zeus::Future<zeus::Raw> _fut) mutable {
	                	return localShared->addDataCallback(_fut, 0);
	                });
	m_callInProgress = true;
	if (find == false) {
		// No data in the buffer
		return;
	}
}
#define APPL_BUFFER_SIZE_FOR_FFMPEG (256*1024)

void appl::MediaDecoder::init() {
	if (    m_isInit == true
	     || m_remote == nullptr
	     || m_remote->sizeReadable() < 1024*1024) {// Need to wait at lease 1MB
		
		return;
	}
	m_remote->startStream();
	m_updateVideoTimeStampAfterSeek = false;
	//m_sourceFilename = _filename;
	ethread::setName("ffmpegThread");
	// open input file, and allocate format context
	#ifdef APPL_USE_GENERIC_FFMPEG
		if (avformat_open_input(&m_formatContext, m_sourceFilename.c_str(), nullptr, nullptr) < 0) {
			APPL_ERROR("Could not open source file " << m_sourceFilename);
			return;
		}
	#else
		if (!(m_formatContext = avformat_alloc_context())) {
			APPL_ERROR("Could not create Format context");
			return;
		}
		uint8_t* ploooooo = (uint8_t*)av_malloc(APPL_BUFFER_SIZE_FOR_FFMPEG);
		m_IOContext = avio_alloc_context(ploooooo, APPL_BUFFER_SIZE_FOR_FFMPEG, 0 /* can not write */, this, g_readFunc, g_writeFunc, g_seekFunc);
		if (m_IOContext == nullptr) {
			APPL_ERROR("Could not create IO stream");
			return;
		}
		// set IO read and write interface
		m_formatContext->pb = m_IOContext;
		if (avformat_open_input(&m_formatContext, nullptr, nullptr, nullptr) < 0) {
			APPL_ERROR("Could not open source file " << m_sourceFilename);
			return;
		}
	#endif
	// retrieve stream information
	if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
		APPL_ERROR("Could not find stream information");
		// TODO : check this, this will create a memeory leak
		return;
	}
	APPL_ERROR("BBBBBB");
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
	APPL_ERROR("ZZZZZZZ");
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
	init();
	if (m_isInit == false) {
		// take some time to sleep the decoding ...
		std::this_thread::sleep_for(std::chrono::milliseconds(60/100));
		return false;
	}
	if (m_seek >= echrono::Duration(0)) {
		// seek requested (create a copy to permit to update it in background):
		echrono::Duration tmpSeek = m_seek;
		m_seek = echrono::Duration(-1);
		applySeek(tmpSeek);
	}
	// Need to wait at lease 1MB
	if (m_remote->sizeReadable() < APPL_BUFFER_SIZE_FOR_FFMPEG) {
		// take some time to sleep the decoding ...
		std::this_thread::sleep_for(std::chrono::milliseconds(60/100));
		return false;
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
	flushMessage();
	avcodec_close(m_videoDecoderContext);
	avcodec_close(m_audioDecoderContext);
	m_formatContext->pb = nullptr;
	avformat_close_input(&m_formatContext);
	av_frame_free(&m_frame);
	av_free(m_IOContext);
	m_IOContext = nullptr;
	m_audioPool.clear();
	m_videoPool.clear();
	m_updateVideoTimeStampAfterSeek = false;
	m_videoStream = nullptr;
	m_audioStream = nullptr;
	m_sourceFilename = "";
	m_videoStream_idx = 0;
	m_audioStream_idx = 0;
	m_videoFrameCount = 0;
	m_audioFrameCount = 0;
	// output format convertion:
	m_convertContext = nullptr;
	m_remote.reset();
	m_isInit = false;
	APPL_PRINT("Demuxing & Decoding succeeded... (DONE)");
}

void appl::MediaDecoder::stop() {
	m_stopRequested = true;
	if (m_remote != nullptr) {
		m_remote->stopStream();
	}
	gale::Thread::stop();
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
