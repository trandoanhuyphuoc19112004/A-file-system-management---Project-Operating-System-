#pragma once
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
void printHexTable(const BYTE* sector);
int getByteValues(BYTE sector[512], int offset, int bytesize); 
int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512]); 
BYTE* clone_sector(const BYTE sector[512]); 