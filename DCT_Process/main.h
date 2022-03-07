#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#define _CRT_SECURE_NO_WARNINGS

#define WIDTH      512              // 영상의 가로 크기
#define HEIGHT     512              // 영상의 세로 크기

#define maxVal    255
#define minVal    0

#define pi        3.141592653589793238462643383279

#define CLIP(x) (x < minVal) ? minVal : x > maxVal ? maxVal : x

#define BLOCK_SIZE   512                                                     //(본인이 수정해야함)
#if BLOCK_SIZE == 512
#define TransType 1 //1이면 현재 블록은 512x512, 0이면 그 이외의 블록
#else
#define TransType 0
#endif


typedef unsigned char UChar;
typedef char          Char;
typedef double		  Double;
typedef int           Int;


typedef struct _DCT_Buffer
{
	Double* imgDCT; //DCT가 적용된 결과 저장 버퍼( 실수 값만 존재 )
}DCT_Val;

typedef struct _Image_Buffer
{
	UChar* ori; //원본 영상 저장을 위한 변수 선언
	UChar* rec; //복원 영상 저장

}Img_Buf;

void DCT_Process(Img_Buf* img, DCT_Val* DCT);
void DCT_Func(UChar* curBlk, Int blkSize, Int blkRow, Int blkCol, DCT_Val* DCT);

void IDCT_Process(Img_Buf* img, DCT_Val* DCT);
void IDCT_Func(Double* curBlk, Int blkSize, Int blkRow, Int blkCol, Img_Buf *img);

void PSNR(Img_Buf* img);