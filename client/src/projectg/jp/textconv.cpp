// JP 983: unidade 005e49c0–005e51f0 (conversões de texto: página 932 do japonês ↔ wide ↔ UTF-8). O mapa a chamava "atl"
// (só tinha o nome ATL::AtlThrow); não há classe. Os conversores CW2A/CA2W são os da ATL 10 (include/jp/atlconv983.h).
// Existe no 839 (005e9760…). Nomes das funções são meus (WideToMultiByte983 já era usado pela refreshinterface/refreshrenderer).
// Sessão B123.
#include "minatl.h"
#include "projectg.h"
#include <malloc.h>
#include <string>
#include "atlconv983.h"

// 005e4d60
std::string WideToMultiByte983(const std::wstring& text)
{
	int n = WideCharToMultiByte(932, 0, text.c_str(), -1, NULL, 0, "?", NULL);
	char* buf = (char*)_alloca(n);
	WideCharToMultiByte(932, 0, text.c_str(), -1, buf, n, "?", NULL);
	return std::string(buf);
}

// 005e4e10
std::string Utf8ToMultiByte983(const char* text)
{
	int nw = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t* wbuf = (wchar_t*)_alloca(nw * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, nw);
	int n = WideCharToMultiByte(932, 0, wbuf, -1, NULL, 0, "?", NULL);
	char* buf = (char*)_alloca(n);
	WideCharToMultiByte(932, 0, wbuf, -1, buf, n, "?", NULL);
	return buf;
}

// 005e4ef0
std::wstring MultiByteToWide983(const std::string& text)
{
	int n = MultiByteToWideChar(932, 0, text.c_str(), -1, NULL, 0);
	wchar_t* buf = (wchar_t*)_alloca(n * sizeof(wchar_t));
	MultiByteToWideChar(932, 0, text.c_str(), -1, buf, n);
	return buf;
}

// 005e4fa0
std::wstring Utf8ToWide983(const char* text)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)_alloca(n * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, buf, n);
	return buf;
}

// 005e5040: chamada pelo ProcessReport (00898f80)
std::string MultiByteToUtf8_983(const char* text)
{
	std::string result;
	if (text)
	{
		std::wstring w = atl983::CA2WEX983<>(text, 932);
		result = atl983::CW2AEX983<>(w.c_str(), CP_UTF8);
	}
	return result.c_str();
}
