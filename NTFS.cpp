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

void NTFS::read() {
	system("cls");
	std::cout << "======================" << std::endl;
	std::cout << "Some information about files or folders" << std::endl;

	this->MFT = new BYTE[512]; 
	int64_t offsetMFT = _begin_cluster_of_MFT * _bytes_per_sector * _sectors_per_cluster; 
	ReadSector(_drive_name, 4, MFT); 
	printHexTable(MFT, 512); 
	//this->rdet = new BYTE[512];
	//this->fat_table = new BYTE[512];

	//int offset_FatTable = _sectors_in_bootsector * _bytes_per_sector;


	//int offsetRDET = (_sectors_in_bootsector * _bytes_per_sector + 2 * _fat_table_size * _bytes_per_sector);

	//int pointer = 0;
	//int sector_index = 0;

	//std::vector<ItemProperties> list;

	//int pointer_of_fattable = 0;
	//int sector_index_of_fat_table = 0;

	//readDIR(list, offsetRDET, sector_index, pointer);
	//readFAT(list, offset_FatTable, pointer_of_fattable, sector_index_of_fat_table);
	//printFolder(list);
	//system("cls");
	//readTXT(list[0]);
	// system("cls");
}