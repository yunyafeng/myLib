#ifndef __FBITMAP_H__
#define __FBITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

//��Ӧ.bmp���ļ�ͷ�ṹ
typedef struct _bitmap_fileheader
{
    U16 reseved;      	//����ַ������ڶ����ֱ߽�
    U16 fileType;      	//�ļ����ͣ�0x424D('BM')Ϊ�ļ��еĴ洢���У�С�˴洢
    U32 fileSize;      	//�ļ���С���ļ�ռ�õ��ֽڴ�С
    U16 reseved1;     	//������1��ֵ����Ϊ0
    U16 reseved2;     	//������2��ֵ����Ϊ0
    U32 dataOffset;    	//ƫ�������ļ�ͷ��ʵ��λͼ���ݵ�ƫ���������ֽ�Ϊ��λ
} BmpFh;

//��Ӧ.bmp����Ϣͷ�ṹ
typedef struct _bitmap_infoheader
{
    U32 infoSize;      	//��Ϣͷ��С������Ϣͷռ�õ��ֽڴ�С��ֵΪ40
    U32 imgWidth;      	//ͼ����
    U32 imgHeight;     	//ͼ��߶ȣ�Ŀǰֻ֧�ֵ���λͼ(���½�Ϊԭ��)
    U16 devPlanes;     	//Ŀ���豸λ������ֵ����Ϊ1
    U16 bitCount;      	//ÿ������ռ������
    U32 compression;   	//ѹ���㷨��Ŀǰֻ֧�ֲ�ѹ��ͼ��(BI_RGB��BI_RGB==0)
    U32 imgSize;       	//ͼ���С��ͼ��ռ�õ��ֽڴ�С����ֵ����Ϊ4�ı���
    U32 xRes;          	//ˮƽ�ֱ��ʣ���λΪ����/��
    U32 yRes;          	//��ֱ�ֱ��ʣ���λΪ����/��
    U32 clrUsed;       	//ʵ��ʹ�õ���ɫ��
    U32 crImportant;  	//��Ҫ����ɫ��
} BmpIh;

#ifdef __cplusplus
}
#endif

#endif //__FBITMAP_H__