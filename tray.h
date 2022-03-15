#pragma once
/*
	윈도우 트레이 아이콘
*/
class Tray {
public:
	void Init();
	Tray();
	Tray(HWND hWnd);
	void SetID(int id);
	void SetCallBackMessage(unsigned int id);

	void SetIcon(HICON main, HICON success, HICON error, HICON warn);
	void SetTip(const char* text);
	void SetInfoTitle(const char* text);
	void SetInfo(const char* text);
	void Apply();

	void ShowMessage(const char* message);
	void ShowErrorMessage(const char* message);
	void ShowWarningMessage(const char* message);
private:
	HWND _hWnd = nullptr;
	HICON _hIcon = nullptr;
	HICON _hSuccessIcon = nullptr;
	HICON _hErrorIcon = nullptr;
	HICON _hWarnIcon = nullptr;

	NOTIFYICONDATA nid = { 0, };
	int _id;
	unsigned int _uCallback;
	const char* tip;
	const char* infotitle;
	const char* info;
};

