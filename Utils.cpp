
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "Utils.h"

void printHexTable(const BYTE *sector, int size) 
{
  
    for (int i = 0; i < size; i++) 
    {
        printf("%02X ", sector[i]);
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }

 
}


int getByteValues(BYTE *sector, int offset, int bytesize)
{
    int value = 0; 
    memcpy(&value, sector + offset, bytesize);
    return value; 
}


// This function read only 512 byte 
int ReadSector(LPCWSTR  drive, int readPoint, BYTE *&sector)
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFileW(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
       // printf("Success!\n");
    }
    return 0;
}


std::string toString(BYTE* data, int offset, int number)
{
    char* tmp = new char[number + 1];
    memcpy(tmp, data + offset, number);
    std :: string s = "";
    for (int i = 0; i < number; i++)
        if (tmp[i] != 0x00 && tmp[i] != 0xFF)
            s += tmp[i];
    delete[] tmp; 
    return s;
}