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
	this->_MFT_entry_size = int(number);  // If size < 0, the formula is size = 2 ^ abs size 
	if (_MFT_entry_size < 0)
		_MFT_entry_size = pow(2, abs(_MFT_entry_size));
}

NTFS::~NTFS()
{
	delete[] BootSector;
	if (MFT != nullptr)
	{
		delete[] MFT;
		MFT = nullptr;
	}
}

void NTFS::getDiskInformation()
{
	std::wcout << "Some information about this removable device " << _drive_name << std::endl;

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
	ReadSector(_drive_name, offsetMFT + 512, lastEntry);
	memcpy(MFT + 512, lastEntry, 512);
	delete lastEntry;
	readMFTEntry();
	delete[] MFT;

	int num_clusters = 0;
	 for(int i = 0; i < _list[0].data_runs.size(); i++) num_clusters += _list[0].data_runs[i].num_clusters;
	 offsetMFT += 1024; 
	 _root_reference = _list[0].parentReference; 
	return num_clusters;
}

void NTFS::readEntry(int offsetMFT) {
	int readed = 0;
	this->MFT = new BYTE[_MFT_entry_size];
	int i = 0;
	while(readed < offsetMFT) {
		int read = _MFT_entry_size - readed > 512 ? 512 : _MFT_entry_size - readed;
		BYTE* lastEntry = new BYTE[read];
		ReadSector(_drive_name, offsetMFT + readed, lastEntry);
		memcpy(MFT + readed, lastEntry, read);
		delete lastEntry;
		i++;
		readed += read;
	}
}

void NTFS::read() {
	system("cls");
	std::cout << "======================" << std::endl;
	std::cout << "Some information about files or folders" << std::endl;

	offsetMFT = _begin_cluster_of_MFT * _bytes_per_sector * _sectors_per_cluster;
	int total_sector = read$MFT() * _bytes_per_sector * _sectors_per_cluster;
	std::cout << total_sector << std::endl;
	while (total_sector > 0) {
		readEntry(offsetMFT);
		readMFTEntry();
		offsetMFT += _MFT_entry_size;
		total_sector -= _MFT_entry_size;
		if (MFT != nullptr)
		{
			delete[] MFT;
			MFT = nullptr; 
		}
	}
	printFileEntry();
	printDirectory(_root_reference);
	system("Pause"); 
}

void NTFS::readMFTEntry() {
	if (toString(MFT, 0, 4) != "FILE") return;
	if (getByteValues(MFT, 0x10, 2) == 0) return;
	FileEntry fileEntry;
	fileEntry.parentReference = _root_reference; 
	if (getByteValues(MFT, 0x16, 2) == 0x01) fileEntry.isDirectory = false;
	else if (getByteValues(MFT, 0x16, 2) == 0x03) fileEntry.isDirectory = true;
	else return;
	
	int startAtribute = getByteValues(MFT, 0x14, 2);
	fileEntry.fileReference = getByteValues(MFT, 0x2C, 4);

	while (getByteValues(MFT, startAtribute, 4) != 0xffffffff) 
	{
		
		int attributeSize = getByteValues(MFT, startAtribute + 4, 4);
		int startContent = getByteValues(MFT, startAtribute + 20, 2);
		if (getByteValues(MFT, startAtribute, 4) == 0x10) { //$STANDARD_INFORMATION
			fileEntry.flag = getByteValues(MFT, startAtribute + startContent + 32, 4);
		}
		else if (getByteValues(MFT, startAtribute, 4) == 0x30) { //$FILE_NAME
			fileEntry.parentReference = getByteValues(MFT, startAtribute + startContent, 4);
			int nameLength = getByteValues(MFT, startAtribute + startContent + 64, 1);
			int nameType = getByteValues(MFT, startAtribute + startContent + 65, 1);
			fileEntry.filename = toString(MFT, startAtribute + startContent + 66, nameLength * 2);
		}
		else if (getByteValues(MFT, startAtribute, 4) == 0x80) { //$DATA
			fileEntry.isResident = getByteValues(MFT, startAtribute + 8, 1) == 0;
			if (fileEntry.isResident) {
			
				int contentLength = getByteValues(MFT, startAtribute + 16, 4);
				int i = 0;
				while (contentLength > 0) {
					fileEntry.data.append(toString(MFT, startAtribute + startContent + i * 512, contentLength > 512 ? 512 : contentLength));
					contentLength -= contentLength > 512 ? 512 : contentLength;
					i++;
				}
			}
			else {
				int offsetDataRun = getByteValues(MFT, startAtribute + 32, 2);
				while (getByteValues(MFT, startAtribute + startContent + offsetDataRun, 2) != 0) {
					DataRun data;
					int byte_cluster = getByteValues(MFT, startAtribute + offsetDataRun, 1) / 16;
					int byte_size = getByteValues(MFT, startAtribute + offsetDataRun, 1) % 16;
					data.num_clusters = getByteValues(MFT, startAtribute + offsetDataRun + 1, byte_size);
					data.startCluster = getByteValues(MFT, startAtribute + offsetDataRun + 1 + byte_size, byte_cluster);
					fileEntry.data_runs.push_back(data);
					offsetDataRun += 1 + byte_size + byte_cluster;

				}
			}
		}

		startAtribute += attributeSize;
	}
	
	EntryMap.insert({ fileEntry.fileReference, _list.size() });
	_list.push_back(fileEntry);
	if (_dir_list.size() == 0 || searchDir(0, _dir_list.size() - 1, fileEntry.parentReference) == -1) {
		Directory dir;
		dir.reference = fileEntry.parentReference;
		addDir(dir);
	}
	int number = searchDir(0, _dir_list.size() - 1, fileEntry.parentReference);
	_dir_list[number].children.push_back(fileEntry.fileReference);
	if (fileEntry.isDirectory) {
		if (_dir_list.size() > 0) {
			if (searchDir(0, _dir_list.size() - 1, fileEntry.fileReference) == -1) {
				Directory dir;
				dir.reference = fileEntry.fileReference;
				addDir(dir);
				
			}
		}
	}
}

void NTFS::addDir(Directory dir) {
	for(int i = 0; i < _dir_list.size(); i++) {
		if(_dir_list[i].reference > dir.reference) {
			_dir_list.insert(_dir_list.begin()+i, dir);
			return;
		}
	}
	_dir_list.push_back(dir);
}


int NTFS::searchDir(int left, int right, int ref) 
{
	if (left > right) return -1;
	int mid = (left + right) / 2;
	if (_dir_list[mid].reference == ref) return mid;
	else if (_dir_list[mid].reference > ref) return searchDir(left, mid - 1, ref);
	else return searchDir(mid + 1, right, ref);
}

void NTFS::printNonResident(std::vector<DataRun>& data) 
{
	system("cls"); 
	for (int i = 0; i < data.size(); i++) {
		int sector_numnber = data[i].num_clusters * _sectors_per_cluster;
		int j = 0;
		while (sector_numnber > 0) {
			BYTE* sector = new BYTE[512];
			ReadSector(_drive_name, data[i].startCluster * _sectors_per_cluster * _bytes_per_sector + j * _bytes_per_sector, sector);
			std::cout << toString(sector, 0, 512) << std::endl;
			delete[] sector;
			j++;
			sector_numnber--;
		}
	}
}

void NTFS::printFileEntry() {
	for (int i = 0; i < _list.size(); i++) {
		// if(_list[i].flag == 2 || _list[i].flag == 6) continue;
		std::cout << "=====================" << std::endl;
		std::cout << "No: " << i << std::endl;
		std::cout << "Name: " << _list[i].filename << std::endl;
		std::cout << "Type: " << (_list[i].isDirectory ? "directory" : "file") << std::endl;
		std::cout << "Flag: " << _list[i].flag << std::endl;
		std::cout << "File Ref: " << _list[i].fileReference << std::endl;
		std::cout << "Parent: " << _list[i].parentReference << std::endl;
		std::cout << "Resident: " << _list[i].isResident << std::endl;
	}
}
void NTFS::printDirectory(int rootreference) {
	system("cls"); 
	int root = searchDir(0, _dir_list.size() - 1, rootreference);
	std::cout << "Root:" << root << std::endl; 
	std::wcout << "Choose the folder to view item in this drive:" << _drive_name << std::endl;
	std::cout << "Dont choose the folder name . or .." << std::endl;
	std::cout << "Choose the file TXT you want to dump! " << std::endl;

	int j = 0;
	std::map<int, int> ListMap; 
	for (int i = 0; i < _dir_list[root].children.size(); i++)
	{
		if (_list[EntryMap[_dir_list[root].children[i]]].flag == 2 || _list[EntryMap[_dir_list[root].children[i]]].flag == 6) continue;
		std::cout << "=====================" << std::endl;
		std::cout << "No:" << j + 1 << std::endl; 
		std::cout << "Ref: " << _dir_list[root].children[i] << std::endl;
		std::cout << "Name: " << _list[EntryMap[_dir_list[root].children[i]]].filename << std::endl;
		std::cout << "Type: " << (_list[EntryMap[_dir_list[root].children[i]]].isDirectory ? "directory" : "file") << std::endl;
		std::cout << "Flag: " << _list[EntryMap[_dir_list[root].children[i]]].flag << std::endl;
		std::cout << "File Ref: " << _list[EntryMap[_dir_list[root].children[i]]].fileReference << std::endl;
		std::cout << "Parent: " << _list[EntryMap[_dir_list[root].children[i]]].parentReference << std::endl;
		std::cout << "Resident: " << _list[EntryMap[_dir_list[root].children[i]]].isResident << std::endl;
		j++; 
		ListMap[j] = EntryMap[_dir_list[root].children[i]]; 

	}

	int choice = -1;

	std::cout << "Your choice:";
	std::cin >> choice;
	if (choice < 0 || choice > ListMap.rbegin()->first)
	{
		std::cout << "Something Wrong!!!" << std::endl;
		system("pause");
		printDirectory(rootreference);
	}
	else if (choice > 0 && choice <= ListMap.rbegin()->first)
	{
		int number = ListMap[choice];
		if (_list[number].isDirectory != true)
		{
			std::size_t dotPosition = _list[number].filename.find_last_of(".");
			std::string fileExtension = _list[number].filename.substr(dotPosition + 1, 3);
			if ((fileExtension == "txt" || fileExtension == "TXT") and _list[number].isResident == true)
			{
				system("cls"); 
				std::cout << _list[number].data << std::endl;
				system("Pause"); 
			}
			else if ((fileExtension == "txt" || fileExtension == "TXT") and _list[number].isResident == false)
			{
				printNonResident(_list[number].data_runs);
				system("Pause");
			}
			else
			{
				std::cout << "This file need a special software to read" << std::endl;
				system("Pause");
			}
		}
		else
		{
			printDirectory(_list[number].fileReference); 
		}
		printDirectory(rootreference);
	}
	else if (choice == 0) exit; 
}