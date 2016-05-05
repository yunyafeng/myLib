#include <string.h>
#include <stdlib.h>

#include "FStringList.h"

/*
 * private: 
 */
typedef struct f_string_list_private 
{
	U32 			count;
	
	U32				strPtrsSize;
	char**			strPtrs;

	U32				dataSize;
	U32				usedDataSize;
	char*			data;
} FStringListPrivate;


static void FStringListPrivate_ctor(FStringListPrivate* me)
{
	F_ASSERT(me, "");
	
	me->count = 0;
	me->strPtrsSize = 0;
	me->strPtrs = NULL;
	me->dataSize = 0;
	me->usedDataSize = 0;
	me->data = NULL;
}

static void FStringListPrivate_dtor(FStringListPrivate* me)
{
	F_ASSERT(me, "");
	
	F_FREE(me->strPtrs);
	F_FREE(me->data);
	me->count = 0;
	me->strPtrsSize = 0;
	me->strPtrs = NULL;
	me->dataSize = 0;
	me->usedDataSize = 0;
	me->data = NULL;
}

static U32 countChar(const char* str, const char* delim)
{
	U32 count = 0;
	while (*delim) {
		int c = *delim;
		char* next = (char*)strchr(str, c);
		while (next) {
			count ++;
			next = (char*)strchr(next+1, *delim);
		}
		delim++;
	}
	return count;
}

/*
 * public:
 */
void FStringList_ctor(FStringList* me)
{
	F_ASSERT(me, "");
	
	me->d = (FStringListPrivate*)F_MALLOC(sizeof(FStringListPrivate));
	FStringListPrivate_ctor(me->d);
}

void FStringList_dtor(FStringList* me)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	
	FStringListPrivate_dtor(me->d);
	F_FREE(me->d);
	me->d = NULL;
}

BOOL FStringList_reserved(FStringList* me, U32 count,  U32 bytes)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	//
	if (me->d->strPtrs || me->d->data)
		return FALSE;
	
	me->d->strPtrs = (char**)F_MALLOC(sizeof(char*) * count);
	if (me->d->strPtrs) {
		me->d->strPtrsSize = count;
		me->d->data = (char*)F_MALLOC(bytes);
		if (me->d->data) {
			me->d->dataSize = bytes;
			return TRUE;
		}
	}

	return FALSE;
}

U32 FStringList_count(FStringList *me)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	
	return me->d->count;
}

char* FStringList_indexOf(FStringList *me, U32 index)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	
	if ((index >= me->d->count) || (NULL == me->d->strPtrs)) {
		return NULL;
	}
	return me->d->strPtrs[index];
}

void FStringList_output(FStringList* me, FILE *stream)
{
	if (me && me->d) {
		fprintf(stream, "StringList(%p):\n[\n", me);
		U32 i = 0;
		for (;i < me->d->count; ++i) {
			fprintf(stream, "  (%d):\t%s\n", i, me->d->strPtrs[i]);
		}
		fprintf(stream, "]\n");
	}
	else {
		fprintf(stderr, "***Error:%s (me)Null pointer detected\n", __func__);
	}
}

BOOL FStringList_pushBack(FStringList* me, const char *string)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	
	if (me->d->count > me->d->strPtrsSize)
	{
		char** newPtrs = (char**)F_REALLOC(me->d->strPtrs, sizeof(char*)*(me->d->strPtrsSize+8));
		if (!newPtrs) {
			return FALSE;
		}
		me->d->strPtrs = newPtrs;
		me->d->strPtrsSize += 8;
	}
	
	U32 len = strlen(string) + 1;
	if (len > (me->d->dataSize - me->d->usedDataSize)) {
		//预留一定的空间后续使用时不需在分配空间
		char *newData = (char*)F_REALLOC(me->d->data, me->d->usedDataSize+len+512);
		if (!newData) {
			return FALSE;
		}
		me->d->data = newData;
		me->d->dataSize += len;
	}

	char *dest = me->d->data + me->d->usedDataSize;
	strcpy(dest, string);
	me->d->strPtrs[me->d->count] = dest;
	me->d->usedDataSize += len;
	me->d->count += 1;
	
	return TRUE;
}

BOOL FStringList_popBack(FStringList* me)
{
	F_ASSERT(me && me->d, "You must call the [FStringList_ctor] before use it");
	
	if (me->d->count == 0) {
		return FALSE;
	}

	//数量减少
	me->d->count -= 1;
	//如果预留空间过多，则释放多余空间(伸缩余量为8个指针空间)
	if ((me->d->count+8) < (me->d->strPtrsSize-32)) {
		char **newPtrs = (char**)F_REALLOC(me->d->strPtrs, sizeof(char*)*(me->d->strPtrsSize-32));
		me->d->strPtrs = newPtrs;
		me->d->strPtrsSize -= 32;
	}
	
	U32 len = strlen(me->d->strPtrs[me->d->count]) + 1;
	me->d->usedDataSize -= len;
	//同上(伸缩余量为512个字节)
	if ((me->d->usedDataSize + 512) < (me->d->dataSize - 512)) {
		char *newData = (char*)F_REALLOC(me->d->data, me->d->usedDataSize + 512);
		me->d->data = newData;
		me->d->dataSize = me->d->usedDataSize + 512;
	}
	return TRUE;
}

/*
 * public & static:
 */
FStringList FStringList_stringTok(char* str, const char *delim)
{
	FStringList result;
	FStringList_ctor(&result);

	U32 count = countChar(str, delim);
	U32 len = strlen(str) + 1;
	if (count == 0) {
		count = 1;
	}
	FStringList_reserved(&result, count, len);

	char* savePtr;
	char* token = strtok_r(str, delim, &savePtr);
	while (token) {
		FStringList_pushBack(&result, token);
		token = strtok_r(NULL, delim, &savePtr);
	}

	return result;
}
