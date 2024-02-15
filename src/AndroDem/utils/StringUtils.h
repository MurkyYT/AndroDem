#pragma once
#include <string>
#include <locale>
#include <codecvt>

bool replace(std::string& str, const char* from, const char* to);
bool replacew(std::wstring& str, const wchar_t* from, const wchar_t* to);
std::string ws2s(const std::wstring& wstr);
std::wstring s2ws(const std::string& str);