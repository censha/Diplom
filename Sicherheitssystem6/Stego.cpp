#include "stdafx.h"
#include <gdipluscolor.h>

void Stego::BitsToBool(int x, bool A[], int pos)
{
	for (int i = 0; i < 8; i++)
	{
		// ! ���� �� ����� ���������� A[pos...pos+7] � ������
		A[pos + 7 - i] = (x % 2 == 1);
		x /= 2;
	}
}

// �������� ����� ������� � ������ �
void Stego::ColorToBool(Color Clr, bool A[], int pos)
{
	BitsToBool(Clr.GetR(), A, pos);
	BitsToBool(Clr.GetG(), A, pos + 8);
	BitsToBool(Clr.GetB(), A, pos + 16);
}

// ������������� ����������� ��� ������ ���
bool * Stego::BitmapToBoolArray(char FileName[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// �������������� GDI ---
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap * bm = LoadBmp(FileName);

	int h = bm->GetHeight(), w = bm->GetWidth();

	bool * result = new bool[24 * h * w];

	int pos = 0; // ������� ������ �������� ������� � result
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			Color clr;

			// ���� � i-�� ������ j-�� ������� ������� ��������
			bm->GetPixel(i, j, &clr);

			// ��������� ���� ���� � ������ result � ���� ������ ���
			ColorToBool(clr, result, pos);

			pos += 24;
		}

	Sizes[0] = w;
	Sizes[1] = h;
	
	delete bm;

	return result;
}

// ��������� ������� ������� ������������� ����
int Stego::PosOfStartBit()
{
	// ����� ���������� ���� ������� ������������� �����
	// � ����� ���� ��� �����������
	int TailBit = 8 * StartByte + 7;

	// ����� ������� ������������� ���� � ���� �� �����
	// � ���� ������ ���
	return TailBit - LastBits + 1;
}

// ��������� ������� ���������� �� cur ������������� ����
int Stego::PosOfNextBit(int cur, int ContainerSize)
{
	// ���� cur-�� ��� �� ��������� � ����� �����
	if (cur % 8 != 7) return cur + 1;

	// ���� ���������

	// ����������, ��� ������� ���� ��� ��������� �����������
	this->UsedBytes++;

	// ����������� � �������� ����� � ����� ���������� ������������� �����
	cur += StepByte * 8;

	// ��������� ���������� ������������ ������� ���
	return (cur - LastBits + 1) % ContainerSize;
}

bool * Stego::GetFalseArray(int n)
{
	bool * fg = new bool[n];
	for (int i = 0; i < n; i++)
		fg[i] = false;
	return fg;
}

// ������� n-������� �������������� ��������
// cur - ������ ������������ ���,
// ������� ���������� ���, � �������� �����
// ���������� ��������� ��������
int Stego::HideValue(int Value, int nBits, bool ContainerBits[], int cLength, int cur,
	bool IsUsed[])
{
	DoubleLinearGen G;

	// ��� �������� ���� � n-������� �����
	int w = (int)G.pow2(nBits - 1);

	for (int i = 0; i < nBits; i++)
	{
		// ���������� i-�� �� ����������� ���
		ContainerBits[cur] = ((Value & w) != 0);
		IsUsed[cur] = true;

		int OldUsedBytes = this->UsedBytes;

		// ����������, ���� �������� ��������� ���
		cur = PosOfNextBit(cur, cLength);

		if (UsedBytes != OldUsedBytes && UsedBytes % SkipStep == SkipStep - 1)
		{
			do
			{
				cur = PosOfNextBit(cur, cLength);			
			}
			while (IsUsed[cur] == true);
		}		

		// ��������� ��� ����
		w /= 2;
	}

	return cur;
}

// ������� ���� � ������ ��� ���������� (������� ������)
void Stego::HideMsg(BYTE MyMsg[], int nBytes, bool ContainerBits[], int cLength)
{
	bool * fg = GetFalseArray(cLength);

	int cur = PosOfStartBit(); // ������� ���������� ���� �������
	
	// �������� ������
	cur = HideValue(nBytes, BitsForSize, ContainerBits, cLength, cur, fg);

	// �������� ���� ��������
	for (int i = 0; i < nBytes; i++)
	{
		cur = HideValue((int)MyMsg[i], 8, ContainerBits, cLength, cur, fg);
	}

	delete[] fg;
}

// ��������� ����� �� ���������� A[pos...pos+7]
int Stego::BoolToInt(bool A[], int pos)
{
	int x = 0, mult = 1;
	for (int i = pos + 7; i >= pos; i--)
	{
		if (A[i] == true) x += mult;
		mult *= 2;
	}
	return x;
}

// ��������� ����� ������� �� ���������� A[pos...pos+23]
void Stego::BoolToColor(bool A[], int pos, Color * c)
{
	int r = BoolToInt(A, pos);
	int g = BoolToInt(A, pos + 8);
	int b = BoolToInt(A, pos + 16);

	c->SetFromCOLORREF(RGB(r, g, b));
}

// ���������� �����������
void Stego::SavePict(char FileName[], bool bits[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// �������������� GDI
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int w = Sizes[0], h = Sizes[1];

	Bitmap bm(w, h);

	// ������� ��� �������������� �� bits �������� �������
	int pos = 0;

	Color c;

	// �� ������� ��� ������ �������
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			BoolToColor(bits, pos, &c);
			bm.SetPixel(i, j, c);

			// ��������� ������� - �� 24 ���� ������
			pos += 24;
		}

	SaveBmp(FileName, &bm);
}

// �������� ����� �������
int Stego::HideData(char FileMsg[], char ImgFile[], char ResultFile[])
{
	this->UsedBytes = 0;

	int Sizes[2]; // ��� ����������� �������� �����������

	// ������������ ����������� �� ����� ImgFile ��� ����� ���
	bool * ContainerBits = BitmapToBoolArray(ImgFile, Sizes);

	BytesArray b(FileMsg);

	// �������� ����� � ������ ���
	HideMsg(b.bytes, b.n, ContainerBits, 24 * Sizes[0] * Sizes[1]);

	// ��������� �����������, �������������� ������� ���
	SavePict(ResultFile, ContainerBits, Sizes);

	delete[] ContainerBits;

	return b.n;
}

// �������������� n-������� �������������� ��������
// cur - ������ ������������ ���
int Stego::RestoreValue(int nBits, bool ContainerBits[], int cLength, int cur, int Value[],
	bool IsUsed[])
{
	DoubleLinearGen G;

	// ��� �������� ���� � n-������� �����
	int w = (int)G.pow2(nBits - 1);

	Value[0] = 0;

	for (int i = 0; i < nBits; i++)
	{
		// ���� i-�� �� ����������� ��� ���������
		if (ContainerBits[cur] == true)
			Value[0] += w; // ������������� �������� ��� ����� ����� ������. ���
		IsUsed[cur] = true;

		int OldUsedBytes = this->UsedBytes;

		// ����������, ��� ����� ��������� ���
		cur = PosOfNextBit(cur, cLength);

		if (UsedBytes != OldUsedBytes && UsedBytes % SkipStep == SkipStep - 1)
		{
			do
			{
				cur = PosOfNextBit(cur, cLength);
			} while (IsUsed[cur] == true);
		}

		// ��������� ��� ����
		w /= 2;
	}

	return cur;
}

// �������������� ���� �� ������� ��� ����������
BytesArray * Stego::RestoreData(bool ContainerBits[], int cLength)
{
	bool * fg = GetFalseArray(cLength);

	int cur = PosOfStartBit(); // ������� ���������� ���� �������

	int msize = 0, Value[1];

	// ��������������� ������
	cur = RestoreValue(BitsForSize, ContainerBits, cLength, cur, Value, fg);
	msize = Value[0];

	BytesArray * msg = new BytesArray(msize);

	// ��������������� �����
	for (int i = 0; i < msize; i++)
	{
		cur = RestoreValue(8, ContainerBits, cLength, cur, Value, fg);
		msg->bytes[i] = (BYTE)Value[0];
	}

	delete[] fg;

	return msg;
}

// �������������� ������
int Stego::RestoreData(char ImgFile[], char ResultFile[])
{
	this->UsedBytes = 0;

	int Sizes[2]; // ��� ����������� �������� �����������

	// ������������ ����������� �� ����� ImgFile ��� ����� ���
	bool * ContainerBits = BitmapToBoolArray(ImgFile, Sizes);

	// ��������������� ������ �� ������� ���
	BytesArray * Restored = RestoreData(ContainerBits, 24 * Sizes[0] * Sizes[1]);

	// ��������� ������ � ����
	Restored->Write(ResultFile);

	delete[] ContainerBits;
	int result = Restored->n;
	delete Restored;
	return result;
}