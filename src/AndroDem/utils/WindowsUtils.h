#pragma once
#include <Windows.h>
#include <string>
#include <io.h>
std::wstring GetCurrentDir();
BOOL FileExists(const char* name);
BOOL FileExists(const wchar_t* name);