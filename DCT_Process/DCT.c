#include "main.h"

void DCT_Func(UChar* curBlk, Int blkSize, Int blkRow, Int blkCol, DCT_Val* DCT)
{
	Double PI = pi;
	Int stride = WIDTH;

	Double* blkReal;

	blkReal = (Double*)calloc(blkSize * blkSize, sizeof(Double));  // �Ǽ� �κи� ����. �޸� �Ҵ�

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
					// DCT ���� ���ϴ� �� : u,v. blkReal�� �ε��� : i * blkSize + j
					// ���� i = u, j = v, k = x, l = y ��
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * DCTcosTable[(2 * k + 1) * i] * DCTcosTable[(2 * l + 1) * j];
#else //8
					blkReal[i * blkSize + j] += curBlk[k * blkSize + l] * cos((2 * k + 1) * i * PI / (2 * (double)blkSize)) * cos((2 * l + 1) * j * PI / (2 * (double)blkSize));
#endif
				}
			}
			//c(u),c(v) ���ϱ�
			if (i == 0 && j == 0)
				blkReal[i * blkSize + j] /= blkSize;
			else if (i == 0 || j == 0)
				blkReal[i * blkSize + j] = (blkReal[i * blkSize + j] * (1.0 / sqrt(2.0))) / (blkSize / 2);
			else
				blkReal[i * blkSize + j] = blkReal[i * blkSize + j] / (blkSize / 2);
		}
	}

	for (Int i = 0; i < blkSize; i++)      //����� ���� �ִ� ���� ��ġ�� ����
	{
		for (Int j = 0; j < blkSize; j++)
		{
			DCT->imgDCT[(blkRow * blkSize + i) * stride + (blkCol * blkSize + j)] = blkReal[i * blkSize + j]; //DCT ����� ����
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
				spectrum[i * wid + j] = CLIP(DCT->imgDCT[i * wid + j]); //  Ŭ����
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
	Int picSize = wid * hei; //���� ������
	Int stride = wid;

	UChar* TL_curBlk;
	UChar* TMP;
	UChar* curBlk; //���� ������ ��� ������� �ɰ��� ���� ��

	DCT->imgDCT = (Double*)calloc(picSize, sizeof(Double)); //DCT��� �ӽ� ����
	

#if TransType == 0
	Int count = 0;
#endif
	//DCT
	curBlk = (UChar*)calloc(blkSize * blkSize, sizeof(UChar));
	for (Int blkRow = 0; blkRow < hei / blkSize; blkRow++)    //����� ���� �� ��ǥ�� ã�� ���� �ݺ���
	{
		for (Int blkCol = 0; blkCol < wid / blkSize; blkCol++)//����� ���� �� ��ǥ�� ã�� ���� �ݺ���
		{
			memset(curBlk, 0, sizeof(UChar) * blkSize * blkSize);
			///////////////////////////////////////////////////////////////////////////////////////////
			TL_curBlk = img->ori + (blkRow * blkSize) * stride + (blkCol * blkSize);                   //����� ���� �� ��ǥ
			for (int k = 0; k < blkSize; k++)         //����� ���� �� ��ǥ�� �������� �� blkSize x blkSize ����
				for (int l = 0; l < blkSize; l++)
				{
					TMP = TL_curBlk + (k * stride + l);
					curBlk[k * blkSize + l] = TMP[0];
				}

			DCT_Func(curBlk, blkSize, blkRow, blkCol, DCT); //curBlk: ���� ���(blkSize x blkSize)
#if TransType == 0
			count += (blkSize * blkSize);
			printf("DCT : %.2f %%\n", (double)count / (wid * hei) * 100);
#endif
		}
	}
	free(curBlk);
}