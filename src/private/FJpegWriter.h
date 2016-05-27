#ifndef __FJPEGWRITER_H__
#define __FJPEGWRITER_H__

#include "includes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

//
BOOL FJpegWriter_write(const char* fileName, const FImg* image);

#ifdef __cplusplus
}
#endif
#endif //__FJPEGWRITER_H__

