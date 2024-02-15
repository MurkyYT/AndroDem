#pragma once
#include <Windows.h>
#include <sys/stat.h>
#include <string>
const wchar_t* GetCurrentDir();
BOOL FileExists(const char* name);