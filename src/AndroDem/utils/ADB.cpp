#include <WS2tcpip.h>
#include <winsock.h>
#include "ADB.h"
#pragma comment(lib, "Ws2_32.lib")
std::wstring ADB::m_adbPath;
BOOL ADB::m_WSAInitialized = FALSE;
std::vector<std::wstring> ADB::GetAllDevices()
{
	std::vector<std::wstring> result = std::vector<std::wstring>();
	std::wstring devices = SendCommandToNativeADB("host:devices");
	while(TRUE)
	{
		size_t tabIndex = devices.find_first_of(L"\t");
		if (tabIndex == std::string::npos)
			break;
		std::wstring devName = devices.substr(0, tabIndex);
		size_t lineEndIndex = devices.find_first_of(L"\n");
		std::wstring deviceStatus = devices.substr(tabIndex+1, 6);
		if(deviceStatus == L"device")
			result.push_back(devName);
		devices = devices.substr(lineEndIndex+1);
	}
	return result;
}
BOOL ADB::IsDeviceLandscaped(std::wstring& serialNumber)
{
	std::wstring result = SendCommandToDeviceShell("dumpsys input | grep \"      SurfaceOrientation: \"",serialNumber);
	return result[result.size() - 2] == '0' || result[result.size() - 2] == '3';
}
void ADB::RebootDevice(std::wstring& serialNumber)
{
	SendCommandToDevice(L"reboot", serialNumber);
}
void ADB::RestartADB()
{
	StopADB();
	StartADB();
}
void ADB::StartADB()
{
	SendCommandToADB(L"start-server");
}
void ADB::StopADB()
{
	SendCommandToADB(L"kill-server");
}
std::wstring ADB::SendCommandToDeviceShell(LPCSTR command, std::wstring& serialNumber, BOOL waitUntillComplete)
{
	char szSize[4];
	int nSize = 0;
	std::wstring szTmp = L"";
	std::string transport = std::string("host:transport:").append(ws2s(serialNumber));
	std::string commandChar = std::string("shell:").append(command);
	SOCKET adb_soc = ConnectSocket(waitUntillComplete);
	if (!adb_soc)
		goto EXIT;
	AdbWrite(transport.c_str(), adb_soc);
	if (!AdbRead(szSize, 4, adb_soc) || memcmp(szSize, "OKAY", 4))
		goto FAIL;
	LOGD(std::wstring(L"[ADB.cpp] Succesfully targeted device: '").append(serialNumber).append(L"'").c_str());
	AdbWrite(commandChar.c_str(), adb_soc);
	if (!AdbRead(szSize, 4, adb_soc) || memcmp(szSize, "OKAY", 4))
		goto FAIL;
	char c;
	while (AdbRead(&c, 1, adb_soc))
		szTmp += c;
	closesocket(adb_soc);
	LOGD(std::wstring(L"[ADB.cpp] Succesfully sent '").append(commandChar.begin(), commandChar.end()).append(L"'").c_str());
	return szTmp;
FAIL:
	return std::wstring(L"FAIL");
EXIT:
	return std::wstring();
}
std::wstring ADB::SendCommandToNativeADB(LPCSTR command,BOOL waitUntillComplete)
{
	std::string commandStr = command;
	char szSize[4];
	int nSize = 0;
	std::wstring szTmp = L"";
	SOCKET adb_soc = ConnectSocket(waitUntillComplete);
	if (!adb_soc)
		goto EXIT;
	AdbWrite(command,adb_soc);
	if (!AdbRead(szSize, 4, adb_soc) || memcmp(szSize, "OKAY", 4))
		goto FAIL;
	if (!AdbRead(szSize, 4, adb_soc))
		goto EXIT;
	nSize = (HexDigit(szSize[0]) << 12) + (HexDigit(szSize[1]) << 8) + (HexDigit(szSize[2]) << 4) + (HexDigit(szSize[3]));
	char c;
	while (nSize-- && AdbRead(&c, 1, adb_soc))
		szTmp += c;
	closesocket(adb_soc);
	LOGD(std::wstring(L"[ADB.cpp] Succesfully sent '").append(commandStr.begin(), commandStr.end()).append(L"'").c_str());
	return szTmp;
EXIT:
	return std::wstring();
FAIL:
	return std::wstring(L"FAIL");
}
SOCKET ADB::ConnectSocket(BOOL waitUntillComplete)
{
	if (!m_WSAInitialized)
	{
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result == 0)
			m_WSAInitialized = TRUE;
		else
			return NULL;
	}
	struct sockaddr_in address = { 0 };
	address.sin_family = AF_INET;
	address.sin_port = htons((unsigned short)5037);
	inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr);
	int iResult;

	SOCKET adb_soc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!waitUntillComplete) {
		DWORD timeout = TIMEOUT;
		setsockopt(adb_soc, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	}
	if (adb_soc == NULL || adb_soc == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		return NULL;
	}
	iResult = connect(adb_soc, (struct sockaddr*)&address, sizeof(address));
	if (iResult < 0)
	{
		SendCommandToADB(L"start-server");
		for (size_t i = 0; i < 5; i++)
		{
			iResult = connect(adb_soc, (struct sockaddr*)&address, sizeof(address));
			if (iResult < 0)
				SendCommandToADB(L"start-server");
		}
	}
	return adb_soc;
}
BOOL ADB::AdbWrite(const char* cmd, SOCKET socket)
{
	size_t size = strlen(cmd);
	char* szSize = (char*)malloc(5 + size);
	if (szSize) 
	{
		sprintf_s(szSize,5, "%04x", (UINT)size);
		strcat_s(szSize,5+ size, cmd);
		return (send(socket, szSize, (int)(size + 4), 0) == size);
	}
	return FALSE;
}
BOOL ADB::AdbRead(char* buf, int size, SOCKET socket)
{
	int sz = recv(socket, buf, size, 0);
	return (sz == size);
}
void ADB::OpenDeviceShell(std::wstring& serialNumber)
{
	std::wstring command = std::wstring(L" -s ").append(serialNumber).append(L" shell").c_str();
	wchar_t* commandLine = new wchar_t[command.size() + 1];
	wcscpy_s(commandLine, command.size() + 1, command.c_str());
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOW siStartInfo;
	BOOL bSuccess = FALSE;

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
	siStartInfo.cb = sizeof(STARTUPINFOW);
	bSuccess = CreateProcessW(ADB::GetADBPath().c_str(),
		commandLine,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		NULL,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 
	if (!bSuccess)
		return;
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
}
std::wstring ADB::SendCommandToDevice(LPCWSTR command, std::wstring& serialNumber)
{
	std::wstring result = std::wstring(L"-s ").
		append(serialNumber.c_str()).
		append(L" ").
		append(command);
		return SendCommandToADB(result.c_str());
}
std::wstring ADB::SendCommandToADB(LPCWSTR command)
{
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr;
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return std::wstring();
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return std::wstring();
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOW siStartInfo;
	BOOL bSuccess = FALSE;

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
	siStartInfo.cb = sizeof(STARTUPINFOW);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	std::wstring res = std::wstring(L" ").append(command);
	size_t size = res.size();
	wchar_t* commandLine = new wchar_t[size + 1];
	wcscpy_s(commandLine, size + 1, res.c_str());
	bSuccess = CreateProcessW(ADB::GetADBPath().c_str(),
		commandLine,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 
	if (!bSuccess)
		return std::wstring();

	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	DWORD dwRead;
	CHAR chBuf[BUFSIZE];
	std::string resultBuf;
	DWORD bytesAvail = 0;
	if (g_hChildStd_OUT_Wr != INVALID_HANDLE_VALUE && g_hChildStd_OUT_Wr)
		CloseHandle(g_hChildStd_OUT_Wr);
	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;
		else resultBuf.append(chBuf, dwRead);
	}
	if (g_hChildStd_OUT_Rd != INVALID_HANDLE_VALUE && g_hChildStd_OUT_Rd)
		CloseHandle(g_hChildStd_OUT_Rd);
	LOGD(std::wstring(L"[ADB.cpp] Succesfully sent '").append(command).append(L"'").c_str());
	return std::wstring(resultBuf.begin(),resultBuf.end());
}
std::wstring ADB::GetADBPath()
{
	if (!ADB::m_adbPath.empty())
		return ADB::m_adbPath;

	const wchar_t* buffer = GetCurrentDir();

	ADB::m_adbPath = std::wstring(buffer).append(L"\\data\\adb.exe");
	return ADB::m_adbPath;
}