#ifndef __INCLUDES_H
#define __INCLUDES_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define F32                                 float                       
#define F64                                 double                      
#define U8                                  unsigned char               
#define U16                                 unsigned short              
#define U32                                 unsigned int                
#define U64                                 unsigned long long          
#define I32                                 signed int

#ifndef BOOL
#define BOOL                                U32
#endif
#ifndef FALSE
#define FALSE                               0
#endif
#ifndef TRUE
#define TRUE                                (!FALSE)
#endif
#ifndef NULL
#ifdef __cplusplus
#define NULL                                0
#else
#define NULL								((void*)0)
#endif
#endif
	
#define F_LOG_LEVEL							4
#define ENABLE_DEBUG						0
#define ENABLE_ASSERT						0

#define F_LOG_VERBO							0
#define F_LOG_TRACE							1
#define F_LOG_DEBUG							2
#define F_LOG_INFO							3
#define F_LOG_ERROR							4


#define F_DECLAR(_class, _name) 			_class _name; _class##_ctor(&_name)
#define F_DDECLAR(_class, _name) 			_class##_dtor(&_name)

#define F_NEW(_class, _name) 				_class* _name = (_class*)malloc(sizeof(_class)); \
											_class##_ctor(_name)
#define F_DELETE(_class, _name) 			_class##_dtor(_name); free(_name)


#if (ENABLE_DEBUG)
static const char strLog[][10] = {{"VERBOSE"}, {"TRACE"}, {"DEBUG"}, {"INFOR"}, {"ERROR"}};
#define F_LOG(level, format, ...) \
	do {\
    	if (level >= F_LOG_LEVEL)\
    	{\
			fprintf(stderr, "[%s In function (%s)] " format, \
				strLog[level], __func__, ##__VA_ARGS__); \
    	}\
    } while (0)
#else
#define F_LOG(level, format, ...)
#endif

#if (ENABLE_ASSERT)
#define F_ASSERT(_expression, _message) \
	do {\
		if (!(_expression)) {\
			F_LOG(F_LOG_ERROR, "F_ASSERT:(%s):%s\nExiting...\n", #_expression, _message);\
			exit(1);\
		}\
	}while (0)
#else
#define F_ASSERT(_expression, _message)
#endif

#ifdef __cplusplus
}
#endif

#endif //__INCLUDES_H
