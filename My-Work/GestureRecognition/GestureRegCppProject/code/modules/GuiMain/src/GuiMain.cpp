#include "GuiMain.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include <cstring>
#include <fstream>
#include <chrono>

#include "GetImageInterface.h"
#include "OnnxDet.h"

using namespace cv;

#pragma warning(disable:4996)

std::unique_ptr<IImageSource> CreateImageSource()
{
    // 用你原来的 GetImage 库实现 IImageSource 接口
    class GetImageSource : public IImageSource {
        GetImage* impl = nullptr;        
        GET_IMG_IMG stImg = {0};
    public:
        int Init(const std::string& url) override {
            impl = new GetImage();
            return impl->Init(GET_IMG_MODE_CAM, url.c_str());
        }
        int GetFrame(RawImage& out) override {
            memset(&stImg, 0, sizeof(stImg));
            int ret = impl->Work(&stImg);
            if (ret != 0) return ret;
            out.width = stImg.width;
            out.height = stImg.height;
            out.data = (unsigned char*)stImg.vir[0];
            return 0;
        }
        void Close() override {
            if (impl) { delete impl; impl = nullptr; }
        }
        ~GetImageSource() override { Close(); }
    };

    return std::unique_ptr<IImageSource>(new GetImageSource());
}

// ============================================================
//  单例
// ============================================================
GuiMain* GuiMain::instance()
{
    static GuiMain* _ins = nullptr;
    if (_ins == nullptr)
        _ins = new GuiMain();
    return _ins;
}

// ============================================================
//  构造
// ============================================================
GuiMain::GuiMain(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setAcceptDrops(true);

    // 窗口居中
    QDesktopWidget* desktop = QApplication::desktop();
    int currentScreenIndex = desktop->screenNumber(this);
    QList<QScreen*> screen_list = QGuiApplication::screens();
    if (currentScreenIndex < screen_list.count()) {
        QRect screen_rect = screen_list[currentScreenIndex]->geometry();
        this->move((screen_rect.width() - this->width()) / 2,
            (screen_rect.height() - this->height()) / 2);
    }

    connect(this, SIGNAL(sigLabelShowImg(QImage*)),
        this, SLOT(LabelShowImg(QImage*)),
        Qt::BlockingQueuedConnection);

    // 缓冲
    m_imgShowDataBGR = (unsigned char*)malloc(1920 * 1080 * 3);
    m_imgShowDataRGB = (unsigned char*)malloc(1920 * 1080 * 3);
    m_imgCapDataBGR = (unsigned char*)malloc(m_imgCapWidth * m_imgCapHeight * 3);

    // 检测器
    m_handDet = new OnnxDet;
    if (m_handDet->Init() != 0) {
        delete m_handDet;
        m_handDet = nullptr;
    }

    StartThreads();
}

GuiMain::~GuiMain()
{
    StopThreads();

    if (m_handDet) {
        m_handDet->UnInit();
        delete m_handDet;
        m_handDet = nullptr;
    }

    if (m_imgSource) {
        m_imgSource->Close();
        m_imgSource.reset();
    }

    free(m_imgShowDataBGR);
    free(m_imgShowDataRGB);
    free(m_imgCapDataBGR);
}

// ============================================================
//  线程启动 / 停止
// ============================================================
void GuiMain::StartThreads()
{
    m_capRunning = true;
    m_capThread = std::thread(&GuiMain::CapThreadProc, this);

    m_detRunning = true;
    m_detThread = std::thread(&GuiMain::DetThreadProc, this);
}

void GuiMain::StopThreads()
{
    // 停采集
    m_capRunning = false;
    if (m_capThread.joinable()) m_capThread.join();

    // 停检测
    m_detRunning = false;
    m_frameCv.notify_all();
    if (m_detThread.joinable()) m_detThread.join();
}

void GuiMain::StartCapThread()
{
    // 采集线程已在 StartThreads 启动，这里只负责初始化图像源
    // 若需动态重启采集线程，可在这里 join 后重建
    if (m_capThread.joinable()) return;
    m_capRunning = true;
    m_capThread = std::thread(&GuiMain::CapThreadProc, this);
}

void GuiMain::StopCapThread()
{
    m_capRunning = false;
    if (m_capThread.joinable()) m_capThread.join();
}

// ============================================================
//  采集线程
// ============================================================
void GuiMain::CapThreadProc()
{
    while (m_capRunning)
    {
        auto t0 = std::chrono::steady_clock::now();

        if (!m_imgSource) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        RawImage frame;
        if (m_imgSource->GetFrame(frame) != 0 || !frame.data) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // 缩放为检测输入尺寸
        Mat orc(frame.height, frame.width, CV_8UC3, frame.data);
        Mat dst(m_imgCapHeight, m_imgCapWidth, CV_8UC3, m_imgCapDataBGR);
        cv::resize(orc, dst, Size(m_imgCapWidth, m_imgCapHeight), 0, 0, INTER_AREA);

        // 通知检测线程：有新帧
        {
            std::lock_guard<std::mutex> lk(m_frameMtx);
            m_hasFrame = true;
        }
        m_frameCv.notify_one();

        // 控制采集频率（约 10ms，即 ~100fps 上限）
        auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        if (cost < 10)
            std::this_thread::sleep_for(std::chrono::milliseconds(10 - cost));
    }
}

// ============================================================
//  检测线程
// ============================================================
void GuiMain::DetThreadProc()
{
    while (m_detRunning)
    {
        // ---------- 等待新帧 ----------
        {
            std::unique_lock<std::mutex> lk(m_frameMtx);
            m_frameCv.wait_for(lk, std::chrono::milliseconds(100),
                [this] { return m_hasFrame || !m_detRunning; });
            if (!m_detRunning) break;
            if (!m_hasFrame) continue;   // 超时
            m_hasFrame = false;
        }

        // ---------- 取一帧 ----------
        Mat orc(m_imgCapHeight, m_imgCapWidth, CV_8UC3, m_imgCapDataBGR);
        Mat frame = orc.clone();   // 复制，避免采集线程改写

        // ---------- 推理 ----------
        if (m_handDet)
        {
            auto t0 = std::chrono::steady_clock::now();

            std::vector<OnnxDetRst> rst;
            m_handDet->ProcessImage(frame, rst);
            m_handDet->drawRect(frame, rst);

            auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
            // printf("det timeuse: %lld ms\n", (long long)cost);
        }

        // ---------- 显示 ----------
        int labelw = ui.m_img_label->width();
        int labelh = ui.m_img_label->height();
        if (labelw <= 0 || labelh <= 0) continue;

        Mat imgShowDataBGR(labelh, labelw, CV_8UC3, m_imgShowDataBGR);
        cv::resize(frame, imgShowDataBGR, Size(labelw, labelh), 0, 0, INTER_AREA);

        Mat imgShowDataRGB(labelh, labelw, CV_8UC3, m_imgShowDataRGB);
        cvtColor(imgShowDataBGR, imgShowDataRGB, COLOR_BGR2RGB);

        QImage showimage((uchar*)imgShowDataRGB.data,
            imgShowDataRGB.cols,
            imgShowDataRGB.rows,
            imgShowDataRGB.cols * imgShowDataRGB.channels(),
            QImage::Format_RGB888);

        emit sigLabelShowImg(&showimage);
    }
}

// ============================================================
//  UI 槽函数
// ============================================================
void GuiMain::on_m_register_lav_clicked()
{
    // 保留原逻辑，如果用不到可以删
    system("LAV/register.bat");
}

void GuiMain::on_m_start_det_clicked()
{
    std::string user = ui.m_user->text().toStdString();
    std::string passwd = ui.m_passwd->text().toStdString();
    std::string ip = ui.m_ip->text().toStdString();

    std::string url = "rtsp://" + user + ":" + passwd + "@" + ip
        + ":554/mode=real&idc=1&ids=2";

    // 关闭旧的图像源
    if (m_imgSource) {
        m_imgSource->Close();
        m_imgSource.reset();
    }

    // 创建新的图像源（这里用你的实现替换）
    m_imgSource = CreateImageSource();   // ← 工厂函数，见下方说明
    if (!m_imgSource) {
        QMessageBox::warning(this, "Error", "Create image source failed");
        return;
    }

    printf("url:%s\n", url.c_str());
    if (m_imgSource->Init(url) != 0) {
        QMessageBox::warning(this, "Error", "Init image source failed");
        m_imgSource->Close();
        m_imgSource.reset();
        return;
    }

    // 若采集线程已停止，重新启动
    StartCapThread();
}

void GuiMain::LabelShowImg(QImage* img)
{
    ui.m_img_label->setPixmap(QPixmap::fromImage(*img));
}

void GuiMain::closeEvent(QCloseEvent* event)
{
    StopThreads();

    if (m_imgSource) {
        m_imgSource->Close();
        m_imgSource.reset();
    }

    event->accept();
}
