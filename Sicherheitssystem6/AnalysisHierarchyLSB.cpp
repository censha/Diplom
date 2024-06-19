#include "stdafx.h"
#include <iostream>
#include <cmath>
using namespace std;

void AnalysisHierarchy::SaveBmp(char FileName[], Bitmap* bm)
{
	wchar_t wFileName[1000];
	size_t length = strlen(FileName);
	mbstowcs_s(&length, wFileName, FileName, 1000);
	CLSID MyEncoder;
	CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}",
		&MyEncoder);
	bm->Save(wFileName, &MyEncoder, NULL);
}

int AnalysisHierarchy::BoolToInt(bool A[], int pos)
{
	int x = 0, mult = 1;
	for (int i = pos + 7; i >= pos; i--)
	{
		if (A[i] == true) x += mult;
		mult *= 2;
	}
	return x;
}

void AnalysisHierarchy::BoolToColorBlackAndWhite(bool A, Color* c)
{
	int r = 255;
	int g = 255;
	int b = 255;

	if (A == true)
	{
		r = 0, g = 0, b = 0;
	}

	c->SetFromCOLORREF(RGB(r, g, b));
}

void AnalysisHierarchy::BoolToColor(bool A[], int pos, Color* c)
{
	int r = BoolToInt(A, pos);
	int g = BoolToInt(A, pos + 8);
	int b = BoolToInt(A, pos + 16);

	c->SetFromCOLORREF(RGB(r, g, b));
}

void AnalysisHierarchy::SavePict(char FileName[], bool bits[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int w = Sizes[0], h = Sizes[1];

	Bitmap bm(w, h);

	int pos = 0;

	Color c;

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			BoolToColor(bits, pos, &c);
			bm.SetPixel(i, j, c);
			pos += 24;
		}

	SaveBmp(FileName, &bm);
}

void AnalysisHierarchy::SavePictBlackAndWhite(char FileName[], bool** bits, int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int w = Sizes[0], h = Sizes[1];

	Bitmap bm(w, h);
	Color c;

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			BoolToColorBlackAndWhite(bits[i][j], &c);
			bm.SetPixel(i, j, c);
		}

	SaveBmp(FileName, &bm);
}

void AnalysisHierarchy::BitsToBool(int x, bool A[], int pos)
{
	for (int i = 0; i < 8; i++)
	{
		A[pos + 7 - i] = (x % 2 == 1);
		x /= 2;
	}
}

void AnalysisHierarchy::ColorToBool(Color Clr, bool A[], int pos)
{
	BitsToBool(Clr.GetR(), A, pos);
	BitsToBool(Clr.GetG(), A, pos + 8);
	BitsToBool(Clr.GetB(), A, pos + 16);
}

bool* AnalysisHierarchy::BitmapToBoolArray(char FileName[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap* bm = LoadBmp(FileName);

	int h = bm->GetHeight(), w = bm->GetWidth();

	bool* result = new bool[24 * h * w];

	int pos = 0;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			Color clr;
			bm->GetPixel(i, j, &clr);



			ColorToBool(clr, result, pos);
			pos += 24;
		}

	Sizes[0] = w;
	Sizes[1] = h;

	delete bm;

	return result;
}

void AnalysisHierarchy::GetSizeBitmap(char FileName[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap* bm = LoadBmp(FileName);

	int h = bm->GetHeight(), w = bm->GetWidth();
	Sizes[0] = w;
	Sizes[1] = h;
}

bool AnalysisHierarchy::GetBeatNumber(int number, int value)
{
	if (number < 0 || number > 7 || value < 0 || value > 255)
		return 0;

	bool* temp = new bool[8];
	for (int i = 0; i < 8; i++)
	{
		temp[7 - i] = (value % 2 == 1);
		value /= 2;
	}

	for (int i = 0; i < 4; i++)
	{
		bool swap = temp[i];
		temp[i] = temp[7 - i];
		temp[7 - i] = swap;
	}

	bool res = temp[number];
	delete[] temp;
	return res;
}

void AnalysisHierarchy::AnalysisLauers(int** Channel, int Sizes[], bool** ResultChannel, int QuantityLayers)
{
	double Y = 0, N = 0;
	double Y1 = 0, N1 = 0, Y2 = 0, N2 = 0, Y3 = 0, N3 = 0;
	int coincidences = 0;
	double n = 2, k = 2;
	double K1 = n * k / n * k + k + 1, K2 = k / n * k + k + 1, K3 = 1 / n * k + k + 1;
	int layer = 0;

	bool** ResupreviousLayer = new bool* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		ResupreviousLayer[i] = new bool[Sizes[1]];
	for (int i = 0; i < Sizes[0]; i++)
		for (int j = 0; j < Sizes[1]; j++)
			ResupreviousLayer[i][j] = 0;

	bool** CurrentLayer = new bool* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		CurrentLayer[i] = new bool[Sizes[1]];
	for (int i = 0; i < Sizes[0]; i++)
		for (int j = 0; j < Sizes[1]; j++)
			CurrentLayer[i][j] = 0;

	for (int i = 0; i < Sizes[0]; i++)
		for (int j = 0; j < Sizes[1]; j++)
			ResultChannel[i][j] = 0;

	while (layer <= QuantityLayers)
	{
		for (int i = 1; i < Sizes[0] - 1; i++)
		{
			for (int j = 1; j < Sizes[1] - 1; j++)
			{
#pragma region K1
				coincidences = 0;

				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i][j - 1]))
					coincidences++;

				N1 = coincidences / 4;
				Y1 = (4 - coincidences) / 4;
#pragma endregion K1

#pragma region K2
				coincidences = 0;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j - 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j - 1]))
					coincidences++;

				N2 = coincidences / 4;
				Y2 = (4 - coincidences) / 4;
#pragma endregion K2

#pragma region K3
				coincidences = 0;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i][j - 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j - 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i - 1][j + 1]))
					coincidences++;
				if (GetBeatNumber(layer, Channel[i][j]) == GetBeatNumber(layer, Channel[i + 1][j - 1]))
					coincidences++;

				double currentValue;
				if (GetBeatNumber(layer, Channel[i][j]) == true)
					currentValue = 1;
				else
					currentValue = 0;

				double mean = coincidences / 8;
				double dc = abs(currentValue - mean);

				if (mean == 1)
				{
					N3 = 1;
					Y3 = 0;
				}
				else if (mean == 0)
				{
					N3 = 0;
					Y3 = 1;
				}
				else
				{
					N3 = 1 - dc;
					Y3 = dc;
				}
#pragma endregion K3

				Y = K1 * Y1 + K2 * Y2 + K3 * Y3;
				N = K1 * N1 + K2 * N2 + K3 * N3;

				if (Y > N)
					CurrentLayer[i][j] = 1;
			}
		}

		for (int i = 0; i < Sizes[0]; i++)
			for (int j = 0; j < Sizes[1]; j++)
			{
				if (layer != 0)
					if (CurrentLayer[i][j] == 1 && ResupreviousLayer[i][j] == 1)
						CurrentLayer[i][j] = 1;
					else
						CurrentLayer[i][j] = 0;

				ResupreviousLayer[i][j] = CurrentLayer[i][j];
			}


		layer++;
	}

	for (int i = 0; i < Sizes[0]; i++)
		for (int j = 0; j < Sizes[1]; j++)
			ResultChannel[i][j] = CurrentLayer[i][j];

	for (int i = 0; i < Sizes[0]; i++)
		delete[] ResupreviousLayer[i];
	delete[] ResupreviousLayer;

	for (int i = 0; i < Sizes[0]; i++)
		delete[] CurrentLayer[i];
	delete[] CurrentLayer;

	return;
}

void AnalysisHierarchy::AnalysisHierarchyLSB(char FileName[], char ImgFileSave[], int QuantityLayers, char channels[])
{
	int Sizes[2];
	bool* ContainerBits = BitmapToBoolArray(FileName, Sizes);

	bool R = false;
	bool G = false;
	bool B = false;

	string str = string(channels);

	for (int i = 0; i < str.length(); i++)
		if (str[i] == 'R')
			R = true;
		else if (str[i] == 'G')
			G = true;
		else if (str[i] == 'B')
			B = true;

	if (R == false && G == false && B == false)
		B = true;

#pragma region Initialization Channels

	int** ChannelR = new int* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		ChannelR[i] = new int[Sizes[1]];

	int** ChannelG = new int* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		ChannelG[i] = new int[Sizes[1]];

	int** ChannelB = new int* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		ChannelB[i] = new int[Sizes[1]];

#pragma endregion Initialization Channels

#pragma region Filling Channels

	int pos = 0, res = 0;
	for (int i = 0; i < Sizes[0]; i++)
		for (int j = 0; j < Sizes[1]; j++)
		{
			bool* temp = new bool[8];

			for (int k = 0; k < 8; k++)
			{
				temp[k] = ContainerBits[pos];
				pos++;
			}

			res = 0;
			for (int k = 0; k < 8; k++)
			{
				if (temp[k] == 1)
					res = 2 * res + 1;
				else if (temp[k] == 0)
					res *= 2;
			}

			ChannelR[i][j] = res;

			for (int k = 0; k < 8; k++)
			{
				temp[k] = ContainerBits[pos];
				pos++;
			}

			res = 0;
			for (int k = 0; k < 8; k++)
			{
				if (temp[k] == 1)
					res = 2 * res + 1;
				else if (temp[k] == 0)
					res *= 2;
			}

			ChannelG[i][j] = res;

			for (int k = 0; k < 8; k++)
			{
				temp[k] = ContainerBits[pos];
				pos++;
			}

			res = 0;
			for (int k = 0; k < 8; k++)
			{
				if (temp[k] == 1)
					res = 2 * res + 1;
				else if (temp[k] == 0)
					res *= 2;
			}

			ChannelB[i][j] = res;

			delete[] temp;
		}

#pragma endregion Filling Channels

#pragma region Analysis Hierarchy

	bool** ResultChannel = new bool* [Sizes[0]];
	for (int i = 0; i < Sizes[0]; i++)
		ResultChannel[i] = new bool[Sizes[1]];

	if (R)
	{
		bool** ResultChannelR = new bool* [Sizes[0]];
		for (int i = 0; i < Sizes[0]; i++)
			ResultChannelR[i] = new bool[Sizes[1]];

		AnalysisLauers(ChannelR, Sizes, ResultChannelR, QuantityLayers);

		for (int i = 0; i < Sizes[0]; i++)
			for (int j = 0; j < Sizes[1]; j++)
				if (ResultChannelR[i][j] == 1)
					ResultChannel[i][j] = 1;

		for (int i = 0; i < Sizes[0]; i++)
			delete[] ResultChannelR[i];
		delete[] ResultChannelR;
	}

	if (G)
	{
		bool** ResultChannelG = new bool* [Sizes[0]];
		for (int i = 0; i < Sizes[0]; i++)
			ResultChannelG[i] = new bool[Sizes[1]];

		AnalysisLauers(ChannelG, Sizes, ResultChannelG, QuantityLayers);

		for (int i = 0; i < Sizes[0]; i++)
			for (int j = 0; j < Sizes[1]; j++)
				if (ResultChannelG[i][j] == 1)
					ResultChannel[i][j] = 1;

		for (int i = 0; i < Sizes[0]; i++)
			delete[] ResultChannelG[i];
		delete[] ResultChannelG;
	}

	if (B)
	{
		bool** ResultChannelB = new bool* [Sizes[0]];
		for (int i = 0; i < Sizes[0]; i++)
			ResultChannelB[i] = new bool[Sizes[1]];

		AnalysisLauers(ChannelB, Sizes, ResultChannelB, QuantityLayers);

		for (int i = 0; i < Sizes[0]; i++)
			for (int j = 0; j < Sizes[1]; j++)
				if (ResultChannelB[i][j] == 1)
					ResultChannel[i][j] = 1;

		for (int i = 0; i < Sizes[0]; i++)
			delete[] ResultChannelB[i];
		delete[] ResultChannelB;
	}

	SavePictBlackAndWhite(ImgFileSave, ResultChannel, Sizes);

#pragma endregion Analysis Hierarchy

#pragma region Delete Channels

	for (int i = 0; i < Sizes[0]; i++)
		delete[] ChannelR[i];
	delete[] ChannelR;

	for (int i = 0; i < Sizes[0]; i++)
		delete[] ChannelG[i];
	delete[] ChannelG;

	for (int i = 0; i < Sizes[0]; i++)
		delete[] ChannelB[i];
	delete[] ChannelB;

	for (int i = 0; i < Sizes[0]; i++)
		delete[] ResultChannel[i];
	delete[] ResultChannel;

#pragma endregion Delete Channels
}