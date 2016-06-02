#ifndef __IMAGELOADER_H__
#define __IMAGELOADER_H__

#include "FIncludes.h"
#include "FImage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ������ */
typedef enum f_img_loader_error_code 
{
	FIMGLOADER_SUCCESS,				//�ɹ�
	FIMGLOADER_IOERROR,				//�ļ�IO���� fopen fread fwrite...
	FIMGLOADER_UNSUPPORTFMT,		//��֧�ֵ��ļ���ʽ
	FIMGLOADER_UNSUPPORTBMP,		//��֧�ֵ�bmp�ļ�
	FIMGLOADER_UNSUPPORTJPG,		//��֧�ֵ�jpeg�ļ�
	FIMGLOADER_UNSUPPORTPNG,		//��֧�ֵ�png�ļ�
	FIMGLOADER_TOOLARGE,			//ͼ��̫��
	FIMGLOADER_OUTOFMEMORY,			//�ڴ治��
	FIMGLOADER_UNKOWNERR,			//δ֪����
} FImgLoaderErrCode;


#define FImgLoaderStar_cast(_objPtr) 	((FImgLoader*)(_objPtr))
#define FImgLoader_cast(_obj) 			((FImgLoader)(_obj))


struct f_img_loader;


/* ����load����ӿ� */
typedef BOOL (*FImageLoad)(struct f_img_loader*, const char*);

/* �������� */
typedef void (*FImgLoaderDtor)(struct f_img_loader*);

/* ͼ��������������'��' */
typedef struct f_img_loader {
	FImageLoad 			load;		//���غ���
	FImgLoaderDtor 		dtor;		//��������
	FImg*				image;		//������ɵ�ͼ������
	FImgLoaderErrCode 	error;		//������
} FImgLoader;


void FImgLoader_ctor(FImgLoader* me, FImageLoad load, FImgLoaderDtor dtor);
void FImgLoader_dtor(FImgLoader* me);


/* ����һ��ͼ������� */
FImgLoader* FImgLoader_create(void);

/* ����һ��ͼ������� */
void FImgLoader_destroy(FImgLoader* me);

/* �õ�������ɵ�ͼ������ */
FImg* FImgLoader_image(FImgLoader* me);

/* �õ�������Ϣ */
const char* FImgLoader_errorMessage(FImgLoader* me);

/* �õ������� */
FImgLoaderErrCode FImgLoader_errorCode(FImgLoader* me);

#ifdef __cplusplus
}
#endif
#endif //__IMAGELOADER_H__
