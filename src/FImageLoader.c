
#include "FImageLoader.h"
#include "FList.h"

//BMP 加载器 继承自加载器
typedef struct f_bmp_loader {
	FImgLoader 	super;
} FBmpLoader;


static void FBmpLoader_ctor(FBmpLoader* me);
static void FBmpLoader_dtor(FBmpLoader* me);
static FImgLoader *FBmpLoader_create();
static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile);



//JPEG 加载器 继承自加载器
typedef struct f_jpeg_loader {
	FImgLoader 	super;
} FJpegLoader;

static void FJpegLoader_ctor(FJpegLoader* me);
static void FJpegLoader_dtor(FJpegLoader* me);
static FImgLoader *FJpegLoader_create();
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile);



//PNG 加载器 继承自加载器
typedef struct f_png_loader {
	FImgLoader 	super;
} FPngLoader;

static void FPngLoader_ctor(FPngLoader* me);
static void FPngLoader_dtor(FPngLoader* me);
static FImgLoader *FPngLoader_create();
static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile);



//加载器组合 继承自加载器
typedef struct f_img_loaders {
	FImgLoader 	super;
	FList 		loaders;
} FImgLoaders;

static void FImgLoaders_ctor(FImgLoaders* me);
static void FImgLoaders_dtor(FImgLoaders* me);
static FImgLoader* FImgLoaders_create();
static BOOL FImgLoaders_load(FImgLoaders* me, const char* imgFile);
static BOOL FImgLoaders_registerLoader(FImgLoaders* me, FImgLoader* loader);


/**
 * Image加载器基础类
 */

// private:
static void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor)
{
	me->load = load;
	me->dtor = dtor;
	FImg_ctor(&me->image, FIMG_RGB888, 0, 0);
}

static void FImgLoader_dtor(FImgLoader* me)
{
	FImg_dtor(&me->image);
}

static FImgLoader* SimpleFactory_create(const char* loaderType)
{
	FImgLoader* loader = NULL;
	
	if (0 == strcmp("BmpLoader", loaderType)) {
		loader = FBmpLoader_create();
	}
	else if (0 == strcmp("JpegLoader", loaderType)) {
		loader = FJpegLoader_create();
	}
	else if (0 == strcmp("PngLoader", loaderType)) {
		loader = FPngLoader_create();
	}
	else if (0 == strcmp("ImageLoaders", loaderType)) {
		loader = FImgLoaders_create();
	}

	return loader;
}

// public:
FImg* FImgLoader_image(FImgLoader* me)
{
	return &me->image;
}


// public static:
FImgLoader* FImgLoader_create()
{
	return SimpleFactory_create("ImageLoaders");
}

void FImgLoader_destroy(FImgLoader* me)
{
	me->dtor(me);
	free(me);
}



/**
 * 派生类Bmp加载器的接口实现
 */ 
//对应.bmp的文件头结构
typedef struct _bitmap_fileheader
{
    U16 usFillChar;      //填充字符，用于对齐字边界
    U16 usFileType;      //文件类型，0x424D('BM')为文件中的存储序列，小端存储
    U32 uiFileSize;      //文件大小，文件占用的字节大小
    U16 usReserved1;     //保留字1，值必须为0
    U16 usReserved2;     //保留字2，值必须为0
    U32 uiDataOffset;    //偏移量，文件头到实际位图数据的偏移量，以字节为单位
} BmpFh;

//对应.bmp的信息头结构
typedef struct _bitmap_infoheader
{
    U32 uiInfoSize;      //信息头大小，该信息头占用的字节大小，值为40
    U32 uiImgWidth;      //图像宽度
    U32 uiImgHeight;     //图像高度，目前只支持倒向位图(左下角为原点)
    U16 usDevPlanes;     //目标设备位面数，值必须为1
    U16 usBitCount;      //每像素所占比特数，目前只支持1位色、8位色、24位色
    U32 uiCompression;   //压缩算法，目前只支持不压缩图像(BI_RGB，BI_RGB==0)
    U32 uiImgSize;       //图像大小，图像占用的字节大小，其值必须为4的倍数
    U32 uiXRes;          //水平分辨率，单位为像素/米
    U32 uiYRes;          //垂直分辨率，单位为像素/米
    U32 uiClrUsed;       //实际使用的颜色数
    U32 uiClrImportant;  //重要的颜色数
} BmpIh;

static void FBmpLoader_ctor(FBmpLoader* me)
{
	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	FImgLoader_dtor(&me->super);
}

static FImgLoader *FBmpLoader_create()
{
	FBmpLoader *bmpLoader = (FBmpLoader*)malloc(sizeof(FBmpLoader));
	FBmpLoader_ctor(bmpLoader);
	return (FImgLoader*)bmpLoader;
}

static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile)
{
	BmpFh fh;
	BmpIh ih;
	U32 h;

    //尝试打开位图文件
    FILE* fp = fopen(imgFile, "r");
	if (NULL == fp) {
		return FALSE;
	}

    //尝试读取位图文件头和信息头
    fseek(fp, 0, SEEK_SET);
    if ((14 != fread(&fh.usFileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	}

    //检查位图文件格式
    if ((fh.usFileType != 0x4d42) || 
        (ih.uiInfoSize != sizeof(BmpIh)) || 
        (ih.usBitCount != 24) || 
        ih.uiCompression) {
		fclose(fp);
		return FALSE;
    }

    //分别计算BMP和IMG的每行图像数据长度，注意BMP按32位对齐而IMG按8位对齐
    U32 bmpLineSize = f_round_up(ih.uiImgWidth * ih.usBitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.uiImgWidth * ih.usBitCount, 8) / 8;
	
	FImg image;
	FImg_ctor(&image, FIMG_RGB888, ih.uiImgWidth, ih.uiImgHeight);
    U8* imgData = FImg_data(&image);
	//指向最后一行
    imgData += imgLineSize * (ih.uiImgHeight - 1);
	
    //为逐行读取图像数据尝试开辟行缓冲
    U8 *bmpLine = (U8 *) malloc(bmpLineSize);

	//逐行读取BMP图像数据，注意目前只支持倒向位图，即BMP数据在垂直方向翻转
	fseek(fp, fh.uiDataOffset, SEEK_SET);
    for (h = 0; h < ih.uiImgHeight; h++) {
       	fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
    }
	
    //释放行缓冲
    free(bmpLine);
	fclose(fp);

	//赋值image
	FImg_dtor(&(me->super.image));
	me->super.image = image;
	
	return TRUE;
}



/**
 * 派生类Jpeg加载器的接口实现
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

static FImgLoader *FBmpLoader_create()
{
	FJpegLoader *jpegLoader = (FBmpLoader*)malloc(sizeof(FBmpLoader));
	FJpegLoader_ctor(jpegLoader);
	return (FImgLoader*)jpegLoader;
}

static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	return FALSE;
}



/**
 * 派生类Png加载器的接口实现
 */ 
static void FPngLoader_ctor(FPngLoader* me)
{
	FImgLoader_ctor(&me->super, (FImageLoad)FPngLoader_load, 
		            (FImgLoaderDtor)FPngLoader_dtor);
}

static void FPngLoader_dtor(FPngLoader* me)
{
	FImgLoader_dtor(&me->super);
}

static FImgLoader *FBmpLoader_create()
{
	FPngLoader *pngLoader = (FBmpLoader*)malloc(sizeof(FBmpLoader));
	FPngLoader_ctor(pngLoader);
	return (FImgLoader*)pngLoader;
}

static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile)
{
	return FALSE;
}



/**
 * 加载器组合的接口实现
 */ 
static void FImgLoaders_ctor(FImgLoaders* me)
{
	//初始化基类
	FImgLoader_ctor(&me->super, (FImageLoad)FImgLoaders_load,
		            (FImgLoaderDtor)FImgLoaders_dtor);
	//初始化其他元素
	FList_ctor(&me->loaders);

	//创建加载器 bmp jpeg png
	FImgLoader* bmpLoader = SimpleFactory_create("BmpLoader");
	FImgLoader* jpegLoader = SimpleFactory_create("JpegLoader");
	FImgLoader* pngLoader = SimpleFactory_create("PngLoader");

	//注册加载器
	FImgLoaders_registerLoader(me, bmpLoader);
	FImgLoaders_registerLoader(me, jpegLoader);
	FImgLoaders_registerLoader(me, pngLoader);
}

static void FImgLoaders_dtor(FImgLoaders* me)
{
	//删除所有的loader 释放空间
	while (!FList_isEmpty(&me->loaders)) {
		FImgLoader* loader =(FImgLoader*)FList_takeAtIndex(&me->loaders, 0);
		loader->dtor(loader);
		free(loader);
	}
	//析构List
	FList_dtor(&me->loaders);
	//析构基类
	FImgLoader_dtor(&me->super);
}

static FImgLoader* FImgLoaders_create()
{
	FImgLoaders *loaders = (FImgLoaders *)malloc(sizeof(FImgLoaders));
	FImgLoaders_ctor(loaders);
	return (FImgLoader*)loaders;
}

static BOOL FImgLoaders_load(FImgLoaders* me, const char* imgFile)
{
	U32 i;
	FList* loaders = &me->loaders;
	U32 count = FList_count(loaders);

	for (i = 0; i < count; ++i) {
		FImgLoader* loader = (FImgLoader*)FList_atIndex(loaders, i);
		if (loader->load(loader, imgFile)) {
			FImg_dtor(&(me->super.image));
			me->super.image = FImg_copy(FImgLoader_image(loader));
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL FImgLoaders_registerLoader(FImgLoaders* me, FImgLoader* loader)
{
	return FList_pushBack(&me->loaders, loader);
}

