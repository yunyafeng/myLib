
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
	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile)
{
	BmpFh fh;
	BmpIh ih;
	U32 h;

    /* 尝试打开位图文件 */
    FILE* fp = fopen(imgFile, "r");
	if (NULL == fp) {
		return FALSE;
	}

    /* 尝试读取位图文件头和信息头 */
    fseek(fp, 0, SEEK_SET);
    if ((14 != fread(&fh.fileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	} 

    /* 检查位图文件格式 */
    if ((fh.fileType != 0x4d42) || 
        (ih.infoSize != sizeof(BmpIh)) || 
        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
        ih.compression) {
		fclose(fp);
		return FALSE;
    }

    /* 分别计算BMP和IMG的每行图像数据长度，注意BMP按32位对齐而IMG按8位对齐 */
    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;

	/* 创建IMAGE */
	if (NULL != me->super.image) {
		FImg_dtor(me->super.image);
	}
	else {
		me->super.image = F_NEW(FImg);
	}
	FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
    U8* imgData = FImg_data(me->super.image);
	
	/* 指向最后一行(注意目前只支持倒向位图，即BMP数据在垂直方向翻转) */
    imgData += imgLineSize * (ih.imgHeight - 1);
    /* 为逐行读取图像数据尝试开辟行缓冲 */
    U8 *bmpLine = F_NEWARR(U8, bmpLineSize);
	/* 逐行读取BMP图像数据,写入image中 */
	fseek(fp, fh.dataOffset, SEEK_SET);
    for (h = 0; h < ih.imgHeight; h++) {
       	int len = fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
		len++;
    }
	
    /* 释放行缓冲 */
    F_DELETE(bmpLine);
	fclose(fp);

	return TRUE;
}


/**
 * 对外提供接口
 */
FImgLoader *FBmpLoader_create()
{
	FBmpLoader *bmpLoader = F_NEW(FBmpLoader);
	FBmpLoader_ctor(bmpLoader);
	return (FImgLoader*)bmpLoader;
}
 