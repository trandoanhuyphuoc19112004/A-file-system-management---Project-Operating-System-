#pragma once
#include "Utils.h"
#include <Windows.h>
#include <vector>
#include <cmath>
#include <map>


	struct Directory {
		std::vector<int> children;
		int reference;
	};
class NTFS
{
private:
	LPCWSTR _drive_name; 
	int _root_reference;
	int _bytes_per_sector;
	int _sectors_per_cluster;
	int64_t _sector_of_disk; 
	int64_t _begin_cluster_of_MFT;
	int _MFT_entry_size; 
	std::vector<FileEntry> _list;
	std::vector<Directory> _dir_list;
	//int _sectors_in_bootsector;
	//int _fat_num; 
	//int _entry_num;
	//int _volume_size;
	//int _fat_table_size;
	//int _first_sector_of_data;

	BYTE* BootSector; 
	BYTE* MFT = nullptr;
    int64_t offsetMFT;
	

	std::map<int,int> EntryMap;
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
	// void readMFT();
    void readMFTEntry();
	void printFileEntry();
	void printNonResident(std::vector<DataRun>& data);
	int read$MFT();
	void createDirTree();
	int searchDir(int left, int right, int ref);
	void printDirectory();
    // void readAttribute(int);
//	void readSDET(const ItemProperties& dir);
//	void readDIR(std::vector<ItemProperties>& list, int offsetDIR, int sector_index, int pointer);
//	void readFAT(std::vector<ItemProperties>& list, int offset_FatTable, int pointer_of_fattable, int sector_index_of_fat_table);
//	void printFolder(std::vector<ItemProperties>& dir);
//	void readTXT(const ItemProperties& file);
};