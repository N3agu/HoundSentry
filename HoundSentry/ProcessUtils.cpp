#include "ProcessUtils.h"
#include <TlHelp32.h>
#include <iostream>

bool IsProcessElevated() {
    HANDLE hToken = nullptr;
    TOKEN_ELEVATION elevation;
    DWORD dwSize = 0;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        // Failed to open process token
        return false;
    }

    bool isElevated = false;
    if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
        isElevated = elevation.TokenIsElevated != 0;
    }

    if (hToken) {
        CloseHandle(hToken);
    }

    return isElevated;
}

std::wstring GetProcessNameByPid(DWORD processId) {
    if (processId == 0) return L"System Idle Process";
    if (processId == 4) return L"System";

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return L"N/A";
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnap, &pe32)) {
        do {
            if (pe32.th32ProcessID == processId) {
                CloseHandle(hSnap);
                return std::wstring(pe32.szExeFile);
            }
        } while (Process32NextW(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return L"N/A";
}

std::string GetFileOperationName(UCHAR opcode) {
    switch (opcode) {
        case 10: return "FileRead";
        case 11: return "FileWrite";
        case 32: return "FileCreate";
        case 35: return "FileDelete";
        case 36: return "FileRename";
        case 37: return "FileQueryInformation";

        default: return "Other (" + std::to_string(opcode) + ")";
    }
}

std::string GetRegistryOperationName(UCHAR opcode) {
    switch (opcode) {
        case 10: return "RegCreateKey";
        case 11: return "RegOpenKey";
        case 12: return "RegDeleteKey";
        case 13: return "RegQueryKey";
        case 14: return "RegSetValue";
        case 15: return "RegDeleteValue";
        case 16: return "RegQueryValue";

        default: return "Other (" + std::to_string(opcode) + ")";
    }
}