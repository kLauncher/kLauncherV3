#include "pch.h"
#include "main.h"
#include "message.h"

#include "exception.h"
#include "socket.h"
#include "tray.h"
#include "common.h"
#include "config.h"
#include "monitor.h"

Tray CNotify;
HWND hMainWindow = nullptr;
int error_code = 0;

int kLauncherStart(HINSTANCE hInstance) {

	try {
		Config::Config(hInstance);
	}
	catch (const MutexException& err) {
		throw err;
	}

	WNDCLASSEX wcex = { 0, };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = hInstance;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = CLASSNAME;
	wcex.hIcon = Config::hLauncherIcon;
	wcex.hIconSm = Config::hLauncherIcon;
	if (!RegisterClassEx(&wcex)) {
		throw MyException(MyError::FAIL_INIT);
	}

	hMainWindow = CreateWindow(CLASSNAME, "", WS_OVERLAPPEDWINDOW, 0, 1, 0, 1, nullptr, nullptr, hInstance, NULL);
	if (!hMainWindow) {
		throw MyException(MyError::FAIL_INIT);
	}


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

bool OnCreate(HWND hWnd) {

	// �⺻ Ʈ���� ����
	CNotify = Tray(hWnd);
	CNotify.SetID(ID_MENU);
	CNotify.SetIcon(
		Config::hLauncherIcon,
		Config::hSuccessIcon,
		Config::hErrorIcon,
		Config::hWarnIcon);

	CNotify.SetTip("kLauncher " VERSION);
	CNotify.SetInfoTitle("kLauncher " VERSION);
	//CNotify.SetInfo("kLauncher�� ���������� ����Ǿ����ϴ�.");
	CNotify.Apply();

	// ������ �������� ���
	if (!Socket::Start()) {
		error_code = 1;
		CNotify.SetCallBackMessage(TRAY_ERROR);
		CNotify.ShowErrorMessage("kLauncher ���࿡ ������ �߻��Ͽ����ϴ�.");
		return false;
	}

	Monitor::Monitor();
	Monitor::Start();


	httplib::Client cli(SERVER_ADDRESS);
	httplib::Result res = cli.Get("/forum/api/v1/version.json");
	if (res) {
		try {
			json j = json::parse(res->body);
			if (j[VERSION]["letest"].get<bool>()) {
				CNotify.SetCallBackMessage(TRAY_SUCCESS);
				CNotify.ShowMessage("kLauncher�� ���������� ����Ǿ����ϴ�.");
			}
			else {
				CNotify.SetCallBackMessage(TRAY_UPDATE);
				CNotify.ShowWarningMessage("���ο� ������Ʈ�� �ֽ��ϴ�.\n��������� ��밡���մϴ�.");
			}
		}
		catch (...) {
			CNotify.SetCallBackMessage(TRAY_UPDATE);
			CNotify.ShowErrorMessage("���ο� ������Ʈ�� �ֽ��ϴ�.\n��������� ����ϽǼ� �����ϴ�.");
		}
	}
	else {
		CNotify.SetCallBackMessage(TRAY_SERVER_FAILED);
		CNotify.ShowErrorMessage("������ ������°� �Ҿ����մϴ�. ����� �ٽ� �õ����ּ���.\n��� ������ ���ӵǸ� Ȩ���������� �缳ġ���ּ���.");
	}


	return 0;
}

void ShowOpen() {
	bool isDefaultPort = Socket::isDefaultPort();
	std::string url;
	if (isDefaultPort) {
		url = "http://127.0.0.1:8080";
	}
	else {
		url = "http://127.0.0.1:8080/port/" + std::to_string(Socket::GetPort());
	}

	ShellExecute(hMainWindow, "open", "explorer.exe", url.c_str(), nullptr, SW_SHOW);
}

void ShowError() {
	std::string url = "http://127.0.0.1:8080/error/" + std::to_string(error_code);
	ShellExecute(hMainWindow, "open", "explorer.exe", url.c_str(), nullptr, SW_SHOW);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	printf("message:%08X wParam:%08X lParam:%08X\n", message, wParam, lParam);
	// 0x402 0x405
	switch (message) {
		case WM_CREATE:
			return OnCreate(hWnd);

		case TRAY_ERROR:
			switch (lParam) {
				// ����ڰ� ǳ���� Ŭ��������
				case NIN_BALLOONUSERCLICK:
					ShowError();
					break;

				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				{
					SetForegroundWindow(hWnd);

					HMENU hPop = CreatePopupMenu();
					InsertMenu(hPop, 0, MF_BYPOSITION | MF_STRING, ID_ERROR, "����");
					InsertMenu(hPop, 1, MF_BYPOSITION | MF_STRING, ID_EXIT, "����");
					SetMenuDefaultItem(hPop, ID_ABOUT, FALSE);
					SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0);

					POINT curpos;
					GetCursorPos(&curpos);

					WORD cmd = TrackPopupMenu(hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos.x, curpos.y, 0, hWnd, NULL);
					SendMessage(hWnd, WM_COMMAND, cmd, 0);
					DestroyMenu(hPop);
					PostMessage(hWnd, WM_APP + 1, 0, 0);
					SetForegroundWindow(hWnd);
					break;
				}
			}
			break;

		case TRAY_SUCCESS:
			switch (lParam) {
				case NIN_BALLOONUSERCLICK:
					ShowOpen();
					// ǳ��Ŭ��������
					break;

				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				{
					SetForegroundWindow(hWnd);

					HMENU hPop = CreatePopupMenu();
					InsertMenu(hPop, 0, MF_BYPOSITION | MF_STRING, ID_ABOUT, "About...");
					InsertMenu(hPop, 1, MF_BYPOSITION | MF_STRING, ID_EXIT, "Exit");
					SetMenuDefaultItem(hPop, ID_ABOUT, FALSE);
					SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0);

					POINT curpos;
					GetCursorPos(&curpos);

					WORD cmd = TrackPopupMenu(hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos.x, curpos.y, 0, hWnd, NULL);
					SendMessage(hWnd, WM_COMMAND, cmd, 0);
					DestroyMenu(hPop);
					PostMessage(hWnd, WM_APP + 1, 0, 0);
					SetForegroundWindow(hWnd);
					break;
				}
			}
			break;

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			//printf("[WM_COMMAND] %d\n", wmId);
			switch (wmId) {
				case ID_OPEN:
					ShowOpen();
					break;

				case ID_ERROR:
					// ��������
					ShowError();
					break;
				case ID_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}

		case WM_CLOSE:
		{
			NOTIFYICONDATA nid = { 0, };
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hWnd;
			nid.uID = ID_MENU;
			Shell_NotifyIcon(NIM_DELETE, &nid);
			PostQuitMessage(0);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		case WM_DESTROY:
		{
			NOTIFYICONDATA nid = { 0, };
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hWnd;
			nid.uID = ID_MENU;
			Shell_NotifyIcon(NIM_DELETE, &nid);
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


int kLauncherMain(HINSTANCE hInstance, int argv, char** argc) {

	if (argv == 2) {
		if (strcmp(argc[1], "-fixGameAppModule") == 0) {
			try {
				Config::Config(hInstance, false);
				int app_pid = FindGameApp();
				bool result = InjectionDll(app_pid, Config::realCurrentDirectory + "\\Lib\\Battle.net.dll");
				return result;
			}
			catch (...) {
			}
		}

		if (strcmp(argc[1], "-load") == 0) {
			try {
				Config::Config(hInstance, false);

				while (true) {

					DWORD sc_pid = FindGameProcessID();
					if (sc_pid) {
						uint64_t running_time = GetProcessRunningTime(sc_pid);
						bool is64bit = is64bitProcess(sc_pid);
						if (running_time > 5000) {
							// 64��Ʈ�� �ƴϸ�
							if (!is64bit) {
								try {
									InjectionDll(sc_pid, Config::realCurrentDirectory + "\\Lib\\load.dll");
								}
								catch (...) {

								}
							}
							break;
						}
					}

					Sleep(1000);
				}
			}
			catch (...) {
			}
		}

		return -1;
	}

	int returnCode = 0;
	try {
		returnCode = kLauncherStart(hInstance);
	}
	//catch (const MutexException& err) {
		// �ߺ����� ��ƾ
	//}
	catch (MyException& e) {
		MessageBoxA(nullptr, e.what(), "kLauncher", MB_ICONERROR | MB_OK);
	}

	catch (std::exception& e) {
		MessageBoxA(nullptr, e.what(), "kLauncher", MB_ICONERROR | MB_OK);
	}

	Config::Free();

	return returnCode;
}


#ifdef CONSOLE
int main(int argv, char** argc) {
	//setlocale(LC_ALL, "");
	SetConsoleOutputCP(65001);
	return kLauncherMain(GetModuleHandle(NULL), argv, argc);
}
#else
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	return kLauncherMain(hInstance);
}
#endif