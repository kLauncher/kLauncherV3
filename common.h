#pragma once
#include "exception.h"

/// <summary>
/// 랜덤숫자
/// </summary>
/// <param name="min">최소</param>
/// <param name="max">최대</param>
/// <returns>랜덤정수</returns>
int GetRandom(int min, int max);

/// <summary>
/// Cert Context 의 설명을 가져오는 함수
/// </summary>
/// <param name="pCertCtx">Cert Context</param>
/// <returns></returns>
std::string GetCertificateDescription(PCCERT_CONTEXT pCertCtx);

/// <summary>
///	StarCraft 가 설치되어있는지(존재하는지) 확인하는 함수
/// </summary>
/// <exception cref="FileNotFound">파일을 찾을수 없음</exception>
/// <returns type="std::string">블리자드앱경로</returns>
bool GameExist();

/// <summary>
///	배틀넷 앱 주소를 찾는 함수
///	<para>인증서까지 블리자드가 맞는지 검사</para>
/// </summary>
/// <exception cref="FileNotFound">파일을 찾을수 없음</exception>
/// <exception cref="CertException">인증서오류(손상된파일)</exception>
/// <returns type="std::string">블리자드앱경로</returns>
std::string FindGameAppLocation();

/// <summary>파일버전을 알아오는 함수</summary>
/// <param name="filePath"></param>
/// <exception cref="VersionException">파일 버전을 찾을수 없을때</exception>
/// <returns>파일버전</returns>
std::string GetFileVersion(std::string filePath);

/// <summary>
/// PID to filePath
/// </summary>
/// <param name="processID">PID</param>
/// <exception cref="LocationException">파일경로를 찾을수 없을때</exception>
/// <returns>파일경로</returns>
std::string GetProcessLocation(DWORD processID);

/// <summary>
/// StarCraft.exe PID 를 찾는 함수
/// </summary>
/// <returns>못찾으면 0</returns>
DWORD FindGameProcessID();

/// <summary>배틀넷App 이 켜져있는지 확인하는 함수</summary>
/// <returns>PID반환 못찾으면 0</returns>
DWORD FindGameApp();

/// <summary>
/// 프로세스의 파일경로를 찾는 함수
/// </summary>
/// <param name="pid">pid</param>
/// <returns>파일경로</returns>
std::string GetProcessFilePath(DWORD pid);

/// <summary>
/// fileTime to uint64_t
/// </summary>
/// <param name="ft"></param>
/// <returns></returns>
uint64_t fileTimeTo64(FILETIME ft);

/// <summary>
/// 프로세스의 실행시간을 알아오는 함수
/// </summary>
/// <param name="pid">pid</param>
/// <exception cref="AccessDeniedException">엑세스 권한이 없을때</exception>
/// <returns></returns>
uint64_t GetProcessRunningTime(DWORD pid);

/// <summary>
/// 프로세스에 DLL 을 인젝션하는 함수
/// </summary>
/// <param name="pid">Process id</param>
/// <param name="dllPath">dll path</param>
/// <returns></returns>
bool InjectionDll(DWORD pid, const std::string dllPath);

/// <summary>
/// 현재 관리자 권한인지 확인하는 함수
/// </summary>
/// <returns></returns>
BOOL IsElevated();

/// <summary>
/// 해당 pid 가 64비트인지 확인하는 함수
/// </summary>
/// <param name="pid">pid</param>
/// <returns></returns>
bool is64bitProcess(DWORD pid);

/// <summary>
/// 해당 프로세스가 관리자 권한인지 확인하는 함수
/// </summary>
/// <param name="pid">pid</param>
/// <returns></returns>
bool isElevationProcess(DWORD pid);

/// <summary>
/// 파일이 존재하는지 확인하는 함수
/// </summary>
/// <param name="filePath">파일경로</param>
/// <returns></returns>
bool file_exist(std::string filePath);


typedef std::function<void(void*)> deleter;
typedef std::function<LSTATUS(HKEY)> reg_deleter;
