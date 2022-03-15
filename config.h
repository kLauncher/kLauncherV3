#pragma once
/*
	debug) localhost + http
	release) server + https
*/

#define WIN32_LEAN_AND_MEAN // or _WINSOCKAPI_
#include <Windows.h>
#include <string>

namespace Config {
	void Config(HINSTANCE hInstance, bool checkMutex = true);
	void Free();

	/// <summary>
	/// 현재 실행되고있는 파일경로
	/// </summary>
	extern std::string exePath;

	/// <summary>
	/// 현재 실행되고있는 파일폴더
	/// </summary>
	extern std::string realCurrentDirectory;

	/// <summary>
	/// 운영체제가 64비트
	/// </summary>
	extern bool os64bit;

	/// <summary>
	/// 현재 관리자권한으로 프로그램이 실행중
	/// </summary>
	extern bool isElevation;


	extern HINSTANCE _hInstance;
	extern HICON hLauncherIcon;
	extern HICON hSuccessIcon;
	extern HICON hErrorIcon;
	extern HICON hWarnIcon;
	extern HANDLE hMutex;
}
