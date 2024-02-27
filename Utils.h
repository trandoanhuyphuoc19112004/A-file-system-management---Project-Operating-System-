#pragma once
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>

struct Node {
	int cluster;
	Node* pNext = nullptr;
};

void printHexTable(const BYTE* sector, int size);
int getByteValues(BYTE sector[512], int offset, int bytesize); 
int ReadSector(LPCWSTR  drive, int readPoint, BYTE*& sector); 
std::string toString(BYTE* data, int offset, int number); 
void deleteLL(Node* head);

struct ItemProperties
{
	~ItemProperties() {
		deleteLL(clusters);
	}
	std::string name;
	int size; 
	Node* clusters = nullptr;
	bool isFile; // true is file, false is folder 
};