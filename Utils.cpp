
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "Utils.h"

void printHexTable(const BYTE *sector) 
{
  
    for (int i = 0; i < 512; i++) 
    {
        printf("%02X ", sector[i]);
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }

 
}

BYTE* clone_sector(const BYTE sector[512])
{
    BYTE* clone = new BYTE[512];
    memcpy(clone, sector, 512); 
    return clone; 
}

int getByteValues(BYTE *sector, int offset, int bytesize)
{
    int value = 0; 
    memcpy(&value, sector + offset, bytesize);
    return value; 
}

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
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
        printf("Success!\n");
    }
}


