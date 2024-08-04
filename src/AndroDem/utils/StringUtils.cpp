#include "StringUtils.h"
#if not _WIN32
#include <locale>
#include <codecvt>
#else
#include <windows.h>
#endif
bool replace(std::string& str, const char* from, const char* to) {
    size_t start_pos = 0;
    while (true) {
        start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, strlen(from), to);
        start_pos += strlen(from);
    }
    return true;
}
bool replacew(std::wstring& str, const wchar_t* from, const wchar_t* to) {
    size_t start_pos = 0;
    while (true) {
        start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, wcslen(from), to);
        start_pos += wcslen(from);
    }
    return true;
}
std::string ws2s(const std::wstring& wstr)
{
#if not _WIN32
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
    return converterX.to_bytes(wstr);
#else
    std::string res;

    int size = WideCharToMultiByte(CP_UTF8, NULL, wstr.data(), (int)wstr.size(), res.data(), NULL, NULL, NULL);

    res.resize(size);

    WideCharToMultiByte(CP_UTF8, NULL, wstr.data(), (int)wstr.size(), res.data(), size, NULL, NULL);
   
    return res;
#endif
}
std::wstring s2ws(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}