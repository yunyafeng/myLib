
#include "FJpegLoader.h"


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


static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	return FALSE;
}


/***
 * �����ṩ�ӿ�
 */
FImgLoader *FJpegLoader_create()
{
	FJpegLoader *jpegLoader = (FJpegLoader*)malloc(sizeof(FJpegLoader));
	FJpegLoader_ctor(jpegLoader);
	return (FImgLoader*)jpegLoader;
}
