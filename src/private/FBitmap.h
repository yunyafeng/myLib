#ifndef __FBITMAP_H__
#define __FBITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

//对应.bmp的文件头结构
typedef struct _bitmap_fileheader
{
    U16 reseved;      	//填充字符，用于对齐字边界
    U16 fileType;      	//文件类型，0x424D('BM')为文件中的存储序列，小端存储
    U32 fileSize;      	//文件大小，文件占用的字节大小
    U16 reseved1;     	//保留字1，值必须为0
    U16 reseved2;     	//保留字2，值必须为0
    U32 dataOffset;    	//偏移量，文件头到实际位图数据的偏移量，以字节为单位
} BmpFh;

//对应.bmp的信息头结构
typedef struct _bitmap_infoheader
{
    U32 infoSize;      	//信息头大小，该信息头占用的字节大小，值为40
    U32 imgWidth;      	//图像宽度
    U32 imgHeight;     	//图像高度，目前只支持倒向位图(左下角为原点)
    U16 devPlanes;     	//目标设备位面数，值必须为1
    U16 bitCount;      	//每像素所占比特数
    U32 compression;   	//压缩算法，目前只支持不压缩图像(BI_RGB，BI_RGB==0)
    U32 imgSize;       	//图像大小，图像占用的字节大小，其值必须为4的倍数
    U32 xRes;          	//水平分辨率，单位为像素/米
    U32 yRes;          	//垂直分辨率，单位为像素/米
    U32 clrUsed;       	//实际使用的颜色数
    U32 crImportant;  	//重要的颜色数
} BmpIh;

#ifdef __cplusplus
}
#endif

#endif //__FBITMAP_H__