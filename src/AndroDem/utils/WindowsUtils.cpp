#include "WindowsUtils.h"
std::wstring GetCurrentDir()
{
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}
BOOL FileExists(const char* name)
{
	return _access_s(name, 0) != ENOENT;
}
BOOL FileExists(const wchar_t* name)
{
	return _waccess_s(name, 0) != ENOENT;
}