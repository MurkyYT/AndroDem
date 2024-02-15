#include "Logger.h"
#ifndef NDEBUG
LogLevel Logger::Level = DEBUG;
#else
LogLevel Logger::Level = INFO;
#endif
BOOL Logger::Enabled = FALSE;
BOOL Logger::WriteDebug(const wchar_t* text)
{
	if (Level == INFO)
		return FALSE;
	else if(Level == VERBOSE)
		WriteInfo(text);
	else
	{
		std::wstring result = std::wstring(text);
		if (result[result.size() - 1] != '\n')
			result.append(L"\n");
		OutputDebugStringW(result.c_str());
	}
	return FALSE;
}
void Logger::GetTime(wchar_t* buffer,size_t size)
{
	time_t rawtime;
	struct tm timeinfo;
	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	wcsftime(buffer, size, L"[%d.%m.%Y %H:%M:%S]", &timeinfo);
}
BOOL Logger::WriteInfo(const wchar_t* text)
{
	wchar_t timeBuffer[24];
	GetTime(timeBuffer, 24);

	std::wstring result = std::wstring(timeBuffer).append(L" ").append(text);
	if (result[result.size()-1] != '\n')
		result.append(L"\n");
	if(Level != DEBUG && Logger::Enabled)
		return Write(result.c_str());
	return WriteDebug(text);
}
BOOL Logger::Write(const wchar_t* text)
{
	FILE* logFile = NULL;
	std::wstring wPath = GetCurrentDir();
	std::string path = ws2s(wPath).append("\\log.txt");
	if (!FileExists(path.c_str()))
		fopen_s(&logFile, path.c_str(), "w+ ,ccs=UTF-16LE");
	else
		fopen_s(&logFile, path.c_str(), "a+ ,ccs=UTF-16LE");
	if (logFile == NULL)
		return -1;
	int success = fputws(text,logFile);
	fclose(logFile);
	return success == 0;
}