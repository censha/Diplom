#include "stdafx.h"
#include <fstream>

using namespace std;

BytesArray::BytesArray(char FileName[])
{
	ifstream is(FileName, ifstream::binary);
	is.seekg(0, is.end);
	n = is.tellg();
	is.seekg(0, is.beg);
	char * chars = new char[n];
	bytes = new BYTE[n];
	is.read(chars, n);
	is.close();
	CharsToBytes(chars);
	delete[] chars;
}

void BytesArray::Write(char FileName[])
{
	ofstream os(FileName, ofstream::binary);
	char * chars = new char[n];
	BytesToChars(chars);
	os.write(chars, n);
	os.close();
	delete[] chars;
}

int BytesArray::CheckSize(char FileName[])
{
	ifstream is(FileName, ifstream::binary);
	is.seekg(0, is.end);
	n = is.tellg();
	is.close();
	return n;
}