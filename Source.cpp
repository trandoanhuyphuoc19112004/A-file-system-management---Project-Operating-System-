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
	std::cout << "AN FILE SYSTEM MANAGEMENT" << std::endl; 
	/*std::cout << "Enter your removable disks:"; 

	std::getline(std::wcin, drivename); 
	std::wstring path = L"\\\\.\\\\" + drivename + L":";
	FAT32 drive(path.c_str()); */

	std::wstring path = L"\\\\.\\\\F:";
	/*fat32 drive(path.c_str());
	print hex table 
	printhextable(drive.getbootsector(), 512); 

	std:: cout << "============================" << std::endl;
	drive.getdiskinformation();
	system("pause"); 
	drive.read(); 

	vector<itemproperties> list = drive.getlist(); 
	if (list.size() == 0)
	{
		std::cout << "no item in drive" << std::endl; 
		exit(0); 
	}
	else drive.printchosen(list);*/

	NTFS drive(path.c_str()); 
	drive.getDiskInformation(); 
	drive.read(); 
	
}