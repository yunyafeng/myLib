
#include "FBmpLoader.h"
#include "FBitmap.h"


//BMP 加载器 继承自加载器
typedef struct f_bmp_loader {
	FImgLoader 	super;
} FBmpLoader;


/**
 * 内部函数
 */
static void FBmpLoader_ctor(FBmpLoader* me);
static void FBmpLoader_dtor(FBmpLoader* me);
static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile);


/**
 * 派生'类'Bmp加载器的接口实现
 */ 
static void FBmpLoader_ctor(FBmpLoader* me)
{
	F_REQUIRE(me);

	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	F_REQUIRE(me);

	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile)
{
	F_REQUIRE(me);

	BmpFh fh;
	BmpIh ih;
	U8 *lineBuffer;
	FILE* inFile;
	
	I32 err = 0;

	do {
	    /* 尝试打开位图文件 */
	    inFile = fopen(imgFile, "r");
		if (NULL == inFile) {
			err = -1;
			me->super.error = FIMGLOADER_IOERROR;
			break;
		}

	    /* 尝试读取位图文件头和信息头 */
	    fseek(inFile, 0, SEEK_SET);
	    if ((14 != fread(&fh.fileType, 1, 14, inFile)) || 
	        (40 != fread(&ih, 1, 40, inFile))) {
			err = -2;
			me->super.error = FIMGLOADER_IOERROR;
			break;
		} 

	    /* 检查图像文件格式是否为bitmap文件 */
	    if (fh.fileType != 0x4d42) {
			err = -3;
			me->super.error = FIMGLOADER_UNSUPPORTFMT;
			break;
		} 

		/*  检查是否支持当前的BMP格式 */
		if ((ih.infoSize != sizeof(BmpIh)) || 
	        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
	        ih.compression) {
			err = -4;
			me->super.error = FIMGLOADER_UNSUPPORTBMP;
			break;
	    }

		/* 创建IMAGE */
		if (NULL != me->super.image) {
			FImg_dtor(me->super.image);
		}
		else {
			me->super.image = F_NEW(FImg);
		}
		FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
		if (!FImg_isValid(me->super.image)) {
			err = -5;
			me->super.error = FIMGLOADER_TOOLARGE;
			break;
		}

	    /* 分别计算BMP和IMG的每行图像数据长度，注意BMP按32位对齐而IMG按8位对齐 */
	    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
	    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;
	    U8* imgData = FImg_data(me->super.image);
		
		/* 指向最后一行(注意目前只支持倒向位图，即BMP数据在垂直方向翻转) */
	    imgData += imgLineSize * (ih.imgHeight - 1);
	    /* 为逐行读取图像数据尝试开辟行缓冲 */
	    lineBuffer = F_NEWARR(U8, bmpLineSize);
		if (NULL == lineBuffer) {
			err = -6;
			me->super.error = FIMGLOADER_OUTOFMEMORY;
			break;
		}

		/* 逐行读取BMP图像数据,写入image中 */
		fseek(inFile, fh.dataOffset, SEEK_SET);
		U32 h;
	    for (h = 0; h < ih.imgHeight; h++) {
	       	int len = fread(lineBuffer, 1, bmpLineSize, inFile);
	        memcpy(imgData, lineBuffer, imgLineSize);
	        imgData -= imgLineSize;
			len++;
		}
	} while (0);

	/* 错误处理 */
	switch (err) {
	case -6:
	case -5:
	case -4:
	case -3:
	case -2:
		fclose(inFile);
	case -1:
		return FALSE;
		break;
	}
	
    /* 释放资源 */
	F_DELETE(lineBuffer);
	fclose(inFile);
	me->super.error = FIMGLOADER_SUCCESS;

	return TRUE;
}


/**
 * 对外提供接口
 */
FImgLoader *FBmpLoader_create()
{
	FBmpLoader *bmpLoader = F_NEW(FBmpLoader);
	FBmpLoader_ctor(bmpLoader);
	return FImgLoaderStar_cast(bmpLoader);
}
 
