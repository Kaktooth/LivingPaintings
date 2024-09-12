/*MIT License

Copyright (c) 2023 Oscar David

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SAMCPP__SAM_H_
#define SAMCPP__SAM_H_

#include <list>
#include <opencv2/core.hpp>
#include <string>

struct SamModel;

#if _MSC_VER
class __declspec(dllexport) Sam {
#else
class Sam {
#endif
    SamModel* m_model { nullptr };

public:
    struct Parameter {
        // Partial options of OrtCUDAProviderOptions to hide the dependency on onnxruntime
        struct Provider {
            // deviceType: 0 - CPU, 1 - CUDA
            int gpuDeviceId { 0 }, deviceType { 0 };
            size_t gpuMemoryLimit { 0 };
        };
        Provider providers[2]; // 0 - embedding, 1 - segmentation
        std::string models[2]; // 0 - embedding, 1 - segmentation
        int threadsNumber { 1 };
        Parameter(const std::string& preModelPath, const std::string& samModelPath, int threadsNumber)
        {
            models[0] = preModelPath;
            models[1] = samModelPath;
            this->threadsNumber = threadsNumber;
        }
    };

    // This constructor is deprecated (cpu runtime only)
    Sam(const std::string& preModelPath, const std::string& samModelPath, int threadsNumber);
    // Recommended constructor
    Sam(const Parameter& param);
    ~Sam();

    cv::Size getInputSize() const;

    bool setWindowResolution(int width, int height);

    bool loadImage(const cv::Mat& image);

    cv::Mat getMask(const std::list<cv::Point>& points, const std::list<cv::Point>& negativePoints,
        const cv::Rect& roi, double* iou = nullptr) const;
    cv::Mat getMask(const std::list<cv::Point>& points, const std::list<cv::Point>& negativePoints,
        double* iou = nullptr) const;
    cv::Mat getMask(const cv::Point& point, double* iou = nullptr) const;

    using cbProgress = void (*)(double);
    cv::Mat autoSegment(const cv::Size& numPoints, cbProgress cb = {},
        const double iouThreshold = 0.86, const double minArea = 100,
        int* numObjects = nullptr) const;
};

#endif // SAMCPP__SAM_H_
