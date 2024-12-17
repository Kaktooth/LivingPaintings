#include "frame_exporter.h"
#include "../vulkan/consts.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using Constants::OUTPUT_FOLDER_NAME;
using Constants::WINDOW_WIDTH;
using Constants::WINDOW_HEIGHT;
using Constants::EXPORT_FRAME_COUNT;
using Constants::STREAM_FRAME_RATE;

#define 	STREAM_CODEC_ID  AV_CODEC_ID_MPEG4
#define 	STREAM_GIF_CODEC_ID AV_CODEC_ID_GIF
#define 	STREAM_PIX_FMT  AV_PIX_FMT_YUV420P
#define 	STREAM_GIF_PIX_FMT   AV_PIX_FMT_RGB8
#define 	STREAM_FRAME_RATE   STREAM_FRAME_RATE

std::vector<unsigned char*> frames;
uint32_t FileSupport::frameCount = EXPORT_FRAME_COUNT;

void FileSupport::setFrameSize(uint32_t frameCount)
{
	FileSupport::frameCount = frameCount;
}

void FileSupport::gatherFrame(unsigned char* frameCopy, bool& writeVideo, std::string fileFormat)
{
	frames.push_back(frameCopy);
	if (frames.size() > FileSupport::frameCount + 1) { // first frames can contain gui, so more frames being added to array and bottom frames will be erased
		auto it = frames.begin();
		frames.erase(it, it + 2);
		if (fileFormat == ".gif") {
			writeFramesToStream(STREAM_GIF_CODEC_ID, STREAM_GIF_PIX_FMT, 1, fileFormat);
		} else if (fileFormat == ".mp4") {
			writeFramesToStream(STREAM_CODEC_ID, STREAM_PIX_FMT, 500, fileFormat);
		}
		writeVideo = false;
	}
}

void FileSupport::writeFramesToStream(AVCodecID codecId, AVPixelFormat frameFormat, uint32_t frameTimestampModifier, std::string fileFormat)
{
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string currentTimeString(30, '\0');
	std::strftime(&currentTimeString[0], currentTimeString.size(),
		"%Y-%m-%d_%H%M%S", std::localtime(&currentTime));
	const std::string OUTPUT_FILE_NAME = OUTPUT_FOLDER_NAME + "/animated-painting_" + currentTimeString.c_str() + fileFormat;
	int res;

	avformat_network_init();

	AVFormatContext* outputFormatContext = nullptr;
	avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, OUTPUT_FILE_NAME.c_str());
	if (!outputFormatContext) {
		std::cerr << " Could not create video output!" << std::endl;
	}

	const AVCodec* gifCodec = avcodec_find_encoder(codecId);
	AVCodecContext* codecContext = avcodec_alloc_context3(gifCodec);
	codecContext->codec_id = codecId;
	codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
	codecContext->bit_rate = 400000;
	codecContext->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
	codecContext->framerate = (AVRational){ STREAM_FRAME_RATE, 1 };
	codecContext->gop_size = 6;
	codecContext->pix_fmt = frameFormat;
	codecContext->width = WINDOW_WIDTH;
	codecContext->height = WINDOW_HEIGHT;

	if (outputFormatContext->flags & AVFMT_GLOBALHEADER) {
		codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	if (avcodec_open2(codecContext, gifCodec, nullptr)) {
		std::cerr << "Could not open codec" << std::endl;
	}

	AVStream* outputStream = avformat_new_stream(outputFormatContext, gifCodec);
	if (!outputStream) {
		std::cerr << "Could not allocate output stream" << std::endl;
	}

	outputStream->avg_frame_rate = codecContext->framerate;
	outputStream->codecpar->codec_id = codecContext->codec_id;
	outputStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	outputStream->codecpar->width = WINDOW_WIDTH;
	outputStream->codecpar->height = WINDOW_HEIGHT;
	outputStream->codecpar->format = codecContext->pix_fmt;
	outputStream->time_base = codecContext->time_base;
	outputStream->avg_frame_rate = (AVRational){ STREAM_FRAME_RATE };

	res = avcodec_parameters_from_context(outputStream->codecpar, codecContext);
	if (res < 0) {
		std::cerr << "Error: Could not copy codec parameters to output stream" << std::endl;
	}

	if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
		if (avio_open(&outputFormatContext->pb, OUTPUT_FILE_NAME.c_str(), AVIO_FLAG_WRITE) < 0) {
			std::cerr << "Could not open output file" << std::endl;
		}
	}

	res = avformat_write_header(outputFormatContext, nullptr);
	if (res < 0) {
		std::cerr << "Error: Could not write output header" << std::endl;
	}

	SwsContext* swsCtxRGBtoYUV = sws_getContext(WINDOW_WIDTH, WINDOW_HEIGHT, AV_PIX_FMT_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, frameFormat, SWS_BILINEAR | SWS_ACCURATE_RND, 0, 0, 0);
	if (!swsCtxRGBtoYUV) {
		std::cerr << "Could not initialize the conversion context\n" << std::endl;
		exit(1);
	}

	int frameCount = 0;
	AVFrame* frame = av_frame_alloc();
	frame->format = frameFormat;
	frame->width = WINDOW_WIDTH;
	frame->height = WINDOW_HEIGHT;
	while (frameCount < frames.size()) {
		res = av_frame_get_buffer(frame, 0);
		av_image_alloc(frame->data, frame->linesize,
			WINDOW_WIDTH, WINDOW_HEIGHT, frameFormat, 32);

		uint8_t* data[1] = { frames[frameCount] };
		int linesize[1] = { 4 * WINDOW_WIDTH };
		res = sws_scale(swsCtxRGBtoYUV, data, linesize, 0, WINDOW_HEIGHT, frame->data, frame->linesize);

		uint32_t frameTimestamp = frameTimestampModifier * frameCount;
		frame->pts = frameTimestamp;

		res = avcodec_send_frame(codecContext, frame);
		if (res < 0) {
			std::cerr << "Error: Could not send frame to output codec" << std::endl;
		}

		AVPacket outPacket;
		av_init_packet(&outPacket);
		outPacket.data = nullptr;
		outPacket.size = 0;
		while (res >= 0)
		{
			res = avcodec_receive_packet(codecContext, &outPacket);
			if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
			{
				break;
			}
			else if (res < 0) {
				std::cerr << "Error: Could not receive packet from output codec" << std::endl;
			}
			outPacket.stream_index = outputStream->index;
			outPacket.pts = frameTimestamp;

			res = av_interleaved_write_frame(outputFormatContext, &outPacket);

			static int call_write = 0;

			call_write++;
			printf("av_interleaved_write_frame %d\n", call_write);

		}
		av_packet_unref(&outPacket);
		frameCount++;
	}

	frames.clear();

	av_write_trailer(outputFormatContext);

	av_free(frame->data[0]);
	av_free(frame);
	avcodec_close(codecContext);
	av_free(codecContext);

	printf("\n");
	avio_close(outputFormatContext->pb);
}