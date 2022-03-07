#include "main.h"

void DCT_Func(UChar* curBlk, Int blkSize, Int blkRow, Int blkCol, DCT_Val* DCT)
{
	Double PI = pi;
	Int stride = WIDTH;

	Double* blkReal;

	blkReal = (Double*)calloc(blkSize * blkSize, sizeof(Double));  // 실수 부분만 존재. 메모리 할당

#if TransType
	Int count = 0;
	Double* DCTcosTable;
	FILE* fp, * op;

	DCTcosTable = (Double*)calloc((2 * WIDTH + 1) * HEIGHT, sizeof(Double));


	fopen_s(&fp, "DCTcosTable.txt", "rb");

	fread(DCTcosTable, sizeof(double), (2 * WIDTH + 1) * HEIGHT, fp);


	fclose(fp);

#endif

	for (Int i = 0; i < blkSize; i++)
	{
#if TransType 
		printf("DCT : %.2f %%\n", (++count) / (double)(WIDTH / blkSize * HEIGHT) * 100);
#endif
		for (Int j = 0; j < blkSize; j++)
		{
			for (Int k = 0; k < blkSize; k++)
			{
				for (Int l = 0; l < blkSize; l++)
				{
#if TransType //512
					// DCT 에서 구하는 것 : u,v. blkReal의 인덱스 : i * blkSize + j
					// 따라서 i = u, j = v, k = x, l = y 임
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * DCTcosTable[(2 * k + 1) * i] * DCTcosTable[(2 * l + 1) * j];
#else //8
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * cos((2 * k + 1) * i * PI / (2 * (double)blkSize)) * cos((2 * l + 1) * j * PI / (2 * (double)blkSize));
#endif
				}
			}
			//c(u),c(v) 구하기
			if (i == 0 && j == 0)
				blkReal[i * blkSize + j] /= blkSize;
			else if (i == 0 || j == 0)
				blkReal[i * blkSize + j] = (blkReal[i * blkSize + j] * (1.0 / sqrt(2.0))) / (blkSize / 2);
			else
				blkReal[i * blkSize + j] = blkReal[i * blkSize + j] / (blkSize / 2);
		}
	}

	for (Int i = 0; i < blkSize; i++)      //블록을 원래 있던 픽쳐 위치에 저장
	{
		for (Int j = 0; j < blkSize; j++)
		{
			DCT->imgDCT[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkReal[i * blkSize + j]; //DCT 결과값 저장
		}
	}

#if TransType
	Int wid = WIDTH;
	Int hei = HEIGHT;


	if (blkSize == wid && blkSize == hei)
	{
		UChar* spectrum;

		FILE* cp;
		fopen_s(&cp, "DCT_Spectrum.raw", "wb");

		spectrum = (UChar*)calloc(blkSize * blkSize, sizeof(UChar));
		for (Int i = 0; i < blkSize; i++)
		{
			for (Int j = 0; j < blkSize; j++)
			{
				spectrum[i * wid + j] = CLIP(DCT->imgDCT[i * wid + j]); //  클리핑
			}
		}
		
		fwrite(spectrum, sizeof(UChar), blkSize * blkSize, cp);

		free(spectrum);
		fclose(cp);
	}
#endif

	free(blkReal);

#if TransType
	free(DCTcosTable);
#endif

}

void DCT_Process(Img_Buf* img, DCT_Val* DCT)
{
	Int blkSize = BLOCK_SIZE;
	Int wid = WIDTH; Int hei = HEIGHT;
	Int min = minVal; Int max = maxVal;
	Int picSize = wid * hei; //영상 사이즈
	Int stride = wid;

	UChar* TL_curBlk;
	UChar* TMP;
	UChar* curBlk; //원본 영상을 블록 사이즈로 쪼개기 위해 함

	DCT->imgDCT = (Double*)calloc(picSize, sizeof(Double)); //DCT결과 임시 저장
	

#if TransType == 0
	Int count = 0;
#endif
	//DCT
	curBlk = (UChar*)calloc(blkSize * blkSize, sizeof(UChar));
	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)    //블록의 왼쪽 위 좌표를 찾기 위한 반복문
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)//블록의 왼쪽 위 좌표를 찾기 위한 반복문
		{
			memset(curBlk, 0, sizeof(UChar) * blkSize * blkSize);
			///////////////////////////////////////////////////////////////////////////////////////////
			TL_curBlk = img->ori + (blkRow * blkSize) * stride + (blkCol * blkSize);                   //블록의 왼쪽 위 좌표
			for (int k = 0; k < blkSize; k++)         //블록의 왼쪽 위 좌표를 기준으로 한 blkSize x blkSize 추출
				for (int l = 0; l < blkSize; l++)
				{
					TMP = TL_curBlk + (k * stride + l);
					curBlk[k * blkSize + l] = TMP[0];
				}

			DCT_Func(curBlk, blkSize, blkRow, blkCol, DCT); //curBlk: 현재 블록(blkSize x blkSize)
#if TransType == 0
			count += (blkSize * blkSize);
			printf("DCT : %.2f %%\n", (double)count / (wid * hei) * 100);
#endif
		}
	}
	free(curBlk);
}