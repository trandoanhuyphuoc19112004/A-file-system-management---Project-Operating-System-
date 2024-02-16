#pragma once
#include "Utils.h"
#include <Windows.h>

class FAT32
{
private:
	LPCWSTR _drive_name; 

	int _bytes_per_sector;
	int _sectors_per_cluster;
	int _sectors_in_bootsector;
	int _fat_num; 
	int _entry_num;
	int _sectors_per_volume;
	int _volume_size;
	int _fat_table_size;
	int _first_sector_of_data;

	BYTE* BootSector; 
public:
	~FAT32(); 
	FAT32(LPCWSTR drivename);
public:
	void getDiskInformation(); 
public:
	 BYTE* getBootSector()
	{
		return BootSector; 
	}
};


