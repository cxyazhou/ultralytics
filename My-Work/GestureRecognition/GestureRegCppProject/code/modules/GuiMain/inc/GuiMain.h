#pragma once

#include <QtCore/qthread.h>
#include <QtCore/qdebug.h>
#include <QtGui/QImage>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>
#include <vector>

#include "ui_GuiMain.h"

// ---------- 图像源抽象接口（替代 GetImageInterface） ----------
struct RawImage {
    int width = 0;
    int height = 0;
    unsigned char* data = nullptr;   // BGR
};

class IImageSource {
public:
    virtual ~IImageSource() = default;
    virtual int  Init(const std::string& url) = 0;
    virtual int  GetFrame(RawImage& out) = 0;   // 0=成功
    virtual void Close() = 0;
};

// ---------- 音频/动作播放占位（替代 PlayMedia） ----------
void PlayMedia(int class_id);

class OnnxDet;

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    static GuiMain* instance();

    GuiMain(QWidget* parent = nullptr);
    ~GuiMain();

    void StartThreads();
    void StopThreads();

protected:
    void closeEvent(QCloseEvent* event) override;

signals:
    void sigLabelShowImg(QImage*);

private slots:
    void on_m_register_lav_clicked();
    void on_m_start_det_clicked();
    void LabelShowImg(QImage* img);

private:
    // 线程主函数（替代 ThreadProc / CapThreadProc）
    void DetThreadProc();
    void CapThreadProc();

    void StartCapThread();
    void StopCapThread();

private:
    Ui::GuiMainClass ui;

    // ---- 采集线程 ----
    std::thread             m_capThread;
    std::atomic<bool>       m_capRunning{ false };

    // ---- 检测线程 ----
    std::thread             m_detThread;
    std::atomic<bool>       m_detRunning{ false };

    // ---- 采集图像 → 检测 之间的同步 ----
    std::mutex              m_frameMtx;
    std::condition_variable m_frameCv;
    bool                    m_hasFrame = false;      // 有未处理的新帧

    // ---- 采集与显示缓冲 ----
    std::unique_ptr<IImageSource> m_imgSource;

    unsigned char* m_imgShowDataBGR = nullptr;
    unsigned char* m_imgShowDataRGB = nullptr;

    int                     m_imgCapWidth = 352;
    int                     m_imgCapHeight = 288;
    unsigned char* m_imgCapDataBGR = nullptr;

    // ---- 检测器 ----
    OnnxDet* m_handDet = nullptr;
};