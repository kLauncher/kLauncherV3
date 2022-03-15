#pragma once
#include "pch.h"
enum class MyError {
	FAIL_INIT,
};
extern std::unordered_map<MyError, const char*> errorMessage;

struct MyException : public std::exception {
	MyError _err;

	MyException(MyError err) {
		_err = err;
	}
	const char* what() const throw() {
		return errorMessage[_err];
	}
};


class FileNotFoundException : public std::exception {
public:
	FileNotFoundException(std::string message);
	const char* what() const noexcept override;
private:
	std::string _message;
};

class CertException : public std::exception {
public:
	const char* what() const noexcept override;
};

class VersionException : public std::exception {
public:
	const char* what() const noexcept override;
};

class ProcessNotFoundException : public std::exception {
public:
	const char* what() const noexcept override;
};

class LocationException : public std::exception {
public:
	const char* what() const noexcept override;
};


class MutexException : public std::exception {
public:
	const char* what() const noexcept override;
};

class AccessDeniedException : public std::exception {
public:
	const char* what() const noexcept override;
};

class MemoryViolationException : public std::exception {
public:
	const char* what() const noexcept override;
};


