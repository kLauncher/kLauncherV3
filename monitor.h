#pragma once

enum eGameStatus : int {
	/// <summary>
	/// -1 �ε���
	/// </summary>
	LOADING = -1,

	/// <summary>
	/// �ƹ��͵� ����Ǿ����� ���� ����, ������ �������� ���� ������ ����
	/// </summary>
	NOTHING = 0,

	/// <summary>
	/// k��ó�� ���������� �ε��� ����
	/// </summary>
	RUN_KDETECT = 1,

	/// <summary>
	/// ��Ÿũ����Ʈ�� �����ִ� ����
	/// </summary>
	RUN_STARCRAFT = 2,

	/// <summary>
	/// ��Ʋ��App ��ó�� �����ִ� ����
	/// </summary>
	RUN_APP = 3,

	/// <summary>
	///  ��Ÿ�� ã���� ����
	/// </summary>
	NOTFOUND_SC = 4,

	/// <summary>
	/// ��Ʋ��App�� ã���� ����
	/// </summary>
	NOTFOUND_APP = 5,

	/// <summary>
	/// �����ϴ� DLL �� ã���� ����
	/// </summary>
	NOTFOUND_DLL = 6,
};


enum eGameAppModuleStatus : int {
	LOAD_WAIT = 1,
	LOAD_SUCCESS = 2,
	LOAD_ERROR = 3,
	LOAD_UNNECESSARY = 4
};

// ����͸� class
namespace Monitor {
	void Monitor();
	void Start();
	void Stop();
	int GetProcessID();

	void ThreadProc();
	void ThreadFunction();
	void lock();
	void unlock();

	extern eGameStatus status;
	extern std::string statusMessage;

	extern int sc_pid;
	extern int app_pid;
	extern int last_injection_gameApp_pid;

	extern bool isLock;
	extern bool isGameInstalled;
	extern bool sc_64bit;
	extern bool isElevation;
	extern std::string gameAppLocation;
	extern std::thread thread;
	extern bool running;
	extern eGameAppModuleStatus bMutexBattleNet;
	extern SRWLOCK g_srwlock;

	extern uint32_t last_app_pid;


}


namespace FileMonitor {
	void FileMonitor();
	void FileMonitorThreadProc();
	bool OnFileChanged();

	extern bool isRunning;
	extern bool isSuccessful;
	extern eGameStatus status;
	extern std::string statusMessage;

	extern SRWLOCK g_srwlock;
	extern void lock();
	extern void unlock();

}
