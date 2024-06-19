#include "stdafx.h"
#include <fstream>
using namespace std;

bool FileExists(char FileName[])
{
	ifstream file(FileName, ios::in);
	if (!file) return false;
	file.close();
	return true;
}

LRESULT AChecker(int A, int a, int first_arg_num)
{
	if (A <= 1 || A % 4 != 1)
		// A - самый первый из числовых аргументов
		return MAKELRESULT(BAD_A_GREAT, first_arg_num);
	// a отстоит 3 позициями правее
	if (a < 2) return MAKELRESULT(BAD_A_SMALL, first_arg_num + 3);
	return 0;
}

LRESULT CheckPseudoRandomParams(COMPLEXSCALAR * A, COMPLEXSCALAR * B,
	COMPLEXSCALAR * Y0, COMPLEXSCALAR * a, COMPLEXSCALAR * b, COMPLEXSCALAR * y0,
	int first_arg_num // каким аргументом по счёту А-параметр
)
{
	double images[] = { A->imag, B->imag, Y0->imag, a->imag, b->imag, y0->imag };
	double reals[] = { A->real, B->real, Y0->real, a->real, b->real, y0->real };

	DoubleLinearGen G;
	int n = 6, i, mod = G.GetModule();

	// запретить ненулевую мнимую часть числа и нецелую вещественную часть
	for (i = 0; i < n; i++)
	{
		if (images[i] != 0 || floor(reals[i]) != reals[i])
			// сначала номер ошибки, затем номер аргумента
			return MAKELRESULT(BAD_GENERATORS_PARAM, first_arg_num + i);

		int x = int(reals[i]);
		int upper_bound = (i < 3) ? mod : 23;

		if (x < 0 || x >= upper_bound)
			return MAKELRESULT(BAD_GENERATORS_PARAM, first_arg_num + i);
	}

	int _A = int(A->real), _a = int(a->real);
	return AChecker(_A, _a, first_arg_num);
}

LRESULT CheckNGammaValues(COMPLEXSCALAR * n, int arg_num)
{
	// если это целое
	if (n->imag == 0 && floor(n->real) == n->real)
	{
		int N = int(n->real);
		if (N > 0 && N <= 1000 && N % VALUES_PER_COLUMN == 0) return 0;
	}
	return MAKELRESULT(HOW_MANY_IS_BAD, arg_num);
}

// поиск индекса числа, которое не больше Lower или не меньше Upper
int Stego::FindViolation(int A[], int n, int Lower[], int Upper[])
{
	for (int i = 0; i < n; i++)
		if (A[i] <= Lower[i] || A[i] >= Upper[i])
			return i;
	return -1;
}

LRESULT Stego::CheckForErrors(int start_arg, int ImageBytes, int MsgBytes = 0)
{
	int Values[] = { StartByte, StepByte, LastBits, SkipStep };
	int Lower[] = { 0, 0, 0, 3 };
	int Upper[] = { ImageBytes, ImageBytes, 8, ImageBytes };

	int p = FindViolation(Values, 4, Lower, Upper);
	if (p != -1)
		return MAKELRESULT(BAD_STEGO_PARAM, start_arg + p);

	if (MsgBytes != 0)
	{
		if (LimitPotentialBytes(ImageBytes) < MsgBytes)
			return MAKELRESULT(LIMIT_CAPACITY_EXCEEDED, 0);
	}

	if (NOD(StepByte, ImageBytes) != 1)
		return MAKELRESULT(BAD_STEP_BYTE, start_arg + 1);
	if (NOD(SkipStep, ImageBytes) != 1)
		return MAKELRESULT(BAD_SKIP_STEP, start_arg + 3);
	return 0;
}

LRESULT CheckStegoParams(COMPLEXSCALAR * StartByte,
	COMPLEXSCALAR * StepByte, COMPLEXSCALAR * LastBits, COMPLEXSCALAR * SkipStep,
	char ContainerFile[], char MsgFile[],
	int first_arg_num // каким аргументом по счёту параметр StartByte
)
{
	double images[] = { StartByte->imag, StepByte->imag, LastBits->imag, SkipStep->imag };
	double reals[] = { StartByte->real, StepByte->real, LastBits->real, SkipStep->real };

	int n = 4, i;

	// запретить ненулевую мнимую часть числа и нецелую вещественную часть
	for (i = 0; i < n; i++)
	{
		if (images[i] != 0 || floor(reals[i]) != reals[i])
			// сначала номер ошибки, затем номер аргумента
			return MAKELRESULT(BAD_STEGO_PARAM, first_arg_num + i);
	}

	Stego Stg(StartByte, StepByte, LastBits, SkipStep);
	int ImageBytes = SizeOfImageByte(ContainerFile);
	BytesArray A;
	int MsgBytes = 0;
	if (MsgFile[0] != 0)
		MsgBytes = A.CheckSize(MsgFile);
			
	return Stg.CheckForErrors(first_arg_num, ImageBytes, MsgBytes);
}

LRESULT CheckFilesForErrors(MCSTRING * Files[], int nFiles)
{
	int i;
	for (i = 0; i < nFiles; i++)
		if (Files[i]->str[0] == 0)
			return MAKELRESULT(NOT_EMPTY_STRING_REQUIRED, i + 1);

	// nFiles - 1: последний файл не должен существовать
	for (i = 0; i < nFiles - 1; i++)
		if (FileExists(Files[i]->str) == false)
			return MAKELRESULT(FILE_DOESNT_EXIST, i + 1);

	return 0;
}

LRESULT CheckOneFile(MCSTRING * FName, int arg_num)
{
	if (FName->str[0] == 0)
		return MAKELRESULT(NOT_EMPTY_STRING_REQUIRED, arg_num);

	if (FileExists(FName->str) == false)
		return MAKELRESULT(FILE_DOESNT_EXIST, arg_num);

	return 0;
}

bool IsInt(COMPLEXSCALAR * Number)
{
	if (Number->imag != 0 || floor(Number->real) != Number->real)
		return false;
	return true;
}