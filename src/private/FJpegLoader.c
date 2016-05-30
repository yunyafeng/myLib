
#include "FJpegLoader.h"
#include "jpeglib.h"
#include "jerror.h"


//JPEG 加载器 继承自加载器
typedef struct f_jpeg_loader {
	FImgLoader 	super;
} FJpegLoader;

/**
 *内部函数相当于私有函数
 */
static void FJpegLoader_ctor(FJpegLoader* me);
static void FJpegLoader_dtor(FJpegLoader* me);
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile);


/**
 * 派生'类'Jpeg加载器的接口实现
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
 *	依赖 libjpeg实现jpeg的加载
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
 * 对外提供接口
 */
FImgLoader *FJpegLoader_create()
{
	FJpegLoader *jpegLoader = F_NEW(FJpegLoader);
	FJpegLoader_ctor(jpegLoader);
	return (FImgLoader*)jpegLoader;
}
