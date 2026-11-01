// Simbolos do projectg que o engine cita; o resto fica sem resolver no link (/FORCE) porque o harness nao os chama.
#include <windows.h>
typedef unsigned long ulong;
template <class T> static T Min(T a, T b) { return a < b ? a : b; }
ulong __fastcall AddDiffuse(ulong left, ulong right, unsigned char value)
{
	ulong blue =
		Min<ulong>(0xff, (left & 0xff) + (((right & 0xff) * value) >> 8));
	ulong green = Min<ulong>(0xff,
		(left >> 8 & 0xff) + (((right >> 8 & 0xff) * value) >> 8));
	ulong red = Min<ulong>(0xff,
		(left >> 16 & 0xff) + (((right >> 16 & 0xff) * value) >> 8));
	return (red << 16) | (green << 8) | blue;
}
