#include "stdafx.h"

#define N_ERRORS 11

char * myErrorMessageTable[N_ERRORS] =
{
	"Bad pseudo random generator parameter value",
	"File doesn't exist",
	"A must be A > 1: A mod 4 = 1",
	"a > 1 required",
	"Not empty string required",
	"Amount of numbers must be 5, 10, 15, ..., 1000",
	"Bad stego parameter value",
	"Must be GCD=1 for StepByte and image bytes amount",
	"Must be GCD=1 for SkipStep and image bytes amount",
	"Limit capacity exceeded",
	"Value must belong to {1, 2, ..., 8}"
};

LRESULT ImageGamming(COMPLEXARRAY * const result, MCSTRING * FileName,
	MCSTRING * FileResult, COMPLEXSCALAR * A, COMPLEXSCALAR * B,
	COMPLEXSCALAR * Y0, COMPLEXSCALAR * a, COMPLEXSCALAR * b, COMPLEXSCALAR * y0)
{
	if (FileName->str[0] == 0)
		return MAKELRESULT(NOT_EMPTY_STRING_REQUIRED, 1);
	if (FileResult->str[0] == 0)
		return MAKELRESULT(NOT_EMPTY_STRING_REQUIRED, 2);

	if (FileExists(FileName->str) == false)
		return MAKELRESULT(FILE_DOESNT_EXIST, 1);

	// первый числовой параметр - по счёту 3-ий!
	LRESULT errorNum = CheckPseudoRandomParams(A, B, Y0, a, b, y0, 3);

	// если с параметрами генератора не всё в порядке
	if (errorNum != 0) return errorNum;

	DoubleLinearGen G((Int64)A->real, (Int64)B->real,
		(Int64)Y0->real, (Int64)a->real, (Int64)b->real, (Int64)y0->real);

	Gamming(FileName->str, FileResult->str, &G, result);

	return 0;
}

LRESULT GetGammingNumbers(COMPLEXARRAY * const result,
	COMPLEXSCALAR * A, COMPLEXSCALAR * B,
	COMPLEXSCALAR * Y0, COMPLEXSCALAR * a, COMPLEXSCALAR * b, COMPLEXSCALAR * y0, COMPLEXSCALAR * n)
{
	LRESULT errorNum = CheckPseudoRandomParams(A, B, Y0, a, b, y0, 1);

	// если с параметрами генератора не всё в порядке
	if (errorNum != 0) return errorNum;

	// если с количеством чисел не всё в порядке
	errorNum = CheckNGammaValues(n, 7);
	if (errorNum != 0) return errorNum;

	DoubleLinearGen G((Int64)A->real, (Int64)B->real,
		(Int64)Y0->real, (Int64)a->real, (Int64)b->real, (Int64)y0->real);
	G.GetNumbers(result, (int)(n->real));

	return 0;
}

LRESULT ImageStegoHide(COMPLEXSCALAR * const result, MCSTRING * MsgFile,
	MCSTRING * ImageFile, MCSTRING * ResultFile, COMPLEXSCALAR * StartByte,
	COMPLEXSCALAR * StepByte, COMPLEXSCALAR * LastBits, COMPLEXSCALAR * SkipStep)
{
	MCSTRING * Files[3] = { MsgFile, ImageFile, ResultFile };
	LRESULT errorNum = CheckFilesForErrors(Files, 3);
	if (errorNum != 0) return errorNum;

	errorNum = CheckStegoParams(StartByte, StepByte, LastBits, SkipStep,
		ImageFile->str, MsgFile->str, 4);
	if (errorNum != 0) return errorNum;

	Stego Stg(StartByte, StepByte, LastBits, SkipStep);
	int BytesHidden = Stg.HideData(MsgFile->str, ImageFile->str, ResultFile->str);
	result->imag = 0;
	result->real = BytesHidden;

	return 0;
}

LRESULT ImageStegoRestore(COMPLEXSCALAR * const result,
	MCSTRING * ImageFile, MCSTRING * ResultFile, COMPLEXSCALAR * StartByte,
	COMPLEXSCALAR * StepByte, COMPLEXSCALAR * LastBits, COMPLEXSCALAR * SkipStep)
{
	MCSTRING * Files[2] = { ImageFile, ResultFile };
	LRESULT errorNum = CheckFilesForErrors(Files, 2);
	if (errorNum != 0) return errorNum;

	char FictiveStr[1];
	FictiveStr[0] = 0;

	errorNum = CheckStegoParams(StartByte, StepByte, LastBits, SkipStep,
		ImageFile->str, FictiveStr, 3);
	if (errorNum != 0) return errorNum;

	Stego Stg(StartByte, StepByte, LastBits, SkipStep);
	int BytesRestored = Stg.RestoreData(ImageFile->str, ResultFile->str);
	result->imag = 0;
	result->real = BytesRestored;

	return 0;
}

LRESULT CheckerMeasure(MCSTRING * ImageFile, COMPLEXSCALAR * MaxLastBits)
{
	LRESULT errorNum = CheckOneFile(ImageFile, 1);
	if (errorNum != 0) return errorNum;

	if (IsInt(MaxLastBits) == false)
		return MAKELRESULT(BAD_LAST_BITS, 2);

	int LB = int(MaxLastBits->real);
	if (LB < 1 || LB > 8) return MAKELRESULT(BAD_LAST_BITS, 2);
	return 0;
}

LRESULT ImageStegoPotential(COMPLEXSCALAR * const result,
	MCSTRING * ImageFile, COMPLEXSCALAR * MaxLastBits)
{
	LRESULT errorNum = CheckerMeasure(ImageFile, MaxLastBits);
	if (errorNum != 0) return errorNum;

	result->imag = 0;
	result->real = GetKeyPotential(ImageFile->str, int(MaxLastBits->real));

	return 0;
}

LRESULT ImageStegoCapacity(COMPLEXSCALAR * const result,
	MCSTRING * ImageFile, COMPLEXSCALAR * LastBits)
{
	LRESULT errorNum = CheckerMeasure(ImageFile, LastBits);
	if (errorNum != 0) return errorNum;

	result->imag = 0;
	result->real = GetImageCapacity(ImageFile->str, int(LastBits->real));

	return 0;
}

LRESULT AnalysisHierarchyLSB(COMPLEXSCALAR* const result,
	MCSTRING* ImgFile, MCSTRING* ImgFileSave, COMPLEXSCALAR* layers, MCSTRING* channels)
{
	MCSTRING* Files[2] = { ImgFile, ImgFileSave };
	LRESULT errorNum = CheckFilesForErrors(Files, 2);
	if (errorNum != 0) return errorNum;

	AnalysisHierarchy analysisHierarchy;

	analysisHierarchy.AnalysisHierarchyLSB(ImgFile->str, ImgFileSave->str, int(layers->real), channels->str);

	result->imag = 0;
	result->real = 0;

	return 0;
}

FUNCTIONINFO fiImageGamming =
{
	"ImageGamming",
	"Source File, Results File, A, B, Y0, a, b, y0",
	"Applying gamma by double congruent generator to image",
	(LPCFUNCTION)ImageGamming,
	COMPLEX_ARRAY,
	8,
	{ STRING, STRING, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR,
	COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR}
};

FUNCTIONINFO fiGetGammingNumbers =
{
	"GetGammingNumbers",
	"A, B, Y0, a, b, y0, n",
	"Generates number serie by double congruent generator",
	(LPCFUNCTION)GetGammingNumbers,
	COMPLEX_ARRAY,
	7,
	{ COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR,
	COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR }
};

FUNCTIONINFO fiImageStegoHide =
{
	"ImageStegoHide",
	"MsgFile, ImageFile, ResultFile, StartByte, StepByte, LastBits, SkipStep",
	"Hides arbitrary file in picture container",
	(LPCFUNCTION)ImageStegoHide,
	COMPLEX_SCALAR,
	7,
	{STRING, STRING, STRING, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR }
};

FUNCTIONINFO fiImageStegoRestore =
{
	"ImageStegoRestore",
	"ImageFile, ResultFile, StartByte, StepByte, LastBits, SkipStep",
	"Restores data hidden in picture container",
	(LPCFUNCTION)ImageStegoRestore,
	COMPLEX_SCALAR,
	6,
	{ STRING, STRING, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR, COMPLEX_SCALAR }
};

FUNCTIONINFO fiImageStegoPotential =
{
	"ImageStegoPotential",
	"ImageFile, MaxLastBits",
	"Calculates amount of possible keys for given container",
	(LPCFUNCTION)ImageStegoPotential,
	COMPLEX_SCALAR,
	2,
	{ STRING, COMPLEX_SCALAR }
};

FUNCTIONINFO fiImageStegoCapacity =
{
	"ImageStegoCapacity",
	"ImageFile, LastBits",
	"Calculates limit message bytes amount for given container and last bits amount",
	(LPCFUNCTION)ImageStegoCapacity,
	COMPLEX_SCALAR,
	2,
	{ STRING, COMPLEX_SCALAR }
};

FUNCTIONINFO fiAnalysisHierarchyLSB =
{
	"AnalysisHierarchyLSB",
	"ImgFile, ImgFileSave, layers, channels",
	"Algorithm for detecting steganographic inserts of the LSB-substitution type based on the hierarchy analysis method",
	(LPCFUNCTION)AnalysisHierarchyLSB,
	COMPLEX_SCALAR,
	4,
	{ STRING, STRING, COMPLEX_SCALAR, STRING}
};

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (CreateUserErrorMessageTable(hModule, N_ERRORS, myErrorMessageTable))
		{
			CreateUserFunction(hModule, &fiImageGamming);
			CreateUserFunction(hModule, &fiGetGammingNumbers);
			CreateUserFunction(hModule, &fiImageStegoHide);
			CreateUserFunction(hModule, &fiImageStegoRestore);
			CreateUserFunction(hModule, &fiImageStegoPotential);
			CreateUserFunction(hModule, &fiImageStegoCapacity);		
			CreateUserFunction(hModule, &fiAnalysisHierarchyLSB);
		}
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}