#include "pch.h"
#include "exception.h"

std::unordered_map<MyError, const char*> errorMessage = {
	{ MyError::FAIL_INIT, "kLauncher.exe �ʱ�ȭ�� �����Ͽ����ϴ�."}
};

FileNotFoundException::FileNotFoundException(std::string message) {
	_message = message;
}

const char* FileNotFoundException::what() const noexcept {
	return _message.c_str();
}


const char* CertException::what() const noexcept {
	return "Certificate error";
}

const char* VersionException::what() const noexcept {
	return "Version error";
}

const char* ProcessNotFoundException::what() const noexcept {
	return "Process is not found";
}

const char* LocationException::what() const noexcept {
	return "Process is not found";
}

const char* MutexException::what() const noexcept {
	return "Mutex error";
}


const char* AccessDeniedException::what() const noexcept {
	return "Mutex error";
}

const char* MemoryViolationException::what() const noexcept {
	return "Mutex error";
}
