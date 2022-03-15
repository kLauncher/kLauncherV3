#pragma once
#include "exception.h"

/// <summary>
/// ��������
/// </summary>
/// <param name="min">�ּ�</param>
/// <param name="max">�ִ�</param>
/// <returns>��������</returns>
int GetRandom(int min, int max);

/// <summary>
/// Cert Context �� ������ �������� �Լ�
/// </summary>
/// <param name="pCertCtx">Cert Context</param>
/// <returns></returns>
std::string GetCertificateDescription(PCCERT_CONTEXT pCertCtx);

/// <summary>
///	StarCraft �� ��ġ�Ǿ��ִ���(�����ϴ���) Ȯ���ϴ� �Լ�
/// </summary>
/// <exception cref="FileNotFound">������ ã���� ����</exception>
/// <returns type="std::string">���ڵ�۰��</returns>
bool GameExist();

/// <summary>
///	��Ʋ�� �� �ּҸ� ã�� �Լ�
///	<para>���������� ���ڵ尡 �´��� �˻�</para>
/// </summary>
/// <exception cref="FileNotFound">������ ã���� ����</exception>
/// <exception cref="CertException">����������(�ջ������)</exception>
/// <returns type="std::string">���ڵ�۰��</returns>
std::string FindGameAppLocation();

/// <summary>���Ϲ����� �˾ƿ��� �Լ�</summary>
/// <param name="filePath"></param>
/// <exception cref="VersionException">���� ������ ã���� ������</exception>
/// <returns>���Ϲ���</returns>
std::string GetFileVersion(std::string filePath);

/// <summary>
/// PID to filePath
/// </summary>
/// <param name="processID">PID</param>
/// <exception cref="LocationException">���ϰ�θ� ã���� ������</exception>
/// <returns>���ϰ��</returns>
std::string GetProcessLocation(DWORD processID);

/// <summary>
/// StarCraft.exe PID �� ã�� �Լ�
/// </summary>
/// <returns>��ã���� 0</returns>
DWORD FindGameProcessID();

/// <summary>��Ʋ��App �� �����ִ��� Ȯ���ϴ� �Լ�</summary>
/// <returns>PID��ȯ ��ã���� 0</returns>
DWORD FindGameApp();

/// <summary>
/// ���μ����� ���ϰ�θ� ã�� �Լ�
/// </summary>
/// <param name="pid">pid</param>
/// <returns>���ϰ��</returns>
std::string GetProcessFilePath(DWORD pid);

/// <summary>
/// fileTime to uint64_t
/// </summary>
/// <param name="ft"></param>
/// <returns></returns>
uint64_t fileTimeTo64(FILETIME ft);

/// <summary>
/// ���μ����� ����ð��� �˾ƿ��� �Լ�
/// </summary>
/// <param name="pid">pid</param>
/// <exception cref="AccessDeniedException">������ ������ ������</exception>
/// <returns></returns>
uint64_t GetProcessRunningTime(DWORD pid);

/// <summary>
/// ���μ����� DLL �� �������ϴ� �Լ�
/// </summary>
/// <param name="pid">Process id</param>
/// <param name="dllPath">dll path</param>
/// <returns></returns>
bool InjectionDll(DWORD pid, const std::string dllPath);

/// <summary>
/// ���� ������ �������� Ȯ���ϴ� �Լ�
/// </summary>
/// <returns></returns>
BOOL IsElevated();

/// <summary>
/// �ش� pid �� 64��Ʈ���� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="pid">pid</param>
/// <returns></returns>
bool is64bitProcess(DWORD pid);

/// <summary>
/// �ش� ���μ����� ������ �������� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="pid">pid</param>
/// <returns></returns>
bool isElevationProcess(DWORD pid);

/// <summary>
/// ������ �����ϴ��� Ȯ���ϴ� �Լ�
/// </summary>
/// <param name="filePath">���ϰ��</param>
/// <returns></returns>
bool file_exist(std::string filePath);


typedef std::function<void(void*)> deleter;
typedef std::function<LSTATUS(HKEY)> reg_deleter;
