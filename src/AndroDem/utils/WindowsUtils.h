#pragma once
#include <Windows.h>
#include <string>
#include <io.h>
std::wstring GetCurrentDir();
BOOL FileExists(const char* name);
BOOL FileExists(const wchar_t* name);

static inline bool IsDarkMode()
{
	DWORD vType;
	DWORD vLen = 0;
	DWORD val = 0;
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	if ((RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, &vType, NULL, &vLen) == ERROR_SUCCESS) && (vType == REG_DWORD)) {
		RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, &vType, (LPBYTE)&val, &vLen);
		RegCloseKey(hKey);
		return val == 0;
	}
	return FALSE;
};