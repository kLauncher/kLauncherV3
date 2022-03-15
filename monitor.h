#pragma once

enum eGameStatus : int {
	/// <summary>
	/// -1 로딩중
	/// </summary>
	LOADING = -1,

	/// <summary>
	/// 아무것도 실행되어있지 않은 상태, 하지만 언제든지 실행 가능한 상태
	/// </summary>
	NOTHING = 0,

	/// <summary>
	/// k런처가 정상적으로 로딩된 상태
	/// </summary>
	RUN_KDETECT = 1,

	/// <summary>
	/// 스타크래프트만 켜져있는 상태
	/// </summary>
	RUN_STARCRAFT = 2,

	/// <summary>
	/// 배틀넷App 런처만 켜져있는 상태
	/// </summary>
	RUN_APP = 3,

	/// <summary>
	///  스타를 찾을수 없음
	/// </summary>
	NOTFOUND_SC = 4,

	/// <summary>
	/// 배틀넷App을 찾을수 없음
	/// </summary>
	NOTFOUND_APP = 5,

	/// <summary>
	/// 구성하는 DLL 을 찾을수 없음
	/// </summary>
	NOTFOUND_DLL = 6,
};


enum eGameAppModuleStatus : int {
	LOAD_WAIT = 1,
	LOAD_SUCCESS = 2,
	LOAD_ERROR = 3,
	LOAD_UNNECESSARY = 4
};

// 모니터링 class
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
