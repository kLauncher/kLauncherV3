#include "pch.h"
#include "resource.h"
#include "exception.h"
#include "common.h"
#include "exception.h"
#include "string.h"

namespace Config {
	void Config(HINSTANCE hInstance, bool checkMutex);
	void Free();

	std::string exePath;
	std::string realCurrentDirectory;
	bool os64bit;
	bool isElevation;

	HINSTANCE _hInstance = nullptr;
	HICON hLauncherIcon = nullptr;
	HICON hSuccessIcon = nullptr;
	HICON hErrorIcon = nullptr;
	HICON hWarnIcon = nullptr;

	HANDLE hMutex = nullptr;
}


void Config::Config(HINSTANCE hInstance, bool checkMutex) {

	_hInstance = hInstance;

	// LoadIconMetric 
	hLauncherIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	hSuccessIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY_SUCCESS));
	hErrorIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY_ERROR));
	hWarnIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY_WARN));

	// OS 확인
	SYSTEM_INFO systemInfo = { 0, };
	GetNativeSystemInfo(&systemInfo);
	os64bit = systemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL;

	isElevation = IsElevated();

	// 파일검사
	// load.dll kDetector.k Battle.net.dll
	exePath = std::string(260, 0);
	DWORD dwLen = GetModuleFileName(hInstance, exePath.data(), exePath.size());
	exePath = exePath.substr(0, dwLen);
	realCurrentDirectory = exePath.substr(0, exePath.find_last_of("\\"));

	if (checkMutex) {
		// Mutex 중복실행검사
		hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, CLASSNAME);
		if (hMutex) {
			CloseHandle(hMutex);
			hMutex = nullptr;
			throw MutexException();
		}
		else {
			hMutex = CreateMutex(nullptr, true, CLASSNAME);
		}
	}
}

void Config::Free() {
	if (hMutex) CloseHandle(hMutex);
	if (hLauncherIcon) DestroyIcon(hLauncherIcon);
	if (hSuccessIcon) DestroyIcon(hSuccessIcon);
	if (hErrorIcon) DestroyIcon(hErrorIcon);
	if (hWarnIcon) DestroyIcon(hWarnIcon);
}