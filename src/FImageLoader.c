
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
 * Image������������
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
 * ������Bmp�������Ľӿ�ʵ��
 */ 
//��Ӧ.bmp���ļ�ͷ�ṹ
typedef struct _bitmap_fileheader
{
    U16 usFillChar;      //����ַ������ڶ����ֱ߽�
    U16 usFileType;      //�ļ����ͣ�0x424D('BM')Ϊ�ļ��еĴ洢���У�С�˴洢
    U32 uiFileSize;      //�ļ���С���ļ�ռ�õ��ֽڴ�С
    U16 usReserved1;     //������1��ֵ����Ϊ0
    U16 usReserved2;     //������2��ֵ����Ϊ0
    U32 uiDataOffset;    //ƫ�������ļ�ͷ��ʵ��λͼ���ݵ�ƫ���������ֽ�Ϊ��λ
} BmpFh;

//��Ӧ.bmp����Ϣͷ�ṹ
typedef struct _bitmap_infoheader
{
    U32 uiInfoSize;      //��Ϣͷ��С������Ϣͷռ�õ��ֽڴ�С��ֵΪ40
    U32 uiImgWidth;      //ͼ����
    U32 uiImgHeight;     //ͼ��߶ȣ�Ŀǰֻ֧�ֵ���λͼ(���½�Ϊԭ��)
    U16 usDevPlanes;     //Ŀ���豸λ������ֵ����Ϊ1
    U16 usBitCount;      //ÿ������ռ��������Ŀǰֻ֧��1λɫ��8λɫ��24λɫ
    U32 uiCompression;   //ѹ���㷨��Ŀǰֻ֧�ֲ�ѹ��ͼ��(BI_RGB��BI_RGB==0)
    U32 uiImgSize;       //ͼ���С��ͼ��ռ�õ��ֽڴ�С����ֵ����Ϊ4�ı���
    U32 uiXRes;          //ˮƽ�ֱ��ʣ���λΪ����/��
    U32 uiYRes;          //��ֱ�ֱ��ʣ���λΪ����/��
    U32 uiClrUsed;       //ʵ��ʹ�õ���ɫ��
    U32 uiClrImportant;  //��Ҫ����ɫ��
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

    //���Դ�λͼ�ļ�
    FILE* fp = fopen(imgFile, "r");
	if (NULL == fp) {
		return FALSE;
	}

    //���Զ�ȡλͼ�ļ�ͷ����Ϣͷ
    fseek(fp, 0, SEEK_SET);
    if ((14 != fread(&fh.usFileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	}

    //���λͼ�ļ���ʽ
    if ((fh.usFileType != 0x4d42) || 
        (ih.uiInfoSize != sizeof(BmpIh)) || 
        (ih.usBitCount != 24) || 
        ih.uiCompression) {
		fclose(fp);
		return FALSE;
    }

    //�ֱ����BMP��IMG��ÿ��ͼ�����ݳ��ȣ�ע��BMP��32λ�����IMG��8λ����
    U32 bmpLineSize = f_round_up(ih.uiImgWidth * ih.usBitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.uiImgWidth * ih.usBitCount, 8) / 8;
	
	FImg image;
	FImg_ctor(&image, FIMG_RGB888, ih.uiImgWidth, ih.uiImgHeight);
    U8* imgData = FImg_data(&image);
	//ָ�����һ��
    imgData += imgLineSize * (ih.uiImgHeight - 1);
	
    //Ϊ���ж�ȡͼ�����ݳ��Կ����л���
    U8 *bmpLine = (U8 *) malloc(bmpLineSize);

	//���ж�ȡBMPͼ�����ݣ�ע��Ŀǰֻ֧�ֵ���λͼ����BMP�����ڴ�ֱ����ת
	fseek(fp, fh.uiDataOffset, SEEK_SET);
    for (h = 0; h < ih.uiImgHeight; h++) {
       	fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
    }
	
    //�ͷ��л���
    free(bmpLine);
	fclose(fp);

	//��ֵimage
	FImg_dtor(&(me->super.image));
	me->super.image = image;
	
	return TRUE;
}



/**
 * ������Jpeg�������Ľӿ�ʵ��
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
 * ������Png�������Ľӿ�ʵ��
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
 * ��������ϵĽӿ�ʵ��
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

