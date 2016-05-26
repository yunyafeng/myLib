
#include "FImageLoader.h"
#include "FList.h"

//BMP ������ �̳��Լ�����
typedef struct f_bmp_loader {
	FImgLoader 	super;
} FBmpLoader;


static void FBmpLoader_ctor(FBmpLoader* me);
static void FBmpLoader_dtor(FBmpLoader* me);
static FImgLoader *FBmpLoader_create();
static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile);



//JPEG ������ �̳��Լ�����
typedef struct f_jpeg_loader {
	FImgLoader 	super;
} FJpegLoader;

static void FJpegLoader_ctor(FJpegLoader* me);
static void FJpegLoader_dtor(FJpegLoader* me);
static FImgLoader *FJpegLoader_create();
static BOOL FJpegLoader_load(FJpegLoader* me, const char* imgFile);



//PNG ������ �̳��Լ�����
typedef struct f_png_loader {
	FImgLoader 	super;
} FPngLoader;

static void FPngLoader_ctor(FPngLoader* me);
static void FPngLoader_dtor(FPngLoader* me);
static FImgLoader *FPngLoader_create();
static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile);



//��������� �̳��Լ�����
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
 * Image��������'��'����ʵ��
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
 * ����'��'Bmp�������Ľӿ�ʵ��
 */ 
//��Ӧ.bmp���ļ�ͷ�ṹ
typedef struct _bitmap_fileheader
{
    U16 reseved;      	//����ַ������ڶ����ֱ߽�
    U16 fileType;      	//�ļ����ͣ�0x424D('BM')Ϊ�ļ��еĴ洢���У�С�˴洢
    U32 fileSize;      	//�ļ���С���ļ�ռ�õ��ֽڴ�С
    U16 reseved1;     	//������1��ֵ����Ϊ0
    U16 reseved2;     	//������2��ֵ����Ϊ0
    U32 dataOffset;    	//ƫ�������ļ�ͷ��ʵ��λͼ���ݵ�ƫ���������ֽ�Ϊ��λ
} BmpFh;

//��Ӧ.bmp����Ϣͷ�ṹ
typedef struct _bitmap_infoheader
{
    U32 infoSize;      	//��Ϣͷ��С������Ϣͷռ�õ��ֽڴ�С��ֵΪ40
    U32 imgWidth;      	//ͼ����
    U32 imgHeight;     	//ͼ��߶ȣ�Ŀǰֻ֧�ֵ���λͼ(���½�Ϊԭ��)
    U16 devPlanes;     	//Ŀ���豸λ������ֵ����Ϊ1
    U16 bitCount;      	//ÿ������ռ������
    U32 compression;   	//ѹ���㷨��Ŀǰֻ֧�ֲ�ѹ��ͼ��(BI_RGB��BI_RGB==0)
    U32 imgSize;       	//ͼ���С��ͼ��ռ�õ��ֽڴ�С����ֵ����Ϊ4�ı���
    U32 xRes;          	//ˮƽ�ֱ��ʣ���λΪ����/��
    U32 yRes;          	//��ֱ�ֱ��ʣ���λΪ����/��
    U32 clrUsed;       	//ʵ��ʹ�õ���ɫ��
    U32 crImportant;  	//��Ҫ����ɫ��
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

    //���Դ�λͼ�ļ�
    FILE* fp = fopen(imgFile, "r");
	if (NULL == fp) {
		return FALSE;
	}

    //���Զ�ȡλͼ�ļ�ͷ����Ϣͷ
    fseek(fp, 0, SEEK_SET);
    if ((14 != fread(&fh.fileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	} 

    //���λͼ�ļ���ʽ
    if ((fh.fileType != 0x4d42) || 
        (ih.infoSize != sizeof(BmpIh)) || 
        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
        ih.compression) {
		fclose(fp);
		return FALSE;
    }

    //�ֱ����BMP��IMG��ÿ��ͼ�����ݳ��ȣ�ע��BMP��32λ�����IMG��8λ����
    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;

	//����IMAGE
	if (NULL != me->super.image) {
		FImg_dtor(me->super.image);
	}
	else {
		me->super.image = (FImg*)malloc(sizeof(FImg));
	}
	FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
    U8* imgData = FImg_data(me->super.image);
	
	//ָ�����һ��(ע��Ŀǰֻ֧�ֵ���λͼ����BMP�����ڴ�ֱ����ת)
    imgData += imgLineSize * (ih.imgHeight - 1);
    //Ϊ���ж�ȡͼ�����ݳ��Կ����л���
    U8 *bmpLine = (U8 *) malloc(bmpLineSize);
	//���ж�ȡBMPͼ������,д��image��
	fseek(fp, fh.dataOffset, SEEK_SET);
    for (h = 0; h < ih.imgHeight; h++) {
       	fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
    }
	
    //�ͷ��л���
    free(bmpLine);
	fclose(fp);

	return TRUE;
}



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
 * ����'��'Png�������Ľӿ�ʵ��
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
 * ��������ϵĽӿ�ʵ��,�̳�FImgLoader
 * �������ģʽ��ְ����ģʽ
 */ 
static void FImgLoaders_ctor(FImgLoaders* me)
{
	//��ʼ������
	FImgLoader_ctor(&me->super, (FImageLoad)FImgLoaders_load,
		            (FImgLoaderDtor)FImgLoaders_dtor);
	//��ʼ������Ԫ��
	FList_ctor(&me->loaders);

	//���������� bmp jpeg png
	FImgLoader* bmpLoader = SimpleFactory_create("BmpLoader");
	FImgLoader* jpegLoader = SimpleFactory_create("JpegLoader");
	FImgLoader* pngLoader = SimpleFactory_create("PngLoader");

	//ע�������
	FImgLoaders_registerLoader(me, bmpLoader);
	FImgLoaders_registerLoader(me, jpegLoader);
	FImgLoaders_registerLoader(me, pngLoader);
}

static void FImgLoaders_dtor(FImgLoaders* me)
{
	//ɾ�����е�loader �ͷſռ�
	while (!FList_isEmpty(&me->loaders)) {
		FImgLoader* loader =(FImgLoader*)FList_takeAtIndex(&me->loaders, 0);
		loader->dtor(loader);
		free(loader);
	}
	//����List
	FList_dtor(&me->loaders);
	//��������
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

