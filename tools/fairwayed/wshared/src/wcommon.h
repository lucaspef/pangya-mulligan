// SPDX-License-Identifier: MIT
// wcommon.h - text encodings, paths and the data folder index.
#ifndef WCOMMON_H
#define WCOMMON_H

#include <stdio.h>
#include <map>
#include <string>
#include <vector>

// The game's strings are CP949; paths and the UI are UTF-8 or UTF-16.
std::wstring Wide(const std::string& utf8);
std::string Narrow(const std::wstring& w);
std::wstring WideCp(const std::string& cp949);
std::string NarrowCp(const std::wstring& w);
std::string Cp949ToUtf8(const std::string& s);
std::string Utf8ToCp949(const std::string& s);
// ASCII only, stepping over CP949 pairs: a trail byte can look like a capital.
std::string LowerCp949(std::string s);

// All paths are UTF-8, whatever the system code page is.
FILE* OpenUtf8(const char* path, const wchar_t* mode);
bool ReadWholeFile(const std::string& path, std::vector<unsigned char>* out);
bool WriteWholeFile(const std::string& path, const std::vector<unsigned char>& data, std::string* error);

std::string PathBaseName(const std::string& path);
std::string PathStem(const std::string& path);	// the base name without its extension
std::string PathDir(const std::string& path);
std::string PathJoin(const std::string& dir, const std::string& name);
bool PathExists(const std::string& path);

// Every file with one of these extensions below root, by LowerCp949(CP949 name) -> UTF-8 path.
// The first file found with a name wins.
void IndexAssets(const std::string& root, const wchar_t* const* extensions, size_t count,
	std::map<std::string, std::string>* index);

#endif // WCOMMON_H
