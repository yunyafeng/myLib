
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
	F_REQUIRE(me);

	FImgLoader_ctor(&me->super, (FImageLoad)FJpegLoader_load, 
		            (FImgLoaderDtor)FJpegLoader_dtor);
}

static void FJpegLoader_dtor(FJpegLoader* me)
{
	F_REQUIRE(me);
	
	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

/**  
 *	依赖 libjpeg实现jpeg的加载
 */
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	F_REQUIRE(me);

	I32 err = 0;
	FILE* inFile;

	/* libjpeg相关 */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	do {
	    /* 打开图像文件 */
	    inFile = fopen(imgFile, "r");
		if (NULL == inFile) {
			err = -1;
			me->super.error = FIMGLOADER_IOERROR;
			break;
		}

	    /*
	     * 读取文件头2个字节和尾部2个字节 
		 * 判断文件是否为jpeg文件 (以0xD8FF开头, 0xD9FF结尾)
		 */
		U16 fileStart, fileEnd;
		int len = fread(&fileStart, 1, 2, inFile);
		fseek(inFile, -2, SEEK_END);
		len = fread(&fileEnd, 1, 2, inFile);
		if (fileStart != 0xD8FF || fileEnd != 0xD9FF) {
			err = -2;
			me->super.error = FIMGLOADER_UNSUPPORTFMT;
			break;
		}
		len--; //warning
		rewind(inFile);

		

		/* libjpeg 的固定步骤*/
	 	cinfo.err = jpeg_std_error(&jerr);
	 	jpeg_create_decompress(&cinfo);
	  	jpeg_stdio_src(&cinfo, inFile);
	  	(void) jpeg_read_header(&cinfo, TRUE);
	 	(void) jpeg_start_decompress(&cinfo);

		if (cinfo.output_components != 3) {
			err = -3;
			me->super.error = FIMGLOADER_UNSUPPORTJPG;
			break;
		}

		/* 创建IMAGE */
		if (NULL != me->super.image) {
			FImg_dtor(me->super.image);
		}
		else {
			me->super.image = F_NEW(FImg);
		}
		FImg_ctor(me->super.image, FIMG_RGB888, cinfo.output_width, cinfo.output_height);
		if (!FImg_isValid(me->super.image)) {
			err = -4;
			me->super.error = FIMGLOADER_TOOLARGE;
			break;
		}

		/* 
		 * 开始解压并读取文件到IMAGE 
		 * libjpeg 默认 RGB分量从低地址到高地址排列
		 * 我们要求正好相反所以需要转换
		 */
	   	U8* imgData = FImg_data(me->super.image);
		U32 w = cinfo.output_width * cinfo.output_components;
		while (cinfo.output_scanline < cinfo.output_height) {
			(void) jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&imgData, 1);

			/* 反转RGB排列(交换R和B) */
			int i;
			for (i = 0; i < w; i += 3) {
				U8 c = imgData[i];
				imgData[i] = imgData[i + 2];
				imgData[i + 2] = c;
			}
			imgData += w;
		}
	} while (0);


	/* 错误处理 */
	switch (err) {
	case -4:
	case -3:
		jpeg_destroy_decompress(&cinfo);
	case -2:
		fclose(inFile);
	case -1:
		return FALSE;
	}

	/* 释放资源 */
	(void) jpeg_finish_decompress(&cinfo);
	fclose(inFile);			
	me->super.error = FIMGLOADER_SUCCESS;

	return TRUE;	
}


/***
 * 对外提供接口
 */
FImgLoader *FJpegLoader_create()
{
	FJpegLoader *jpegLoader = F_NEW(FJpegLoader);
	FJpegLoader_ctor(jpegLoader);
	return FImgLoaderStar_cast(jpegLoader);
}
