#pragma once

#include <string>

namespace UWIN
{
	unsigned int GetFileSize(const char* filename);
	void GenerateCurrentFullPath(const char* filename, char* fullpath);
	void DeleteReadOnlyFile(const char* filename);
	void DeleteReadOnlyFiles(const char* filename);
	int IsDots(const char* str);
	int DeleteDirectory(const char* path);
	void SetWindowVisible(HWND hWnd, bool visible);
	bool GetExeFilename(HWND hWnd, char* filename);
	bool FindProcessByModuleName(const char* moduleName);
	void* BeginThreadEx(unsigned(__stdcall* startAddress)(void*), void* arg);
	int axtoi(char* hex);
	int IP2Integer(const char* ip);
	void AlterChar(std::string& str, char from, char to);
	int GetLocalMacAddress(char* buf, int len, char sep);
}
