#pragma once
#include <Windows.h>
#include <sys/stat.h>
wchar_t* GetCurrentDir();
BOOL FileExists(const char* name);