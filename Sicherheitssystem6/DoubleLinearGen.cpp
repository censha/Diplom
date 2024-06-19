#include "stdafx.h"

Int64 DoubleLinearGen::SetRR(Int64 Value, int Shift)
{
	Int64 mask = pow2(Shift) - 1;

	// последние Shift бит

	//int a = 5 | 2;          // 101 | 010 = 111  - 7
	//int b = 6 & 2;          // 110 & 010 = 10  - 2
	//int c = 5 ^ 2;          // 101 ^ 010 = 111 - 7

	Int64 LastBits = Value & mask;

	//int a = 2 << 2;           // 10  на два разрядов влево = 1000 = 8
	//int b = 16 >> 3;          // 10000 на три разряда вправо = 10 = 2

	// первые 24-Shift бит становятся последними
	Value = Value >> Shift;

	// последние Shift бит становятся первыми
	Int64 FirstBits = LastBits << (24 - Shift);
	Value = Value | FirstBits;

	return Value;
}

void DoubleLinearGen::GetNumbers(COMPLEXARRAY* p, int HowMany)
{
	MathcadArrayAllocate(p, HowMany / VALUES_PER_COLUMN, VALUES_PER_COLUMN, TRUE, FALSE);

	for (int i = 0; i < HowMany; i++)
	{
		// сначала столбец, потом строка
		p->hReal[i % VALUES_PER_COLUMN][i / VALUES_PER_COLUMN] = GetNext();
	}
}