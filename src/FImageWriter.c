#include "FImageWriter.h"
#include "FBmpWriter.h"
#include "FJpegWriter.h"
#include "FPngWriter.h"


//实现生成图片的接口
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

