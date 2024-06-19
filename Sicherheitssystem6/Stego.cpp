#include "stdafx.h"
#include <gdipluscolor.h>

void Stego::BitsToBool(int x, bool A[], int pos)
{
	for (int i = 0; i < 8; i++)
	{
		// ! идем от конца подмассива A[pos...pos+7] к началу
		A[pos + 7 - i] = (x % 2 == 1);
		x /= 2;
	}
}

// выгрузка цвета пикселя в массив А
void Stego::ColorToBool(Color Clr, bool A[], int pos)
{
	BitsToBool(Clr.GetR(), A, pos);
	BitsToBool(Clr.GetG(), A, pos + 8);
	BitsToBool(Clr.GetB(), A, pos + 16);
}

// представление изображения как набора бит
bool * Stego::BitmapToBoolArray(char FileName[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// инициализируем GDI ---
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap * bm = LoadBmp(FileName);

	int h = bm->GetHeight(), w = bm->GetWidth();

	bool * result = new bool[24 * h * w];

	int pos = 0; // позиция записи текущего пикселя в result
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			Color clr;

			// цвет в i-ой строке j-ом столбце матрицы пикселей
			bm->GetPixel(i, j, &clr);

			// выгружаем этот цвет в массив result в виде набора бит
			ColorToBool(clr, result, pos);

			pos += 24;
		}

	Sizes[0] = w;
	Sizes[1] = h;
	
	delete bm;

	return result;
}

// получение позиции первого используемого бита
int Stego::PosOfStartBit()
{
	// номер последнего бита первого используемого байта
	// в общем ряду бит изображения
	int TailBit = 8 * StartByte + 7;

	// номер первого используемого бита в этом же байте
	// и есть нужный бит
	return TailBit - LastBits + 1;
}

// получение позиции следующего за cur используемого бита
int Stego::PosOfNextBit(int cur, int ContainerSize)
{
	// если cur-ый бит не последний в своем байте
	if (cur % 8 != 7) return cur + 1;

	// если последний

	// запоминаем, что текущий байт был полностью использован
	this->UsedBytes++;

	// переносимся с заданным шагом в хвост следующего используемого байта
	cur += StepByte * 8;

	// учитываем количество используемых младших бит
	return (cur - LastBits + 1) % ContainerSize;
}

bool * Stego::GetFalseArray(int n)
{
	bool * fg = new bool[n];
	for (int i = 0; i < n; i++)
		fg[i] = false;
	return fg;
}

// зашитие n-битного целочисленного значения
// cur - первый используемый бит,
// функция возвращает бит, с которого будет
// зашиваться следующее значение
int Stego::HideValue(int Value, int nBits, bool ContainerBits[], int cLength, int cur,
	bool IsUsed[])
{
	DoubleLinearGen G;

	// вес старшего бита у n-битного числа
	int w = (int)G.pow2(nBits - 1);

	for (int i = 0; i < nBits; i++)
	{
		// определяем i-ый по старшинству бит
		ContainerBits[cur] = ((Value & w) != 0);
		IsUsed[cur] = true;

		int OldUsedBytes = this->UsedBytes;

		// определяем, куда зашивать следующий бит
		cur = PosOfNextBit(cur, cLength);

		if (UsedBytes != OldUsedBytes && UsedBytes % SkipStep == SkipStep - 1)
		{
			do
			{
				cur = PosOfNextBit(cur, cLength);			
			}
			while (IsUsed[cur] == true);
		}		

		// уменьшаем вес бита
		w /= 2;
	}

	return cur;
}

// зашитие байт в массив бит контейнера (включая размер)
void Stego::HideMsg(BYTE MyMsg[], int nBytes, bool ContainerBits[], int cLength)
{
	bool * fg = GetFalseArray(cLength);

	int cur = PosOfStartBit(); // позиция стартового бита зашивки
	
	// зашиваем размер
	cur = HideValue(nBytes, BitsForSize, ContainerBits, cLength, cur, fg);

	// зашиваем коды символов
	for (int i = 0; i < nBytes; i++)
	{
		cur = HideValue((int)MyMsg[i], 8, ContainerBits, cLength, cur, fg);
	}

	delete[] fg;
}

// получение байта из подмассива A[pos...pos+7]
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

// получение цвета пикселя из подмассива A[pos...pos+23]
void Stego::BoolToColor(bool A[], int pos, Color * c)
{
	int r = BoolToInt(A, pos);
	int g = BoolToInt(A, pos + 8);
	int b = BoolToInt(A, pos + 16);

	c->SetFromCOLORREF(RGB(r, g, b));
}

// сохранение изображения
void Stego::SavePict(char FileName[], bool bits[], int Sizes[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// инициализируем GDI
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int w = Sizes[0], h = Sizes[1];

	Bitmap bm(w, h);

	// позиция для восстановления из bits текущего пикселя
	int pos = 0;

	Color c;

	// по массиву бит строим пиксели
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			BoolToColor(bits, pos, &c);
			bm.SetPixel(i, j, c);

			// следующий пиксель - на 24 бита вправо
			pos += 24;
		}

	SaveBmp(FileName, &bm);
}

// основной метод зашития
int Stego::HideData(char FileMsg[], char ImgFile[], char ResultFile[])
{
	this->UsedBytes = 0;

	int Sizes[2]; // для запоминания размеров изображения

	// представляем изображение из файла ImgFile как набор бит
	bool * ContainerBits = BitmapToBoolArray(ImgFile, Sizes);

	BytesArray b(FileMsg);

	// зашиваем текст в массив бит
	HideMsg(b.bytes, b.n, ContainerBits, 24 * Sizes[0] * Sizes[1]);

	// сохраняем изображение, представленное набором бит
	SavePict(ResultFile, ContainerBits, Sizes);

	delete[] ContainerBits;

	return b.n;
}

// восстановление n-битного целочисленного значения
// cur - первый используемый бит
int Stego::RestoreValue(int nBits, bool ContainerBits[], int cLength, int cur, int Value[],
	bool IsUsed[])
{
	DoubleLinearGen G;

	// вес старшего бита у n-битного числа
	int w = (int)G.pow2(nBits - 1);

	Value[0] = 0;

	for (int i = 0; i < nBits; i++)
	{
		// если i-ый по старшинству бит единичный
		if (ContainerBits[cur] == true)
			Value[0] += w; // рассматриваем значение как сумму весов единич. бит
		IsUsed[cur] = true;

		int OldUsedBytes = this->UsedBytes;

		// определяем, где зашит следующий бит
		cur = PosOfNextBit(cur, cLength);

		if (UsedBytes != OldUsedBytes && UsedBytes % SkipStep == SkipStep - 1)
		{
			do
			{
				cur = PosOfNextBit(cur, cLength);
			} while (IsUsed[cur] == true);
		}

		// уменьшаем вес бита
		w /= 2;
	}

	return cur;
}

// восстановление байт из массива бит контейнера
BytesArray * Stego::RestoreData(bool ContainerBits[], int cLength)
{
	bool * fg = GetFalseArray(cLength);

	int cur = PosOfStartBit(); // позиция стартового бита зашивки

	int msize = 0, Value[1];

	// восстанавливаем размер
	cur = RestoreValue(BitsForSize, ContainerBits, cLength, cur, Value, fg);
	msize = Value[0];

	BytesArray * msg = new BytesArray(msize);

	// восстанавливаем байты
	for (int i = 0; i < msize; i++)
	{
		cur = RestoreValue(8, ContainerBits, cLength, cur, Value, fg);
		msg->bytes[i] = (BYTE)Value[0];
	}

	delete[] fg;

	return msg;
}

// восстановление данных
int Stego::RestoreData(char ImgFile[], char ResultFile[])
{
	this->UsedBytes = 0;

	int Sizes[2]; // для запоминания размеров изображения

	// представляем изображение из файла ImgFile как набор бит
	bool * ContainerBits = BitmapToBoolArray(ImgFile, Sizes);

	// восстанавливаем данные по массиву бит
	BytesArray * Restored = RestoreData(ContainerBits, 24 * Sizes[0] * Sizes[1]);

	// сохраняем данные в файл
	Restored->Write(ResultFile);

	delete[] ContainerBits;
	int result = Restored->n;
	delete Restored;
	return result;
}