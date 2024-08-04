#include "RegistrySettings.h"

RegistrySettings::~RegistrySettings()
{
	if (m_softwareKey != INVALID_HANDLE_VALUE)
		RegCloseKey(m_softwareKey);
	if (m_settingsKey != INVALID_HANDLE_VALUE)
		RegCloseKey(m_settingsKey);
}
RegistrySettings::RegistrySettings(const std::wstring& company, const std::wstring& product) : m_company(company), m_product(product)
{
	LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE",0,KEY_ALL_ACCESS,&m_softwareKey);
	if (res != ERROR_SUCCESS)
		throw "Error";
	res = RegCreateKeyW(m_softwareKey, m_company.append(L"\\").append(m_product).c_str(), &m_settingsKey);
	if (res != ERROR_SUCCESS)
		throw "Error";
}
std::wstring RegistrySettings::getWideString(const std::string& name)
{
	wchar_t* buf;
	getString(name, (LPCWSTR*)&buf);
	std::wstring res = std::wstring(buf);
	delete[] buf;
	return res;
}
BOOL RegistrySettings::getString(const std::string& name,LPCWSTR* buf)
{
	*buf = new wchar_t[0];
	std::wstring wValue = s2ws(name);
	DWORD dwBufSize = 0;
	DWORD valueType = 0;
	LONG lRetVal = RegQueryValueExW(m_settingsKey,
		wValue.c_str(), NULL, &valueType, NULL, &dwBufSize);
	if (ERROR_SUCCESS != lRetVal) {
		return FALSE;
	}
	if (dwBufSize > 0 && valueType == REG_SZ)
	{
		*buf = new wchar_t[dwBufSize]/*(wchar_t*)malloc(dwBufSize)*/;
		lRetVal = RegQueryValueExW(m_settingsKey, wValue.c_str(), NULL, NULL,
			(BYTE*)(*buf), &dwBufSize);
		return TRUE;
	}
	return FALSE;
}
BOOL RegistrySettings::getBool(const std::string& name, BOOL* buf)
{
	*buf = NULL;
	std::wstring wValue = s2ws(name);
	DWORD dwBufSize = 0;
	DWORD valueType = 0;
	LONG lRetVal = RegQueryValueExW(m_settingsKey,
		wValue.c_str(), NULL, &valueType, NULL, &dwBufSize);
	if (ERROR_SUCCESS != lRetVal) {

		return FALSE;
	}
	if (dwBufSize > 0 && valueType == REG_BINARY)
	{
		BYTE* pBuffer = new BYTE[dwBufSize];
		lRetVal = RegQueryValueExW(m_settingsKey, wValue.c_str(), NULL, NULL,
			(LPBYTE)pBuffer, &dwBufSize);
		if (*pBuffer == 0)
			*buf = FALSE;
		else
			*buf = TRUE;
		delete[] pBuffer;
		return TRUE;
	}
	return FALSE;;
}
BOOL RegistrySettings::getDWORD(const std::string& name, DWORD* buf)
{
	*buf = NULL;
	std::wstring wValue = s2ws(name);
	DWORD dwBufSize = 0;
	DWORD valueType = 0;
	LONG lRetVal = RegQueryValueExW(m_settingsKey,
		wValue.c_str(), NULL, &valueType, NULL, &dwBufSize);
	if (ERROR_SUCCESS != lRetVal) {

		return FALSE;
	}
	if (dwBufSize > 0 && valueType == REG_DWORD)
	{
		lRetVal = RegQueryValueExW(m_settingsKey, wValue.c_str(), NULL, NULL,
			(LPBYTE)buf, &dwBufSize);
		return TRUE;
	}
	return FALSE;;
}
void RegistrySettings::set(const std::string& name, LPCWSTR value)
{
	LSTATUS stat;
	if(m_settingsKey)
		stat = RegSetValueExW(m_settingsKey, s2ws(name).c_str(), 0,REG_SZ, reinterpret_cast<const BYTE*>(value), (DWORD)(wcslen(value) * sizeof(wchar_t)));
}
void RegistrySettings::set(const std::string& name, BOOL value)
{
	LSTATUS stat;
	if (m_settingsKey)
		stat = RegSetValueExW(m_settingsKey, s2ws(name).c_str(), 0, REG_BINARY, (BYTE*)&value, 1);
}
void RegistrySettings::set(const std::string& name, DWORD value)
{
	LSTATUS stat;
	if (m_settingsKey)
		stat = RegSetValueExW(m_settingsKey, s2ws(name).c_str(), 0, REG_BINARY, (BYTE*)&value, sizeof(DWORD));
}