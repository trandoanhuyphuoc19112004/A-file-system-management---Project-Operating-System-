
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "Utils.h"

void printHexTable(BYTE sector[512]) 
{
    for (int i = 0; i < 512; i++) 
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sector[i]) << " ";
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }
}

int getByteValues(BYTE sector[512], int offset, int bytesize)
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

bool isLittleEndian() {
    uint32_t num = 1;
    return (*reinterpret_cast<uint8_t*>(&num) == 1);
}
