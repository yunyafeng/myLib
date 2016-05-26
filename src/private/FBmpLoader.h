#ifndef __FBMPLOADER_H__
#define __FBMPLOADER_H__

#include "FImageLoader.h"

#ifdef __cplusplus
extern "C" {
#endif

//创建一个BmpLoader 返回基'类'的指针 需要手动调用free来释放
FImgLoader *FBmpLoader_create();

#ifdef __cplusplus
}
#endif

#endif
