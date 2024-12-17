#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <format>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/avutil.h"
#include <libavutil/opt.h>
#include "libswscale/swscale.h"
}

class FileSupport {

	//TODO create queue of files to be written
	static uint32_t frameCount;

public:
	static void setFrameSize(uint32_t frameCount);
	static void gatherFrame(unsigned char* frame, bool& writeVideo, std::string fileFormat);
	static void writeFramesToStream(AVCodecID codecId, AVPixelFormat frameFormat,
		uint32_t frameTimestampModifier, std::string fileFormat);
};