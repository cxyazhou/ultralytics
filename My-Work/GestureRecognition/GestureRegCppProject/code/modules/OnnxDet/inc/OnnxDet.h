#ifndef __ONNX_DET_H__
#define __ONNX_DET_H__

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdint>

#include "opencv2/opencv.hpp"

// 用标准类型替代 uint / uint64
using uint = unsigned int;
using uint64 = unsigned long long;

struct OnnxDetRst
{
    int         class_id;
    float       confidence;
    cv::Rect2f  bbox;  // 使用浮点矩形，保留精度
};

// 结构体用于存储填充信息和缩放比例
struct PaddingInfo
{
    float   scale;
    int     left;
    int     top;
    int     right;
    int     bottom;
};

class OnnxDet
{
public:
	OnnxDet();
    ~OnnxDet();

    int Init();
    int UnInit();

    PaddingInfo ResizePadding(const cv::Mat& src, cv::Mat& dst, const cv::Size& target_size, const cv::Scalar& color = cv::Scalar(114, 114, 114));
    cv::Rect2f  ResizePaddingMapRect(const cv::Rect& bbox, const cv::Size& orig_size, const PaddingInfo& pad_info);
    std::vector<float> PreProcessImage(const cv::Mat& image);
    int ProcessImage(cv::Mat& image, std::vector<OnnxDetRst>& rst);
    void drawRect(cv::Mat& image, std::vector<OnnxDetRst>& rst);

    void CreatTrackHandle(int w, int h);
    void Track(cv::Mat& image, std::vector<OnnxDetRst>& stObj);

    int detObjs(cv::Mat& src, cv::Size orig_size, PaddingInfo& paddingInfo);

private:
    int ThreadProc();

private:
    // ---- 线程与同步 ----
    std::thread             m_thread;
    std::atomic<bool>       m_stop{ false };          // 停止标志
    std::atomic<bool>       m_has_task{ false };      // 是否有待处理任务
    std::atomic<bool>       m_done{ false };          // 结果是否就绪
    std::mutex              m_mtx;
    std::condition_variable m_cv;

    int                     m_width;
    int                     m_height;
    unsigned int            m_obj_det_cnt;

    cv::Mat                 m_det_img;
    std::vector<OnnxDetRst> m_rst;
    std::vector<cv::Rect>   m_obj_rst;
};

#endif
