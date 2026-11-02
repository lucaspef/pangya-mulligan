#pragma once
#include <windows.h>
#include <string>
#include "wutil.h"

// No 645 era inline e devolvia o próprio texto; no JP 983 é uma função da unidade lang (src/wangreal/langtable.cpp)
// que procura o texto coreano em korea.dat e devolve o par de japan.dat.
const char* K2L_Compatibility(const char* str);

// Texto do jogo é Shift-JIS; as APIs A convertem pelo CP_ACP e saem ilegíveis fora do Windows japonês.
inline std::wstring GameText(const char* text)
{
	std::wstring wide(MultiByteToWideChar(GAME_CODEPAGE, 0, text, -1, NULL, 0), L'\0');
	if (!wide.empty())
	{
		MultiByteToWideChar(GAME_CODEPAGE, 0, text, -1, &wide[0], (int)wide.size());
		wide.resize(wide.size() - 1);
	}
	return wide;
}

inline int GameMessageBox(HWND hWnd, const char* text, const char* caption, UINT type)
{
	return MessageBoxW(hWnd, GameText(text).c_str(), GameText(caption).c_str(), type);
}

// Janela de classe A: SetWindowTextW voltaria a passar pelo CP_ACP a caminho do wndproc; DefWindowProcW grava direto.
inline void GameSetWindowText(HWND hWnd, const char* text)
{
	DefWindowProcW(hWnd, WM_SETTEXT, 0, (LPARAM)GameText(text).c_str());
}
