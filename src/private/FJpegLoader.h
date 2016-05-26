
#ifndef __FJPEGLOADER_H__
#define __FJPEGLOADER_H__

#include "FImageLoader.h"

#ifdef __cplusplus
extern "C" {
#endif

//创建一个JpegLoader 返回基'类'的指针 需要手动调用free来释放
FImgLoader *FJpegLoader_create();

#ifdef __cplusplus
}
#endif

#endif //__FJPEGLOADER_H__

