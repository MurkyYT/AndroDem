#pragma once
#include <windows.h>
#include <string>
struct Config
{
	BOOL saveLogs;
	std::wstring currentDevice;
	BOOL runOnStartup;
};