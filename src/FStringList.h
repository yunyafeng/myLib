
#ifndef __STRINGLIST_H__
#define __STRINGLIST_H__

#include <stdio.h>

#include "includes.h"

#ifdef __cplusplus
extern "C" {
#endif

//ǰ�������ڲ��ṹ
struct f_string_list_private;

//�ַ����б���
typedef struct f_string_list 
{
	struct f_string_list_private* d;
} FStringList;



/*
 *PUBLIC:
 */
//�൱�ڹ��캯��
void  FStringList_ctor		(FStringList* me);
//�൱����������
void  FStringList_dtor		(FStringList* me);
//Ϊһ��StringList ����һ���Ŀռ䣬�����Ż�Ƶ�����ڴ����
BOOL  FStringList_reserved	(FStringList* me, U32 count,  U32 bytes);
//����StringList ���ַ����ĸ���
U32   FStringList_count		(FStringList *me);
//��ȡָ�������ϵ��ַ���(char*)
char* FStringList_atIndex	(FStringList *me, U32 index);
//���StringList����׼��
void  FStringList_output	(FStringList* me, FILE *stream);
//��StringList��β��׷��һ���ַ���
BOOL  FStringList_pushBack	(FStringList* me, const char *string);
//����β�����ַ���
BOOL  FStringList_popBack	(FStringList* me);


/*
 *PUBLIC STATIC:
 */
//�൱����ľ�̬����
//�����ָ��ַ�����������һ��StringList,��StringList��Ҫ�ֶ�����
//StringList_dtor�������ͷ��ڴ�
//tips:�ú������޸�Դ�ַ���str������
FStringList FStringList_stringTok(char* str, const char *delim);


#ifdef __cplusplus
}
#endif

#endif //__STRINGLIST_H__