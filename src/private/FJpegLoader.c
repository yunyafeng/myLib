
#include "FJpegLoader.h"
#include "jpeglib.h"
#include "jerror.h"


//JPEG ������ �̳��Լ�����
typedef struct f_jpeg_loader {
	FImgLoader 	super;
} FJpegLoader;

/**
 *�ڲ������൱��˽�к���
 */
static void FJpegLoader_ctor(FJpegLoader* me);
static void FJpegLoader_dtor(FJpegLoader* me);
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile);


/**
 * ����'��'Jpeg�������Ľӿ�ʵ��
 */ 
static void FJpegLoader_ctor(FJpegLoader* me)
{
	FImgLoader_ctor(&me->super, (FImageLoad)FJpegLoader_load, 
		            (FImgLoaderDtor)FJpegLoader_dtor);
}

static void FJpegLoader_dtor(FJpegLoader* me)
{
	FImgLoader_dtor(&me->super);
}

/**  
 *	���� libjpegʵ��jpeg�ļ���
 */
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *infile = NULL;

	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	return TRUE;
}


/***
 * �����ṩ�ӿ�
 */
FImgLoader *FJpegLoader_create()
{
	FJpegLoader *jpegLoader = F_NEW(FJpegLoader);
	FJpegLoader_ctor(jpegLoader);
	return (FImgLoader*)jpegLoader;
}
