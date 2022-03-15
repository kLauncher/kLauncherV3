#include "pch.h"
#include "tray.h"

Tray::Tray() {
	Init();
}
Tray::Tray(HWND hWnd) {
	_hWnd = hWnd;
	Init();
}

void Tray::Init() {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uVersion = NOTIFYICON_VERSION_4;
}

void Tray::SetIcon(HICON main, HICON success, HICON error, HICON warn) {
	_hIcon = main;
	_hSuccessIcon = success;
	_hErrorIcon = error;
	_hWarnIcon = warn;
}

void Tray::SetTip(const char* text) {
	tip = text;
}

void Tray::SetInfoTitle(const char* text) {
	infotitle = text;
}

void Tray::SetInfo(const char* text) {
	info = text;
}

void Tray::SetID(int id) {
	_id = id;
}

void Tray::SetCallBackMessage(unsigned int uCallBack) {
	_uCallback = uCallBack;
}

void Tray::Apply() {
	nid.hWnd = _hWnd;
	nid.uID = _id;
	nid.dwInfoFlags = NIIF_USER;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP; // NIF_MESSAGE
	//nid.uCallbackMessage = _uCallback;
	nid.hIcon = _hIcon;
	strcpy_s(nid.szTip, tip);
	strcpy_s(nid.szInfoTitle, infotitle);
	strcpy_s(nid.szInfo, "");
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void Tray::ShowMessage(const char* message) {
	nid.hIcon = _hSuccessIcon;
	nid.uFlags |= NIF_INFO;
	strcpy_s(nid.szInfo, message);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void Tray::ShowErrorMessage(const char* message) {
	nid.hIcon = _hErrorIcon;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE | NIF_INFO;
	nid.uCallbackMessage = _uCallback;
	strcpy_s(nid.szTip, tip);
	strcpy_s(nid.szInfoTitle, infotitle);
	strcpy_s(nid.szInfo, message);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void Tray::ShowWarningMessage(const char* message) {
	nid.hIcon = _hWarnIcon;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE | NIF_INFO;
	nid.uCallbackMessage = _uCallback;
	strcpy_s(nid.szTip, tip);
	strcpy_s(nid.szInfoTitle, infotitle);
	strcpy_s(nid.szInfo, message);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

