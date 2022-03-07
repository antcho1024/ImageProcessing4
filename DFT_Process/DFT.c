#include "main.h"
#if flagLPF
void LPF(Double* blkReal, Double* blkImag, Int wid, Int hei)
{
	// 버터워스 LPF 구현 

	int x, y;
	int tempx, tempy;
	int halfcols, halfrows; 
	double butterworth, coordinate;
	halfcols = hei / 2;
	halfrows = wid / 2; // cutoff 찾기 위해 1/2 값 구함

	for (y = 0; y < hei; y++)
	{
		if (y >= halfcols)
			tempy = y - halfcols;
		else
			tempy = y + halfcols; //DC로부터의 y좌표 거리 구함

		for (x = 0; x < wid; x++)
		{

			if (x >= halfrows)
				tempx = x - halfrows;
			else
				tempx = x + halfrows; //DC로부터의 x좌표 거리 구함

			coordinate = sqrt(pow((double)(tempx - halfcols), 2.0) + pow((double)(tempy - halfrows), 2.0)); // DC로부터의 거리
			butterworth = 1.0 / (1.0 + pow(coordinate / D0, 2 * N0)); //버터워스 필터의 크기 계산
			
			
			blkReal[y * wid + x] *= butterworth;
			blkImag[y * wid + x] *= butterworth; // butterworth low pass filter 적용 -> frequency 높은 것들(= DC로부터 거리 먼 것들) 없애기
			
			
		}

	}
}
#endif
void DFT_Func(UChar* curBlk, Int blkSize, Int blkRow, Int blkCol, DFT_Val* DFT)
{
	Double PI = pi;
	Int stride = WIDTH;

	Double* blkReal;
	Double* blkImag;
	Double* blkMag;
	Double* blkPha;

	blkReal = (Double*)calloc(blkSize * blkSize, sizeof(Double)); // 실수, 허수, 크기, 위상 부분 공간 할당
	blkImag = (Double*)calloc(blkSize * blkSize, sizeof(Double)); //블록 크기 blkSize * blkSize 크기로 공간 할당
	blkMag = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	blkPha = (Double*)calloc(blkSize * blkSize, sizeof(Double));

#if TransType // 512*512 의 경우
	Int count = 0;
	Double* cosTable, * sinTable;
	FILE* fp, * op;

	cosTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double));
	sinTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double));


	fopen_s(&fp, "cosTable.txt", "rb");
	fopen_s(&op, "sinTable.txt", "rb");

	fread(cosTable, sizeof(double), WIDTH * HEIGHT * 2, fp);
	fread(sinTable, sizeof(double), WIDTH * HEIGHT * 2, op); // 시간 단축을 위해 cos, sin 테이블 만들어놓음

	fclose(fp);
	fclose(op);
#endif // 8*8의 경우

	for (Int i = 0; i < blkSize; i++) //i,j: time 도메인. k,l: frequency 도메인
	{
#if TransType 
		printf("DFT : %.2f %%\n", (++count) / (double)(WIDTH / blkSize * HEIGHT) * 100);
#endif
		for (Int j = 0; j < blkSize; j++)
		{
			for (Int k = 0; k < blkSize; k++)
			{
				for (Int l = 0; l < blkSize; l++)
				{
#if TransType 
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * cosTable[i * k + j * l];
					blkImag[i * blkSize + j] -= curBlk[k * blkSize + l] * sinTable[i * k + j * l];
#else
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * cos(2 * PI * (i * k + j * l) / (double)blkSize); // 실수부 계산
					blkImag[i * blkSize + j] -= curBlk[k * blkSize + l] * sin(2 * PI * (i * k + j * l) / (double)blkSize);// 허수부 계산

#endif
				}
			}
			blkReal[i * blkSize + j] = blkReal[i * blkSize + j] / (blkSize * blkSize);
			blkImag[i * blkSize + j] = blkImag[i * blkSize + j] / (blkSize * blkSize);
	
		}
	}


#if TransType

#if flagLPF // flagLPF=1 이면
	LPF(blkReal, blkImag, WIDTH, HEIGHT); // Low pass filter
#endif
#endif

	for (Int i = 0; i < blkSize; i++)
	{
		for (Int j = 0; j < blkSize; j++)
		{
			blkMag[i * blkSize + j] = sqrt(blkReal[i * blkSize + j] * blkReal[i * blkSize + j] + blkImag[i * blkSize + j] * blkImag[i * blkSize + j]); // 크기 구하기
			blkPha[i * blkSize + j] = atan2(blkImag[i * blkSize + j], blkReal[i * blkSize + j]); // 위상 구하기
		}
	}

	for (Int i = 0; i < blkSize; i++)      //블록을 원래 있던 픽쳐 위치에 저장
	{
		for (Int j = 0; j < blkSize; j++)
		{
			DFT->picReal     [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkReal[i * blkSize + j];
			DFT->picImag     [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkImag[i * blkSize + j];
			DFT->picMagnitude[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkMag[i * blkSize + j];
			DFT->picPhase    [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkPha[i * blkSize + j];
		}
	}
	// 스펙트럼 구하기
#if TransType

	Int wid = WIDTH;
	Int hei = HEIGHT;
	
	Double C, Temp, Spec;
	Double* SpecTmp;
	UChar* Shuffling;

	SpecTmp   = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	Shuffling = (UChar*)calloc(blkSize * blkSize, sizeof(UChar));

	if (blkSize == wid && blkSize == hei) // 주파수 동작 범위가 넓어서 스케일 다운을 위해 로그 취함
	{
		C = hypot(DFT->picReal[(blkRow * blkSize) + (blkCol * (wid + (wid % blkSize)) * blkSize)], DFT->picImag[(blkRow * blkSize) + (blkCol * (wid + (wid % blkSize)) * blkSize)]);
		for (Int i = 0; i < blkSize; i++)
		{
			Spec = 0;
			for (Int j = 0; j < blkSize; j++)
			{
				Temp = hypot(DFT->picReal[(blkRow * blkSize) + (blkCol * (wid + (wid % blkSize)) * blkSize) + ((wid + (wid % blkSize)) * i) + j], DFT->picImag[(blkRow * blkSize) + (blkCol * (wid + (wid % blkSize)) * blkSize) + ((wid + (wid % blkSize)) * i) + j]);
				Spec = (C * log(1.0 + abs(Temp))) < 0.0 ? 0.0 : (C * log(1.0 + abs(Temp))) > 255.0 ? 255.0 : (C * log(1.0 + abs(Temp)));
				SpecTmp[i * blkSize + j] = Spec;
			}
		}

		// 셔플링 -> 대칭을 이용해 에너지 센터링
		for (Int i = 0; i < blkSize; i += (blkSize / 2))
		{
			for (Int j = 0; j < blkSize; j += (blkSize / 2))
			{
				for (Int k = 0; k < (blkSize / 2); k++)
				{
					for (Int l = 0; l < (blkSize / 2); l++)
					{
						Shuffling[wid * (k + i) + (l + j)] = (UChar)SpecTmp[wid * (255 - k + i) + (255 - l + j)];
					}
				}
			}
		}
		FILE* cp;
		fopen_s(&cp, "DFT_Spectrum.raw", "wb");
		
		fwrite(Shuffling, sizeof(UChar), blkSize * blkSize, cp); // 스펙트럼 출력

		free(SpecTmp);
		free(Shuffling);

		fclose(cp);
	}

	
#endif



	free(blkReal);
	free(blkImag);
	free(blkMag);
	free(blkPha);

#if TransType
	free(cosTable);
	free(sinTable);
#endif
	
}

void DFT_Process(Img_Buf* img, DFT_Val* DFT)
{
	Int blkSize = BLOCK_SIZE; //블록 사이즈 8 or 512
	Int wid = WIDTH; Int hei = HEIGHT; // 영상 사이즈
	Int min = minVal; Int max = maxVal;
	Int picSize = wid * hei; //영상 사이즈
	Int stride = wid;

	UChar* TL_curBlk; // 블록의 왼쪽 위 좌표
	UChar* TMP;
	UChar* curBlk; //블록 반환 변수  

	DFT->picReal      = (Double*)calloc(picSize, sizeof(Double)); //실수부분, 허수부분, 크기, 위상 메모리 할당 
	DFT->picImag      = (Double*)calloc(picSize, sizeof(Double)); //영상 사이즈 만큼 할당
	DFT->picMagnitude = (Double*)calloc(picSize, sizeof(Double));
	DFT->picPhase     = (Double*)calloc(picSize, sizeof(Double));

#if TransType == 0
	Int count = 0;
#endif
	//DFT
	curBlk = (UChar*)calloc(blkSize * blkSize, sizeof(UChar)); // 블록 공간 할당

	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)    //블록의 왼쪽 위 좌표를 찾기 위한 반복문
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)//블록의 왼쪽 위 좌표를 찾기 위한 반복문
		{
			memset(curBlk, 0, sizeof(UChar) * blkSize * blkSize);
			///////////////////////////////////////////////////////////////////////////////////////////
			TL_curBlk = img->ori +(blkRow* blkSize)*stride + (blkCol* blkSize);         // 블록의 왼쪽 위 좌표
			for (int k = 0; k < blkSize; k++)         //블록의 왼쪽 위 좌표를 기준으로 한 blkSize x blkSize 추출
				for (int l = 0; l < blkSize; l++)
				{
					TMP = TL_curBlk + (k * stride + l);
					curBlk[k * blkSize + l] = TMP[0]; 
				}//curBlk에 blkSize x blkSize 의 화소 값 들어있음 -> DFT 에 넣기

			DFT_Func(curBlk, blkSize, blkRow, blkCol, DFT); //curBlk: 현재 블록(blkSize x blkSize)
#if TransType == 0
			count += (blkSize * blkSize);// 현재 DFT가 얼마나 진행 되었는지 알려주는 것
			printf("DFT : %.2f %%\n", (double)count/(wid*hei)*100);
#endif
		}
	}
	free(curBlk);
}