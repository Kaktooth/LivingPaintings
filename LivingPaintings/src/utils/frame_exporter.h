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

class FrameExport {

	//TODO create queue of files to be written
	static uint32_t frameCount;
	static uint32_t windowWidth;
	static uint32_t windowHeight;
	static AVPixelFormat presentationSurfaceFormat;

public:
	static void setPresentationSurfaceFormat(uint32_t surfaceFormat);
	static void setExportParams(uint32_t frameCount, uint32_t windowWidth, uint32_t windowHeight);
	static void gatherFrame(std::shared_ptr<unsigned char> frame, bool& writeVideo, std::string& fileFormat);
	static void writeFramesToStream(AVCodecID codecId, AVPixelFormat frameFormat,
		uint32_t frameTimestampModifier, std::string& fileFormat);
};