#include "main.h"

void IDFT_Func(Double* blkMag, Double* blkPha, Int blkSize, Int blkRow, Int blkCol, Img_Buf* img)
{ 
	Double PI = pi;
	Int stride = WIDTH;

	Double* DFT_Real;
	Double* DFT_Imag;
	Double Recon_R;

	DFT_Real = (Double*)calloc(blkSize * blkSize, sizeof(Double)); 
	DFT_Imag = (Double*)calloc(blkSize * blkSize, sizeof(Double));

#if TransType //512 경우
	Int count = 0;
	Double* cosTable, * sinTable;
	FILE* fp, * op;

	cosTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double));
	sinTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double)); // 계산 시간 줄이기 위해 sin, cos table 만들어놓음


	fopen_s(&fp, "cosTable.txt", "rb");
	fopen_s(&op, "sinTable.txt", "rb");

	fread(cosTable, sizeof(double), WIDTH * HEIGHT * 2, fp);
	fread(sinTable, sizeof(double), WIDTH * HEIGHT * 2, op);

	fclose(fp);
	fclose(op);
#endif //8의 경우

	for (Int i = 0; i < blkSize; i++)// 크기와 위상으로 실수부와 허수부 구해주기
	{
		for (Int j = 0; j < blkSize; j++)
		{
			DFT_Real[i * blkSize + j] = blkMag[i * blkSize + j] * cos(blkPha[i * blkSize + j]);
			DFT_Imag[i * blkSize + j] = blkMag[i * blkSize + j] * sin(blkPha[i * blkSize + j]);
		}
	}

	for (Int i = 0; i < blkSize; i++)
	{
#if TransType 
		printf("IDFT : %.2f %%\n", (++count) / (double)(WIDTH / blkSize * HEIGHT) * 100);
#endif
		for (Int j = 0; j < blkSize; j++)
		{
			Recon_R = 0;
			for (Int k = 0; k < blkSize; k++)
			{
				for (Int l = 0; l < blkSize; l++)
				{
#if TransType //512
					Recon_R += DFT_Real[k * blkSize + l] * cosTable[i * k + j * l] -
						DFT_Imag[k * blkSize + l] * sinTable[i * k + j * l]; 
#else //8
					Recon_R += DFT_Real[k * blkSize + l] * cos(2 * PI * (i * k + j * l) / (double)blkSize) -
						DFT_Imag[k * blkSize + l] * sin(2 * PI * (i * k + j * l) / (double)blkSize);
#endif
				}
			}

			if (Recon_R < 0)
				Recon_R = (int)(Recon_R - 0.5);

			else
				Recon_R = (int)(Recon_R + 0.5); //클리핑

			img->rec[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = CLIP(Recon_R); //각각 복원된 것을 복원영상에 넣어줌
		}
	}

	free(DFT_Real);
	free(DFT_Imag);
#if TransType
	free(cosTable);
	free(sinTable);
#endif

}

void IDFT_Process(Img_Buf* img, DFT_Val* DFT)
{
	Int blkSize = BLOCK_SIZE;
	Int wid = WIDTH; Int hei = HEIGHT;
	Int stride = wid;

	// 실수부분에서 크기와 페이즈만 갖고 옴

	Double* TL_blkMag;
	Double* Mag_TMP;
	Double* blkMag;

	Double* TL_blkPha;
	Double* Pha_TMP;
	Double* blkPha;

	

#if TransType == 0
	Int count = 0;
#endif
	blkMag = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	blkPha = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)
		{
			memset(blkMag, 0, sizeof(Double) * blkSize * blkSize);
			memset(blkPha, 0, sizeof(Double) * blkSize * blkSize); 
			// 픽처 단위 -> 블록단위로 다시 바꿔줌
			TL_blkMag = DFT->picMagnitude + (blkRow * blkSize) * stride + (blkCol * blkSize); 
			TL_blkPha = DFT->picPhase     + (blkRow * blkSize) * stride + (blkCol * blkSize);
			for (int k = 0; k < blkSize; k++)
				for (int l = 0; l < blkSize; l++)
				{
					Mag_TMP = TL_blkMag + (k * stride + l);
					blkMag[k * blkSize + l] = Mag_TMP[0]; // 크기를 블록 단위로 가져오기

					Pha_TMP = TL_blkPha + (k * stride + l);
					blkPha[k * blkSize + l] = Pha_TMP[0]; // 위상을 블록단위로 가져오기
				}

			IDFT_Func(blkMag, blkPha, blkSize, blkRow, blkCol, img); 
#if TransType == 0
			count += (blkSize * blkSize);
			printf("IDFT : %.2f %%\n", (double)count / (wid * hei) * 100);
#endif
		}
	}
	free(blkMag);
	free(blkPha);

	free(DFT->picMagnitude);
	free(DFT->picPhase);
	free(DFT->picReal);
	free(DFT->picImag);
}