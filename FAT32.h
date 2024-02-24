#pragma once
#include "Utils.h"
#include <Windows.h>
#include <vector>
class FAT32
{
private:
	LPCWSTR _drive_name; 

	int _bytes_per_sector;
	int _sectors_per_cluster;
	int _sectors_in_bootsector;
	int _fat_num; 
	int _entry_num;
	int _volume_size;
	int _fat_table_size;
	int _first_sector_of_data;

	BYTE* BootSector; 
	BYTE* rdet; 
	BYTE* fat_table; 
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
public:
	void getRDET(); 
};


struct FileProperties
{
	std::string filename; 
	int filesize; 
	int startcluster; 
	int endcluster; 
};

struct FolderProperties
{
	std::string foldername; 
	int startcluster; 
	int endcluster; 
};