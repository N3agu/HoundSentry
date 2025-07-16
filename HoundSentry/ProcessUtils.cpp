#include <windows.h>

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