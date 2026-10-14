#include "GetImageInterface.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui_c.h"

using namespace cv;

void GetImgTest()
{
	GetImage* m_pstGetImage = new GetImage();
    int ret = m_pstGetImage->Init(GET_IMG_MODE_RECORD_PIC, "F:\\work\\test.avi");
//  int ret = m_pstGetImage->Init(GET_IMG_MODE_CAM, "rtsp://admin:tdk123456@192.168.12.151/mode=real&idc=1&ids=1");
//  int ret = m_pstGetImage->Init(GET_IMG_MODE_PIC, "E:\\zipfeel\\project\\media\\图片\\*.jpg");
    if(ret != 0)
    {
        return;
    }

	const char* imageName = "IMG";

	//create window
	namedWindow(imageName, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(imageName, 100, 100);
	printf("fps================%d,totalframes===================%d\n", m_pstGetImage->GetFps(), m_pstGetImage->GetTotalFrames());
	int spf = 0;
	if (m_pstGetImage->GetFps() != 0)
		spf = (int)(1000 / m_pstGetImage->GetFps());//视频每一帧间隔

	int64 start = 0;
	int64 end = 0;
	while (1)
	{
		start = getTickCount();

        GET_IMG_IMG stImg;
        memset(&stImg, 0, sizeof(GET_IMG_IMG));

        if (0 != m_pstGetImage->Work(&stImg))
		{
			break;
		}

        Mat bgr_package_image_mat(stImg.height, stImg.width, CV_8UC3, (uchar*)stImg.vir[0]);
		//show image
		imshow(imageName, bgr_package_image_mat);

		end = getTickCount();

		int tick = (end - start) / 10000;
		if (tick >= spf)
		{
			char c = waitKey(1);
			if (c == 27)// 按下Esc退出播放    
			{
				break;
			}
		}
		else
		{
			int sleepTime = spf - tick;
			char c = waitKey(sleepTime);
			if (c == 27)// 按下Esc退出播放    
			{
				break;
			}
		}
	}
	return;
}

