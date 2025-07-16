#include <iostream>
#include <windows.h>

using std::cout;

#pragma comment(lib, "tdh.lib")

// https://learn.microsoft.com/en-us/windows/win32/etw/nt-kernel-logger-constants

// GUID for the Kernel File I/O Provider
static const GUID FileIoProviderGuid = { 0x90cbdc39, 0x4a3e, 0x421c, { 0x9a, 0x61, 0xb4, 0xad, 0xf3, 0x63, 0xfd, 0x24 } };
// GUID for the Kernel Registry Provider
static const GUID RegistryProviderGuid = { 0xae53722e, 0xc863, 0x11d2, { 0x86, 0x5a, 0x00, 0x60, 0x08, 0x75, 0x1d, 0x7b } };

int main() {
	cout << R"( _   _                       _ ____             _              
| | | | ___  _   _ _ __   __| / ___|  ___ _ __ | |_ _ __ _   _ 
| |_| |/ _ \| | | | '_ \ / _` \___ \ / _ \ '_ \| __| '__| | | |
|  _  | (_) | |_| | | | | (_| |___) |  __/ | | | |_| |  | |_| |
|_| |_|\___/ \__,_|_| |_|\__,_|____/ \___|_| |_|\__|_|   \__, |
                                                         |___/ 
)";
	cout << "\t   The Backbone of ChangeHound - by N3agu";
	return 0;
}