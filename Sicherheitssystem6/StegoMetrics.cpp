#include "stdafx.h"

// поиск НОД(a, b)
int Stego::NOD(int a, int b)
{
	if (b == 0) return a;
	return NOD(b, a % b);
}

int Stego::CountGCD1(int n, int start)
{
	int count = 0;
	for (int i = start; i < n; i++)
		if (NOD(n, i) == 1)
			count++;

	return count;
}

int Stego::LimitPotentialBytes(int ImageBytes)
{
	// сколько бит можно зашить?
	int nBits = ImageBytes * LastBits - BitsForSize;

	return nBits / 8;
}

double GetKeyPotential(char ImgFile[], int MaxLastBits)
{
	Stego stg;

	// количество вариантов выбрать стартовый байт
	double n1 = SizeOfImageByte(ImgFile);
	
	// количество вариантов выбрать шаг байта
	double n2 = stg.CountGCD1(n1, 2);

	// количество вариантов выбрать пропуск
	double n3 = n2;
	if (int(n1) % 2 == 0) n3--;
	
	return n1 * n2 * n3 * double(MaxLastBits);
}

int GetImageCapacity(char ImgFile[], int LastBits)
{
	Stego stg(0, 0, LastBits, 0);

	int nBytes = SizeOfImageByte(ImgFile);

	return stg.LimitPotentialBytes(nBytes);
}