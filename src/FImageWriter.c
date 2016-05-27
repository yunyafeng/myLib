#include "FImageWriter.h"
#include "FBmpWriter.h"
#include "FJpegWriter.h"
#include "FPngWriter.h"


//ʵ������ͼƬ�Ľӿ�
BOOL FImgWriter_write(const char* fileName, const FImg* image, FImgWriterFmt format)
{
	switch (format) 
	{
	case FIMG_BMP:
		return FBmpWriter_write(fileName, image);
	case FIMG_JPEG:
		return FJpegWriter_write(fileName, image);
	case FIMG_PNG:
		return FPngWriter_write(fileName, image);
	default:
		break;
	}

	return FALSE;
}

