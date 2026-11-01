// SPDX-License-Identifier: MIT
// wcommon.cpp - see wcommon.h.
#include "wcommon.h"
#include <windows.h>

static std::wstring ToWide(const std::string& s, UINT codepage)
{
	int n = MultiByteToWideChar(codepage, 0, s.c_str(), -1, 0, 0);
	std::wstring w(n > 0 ? n - 1 : 0, L' ');
	if (n > 1)
		MultiByteToWideChar(codepage, 0, s.c_str(), -1, &w[0], n);
	return w;
}

static std::string FromWide(const std::wstring& w, UINT codepage)
{
	int n = WideCharToMultiByte(codepage, 0, w.c_str(), -1, 0, 0, 0, 0);
	std::string s(n > 0 ? n - 1 : 0, ' ');
	if (n > 1)
		WideCharToMultiByte(codepage, 0, w.c_str(), -1, &s[0], n, 0, 0);
	return s;
}

std::wstring Wide(const std::string& s) { return ToWide(s, CP_UTF8); }
std::string Narrow(const std::wstring& w) { return FromWide(w, CP_UTF8); }
std::wstring WideCp(const std::string& s) { return ToWide(s, 949); }
std::string NarrowCp(const std::wstring& w) { return FromWide(w, 949); }

// Text that isn't valid in the source encoding comes back unchanged. Embedded terminators survive.
static std::string Recode(const std::string& s, UINT from, UINT to)
{
	if (s.empty())
		return s;
	int wlen = MultiByteToWideChar(from, 0, s.data(), (int)s.size(), 0, 0);
	if (wlen <= 0)
		return s;
	std::wstring w(wlen, L' ');
	MultiByteToWideChar(from, 0, s.data(), (int)s.size(), &w[0], wlen);
	int len = WideCharToMultiByte(to, 0, w.data(), wlen, 0, 0, 0, 0);
	std::string out(len, ' ');
	WideCharToMultiByte(to, 0, w.data(), wlen, &out[0], len, 0, 0);
	return out;
}

std::string Cp949ToUtf8(const std::string& s) { return Recode(s, 949, CP_UTF8); }
std::string Utf8ToCp949(const std::string& s) { return Recode(s, CP_UTF8, 949); }

std::string LowerCp949(std::string s)
{
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] >= 'A' && s[i] <= 'Z')
			s[i] = (char)(s[i] + 32);
		else if ((unsigned char)s[i] >= 0x80 && i + 1 < s.size())
			++i;
	}
	return s;
}

FILE* OpenUtf8(const char* path, const wchar_t* mode)
{
	std::wstring w = Wide(path);
	FILE* f = 0;
	if (_wfopen_s(&f, w.c_str(), mode))
		return 0;
	return f;
}

bool ReadWholeFile(const std::string& path, std::vector<unsigned char>* out)
{
	FILE* f = OpenUtf8(path.c_str(), L"rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	fseek(f, 0, SEEK_SET);
	out->resize(n);
	bool ok = n == 0 || fread(&(*out)[0], 1, n, f) == (size_t)n;
	fclose(f);
	return ok;
}

bool WriteWholeFile(const std::string& path, const std::vector<unsigned char>& data, std::string* error)
{
	FILE* f = OpenUtf8(path.c_str(), L"wb");
	if (!f)
	{
		*error = "cannot write " + path;
		return false;
	}
	bool ok = data.empty() || fwrite(&data[0], 1, data.size(), f) == data.size();
	fclose(f);
	if (!ok)
		*error = "short write to " + path;
	return ok;
}

std::string PathBaseName(const std::string& path)
{
	size_t p = path.find_last_of("/\\");
	return p == std::string::npos ? path : path.substr(p + 1);
}

std::string PathStem(const std::string& path)
{
	std::string b = PathBaseName(path);
	size_t p = b.rfind('.');
	return p == std::string::npos ? b : b.substr(0, p);
}

std::string PathDir(const std::string& path)
{
	size_t p = path.find_last_of("/\\");
	return p == std::string::npos ? std::string() : path.substr(0, p);
}

std::string PathJoin(const std::string& dir, const std::string& name)
{
	if (dir.empty())
		return name;
	char last = dir[dir.size() - 1];
	return (last == '/' || last == '\\') ? dir + name : dir + "/" + name;
}

bool PathExists(const std::string& path)
{
	FILE* f = OpenUtf8(path.c_str(), L"rb");
	if (!f)
		return false;
	fclose(f);
	return true;
}

// Patched files often unpacked with their CP949 names read as Latin-1, so "기본발" sits
// on disk as "±âº»¹ß". Scripts name files in CP949: index by what the bytes really are.
static std::string AssetKey(const wchar_t* fileName)
{
	bool latin = true, ascii = true;
	for (const wchar_t* c = fileName; *c; ++c)
	{
		if (*c >= 0x80)
			ascii = false;
		if (*c >= 0x3000)
			latin = false;
	}
	UINT codepage = !ascii && latin ? 1252 : 949;
	char buf[1024];
	int n = WideCharToMultiByte(codepage, 0, fileName, -1, buf, sizeof(buf), 0, 0);
	if (n <= 0)
		return std::string();
	return LowerCp949(std::string(buf, n - 1));
}

static void WalkAssets(const std::wstring& dir, const wchar_t* const* extensions, size_t count,
	std::map<std::string, std::string>* index)
{
	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L".."))
			continue;
		std::wstring path = dir + L"\\" + fd.cFileName;
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			WalkAssets(path, extensions, count, index);
			continue;
		}
		const wchar_t* dot = wcsrchr(fd.cFileName, L'.');
		bool wanted = false;
		for (size_t i = 0; dot && i < count; ++i)
			if (!_wcsicmp(dot, extensions[i]))
				wanted = true;
		if (!wanted)
			continue;
		std::string key = AssetKey(fd.cFileName);
		if (!key.empty() && index->find(key) == index->end())
			(*index)[key] = Narrow(path);
	} while (FindNextFileW(h, &fd));
	FindClose(h);
}

void IndexAssets(const std::string& root, const wchar_t* const* extensions, size_t count,
	std::map<std::string, std::string>* index)
{
	if (!root.empty())
		WalkAssets(Wide(root), extensions, count, index);
}
