#include "FImage.h"

//��Ӧ16λɫ�ṹ
typedef struct _rgb16 {
	U16 B:5;
	U16	G:6;
	U16 R:5;
} TRGB16;

//��Ӧ24λɫ�ṹ
typedef struct _rgb24 {
	U8 B, G, R;
} TRGB24;

//��Ӧ32λɫ�ṹ
typedef struct _rgb32 {
	U8 B, G, R, A;
} TRGB32;

		
typedef struct f_image_private 
{
	U32 headerBytes;
	U32 width;
	U32 height;
	U32 depth;
	U32 lineSize;
	U32 imgBytes;
	U8* data;
} FImgPrivate;


/*
 *
 * ����������� 17.15�ķ�ʽ�Ż�
 * Ҫ�ڳ���˽������ʾ����ֵ�ķ�Χ�Լ����п������������ʹ��
 * �ڱ�������С��λ���Ϊ15λ �� 17.15
 */
#define  DecBits			(15)			//С��λռ�õ�λ��
#define  IntMask			(0xFFFF8000)	//������������
#define  DecMask			(0x00007FFF)	//С����������
#define  One				(1 << DecBits)	//����֮���1


//����ͼ���(x, y)����������
#define Pixel(colorType, img, x, y) \
		((colorType*)((img)->d->data + (img)->d->lineSize * (y)) + (x))



//�ٽ���ֵ������ͼ��
static void FImg_ZoomNearestNeighbour(FImg *pDst, FImg *pSrc);
//˫���Բ�ֵ������ͼ��
static void FImg_ZoomBilinear(FImg *pDst, FImg *pSrc);
////˫���β�ֵ�㷨��Bicubic interpolation��
static void FImg_ZoomBicubic(FImg *pDst, FImg *pSrc);



void FImg_ctor(FImg* me, U32 depth, U32 w, U32 h)
{
	U32 imgSize, totalBytes, lineSize;

	lineSize = f_round_up(w * depth, 8) / 8;

	imgSize = lineSize * h;
	totalBytes = sizeof(FImgPrivate) + imgSize;
	me->d = (FImgPrivate*)malloc(totalBytes);
	me->d->headerBytes = sizeof(FImgPrivate);
	me->d->imgBytes = imgSize;
	me->d->width = w;
	me->d->height = h;
	me->d->depth = depth;
	me->d->lineSize = lineSize;
	if (totalBytes > sizeof(FImgPrivate)) {
		me->d->data = (U8*)(me->d) + sizeof(FImgPrivate);
	}
	else {
		me->d->data = NULL;
	}
}

FImg FImg_copy(FImg* me)
{
	FImg copy;
	FImg_ctor(&copy, me->d->depth, me->d->width, me->d->height);
	if (me->d->data != NULL) {
		FImg_setData(&copy, me->d->data);
	}

	return copy;
}

void FImg_dtor(FImg* me)
{
	free(me->d);
	me->d = NULL;
}

U32 FImg_width(FImg* me)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	return me->d->width;
}

U32 FImg_height(FImg* me)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	return me->d->height;
}

U32 FImg_depth(FImg* me)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	return me->d->depth;
}

U8* FImg_data(FImg* me)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	return me->d->data;
}

void FImg_setData(FImg* me, U8* data)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	memcpy(me->d->data, data, me->d->imgBytes);
}

BOOL FImg_isValid(FImg* me)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	return me->d->data != NULL;
}

BOOL FImg_toRGB565(FImg* me)
{
	if (me->d->depth == FIMG_RGB565)
		return TRUE;
	
	if (me->d->data != NULL) {
		U32 x, y;
		FImg newImg;
		FImg_ctor(&newImg, FIMG_RGB565, me->d->width, me->d->height);
		TRGB16* dData = (TRGB16*)newImg.d->data;

		/* ����Դ��λ��Ƚ���ת�� */
		switch (me->d->depth) 
		{
		case FIMG_RGB888: {
			TRGB24* sData = (TRGB24*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ16λ(�ֱ�ض�RGB����) */
					dData->B = sData->B >> 3;
					dData->G = sData->G >> 2;
					dData->R = sData->R >> 3;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		case FIMG_ARGB8888: {
			TRGB32* sData = (TRGB32*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ16λ(�ֱ�ض�RGB����������alpha����) */
					dData->B = sData->B >> 3;
					dData->G = sData->G >> 2;
					dData->R = sData->R >> 3;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		default:
			break;
		}
	}

	return FALSE;
}

BOOL FImg_toRGB888(FImg* me)
{	
	if (me->d->depth == FIMG_RGB888)
		return TRUE;
	
	if (me->d->data != NULL) {
		U32 x, y;
		FImg newImg;
		FImg_ctor(&newImg, FIMG_RGB888, me->d->width, me->d->height);
		TRGB24* dData = (TRGB24*)newImg.d->data;

		/* ����Դ��λ��Ƚ���ת�� */
		switch (me->d->depth) 
		{
		case FIMG_RGB565: {
			TRGB16* sData = (TRGB16*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ24λ(��չRGB����,��λ��0) */
					dData->B = sData->B << 3;
					dData->G = sData->G << 2;
					dData->R = sData->R << 3;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		case FIMG_ARGB8888: {
			TRGB32* sData = (TRGB32*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ24λ(����alpha����) */
					dData->B = sData->B;
					dData->G = sData->G;
					dData->R = sData->R;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		default:
			break;
		}
	}	
	return FALSE;
}

BOOL FImg_toARGB8888(FImg* me)
{
	if (me->d->depth == FIMG_ARGB8888)
		return TRUE;
	
	if (me->d->data != NULL) {
		U32 x, y;
		FImg newImg;
		FImg_ctor(&newImg, FIMG_ARGB8888, me->d->width, me->d->height);
		TRGB32* dData = (TRGB32*)newImg.d->data;

		/* ����Դ��λ��Ƚ���ת�� */
		switch (me->d->depth) 
		{
		case FIMG_RGB565: {
			TRGB16* sData = (TRGB16*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ32λ(��չRGB����,��λ��0,alpha����Ĭ��Ϊ0) */
					dData->B = sData->B << 3;
					dData->G = sData->G << 2;
					dData->R = sData->R << 3;
					dData->A =  0;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		case FIMG_RGB888: {
			TRGB24* sData = (TRGB24*)me->d->data;
			for (y = 0; y < me->d->height; ++y) {
				for (x = 0; x < me->d->width; ++x) {

					/* ת��Ϊ32λ(alpha����Ĭ��Ϊ0) */
					dData->B = sData->B;
					dData->G = sData->G;
					dData->R = sData->R;
					dData->A =  0;

					//��һ�����ص�
					dData += 1;
					sData += 1;
				}
			}
			FImg_dtor(me);
			*me = newImg;
			return TRUE;
		}
		default:
			break;
		}
	}	
	
	return FALSE;
}

void FImg_output(FImg* me, FILE* stream)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	fprintf(stream, "Image(%p):", me);
	fprintf(stream, "[Size(%d, %d), Depth(%d)]\n", me->d->width, me->d->height, me->d->depth);
}

//����ͼ��
void FImg_resize(FImg* me, F32 wScale, F32 hScale, U32 zoomHint)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	if (me->d->data ==  NULL)
		return;
	U32 w = me->d->width * wScale;
	U32 h = me->d->height * hScale;

	FImg newImg;
	FImg_ctor(&newImg, me->d->depth, w, h);

	if (FImg_isValid(&newImg)) {
		switch (zoomHint) 
		{
		case FIMG_SPEED_PRIORITY:	//�ٶ����ȣ�ʹ���ٽ���ֵ��
			FImg_ZoomNearestNeighbour(&newImg, me);
			break;
		case FIMG_QUALITY_PRIORITY:	//�������ȣ�˫���β�ֵ�㷨
			FImg_ZoomBicubic(&newImg, me);
			break;
		case FIMG_BALANCED:			//���⣬˫���Բ�ֵ
		default:
			FImg_ZoomBilinear(&newImg, me);
			break;
		}
	}

	FImg_dtor(me);
	*me = newImg;
}



// ������ز�ֵ�㷨��Nearest Neighbour interpolation��
// x' = xscale * x + xsheer * y + dx
// y' = yscale * y + ysheer * x + dy
// �ú���ʹ��17.15�������Ż�
static void FImg_ZoomNearestNeighbour16(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height;
	TRGB16* dData = (TRGB16*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> DecBits;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> DecBits;
			*dData++ = (*Pixel(TRGB16, pSrc, sx, sy));
			//(*Pixel(TRGB16, pDst, x, y)) = (*Pixel(TRGB16, pSrc, sx, sy));
		}
	}
}

static void FImg_ZoomNearestNeighbour24(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height;
	TRGB24* dData = (TRGB24*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> DecBits;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> DecBits;
			*dData++ = (*Pixel(TRGB24, pSrc, sx, sy));
			//(*Pixel(TRGB24, pDst, x, y)) = (*Pixel(TRGB24, pSrc, sx, sy));
		}
	}
}

static void FImg_ZoomNearestNeighbour32(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height;
	TRGB32* dData = (TRGB32*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> DecBits;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> DecBits;
			*dData++ = (*Pixel(TRGB32, pSrc, sx, sy));
			//(*Pixel(TRGB32, pDst, x, y)) = (*Pixel(TRGB32, pSrc, sx, sy));
		}
	}
}

static void FImg_ZoomNearestNeighbour(FImg *pDst, FImg *pSrc)
{
	if (pDst->d->depth != pSrc->d->depth)
		return;

	switch (pDst->d->depth) 
	{
	case FIMG_RGB565:
		FImg_ZoomNearestNeighbour16(pDst, pSrc);
		break;
	case FIMG_RGB888:
		FImg_ZoomNearestNeighbour24(pDst, pSrc);
		break;
	case FIMG_ARGB8888:
		FImg_ZoomNearestNeighbour32(pDst, pSrc);
		break;
	default:
		break;
	}			
}


//˫���Բ�ֵ��Bilinear interpolation��
// f(x, y) = (1-u)*(1-v)*f(x1,y1) + (1-u)*v*f(x1,y2) + u*(1-v)*f(x2, y1) + u*v*f(x2, y2)
// x2 = x1 + 1, y2 = y1 + 1
// �ú���ʹ��17.15�������Ż�
static void FImg_ZoomBilinear16(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height;
	TRGB16* dData = (TRGB16*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & DecMask; 				//С������
		y1 &= IntMask;					//��������
		y1 >>= DecBits;					//����ֵ
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & DecMask; 			//С������
			x1 &= IntMask;				//��������
			x1 >>= DecBits;				//����ֵ
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB16 p1 = *(Pixel(TRGB16, pSrc, x1, y1));
			TRGB16 p2 = *(Pixel(TRGB16, pSrc, x1, y2));
			TRGB16 p3 = *(Pixel(TRGB16, pSrc, x2, y1));
			TRGB16 p4 = *(Pixel(TRGB16, pSrc, x2, y2));
			
			U32 a = (One - u) * (One - v) >> DecBits;
			U32 b = (One - v) * u >> DecBits;
			U32 c = (One - u) * v >> DecBits;
			U32 d = v * u >> DecBits;	

			
			dData->R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			dData->G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			dData->B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;
			dData += 1;

			/*
			TRGB16 target;
			
			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;

			*Pixel(TRGB16, pDst, x, y) = target;
			*/
		}
	}
}


static void FImg_ZoomBilinear24(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width ;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height ;
	TRGB24* dData = (TRGB24*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & DecMask; 				//С������
		y1 &= IntMask;					//��������
		y1 >>= DecBits;					//����ֵ
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & DecMask; 			//С������
			x1 &= IntMask;				//��������
			x1 >>= DecBits;				//����ֵ
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB24 p1 = *(Pixel(TRGB24, pSrc, x1, y1));
			TRGB24 p2 = *(Pixel(TRGB24, pSrc, x1, y2));
			TRGB24 p3 = *(Pixel(TRGB24, pSrc, x2, y1));
			TRGB24 p4 = *(Pixel(TRGB24, pSrc, x2, y2));
			
			U32 a = (One - u) * (One - v) >> DecBits;
			U32 b = (One - v) * u >> DecBits;
			U32 c = (One - u) * v >> DecBits;
			U32 d = v * u >> DecBits;	
			
			dData->R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			dData->G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			dData->B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;
			dData += 1;
			/*
			TRGB24 target;

			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;

			*Pixel(TRGB24, pDst, x, y) = target;
			*/
		}
	}
}

static void FImg_ZoomBilinear32(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << DecBits) / pDst->d->width;
	U32 yRatio = (pSrc->d->height << DecBits) / pDst->d->height;
	TRGB32* dData = (TRGB32*)pDst->d->data;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & DecMask; 				//С������
		y1 &= IntMask;					//��������
		y1 >>= DecBits;					//����ֵ
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & DecMask; 			//С������
			x1 &= IntMask;				//��������
			x1 >>= DecBits;				//����ֵ
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB32 p1 = *(Pixel(TRGB32, pSrc, x1, y1));
			TRGB32 p2 = *(Pixel(TRGB32, pSrc, x1, y2));
			TRGB32 p3 = *(Pixel(TRGB32, pSrc, x2, y1));
			TRGB32 p4 = *(Pixel(TRGB32, pSrc, x2, y2));
			
			U32 a = (One - u) * (One - v) >> DecBits;
			U32 b = (One - v) * u >> DecBits;
			U32 c = (One - u) * v >> DecBits;
			U32 d = v * u >> DecBits;

			
			dData->R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			dData->G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			dData->B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;
			dData->A = (a * p1.A + b * p2.A + c * p3.A + d * p4.A) >> DecBits;
			dData += 1;
			/*
			TRGB32 target;

			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> DecBits;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> DecBits;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> DecBits;
			target.A = (a * p1.A + b * p2.A + c * p3.A + d * p4.A) >> DecBits;
			
			*Pixel(TRGB32, pDst, x, y) = target;
			*/
		}
	}
}

static void FImg_ZoomBilinear(FImg *pDst, FImg *pSrc)
{
	if (pDst->d->depth != pSrc->d->depth)
		return;

	switch (pDst->d->depth) 
	{
	case FIMG_RGB565:
		FImg_ZoomBilinear16(pDst, pSrc);
		break;
	case FIMG_RGB888:
		FImg_ZoomBilinear24(pDst, pSrc);
		break;
	case FIMG_ARGB8888:
		FImg_ZoomBilinear32(pDst, pSrc);
		break;
	default:
		break;
	}
}


//˫���β�ֵ�㷨��Bicubic interpolation��
static void FImg_ZoomBicubic(FImg *pDst, FImg *pSrc)
{
	//U32 x, y;
	//U32 sx, sy; 

	if (pDst->d->depth != pSrc->d->depth)
		return;

}

