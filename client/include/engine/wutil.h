#pragma once
#include "wminmax.h"

template <class T>
inline T Abs(T value)
{
	T result;
	if (value > 0)
		result = value;
	else
		result = -value;
	return result;
}

template <class T>
inline T Between(T minimum, T value, T maximum)
{
	return value < minimum ? minimum : value > maximum ? maximum : value;
}

template <class T>
inline void Swap(T& left, T& right)
{
	T temp = left;
	left = right;
	right = temp;
}

inline ulong __fastcall GetHashCode(const char* text)
{
	ulong code = 0x1505;
	int letter;
	while ((letter = *text++) != 0)
		code = code * 33 + letter;
	return code;
}

inline ulong __fastcall Modulate(ulong left, ulong right)
{
	ulong red = ((left >> 16 & 0xff) * (right >> 16 & 0xff)) >> 8;
	ulong green = ((left >> 8 & 0xff) * (right >> 8 & 0xff)) >> 8;
	ulong blue = ((left & 0xff) * (right & 0xff)) >> 8;
	return (red << 16) + (green << 8) + blue;
}

inline ulong __fastcall AddDiffuse(ulong left, ulong right)
{
	ulong blue = Min<ulong>(0xff, (left & 0xff) + (right & 0xff));
	ulong green = Min<ulong>(0xff, (left >> 8 & 0xff) + (right >> 8 & 0xff));
	ulong red = Min<ulong>(0xff, (left >> 16 & 0xff) + (right >> 16 & 0xff));
	return (red << 16) + (green << 8) + blue;
}

ulong __fastcall AddDiffuse(ulong left, ulong right, unsigned char value);

// JP 983: primeiro byte de Shift-JIS (0x80..0x9f e 0xe0..0xfc; ex.: GetStep 00bf7ae0, UpperCase 0098bc60). O 645 (coreano)
// testava só o bit 7, o que tomaria o katakana de meia largura (0xa1..0xdf) por início de caractere duplo.
inline bool Custom_IsLeadByte(unsigned char letter)
{
	return (letter >= 0x80 && letter < 0xa0) || (unsigned char)(letter + 0x20) <= 0x1c;
}

// Texto do jogo é sempre Shift-JIS; o 983 usa CP_ACP, que só acerta em Windows japonês.
enum { GAME_CODEPAGE = 932 };
