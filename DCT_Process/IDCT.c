#include "main.h"

void IDCT_Func(Double* curBlk, Int blkSize, Int blkRow, Int blkCol, Img_Buf* img)
{
	Double PI = pi;
	Int stride = WIDTH;

	Double DCT_Real;
	Double Recon_R;

#if TransType
	Int count = 0;
	
	Double* DCTcosTable;
	FILE* fp, * op;

	DCTcosTable = (Double*)calloc((2 * WIDTH + 1) * HEIGHT, sizeof(Double)); //코사인 table


	fopen_s(&fp, "DCTcosTable.txt", "rb");

	fread(DCTcosTable, sizeof(double), (2 * WIDTH + 1) * HEIGHT, fp);


	fclose(fp);
#endif

	for (Int i = 0; i < blkSize; i++)
	{
#if TransType 
		printf("IDCT : %.2f %%\n", (++count) / (double)(WIDTH / blkSize * HEIGHT) * 100);
#endif
		for (Int j = 0; j < blkSize; j++)
		{
			Recon_R = 0;
			for (Int k = 0; k < blkSize; k++)
			{
				for (Int l = 0; l < blkSize; l++)
				{
#if TransType //512
					//i=x.j=y, k=u, l=v.
					DCT_Real = curBlk[k * blkSize + l] * DCTcosTable[(2 * i + 1) * k] * DCTcosTable[(2 * j + 1) * l];
#else//8
					DCT_Real = curBlk[k * blkSize + l] * cos((2 * i + 1) * k * PI / (2 * blkSize)) * cos((2 * j + 1) * l * PI / (2 * blkSize));//?
#endif
					//c(u),c(v) 구하기
					if (k == 0 && l == 0)
						Recon_R += DCT_Real / blkSize;
					else if (k == 0 || l == 0)
						Recon_R += (DCT_Real * (1.0 / sqrt(2.0))) / (blkSize / 2);
					else
						Recon_R += DCT_Real / (blkSize / 2);
				}
			}

			if (Recon_R < 0)//클리핑
				Recon_R = (int)(Recon_R - 0.5);

			else
				Recon_R = (int)(Recon_R + 0.5);

			img->rec[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = CLIP(Recon_R);
		}
	}

#if TransType
	free(DCTcosTable);
	
#endif

}

void IDCT_Process(Img_Buf* img, DCT_Val* DCT)
{
	Int blkSize = BLOCK_SIZE;
	Int wid = WIDTH; Int hei = HEIGHT;
	Int stride = wid;

	Double* TL_curBlk;
	Double* TMP;
	Double* curBlk;

#if TransType == 0
	Int count = 0;
#endif
	curBlk = (Double*)calloc(blkSize * blkSize, sizeof(Double));

	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)//블록의 왼쪽 위 좌표를 찾기 위한 반복문
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)
		{
			memset(curBlk, 0, sizeof(Double) * blkSize * blkSize);

			TL_curBlk = DCT->imgDCT + (blkRow * blkSize) * stride + (blkCol * blkSize); 

			for (int k = 0; k < blkSize; k++)
				for (int l = 0; l < blkSize; l++)
				{
					TMP = TL_curBlk + (k * stride + l);
					curBlk[k * blkSize + l] = TMP[0];
				}

			IDCT_Func(curBlk, blkSize, blkRow, blkCol, img);
#if TransType == 0
			count += (blkSize * blkSize);
			printf("IDCT : %.2f %%\n", (double)count / (wid * hei) * 100);
#endif
		}
	}
	free(curBlk);


	free(DCT->imgDCT);
}