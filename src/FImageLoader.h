#ifndef _IMAGELOADER_H_
#define _IMAGELOADER_H_

#include "includes.h"
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
	FImg			image;
} FImgLoader;


//����һ��ͼ�������
FImgLoader* FImgLoader_create(void);

//����һ��ͼ�������
void FImgLoader_destroy(FImgLoader* me);

//�õ�������ɵ�ͼ������
FImg* FImgLoader_image(FImgLoader* me);


#ifdef __cplusplus
}
#endif
#endif
