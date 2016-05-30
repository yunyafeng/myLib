
#include "FBmpLoader.h"
#include "FBitmap.h"


//BMP ������ �̳��Լ�����
typedef struct f_bmp_loader {
	FImgLoader 	super;
} FBmpLoader;


/**
 * �ڲ�����
 */
static void FBmpLoader_ctor(FBmpLoader* me);
static void FBmpLoader_dtor(FBmpLoader* me);
static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile);


/**
 * ����'��'Bmp�������Ľӿ�ʵ��
 */ 
static void FBmpLoader_ctor(FBmpLoader* me)
{
	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile)
{
	BmpFh fh;
	BmpIh ih;
	U32 h;

    /* ���Դ�λͼ�ļ� */
    FILE* fp = fopen(imgFile, "r");
	if (NULL == fp) {
		return FALSE;
	}

    /* ���Զ�ȡλͼ�ļ�ͷ����Ϣͷ */
    fseek(fp, 0, SEEK_SET);
    if ((14 != fread(&fh.fileType, 1, 14, fp)) || 
        (40 != fread(&ih, 1, 40, fp))) {
		fclose(fp);
		return FALSE;
	} 

    /* ���λͼ�ļ���ʽ */
    if ((fh.fileType != 0x4d42) || 
        (ih.infoSize != sizeof(BmpIh)) || 
        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
        ih.compression) {
		fclose(fp);
		return FALSE;
    }

    /* �ֱ����BMP��IMG��ÿ��ͼ�����ݳ��ȣ�ע��BMP��32λ�����IMG��8λ���� */
    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;

	/* ����IMAGE */
	if (NULL != me->super.image) {
		FImg_dtor(me->super.image);
	}
	else {
		me->super.image = F_NEW(FImg);
	}
	FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
    U8* imgData = FImg_data(me->super.image);
	
	/* ָ�����һ��(ע��Ŀǰֻ֧�ֵ���λͼ����BMP�����ڴ�ֱ����ת) */
    imgData += imgLineSize * (ih.imgHeight - 1);
    /* Ϊ���ж�ȡͼ�����ݳ��Կ����л��� */
    U8 *bmpLine = F_NEWARR(U8, bmpLineSize);
	/* ���ж�ȡBMPͼ������,д��image�� */
	fseek(fp, fh.dataOffset, SEEK_SET);
    for (h = 0; h < ih.imgHeight; h++) {
       	int len = fread(bmpLine, 1, bmpLineSize, fp);
        memcpy(imgData, bmpLine, imgLineSize);
        imgData -= imgLineSize;
		len++;
    }
	
    /* �ͷ��л��� */
    F_DELETE(bmpLine);
	fclose(fp);

	return TRUE;
}


/**
 * �����ṩ�ӿ�
 */
FImgLoader *FBmpLoader_create()
{
	FBmpLoader *bmpLoader = F_NEW(FBmpLoader);
	FBmpLoader_ctor(bmpLoader);
	return (FImgLoader*)bmpLoader;
}
 