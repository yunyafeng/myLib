
#include "FPngLoader.h"


//PNG ������ �̳��Լ�����
typedef struct f_png_loader {
	FImgLoader 	super;
} FPngLoader;

/**
 * �ڲ��ӿ�
 */
static void FPngLoader_ctor(FPngLoader* me);
static void FPngLoader_dtor(FPngLoader* me);
static BOOL FPngLoader_load(FPngLoader* me, const char* imgFile);


/**
 * ����'��'Png�������Ľӿ�ʵ��
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
 * �����ṩ�ӿ�
 */
FImgLoader *FPngLoader_create()
{
	FPngLoader *pngLoader = F_NEW(FPngLoader);
	FPngLoader_ctor(pngLoader);
	return (FImgLoader*)pngLoader;
}
