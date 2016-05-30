
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
	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

/**  
 *	���� libjpegʵ��jpeg�ļ���
 */
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	I32 err = 0;
	U16 fileStart, fileEnd;

    /* ��ͼ���ļ� */
    FILE* inFile = fopen(imgFile, "r");
	if (NULL == inFile) {
		err = -1;
		goto ERROR;
	}

    /*
     * ��ȡ�ļ�ͷ2���ֽں�β��2���ֽ� 
	 * �ж��ļ��Ƿ�Ϊjpeg�ļ� (��0xD8FF��ͷ, 0xD9FF��β)
	 */
	int len = fread(&fileStart, 1, 2, inFile);
	fseek(inFile, -2, SEEK_END);
	len = fread(&fileEnd, 1, 2, inFile);
	if (fileStart != 0xD8FF || fileEnd != 0xD9FF) {
		err = -2;
		goto ERROR;
	}
	len--; //warning
	rewind(inFile);

	
	/* ʹ��libjpeg������jpeg�ļ� */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	/* libjpeg �Ĺ̶�����*/
 	cinfo.err = jpeg_std_error(&jerr);
 	jpeg_create_decompress(&cinfo);
  	jpeg_stdio_src(&cinfo, inFile);
  	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	if (cinfo.output_components != 3) {
		err = -3;
		goto ERROR;
	}

	/* ����IMAGE */
	if (NULL != me->super.image) {
		FImg_dtor(me->super.image);
	}
	else {
		me->super.image = F_NEW(FImg);
	}
	FImg_ctor(me->super.image, FIMG_RGB888, cinfo.output_width, cinfo.output_height);

	/* 
	 * ��ȡ�ļ���IMAGE 
	 * libjpeg Ĭ�� RGB�����ӵ͵�ַ���ߵ�ַ����
	 * ����Ҫ�������෴������Ҫת��
	 */
    U8* imgData = FImg_data(me->super.image);
	U32 w = cinfo.output_width * cinfo.output_components;
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&imgData, 1);

		/* ��תRGB����(����R��B) */
		int i;
		for (i = 0; i < w; i += 3) {
			U8 c = imgData[i];
			imgData[i] = imgData[i + 2];
			imgData[i + 2] = c;
		}
		imgData += w;
	}

ERROR:
	switch (err) {
	case 0:
	case -3:
		(void) jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	case -2:
		fclose(inFile);
	case -1:
		break;
	}

	return !err ? TRUE : FALSE;	
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
