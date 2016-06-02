
#include "FImageLoader.h"
#include "FBmpLoader.h"
#include "FJpegLoader.h"
#include "FPngLoader.h"
#include "FList.h"


/**
 * ������Ϣ
 */
static const char *errorMessage[] = {
	"Success!",
	"System IO error!",
	"Unsupported image format!",
	"Unsupported BMP format!",
	"Unsupported JPG format!",
	"Unsupported PNG format!",
	"Image is too large, need more memory!",
	"Out of memory!",
	"Unknow error!"
};


//��������� �̳��Լ�����
typedef struct f_img_loaders {
	FImgLoader 	super;
	FList 		loaders;
} FImgLoaders;

static void FImgLoaders_ctor(FImgLoaders* me);
static void FImgLoaders_dtor(FImgLoaders* me);
static FImgLoader* FImgLoaders_create();
static BOOL FImgLoaders_load(FImgLoaders* me, const char* imgFile);
static BOOL FImgLoaders_add(FImgLoaders* me, FImgLoader* loader);



//�����������Ĺ���(��ʵ��û��ʲô����)
static FImgLoader* SimpleFactory_create(const char* loaderType);


/**
 * Image��������'��'����ʵ��
 */
// private:
void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor)
{
	F_REQUIRE(me);

	me->load = load;
	me->dtor = dtor;
	me->image = NULL;
	me->error = FIMGLOADER_UNKOWNERR;
}

void FImgLoader_dtor(FImgLoader* me)
{
	F_REQUIRE(me);

	me->load = NULL;
	me->dtor = NULL;
	me->image = NULL;
	me->error = FIMGLOADER_UNKOWNERR;
}

// public:
FImg* FImgLoader_image(FImgLoader* me)
{
	F_REQUIRE(me);

	return me->image;
}

//�õ�������Ϣ
const char* FImgLoader_errorMessage(FImgLoader* me)
{
	F_REQUIRE(me);
	return errorMessage[me->error];
}

//�õ�������
FImgLoaderErrCode FImgLoader_errorCode(FImgLoader* me)
{
	F_REQUIRE(me);
	return me->error;
}

// public static:
FImgLoader* FImgLoader_create()
{
	return FImgLoaders_create();
}

void FImgLoader_destroy(FImgLoader* me)
{
	F_REQUIRE(me);

	me->dtor(me);
	F_DELETE(me);
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
 
	return loader;
}



/**
 * ��������ϵĽӿ�ʵ��,�̳�FImgLoader
 * �������ģʽ��ְ����ģʽ
 */ 
static void FImgLoaders_ctor(FImgLoaders* me)
{
	F_REQUIRE(me);

	//��ʼ������
	FImgLoader_ctor(&me->super, (FImageLoad)FImgLoaders_load,
		            (FImgLoaderDtor)FImgLoaders_dtor);
	//��ʼ������Ԫ��
	FList_ctor(&me->loaders);
}

static void FImgLoaders_dtor(FImgLoaders* me)
{
	F_REQUIRE(me);

	//ɾ�����е�loader �ͷſռ�
	while (!FList_isEmpty(&me->loaders)) {
		FImgLoader* loader =(FImgLoader*)FList_takeAtIndex(&me->loaders, 0);
		loader->dtor(loader);
		F_DELETE(loader);
	}
	//����List
	FList_dtor(&me->loaders);
	//��������
	FImgLoader_dtor(&me->super);
}

static FImgLoader* FImgLoaders_create()
{
	FImgLoaders *loaders = F_NEW(FImgLoaders);
	FImgLoaders_ctor(loaders);

	//����������Ӳ���(���������)
	FImgLoaders_add(loaders, SimpleFactory_create("BmpLoader"));
	FImgLoaders_add(loaders, SimpleFactory_create("JpegLoader"));
	FImgLoaders_add(loaders, SimpleFactory_create("PngLoader"));
	
	return FImgLoaderStar_cast(loaders);
}

static BOOL FImgLoaders_load(FImgLoaders* me, const char* imgFile)
{
	F_REQUIRE(me);

	U32 i;
	FList* allLoaders = &me->loaders;
	U32 count = FList_count(allLoaders);

	/*
	 * �������еļ�����������ȥ����ͼƬ�ļ�,������FALSE�����ô�����
	 *
	 * ��ĳ������������ʧ���Ҵ�����ΪFIMGLOADER_UNSUPPORTFMT��ʱ��Ż�
	 * ����ʹ����һ�����м��أ�������Ϊ����ʧ�ܲ�������ش�����
	 * ȫ������ʧ���������ΪFIMGLOADER_UNSUPPORTFMT
	 */
	for (i = 0; i < count; ++i) {
		FImgLoader* loader = (FImgLoader*)FList_atIndex(allLoaders, i);
		if (loader->load(loader, imgFile)) {
			me->super.image = FImgLoader_image(loader);
			me->super.error = FIMGLOADER_SUCCESS;
			return TRUE;
		}
		else {
			if (loader->error == FIMGLOADER_UNSUPPORTFMT) {
				continue;
			}
			me->super.error = loader->error;
			return FALSE;
		}
	}
	me->super.error = FIMGLOADER_UNSUPPORTFMT;
	return FALSE;
}

static BOOL FImgLoaders_add(FImgLoaders* me, FImgLoader* loader)
{	
	F_REQUIRE(me);

	return loader ? FList_pushBack(&me->loaders, loader) : FALSE;
}

/*
static BOOL FImgLoaders_remove(FImgLoaders* me, FImgLoader* loader)
{	
	F_REQUIRE(me);

	int index = FList_findForward(&me->loaders, loader, NULL);
	return FList_takeAtIndex(&me->loaders, index) ? TRUE : FALSE;
}
*/

