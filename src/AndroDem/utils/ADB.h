#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "StringUtils.h"
#include "Logger.h"
#define HexDigit(c) (((c>='0') && (c<='9')) ? c-'0' : ((c>='a') && (c<='f')) ? c-'a'+10 :((c>='A') && (c<='F')) ? c-'A'+10 : 0)
#define TIMEOUT 400
#define BUFSIZE 4096 
class ADB {
public:
	static std::wstring GetADBPath();
	static std::vector<std::wstring> GetAllDevices();
	static void RestartADB();
	static void StartADB();
	static void StopADB();
	static BOOL IsDeviceLandscaped(std::wstring& serialNumber);
	static std::wstring SendCommandToDeviceShell(LPCSTR command, std::wstring& serialNumber,BOOL waitUntillComplete = FALSE);
	static std::wstring SendCommandToNativeADB(LPCSTR command,BOOL waitUntillComplete = FALSE);
	static void OpenDeviceShell(std::wstring& serialNumber);
	static std::wstring SendCommandToDevice(LPCWSTR command, std::wstring& serialNumber);
	static std::wstring SendCommandToADB(LPCWSTR command);
	static void RebootDevice(std::wstring& serialNumber);
private:
	static std::wstring m_adbPath;
	static BOOL AdbWrite(const char* cmd,SOCKET socket);
	static BOOL AdbRead(char* buf, int size, SOCKET socket);
	static SOCKET ConnectSocket(BOOL waitUntillComplete);
	static BOOL m_WSAInitialized;
};