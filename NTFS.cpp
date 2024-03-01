#include "NTFS.h"
#include "Utils.h"


NTFS::NTFS(LPCWSTR drivename)
{
	this->_drive_name = drivename;
	this->BootSector = new BYTE[512];

	int error = 0;
	error = ReadSector(_drive_name, 0, this->BootSector);

	this->_bytes_per_sector = getByteValues(BootSector, 0x0B, 2);
	this->_sectors_per_cluster = getByteValues(BootSector, 0x0D, 1);
	this->_sector_of_disk = getByteValues(BootSector, 0x28, 8); 
	this->_begin_cluster_of_MFT = getByteValues(BootSector, 0x30, 8); 

	signed char number = 0; 
	memcpy(&number, BootSector + 0x40, 1);
	this->_MFT_entry_size = int(number);  // If size < 0, the formula is size = 2 ^ -size 
	if (_MFT_entry_size < 0)
		_MFT_entry_size = pow(2, abs(_MFT_entry_size)); 
}

NTFS::~NTFS()
{
	delete[] BootSector; 
	delete[] MFT; 
}

void NTFS::getDiskInformation()
{
	//std::cout << "Some information about this removable device " << _drive_name << std::endl;

	std::cout << "Bytes per sector:" << _bytes_per_sector << std::endl;
	std::cout << "Sectors per cluster:" << _sectors_per_cluster << std::endl;
	std::cout << "Sectors in disk:" << _sector_of_disk << std::endl;
	std::cout << "The begin cluster of MFT:" << _begin_cluster_of_MFT << std::endl;
	std::cout << "MFT entry size:" << _MFT_entry_size << std::endl; 
}

int NTFS::read$MFT() {
	this->MFT = new BYTE[1024]; 
	ReadSector(_drive_name, offsetMFT, MFT);
	BYTE* lastEntry = new BYTE[512];
	ReadSector(_drive_name, offsetMFT+512, lastEntry);
	memcpy(MFT+512, lastEntry, 512);
	delete lastEntry;
	readMFTEntry();
	delete[] MFT;
	
	int num_clusters = 64;
	// for(int i = 0; i < _list[0].data_runs.size(); i++) num_clusters += _list[0].data_runs[i].num_clusters;
	return num_clusters;
}

void NTFS::read() {
	system("cls");
	std::cout << "======================" << std::endl;
	std::cout << "Some information about files or folders" << std::endl;

	
	// printHexTable(MFT, 1024);
	int total_sector = read$MFT()*_bytes_per_sector*_sectors_per_cluster;
	offsetMFT = _begin_cluster_of_MFT * _bytes_per_sector * _sectors_per_cluster+1024;
	std::cout << total_sector << std::endl;
	while(total_sector > 0) {
		this->MFT = new BYTE[1024];
		ReadSector(_drive_name, offsetMFT, MFT);
		BYTE* lastEntry = new BYTE[512];
		ReadSector(_drive_name, offsetMFT+512, lastEntry);
		memcpy(MFT+512, lastEntry, 512);
		delete lastEntry;
		// printHexTable(MFT, 1024);
		readMFTEntry();
		offsetMFT += 1024;
		total_sector -= 1024;
		delete[] MFT;
	}
	printFileEntry();
	printDirectory();
}

void NTFS::readMFTEntry() {
	// std::cout << "ok" << std::endl;
    if(toString(MFT,0,4) != "FILE") return;
    if(getByteValues(MFT, 0x10, 2) == 0) return;
	FileEntry fileEntry;
	if(getByteValues(MFT, 0x16, 2) == 0x01) fileEntry.isDirectory = false;
	else if(getByteValues(MFT, 0x16, 2) == 0x03) fileEntry.isDirectory = true;
	else return;
	// std::cout << "ok" << std::endl;
    int startAtribute = getByteValues(MFT, 0x14, 2);
	fileEntry.fileReference = getByteValues(MFT, 0x2C, 4);
	while(getByteValues(MFT, startAtribute, 4) != 0xffffffff) {
		int attributeSize = getByteValues(MFT, startAtribute+4, 4);
		int startContent = getByteValues(MFT, startAtribute+20, 2);
		if(getByteValues(MFT, startAtribute, 4) == 0x10) { //$STANDARD_INFORMATION
			fileEntry.flag = getByteValues(MFT, startAtribute+startContent+32,4);
		} else if(getByteValues(MFT, startAtribute, 4) == 0x30) { //$FILE_NAME
			fileEntry.parentReference = getByteValues(MFT, startAtribute+startContent, 4);
			int nameLength = getByteValues(MFT, startAtribute+startContent+64, 1);
			int nameType = getByteValues(MFT, startAtribute+startContent+65,1);
			fileEntry.filename = toString(MFT, startAtribute+startContent+66, nameLength*2);
		} else if(getByteValues(MFT, startAtribute, 4) == 0x80) { //$DATA
			fileEntry.isResident = getByteValues(MFT, startAtribute+8,1) == 0;
			if(fileEntry.isResident) {
				int contentLength = getByteValues(MFT, startAtribute+16,4);
				int i = 0;
				while(contentLength > 0) {
					fileEntry.data.append(toString(MFT, startAtribute+startContent+i*512, contentLength > 512 ? 512 : contentLength));
					contentLength -= contentLength > 512 ? 512 : contentLength;
					i++;
				}
			} else {
				int offsetDataRun = getByteValues(MFT, startAtribute+32,2);
				while(getByteValues(MFT, startAtribute+startContent+offsetDataRun,2) != 0) {
					DataRun data;
					int byte_cluster = getByteValues(MFT, startAtribute+offsetDataRun, 1)/16;
					int byte_size = getByteValues(MFT, startAtribute+offsetDataRun, 1)%16;
					data.num_clusters = getByteValues(MFT, startAtribute+offsetDataRun+1, byte_size);
					data.startCluster = getByteValues(MFT, startAtribute+offsetDataRun+1+byte_size, byte_cluster);
					fileEntry.data_runs.push_back(data);
					offsetDataRun += 1 + byte_size + byte_cluster;
					
				}
			}
		}

		startAtribute += attributeSize;
	}
	EntryMap.insert({fileEntry.fileReference, _list.size()});
	_list.push_back(fileEntry);
	if(_dir_list.size() == 0 || searchDir(0, _dir_list.size()-1,fileEntry.parentReference) == -1) {
		Directory dir;
		dir.reference = fileEntry.parentReference;
		_dir_list.push_back(dir);
	}
	_dir_list[searchDir(0, _dir_list.size()-1,fileEntry.parentReference)].children.push_back(fileEntry.fileReference);
	if(fileEntry.isDirectory) {
		if(_dir_list.size() > 0) {
			if(searchDir(0, _dir_list.size()-1, fileEntry.fileReference) == -1) {
				Directory dir;
				dir.reference = fileEntry.fileReference;
				_dir_list.push_back(dir);
			}
		}
	}
	
}


int NTFS::searchDir(int left, int right, int ref) {
	if(left > right) return -1;
	int mid = (left + right) / 2;
	if(_dir_list[mid].reference == ref) return mid;
	else if(_dir_list[mid].reference > ref) return searchDir(left, mid-1, ref);
	else return searchDir(mid+1, right, ref);
}

void NTFS::printNonResident(std::vector<DataRun>& data) {
	for(int i = 0; i < data.size(); i++) {
		int sector_numnber = data[i].num_clusters*_sectors_per_cluster;
		int j = 0;
		while (sector_numnber > 0) {
			BYTE* sector = new BYTE[512];
			ReadSector(_drive_name, data[i].startCluster*_sectors_per_cluster*_bytes_per_sector+j*_bytes_per_sector, sector);
			std::cout << toString(sector, 0, 512) << std::endl;
			delete[] sector;
			j++;
			sector_numnber--;
		}
	}
}

void NTFS::printFileEntry() {
	for(int i = 0; i < _list.size(); i++) {
		// if(_list[i].flag == 2 || _list[i].flag == 6) continue;
		std::cout << "=====================" << std::endl;
		std::cout << "No: " << i<< std::endl;
		std::cout << "Name: " << _list[i].filename << std::endl;
		std::cout << "Type: " << (_list[i].isDirectory ? "directory" : "file") << std::endl;
		std::cout << "Flag: " << _list[i].flag <<std::endl;
		std::cout << "File Ref: " << _list[i].fileReference <<std::endl;
		std::cout << "Parent: " << _list[i].parentReference << std::endl;
		std::cout << "Resident: " << _list[i].isResident << std::endl;
	}
}
void NTFS::printDirectory() {
	for(int i = 0; i < _dir_list.size(); i++) {
		// if(_list[i].flag == 2 || _list[i].flag == 6) continue;
		std::cout << "=====================" << std::endl;
		std::cout << "Dir: " << _dir_list[i].reference << std::endl;
		for(int j = 0; j < _dir_list[i].children.size(); j++) {
			std::cout << "Ref: " << _dir_list[i].children[j]<< std::endl;

		}
	}
}