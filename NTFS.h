#pragma once
#include "Utils.h"
#include <Windows.h>
#include <vector>

class NTFS
{
private:
	LPCWSTR _drive_name; 

	int _bytes_per_sector;
	int _sectors_per_cluster;
	int64_t _sector_of_disk; 
	int64_t _begin_cluster_of_MFT;
	int _MFT_entry_size; 

	//int _sectors_in_bootsector;
	//int _fat_num; 
	//int _entry_num;
	//int _volume_size;
	//int _fat_table_size;
	//int _first_sector_of_data;

	BYTE* BootSector; 
	BYTE* MFT;

	
public:
	~NTFS(); 
	NTFS(LPCWSTR drivename);
public:
	void getDiskInformation(); 
public:
	 BYTE* getBootSector()
	{
		return BootSector;
	}
public:
	void read();
	void readMFT(); 
//	void readSDET(const ItemProperties& dir);
//	void readDIR(std::vector<ItemProperties>& list, int offsetDIR, int sector_index, int pointer);
//	void readFAT(std::vector<ItemProperties>& list, int offset_FatTable, int pointer_of_fattable, int sector_index_of_fat_table);
//	void printFolder(std::vector<ItemProperties>& dir);
//	void readTXT(const ItemProperties& file);
};