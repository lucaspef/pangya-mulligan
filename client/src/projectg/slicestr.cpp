#include "minatl.h"
#include "slicestr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char s_delimiter[] = "*";

bool WSliceStr::Get(const char* key, const char* format, ...)
{
	if (m_str == NULL)
		return false;

	va_list ap;
	char buf[256];
	float fValue;
	int i;
	const char* p;
	char c;
	int width;

	p = strstr(m_str, key);

	if (p != NULL)
	{
		p += strlen(key);

		va_start(ap, format);

		while (*format)
		{
			if (*format++ == '%')
			{
				while (*p == ' ' || *p == '\t')
					p++;

				// Fim do texto: p fica no terminador, os campos que faltam saem vazios
				for (i = 0; (unsigned char)(c = *p++) > ' ';)
				{
					if (strchr(s_delimiter, c))
						break;
					if (i < (int)sizeof(buf) - 1)
						buf[i++] = c;
				}
				buf[i] = '\0';
				if (c == '\0')
					p--;

				// "%63s" limita o texto ao tamanho do destino
				for (width = 0; *format >= '0' && *format <= '9'; format++)
					width = width * 10 + *format - '0';
				if (width > 0 && width < i)
					buf[width] = '\0';

				switch (*format++)
				{
				case 'f':
				case 'F':
				case 'g':
				case 'G':
					fValue = (float)atof(buf);
					memcpy(va_arg(ap, float*), &fValue, sizeof(fValue));
					break;
				case 'd':
				case 'D':
				case 'u':
				case 'U':
					*va_arg(ap, int*) = atoi(buf);
					break;
				case 'x':
				case 'X':
					sscanf(buf, "%x", va_arg(ap, int*));
					break;
				case 's':
				case 'S':
					strcpy(va_arg(ap, char*), buf);
					break;
				}
			}
		}

		va_end(ap);
		return true;
	}

	return false;
}
