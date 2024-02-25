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

void FAT32::getRDET()
{
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

	std::string name = " ";

	// Read RDET and FAT Table 
	while (true)
	{
		ReadSector(_drive_name, offsetRDET + sector_index * 512, rdet);
		if (getByteValues(rdet, pointer + 0x0B, 1) == 0x00)
			break;
		while (true)
		{
			ItemProperties item;
			if (pointer == 512)
			{
				pointer = 0;
				sector_index++;
				break;
			}
			
			if (getByteValues(rdet, pointer + 0x0B, 1) == 0x00)
				break;
			
			if (getByteValues(rdet, pointer, 1) == 0xE5) // Ignore deleted file are stil exsiting in rdet 
			{
				pointer += 32;
				continue;
			} 

			if (getByteValues(rdet, pointer + 0x0B, 1) == 0x0F) //Entry phu
			{
				name = toString(rdet, pointer + 1, 10) + toString(rdet, pointer + 0x0E, 12) + toString(rdet, pointer + 0x1C, 4) + name;

			}

			else
				if (getByteValues(rdet, pointer + 0x0B, 1) == 0x10) // Entry chinh, folder 
				{
					item.isFile = false; 
					if (name == " ")
					{
						item.name = toString(rdet, pointer, 8);
						std::cout << item.name << std::endl;
					}
					else
					{
						item.name = name;
						name = " ";
					}
					item.size = 0; 
					item.startcluster = getByteValues(rdet, pointer + 0x1A, 2);
					item.endcluster = 0;
					list.push_back(item);
				}

				else
					if (getByteValues(rdet, pointer + 0x0B, 1) == 0x16) // Entry chinh, system folder 
					{
						if (name == " ")
						{
							item.name = toString(rdet, pointer, 8);
							std::cout << item.name << std::endl;
						}
						else
						{
							item.name = name;
							name = " ";
						}
						item.startcluster = 0;
						item.endcluster = 0;
					}

					else
						if (getByteValues(rdet, pointer + 0x0B, 1) == 0x20) // Entry chinh, file
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
							item.size = getByteValues(rdet, pointer + 0x1C, 1);
							item.startcluster = getByteValues(rdet, pointer + 0x1A, 2);
							item.endcluster = 0;
							list.push_back(item);
						}

			pointer += 32;
		}
	}


	//read fat table second 
	int idx = 0; 
	int endcluster; 
	while (true)
	{
		system("cls");
		std::cout << "Sector index of fat table:" << sector_index_of_fat_table << std::endl;
		ReadSector(_drive_name, offset_FatTable + sector_index_of_fat_table * 512, fat_table);
		if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x00000000)
		{
			break;
		}
		printHexTable(fat_table, 512);
		std::cout << "===============" << std::endl;

		do
		{
			if (pointer_of_fattable == 512)
			{
				pointer_of_fattable = 0;
				sector_index_of_fat_table++;
				break;
			}

			if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x0ffffff8 || getByteValues(fat_table, pointer_of_fattable, 4) == 0x0fffffff || getByteValues(fat_table, pointer_of_fattable, 4) == 0xffffffff)
			{
				pointer_of_fattable += 4;
				continue;
			}
			else if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x0ffffff7)
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
				while (getByteValues(fat_table, pointer_of_fattable, 4) != 0x0fffffff && pointer_of_fattable != 512)
				{
					endcluster = getByteValues(fat_table, pointer_of_fattable, 4);
					pointer_of_fattable += 4;
				}
			
				list[idx].endcluster = endcluster - 1;
				if (pointer_of_fattable != 512) 
					idx++;
			}
		} while (true);

		system("Pause"); 
	}
	
	 //Print information about file
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
		std::cout << "The start cluster:" << list[i].startcluster << std::endl;
		std::cout << "The end cluster:" << list[i].endcluster << std::endl; 
	}
	system("Pause");
	system("cls");

}