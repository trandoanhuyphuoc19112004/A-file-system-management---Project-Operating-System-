#pragma once
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
void printHexTable(BYTE sector[512]);
int getByteValues(BYTE sector[512], int offset, int bytesize); 
int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512]); 
bool isLittleEndian();
