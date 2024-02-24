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

	std::vector<FileProperties> list;
	std::vector<FolderProperties> list2;

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
			FileProperties item;
			FolderProperties item2;
			if (pointer == 512)
			{
				pointer = 0;
				sector_index++;
				break;
			}

			if (getByteValues(rdet, pointer + 0x0B, 1) == 0x00)
				break;

			if (getByteValues(rdet, pointer + 0x0B, 1) == 0x0F) //Entry phu
			{
				name = toString(rdet, pointer + 1, 10) + toString(rdet, pointer + 0x0E, 12) + toString(rdet, pointer + 0x1C, 4) + name;

			}

			else
				if (getByteValues(rdet, pointer + 0x0B, 1) == 0x10) // Entry chinh, folder 
				{
					if (name == " ")
					{
						item2.foldername = toString(rdet, pointer, 8);
						std::cout << item2.foldername << std::endl;
					}
					else
					{
						item2.foldername = name;
						name = " ";
					}
					item2.startcluster = getByteValues(rdet, pointer + 0x1A, 2);
					item2.endcluster = 0;
					list2.push_back(item2);
				}

				else
					if (getByteValues(rdet, pointer + 0x0B, 1) == 0x16) // Entry chinh, system folder 
					{
						if (name == " ")
						{
							item2.foldername = toString(rdet, pointer, 8);
							std::cout << item2.foldername << std::endl;
						}
						else
						{
							item2.foldername = name;
							name = " ";
						}
						item2.startcluster = 0;
						item2.endcluster = 0;
						list2.push_back(item2);
					}

					else
						if (getByteValues(rdet, pointer + 0x0B, 1) == 0x20) // Entry chinh, file
						{
							if (name == " ")
							{
								item.filename = toString(rdet, pointer, 8) + "." + toString(rdet, pointer + 8, 3);
							}
							else
							{
								item.filename = name;
								name = " ";
							}
							item.filesize = getByteValues(rdet, pointer + 0x1C, 1);
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
		std::cout << "Loop" << std::endl;
		std::cout << sector_index_of_fat_table << std::endl; 
		ReadSector(_drive_name, offset_FatTable + sector_index_of_fat_table * 512, fat_table);
		if (getByteValues(fat_table, pointer_of_fattable, 4) == 0x00000000)
		{
			std::cout << "End of loop" << std::endl;
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

			else
			{
				while (getByteValues(fat_table, pointer_of_fattable, 4) != 0x0fffffff && pointer_of_fattable !=512)
				{
					endcluster = getByteValues(fat_table, pointer_of_fattable, 4);
					std::cout << "End cluster:" << endcluster << std::endl;
					pointer_of_fattable += 4;
					std::cout << "Pointer:" << pointer_of_fattable << std::endl;
				}
				list[idx].endcluster = endcluster - 1; 
				idx++; 
			}
		} while (true); 

		//while (true)
		//{
		//	if (pointer_of_fattable == 512)
		//	{
		//		pointer_of_fattable = 0;
		//		sector_index_of_fat_table++;
		//		break;
		//	}

		//	while (getByteValues(fat_table, pointer_of_fattable, 4) != 0x0fffffff)
		//	{
		//		if (pointer_of_fattable == 512)
		//			break;
		//		int endcluster = getByteValues(fat_table, pointer_of_fattable, 4);
		//		std::cout << endcluster << std::endl;
		//		pointer_of_fattable += 4;
		//	}
		//	pointer_of_fattable += 4; 
		//}





		//pointer_of_fattable += 4; 

		/*for (int i = 0; i < list.size(); i++)
		{
			while (getByteValues(fat_table, pointer_of_fattable, 4) != 0x0fffffff)
			{
				list[i].endcluster = getByteValues(fat_table, pointer_of_fattable, 4);
				std::cout << list[i].endcluster << std::endl;
				pointer_of_fattable += 4;
			}
			std::cout << pointer_of_fattable << std::endl; 
			pointer_of_fattable += 8;
		}*/
		system("Pause");
	}

	
	
	 //Print information about file:
	std::cout << "Type:File" << std::endl;
	for (int i = 0; i < list.size(); i++)
	{
		std::cout << "======================" << std::endl;
		std::cout << "File no:" << i + 1 << std::endl; 
		std::cout << "File name:" << list[i].filename << std::endl;
		std::cout << "File size:" << list[i].filesize << std::endl;
		std::cout << "The start cluster:" << list[i].startcluster << std::endl;
		std::cout << "The end cluster:" << list[i].endcluster << std::endl; 
	}
	system("Pause");
	system("cls");

	//// Print information about folder: 
	//std::cout << "Type:Folder" << std::endl;
	//for (int i = 0; i < list2.size(); i++)
	//{
	//	std::cout << "======================" << std::endl;
	//	std::cout << "Folder no:" << i + 1 << std::endl;
	//	std::cout << "Folder name:" << list2[i].foldername << std::endl;
	//	std::cout << "The start cluster:" << list2[i].startcluster << std::endl;
	//	std::cout << "The end cluster:" << list2[i].endcluster << std::endl;
	//}
}