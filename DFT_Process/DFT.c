#include "main.h"
#if flagLPF
void LPF(Double* blkReal, Double* blkImag, Int wid, Int hei)
{
	// ���Ϳ��� LPF ���� 

	int x, y;
	int tempx, tempy;
	int halfcols, halfrows; 
	double butterworth, coordinate;
	halfcols = hei / 2;
	halfrows = wid / 2; // cutoff ã�� ���� 1/2 �� ����

	for (y = 0; y < hei; y++)
	{
		if (y >= halfcols)
			tempy = y - halfcols;
		else
			tempy = y + halfcols; //DC�κ����� y��ǥ �Ÿ� ����

		for (x = 0; x < wid; x++)
		{

			if (x >= halfrows)
				tempx = x - halfrows;
			else
				tempx = x + halfrows; //DC�κ����� x��ǥ �Ÿ� ����

			coordinate = sqrt(pow((double)(tempx - halfcols), 2.0) + pow((double)(tempy - halfrows), 2.0)); // DC�κ����� �Ÿ�
			butterworth = 1.0 / (1.0 + pow(coordinate / D0, 2 * N0)); //���Ϳ��� ������ ũ�� ���
			
			
			blkReal[y * wid + x] *= butterworth;
			blkImag[y * wid + x] *= butterworth; // butterworth low pass filter ���� -> frequency ���� �͵�(= DC�κ��� �Ÿ� �� �͵�) ���ֱ�
			
			
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

	blkReal = (Double*)calloc(blkSize * blkSize, sizeof(Double)); // �Ǽ�, ���, ũ��, ���� �κ� ���� �Ҵ�
	blkImag = (Double*)calloc(blkSize * blkSize, sizeof(Double)); //��� ũ�� blkSize * blkSize ũ��� ���� �Ҵ�
	blkMag = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	blkPha = (Double*)calloc(blkSize * blkSize, sizeof(Double));

#if TransType // 512*512 �� ���
	Int count = 0;
	Double* cosTable, * sinTable;
	FILE* fp, * op;

	cosTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double));
	sinTable = (Double*)calloc(WIDTH * HEIGHT * 2, sizeof(Double));


	fopen_s(&fp, "cosTable.txt", "rb");
	fopen_s(&op, "sinTable.txt", "rb");

	fread(cosTable, sizeof(double), WIDTH * HEIGHT * 2, fp);
	fread(sinTable, sizeof(double), WIDTH * HEIGHT * 2, op); // �ð� ������ ���� cos, sin ���̺� ��������

	fclose(fp);
	fclose(op);
#endif // 8*8�� ���

	for (Int i = 0; i < blkSize; i++) //i,j: time ������. k,l: frequency ������
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
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * cos(2 * PI * (i * k + j * l) / (double)blkSize); // �Ǽ��� ���
					blkImag[i * blkSize + j] -= curBlk[k * blkSize + l] * sin(2 * PI * (i * k + j * l) / (double)blkSize);// ����� ���

#endif
				}
			}
			blkReal[i * blkSize + j] = blkReal[i * blkSize + j] / (blkSize * blkSize);
			blkImag[i * blkSize + j] = blkImag[i * blkSize + j] / (blkSize * blkSize);
	
		}
	}


#if TransType

#if flagLPF // flagLPF=1 �̸�
	LPF(blkReal, blkImag, WIDTH, HEIGHT); // Low pass filter
#endif
#endif

	for (Int i = 0; i < blkSize; i++)
	{
		for (Int j = 0; j < blkSize; j++)
		{
			blkMag[i * blkSize + j] = sqrt(blkReal[i * blkSize + j] * blkReal[i * blkSize + j] + blkImag[i * blkSize + j] * blkImag[i * blkSize + j]); // ũ�� ���ϱ�
			blkPha[i * blkSize + j] = atan2(blkImag[i * blkSize + j], blkReal[i * blkSize + j]); // ���� ���ϱ�
		}
	}

	for (Int i = 0; i < blkSize; i++)      //����� ���� �ִ� ���� ��ġ�� ����
	{
		for (Int j = 0; j < blkSize; j++)
		{
			DFT->picReal     [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkReal[i * blkSize + j];
			DFT->picImag     [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkImag[i * blkSize + j];
			DFT->picMagnitude[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkMag[i * blkSize + j];
			DFT->picPhase    [(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkPha[i * blkSize + j];
		}
	}
	// ����Ʈ�� ���ϱ�
#if TransType

	Int wid = WIDTH;
	Int hei = HEIGHT;
	
	Double C, Temp, Spec;
	Double* SpecTmp;
	UChar* Shuffling;

	SpecTmp   = (Double*)calloc(blkSize * blkSize, sizeof(Double));
	Shuffling = (UChar*)calloc(blkSize * blkSize, sizeof(UChar));

	if (blkSize == wid && blkSize == hei) // ���ļ� ���� ������ �о ������ �ٿ��� ���� �α� ����
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

		// ���ø� -> ��Ī�� �̿��� ������ ���͸�
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
		
		fwrite(Shuffling, sizeof(UChar), blkSize * blkSize, cp); // ����Ʈ�� ���

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
	Int blkSize = BLOCK_SIZE; //��� ������ 8 or 512
	Int wid = WIDTH; Int hei = HEIGHT; // ���� ������
	Int min = minVal; Int max = maxVal;
	Int picSize = wid * hei; //���� ������
	Int stride = wid;

	UChar* TL_curBlk; // ����� ���� �� ��ǥ
	UChar* TMP;
	UChar* curBlk; //��� ��ȯ ����  

	DFT->picReal      = (Double*)calloc(picSize, sizeof(Double)); //�Ǽ��κ�, ����κ�, ũ��, ���� �޸� �Ҵ� 
	DFT->picImag      = (Double*)calloc(picSize, sizeof(Double)); //���� ������ ��ŭ �Ҵ�
	DFT->picMagnitude = (Double*)calloc(picSize, sizeof(Double));
	DFT->picPhase     = (Double*)calloc(picSize, sizeof(Double));

#if TransType == 0
	Int count = 0;
#endif
	//DFT
	curBlk = (UChar*)calloc(blkSize * blkSize, sizeof(UChar)); // ��� ���� �Ҵ�

	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)    //����� ���� �� ��ǥ�� ã�� ���� �ݺ���
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)//����� ���� �� ��ǥ�� ã�� ���� �ݺ���
		{
			memset(curBlk, 0, sizeof(UChar) * blkSize * blkSize);
			///////////////////////////////////////////////////////////////////////////////////////////
			TL_curBlk = img->ori +(blkRow* blkSize)*stride + (blkCol* blkSize);         // ����� ���� �� ��ǥ
			for (int k = 0; k < blkSize; k++)         //����� ���� �� ��ǥ�� �������� �� blkSize x blkSize ����
				for (int l = 0; l < blkSize; l++)
				{
					TMP = TL_curBlk + (k * stride + l);
					curBlk[k * blkSize + l] = TMP[0]; 
				}//curBlk�� blkSize x blkSize �� ȭ�� �� ������� -> DFT �� �ֱ�

			DFT_Func(curBlk, blkSize, blkRow, blkCol, DFT); //curBlk: ���� ���(blkSize x blkSize)
#if TransType == 0
			count += (blkSize * blkSize);// ���� DFT�� �󸶳� ���� �Ǿ����� �˷��ִ� ��
			printf("DFT : %.2f %%\n", (double)count/(wid*hei)*100);
#endif
		}
	}
	free(curBlk);
}