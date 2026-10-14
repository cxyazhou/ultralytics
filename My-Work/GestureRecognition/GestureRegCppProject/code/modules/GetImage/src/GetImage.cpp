#include "GetImageInterface.h"

#include "opencv2/opencv.hpp"

using namespace cv;

GetImage::GetImage()
{
    m_Mode = GET_IMG_MODE_ALL;
    m_pic_file_idx = 0;
	m_Capture = NULL;
    m_fps = 0;
    m_TotalFrames = 0;
	m_OrcImg = NULL;
}

int GetImage::Init(GET_IMG_MODE_E mode, const char* pPath)
{
    m_Mode = mode;
    
	Mat* orcImg = new Mat();
	m_OrcImg = (void*)orcImg;

    if (m_Mode == GET_IMG_MODE_RECORD_PIC)
    {
		VideoCapture* cap = new VideoCapture();
		if(!cap->open(pPath))
		{
            return -1;
		}
		m_fps = cap->get(CAP_PROP_FPS);
		m_TotalFrames = (int)(cap->get(CAP_PROP_FRAME_COUNT));
		m_Capture = (void*)cap;
        m_mediaName = pPath;
    }
	else if (m_Mode == GET_IMG_MODE_CAM)
	{
		VideoCapture* cap = new VideoCapture();
		if (pPath == NULL)
		{
            if(!cap->open(0))
            {
                return -1;
            }
		}
        else
		{
            if(!cap->open(pPath))
            {
                return -1;
            }
		}
		m_fps = cap->get(CAP_PROP_FPS);//每秒显示帧数
		m_TotalFrames = (int)(cap->get(CAP_PROP_FRAME_COUNT));
		m_Capture = (void*)cap;
        m_mediaName = pPath;
	}
    else if(m_Mode == GET_IMG_MODE_PIC_DIR)
    {
        cv::glob(pPath, m_pic_files, true);
        m_pic_file_idx = 0;
        m_fps = 1;
        m_TotalFrames = m_pic_files.size();
        if(!m_TotalFrames)
        {
            return -1;
        }
	}
    else if(m_Mode == GET_IMG_MODE_RECORD_DIR)
    {
        cv::glob(pPath, m_record_files, true);
        m_record_file_idx = 0;
        if(!m_record_files.size())
        {
            return -1;
        }
		VideoCapture* cap = new VideoCapture();
		m_Capture = (void*)cap;
	}

    return 0;
}

GetImage::~GetImage()
{
	VideoCapture* cap = (VideoCapture*)m_Capture;
    if(m_Mode != GET_IMG_MODE_PIC_DIR && cap != NULL)
    {
        cap->release();
		m_Capture = NULL;
    }
    
	Mat* orcImg = (Mat*)m_OrcImg;
	if (orcImg != NULL)
	{
		delete orcImg;
		m_OrcImg = NULL;
	}
}

int GetImage::Work(GET_IMG_IMG* pstImage)
{
    memset(pstImage, 0, sizeof(GET_IMG_IMG));
    
	Mat* orc = (Mat*)m_OrcImg;
    
    if((m_Mode == GET_IMG_MODE_CAM) || (m_Mode == GET_IMG_MODE_RECORD_PIC))
    {
        VideoCapture* cap = (VideoCapture*)m_Capture;
        (*cap) >> (*orc);
    }
    else if(m_Mode == GET_IMG_MODE_PIC_DIR)
    {
        if(m_pic_file_idx >= m_pic_files.size())
        {
            return -1;
        }
        m_mediaName = m_pic_files[m_pic_file_idx].c_str();

        (*orc) = imread(m_pic_files[m_pic_file_idx], IMREAD_COLOR);//读取图片
        m_pic_file_idx++;

        if(orc->empty())
        {
            return -1;
        }
    }
    else if(m_Mode == GET_IMG_MODE_RECORD_DIR)
    {
openfile:
        if(m_record_file_idx >= m_record_files.size())
        {
            return -1;
        }
        
        VideoCapture* cap = (VideoCapture*)m_Capture;
        if(!cap->isOpened())
        {
            if(!cap->open(m_record_files[m_record_file_idx]))
            {
                printf("%s open err.\n", m_record_files[m_record_file_idx].c_str());
                m_record_file_idx++;
                goto openfile;
            }
            
            (*cap) >> (*orc);
            if (orc->empty())
            {
                printf("%s is empty.\n", m_record_files[m_record_file_idx].c_str());
                m_record_file_idx++;
                goto openfile;
            }
            m_fps = cap->get(CAP_PROP_FPS);
            m_TotalFrames = (int)(cap->get(CAP_PROP_FRAME_COUNT));
            m_Capture = (void*)cap;
            m_mediaName = m_record_files[m_record_file_idx].c_str();
        }
        else
        {
            (*cap) >> (*orc);
            if (orc->empty())
            {
                m_record_file_idx++;
                if(m_record_file_idx >= m_record_files.size())
                {
                    return -1;
                }
                
                if(!cap->open(m_record_files[m_record_file_idx]))
                {
                    printf("%s open err.\n", m_record_files[m_record_file_idx].c_str());
                    m_record_file_idx++;
                    goto openfile;
                }
                
                (*cap) >> (*orc);
                if (orc->empty())
                {
                    printf("%s is empty.\n", m_record_files[m_record_file_idx].c_str());
                    m_record_file_idx++;
                    goto openfile;
                }
                m_fps = cap->get(CAP_PROP_FPS);
                m_TotalFrames = (int)(cap->get(CAP_PROP_FRAME_COUNT));
                m_Capture = (void*)cap;
                m_mediaName = m_record_files[m_record_file_idx].c_str();
            }
        }
    }

    pstImage->enType = GET_IMG_TYPE_BGR_PACKAGE;
	pstImage->vir[0] = (unsigned long long)orc->data;
	pstImage->stride[0] = orc->cols;
    pstImage->width = orc->cols;
    pstImage->height = orc->rows;

    return 0;
}

