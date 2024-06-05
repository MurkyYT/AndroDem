#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include <time.h>
#include "WindowsUtils.h"
#include "StringUtils.h"

#define LOGD(text) if(Logger::Level == DEBUG || Logger::Level == VERBOSE) Logger::WriteDebug(text)
#define LOGI(text) Logger::WriteInfo(text)

enum LogLevel
{
	INFO,
	DEBUG,
	VERBOSE
};
class Logger
{
public:
	static LogLevel Level;
	static BOOL Enabled;
	static BOOL Write(const wchar_t* text);
	static BOOL WriteInfo(const wchar_t* text);
	static BOOL WriteDebug(const wchar_t* text);
	static BOOL WriteInfo(const std::wstring& text) { return WriteInfo(text.c_str()); };
	static BOOL WriteDebug(const std::wstring& text) { return WriteDebug(text.c_str()); };
	static BOOL Write(const std::wstring& text) { return Write(text.c_str()); };
private:
	static void GetTime(wchar_t* buffer, size_t size);
	
};