#include "pch.h"
#include "string.h"

std::wstring String::to_wstring(const std::string dllPath) {
	const int cbBuffer = 500;

	std::wstring unicode;
	wchar_t strUnicode[cbBuffer] = { 0, };
	int characters = MultiByteToWideChar(CP_ACP, 0, dllPath.c_str(), dllPath.size(), nullptr, 0);
	unicode.resize(characters);
	MultiByteToWideChar(CP_ACP, 0, dllPath.c_str(), dllPath.size(), unicode.data(), unicode.size());
	return unicode;
}
