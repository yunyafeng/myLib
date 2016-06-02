
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
	F_REQUIRE(me);

	FImgLoader_ctor(&me->super, (FImageLoad)FBmpLoader_load, 
		            (FImgLoaderDtor)FBmpLoader_dtor);
}

static void FBmpLoader_dtor(FBmpLoader* me)
{
	F_REQUIRE(me);

	if (me->super.image) {
		FImg_dtor(me->super.image);
		F_DELETE(me->super.image);
	}
	FImgLoader_dtor(&me->super);
}

static BOOL FBmpLoader_load(FBmpLoader* me, const char* imgFile)
{
	F_REQUIRE(me);

	BmpFh fh;
	BmpIh ih;
	U8 *lineBuffer;
	FILE* inFile;
	
	I32 err = 0;

	do {
	    /* ���Դ�λͼ�ļ� */
	    inFile = fopen(imgFile, "r");
		if (NULL == inFile) {
			err = -1;
			me->super.error = FIMGLOADER_IOERROR;
			break;
		}

	    /* ���Զ�ȡλͼ�ļ�ͷ����Ϣͷ */
	    fseek(inFile, 0, SEEK_SET);
	    if ((14 != fread(&fh.fileType, 1, 14, inFile)) || 
	        (40 != fread(&ih, 1, 40, inFile))) {
			err = -2;
			me->super.error = FIMGLOADER_IOERROR;
			break;
		} 

	    /* ���ͼ���ļ���ʽ�Ƿ�Ϊbitmap�ļ� */
	    if (fh.fileType != 0x4d42) {
			err = -3;
			me->super.error = FIMGLOADER_UNSUPPORTFMT;
			break;
		} 

		/*  ����Ƿ�֧�ֵ�ǰ��BMP��ʽ */
		if ((ih.infoSize != sizeof(BmpIh)) || 
	        (ih.bitCount != 16 && ih.bitCount != 24 && ih.bitCount != 32) || 
	        ih.compression) {
			err = -4;
			me->super.error = FIMGLOADER_UNSUPPORTBMP;
			break;
	    }

		/* ����IMAGE */
		if (NULL != me->super.image) {
			FImg_dtor(me->super.image);
		}
		else {
			me->super.image = F_NEW(FImg);
		}
		FImg_ctor(me->super.image, ih.bitCount, ih.imgWidth, ih.imgHeight);
		if (!FImg_isValid(me->super.image)) {
			err = -5;
			me->super.error = FIMGLOADER_TOOLARGE;
			break;
		}

	    /* �ֱ����BMP��IMG��ÿ��ͼ�����ݳ��ȣ�ע��BMP��32λ�����IMG��8λ���� */
	    U32 bmpLineSize = f_round_up(ih.imgWidth * ih.bitCount, 32) / 8;
	    U32 imgLineSize = f_round_up(ih.imgWidth * ih.bitCount, 8) / 8;
	    U8* imgData = FImg_data(me->super.image);
		
		/* ָ�����һ��(ע��Ŀǰֻ֧�ֵ���λͼ����BMP�����ڴ�ֱ����ת) */
	    imgData += imgLineSize * (ih.imgHeight - 1);
	    /* Ϊ���ж�ȡͼ�����ݳ��Կ����л��� */
	    lineBuffer = F_NEWARR(U8, bmpLineSize);
		if (NULL == lineBuffer) {
			err = -6;
			me->super.error = FIMGLOADER_OUTOFMEMORY;
			break;
		}

		/* ���ж�ȡBMPͼ������,д��image�� */
		fseek(inFile, fh.dataOffset, SEEK_SET);
		U32 h;
	    for (h = 0; h < ih.imgHeight; h++) {
	       	int len = fread(lineBuffer, 1, bmpLineSize, inFile);
	        memcpy(imgData, lineBuffer, imgLineSize);
	        imgData -= imgLineSize;
			len++;
		}
	} while (0);

	/* ������ */
	switch (err) {
	case -6:
	case -5:
	case -4:
	case -3:
	case -2:
		fclose(inFile);
	case -1:
		return FALSE;
		break;
	}
	
    /* �ͷ���Դ */
	F_DELETE(lineBuffer);
	fclose(inFile);
	me->super.error = FIMGLOADER_SUCCESS;

	return TRUE;
}


/**
 * �����ṩ�ӿ�
 */
FImgLoader *FBmpLoader_create()
{
	FBmpLoader *bmpLoader = F_NEW(FBmpLoader);
	FBmpLoader_ctor(bmpLoader);
	return FImgLoaderStar_cast(bmpLoader);
}
 
