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

// TODO: Remove this ==> this is bad ...
ememory::SharedPtr<appl::widget::VideoDisplay> g_diplayElement;


appl::Decoder::Decoder() {
	m_formatContext = nullptr;
	m_videoDecoderContext = nullptr;
	m_audioDecoderContext = nullptr;
	m_size = ivec2(0,0);
	m_videoStream = nullptr;
	m_audioStream = nullptr;
	
	m_videoDestinationData[0] = nullptr;
	
	m_videoDestinationRGBData[0] = nullptr;
	
	m_videoStream_idx = -1;
	m_audioStream_idx = -1;
	m_frame = nullptr;
	m_videoFrameCount = 0;
	m_audioFrameCount = 0;
	
	// output format convertion:
	m_convertContext = nullptr;
	
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
			APPL_INFO("video_frame " << (_cached?"(cached)":"")
			          << " n=" << m_videoFrameCount
			          << " coded_n=" << m_frame->coded_picture_number
			          << " pts=" << av_ts2timestr(m_frame->pts, &m_videoDecoderContext->time_base));
			m_videoFrameCount++;
			// copy decoded frame to destination buffer: this is required since rawvideo expects non aligned data
			if (false) {
				av_image_copy(m_videoDestinationData, m_videoDestinationLineSize,
				              (const uint8_t **)(m_frame->data), m_frame->linesize,
				              m_pixelFormat, m_size.x(), m_size.y());
			} else {
				// Convert Image in RGB:
				sws_scale(m_convertContext, (const uint8_t **)(m_frame->data), m_frame->linesize, 0, m_frame->height, m_videoDestinationRGBData, m_videoDestinationRGBLineSize);
				// Send it to the display engine ...
				if (g_diplayElement != nullptr) {
					g_diplayElement->setRawData(m_size, m_videoDestinationRGBData[0]);
				}
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
			size_t unPaddedLineSize = m_frame->nb_samples * av_get_bytes_per_sample((enum AVSampleFormat)m_frame->format);
			APPL_INFO("audio_frame " << (_cached?"(cached)":"")
			          << " n=" << m_audioFrameCount
			          << " nb_samples=" << m_frame->nb_samples
			          << " pts=" << av_ts2timestr(m_frame->pts, &m_audioDecoderContext->time_base));
			m_audioFrameCount++;
			// Write the raw audio data samples of the first plane. This works fine for packed formats (e.g. AV_SAMPLE_FMT_S16).
			// However, most audio decoders output planar audio, which uses a separate plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			// In other words, this code will write only the first audio channel in these cases.
			// You should use libswresample or libavfilter to convert the frame to packed data.
			// TODO: fwrite(m_frame->extended_data[0], 1, unPaddedLineSize, m_audioDestinationFile);
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
		ret = av_image_alloc(m_videoDestinationData, m_videoDestinationLineSize, m_size.x(), m_size.y(), m_pixelFormat, 1);
		if (ret < 0) {
			APPL_ERROR("Could not allocate raw video buffer");
			return; // TODO : An error occured ... !!!!!
		}
		m_videoDestinationBufferSize = ret;
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
		m_audioStream = m_formatContext->streams[m_audioStream_idx];
		m_audioDecoderContext = m_audioStream->codec;
		
		// Number of channels: m_audioDecoderContext->channels
		// Framerate:          m_audioDecoderContext->sample_rate
		APPL_INFO("Open audio stream with audio property: " << int32_t(m_audioDecoderContext->channels) << " channel(s) & samplerate=" << m_audioDecoderContext->sample_rate << " Hz");
		bool isPlanar = false;
		switch(m_audioDecoderContext->sample_fmt) {
			case AV_SAMPLE_FMT_NONE:
				APPL_ERROR("Unsupported audio format : UNKNOW ...");
				break;
			case AV_SAMPLE_FMT_U8:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_U8");
				isPlanar = false;
				break;
			case AV_SAMPLE_FMT_S16:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_S16");
				isPlanar = false;
				break;
			case AV_SAMPLE_FMT_S32:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_S32 (double)");
				isPlanar = false;
				break;
			case AV_SAMPLE_FMT_FLT:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_FLT (float)");
				isPlanar = false;
				break;
			case AV_SAMPLE_FMT_DBL:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_DBL");
				APPL_ERROR("    ==> Unsupported audio format : double");
				break;
			case AV_SAMPLE_FMT_U8P:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_U8P (planar)");
				isPlanar = true;
				break;
			case AV_SAMPLE_FMT_S16P:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_S16P (planar)");
				isPlanar = true;
				break;
			case AV_SAMPLE_FMT_S32P:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_S32P (planar)");
				isPlanar = true;
				break;
			case AV_SAMPLE_FMT_FLTP:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_FLTP (float/planar)");
				isPlanar = true;
				break;
			case AV_SAMPLE_FMT_DBLP:
				APPL_INFO("ffmpeg mode : AV_SAMPLE_FMT_DBLP (double/planar)");
				APPL_ERROR("    ==> Unsupported audio format : double Planar");
				break;
			case AV_SAMPLE_FMT_NB:
				APPL_ERROR("Unsupported audio format : Wrong ID ...");
				break;
			default:
				APPL_CRITICAL("Unsupported audio format :" << m_audioDecoderContext->sample_fmt << " ...");
				break;
		}
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
	APPL_INFO("Work on decoding");
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
	av_free(m_videoDestinationData[0]);
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
	g_diplayElement = ememory::dynamicPointerCast<appl::widget::VideoDisplay>(sharedFromThis());
}
void appl::widget::VideoDisplay::setRawData(const ivec2& _size, void* _dataPointer) {
	APPL_INFO("setRaw Data ... " << _size << " " << int64_t(_dataPointer) << " " << int64_t(m_resource.get()));
	if (m_resource == nullptr) {
		m_resource = ewol::resource::Texture::create();
		if (m_resource == nullptr) {
			EWOL_ERROR("Can not CREATE Image resource");
			return;
		}
		m_videoSize = ivec2(0,0);
	}
	if (m_videoSize != _size) {
		// Resize the buffer:
		m_videoSize = _size;
		m_resource->setImageSize(m_videoSize);
		m_imageSize = m_resource->get().getSize();
	}
	egami::Image& image = m_resource->get();
	uint8_t* pointer = (uint8_t*)_dataPointer;
	for (int32_t yyy=0; yyy<_size.y(); ++yyy) {
		for (int32_t xxx=0; xxx<_size.x(); ++xxx) {
			uint8_t r = *pointer++;
			uint8_t g = *pointer++;
			uint8_t b = *pointer++;
			etk::Color<>(r,g,b);
			image.set(ivec2(xxx,yyy), etk::Color<>(r,g,b));
		}
	}
	//egami::store(image, "test.bmp");
	m_resource->flush();
	m_nbFramePushed++;
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
	m_decoder.start();
	markToRedraw();
}

void appl::widget::VideoDisplay::onDraw() {
	APPL_INFO("Draw ..................................");
	if (m_VBO->bufferSize(m_vboIdCoord) <= 0) {
		APPL_WARNING("Nothink to draw...");
		return;
	}
	if (m_resource == nullptr) {
		// this is a normale case ... the user can choice to have no image ...
		APPL_ERROR("No Resource ...");
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
	m_LastResetCounter += _event.getDeltaCallDuration();
	if (m_LastResetCounter > echrono::seconds(1)) {
		m_LastResetCounter.reset();
		signalFps.emit(m_nbFramePushed);
		m_nbFramePushed = 0;
	}
	markToRedraw();
}
