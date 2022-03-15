#include "pch.h"
#include "monitor.h"
#include "common.h"
#include "socket.h"
#include "string.h"
#include "config.h"


namespace Monitor {
	void Monitor();
	void Start();
	void Stop();
	int GetProcessID();

	void ThreadProc();
	void ThreadFunction();
	void lock();
	void unlock();

	eGameStatus status = eGameStatus::LOADING;
	std::string statusMessage;

	int sc_pid;
	int app_pid;
	int last_injection_gameApp_pid;

	bool isLock;
	bool isGameInstalled;
	bool sc_64bit;
	bool isElevation;
	std::string gameAppLocation;
	std::thread thread;
	std::thread file_monitor_thread;

	bool running;
	eGameAppModuleStatus bMutexBattleNet;
	SRWLOCK g_srwlock;

	uint32_t last_app_pid;


};

std::string libraryFiles[] = {
	"Lib\\load.dll", "Lib\\kDetector.k",
	"Lib\\Battle.net.dll", "Etc\\zrescue.wav",
	"Etc\\D2CodingBold-Ver1.3.2-20180524.ttf" };

namespace FileMonitor {
	void FileMonitor();
	void FileMonitorThreadProc();

	bool isRunning = false;
	eGameStatus status;
	std::string statusMessage;
	SRWLOCK g_srwlock;
	bool isSuccessful = true;
}


void Monitor::ThreadFunction() {
	HANDLE hMutex = nullptr;

	// 구성하는 DLL 확인
	if (FileMonitor::isRunning) {

		FileMonitor::lock();
		bool isSuccessful = FileMonitor::isSuccessful;
		if (!isSuccessful) {
			status = FileMonitor::status;
			statusMessage = FileMonitor::statusMessage;
		}
		FileMonitor::unlock();

		if (!isSuccessful)
			return;
	}
	else {

		for (auto& file : libraryFiles) {
			if (!file_exist(Config::realCurrentDirectory + "\\" + file)) {
				status = eGameStatus::NOTFOUND_DLL;
				statusMessage = String::format("%s is not exist", file.c_str());
			}
		}
	}

	// 1. kDetector 가 정상적으로 로드가 되었는가?
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, "kDetecotrV3");
	if (hMutex) {
		CloseHandle(hMutex);
		status = eGameStatus::RUN_KDETECT;
		return;
	}

	// 2. 스타크래프트는 실행중인가?
	sc_pid = FindGameProcessID();
	if (sc_pid) {
		// 64비트로 실행중인가?
		sc_64bit = is64bitProcess(sc_pid);

		// 관리자권한인가?
		isElevation = isElevationProcess(sc_pid);

		status = eGameStatus::RUN_STARCRAFT;
		return;
	}


	// 3. 배틀넷App 은 실행중인가?
	app_pid = FindGameApp();
	if (app_pid) {
		gameAppLocation = GetProcessFilePath(app_pid);

		// 시스템이 64비트일때, 경로 자동변환 모듈이 인젝션 되었나?
		if (Config::os64bit) {
			hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, "kLauncherBattleNet");
			if (hMutex) {
				bMutexBattleNet = eGameAppModuleStatus::LOAD_SUCCESS;
				CloseHandle(hMutex);
			}
			else {
				bMutexBattleNet = eGameAppModuleStatus::LOAD_ERROR;

				// App 실행한지 10초가 지났다면 인젝션
				try {
					uint64_t running_time = GetProcessRunningTime(app_pid);
					if (running_time > 10000) {
						if (last_injection_gameApp_pid != app_pid) {
							last_injection_gameApp_pid = app_pid;
							try {
								bool result = InjectionDll(app_pid, Config::realCurrentDirectory + "\\Lib\\Battle.net.dll");
								if (result) {
									bMutexBattleNet = eGameAppModuleStatus::LOAD_SUCCESS;
								}
							}
							catch (const MemoryViolationException&) {
								// kernel32::LoadLibraryW 를 못구해오는경우
							}
							catch (const AccessDeniedException&) {
								// 권한이 없는경우
							}
						}
					}

					// 15초가 안넘어가면
					if (running_time < 15000) {
						bMutexBattleNet = eGameAppModuleStatus::LOAD_WAIT;
					}
				}
				catch (...) {

				}


			}
		}
		else {
			// 32비트라면
			bMutexBattleNet = eGameAppModuleStatus::LOAD_UNNECESSARY;
		}
		status = eGameStatus::RUN_APP;
		return;
	}

	// 4. 배틀넷App 경로를 찾을수 없는가?
	try {
		gameAppLocation = FindGameAppLocation();
		printf("monitor %s\n", gameAppLocation.c_str());
	}
	catch (...) {
		status = eGameStatus::NOTFOUND_APP;
		return;
	}

	// 5. 배틀넷App 이 있지만 스타가 설치되어있지 않나? 
	try {
		isGameInstalled = GameExist();
	}
	catch (...) {
		status = eGameStatus::NOTFOUND_SC;
		return;
	}


	// 아무것도 실행되지 않은 상태 (언제든 실행 가능한 상태)
	status = eGameStatus::NOTHING;
}

void Monitor::ThreadProc() {
	running = true;

	int oldStatus;
	bool isChanged;
	eGameAppModuleStatus oldbMutexBattleNet;


	while (running) {
		//status = eGameStatus::LOADING;

		AcquireSRWLockExclusive(&g_srwlock);
		{
			sc_pid = 0;
			app_pid = 0;
			sc_64bit = false;
			isElevation = false;
			gameAppLocation = "";
			isGameInstalled = false;


			oldStatus = status;
			oldbMutexBattleNet = bMutexBattleNet;
			isChanged = false;

			ThreadFunction();

			// 변수가 다르다면
			if (oldStatus != status ||
				oldbMutexBattleNet != bMutexBattleNet) {
				isChanged = true;
			}
		}
		ReleaseSRWLockExclusive(&g_srwlock);


		if (isChanged) {
			Socket::onStatusChanged();
		}


		Sleep(1000);
	}
}

void Monitor::Monitor() {
	InitializeSRWLock(&g_srwlock);
}



void Monitor::Start() {
	file_monitor_thread = std::thread(FileMonitor::FileMonitorThreadProc);
	thread = std::thread(ThreadProc);
}

void Monitor::Stop() {
	running = false;
	thread.join();
}


void Monitor::lock() {
	AcquireSRWLockShared(&g_srwlock);
}

void Monitor::unlock() {
	ReleaseSRWLockShared(&g_srwlock);
}

int Monitor::GetProcessID() {
	return sc_pid;
}



void FileMonitor::FileMonitorThreadProc() {
	isRunning = true;

	std::wstring currentDir = String::to_wstring(Config::realCurrentDirectory);

	HANDLE hDir = CreateFileW(currentDir.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
	if (hDir == INVALID_HANDLE_VALUE) {
		isRunning = false;
		return;
	}

	const DWORD cbBuffer = 1024 * 1024;
	const DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

	LPBYTE pBuffer = new BYTE[cbBuffer];
	DWORD bytesReturned;
	wchar_t temp[MAX_PATH] = { 0 };

	while (isRunning) {

		FILE_NOTIFY_INFORMATION* pfni;
		BOOL fOk = ReadDirectoryChangesW(hDir, pBuffer, cbBuffer, TRUE, dwNotifyFilter, &bytesReturned, nullptr, nullptr);
		if (!fOk) {
			break;
		}

		pfni = (FILE_NOTIFY_INFORMATION*)pBuffer;

		//do {
		//	switch (pfni->Action) {
		//		case FILE_ACTION_ADDED:
		//			wprintf(L"FILE_ACTION_ADDED\n");
		//			break;
		//		case FILE_ACTION_REMOVED:
		//			wprintf(L"FILE_ACTION_REMOVED\n");
		//			break;
		//		case FILE_ACTION_MODIFIED:
		//			wprintf(L"FILE_ACTION_MODIFIED\n");
		//			break;
		//		case FILE_ACTION_RENAMED_OLD_NAME:
		//			wprintf(L"FILE_ACTION_RENAMED_OLD_NAME\n");
		//			break;
		//		case FILE_ACTION_RENAMED_NEW_NAME:
		//			wprintf(L"FILE_ACTION_RENAMED_NEW_NAME\n");
		//			break;
		//		default:
		//			break;
		//	}

		//	pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
		//} while (pfni->NextEntryOffset > 0);

		AcquireSRWLockExclusive(&g_srwlock);
		isSuccessful = true;
		for (auto& file : libraryFiles) {
			if (!file_exist(Config::realCurrentDirectory + "\\" + file)) {
				status = eGameStatus::NOTFOUND_DLL;
				statusMessage = String::format("%s is not exist", file.c_str());
				isSuccessful = false;
			}
		}
		ReleaseSRWLockExclusive(&g_srwlock);
	}

	isRunning = false;
}

void FileMonitor::FileMonitor() {
	InitializeSRWLock(&g_srwlock);
}
void FileMonitor::lock() {
	AcquireSRWLockShared(&g_srwlock);
}

void FileMonitor::unlock() {
	ReleaseSRWLockShared(&g_srwlock);
}