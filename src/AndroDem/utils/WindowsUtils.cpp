#include "WindowsUtils.h"
const wchar_t* GetCurrentDir()
{
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos).c_str();
}
BOOL FileExists(const char* name)
{
	struct stat   buffer;
	return (stat(name, &buffer) == 0);
}