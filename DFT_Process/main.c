#include "main.h"

void ImageCreate(Img_Buf *img, Int picSize) // 이미지 불러오기 함수
{
	FILE* fp;

	fopen_s(&fp, "lena_512x512.raw", "rb"); //원본 영상 열기
	img->ori = (UChar*)malloc(sizeof(UChar) * picSize); //원본 영상 크기만큼 공간 선언
	img->rec = (UChar*)malloc(sizeof(UChar) * picSize); //결과 영상 크기만큼 공간 선언

	memset(img->ori, 0, sizeof(UChar) * picSize); //0으로 초기화
	fread(img->ori, sizeof(UChar), picSize, fp); // 원본 영상 읽기(원본 영상의 픽셀 값을 배열 변수에 저장)

	fclose(fp);
}

void ImageOutput(Img_Buf* img, Int picSize) //이미지 출력 함수
{
	FILE* fp;

	fopen_s(&fp, "Rec_DFT_LPF.raw", "wb"); //결과 영상 파일 열기

	//파일 이름 저장 형식
	//512*512 , LPF x : Rec_DFT.raw
	//512*512 , LPF o : Rec_DFT_LPF.raw
	// 8*8 : Rec_DFT_8.raw
	
	fwrite(img->rec, sizeof(UChar), picSize, fp); // 원본 영상 읽기(원본 영상의 픽셀 값을 배열 변수에 저장)

	fclose(fp);
	free(img->ori);
	free(img->rec);
}

void main()
{
	
	Img_Buf img; //원본 영상과 복원 영상 버퍼를 잡아주는 버퍼가 들어있음

	Int wid = WIDTH; Int hei = HEIGHT;
	Int min = minVal; Int max = maxVal;

	Int picSize = wid * hei; //영상 사이즈


	DFT_Val DFT; // DFT 버퍼
	clock_t start, end; // 시간을 세기 위한 변수
	float res;

	ImageCreate(&img, picSize);

	start = clock(); // 시간 (초) 세기 시작
	DFT_Process (&img, &DFT); // Forward DFT
	IDFT_Process(&img, &DFT); // inverse DFT
	PSNR(&img); // 생겨난 원본 영상과 복원 영상의 MSE 와 PSNR을 계산함
	end = clock(); // 시간 세기 끝
	res = (float)(end - start) / CLOCKS_PER_SEC; // 기록된 시간 계산
	ImageOutput(&img, picSize); // 이미지 출력

	printf("\nDFT 소요된 시간 : %.3f \n", res);
	printf("start = %d \n", start);
	printf("end   = %d \n", end);

	printf("\n\n\n");

	
}

void PSNR(Img_Buf* img)
{
	Int i, j;
	Int wid = WIDTH; Int hei = HEIGHT;
	Double mse = 0, psnr = 0, max = 0; //mse : 평균제곱오차, psnr: 새로 생긴 영상에 대한 손실 정보를 갖고 있음 ( 손실이 적을수록 높은 값을 가짐 )
	UChar Img1 = 0, Img2 = 0;

	for (i = 0; i < hei; i++)
	{
		for (j = 0; j < wid; j++)
		{
			Img1 = img->ori[i * wid + j];
			Img2 = img->rec[i * wid + j];

			mse += ((Img1 - Img2) * (Img1 - Img2)); // 오차 값 제곱 누적
			if (Img1 > max)
				max = Img1;
		}
	}

	mse = mse / (wid * hei); // 평균 구함 
	printf("MSE : %lf\n", mse);
	psnr = 20 * log10(max / sqrt(mse));// 무손실 영상의 경우 ( = mse 가 0이면 ) 계산 할 수 없음 -> 무한대
	printf("PSNR : %lf\n", psnr); // 

	if (mse == 0)
		printf("\n영상 일치\n"); // 오차 0 이면 일치
	else
		printf("\n영상 불일치\n"); // 오차가 0 보다 크면 불일치
}