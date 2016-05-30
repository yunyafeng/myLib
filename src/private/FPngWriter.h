#ifndef __FPNGWRITER_H__
#define __FPNGWRITER_H__

#include "FIncludes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

//
BOOL FPngWriter_write(const char* fileName, const FImg* image);

#ifdef __cplusplus
}
#endif
#endif //__FPNGWRITER_H__
