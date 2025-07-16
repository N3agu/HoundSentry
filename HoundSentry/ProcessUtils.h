#pragma once
#include <string>
#include <windows.h>

bool IsProcessElevated();

std::wstring GetProcessNameByPid(DWORD processId);

std::string GetFileOperationName(UCHAR opcode);

std::string GetRegistryOperationName(UCHAR opcode);