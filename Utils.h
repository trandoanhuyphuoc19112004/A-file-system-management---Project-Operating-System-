#pragma once
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <vector>

void printHexTable(const BYTE* sector, int size);
int64_t getByteValues(BYTE sector[512], int offset, int bytesize);
int ReadSector(LPCWSTR  drive, int readPoint, BYTE*& sector); 
std::string toString(BYTE* data, int offset, int number);
void getBPB(BYTE* sector, BYTE* BPB, int offset, int bytesize); 
struct ItemProperties
{
	std::string name;
	int size; 
	std::vector<int> clusters;
	bool isFile; // true is file, false is folder 
};