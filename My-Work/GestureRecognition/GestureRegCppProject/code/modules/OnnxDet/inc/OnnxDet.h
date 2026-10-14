#ifndef __ONNX_DET_H__
#define __ONNX_DET_H__

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

#include "opencv2/opencv.hpp"
#include "CPPBase.h"
#include "iva_lib.h"

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

typedef struct _TRACK_WORK_
{
	void*		            pvIVAFilter;					 /*iva处理模块指针*/
	IVA_OUT_INFO	        stIVAOut;						 /*iva模块输出信息*/
	IVA_MEM_TAB 	        iva_mem_tab;					 /*算法库内存管理器*/
	IVA_VIDEO_FRAME         iva_frame;						 /*输入图像*/
	BLOB_LIST               blob_list;                       /*基于检测框的团块链表*/
    IVA_RULE_LIST           ivaRuleList;                     /*规则结构体*/
}TRACK_WORK;

class OnnxDet : public CCPPObject
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
    int ThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout);

private:
    int                     m_width;
    int                     m_height;
	CCPPThreadWorker*       m_thread_worker;
    cv::Mat                 m_det_img;
    std::vector<OnnxDetRst> m_rst;
    int                     m_process_flag;
    int                     m_stop;

    TRACK_WORK              m_track;

    unsigned int            m_obj_det_cnt;
    std::vector<cv::Rect>   m_obj_rst;
};

#endif
