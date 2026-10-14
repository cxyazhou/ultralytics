#include "OnnxDet.h"

#include "onnxruntime_cxx_api.h"

using namespace cv;
using namespace std;

OnnxDet::OnnxDet()
{
    m_thread_worker = NULL;
    m_process_flag = 0;
    m_stop = 0;
    m_width = 320;
    m_height = 320;
    
    memset(&m_track, 0, sizeof(TRACK_WORK));

    m_obj_det_cnt = 0;
}

OnnxDet::~OnnxDet()
{
    UnInit();
}

int OnnxDet::Init()
{
    CreatTrackHandle(m_width, m_height);
    
    void* cast = this;
    m_thread_worker = new CCPPThreadWorker(2, "smart");
	m_thread_worker->Attach((CCPPObject*)cast, (CPPTHREADAPI)&OnnxDet::ThreadProc);
    
    return 0;
}

int OnnxDet::UnInit()
{
    m_stop = 1;
	if (m_thread_worker != NULL)
	{
        void* cast = this;
		m_thread_worker->Detach((CCPPObject*)cast, (CPPTHREADAPI)&OnnxDet::ThreadProc);
		CPPDel(m_thread_worker);
	}
    
    return 0;
}

int OnnxDet::detObjs(cv::Mat& src, cv::Size orig_size, PaddingInfo& paddingInfo)
{
    if(!m_rst.empty()) {
        return 0;
    }
    
    if(m_obj_det_cnt % 20 != 0) {
        m_obj_det_cnt++;
        return 0;
    }
    m_obj_det_cnt++;
    
    // 2. 转换到HSV颜色空间
    Mat hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);
    
    // 3. 图像预处理：降噪
    GaussianBlur(hsv, hsv, Size(5, 5), 1.0);
    
    // 4. K-means颜色聚类
    int clusterCount = 4; // 聚类数量
    Mat samples(src.rows * src.cols, 3, CV_32F);
    for (int y = 0; y < hsv.rows; y++) {
        for (int x = 0; x < hsv.cols; x++) {
            for (int z = 0; z < 3; z++) {
                samples.at<float>(y + x * hsv.rows, z) = hsv.at<Vec3b>(y, x)[z];
            }
        }
    }
    
    Mat labels;
    int attempts = 5;
    Mat centers;
    kmeans(samples, clusterCount, labels, TermCriteria(TermCriteria::MAX_ITER | TermCriteria::EPS, 10000, 0.0001), 
           attempts, KMEANS_PP_CENTERS, centers);
    
    // 5. 寻找主要颜色（最大面积的颜色）
    vector<int> counts(clusterCount, 0);
    for (int i = 0; i < labels.rows; i++) {
        counts[labels.at<int>(i)]++;
    }
    
    int mainCluster = 0;
    int maxCount = 0;
    for (int i = 0; i < clusterCount; i++) {
        if (counts[i] > maxCount) {
            maxCount = counts[i];
            mainCluster = i;
        }
    }
    
    // 6. 创建主色掩码
    Mat mask = Mat::zeros(src.size(), CV_8U);
    for (int y = 0; y < hsv.rows; y++) {
        for (int x = 0; x < hsv.cols; x++) {
            int clusterIdx = labels.at<int>(y + x * hsv.rows, 0);
            if (clusterIdx == mainCluster) {
                mask.at<uchar>(y, x) = 255;
            }
        }
    }
    
    // 7. 形态学操作优化掩码
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(mask, mask, MORPH_OPEN, kernel); // 开运算去除小噪点
    morphologyEx(mask, mask, MORPH_CLOSE, kernel); // 闭运算填充小孔洞
    
    // 8. 查找轮廓
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    // 9. 收集所有候选矩形及其面积
    struct CandidateRect {
        Rect rect;
        double area;
        double confidence; // 可添加其他特征
    };
    
    vector<CandidateRect> candidates;
    
    for (size_t i = 0; i < contours.size(); i++) {
        // 计算轮廓面积
        double area = contourArea(contours[i]);
        
        // 面积过滤
        if (area < 1000) 
            continue;
        if (area > src.cols * src.rows / 8) 
            continue;
        
        // 计算边界矩形
        Rect rect = boundingRect(contours[i]);
        
        // 添加候选目标
        candidates.push_back({rect, area, area});
    }
    
    // 10. 按面积升序排序（优先小目标）
    sort(candidates.begin(), candidates.end(), [](const CandidateRect& a, const CandidateRect& b) {
        return a.area < b.area; // 升序排列：小目标在前
    });
    
    // 11. 自定义NMS：优先保留小目标
    const double overlapThreshold = 0.3; // 重叠阈值
    vector<Rect> finalRects;
    
    // 创建标记数组，记录是否保留
    vector<bool> keep(candidates.size(), true);
    
    // 从最小目标开始处理
    for (size_t i = 0; i < candidates.size(); i++) {
        if (!keep[i]) continue; // 已被抑制
        
        // 保留当前小目标
        finalRects.push_back(candidates[i].rect);
        
        // 检查并抑制与当前目标重叠的大目标
        for (size_t j = i + 1; j < candidates.size(); j++) {
            if (!keep[j]) continue;
            
            // 计算IOU（交并比）
            Rect inter = candidates[i].rect & candidates[j].rect;
            double iou = (inter.area() > 0) ? 
                         (double)inter.area() / (candidates[i].rect | candidates[j].rect).area() : 
                         0.0;
            
            // 如果重叠超过阈值，抑制该目标
            if (iou > overlapThreshold) {
                keep[j] = false;
            }
        }
    }
    
    // 12. 可选的二次过滤：确保小目标不被遗漏
    // 对于被抑制的目标，如果与任何已保留目标重叠很小，可以恢复
    for (size_t i = 0; i < candidates.size(); i++) {
        if (keep[i]) continue; // 已保留
        
        bool isOverlap = false;
        for (const Rect& kept : finalRects) {
            Rect inter = candidates[i].rect & kept;
            double iou = (inter.area() > 0) ? 
                         (double)inter.area() / (candidates[i].rect | kept).area() : 
                         0.0;
            
            if (iou > overlapThreshold * 0.5) { // 较宽松的阈值
                isOverlap = true;
                break;
            }
        }
        
        // 如果没有显著重叠，恢复该小目标
        if (!isOverlap) {
            finalRects.push_back(candidates[i].rect);
            keep[i] = true;
        }
    }
    
    vector<Rect> outputObjs;
    for (const auto& d : finalRects)
    {
        Rect obj = ResizePaddingMapRect(d, orig_size, paddingInfo);
        outputObjs.push_back(obj);
    }
    
    m_obj_rst = outputObjs;
        
    return 0;
}

PaddingInfo OnnxDet::ResizePadding(const cv::Mat& src, cv::Mat& dst, const cv::Size& target_size, const cv::Scalar& color)
{
    // 获取原始尺寸
    int orig_h = src.rows;
    int orig_w = src.cols;

    //目标尺寸
    int target_w = target_size.width;
    int target_h = target_size.height;

    // 计算缩放比例
    float scale = CPPMIN(static_cast<float>(target_w) / orig_w,
        static_cast<float>(target_h) / orig_h);

    // 计算新尺寸
    int new_w = static_cast<int>(orig_w * scale);
    int new_h = static_cast<int>(orig_h * scale);

    // 缩放图像
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_LINEAR);

    // 计算填充位置
    int dw = target_w - new_w;
    int dh = target_h - new_h;
    int left = dw / 2;
    int top = dh / 2;
    int right = dw - left;
    int bottom = dh - top;

    // 创建填充图像
    dst = cv::Mat(target_h, target_w, src.type(), color);
    resized.copyTo(dst(cv::Rect(left, top, new_w, new_h)));

    // 返回填充信息
    return { scale, left, top, right, bottom };
}

/**
 * 将填充图像上的坐标映射回原始图像
 *
 * @param bbox 填充图像上的边界框 (xmin, ymin, xmax, ymax)
 * @param orig_size 原始图像尺寸 (宽, 高)
 * @param pad_info 填充信息结构体
 * @return cv::Rect2f 原始图像上的边界框 (浮点精度)
 */
cv::Rect2f OnnxDet::ResizePaddingMapRect(const cv::Rect& bbox, const cv::Size& orig_size, const PaddingInfo& pad_info)
{
    // 提取填充信息
    float scale = pad_info.scale;
    int left = pad_info.left;
    int top = pad_info.top;

    int orig_w = orig_size.width;
    int orig_h = orig_size.height;

    // 移除填充偏移并缩放回原始尺寸
    float xmin = (bbox.x - left) / scale;
    float ymin = (bbox.y - top) / scale;
    float xmax = (bbox.x + bbox.width - left) / scale;
    float ymax = (bbox.y + bbox.height - top) / scale;

    // 确保坐标在图像范围内
    xmin = CPPMAX(0.0f, CPPMIN(xmin, static_cast<float>(orig_w)));
    ymin = CPPMAX(0.0f, CPPMIN(ymin, static_cast<float>(orig_h)));
    xmax = CPPMAX(0.0f, CPPMIN(xmax, static_cast<float>(orig_w)));
    ymax = CPPMAX(0.0f, CPPMIN(ymax, static_cast<float>(orig_h)));

    // 返回边界框 (使用浮点精度保持准确性)
    return cv::Rect2f(xmin, ymin, xmax - xmin, ymax - ymin);
}

// 手动实现图像预处理
std::vector<float> OnnxDet::PreProcessImage(const cv::Mat& image)
{
    // 转换为float并归一化
    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255.0);

    // 转换为NCHW格式 (手动转换)
    std::vector<float> input_tensor(image.rows * image.cols * 3);
    float* dst = input_tensor.data();

    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < image.rows; ++y) {
            for (int x = 0; x < image.cols; ++x) {
                dst[c * image.cols * image.rows + y * image.cols + x] =
                    float_image.at<cv::Vec3f>(y, x)[c];
            }
        }
    }

    return input_tensor;
}

int OnnxDet::ProcessImage(cv::Mat& image, std::vector<OnnxDetRst>& rst)
{
    m_det_img = image;
    m_process_flag = 1;
    while(1)
    {
        if(m_process_flag == 0)
        {
            break;
        }
        CPPSleepMS(40);
    }

    rst = m_rst;
    
    return 0;
}

void OnnxDet::drawRect(cv::Mat& image, std::vector<OnnxDetRst>& rst)
{
    // 使用OpenCV绘制检测结果
    for (const auto& d : rst) {
        // 创建整数矩形
        cv::Rect bbox(
            static_cast<int>(d.bbox.x),
            static_cast<int>(d.bbox.y),
            static_cast<int>(d.bbox.width),
            static_cast<int>(d.bbox.height)
        );
    
        // 确保矩形在图像范围内
        //bbox = bbox & cv::Rect(0, 0, orig_width, orig_height);
        // 绘制边界框
        cv::rectangle(image, bbox, cv::Scalar(0, 255, 0), 2);
    
        // 创建标签文本
        std::ostringstream label;
        label << "Class " << d.class_id << ": " << std::fixed << std::setprecision(2) << d.confidence;
    
        // 计算文本位置
        int baseline = 0;
        cv::Size text_size = cv::getTextSize(label.str(), cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::Point text_org(bbox.x, bbox.y - 5);
    
        // 确保文本在图像顶部不会超出
        if (text_org.y < text_size.height + 5) {
            text_org.y = bbox.y + bbox.height + text_size.height + 5;
        }
    
        // 绘制标签背景
        cv::rectangle(image,
            cv::Point(bbox.x, text_org.y - text_size.height - 5),
            cv::Point(bbox.x + text_size.width, text_org.y + 5),
            cv::Scalar(0, 255, 0),
            cv::FILLED);
    
        // 绘制标签文本
        cv::putText(image, label.str(),
            text_org,
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(0, 0, 0), 1);
    }
    
    for (const auto& d : m_obj_rst)
    {
        cv::rectangle(image, d, cv::Scalar(0, 255, 0), 2);
    }
}

void OnnxDet::CreatTrackHandle(int w, int h)
{
    if(m_track.pvIVAFilter != NULL)
    {
        if(m_track.iva_mem_tab.base != NULL)
        {
            free(m_track.iva_mem_tab.base);
            m_track.iva_mem_tab.base = NULL;
        }
        m_track.pvIVAFilter = NULL;
    }
    
    IVA_CREATE_PARAM create_param = {0};
    create_param.image_w = w;
    create_param.image_h = h;
    create_param.bgs_downsample = 1;
    
    IVA_VIDEO_FRAME *frame   = &m_track.iva_frame;
    IVA_MEM_TAB     *mem_tab = &m_track.iva_mem_tab;
    
    // 获取iva需要的内存大小
    frame->video_header.image_width  = w;
    frame->video_header.image_height = h;
    frame->video_header.stride       = w;
    frame->yuv_frame.y               = NULL;
    frame->yuv_frame.u               = NULL;
    frame->yuv_frame.v               = NULL;
    frame->video_header.frame_rate   = 25;
    frame->video_header.format       = IVA_I420;
    
    int ret = QVIVA_GetMemSizeV2(&create_param, mem_tab);
    if(QV_IVA_LIB_S_OK != ret)
    {
        printf("QVIVA_GetMemSizeV2 is failed!(0x%x)\n", ret);
        return ;
    }
    
    printf("com iva filter mem size:%d\n", mem_tab->size);
    
    mem_tab->base = malloc(mem_tab->size);
    if(NULL == mem_tab->base)
    {
        printf("malloc err.\n");
        return ;
    }
    
    // 创建
    ret = QVIVA_CreateV2(&create_param, mem_tab, &m_track.pvIVAFilter);
    if(QV_IVA_LIB_S_OK != ret)
    {
        printf("QVIVA_CreateV2 is failed!(0x%x)\n", ret);
        free(mem_tab->base);
        return ;
    }
    
    //设置ivatrack参数
    IVA_LIB_PARAM iva_param;
    memset(&iva_param, 0, sizeof(IVA_LIB_PARAM));
    iva_param.param_key = IVA_SUPPRESS_STATIC;
    iva_param.param_val = 0;
    ret = QVIVA_SetConfig(m_track.pvIVAFilter,IVA_CFG_SINGLE_PARAM,(void*)&iva_param,sizeof(IVA_LIB_PARAM));
    if(QV_IVA_LIB_S_OK != ret)
    {
        printf("QVIVA_SetConfig is failed!(0x%x)\n", ret);
    }
    
    return ;
}

void OnnxDet::Track(cv::Mat& image, std::vector<OnnxDetRst>& stObj)
{
    if(m_track.pvIVAFilter == NULL)
    {
        return ;
    }
    
    IVA_BLOB   *blob = NULL;
    IVA_TARGET *target = NULL;
    
    //输入检测器检测结果
    int x1, x2, y1, y2;
    blob = m_track.blob_list.blob;
    memset(&m_track.blob_list, 0, sizeof(BLOB_LIST));
    for (const auto& d : stObj)
    {
        if(m_track.blob_list.blob_num >= MAX_BLOB_NUM)
        {
            break;
        }
        
        blob[m_track.blob_list.blob_num].conf = d.confidence;
    
        blob[m_track.blob_list.blob_num].cx = d.bbox.x + (d.bbox.width / 2);
        blob[m_track.blob_list.blob_num].cy = d.bbox.y + (d.bbox.height / 2);
        blob[m_track.blob_list.blob_num].rw = d.bbox.width / 2;
        blob[m_track.blob_list.blob_num].rh = d.bbox.height / 2;
        
        blob[m_track.blob_list.blob_num].type = d.class_id;
        m_track.blob_list.blob_num++;
    }

    unsigned char* data = image.ptr<uchar>(0);
    
    m_track.iva_frame.video_header.stride = image.cols;
    m_track.iva_frame.yuv_frame.y = data;
    m_track.iva_frame.yuv_frame.u = data + image.rows * image.cols;
    m_track.iva_frame.yuv_frame.v = data + image.rows * image.cols * 2;
    m_track.iva_frame.video_header.time_stamp = CCPPTime::GetClockTimeMS();
   
    //基于检测框的智能处理
    int ret = QVIVA_ProcessV2(m_track.pvIVAFilter, &m_track.iva_frame,
    (void*)(&m_track.blob_list),
    sizeof(BLOB_LIST), (void*)(&m_track.stIVAOut),sizeof(IVA_OUT_INFO));            
    if(ret != QV_IVA_LIB_S_OK)
    {
        printf("QVIVA_ProcessV2 err:%d\n", ret);
        return ;
    }
    
    std::vector<OnnxDetRst> outputObjs;
    
    target = m_track.stIVAOut.target_list.target;
    for(int idx = 0; idx < m_track.stIVAOut.target_list.target_num; idx++)
    {
        OnnxDetRst d;
        d.class_id = target->type;
        d.confidence = target->conf;
    
        cv::Rect2f detected_bbox(target->rect.x*image.cols, target->rect.y*image.rows, target->rect.width*image.cols, target->rect.height*image.rows);
    
        d.bbox = detected_bbox;
    
        outputObjs.push_back(d);
        
        target++;
    }

    stObj = outputObjs;

    return ;
}

int OnnxDet::ThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout)
{
    // 初始化ONNX Runtime环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNX_DETECTION");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // 加载ONNX模型
    const wchar_t* model_path = L"GestureReg.onnx";
    Ort::Session session(env, model_path, session_options);

    // 获取模型输入信息
    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name_ptr = session.GetInputNameAllocated(0, allocator);
    const char* input_names[] = { input_name_ptr.get() };

    // 获取输入形状
    auto input_shape_info = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
    std::vector<int64_t> input_shape = input_shape_info.GetShape();

    // 处理动态batch维度
    if (input_shape[0] == -1) {
        input_shape[0] = 1;  // 设置batch size为1
    }

    const int batch_size = input_shape[0];
    const int channels = input_shape[1];
    const int input_height = input_shape[2];
    const int input_width = input_shape[3];

    std::cout << "Model input shape: "
        << batch_size << "x" << channels << "x"
        << input_height << "x" << input_width << std::endl;

    // 获取输出信息
    auto output_name_ptr = session.GetOutputNameAllocated(0, allocator);
    const char* output_names[] = { output_name_ptr.get() };

    // 获取输出形状
    auto output_shape_info = session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo();
    std::vector<int64_t> output_shape = output_shape_info.GetShape();

    // 确认输出形状为 [1, 50, 6]
    if (output_shape.size() != 3 || output_shape[0] != 1 || output_shape[1] != 50 || output_shape[2] != 6) {
        std::cerr << "Error: Unexpected output shape: [";
        for (size_t i = 0; i < output_shape.size(); ++i) {
            std::cerr << output_shape[i];
            if (i < output_shape.size() - 1) std::cerr << ", ";
        }
        std::cerr << "]" << std::endl;
        std::cerr << "Expected shape: [1, 50, 6]" << std::endl;
        return -1;
    }

    while(1)
    {
        if(m_stop)
        {
            break;
        }
        if(m_process_flag == 1)
        {
            // 保存原始图像尺寸用于后处理
            cv::Size orig_size(m_det_img.cols, m_det_img.rows);
            
            cv::Mat padded_img;
            PaddingInfo pad_info = ResizePadding(m_det_img, padded_img, cv::Size(m_width, m_height));
            
            // 保存填充后的图像
//          cv::imwrite("padded.jpg", padded_img);

            // 颜色团块目标检测
//          detObjs(padded_img, orig_size, pad_info);
            
            // 手动预处理图像
            std::vector<float> input_tensor_values = PreProcessImage(padded_img);
            
            // 创建输入Tensor
            Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
                OrtArenaAllocator,
                OrtMemTypeDefault
            );
            
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info,
                input_tensor_values.data(),
                input_tensor_values.size(),
                input_shape.data(),
                input_shape.size()
            );

            uint64 startRunTime = CCPPTime::GetClockTimeMS();
            // 执行推理
            auto output_tensors = session.Run(
                Ort::RunOptions{ nullptr },
                input_names,
                &input_tensor,
                1,
                output_names,
                1
            );
            uint64 endRunTime = CCPPTime::GetClockTimeMS();
//          printf("Run timeuse : %lld\n", endRunTime - startRunTime);
            
            if (output_tensors.empty()) {
                std::cerr << "Error: No output tensors" << std::endl;
                return -1;
            }
            
            Ort::Value& output = output_tensors.front();
            float* output_data = output.GetTensorMutableData<float>();
            
            // 解析输出结果 (形状: [1, 50, 6])
            const int num_detections = output_shape[1];  // 50个检测框
//          std::cout << "Model output contains " << num_detections << " detections" << std::endl;
            
            std::vector<OnnxDetRst> detections;
            for (int i = 0; i < num_detections; ++i) {
                // 每6个值表示一个检测结果
                const float* det = output_data + i * 6;
            
                // 假设输出格式为: [x1, y1, x2, y2, confidence, class_id]
                // 注意：具体顺序可能需要根据模型调整
                float confidence = det[4];
            
                // 应用置信度阈值
                if (confidence < 0.6) continue;
            
                OnnxDetRst d;
                d.class_id = static_cast<int>(det[5]);
                d.confidence = confidence;
            
                cv::Rect2f detected_bbox(det[0], det[1], det[2] - det[0], det[3] - det[1]);
//              cv::Rect2f orig_bbox = ResizePaddingMapRect(detected_bbox, orig_size, pad_info);
            
                d.bbox = detected_bbox;
            
                detections.push_back(d);
            
                // 打印检测信息
                std::cout << "Detection " << i << ": "
                    << "class=" << d.class_id << ", "
                    << "conf=" << std::fixed << std::setprecision(2) << d.confidence << ", "
                    << "bbox=[" << d.bbox.x << ", " << d.bbox.y << ", "
                    << d.bbox.width << ", " << d.bbox.height << "]" << std::endl;
            }
            
//          std::cout << "Valid detections: " << detections.size() << std::endl;
            
            Track(padded_img, detections);
            
            std::vector<OnnxDetRst> outputObjs;
            for (const auto& d : detections)
            {
                OnnxDetRst obj = d;
                obj.bbox = ResizePaddingMapRect(d.bbox, orig_size, pad_info);
                outputObjs.push_back(obj);
            }

            m_rst = outputObjs;
            
            m_process_flag = 0;
        }
        else
        {
            CPPSleepMS(40);
        }
    }

    timeout = 10;
    
    return 0;
}

/*
  0: two_up_inverted
  1: call
  2: dislike
  3: fist
  4: four
  5: like
  6: mute
  7: ok
  8: one
  9: palm
  10: peace
  11: peace_inverted
  12: rock
  13: stop
  14: stop_inverted
  15: three
  16: three2
  17: two_up
*/