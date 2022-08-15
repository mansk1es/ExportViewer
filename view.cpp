#include <iostream>
#include <Windows.h>

int main(int argc, char* argv[])
{
    
    HANDLE dllHandle = NULL;
    DWORD dllSize = NULL;

    HANDLE hFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE filemap = NULL;
    filemap = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
    PVOID addr = NULL;
    addr = MapViewOfFile(filemap, FILE_MAP_READ, NULL, NULL, NULL);
    
    PIMAGE_DOS_HEADER dllDOS = (PIMAGE_DOS_HEADER)addr;

    PIMAGE_NT_HEADERS dllNT = (PIMAGE_NT_HEADERS)((DWORD_PTR)addr + dllDOS->e_lfanew);

    DWORD exportDirectoryRVA = dllNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    PIMAGE_EXPORT_DIRECTORY exportDirAddr = (PIMAGE_EXPORT_DIRECTORY)((DWORD_PTR)addr + exportDirectoryRVA);
    
    PDWORD addrOfFunctions = (PDWORD)((DWORD_PTR)addr + exportDirAddr->AddressOfFunctions);
    PDWORD addrOfNames = (PDWORD)((DWORD_PTR)addr + exportDirAddr->AddressOfNames);
    PWORD addrOfOrds = (PWORD)((DWORD_PTR)addr + exportDirAddr->AddressOfNameOrdinals);

    DWORD_PTR functionAddressRVA = 0;
    PDWORD functionAddress = NULL;

    DWORD ordinals = NULL;


    DWORD lengthofArgwithoutDLL = strlen(argv[1]) - 4;

    for (DWORD i = 0; i < exportDirAddr->NumberOfFunctions; i++)
    {
        DWORD functionNameRVA = addrOfNames[i];
        DWORD_PTR functionNameVA = (DWORD_PTR)addr + functionNameRVA;
        char* functionName = (char*)functionNameVA;
        
        functionAddressRVA = addrOfFunctions[addrOfOrds[i]];
        functionAddress = (PDWORD)((DWORD_PTR)addr + functionAddressRVA);
        ordinals = addrOfOrds[i] + exportDirAddr->Base;
        printf("#pragma comment(linker,\"/export:%s=%.*s.%s,@%d\")\n", functionName, lengthofArgwithoutDLL, argv[1], functionName, ordinals);
    }
}
