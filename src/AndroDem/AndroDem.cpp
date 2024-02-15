// AndroDem.cpp : Defines the entry point for the application.
//

#pragma region Includes
#include "AndroDem.h"
#pragma endregion
#pragma region Defines
#define ADB_SERVER_EXECUTE "CLASSPATH=/data/local/tmp/classes.dex app_process / com.murky.androdem.Server"
#define DISPLAY_MODE_OFF "0"
#define DISPLAY_MODE_ON "2"
#define MAX_LOADSTRING 100
#define WM_TRAY (WM_USER+1)
#define TRAY_ICON_ID 0xEB6E16
#define TIMER_POLL 1000// in ms
#pragma endregion
#pragma region Consts
CONST UINT WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
#pragma endregion
#pragma region Global Variables
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATA niData;
HICON hMainIcon, hNoWifiIcon, hLevel0Icon, hLevel1Icon, hLevel2Icon, hLevel3Icon, hLevel4Icon;
RegistrySettings settings(L"Murky", L"AndroDem");
Config config;
BOOL m_Connected = FALSE;
#pragma endregion

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LoadIcons();
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void ShowRightClickMenu(HWND hWnd);
void ConnectToDevice(std::wstring& device);
void DisconnectFromDevice();
DWORD WINAPI TimerProc();
void UpdateWifiStatus();
void ParseArgv(LPWSTR lpCmdLine);
void LoadConfig();
void SaveConfig();
void ConnectToLastDevice();
void SetAutoStartup(BOOL enabled);
BOOL FilesPresent();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	LoadConfig();
	if (!FilesPresent())
	{
		MessageBox(NULL, L"Not all files present in data folder, please redownload them.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	LOGI(L"[AndroDem.cpp] AndroDem v" VER " is starting up.");
	ADB::StartADB();
	LOGD(L"[AndroDem.cpp] Started ADB");
	ParseArgv(lpCmdLine);
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_ANDRODEM, szWindowClass, MAX_LOADSTRING);
	if (FindWindow(szWindowClass, szTitle) != NULL)
		return FALSE;
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ANDRODEM));
	MSG msg = {};

	ConnectToLastDevice();
	SetAutoStartup(config.runOnStartup);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
void ConnectToLastDevice()
{
	std::vector<std::wstring> devices = ADB::GetAllDevices();
	if (std::find(devices.begin(), devices.end(), config.currentDevice) != devices.end())
		ConnectToDevice(config.currentDevice);
	else
		config.currentDevice = L"";
}
void ParseArgv(LPWSTR lpCmdLine)
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	for (int i = 0; i < argc; i++)
	{
		if (lstrcmpi(argv[i],L"--verbose") == 0) {
			Logger::Level = VERBOSE;
			LOGI(L"Running in verbose mode");
		}
	}
}
BOOL FilesPresent()
{
	wchar_t* buf = GetCurrentDir();
	std::wstring adbExe = ADB::GetADBPath();
	std::wstring adbApiDLL = std::wstring(buf).append(L"\\data\\AdbWinApi.dll");
	std::wstring adbApiUSBDLL = std::wstring(buf).append(L"\\data\\AdbWinUsbApi.dll");
	std::wstring classesDex = std::wstring(buf).append(L"\\data\\classes.dex");

	return FileExists(ws2s(adbExe).c_str())
		&& FileExists(ws2s(adbApiDLL).c_str())
		&& FileExists(ws2s(adbApiUSBDLL).c_str())
		&& FileExists(ws2s(classesDex).c_str());
}
DWORD WINAPI TimerProc()
{
	while (TRUE)
	{
		if (!config.currentDevice.empty())
			UpdateWifiStatus();
		else
		{
			niData.hIcon = hMainIcon;
			wcscpy_s(niData.szTip, L"AndroDem");
			Shell_NotifyIcon(NIM_MODIFY, &niData);
		}
		Sleep(TIMER_POLL);
	}
	return 0;
}
void SaveConfig()
{
	settings.set("CurrentDevice", config.currentDevice.c_str());
	settings.set("RunOnStartup", config.runOnStartup);
	settings.set("SaveLogs", config.saveLogs);
}
void LoadConfig()
{
	config.currentDevice = settings.getWideString("CurrentDevice");
	settings.getBool("RunOnStartup", &config.runOnStartup);
	settings.getBool("SaveLogs", &config.saveLogs);
	Logger::Enabled = config.saveLogs;
}
void SetAutoStartup(BOOL enabled)
{
	DWORD val = 0;
	DWORD valSize = sizeof(DWORD);
	DWORD valType = REG_NONE;
	HKEY runHkey = NULL;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &runHkey) != ERROR_SUCCESS)
	{
		RegCloseKey(runHkey);
		return;
	}
	if (enabled) {
		wchar_t buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		RegSetValueEx(runHkey, L"AndroDem", 0, REG_SZ, reinterpret_cast<const BYTE*>(buffer), MAX_PATH);
	}
	else {
		RegDeleteValue(runHkey, L"AndroDem");
	}
	RegCloseKey(runHkey);
}
void UpdateWifiStatus()
{
	std::wstring wifiStatus = ADB::SendCommandToDeviceShell("dumpsys wifi | grep -E \"mWifiInfo |SignalLevel\"", config.currentDevice,TRUE);
	std::wstring stringSignalStrength;
	std::wstring deviceName = ADB::SendCommandToDeviceShell("settings get global device_name", config.currentDevice);
	std::wstring SSID;
	std::wstring linkSpeed;
	size_t lastSignalIndex = -1;
	int signalStrength = -1;
	//Couldn't get any wifi statistics about the device
	if (wifiStatus == L"FAIL" || wifiStatus.empty())
		goto NO_WIFI;
	if (deviceName == L"FAIL" || deviceName.empty())
		deviceName = ADB::SendCommandToDeviceShell("getprop ro.product.model", config.currentDevice);
	lastSignalIndex = wifiStatus.find(L"mLastSignalLevel ");
	if (lastSignalIndex == std::string::npos)
		goto NO_WIFI;
	stringSignalStrength = wifiStatus.substr(lastSignalIndex + 17, lastSignalIndex + 17 + 2);
	replacew(deviceName, L"\n", L"");
	replacew(deviceName, L"\r", L"");
	SSID = wifiStatus.substr(wifiStatus.find(L"SSID: ") + 6, wifiStatus.find(L",") - (wifiStatus.find(L"SSID: ") + 6));
	if (SSID.find('"') != std::string::npos)
		SSID = SSID.substr(1, SSID.size() - 2);
	linkSpeed = wifiStatus.substr(wifiStatus.find(L"speed: ") + 7, wifiStatus.find(L"s,") - (wifiStatus.find(L"speed: ") + 7) + 1);
	signalStrength = stringSignalStrength[0] - '0';
	// Device is probably not connected to a wifi network
	if (SSID == L"<unknown ssid>" || linkSpeed == L"-1Mbps")
		goto NO_WIFI;
	switch (signalStrength)
	{
	case 0:
		niData.hIcon = hLevel0Icon;
		break;
	case 1:
		niData.hIcon = hLevel1Icon;
		break;
	case 2:
		niData.hIcon = hLevel2Icon;
		break;
	case 3:
		niData.hIcon = hLevel3Icon;
		break;
	case 4:
		niData.hIcon = hLevel4Icon;
		break;
	default:
		niData.hIcon = hNoWifiIcon;
		break;
	}
	wcscpy_s(niData.szTip, std::wstring(deviceName).append(L": ").append(SSID).append(L" (").append(linkSpeed).append(L")").c_str());
	Shell_NotifyIcon(NIM_MODIFY, &niData);
	return;
NO_WIFI:
	niData.hIcon = hNoWifiIcon;
	wcscpy_s(niData.szTip, L"No wifi found");
	Shell_NotifyIcon(NIM_MODIFY, &niData);
	return;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ANDRODEM));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ANDRODEM);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ANDRODEM));

	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	return TRUE;
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
	{
		ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

		LoadIcons();

		niData.cbSize = sizeof(NOTIFYICONDATA);
		niData.uID = TRAY_ICON_ID;

		niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

		// Load the icon to be displayed in the notification tray area.
		niData.hIcon = hMainIcon;

		niData.hWnd = hWnd;
		//niData.dwInfoFlags = NIIF_INFO;
		niData.uTimeout = 10000;
		niData.uVersion = NOTIFYICON_VERSION_4;
		//wcsncpy_s(niData.szInfoTitle, 64, L"Title for balloon", 64);
		//wcsncpy_s(niData.szInfo, 256, L"Body for balloon", 256);
		niData.uCallbackMessage = WM_TRAY;
		wcscpy_s(niData.szTip, L"AndroDem");

		Shell_NotifyIcon(NIM_ADD, &niData);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimerProc, NULL, 0, NULL);
		break;
	}
	case WM_TRAY:
	{
		// This is a message that originated with the
		// Notification Tray Icon. The lParam tells use exactly which event
		// it is.
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		{
			ShowRightClickMenu(hWnd);
			break;
		}
		}
		break;
	}
	default:
		if (uMsg == WM_TASKBARCREATED)
			Shell_NotifyIcon(NIM_ADD, &niData);
		break;
	case WM_QUERYENDSESSION:
		ShutdownBlockReasonCreate(hWnd, L"Cleaning device...");
		DisconnectFromDevice();
		SaveConfig();
		ShutdownBlockReasonDestroy(hWnd);
		return TRUE;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &niData);
		DisconnectFromDevice();
		SaveConfig();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void LoadIcons()
{
	hMainIcon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_ANDRODEM),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hNoWifiIcon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_NOWIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hLevel0Icon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_LEVEL0WIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hLevel1Icon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_LEVEL1WIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hLevel2Icon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_LEVEL2WIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hLevel3Icon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_LEVEL3WIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
	hLevel4Icon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_LEVEL4WIFI),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADTRANSPARENT);
}
void ShowRightClickMenu(HWND hWnd)
{
	SetForegroundWindow(hWnd);
	// Get current mouse position.
	POINT curPoint;
	GetCursorPos(&curPoint);

	HMENU hMenu = CreatePopupMenu();
	HMENU devicesMenu = CreatePopupMenu();
	HMENU deviceMenu = CreatePopupMenu();
	AppendMenu(deviceMenu, MF_STRING, IDM_ENABLEDSPL, L"Enable Display");
	AppendMenu(deviceMenu, MF_STRING, IDM_DISABLEDSPL, L"Disable Display");
	AppendMenu(deviceMenu, MF_STRING, IDM_RESTARTWIFI, L"Restart WIFI");
	AppendMenu(deviceMenu, MF_STRING, IDM_OPENSHELL, L"Open Shell");
	AppendMenu(hMenu, MFS_DISABLED | MF_STRING | MF_DISABLED, NULL, L"AndroDem - " VER);
	AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hMenu, MF_STRING | (config.runOnStartup ? MF_CHECKED : NULL), IDM_STARTWITHWINDOWS, L"Start With Windows");
	AppendMenu(hMenu, MF_STRING | (config.saveLogs ? MF_CHECKED : NULL), IDM_SAVELOGS, L"Save Logs");
	AppendMenu(hMenu, MF_STRING | MF_POPUP | (config.currentDevice.empty() ? MFS_GRAYED | MF_DISABLED : NULL), (UINT_PTR)deviceMenu, L"Device Options");
	AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)devicesMenu, L"Select Device");
	AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"E&xit");
	std::vector<std::wstring> devices = ADB::GetAllDevices();
	for (size_t i = 0; i < devices.size(); i++)
		AppendMenu(devicesMenu, MF_STRING | (config.currentDevice == devices[i] ? MF_CHECKED : NULL), (i + 1), devices[i].c_str());

	UINT_PTR clicked = TrackPopupMenu(
		hMenu,
		TPM_RETURNCMD | TPM_NONOTIFY,
		curPoint.x,
		curPoint.y,
		0,
		hWnd,
		NULL
	);

	if (clicked) {
		std::wstring result;
		switch (clicked)
		{
		case IDM_STARTWITHWINDOWS:
			config.runOnStartup = !config.runOnStartup;
			SetAutoStartup(config.runOnStartup);
			break;
		case IDM_SAVELOGS:
			config.saveLogs = !config.saveLogs;
			Logger::Enabled = config.saveLogs;
			break;
		case IDM_ENABLEDSPL:
			result = ADB::SendCommandToDeviceShell(ADB_SERVER_EXECUTE " display " DISPLAY_MODE_ON, config.currentDevice, TRUE);
			LOGD(result.c_str());
			break;
		case IDM_DISABLEDSPL:
			result = ADB::SendCommandToDeviceShell(ADB_SERVER_EXECUTE " display " DISPLAY_MODE_OFF, config.currentDevice, TRUE);
			LOGD(result.c_str());
			break;
		case IDM_RESTARTWIFI:
			result = ADB::SendCommandToDeviceShell(ADB_SERVER_EXECUTE " restart-wifi", config.currentDevice, TRUE);
			LOGD(result.c_str());
			break;
		case IDM_OPENSHELL:
			ADB::OpenDeviceShell(config.currentDevice);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			if (clicked - 1 < devices.size())
				ConnectToDevice(devices[clicked-1]);
			break;
		}
	}
}
void ConnectToDevice(std::wstring& device)
{
	DisconnectFromDevice();
	if (device == config.currentDevice && m_Connected) {
		config.currentDevice = L"";
		return;
	}
	// Windows on auto startup default dir is c:\system32\windows
	wchar_t* buf = GetCurrentDir();
	std::wstring pushLocal = std::wstring(L"push \"").append(buf).append(L"\\data\\classes.dex\" \"data/local/tmp\"");
	config.currentDevice = device;
	if(ADB::SendCommandToDevice(pushLocal.c_str(), config.currentDevice).find(L"adb: error: failed to copy") != std::string::npos)
	{
		//Work around by sending to sdcard then moving using shell to local tmp
		std::wstring pushSDCard = std::wstring(L"push \"").append(buf).append(L"\\data\\classes.dex\" \"sdcard/\"");
		ADB::SendCommandToDevice(pushSDCard.c_str(), config.currentDevice);
		std::wstring result = ADB::SendCommandToDeviceShell("mv \"sdcard/classes.dex\" \"data/local/tmp\"", config.currentDevice);
		if (result == L"FAIL" || !result.empty())
		{
			config.currentDevice = L"";
			MessageBox(NULL, L"Couldn't push ther server to the device, make sure it is connected and ADB debugging is enabled", L"Error", MB_OK | MB_ICONERROR);
			return;
		}
	}
	ADB::SendCommandToDeviceShell("svc power stayon true", config.currentDevice, TRUE);
	std::wstring result = ADB::SendCommandToDeviceShell(ADB_SERVER_EXECUTE " display " DISPLAY_MODE_OFF, config.currentDevice, TRUE);
	LOGD(result.c_str());
	LOGI(std::wstring(L"[AndroDem.cpp] Successfully connected to device: '").append(device).append(L"'").c_str());
	m_Connected = TRUE;
}
void DisconnectFromDevice()
{
	if (!config.currentDevice.empty() && m_Connected)
	{
		std::wstring result = ADB::SendCommandToDeviceShell(ADB_SERVER_EXECUTE " cleanup", config.currentDevice, TRUE);
		LOGD(result.c_str());
	}
	else if (config.currentDevice.empty())
		m_Connected = FALSE;
}
