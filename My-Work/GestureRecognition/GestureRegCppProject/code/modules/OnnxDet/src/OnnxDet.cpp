#include "OnnxDet.h"

#include "onnxruntime_cxx_api.h"

using namespace cv;
using namespace std;

extern char g_exe_path[];

uint64_t GetClockTimeMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

OnnxDet::OnnxDet()
{
    m_stop = false;
    m_has_task = false;
    m_done = false;
    m_width = 320;
    m_height = 320;
    m_obj_det_cnt = 0;
}

OnnxDet::~OnnxDet()
{
    UnInit();
}

int OnnxDet::Init()
{
    CreatTrackHandle(m_width, m_height);
    
    // 复位状态
    m_stop = false;
    m_has_task = false;
    m_done = false;

    // 启动线程（无参版本，干净）
    m_thread = std::thread(&OnnxDet::ThreadProc, this);

    return 0;
}

int OnnxDet::UnInit()
{
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_stop = true;
    }
    m_cv.notify_all();                 // 唤醒可能正在等待的线程

    if (m_thread.joinable()) {
        m_thread.join();               // 等线程真正结束
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
    float scale = MIN(static_cast<float>(target_w) / orig_w,
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
    xmin = MAX(0.0f, MIN(xmin, static_cast<float>(orig_w)));
    ymin = MAX(0.0f, MIN(ymin, static_cast<float>(orig_h)));
    xmax = MAX(0.0f, MIN(xmax, static_cast<float>(orig_w)));
    ymax = MAX(0.0f, MIN(ymax, static_cast<float>(orig_h)));

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
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_det_img = image.clone();
        m_has_task = true;
        m_done = false;
    }
    m_cv.notify_one();                 // 唤醒工作线程

    {
        std::unique_lock<std::mutex> lk(m_mtx);
        // 等结果就绪（或线程被停止）
        m_cv.wait(lk, [this] { return m_done.load() || m_stop.load(); });
        rst = m_rst;
    }

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
    return ;
}

void OnnxDet::Track(cv::Mat& image, std::vector<OnnxDetRst>& stObj)
{
    return ;
}

int OnnxDet::ThreadProc()
{
    // 初始化ONNX Runtime环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNX_DETECTION");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // 加载ONNX模型
    wchar_t  modelPath[256] = { 0 };
    _snwprintf_s(modelPath, 256, _TRUNCATE, L"%hs/model/GestureReg.onnx", g_exe_path);
    const wchar_t* model_path = modelPath;

    Ort::Session session{ nullptr };
    try {
        session = Ort::Session(env, model_path, session_options);
    }
    catch (const Ort::Exception& e) {
        std::cerr << "Load model failed: " << e.what() << std::endl;
        return -1;
    }

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

    const int batch_size = static_cast<int>(input_shape[0]);
    const int channels = static_cast<int>(input_shape[1]);
    const int input_height = static_cast<int>(input_shape[2]);
    const int input_width = static_cast<int>(input_shape[3]);

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

    Ort::MemoryInfo memory_info =
        Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    // ---------- 2. 主循环 ----------
    while (true)
    {
        cv::Mat img;
        {
            std::unique_lock<std::mutex> lk(m_mtx);

            // 等待：有任务 或 要停止；最多等 40ms 超时
            m_cv.wait_for(lk, std::chrono::milliseconds(40),
                [this] { return m_has_task.load() || m_stop.load(); });

            if (m_stop.load()) break;      // 退出

            if (!m_has_task.load()) {
                continue;                  // 超时且无任务
            }

            img = m_det_img.clone();       // 取出任务数据
            m_has_task = false;
            m_done = false;
        }

        // ---------- 3. 锁外做耗时推理 ----------
        cv::Size orig_size(img.cols, img.rows);

        cv::Mat padded_img;
        PaddingInfo pad_info = ResizePadding(img, padded_img, cv::Size(m_width, m_height));

        // 保存填充后的图像
//          cv::imwrite("padded.jpg", padded_img);

            // 颜色团块目标检测
//          detObjs(padded_img, orig_size, pad_info);

            // 手动预处理图像
        std::vector<float> input_tensor_values = PreProcessImage(padded_img);

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_tensor_values.data(),
            input_tensor_values.size(),
            input_shape.data(),
            input_shape.size()
        );

        auto t0 = std::chrono::steady_clock::now();

        std::vector<Ort::Value> output_tensors;
        try {
            output_tensors = session.Run(Ort::RunOptions{ nullptr },
                input_names, &input_tensor, 1,
                output_names, 1);
        }
        catch (const Ort::Exception& e) {
            std::cerr << "Inference failed: " << e.what() << std::endl;
            continue;
        }

        auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        // std::cout << "Run timeuse : " << cost_ms << " ms\n";

        if (output_tensors.empty()) {
            std::cerr << "Error: No output tensors" << std::endl;
            continue;
        }

        float* output_data = output_tensors.front().GetTensorMutableData<float>();

        // 解析输出结果 (形状: [1, 50, 6])
        const int num_detections = static_cast<int>(output_shape[1]);  // 50个检测框
        //          std::cout << "Model output contains " << num_detections << " detections" << std::endl;

        std::vector<OnnxDetRst> detections;
        for (int i = 0; i < num_detections; ++i) {
            // 每6个值表示一个检测结果
            const float* det = output_data + i * 6;

            // 假设输出格式为: [x1, y1, x2, y2, confidence, class_id]
            // 注意：具体顺序可能需要根据模型调整
            float confidence = det[4];

            // 应用置信度阈值
            if (confidence < 0.6f) continue;

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
        outputObjs.reserve(detections.size());
        for (const auto& d : detections)
        {
            OnnxDetRst obj = d;
            obj.bbox = ResizePaddingMapRect(d.bbox, orig_size, pad_info);
            outputObjs.push_back(obj);
        }

        // ---------- 4. 写结果并通知 ----------
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_rst = std::move(outputObjs);
            m_done = true;
        }
        m_cv.notify_all();
    }

    return 0;
}
