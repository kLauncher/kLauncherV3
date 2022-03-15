#include "pch.h"
#include "common.h"
#include "exception.h"
#include "string.h"
#include "config.h"

int GetRandom(int min, int max) {
	std::random_device rd;
	std::mt19937 mersenne(rd());
	std::uniform_int_distribution<int> die(min, max);
	return die(mersenne);
}

std::string GetCertificateDescription(PCCERT_CONTEXT pCertCtx) {
	std::string szSubjectRDN;
	DWORD dwStrType;
	DWORD dwLength;
	dwStrType = CERT_X500_NAME_STR;
	dwLength = CertGetNameString(pCertCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, &dwStrType, NULL, 0);
	if (dwLength > 0) {
		szSubjectRDN.resize(dwLength - 1, 0);
		CertGetNameString(pCertCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, &dwStrType, (LPSTR)szSubjectRDN.data(), dwLength);
	}
	return szSubjectRDN;
}

bool GameExist() {
	HKEY hKey;
	LONG lResult;

	lResult = RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\StarCraft", &hKey);
	std::unique_ptr<HKEY__, reg_deleter> ptr(hKey, &RegCloseKey);

	if (lResult != ERROR_SUCCESS)
		throw FileNotFoundException("StarCraft is not found");

	return true;
}

std::string FindGameAppLocation() {
	HKEY hKey;
	LONG lResult;
	DWORD dwType = 0;
	DWORD dwLen = 0;

	lResult = RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Battle.net", &hKey);
	std::shared_ptr<void> ptr(hKey, &RegCloseKey);

	if (lResult != ERROR_SUCCESS)
		throw FileNotFoundException("Battle.Net is not found");

	lResult = RegGetValueA(hKey, nullptr, "InstallLocation", RRF_RT_REG_SZ, &dwType, nullptr, &dwLen);

	if (lResult != ERROR_SUCCESS || !dwLen)
		throw FileNotFoundException("Battle.Net is not found");


	std::string filePath(dwLen, 0);
	lResult = RegGetValueA(hKey, nullptr, "InstallLocation", RRF_RT_REG_SZ, &dwType, (PVOID)filePath.data(), &dwLen);
	if (lResult != ERROR_SUCCESS)
		throw FileNotFoundException("Battle.Net is not found");

	filePath = std::string(filePath.c_str());
	filePath += "\\Battle.net.exe";
	if (!PathFileExists(filePath.c_str()))
		throw FileNotFoundException("Battle.Net is not found");


	// 인증서 검사
	WINTRUST_FILE_INFO sWintrustFileInfo = { 0, };
	WINTRUST_DATA sWintrustData = { 0, };
	std::wstring wFilePath;
	wFilePath.assign(filePath.begin(), filePath.end());

	sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
	sWintrustFileInfo.pcwszFilePath = wFilePath.c_str();
	sWintrustFileInfo.hFile = nullptr;

	sWintrustData.cbStruct = sizeof(WINTRUST_DATA);
	sWintrustData.dwUIChoice = WTD_UI_NONE;
	sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	sWintrustData.dwUnionChoice = WTD_CHOICE_FILE;
	sWintrustData.pFile = &sWintrustFileInfo;
	sWintrustData.dwStateAction = WTD_STATEACTION_VERIFY;

	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	HRESULT hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
	// @ref https://docs.microsoft.com/en-us/windows/win32/seccrypto/wthelpergetfilehash

	bool bCertSuccess = false;

	CRYPT_PROVIDER_DATA const* psProvData = nullptr;
	CRYPT_PROVIDER_SGNR* psProvSigner = nullptr;
	CRYPT_PROVIDER_CERT* psProvCert = nullptr;
	psProvData = WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
	if (psProvData) {
		psProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0, FALSE, 0);
		if (psProvSigner) {
			psProvCert = WTHelperGetProvCertFromChain(psProvSigner, 0);
			if (psProvCert) {
				std::string szCertDesc = GetCertificateDescription(psProvCert->pCert);
				if (szCertDesc.compare("Blizzard Entertainment, Inc.") == 0) {
					bCertSuccess = true;
				}
			}
		}
	}

	sWintrustData.dwUIChoice = WTD_UI_NONE;
	sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
	WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

	if (!bCertSuccess)
		throw new CertException();

	return filePath;
}


std::string GetFileVersion(std::string filePath) {
	DWORD infoSize = GetFileVersionInfoSize(filePath.c_str(), 0);
	if (infoSize == 0)
		throw VersionException();

	std::vector<unsigned char> buffer(infoSize);

	if (!GetFileVersionInfo(filePath.c_str(), 0, infoSize, buffer.data()))
		throw VersionException();

	VS_FIXEDFILEINFO* pFineInfo = NULL;
	UINT bufLen = 0;
	if (!VerQueryValue(buffer.data(), "\\", (LPVOID*)&pFineInfo, &bufLen))
		throw VersionException();

	WORD majorVer, minorVer, buildNum, revisionNum;
	majorVer = HIWORD(pFineInfo->dwFileVersionMS);
	minorVer = LOWORD(pFineInfo->dwFileVersionMS);
	buildNum = HIWORD(pFineInfo->dwFileVersionLS);
	revisionNum = LOWORD(pFineInfo->dwFileVersionLS);
	return String::format("%d.%d.%d.%d", majorVer, minorVer, buildNum, revisionNum);
}

std::string GetProcessLocation(DWORD processID) {
	std::shared_ptr<void> hProcess(
		OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID),
		&CloseHandle);

	char fileName[MAX_PATH] = { 0, };
	if (hProcess.get()) {
		if (GetModuleFileNameEx(hProcess.get(), NULL, fileName, MAX_PATH)) {
			return fileName;
		}
	}
	throw LocationException();
}


DWORD FindGameProcessID() {
	//HWND hWnd = FindWindow("OsWindow", "Brood War");
	//if (hWnd) {
	//	DWORD processID = 0;
	//	GetWindowThreadProcessId(hWnd, &processID);
	//	if (processID)
	//		return processID;
	//}

	std::shared_ptr<void> hSnapshot(
		CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL),
		&CloseHandle);

	PROCESSENTRY32 entry = { 0, };
	entry.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot.get(), &entry)) {
		do {
			if (_stricmp(entry.szExeFile, "StarCraft.exe") == 0) {
				return entry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot.get(), &entry));
	}
	return 0;
}

DWORD FindGameApp() {
	std::shared_ptr<void> hSnapshot(
		CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL),
		&CloseHandle);

	PROCESSENTRY32 entry = { 0, };
	entry.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot.get(), &entry)) {
		do {
			if (_stricmp(entry.szExeFile, "Battle.net.exe") == 0) {
				return entry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot.get(), &entry));
	}
	return false;
}

std::string GetProcessFilePath(DWORD pid) {
	std::shared_ptr<void> process(
		OpenProcess(MAXIMUM_ALLOWED, false, pid),
		&CloseHandle);


	if (process.get() == nullptr) {
		// getlasterror ERROR_ACCESS_DENIED
		// throw 
	}

	char filePath[MAX_PATH] = { 0, };
	DWORD dwLen = _countof(filePath);

	QueryFullProcessImageName(process.get(), 0, filePath, &dwLen);
	return filePath;
}

uint64_t fileTimeTo64(FILETIME ft) {
	LARGE_INTEGER a2 = { 0, };
	a2.LowPart = ft.dwLowDateTime;
	a2.HighPart = ft.dwHighDateTime;
	return a2.QuadPart;
}

uint64_t GetProcessRunningTime(DWORD pid) {
	std::shared_ptr<void> process(
		OpenProcess(PROCESS_QUERY_INFORMATION, false, pid),
		&CloseHandle);

	if (process.get() == nullptr) {
		throw AccessDeniedException();
	}

	// get process time
	FILETIME createTime = { 0, };
	FILETIME dummy = { 0, };
	GetProcessTimes(process.get(), &createTime, &dummy, &dummy, &dummy);

	// get current time
	SYSTEMTIME currentTime = { 0, };
	GetSystemTime(&currentTime);

	FILETIME currentFileTime = { 0, };
	SystemTimeToFileTime(&currentTime, &currentFileTime);

	return (fileTimeTo64(currentFileTime) - fileTimeTo64(createTime)) / 10000;
}


bool InjectionDll(DWORD pid, const std::string dllPath) {
	// TODO: EAT 동적으로 구해서 함수이름 hash 해서 인젝션하기
	// https://stackoverflow.com/questions/27939882/fast-crc-algorithm

	// 유니코드 변환
	std::wstring wDllPath = String::to_wstring(dllPath);
	int bufferSize = wDllPath.size() * 2;

	// 호출할 주소 구하기
	HMODULE hMod = GetModuleHandle("kernel32.dll");
	if (!hMod) {
		hMod = LoadLibrary("kernel32.dll");
		if (!hMod)
			throw MemoryViolationException();
	}
	LPTHREAD_START_ROUTINE lpRemoteThread = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");
	if (!lpRemoteThread)
		throw MemoryViolationException();

	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, pid);
	if (!hProcess)  throw AccessDeniedException();
	std::unique_ptr<void, deleter> process((void*)hProcess, &CloseHandle);

	LPVOID lpRemoteBuffer = VirtualAllocEx(process.get(), nullptr, bufferSize + 2, MEM_COMMIT, PAGE_READWRITE);
	if (!lpRemoteBuffer) throw AccessDeniedException();
	std::unique_ptr<void, deleter> remoteBuffer(lpRemoteBuffer, [&process](void* p) {
		VirtualFreeEx(process.get(), p, 0, MEM_RELEASE);
	});

	if (!WriteProcessMemory(process.get(), remoteBuffer.get(), wDllPath.c_str(), bufferSize, nullptr))
		throw AccessDeniedException();

	HANDLE hThread = CreateRemoteThread(process.get(), nullptr, 0, lpRemoteThread, remoteBuffer.get(), 0, nullptr);
	if (!hThread) throw AccessDeniedException();
	std::unique_ptr<void, deleter> thread(hThread, [](void* p) {
		CloseHandle(p);
	});

	WaitForSingleObject(thread.get(), INFINITE);

	DWORD dwExitCode = 0;
	GetExitCodeThread(thread.get(), &dwExitCode);
	return !!dwExitCode;
}

BOOL IsElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}

bool is64bitProcess(DWORD pid) {
	if (!Config::os64bit)
		return false;

	BOOL wow64 = false;
	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, false, pid);

	if (hProcess) {
		IsWow64Process(hProcess, &wow64);
		CloseHandle(hProcess);
	}

	return !wow64;

}

bool isElevationProcess(DWORD pid) {

	std::shared_ptr<void> process(
		OpenProcess(PROCESS_QUERY_INFORMATION, false, pid),
		&CloseHandle);

	if (process.get()) {
		HANDLE hToken = nullptr;
		std::shared_ptr<void> token(
			OpenProcessToken(process.get(), TOKEN_QUERY, &hToken) ? hToken : nullptr,
			&CloseHandle);

		if (token.get()) {
			DWORD dwSize = 0;
			TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
			// TokenElevationTypeDefault -- User is not using a split token. (e.g. UAC disabled or local admin "Administrator" account which UAC may not apply to.)
			// TokenElevationTypeFull    -- User has a split token, and the process is running elevated.
			// TokenElevationTypeLimited -- User has a split token, but the process is not running elevated.
			BOOL bTokenInfo = GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(elevationType), &dwSize);
			return (bTokenInfo && dwSize) && elevationType != TokenElevationTypeDefault;
		}
	}

	return true;
}

bool file_exist(std::string filePath) {
	return PathFileExists(filePath.c_str());
}