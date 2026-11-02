#pragma once

// A930: formato vindo de pak ou servidor. Com %s no lugar de %d o 983 le ponteiro lixo e cai. Confere so o que derruba
// ou le fora dos argumentos: dword a dword como na pilha do ..., sem tocar em %d com tipo errado, que so imprime errado.
enum { FMT_ARG_DWORD, FMT_ARG_QWORD, FMT_ARG_PTR };
// Classe no ... (CString etc.) nao tem como conferir: conta como ponteiro, passa como antes
template <class T> struct FmtArgKind { enum { value = __is_class(T) ? FMT_ARG_PTR : sizeof(T) > 4 ? FMT_ARG_QWORD : FMT_ARG_DWORD }; };
template <class T> struct FmtArgKind<T*> { enum { value = FMT_ARG_PTR }; };
template <> struct FmtArgKind<float> { enum { value = FMT_ARG_QWORD }; }; // promovido a double no ...

inline bool FormatTakeArg(const unsigned char* kinds, int count, int& arg, bool& half, int dwords, bool pointer)
{
	for (; dwords > 0; --dwords)
	{
		if (arg >= count || (pointer && kinds[arg] != FMT_ARG_PTR))
			return false;
		if (kinds[arg] == FMT_ARG_QWORD && !half)
			half = true;
		else
		{
			half = false;
			++arg;
		}
	}
	return true;
}

inline bool FormatMatchesArgs(const char* format, const unsigned char* kinds, int count)
{
	if (!format)
		return false;
	int arg = 0;
	bool half = false;
	for (const char* p = format; *p; ++p)
	{
		if (*p != '%')
			continue;
		++p;
		if (*p == '%')
			continue;
		while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0')
			++p;
		for (int part = 0; part < 2; ++part) // largura e precisao
		{
			if (*p == '*')
			{
				if (!FormatTakeArg(kinds, count, arg, half, 1, false))
					return false;
				++p;
			}
			while (*p >= '0' && *p <= '9')
				++p;
			if (part == 1 || *p != '.')
				break;
			++p;
		}
		int dwords = 1;
		if (p[0] == 'I' && p[1] == '6' && p[2] == '4')
		{
			dwords = 2;
			p += 3;
		}
		else if (p[0] == 'I' && p[1] == '3' && p[2] == '2')
			p += 3;
		else if (p[0] == 'l' && p[1] == 'l')
		{
			dwords = 2;
			p += 2;
		}
		else if (*p == 'h' || *p == 'l' || *p == 'w' || *p == 'L' || *p == 'I')
			++p;
		switch (*p)
		{
		case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'c': case 'C': case 'p':
			if (!FormatTakeArg(kinds, count, arg, half, dwords, false))
				return false;
			break;
		case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
			if (!FormatTakeArg(kinds, count, arg, half, 2, false))
				return false;
			break;
		case 's': case 'S':
			if (!FormatTakeArg(kinds, count, arg, half, 1, true))
				return false;
			break;
		default: // %n, fim do texto ou conversao que a CRT recusa encerrando o processo
			return false;
		}
	}
	return true;
}

#if !defined(_MSC_VER) || _MSC_VER >= 1800
template <class... A> inline bool FormatFits(const char* format, A...)
{
	const unsigned char kinds[] = { (unsigned char)FmtArgKind<A>::value..., 0 };
	return FormatMatchesArgs(format, kinds, (int)sizeof...(A));
}

// Formato que nao casa com os argumentos vira texto vazio
template <class... A> inline const char* CheckFormat(const char* format, A... a)
{
	return FormatFits(format, a...) ? format : "";
}
#else
inline const char* CheckFormat(const char* format, ...) { return format; } // VC10 sem variadic template, so diagnostico
#endif
