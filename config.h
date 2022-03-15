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
	/// ���� ����ǰ��ִ� ���ϰ��
	/// </summary>
	extern std::string exePath;

	/// <summary>
	/// ���� ����ǰ��ִ� ��������
	/// </summary>
	extern std::string realCurrentDirectory;

	/// <summary>
	/// �ü���� 64��Ʈ
	/// </summary>
	extern bool os64bit;

	/// <summary>
	/// ���� �����ڱ������� ���α׷��� ������
	/// </summary>
	extern bool isElevation;


	extern HINSTANCE _hInstance;
	extern HICON hLauncherIcon;
	extern HICON hSuccessIcon;
	extern HICON hErrorIcon;
	extern HICON hWarnIcon;
	extern HANDLE hMutex;
}
