#include "FAT32.h"
#include "Utils.h"

void FAT32::getDiskInformation()
{
	std::cout << "Some information about this removable device " << _drive_name << std::endl;
	std::cout << "Bytes per sector:" << _bytes_per_sector << std::endl;
	std::cout << "Sectors per cluster:" << _sectors_per_cluster << std::endl; 
	std::cout << "Sectors in bootsector:" << _sectors_in_bootsector << std::endl; 
	std::cout << "The number of fat:" << _fat_num << std::endl; 
	std::cout << "The number of entries:" << _entry_num << std::endl; 
	std::cout << "Sectors per volume:" << _sectors_per_volume << std::endl; 
	std::cout << "Volume size:" << _volume_size << std::endl; 
	std::cout << "Fat table size:" << _fat_table_size << std::endl; 
}


FAT32::~FAT32()
{
	delete[] BootSector;
}

FAT32::FAT32(LPCWSTR drivename)
{
	this->_drive_name = drivename;
	this->BootSector = new BYTE[512];

	int error = 0;
	error = ReadSector(_drive_name, 0, this->BootSector);

	
	this->_bytes_per_sector = getByteValues(BootSector, 0x0B, 2);
	this->_sectors_per_cluster = getByteValues(BootSector, 0x0D, 1); 
	this->_sectors_in_bootsector = getByteValues(BootSector, 0x0E, 2); 
	this->_fat_num = getByteValues(BootSector, 0x10, 1); 
	this->_entry_num = getByteValues(BootSector, 0x11, 2); 
	this->_sectors_per_volume = getByteValues(BootSector, 0x13, 2); 
	this->_volume_size = getByteValues(BootSector, 0x20, 4); 
	this->_fat_table_size = getByteValues(BootSector, 0x24, 4); 
}