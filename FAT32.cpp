#include "FAT32.h"
#include "Utils.h"

void FAT32::getDiskInformation()
{
	std::cout << "Some information about this removable device " << _drive_name << std::endl;
	std::cout << "Bytes per sector:" << _bytes_per_sector << std::endl;
	std::cout << "Sectors per cluster:" << _sectors_per_cluster << std::endl; 
	std::cout << "Sectors in bootsector:" << _sectors_in_bootsector << std::endl; 

	std::cout << "The number of fat:" << _fat_num << std::endl; 
	std::cout << "Fat table size:" << _fat_table_size << std::endl;

	std::cout << "The number of entries:" << _entry_num << std::endl; 
	std::cout << "The sectors of RDETS:" << _first_sector_of_data << std::endl;

	std::cout << "Volume size:" << _volume_size << std::endl; 

	std::cout << "First sector of data:" << _first_sector_of_data << std::endl;
	 
}

FAT32::~FAT32()
{
	delete[] BootSector;
	delete[] rdet; 
	delete[] fat_table; 
}

FAT32::FAT32(LPCWSTR drivename)
{
	this->_drive_name = drivename;
	this->BootSector = new BYTE[512];
	this->rdet = nullptr; 

	int error = 0;
	error = ReadSector(_drive_name, 0, this->BootSector);
	
	
	this->_bytes_per_sector = getByteValues(BootSector, 0x0B, 2);
	this->_sectors_per_cluster = getByteValues(BootSector, 0x0D, 1); 
	this->_sectors_in_bootsector = getByteValues(BootSector, 0x0E, 2); 

	this->_fat_num = getByteValues(BootSector, 0x10, 1); 
	this->_fat_table_size = getByteValues(BootSector, 0x24, 4);

	this->_entry_num = getByteValues(BootSector, 0x11, 2); 

	this->_volume_size = getByteValues(BootSector, 0x20, 4); 

	this->_first_sector_of_data = _sectors_in_bootsector + _fat_table_size * _fat_num;
}

void FAT32::read() {
	system("cls");
	std::cout << "======================" << std::endl;
	std::cout << "Some information about files or folders" << std::endl;

	this->rdet = new BYTE[512];
	this->fat_table = new BYTE[512];

	int offset_FatTable = _sectors_in_bootsector * _bytes_per_sector;


	int offsetRDET = (_sectors_in_bootsector * _bytes_per_sector + 2 * _fat_table_size * _bytes_per_sector);

	int pointer = 0;
	int sector_index = 0;

	std::vector<ItemProperties> list;

	int pointer_of_fattable = 0;
	int sector_index_of_fat_table = 0;

	readDIR(list, offsetRDET, sector_index, pointer);
	readFAT(list, offset_FatTable, pointer_of_fattable, sector_index_of_fat_table);
	printFolder(list);
	system("cls");
	readTXT(list[0]);
	// system("cls");
}

void FAT32::readDIR(std::vector<ItemProperties>& list, int offsetDIR, int sector_index, int pointer)
{
	// Read RDET and FAT Table
	std::string name = " ";

	while (true)
	{
		ReadSector(_drive_name, offsetDIR + sector_index * 512, rdet);
		printHexTable(rdet, 512);
		if (getByteValues(rdet, pointer + 0x0B, 1) == 0x00)
			break;
		while (true)
		{
			ItemProperties item;

			// Out condition
			if (pointer == 512)
			{
				pointer = 0;
				sector_index++;
				break;
			}
			if (getByteValues(rdet, pointer, 1) == 0xE5) // Ignore deleted file are stil exsiting in rdet 
			{
				pointer += 32;
				continue;
			}
			std::cout << pointer << std::endl;
			if (getByteValues(rdet, pointer, 1) == 0x00)
				break;

			if (getByteValues(rdet, pointer + 0x0B, 1) == 0x0F) //Entry phu
			{
				name = toString(rdet, pointer + 1, 10) + toString(rdet, pointer + 0x0E, 12) + toString(rdet, pointer + 0x1C, 4) + name;

			}

			else if (getByteValues(rdet, pointer + 0x0B, 1) == 0x10) // Entry chinh, folder 
			{

				item.isFile = false; 
				if (name == " ")
				{
					item.name = toString(rdet, pointer, 8);
					// std::cout << item.name << std::endl;
				}
				else
				{
					item.name = name;
					name = " ";
				}
				item.size = 0;
				item.clusters.push_back(getByteValues(rdet, pointer + 0x1A, 2));
				std::cout << "ok1" << std::endl;
				list.push_back(item);
				std::cout << "ok2" << std::endl;
				
			}

			else if (getByteValues(rdet, pointer + 0x0B, 1) == 0x16) // Entry chinh, system folder 
			{
				if (name == " ")
				{
					item.name = toString(rdet, pointer, 8);
					// std::cout << item.name << std::endl;
				}
				else
				{
					item.name = name;
					name = " ";
				}
			}

			else if (getByteValues(rdet, pointer + 0x0B, 1) == 0x20) // Entry chinh, file
			{
				item.isFile = true;
				if (name == " ")
				{
					item.name = toString(rdet, pointer, 8) + "." + toString(rdet, pointer + 8, 3);
				}
				else
				{
					item.name = name;
					name = " ";
				}
				item.size = getByteValues(rdet, pointer + 0x1C, 4);
				item.clusters.push_back(getByteValues(rdet, pointer + 0x1A, 2));
				list.push_back(item);
			}

			pointer += 32;
			std::cout << "ok" << std::endl;

		}
	}
	std::cout << "ok" << std::endl;
}

void FAT32::readFAT(std::vector<ItemProperties>& list, int offset_FatTable, int pointer_of_fattable, int sector_index_of_fat_table) {
	//read fat table second 
	int idx = 0; 
	int endcluster = 0 ; 
	while (true)
	{
		// system("cls");
		
		// std::cout << "Sector index of fat table:" << sector_index_of_fat_table << std::endl;
		ReadSector(_drive_name, offset_FatTable + sector_index_of_fat_table * 512, fat_table);
		// printHexTable(fat_table, 512);
		// std::cout << "===============" << std::endl;
		
		if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x0ffffff7)
		{
			std::cout << "fat table cannot be read" << std::endl;
			break;
		}
		else if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x00000000)
		{
			break;
		}
		else
		{
			int cluster_id = 0;
			pointer_of_fattable = list[idx].clusters[cluster_id]* 4 - sector_index_of_fat_table*512;
			if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x0fffffff)
			{
				idx++;
			}
			else
			{
				while (getByteValues(fat_table, pointer_of_fattable, 4) != 0x0fffffff )
				{
					list[idx].clusters.push_back(getByteValues(fat_table, pointer_of_fattable, 4));
					pointer_of_fattable = list[idx].clusters[++cluster_id]*4 - sector_index_of_fat_table*512;
					std::cout << "cluster: " << cluster_id << " " << list[idx].clusters[cluster_id] << std::endl;
					if (pointer_of_fattable == 512) 
					{
						pointer_of_fattable = 0;
						sector_index_of_fat_table++;
						ReadSector(_drive_name, offset_FatTable + sector_index_of_fat_table * 512, fat_table);
					}
				}
				idx++;
			}
		}
		if (idx == list.size())
			break;
	}
}

void FAT32::printFolder(std::vector<ItemProperties>& list) {
	for (int i = 0; i < list.size(); i++)
	{
		std::cout << "======================" << std::endl; 
		std::cout << "No:" << i + 1 << std::endl; 
		if (list[i].isFile == true)
			std::cout << "Type: File" << std::endl;
		else
			std::cout << "Type:Folder" << std::endl;
		std::cout << "Name:" << list[i].name << std::endl;
		std::cout << "Size:" << list[i].size << std::endl;
		std::cout << "The start cluster:" << list[i].clusters[0] << std::endl;
		std::cout << "Sector range:" << std::endl;

	}
	std::cout << std::endl; 
	system("Pause");
}

void FAT32::readSDET(const ItemProperties& dir) {
	system("cls");
	std::cout << "======================" << std::endl;
	std::cout << "Some information about files or folders in " << dir.name << std::endl;

	this->rdet = new BYTE[512];
	this->fat_table = new BYTE[512];

	int offset_FatTable = _sectors_in_bootsector * _bytes_per_sector;


	int offsetDIR = (_sectors_in_bootsector * _bytes_per_sector + 2 * _fat_table_size * _bytes_per_sector) + ((dir.clusters[0]-2)*_sectors_per_cluster*_bytes_per_sector);

	int pointer = 0;
	int sector_index = 0;

	std::vector<ItemProperties> list;

	int pointer_of_fattable = 0;
	int sector_index_of_fat_table = 0;

	readDIR(list, offsetDIR, sector_index, pointer);
	readFAT(list, offset_FatTable, pointer_of_fattable, sector_index_of_fat_table);
	printFolder(list);
	system("cls");
}

void FAT32::readTXT(const ItemProperties& file) {
	int remain = file.size;
	std::string data = "";
	int id = 0;
	int cluster_id = 0;
	int byte_per_cluster = _sectors_per_cluster*_bytes_per_sector;
	while(remain > 0) {
		int offsetFile = (_sectors_in_bootsector * _bytes_per_sector + 2 * _fat_table_size * _bytes_per_sector) + ((file.clusters[cluster_id]-2)*byte_per_cluster) + 512*id;
		ReadSector(_drive_name, offsetFile, rdet);
		data.append(toString(rdet, 0, remain > 512 ? 512 : remain));
		remain -= 512;
		id++;
		if(id == _sectors_per_cluster) {
			cluster_id++;
			id = 0;
		}
	}
	system("cls");
	std::cout << data << std::endl;
	system("pause");
}