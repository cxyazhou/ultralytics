#ifndef __GET_IMAGE_INTERFACE_H__
#define __GET_IMAGE_INTERFACE_H__

#include <vector>
#include <string>

typedef enum enGET_IMG_MODE_E
{
	GET_IMG_MODE_RECORD_PIC = 0x0,
	GET_IMG_MODE_PIC_DIR    = 0x1,
    GET_IMG_MODE_RECORD_DIR = 0x2,
    GET_IMG_MODE_CAM        = 0x3,
    GET_IMG_MODE_ALL
} GET_IMG_MODE_E;

typedef enum enGET_IMG_TYPE_E
{
	GET_IMG_TYPE_BGR_PACKAGE    = 0x0,
    GET_IMG_TYPE_ALL
} GET_IMG_TYPE_E;

typedef struct tagGetImgImg
{
	GET_IMG_TYPE_E      enType;
	unsigned long long  vir[3];
	unsigned long long  phy[3];
	unsigned int        stride[3];
	unsigned int        width;
	unsigned int        height;
}GET_IMG_IMG;

void GetImgTest();

class GetImage
{
public:
	GetImage();
	~GetImage();

	int Init(GET_IMG_MODE_E mode, const char* pPath);
	int Work(GET_IMG_IMG* pstImage);

    //注意：GET_IMG_MODE_RECORD_DIR 模式下，Work成功后才能获取到以下信息
	int GetFps()
	{
		return m_fps;
	};
	int GetTotalFrames()
	{
		return m_TotalFrames;
	};
    std::string GetMediaName()
    {
        return m_mediaName;
    };
public:
	GET_IMG_MODE_E              m_Mode;
	std::vector<std::string>    m_pic_files;
	unsigned int                m_pic_file_idx;
    
	std::vector<std::string>    m_record_files;
	unsigned int                m_record_file_idx;

	void*       m_Capture;
    void*       m_OrcImg;
	int         m_fps;
	int         m_TotalFrames;
    std::string m_mediaName;
};

#endif
