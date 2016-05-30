
#include "FImageLoader.h"
#include "FBmpLoader.h"
#include "FJpegLoader.h"
#include "FPngLoader.h"
#include "FList.h"


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


//生产加载器的工厂(其实并没有什么卵用)
static FImgLoader* SimpleFactory_create(const char* loaderType);


/**
 * Image加载器基'类'函数实现
 */
// private:
void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor)
{
	me->load = load;
	me->dtor = dtor;
	me->image = NULL;
}

void FImgLoader_dtor(FImgLoader* me)
{
	me->load = NULL;
	me->dtor = NULL;
	me->image = NULL;
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
	else if (0 == strcasecmp("ImageLoaders", loaderType)) {
		loader = FImgLoaders_create();
	}

	return loader;
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

	//创建并注册加载器
	FImgLoaders_registerLoader(me, SimpleFactory_create("BmpLoader"));
	FImgLoaders_registerLoader(me, SimpleFactory_create("JpegLoader"));
	FImgLoaders_registerLoader(me, SimpleFactory_create("PngLoader"));
}

static void FImgLoaders_dtor(FImgLoaders* me)
{
	//删除所有的loader 释放空间
	while (!FList_isEmpty(&me->loaders)) {
		FImgLoader* loader =(FImgLoader*)FList_takeAtIndex(&me->loaders, 0);
		loader->dtor(loader);
		F_DELETE(loader);
	}
	//析构List
	FList_dtor(&me->loaders);
	//析构基类
	FImgLoader_dtor(&me->super);
}

static FImgLoader* FImgLoaders_create()
{
	FImgLoaders *loaders = F_NEW(FImgLoaders);
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

