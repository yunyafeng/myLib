#ifndef _IMAGELOADER_H_
#define _IMAGELOADER_H_

#include "includes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

struct f_img_loader;

//定义load处理接口
typedef BOOL (*FImageLoad)(struct f_img_loader*, const char*);

//析构函数
typedef void (*FImgLoaderDtor)(struct f_img_loader*);

//图像加载器定义
typedef struct f_img_loader {
	FImageLoad 		load;
	FImgLoaderDtor 	dtor;
	FImg			image;
} FImgLoader;


//创建一个图像加载器
FImgLoader* FImgLoader_create(void);

//销毁一个图像加载器
void FImgLoader_destroy(FImgLoader* me);

//得到加载完成的图像数据
FImg* FImgLoader_image(FImgLoader* me);


#ifdef __cplusplus
}
#endif
#endif
