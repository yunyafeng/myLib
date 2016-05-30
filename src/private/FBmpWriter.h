#ifndef __FBMPWRITER_H__
#define __FBMPWRITER_H__

#include "FIncludes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

//
BOOL FBmpWriter_write(const char* fileName, const FImg* image);

#ifdef __cplusplus
}
#endif
#endif //__FBMPWRITER_H__
