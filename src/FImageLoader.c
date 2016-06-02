
#include "FImageLoader.h"
#include "FBmpLoader.h"
#include "FJpegLoader.h"
#include "FPngLoader.h"
#include "FList.h"


/**
 * 错误信息
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


//加载器组合 继承自加载器
typedef struct f_img_loaders {
	FImgLoader 	super;
	FList 		loaders;
} FImgLoaders;

static void FImgLoaders_ctor(FImgLoaders* me);
static void FImgLoaders_dtor(FImgLoaders* me);
static FImgLoader* FImgLoaders_create();
static BOOL FImgLoaders_load(FImgLoaders* me, const char* imgFile);
static BOOL FImgLoaders_add(FImgLoaders* me, FImgLoader* loader);



//生产加载器的工厂(其实并没有什么卵用)
static FImgLoader* SimpleFactory_create(const char* loaderType);


/**
 * Image加载器基'类'函数实现
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

//得到错误信息
const char* FImgLoader_errorMessage(FImgLoader* me)
{
	F_REQUIRE(me);
	return errorMessage[me->error];
}

//得到错误码
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
 * 加载器组合的接口实现,继承FImgLoader
 * 采用组合模式和职责链模式
 */ 
static void FImgLoaders_ctor(FImgLoaders* me)
{
	F_REQUIRE(me);

	//初始化基类
	FImgLoader_ctor(&me->super, (FImageLoad)FImgLoaders_load,
		            (FImgLoaderDtor)FImgLoaders_dtor);
	//初始化其他元素
	FList_ctor(&me->loaders);
}

static void FImgLoaders_dtor(FImgLoaders* me)
{
	F_REQUIRE(me);

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

	//创建并添加子部件(各类加载器)
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
	 * 遍历所有的加载器，尝试去加载图片文件,出错返回FALSE并设置错误码
	 *
	 * 当某个加载器加载失败且错误码为FIMGLOADER_UNSUPPORTFMT的时候才会
	 * 继续使用下一个进行加载，否则认为加载失败并设置相关错误码
	 * 全部加载失败则错误码为FIMGLOADER_UNSUPPORTFMT
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

