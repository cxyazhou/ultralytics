#pragma once

#include <QtCore/qthread.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/QVariant>

#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/qfiledialog.h>

#include "ui_GuiMain.h"
#include "CPPBase.h"

typedef enum enThreadState
{
	THREAD_STATE_E_Start = 0,
	THREAD_STATE_E_PrepareStop,
	THREAD_STATE_E_Stop
}THREAD_STATE_E;

class GuiMain;
class GuiMainThread :public QThread
{
	Q_OBJECT
public:
	GuiMainThread(QObject* parent, GuiMain* gui);
	~GuiMainThread();

	void run();//线程入口函数（工作线程的主函数）

private:
	GuiMain* m_gui;
};

class OnnxDet;
class GuiMain : public QMainWindow, public CCPPObject
{
	Q_OBJECT

public:
    static GuiMain* instance();
    
	GuiMain(QWidget* parent = Q_NULLPTR);
	~GuiMain();

	THREAD_STATE_E	GetThreadState() { return m_thread_state; };
	int				SetThreadState(THREAD_STATE_E state) { return m_thread_state = state; };
	void			StartThread();
	void			StopThread();

    void            PlayMedia(int class_id);

protected:
	void closeEvent(QCloseEvent* event);                //关闭按钮触发

private:
	int ThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout);
	int CapThreadProc(CCPPMessageT* msg, uint timestamp, uint& timeout);

signals:
	void sigLabelShowImg(QImage*);

private slots:
	void on_m_register_lav_clicked();
	void on_m_start_det_clicked();
	void LabelShowImg(QImage* img);

private:
	Ui::GuiMainClass        ui;
    
	THREAD_STATE_E			m_thread_state;
	GuiMainThread*	        m_thread;

	CCPPThreadWorker*       m_thread_cap;
	CCPPThreadWorker*       m_thread_worker;
	void*                   m_getImg;
	unsigned char*          m_imgShowDataBGR;
	unsigned char*          m_imgShowDataRGB;

    int                     m_imgFlag;
    int                     m_imgCapWidth;
    int                     m_imgCapHeight;
	unsigned char*          m_imgCapDataBGR;

    OnnxDet*                m_handDet;
};

