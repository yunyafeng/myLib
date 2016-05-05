#ifndef __INCLUDES_H
#define __INCLUDES_H

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
	
#define LOG_LEVEL							4
#define ENABLE_DEBUG						1
#define ENABLE_ASSERT						1

#define LOG_VERBO							0
#define LOG_TRACE							1
#define LOG_DEBUG							2
#define LOG_INFO							3
#define LOG_ERROR							4


static inline void* Malloc(U32 size)
{
	void *ptr = malloc(size);
	printf("##malloc:(%p)\n", ptr);
	return ptr;
}

static inline void Free(void *ptr)
{
	printf("##free:(%p)\n", ptr);
	free(ptr);
}

#define F_MALLOC(_size) 					Malloc(_size)
#define F_REALLOC(_ptr, _size) 				realloc(_ptr, _size)
#define F_FREE(_ptr) 						Free(_ptr)

#define F_DECLAR(_class, _name) 			_class _name; _class##_ctor(&_name)
#define F_DDECLAR(_class, _name) 			_class##_dtor(&_name)

#define F_NEW(_class, _name) 				_class* _name = (_class*)F_MALLOC(sizeof(_class)); \
											_class##_ctor(_name)
#define F_DELETE(_class, _name) 			_class##_dtor(_name); F_FREE(_name)


#if (ENABLE_DEBUG)
static const char strLog[][10] = {{"VERBOSE"}, {"TRACE"}, {"DEBUG"}, {"INFOR"}, {"ERROR"}};
#define LOG(level, format, ...) \
	do {\
    	if (level >= LOG_LEVEL)\
    	{\
			fprintf(stderr, "[%s In function (%s)] " format, \
				strLog[level], __func__, ##__VA_ARGS__); \
    	}\
    } while (0)
#else
#define LOG(level, format, ...)
#endif

#if (ENABLE_ASSERT)
#define F_ASSERT(_expression, _message) \
	do {\
		if (!(_expression)) {\
			LOG(LOG_ERROR, "F_ASSERT:(%s):%s\nExiting...\n", #_expression, _message);\
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
