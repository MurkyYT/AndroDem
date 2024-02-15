#pragma once
#include "StringUtils.h"
#include <Windows.h>
#include <string>
#include <typeinfo>
class RegistrySettings
{
public:
	RegistrySettings(const std::wstring& company, const std::wstring& product);
	~RegistrySettings();
	void set(const std::string& name, LPCWSTR value);
	void set(const std::string& name, BOOL value);
	void set(const std::string& name, DWORD value);
	std::wstring getWideString(const std::string& name);
	BOOL getString(const std::string& name,LPCWSTR* buf);
	BOOL getBool(const std::string& name,BOOL* buf);
	BOOL getDWORD(const std::string& name,DWORD* buf);
private:
	HKEY m_softwareKey;
	HKEY m_settingsKey;
	std::wstring m_company;
	std::wstring m_product;
};