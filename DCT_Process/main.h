#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#define _CRT_SECURE_NO_WARNINGS

#define WIDTH      512              // ������ ���� ũ��
#define HEIGHT     512              // ������ ���� ũ��

#define maxVal    255
#define minVal    0

#define pi        3.141592653589793238462643383279

#define CLIP(x) (x < minVal) ? minVal : x > maxVal ? maxVal : x

#define BLOCK_SIZE   512                                                     //(������ �����ؾ���)
#if BLOCK_SIZE == 512
#define TransType 1 //1�̸� ���� ����� 512x512, 0�̸� �� �̿��� ���
#else
#define TransType 0
#endif


typedef unsigned char UChar;
typedef char          Char;
typedef double		  Double;
typedef int           Int;


typedef struct _DCT_Buffer
{
	Double* imgDCT; //DCT�� ����� ��� ���� ����( �Ǽ� ���� ���� )
}DCT_Val;

typedef struct _Image_Buffer
{
	UChar* ori; //���� ���� ������ ���� ���� ����
	UChar* rec; //���� ���� ����

}Img_Buf;

void DCT_Process(Img_Buf* img, DCT_Val* DCT);
void DCT_Func(UChar* curBlk, Int blkSize, Int blkRow, Int blkCol, DCT_Val* DCT);

void IDCT_Process(Img_Buf* img, DCT_Val* DCT);
void IDCT_Func(Double* curBlk, Int blkSize, Int blkRow, Int blkCol, Img_Buf *img);

void PSNR(Img_Buf* img);