#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "Utils.h"
#include "FAT32.h"
#include "NTFS.h"

int main(int argc, char** argv)
{
	std::wstring drivename; 
	BYTE* bootsector;
	bootsector = new BYTE[512]; 
	std::cout << "AN FILE SYSTEM MANAGEMENT" << std::endl; 

	std::cout << "Enter your removable disks:"; 

	std::getline(std::wcin, drivename); 
	std::wstring path = L"\\\\.\\\\" + drivename + L":"; 

	std::string formatname; 
	ReadSector(path.c_str(), 0, bootsector); 
	formatname = toString(bootsector, 0x52, 8);

	if (formatname == "FAT32   ") //FAT32
	{
		FAT32 drive(path.c_str());
		//print hex table
		printHexTable(drive.getBootSector(), 512);
		std::cout << "============================" << std::endl;
		drive.getDiskInformation();
		system("Pause");
		drive.read();
		vector<ItemProperties> list = drive.getList();
		if (list.size() == 0)
		{
			std::cout << "no item in drive" << std::endl;
			system("Pause");
			exit(0);
		}
		else drive.printChosen(list);
	}
	else //NTFS 
	{
		NTFS drive(path.c_str()); 
		drive.getDiskInformation();
		system("Pause"); 
		drive.read(); 
	}
	delete[] bootsector; 
}