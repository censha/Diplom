#include "stdafx.h"
#include <gdipluscolor.h>

Color* ApplyGammaFragment(Int64 Value, Color clr, Int64 mod)
{
	// получаем гаммируемое число по clr
	// 65536*R+256*G+B - имеем число от 0 до 2^24-1
	Int64 toGamma = Int64(65536) * Int64(clr.GetR());
	toGamma += Int64(256) * Int64(clr.GetG());
	toGamma += Int64(clr.GetB());

	// применяем побитовое искл или
	toGamma = (Value ^ toGamma);
	toGamma = (toGamma & (mod - 1));

	int b = toGamma % 256;
	toGamma = toGamma >> 8;
	int g = toGamma % 256;
	toGamma = toGamma >> 8;
	int r = toGamma;

	Color* c = new Color(clr.GetA(), r, g, b);
	return c;
}

void SaveBmp(char FileName[], Bitmap * bm)
{
	wchar_t wFileName[1000];
	size_t length = strlen(FileName);
	mbstowcs_s(&length, wFileName, FileName, 1000);
	CLSID MyEncoder;
	CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}",
		&MyEncoder);
	bm->Save(wFileName, &MyEncoder, NULL);
}

Bitmap * LoadBmp(char FileName[])
{
	wchar_t wFileName[1000];
	size_t length = strlen(FileName);
	mbstowcs_s(&length, wFileName, FileName, 1000);

	Bitmap * bm = new Bitmap(wFileName);
	return bm;
}

void Gamming(char FileName[], char FileResult[], DoubleLinearGen * g, COMPLEXARRAY * sizes)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// инициализируем GDI
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap * bm = LoadBmp(FileName);

	int h = bm->GetHeight(), w = bm->GetWidth();
	
	MathcadArrayAllocate(sizes, 2, 1, TRUE, FALSE);
	sizes->hReal[0][0] = w;
	sizes->hReal[0][1] = h;
	//cout << "h = " << h << ", w = " << w << "\n";

	Int64 mod = g->GetModule();
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			Color clr;
			bm->GetPixel(i, j, &clr);
			Color * inv = ApplyGammaFragment(g->GetNext(), clr, mod);
			bm->SetPixel(i, j, *inv);
			delete inv;
		}

	SaveBmp(FileResult, bm);
	delete bm;
}

// сколько байт у картинки?
int SizeOfImageByte(char FileName[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// инициализируем GDI
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap * bm = LoadBmp(FileName);
	return 3 * (bm->GetWidth()) * (bm->GetHeight());
}