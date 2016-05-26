
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
 * Image加载器基'类'函数实现
 */

// private:
static void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor)
{
	me->load = load;
	me->dtor = dtor;
	me->image = NULL;
}

static void FImgLoader_dtor(FImgLoader* me)
{
	me->load = NULL;
	me->dtor = NULL;
	me->image = NULL;
}

static FImgLoader* SimpleFactory_create(const char* loaderType)
{
	FImgLoader* loader = NULL;
	
	if (0 == strcasecmp("BmpLoader", loaderType)) {
		loader = FBmpLoader_create();
	}
	else if (0 == strcasecmp("JpegLoader", loaderType)) {
		loader = FJpegLoader_create();
	}
	else if (0 == strcasecmp("PngLoader", loaderType)) {
		loader = FPngLoader_create();
	}
	else if (0 == strcasecmp("ImageLoaders", loaderType)) {
		loader = FImgLoaders_create();
	}

	return loader;
}

// public:
FImg* FImgLoader_image(FImgLoader* me)
{
	return me->image;
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
 * 派生'类'Bmp加载器的接口实现
 */ 
//对应.bmp的文件头结构
typedef struct _bitmap_fileheader
{
    U16 reseved;      	//填充字符，用于对齐字边界
    U16 fileType;      	//文件类型，0x424D('BM')为文件中的存储序列，小端存储
    U32 fileSize;      	//文件大小，文件占用的字节大小
    U16 reseved1;     	//保留字1，值必须为0
    U16 reseved2;     	//保留字2，值必须为0
    U32 dataOffset;    	//偏移量，文件头到实际位图数据的偏移量，以字节为单位
} BmpFh;

//对应.bmp的信息头结构
typedef struct _bitmap_infoheader
{
    U32 infoSize;      	//信息头大小，该信息头占用的字节大小，值为40
    U32 imgWidth;      	//图像宽度
    U32 imgHeight;     	//图像高度，目前只支持倒向位图(左下角为原点)
    U16 devPlanes;     	//目标设备位面数，值必须为1
    U16 bitCount;      	//每像素所占比特数
    U32 compression;   	//压缩算法，目前只支持不压缩图像(BI_RGB，BI_RGB==0)
    U32 imgSize;       	//图像大小，图像占用的字节大小，其值必须为4的倍数
    U32 xRes;          	//水平分辨率，单位为像素/米
    U32 yRes;          	//垂直分辨率，单位为像素/米
    U32 clrUsed;       	//实际使用的颜色数
    U32 crImportant;  	//重要的颜色数
} BmpIh;

static void FBmpLoader_ctor(FBmpLoader* me)
{
	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	if (me->super.image) {
		FImg_dtor(me->super.image);
		free(me->super.image);
	}
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
    if ((14 != fread(&fh.fileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	} 

    //检查位图文件格式
    if ((fh.fileType != 0x4d42) || 
        (ih.infoSize != sizeof(BmpIh)) || 
        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
        ih.compression) {
		fclose(fp);
		return FALSE;
    }

    //分别计算BMP和IMG的每行图像数据长度，注意BMP按32位对齐而IMG按8位对齐
    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;

	//创建IMAGE
	if (NULL != me->super.image) {
		FImg_dtor(me->super.image);
	}
	else {
		me->super.image = (FImg*)malloc(sizeof(FImg));
	}
	FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
    U8* imgData = FImg_data(me->super.image);
	
	//指向最后一行(注意目前只支持倒向位图，即BMP数据在垂直方向翻转)
    imgData += imgLineSize * (ih.imgHeight - 1);
    //为逐行读取图像数据尝试开辟行缓冲
    U8 *bmpLine = (U8 *) malloc(bmpLineSize);
	//逐行读取BMP图像数据,写入image中
	fseek(fp, fh.dataOffset, SEEK_SET);
    for (h = 0; h < ih.imgHeight; h++) {
       	fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
    }
	
    //释放行缓冲
    free(bmpLine);
	fclose(fp);

	return TRUE;
}



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

static FImgLoader *FJpegLoader_create()
{
	FJpegLoader *jpegLoader = (FJpegLoader*)malloc(sizeof(FBmpLoader));
	FJpegLoader_ctor(jpegLoader);
	return (FImgLoader*)jpegLoader;
}

static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile)
{
	return FALSE;
}



/**
 * 派生'类'Png加载器的接口实现
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

static FImgLoader *FPngLoader_create()
{
	FPngLoader *pngLoader = (FPngLoader*)malloc(sizeof(FBmpLoader));
	FPngLoader_ctor(pngLoader);
	return (FImgLoader*)pngLoader;
}

static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile)
{
	return FALSE;
}



/**
 * 加载器组合的接口实现,继承FImgLoader
 * 采用组合模式和职责链模式
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
			me->super.image = FImgLoader_image(loader);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL FImgLoaders_registerLoader(FImgLoaders* me, FImgLoader* loader)
{
	return FList_pushBack(&me->loaders, loader);
}

