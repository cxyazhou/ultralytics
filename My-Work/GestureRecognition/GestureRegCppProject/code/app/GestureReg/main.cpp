#if defined(WIN32) | defined(WIN64)
//#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "Comdlg32.lib")

#pragma comment(lib, "GuiMain.lib")

#pragma comment(lib, "IvaTrack.lib")
#pragma comment(lib, "OnnxDet.lib")
#pragma comment(lib, "MediaPlayer.lib")
#pragma comment(lib, "GetAudio.lib")
#pragma comment(lib, "GetImage.lib")
#pragma comment(lib, "AudioProcess.lib")
#pragma comment(lib, "ConfigManager.lib")
#pragma comment(lib, "json.lib")
#pragma comment(lib, "CPPBase.lib")

#pragma comment(lib, "onnxruntime.lib")

#pragma comment(lib, "libvlc.lib")
#pragma comment(lib, "libvlccore.lib")

#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")

#pragma comment(lib, "portaudio.lib")

#pragma comment(lib, "zlib.lib")

#ifndef _DEBUG
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5Widgets.lib")

#pragma comment(lib, "opencv_calib3d450.lib")
#pragma comment(lib, "opencv_core450.lib")
#pragma comment(lib, "opencv_dnn450.lib")
#pragma comment(lib, "opencv_features2d450.lib")
#pragma comment(lib, "opencv_flann450.lib")
#pragma comment(lib, "opencv_gapi450.lib")
#pragma comment(lib, "opencv_highgui450.lib")
#pragma comment(lib, "opencv_imgcodecs450.lib")
#pragma comment(lib, "opencv_imgproc450.lib")
#pragma comment(lib, "opencv_ml450.lib")
#pragma comment(lib, "opencv_objdetect450.lib")
#pragma comment(lib, "opencv_photo450.lib")
#pragma comment(lib, "opencv_stitching450.lib")
#pragma comment(lib, "opencv_ts450.lib")
#pragma comment(lib, "opencv_video450.lib")
#pragma comment(lib, "opencv_videoio450.lib")
#pragma comment(lib, "ade.lib")
#pragma comment(lib, "IlmImf.lib")
#pragma comment(lib, "ippicvmt.lib")
#pragma comment(lib, "ippiw.lib")
#pragma comment(lib, "ittnotify.lib")
#pragma comment(lib, "libjpeg-turbo.lib")
#pragma comment(lib, "libopenjp2.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libtiff.lib")
#pragma comment(lib, "libwebp.lib")

#pragma comment(lib, "libavcodec.lib")
#pragma comment(lib, "libavdevice.lib")
#pragma comment(lib, "libavfilter.lib")
#pragma comment(lib, "libavformat.lib")
#pragma comment(lib, "libavutil.lib")
#pragma comment(lib, "libswresample.lib")
#pragma comment(lib, "libswscale.lib")
#else
#pragma comment(lib, "Qt5Cored.lib")
#pragma comment(lib, "Qt5Guid.lib")
#pragma comment(lib, "Qt5Widgetsd.lib")

#pragma comment(lib, "opencv_calib3d450d.lib")
#pragma comment(lib, "opencv_core450d.lib")
#pragma comment(lib, "opencv_dnn450d.lib")
#pragma comment(lib, "opencv_features2d450d.lib")
#pragma comment(lib, "opencv_flann450d.lib")
#pragma comment(lib, "opencv_gapi450d.lib")
#pragma comment(lib, "opencv_highgui450d.lib")
#pragma comment(lib, "opencv_imgcodecs450d.lib")
#pragma comment(lib, "opencv_imgproc450d.lib")
#pragma comment(lib, "opencv_ml450d.lib")
#pragma comment(lib, "opencv_objdetect450d.lib")
#pragma comment(lib, "opencv_photo450d.lib")
#pragma comment(lib, "opencv_stitching450d.lib")
#pragma comment(lib, "opencv_ts450d.lib")
#pragma comment(lib, "opencv_video450d.lib")
#pragma comment(lib, "opencv_videoio450d.lib")
#pragma comment(lib, "ade.lib")
#pragma comment(lib, "IlmImfd.lib")
#pragma comment(lib, "ippicvmt.lib")
#pragma comment(lib, "ippiwd.lib")
#pragma comment(lib, "ittnotifyd.lib")
#pragma comment(lib, "libjpeg-turbod.lib")
#pragma comment(lib, "libopenjp2d.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libtiffd.lib")
#pragma comment(lib, "libwebpd.lib")

#pragma comment(lib, "libavcodecd.lib")
#pragma comment(lib, "libavdeviced.lib")
#pragma comment(lib, "libavfilterd.lib")
#pragma comment(lib, "libavformatd.lib")
#pragma comment(lib, "libavutild.lib")
#pragma comment(lib, "libswresampled.lib")
#pragma comment(lib, "libswscaled.lib")
#endif

#endif //defined(WIN32) | defined(WIN64)

#include "ConfigManager.h"
#include "GuiMain.h"

int main(int argc, char* argv[])
{
#if defined(WIN32) | defined(WIN64)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    char path[1024] = { 0 };
    char processname[1024] = { 0 };
    ICPPFile::GetExecutablePath(path, processname, sizeof(path));
    printf("path:%s name:%s\n", path, processname);
    CCPPString configDir(path);
    configDir += "Config";
    CConfigManager::instance()->initialize(APP_APP, configDir.GetBuffer());
        
	QApplication app(argc, argv);
        
	GuiMain* v = GuiMain::instance();
	if (NULL != v)
	{
		v->show();
	}
        
	app.exec();
    
    return 0;
}

