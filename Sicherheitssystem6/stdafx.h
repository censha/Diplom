// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // ��������� ����� ������������ ���������� �� ���������� Windows
// ����� ���������� Windows:
#include <windows.h>

#include "MCADINCL.H"
#pragma comment(lib, "mcaduser.lib")

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment (lib,"Gdiplus.lib")

typedef long long Int64;

#define BAD_GENERATORS_PARAM 1
#define FILE_DOESNT_EXIST 2
#define BAD_A_GREAT 3
#define BAD_A_SMALL 4
#define NOT_EMPTY_STRING_REQUIRED 5
#define HOW_MANY_IS_BAD 6
#define BAD_STEGO_PARAM 7
#define BAD_STEP_BYTE 8
#define BAD_SKIP_STEP 9
#define LIMIT_CAPACITY_EXCEEDED 10
#define BAD_LAST_BITS 11

#define VALUES_PER_COLUMN 5

class DoubleLinearGen
{
	Int64 A, Y0, B; // ��� ������� �������
	Int64 a, y0, b; // ��� ������
	Int64 Cur, RR;
	Int64 module;

	Int64 SetRR(Int64 Value, int Shift);
public:
	DoubleLinearGen(int A=0, int B=0, int Y0=0, int a=0, int b=0, int y0=0)
	{
		this->A = A;
		this->Y0 = Y0;
		this->B = B;
		this->a = a;
		this->y0 = y0;
		this->b = b;
		this->Cur = Y0;
		this->RR = y0;
		this->module = pow2(24);
	}

	Int64 pow2(int n)
	{
		Int64 x = 1;
		for (int i = 1; i <= n; i++)
			x *= 2;
		return x;
	}

	int GetNext()
	{
		Cur = (A * Cur + B) % module;
		RR = (a * RR + b) % 23;

		return SetRR(Cur, RR + 1);
	}

	int GetRR() { return RR + 1; }

	Int64 GetModule() { return module; }

	void GetNumbers(COMPLEXARRAY * p, int HowMany);
};

struct BytesArray
{
	BYTE * bytes;
	int n;

	void CharsToBytes(char c[])
	{
		for (int i = 0; i < n; i++)
			bytes[i] = (BYTE)c[i];
	}

	void BytesToChars(char c[])
	{
		for (int i = 0; i < n; i++)
			c[i] = (char)bytes[i];
	}

	BytesArray(char FileName[]);
	BytesArray()
	{
		n = 0;
		bytes = NULL;
	}
	BytesArray(int n)
	{
		this->n = n;
		this->bytes = new BYTE[n];
	}
	int CheckSize(char FileName[]);

	void Write(char FileName[]);

	~BytesArray() { if (bytes != NULL) delete[] bytes; }
};

class Stego
{
	int StartByte; // ����� ���������� ����� ��� �������
	int StepByte; // ��� � ������
	int LastBits; // ����� ������������ ������� ���
	int SkipStep; // ��� ���������
	int BitsForSize; // ����� ��� ��� ������

	int UsedBytes; // ����� ��� �������������� ����

	// �������� 8-������� �������� � ������ �, ������� � ������� pos
	void BitsToBool(int x, bool A[], int pos);

	// �������� ����� ������� � ������ �
	void ColorToBool(Color Clr, bool A[], int pos);

	// ������������� ����������� ��� ������ ���
	bool * BitmapToBoolArray(char FileName[], int Sizes[]);

	// ��������� ������� ������� ������������� ����
	int PosOfStartBit();

	// ��������� ������� ���������� �� cur ������������� ����
	int PosOfNextBit(int cur, int ContainerSize);

	// ������� n-������� �������������� ��������
	int HideValue(int Value, int nBits, bool ContainerBits[], int cLength, int cur,
		bool IsUsed[]);

	// ������� ���� � ������ ��� ���������� (������� ������)
	void HideMsg(BYTE MyMsg[], int nBytes, bool ContainerBits[], int cLength);

	// ��������� ����� �� ���������� A[pos...pos+7]
	int BoolToInt(bool A[], int pos);

	// ��������� ����� ������� �� ���������� A[pos...pos+23]
	void BoolToColor(bool A[], int pos, Color * c);

	// ���������� �����������
	void SavePict(char FileName[], bool bits[], int Sizes[]);

	// �������������� n-������� �������������� ��������
	// cur - ������ ������������ ���
	int RestoreValue(int nBits, bool ContainerBits[], int cLength, int cur, int Value[],
		bool IsUsed[]);

	// �������������� ���� �� ������� ��� ����������
	BytesArray * RestoreData(bool ContainerBits[], int cLength);

	// ����� ���(a, b)
	int NOD(int a, int b);

	// ����� ������� �����, ������� �� ������ 0 ��� �� ������ Upper
	int FindViolation(int A[], int n, int Lower[], int Upper[]);

	bool * GetFalseArray(int n);
public:
	Stego(int StartByte, int StepByte, int LastBits, int SkipStep)
	{
		this->StartByte = StartByte;
		this->StepByte = StepByte;
		this->LastBits = LastBits;
		this->BitsForSize = 24;
		this->SkipStep = SkipStep;
	}

	Stego(COMPLEXSCALAR * StartByte,
		COMPLEXSCALAR * StepByte, COMPLEXSCALAR * LastBits, COMPLEXSCALAR * SkipStep) :
		Stego(int(StartByte->real), int(StepByte->real), int(LastBits->real),
			int(SkipStep->real))
	{

	}
	Stego() : Stego(0, 0, 0, 0)
	{

	}

	// ���������� ������� ���������
	int LimitPotentialBytes(int ImageBytes);

	// ����� ���������� ���� ������� ������� � n �� start �� n
	int CountGCD1(int n, int start);

	// �������� ����� �������
	int HideData(char FileMsg[], char ImgFile[], char ResultFile[]);

	// �������������� ������
	int RestoreData(char ImgFile[], char ResultFile[]);

	LRESULT CheckForErrors(int start_arg, int ImageBytes, int MsgBytes);
};

class AnalysisHierarchy
{
public:

	AnalysisHierarchy() {};

	void SaveBmp(char FileName[], Bitmap* bm);

	int BoolToInt(bool A[], int pos);

	void BoolToColorBlackAndWhite(bool A, Color* c);

	void BoolToColor(bool A[], int pos, Color* c);

	void SavePict(char FileName[], bool bits[], int Sizes[]);

	void SavePictBlackAndWhite(char FileName[], bool** bits, int Sizes[]);

	void BitsToBool(int x, bool A[], int pos);

	void ColorToBool(Color Clr, bool A[], int pos);

	bool* BitmapToBoolArray(char FileName[], int Sizes[]);

	void GetSizeBitmap(char FileName[], int Sizes[]);

	bool GetBeatNumber(int number, int value);

	void AnalysisHierarchyLSB(char FileName[], char ImgFileSave[], int QuantityLayers, char channels[]);

	void AnalysisLauers(int** Channel, int Sizes[], bool** ResultChannel, int QuantityLayers);
};

void Gamming(char FileName[], char FileResult[], DoubleLinearGen * g,
	COMPLEXARRAY * sizes);

bool FileExists(char FileName[]);

LRESULT CheckPseudoRandomParams(COMPLEXSCALAR * A, COMPLEXSCALAR * B,
	COMPLEXSCALAR * Y0, COMPLEXSCALAR * a, COMPLEXSCALAR * b, COMPLEXSCALAR * y0,
	int first_arg_num // ����� ���������� �� ����� �-��������
);

LRESULT CheckNGammaValues(COMPLEXSCALAR * n, int arg_num);

void SaveBmp(char FileName[], Bitmap * bm);

Bitmap * LoadBmp(char FileName[]);

// ������� ���� � ��������?
int SizeOfImageByte(char FileName[]);

LRESULT CheckStegoParams(COMPLEXSCALAR * StartByte,
	COMPLEXSCALAR * StepByte, COMPLEXSCALAR * LastBits, COMPLEXSCALAR * SkipStep,
	char ContainerFile[], char MsgFile[],
	int first_arg_num // ����� ���������� �� ����� �������� StartByte
);

LRESULT CheckFilesForErrors(MCSTRING * Files[], int nFiles);

double GetKeyPotential(char ImgFile[], int MaxLastBits);
LRESULT CheckOneFile(MCSTRING * FName, int arg_num);
bool IsInt(COMPLEXSCALAR * Number);
int GetImageCapacity(char ImgFile[], int LastBits);