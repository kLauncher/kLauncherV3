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

	// �����ϴ� DLL Ȯ��
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

	// 1. kDetector �� ���������� �ε尡 �Ǿ��°�?
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, "kDetecotrV3");
	if (hMutex) {
		CloseHandle(hMutex);
		status = eGameStatus::RUN_KDETECT;
		return;
	}

	// 2. ��Ÿũ����Ʈ�� �������ΰ�?
	sc_pid = FindGameProcessID();
	if (sc_pid) {
		// 64��Ʈ�� �������ΰ�?
		sc_64bit = is64bitProcess(sc_pid);

		// �����ڱ����ΰ�?
		isElevation = isElevationProcess(sc_pid);

		status = eGameStatus::RUN_STARCRAFT;
		return;
	}


	// 3. ��Ʋ��App �� �������ΰ�?
	app_pid = FindGameApp();
	if (app_pid) {
		gameAppLocation = GetProcessFilePath(app_pid);

		// �ý����� 64��Ʈ�϶�, ��� �ڵ���ȯ ����� ������ �Ǿ���?
		if (Config::os64bit) {
			hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, "kLauncherBattleNet");
			if (hMutex) {
				bMutexBattleNet = eGameAppModuleStatus::LOAD_SUCCESS;
				CloseHandle(hMutex);
			}
			else {
				bMutexBattleNet = eGameAppModuleStatus::LOAD_ERROR;

				// App �������� 10�ʰ� �����ٸ� ������
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
								// kernel32::LoadLibraryW �� �����ؿ��°��
							}
							catch (const AccessDeniedException&) {
								// ������ ���°��
							}
						}
					}

					// 15�ʰ� �ȳѾ��
					if (running_time < 15000) {
						bMutexBattleNet = eGameAppModuleStatus::LOAD_WAIT;
					}
				}
				catch (...) {

				}


			}
		}
		else {
			// 32��Ʈ���
			bMutexBattleNet = eGameAppModuleStatus::LOAD_UNNECESSARY;
		}
		status = eGameStatus::RUN_APP;
		return;
	}

	// 4. ��Ʋ��App ��θ� ã���� ���°�?
	try {
		gameAppLocation = FindGameAppLocation();
		printf("monitor %s\n", gameAppLocation.c_str());
	}
	catch (...) {
		status = eGameStatus::NOTFOUND_APP;
		return;
	}

	// 5. ��Ʋ��App �� ������ ��Ÿ�� ��ġ�Ǿ����� �ʳ�? 
	try {
		isGameInstalled = GameExist();
	}
	catch (...) {
		status = eGameStatus::NOTFOUND_SC;
		return;
	}


	// �ƹ��͵� ������� ���� ���� (������ ���� ������ ����)
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

			// ������ �ٸ��ٸ�
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