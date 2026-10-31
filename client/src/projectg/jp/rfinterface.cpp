// JP 983: RFInterface.cpp da Refresh (B153, F3.7.13): a implementação padrão da interface com o jogo (0042ac40–0042ad80).
// O jogo deriva o RefreshInterface (refreshinterface.cpp) e troca quase tudo; estas são as da base.
#include <windows.h>
#include <stdio.h>
#include <string>
#include "refresh/lib/rfinterface.h"

// 0042ac40: sem pasta base
std::wstring RFInterface::GetBasePath(RFBASEPATHTYPE)
{
	return L"";
}

// 0042ac80
void* RFInterface::FOpen(const std::wstring& path)
{
	FILE* file;
	if (_wfopen_s(&file, path.c_str(), L"rb") != 0)
		return NULL;
	return file;
}

// 0042acb0
void RFInterface::FClose(void* file)
{
	fclose((FILE*)file);
}

// 0042acd0
long RFInterface::FTell(void* file)
{
	return ftell((FILE*)file);
}

// 0042acf0
int RFInterface::FSeek(void* file, long offset, int origin)
{
	return fseek((FILE*)file, offset, origin);
}

// 0042ad10: um bloco de `size` bytes; devolve os bytes lidos
unsigned int RFInterface::FRead(void* file, void* buffer, unsigned int size)
{
	return fread(buffer, size, 1, (FILE*)file) * size;
}

// 0042ad40: sem tradução
std::wstring RFInterface::TranslateText(const std::wstring& text)
{
	return text;
}

// 0042ad80
void RFInterface::DebugPrint(const wchar_t* text)
{
	OutputDebugStringW(text);
}
