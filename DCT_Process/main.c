#include "main.h"

void ImageCreate(Img_Buf* img, Int picSize) //이미지 열기
{
	FILE* fp;

	fopen_s(&fp, "lena_512x512.raw", "rb"); //원본 영상 열기
	img->ori = (UChar*)malloc(sizeof(UChar) * picSize); //원본 영상 크기만큼 공간 선언
	img->rec = (UChar*)malloc(sizeof(UChar) * picSize); //결과 영상 크기만큼 공간 선언

	memset(img->ori, 0, sizeof(UChar) * picSize); //0으로 초기화
	fread(img->ori, sizeof(UChar), picSize, fp); // 원본 영상 읽기(원본 영상의 픽셀 값을 배열 변수에 저장)

	fclose(fp);
}

void ImageOutput(Img_Buf* img, Int picSize) // 이미지 출력
{
	FILE* fp;

	fopen_s(&fp, "Rec_DCT_512.raw", "wb"); //결과 영상 파일 열기

	// 8*8 : Rec_DCT_8.raw
	// 512*512 : Rec_DCT_512.raw

	fwrite(img->rec, sizeof(UChar), picSize, fp); // 원본 영상 읽기(원본 영상의 픽셀 값을 배열 변수에 저장)

	fclose(fp);
	free(img->ori);
	free(img->rec);
}

void main()
{

	Img_Buf img;

	Int wid = WIDTH; Int hei = HEIGHT;
	Int min = minVal; Int max = maxVal;

	Int picSize = wid * hei; //영상 사이즈


	DCT_Val DCT;
	clock_t start, end;
	float res;

	ImageCreate(&img, picSize);

	start = clock(); //시간 기록 시작
	DCT_Process(&img, &DCT);
	IDCT_Process(&img, &DCT);
	PSNR(&img);
	end = clock(); //시간 기록 끝
	res = (float)(end - start) / CLOCKS_PER_SEC; // 시간 기록
	ImageOutput(&img, picSize);

	printf("\nDCT 소요된 시간 : %.3f \n", res);
	printf("start = %d \n", start);
	printf("end   = %d \n", end);

	printf("\n\n\n");


}

void PSNR(Img_Buf* img) //DFT 참고
{
	Int i, j;
	Int wid = WIDTH; Int hei = HEIGHT;
	Double mse = 0, psnr = 0, max = 0;
	UChar Img1 = 0, Img2 = 0;

	for (i = 0; i < hei; i++)
	{
		for (j = 0; j < wid; j++)
		{
			Img1 = img->ori[i * wid + j];
			Img2 = img->rec[i * wid + j];

			mse += ((Img1 - Img2) * (Img1 - Img2));
			if (Img1 > max)
				max = Img1;
		}
	}

	mse = mse / (wid * hei);
	printf("MSE : %lf\n", mse);
	psnr = 20 * log10(max / sqrt(mse));
	printf("PSNR : %lf\n", psnr);

	if (mse == 0)
		printf("\n영상 일치\n");
	else
		printf("\n영상 불일치\n");
}