#include "FImage.h"

//对应16位色结构
typedef struct _rgb16 {
	U16 B:5;
	U16	G:6;
	U16 R:5;
} TRGB16;

//对应24位色结构
typedef struct _rgb24 {
	U8 B, G, R;
} TRGB24;

//对应32位色结构
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


//返回24位图像的(x, y)的像素数据
#define Pixel(colorType, img, x, y) \
		((colorType*)((img)->d->data + (img)->d->lineSize * (y)) + (x))


//临近插值法缩放图像
static void FImg_ZoomNearestNeighbour(FImg *pDst, FImg *pSrc);
//双线性插值法缩放图像
static void FImg_ZoomBilinear(FImg *pDst, FImg *pSrc);
////双三次插值算法（Bicubic interpolation）
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
	
	//TODO...
	switch (me->d->depth) 
	{
	case FIMG_RGB888:
		break;
	case FIMG_ARGB8888:
		break;
	default:
		break;
	}

	return FALSE;
}

BOOL FImg_toRGB888(FImg* me)
{	
	if (me->d->depth == FIMG_RGB888)
		return TRUE;
	//TODO...
	
	return FALSE;
}

BOOL FImg_toARGB8888(FImg* me)
{
	if (me->d->depth == FIMG_ARGB8888)
		return TRUE;
	//TODO...
	
	return FALSE;
}

//缩放图像
void FImg_resize(FImg* me, F32 wScale, F32 hScale, U32 zoomHint)
{
	F_ASSERT(me && me->d, "You must call the [FImg_ctor] before use it");
	if (me->d->data ==  NULL)
		return;
	U32 w = me->d->width * wScale;
	U32 h = me->d->height * hScale;

	FImg newImg;
	FImg_ctor(&newImg, me->d->depth, w, h);

	switch (zoomHint) 
	{
	case FIMG_SPEED_PRIORITY:	//速度优先，使用临近插值法
		FImg_ZoomNearestNeighbour(&newImg, me);
		break;
	case FIMG_QUALITY_PRIORITY:	//质量优先，双三次插值算法
		FImg_ZoomBicubic(&newImg, me);
		break;
	case FIMG_BALANCED:			//均衡，双线性插值
	default:
		FImg_ZoomBilinear(&newImg, me);
		break;
	}

	FImg_dtor(me);
	me->d = newImg.d;
}



// 最近像素插值算法（Nearest Neighbour interpolation）
// x' = xscale * x + xsheer * y + dx
// y' = yscale * y + ysheer * x + dy
// 该函数使用16.16定点数优化
static void FImg_ZoomNearestNeighbour16(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> 16;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> 16;
			(*Pixel(TRGB16, pDst, x, y)) = (*Pixel(TRGB16, pSrc, sx, sy));
		}
	}
}

static void FImg_ZoomNearestNeighbour24(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> 16;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> 16;
			(*Pixel(TRGB24, pDst, x, y)) = (*Pixel(TRGB24, pSrc, sx, sy));
		}
	}
}

static void FImg_ZoomNearestNeighbour32(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;

	for (y = 0; y < pDst->d->height; ++y) {	
		sy = (y * yRatio) >> 16;	
		for (x = 0; x < pDst->d->width; ++x) {			
			sx = (x * xRatio) >> 16;
			(*Pixel(TRGB32, pDst, x, y)) = (*Pixel(TRGB32, pSrc, sx, sy));
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


//双线性插值（Bilinear interpolation）
// f(x, y) = (1-u)*(1-v)*f(x1,y1) + (1-u)*v*f(x1,y2) + u*(1-v)*f(x2, y1) + u*v*f(x2, y2)
// x2 = x1 + 1, y2 = y1 + 1
// 该函数使用16.16定点数优化
static void FImg_ZoomBilinear16(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;
	U32 one = 1 << 16;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & 0x0000FFFF; 		//小数部分
		y1 &= 0xFFFF0000;			//整数部分
		y1 >>= 16;					//坐标值
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & 0x0000FFFF; 	//小数部分
			x1 &= 0xFFFF0000;		//整数部分
			x1 >>= 16;				//坐标值
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB16 p1 = *(Pixel(TRGB16, pSrc, x1, y1));
			TRGB16 p2 = *(Pixel(TRGB16, pSrc, x1, y2));
			TRGB16 p3 = *(Pixel(TRGB16, pSrc, x2, y1));
			TRGB16 p4 = *(Pixel(TRGB16, pSrc, x2, y2));
			
			U32 a = (one - u) * (one - v) >> 16;
			U32 b = (one - v) * u >> 16;
			U32 c = (one - u) * v >> 16;
			U32 d = v * u >> 16;	

			TRGB16 target;

			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> 16;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> 16;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> 16;

			*Pixel(TRGB16, pDst, x, y) = target;
		}
	}
}

static void FImg_ZoomBilinear24(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;
	U32 one = 1 << 16;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & 0x0000FFFF; 		//小数部分
		y1 &= 0xFFFF0000;			//整数部分
		y1 >>= 16;					//坐标值
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & 0x0000FFFF; 	//小数部分
			x1 &= 0xFFFF0000;		//整数部分
			x1 >>= 16;				//坐标值
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB24 p1 = *(Pixel(TRGB24, pSrc, x1, y1));
			TRGB24 p2 = *(Pixel(TRGB24, pSrc, x1, y2));
			TRGB24 p3 = *(Pixel(TRGB24, pSrc, x2, y1));
			TRGB24 p4 = *(Pixel(TRGB24, pSrc, x2, y2));
			
			U32 a = (one - u) * (one - v) >> 16;
			U32 b = (one - v) * u >> 16;
			U32 c = (one - u) * v >> 16;
			U32 d = v * u >> 16;	

			TRGB24 target;

			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> 16;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> 16;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> 16;

			*Pixel(TRGB24, pDst, x, y) = target;
		}
	}
}

static void FImg_ZoomBilinear32(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 x1, y1, x2, y2;
	U32 u, v;
	
	U32 xRatio = (pSrc->d->width << 16) / pDst->d->width + 1;
	U32 yRatio = (pSrc->d->height << 16) / pDst->d->height + 1;
	U32 one = 1 << 16;

	for (y = 0; y < pDst->d->height; ++y) {
		y1 = (y * yRatio);
		u = y1 & 0x0000FFFF; 		//小数部分
		y1 &= 0xFFFF0000;			//整数部分
		y1 >>= 16;					//坐标值
		y2 = (y == (pDst->d->height - 1)) ? y1 : y1 + 1;
		for (x = 0; x < pDst->d->width; ++x) {
			x1 = (x * xRatio);
			v = x1 & 0x0000FFFF; 	//小数部分
			x1 &= 0xFFFF0000;		//整数部分
			x1 >>= 16;				//坐标值
			x2 = (x == (pDst->d->width- 1)) ? x1 : x1 + 1;
	
			TRGB32 p1 = *(Pixel(TRGB32, pSrc, x1, y1));
			TRGB32 p2 = *(Pixel(TRGB32, pSrc, x1, y2));
			TRGB32 p3 = *(Pixel(TRGB32, pSrc, x2, y1));
			TRGB32 p4 = *(Pixel(TRGB32, pSrc, x2, y2));
			
			U32 a = (one - u) * (one - v) >> 16;
			U32 b = (one - v) * u >> 16;
			U32 c = (one - u) * v >> 16;
			U32 d = v * u >> 16;	

			TRGB32 target;

			target.R = (a * p1.R + b * p2.R + c * p3.R + d * p4.R) >> 16;
			target.G = (a * p1.G + b * p2.G + c * p3.G + d * p4.G) >> 16;
			target.B = (a * p1.B + b * p2.B + c * p3.B + d * p4.B) >> 16;
			target.A = (a * p1.A + b * p2.A + c * p3.A + d * p4.A) >> 16;
			
			*Pixel(TRGB32, pDst, x, y) = target;
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


//双三次插值算法（Bicubic interpolation）
static void FImg_ZoomBicubic(FImg *pDst, FImg *pSrc)
{
	U32 x, y;
	U32 sx, sy; 

	if (pDst->d->depth != pSrc->d->depth)
		return;

}

