#pragma once
#include <Windows.h>
#include <sys/stat.h>
#include <string>
std::wstring GetCurrentDir();
BOOL FileExists(const char* name);