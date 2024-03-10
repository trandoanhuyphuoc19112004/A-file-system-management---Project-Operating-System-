#pragma once
#include "Utils.h"
#include <Windows.h>
#include <vector>
#include <cmath>
#include <map>


struct Directory 
{
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
    void readMFTEntry();
	void printFileEntry();
	void printNonResident(std::vector<DataRun>& data);
	int read$MFT();
	int searchDir(int left, int right, int ref);
	void addDir(Directory dir);
	void readEntry(int offsetMFT);
	void printDirectory(int rootreference);
	void printChosen(std::vector<ItemProperties> List);
};