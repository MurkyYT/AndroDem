#include "StringUtils.h"
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
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
std::wstring s2ws(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}