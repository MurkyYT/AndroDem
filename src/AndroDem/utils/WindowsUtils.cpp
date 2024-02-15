#include "WindowsUtils.h"
wchar_t* GetCurrentDir()
{
	wchar_t* buf = NULL;
	UINT32 buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		free(buf);
	}
	return buf;
}
BOOL FileExists(const char* name)
{
	struct stat   buffer;
	return (stat(name, &buffer) == 0);
}