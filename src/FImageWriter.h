#ifndef _FIMAGEWRITER_H_
#define _FIMAGEWRITER_H_

#include "includes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

//Ҫ����ͼƬ�ĸ�ʽ
typedef enum f_image_writer_formt
{
	FIMG_BMP,
	FIMG_JPEG,
	FIMG_PNG,
} FImgWriterFmt;

//��������ͼƬ�Ľӿ�
BOOL FImgWriter_write(const char* fileName, const FImg* image, FImgWriterFmt format);

#ifdef __cplusplus
}
#endif
#endif //_FIMAGEWRITER_H_

