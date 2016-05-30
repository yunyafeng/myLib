#ifndef __IMAGELOADER_H__
#define __IMAGELOADER_H__

#include "FIncludes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

struct f_img_loader;

//����load����ӿ�
typedef BOOL (*FImageLoad)(struct f_img_loader*, const char*);

//��������
typedef void (*FImgLoaderDtor)(struct f_img_loader*);

//ͼ�����������
typedef struct f_img_loader {
	FImageLoad 		load;
	FImgLoaderDtor 	dtor;
	FImg*			image;
} FImgLoader;

/**
 * Image��������'��'����ʵ��
 */
void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor);

void FImgLoader_dtor(FImgLoader* me);


//����һ��ͼ�������
FImgLoader* FImgLoader_create(void);

//����һ��ͼ�������
void FImgLoader_destroy(FImgLoader* me);

//�õ�������ɵ�ͼ������
FImg* FImgLoader_image(FImgLoader* me);


#ifdef __cplusplus
}
#endif
#endif //__IMAGELOADER_H__
