
#include "FPngLoader.h"


//PNG 加载器 继承自加载器
typedef struct f_png_loader {
	FImgLoader 	super;
} FPngLoader;

/**
 * 内部接口
 */
static void FPngLoader_ctor(FPngLoader* me);
static void FPngLoader_dtor(FPngLoader* me);
static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile);


/**
 * 派生'类'Png加载器的接口实现
 */ 
static void FPngLoader_ctor(FPngLoader* me)
{
	F_ASSERT(me);

	FImgLoader_ctor(&me->super, (FImageLoad)FPngLoader_load, 
		            (FImgLoaderDtor)FPngLoader_dtor);
}

static void FPngLoader_dtor(FPngLoader* me)
{
	F_ASSERT(me);

	FImgLoader_dtor(&me->super);
}


static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile)
{
	F_ASSERT(me);

	me->super.error = FIMGLOADER_UNSUPPORTFMT;
	
	return FALSE;
}


/**
 * 对外提供接口
 */
FImgLoader *FPngLoader_create()
{
	FPngLoader *pngLoader = F_NEW(FPngLoader);
	FPngLoader_ctor(pngLoader);
	return (FImgLoader*)pngLoader;
}
