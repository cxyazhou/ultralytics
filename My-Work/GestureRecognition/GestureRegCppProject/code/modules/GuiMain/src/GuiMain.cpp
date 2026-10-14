#include <string.h>
#include <atlconv.h>
#include <fstream>

#include "GuiMain.h"
#include "CPPBase.h"

#include "GetAudioInterface.h"
#include "MediaPlayer.h"
#include "GetImageInterface.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui_c.h"

#include "OnnxDet.h"

using namespace cv;

#pragma warning(disable:4996)

GuiMainThread::GuiMainThread(QObject* parent, GuiMain* gui)
	:QThread(parent)
{
	m_gui = NULL;

	if (gui == NULL)
		return;

	m_gui = gui;

	start();
}

GuiMainThread::~GuiMainThread()
{
}

void GuiMainThread::run()
{
    int serverState = 0;
	while (1)
	{
		//程序退出时终止线程
		if (m_gui->GetThreadState() == THREAD_STATE_E_PrepareStop)
		{
			m_gui->SetThreadState(THREAD_STATE_E_Stop);
			return;
		}

		QThread::msleep(1000);
	}
}

GuiMain* GuiMain::instance()
{
	static GuiMain* _ins = NULL;
	if ( _ins == NULL )
	{
		_ins = new GuiMain();
	}

	return _ins;
}

GuiMain::GuiMain(QWidget* parent)
	: QMainWindow(parent)
{
    char path[1024] = {0};
    char processname[1024] = {0};
    ICPPFile::GetExecutablePath(path, processname, sizeof(path));
    QString rcc = path;
    rcc += "GuiMain.rcc";
    QResource::registerResource(rcc);

    ui.setupUi(this);
    
    //启用控件的放下操作
    setAcceptDrops(true);

    //去掉标题栏
//  this->setWindowFlags(Qt::FramelessWindowHint);

    //窗口移到中间
    QDesktopWidget *desktop = QApplication::desktop();
    int currentScreenIndex = desktop->screenNumber(this);
    QList<QScreen *> screen_list = QGuiApplication::screens();
    if(currentScreenIndex < screen_list.count()) {
        QScreen *tem = screen_list[currentScreenIndex];
        QRect screen_rect  = tem->geometry();

        //获取到软件窗口所在屏幕的 宽 高 尺寸
        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();//减去 任务栏高度

        //移动窗口到 居中
        this->move((screen_width - this->width()) / 2,  (screen_height - this->height()) / 2);
    }
    
    connect(this, SIGNAL(sigLabelShowImg(QImage*)), this, SLOT(LabelShowImg(QImage*)), Qt::BlockingQueuedConnection);

	m_thread_cap = NULL;
	m_thread_worker = NULL;
	m_getImg = NULL;
    m_imgShowDataBGR = (unsigned char*)CPPMalloc0(1920 * 1080 * 3 * sizeof(char));
    m_imgShowDataRGB = (unsigned char*)CPPMalloc0(1920 * 1080 * 3 * sizeof(char));

    m_imgFlag = 0;
    m_imgCapWidth = 352;
    m_imgCapHeight = 288;
    m_imgCapDataBGR = (unsigned char*)CPPMalloc0(m_imgCapWidth * m_imgCapHeight * 3 * sizeof(char));

    m_handDet = NULL;
    m_handDet = new OnnxDet;
	int ret = m_handDet->Init();
    if(ret != 0)
    {
        CPPDel(m_handDet);
    }

	StartThread();
    
    MediaPlayer::instance()->start();
}

GuiMain::~GuiMain()
{    
    CPPFree(m_imgShowDataBGR);
    CPPFree(m_imgShowDataRGB);
    CPPFree(m_imgCapDataBGR);
    StopThread();
    if(m_handDet)
    {
        m_handDet->UnInit();
    }
    CPPDel(m_handDet);
}

void GuiMain::StartThread()
{
	m_thread_state = THREAD_STATE_E_Start;
	m_thread = new GuiMainThread(NULL, this);
}

void GuiMain::StopThread()
{
	if (m_thread)
	{
		//设置准备停止状态
		m_thread_state = THREAD_STATE_E_PrepareStop;
		//一直等到停止
		while (1)
		{
			//printf("wait show img thread stop\n");
			if (m_thread_state == THREAD_STATE_E_Stop)
				break;
			QThread::msleep(100);
		}
		QThread::msleep(50);
		delete m_thread;
		m_thread = NULL;
	}
}

void GuiMain::on_m_register_lav_clicked()
{
	char path[1024] = { 0 };
	char processname[1024] = { 0 };
	ICPPFile::GetExecutablePath(path, processname, sizeof(path));

	CCPPString LAV = "\"";
	LAV += path;
	LAV += "LAV/register.bat";
	LAV += "\"";
	system(LAV.GetBuffer());

    return;
}

void GuiMain::on_m_start_det_clicked()
{
	void* cast = this;
    
//	if (m_thread_worker != NULL)
//	{
//		m_thread_worker->Detach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::ThreadProc);
//	}

	if (m_thread_worker == NULL)
	{
		m_thread_worker = new CCPPThreadWorker(2, "smart");
        m_thread_worker->Attach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::ThreadProc);
	}
    
	if (m_thread_cap != NULL)
	{
		m_thread_cap->Detach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::CapThreadProc);
	}

	if (m_thread_cap == NULL)
	{
		m_thread_cap = new CCPPThreadWorker(2, "smart");
	}

	CCPPString user = ui.m_user->text().toStdString().c_str();
	CCPPString passwd = ui.m_passwd->text().toStdString().c_str();
	CCPPString ip = ui.m_ip->text().toStdString().c_str();

	CCPPString url = "rtsp://";
	url += user;
	url += ":";
	url += passwd;
	url += "@";
	url += ip;
	url += ":554/mode=real&idc=1&ids=2";

	GetImage* img = (GetImage*)m_getImg;
	if (img != NULL)
	{
		delete img;
		img = NULL;
		m_getImg = NULL;
	}

	GetImage* getImage = new GetImage();
	int ret = getImage->Init(GET_IMG_MODE_CAM, url.GetBuffer());
	if (ret != 0)
	{
		delete getImage;
		getImage = NULL;
		return;
	}
	m_getImg = (void*)getImage;

	m_thread_cap->Attach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::CapThreadProc);
	
	return;
}

void GuiMain::LabelShowImg(QImage* img)
{
    ui.m_img_label->setPixmap(QPixmap::fromImage(*img));
}

void GuiMain::closeEvent(QCloseEvent* event)
{
	if (1)
	{
        void* cast = this;
		if (m_thread_worker != NULL)
		{
            //会卡住
//			m_thread_worker->Detach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::ThreadProc);
//          CPPDel(m_thread_worker);
		}
        
		if (m_thread_cap != NULL)
		{
			m_thread_cap->Detach((CCPPObject*)cast, (CPPTHREADAPI)&GuiMain::CapThreadProc);
            CPPDel(m_thread_cap);
		}

		event->accept();
	}
	else
	{
		event->ignore();
	}
}

int GuiMain::CapThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout)
{
    GetImage* img = (GetImage*)m_getImg;
    if (img == NULL)
    {
        timeout = 1000;
        return 0;
    }

	GET_IMG_IMG stImg;
	memset(&stImg, 0, sizeof(GET_IMG_IMG));

	int ret = img->Work(&stImg);
    if (ret != 0)
    {
        printf("img work err\n");
        timeout = 1000;
        return 0;
    }

    if(m_imgFlag)
    {
        timeout = 10;
        return 0;
    }

    Mat orc(stImg.height, stImg.width, CV_8UC3, (char*)stImg.vir[0]);
    Mat dst(m_imgCapHeight, m_imgCapWidth, CV_8UC3, (char*)m_imgCapDataBGR);
    cv::resize(orc, dst, Size(m_imgCapWidth, m_imgCapHeight), (0, 0), (0, 0), INTER_AREA);

    m_imgFlag = 1;

    timeout = 10;
    return 0;
}

int GuiMain::ThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout)
{
    if(m_imgFlag == 0)
    {
        timeout = 10;
        return 0;
    }
    
    //orc bbbbb ggggg rrrrr
    Mat orc(m_imgCapHeight, m_imgCapWidth, CV_8UC3, (char*)m_imgCapDataBGR);

    if(m_handDet)
    {
        uint64 detS = CCPPTime::GetClockTimeMS();
        std::vector<OnnxDetRst> rst;
        m_handDet->ProcessImage(orc, rst);
        m_handDet->drawRect(orc, rst);
        uint64 detE = CCPPTime::GetClockTimeMS();
//      printf("det timeuse:%lld\n", detE -detS);

        for (const auto& d : rst)
        {
            PlayMedia(d.class_id);
            break;
        }
    }
    
    int labelw = ui.m_img_label->width();
    int labelh = ui.m_img_label->height();
    
    //缩放为界面大小的bgr
    Mat imgShowDataBGR(labelh, labelw, CV_8UC3, m_imgShowDataBGR);
    cv::resize(orc, imgShowDataBGR, Size(labelw, labelh), (0, 0), (0, 0), INTER_AREA);

    Mat imgShowDataRGB(labelh, labelw, CV_8UC3, m_imgShowDataRGB);
    cvtColor(imgShowDataBGR, imgShowDataRGB, CV_BGR2RGB);

    QImage showimage = QImage((uchar*)imgShowDataRGB.data, imgShowDataRGB.cols, imgShowDataRGB.rows, imgShowDataRGB.cols * imgShowDataRGB.channels(), QImage::Format_RGB888);

    emit sigLabelShowImg(&showimage);

    m_imgFlag = 0;

    timeout = 10;
    return 0;
}

void GuiMain::PlayMedia(int class_id)
{
    CCPPString fileName;
    fileName.Append("music/%d.mp3", class_id);
    fileName.Dump();
    MediaPlayer::instance()->playOneFileIfEndMutex(fileName.GetBuffer());

    return ;
}
