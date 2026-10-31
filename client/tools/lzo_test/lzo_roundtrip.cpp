// Teste de ida e volta do LZO1X próprio (src/shared/lzo1x.cpp) e leitura de vetores no formato padrão.
// Compilar em client\tools\lzo_test:
//   cl /nologo /EHsc /O2 /I..\..\src\shared lzo_roundtrip.cpp ..\..\src\shared\lzo1x.cpp
#include "lzo1x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

static int g_fail = 0;

static void RoundTrip(const char* name, const std::vector<unsigned char>& data)
{
	std::vector<unsigned char> packed(Lzo1xCompressBound(data.size())), back(data.size() + 16);
	size_t packedSize = 0, backSize = 0;
	const int a = Lzo1xCompress(data.data(), data.size(), packed.data(), packed.size(), &packedSize);
	const int b = a ? a : Lzo1xDecompress(packed.data(), packedSize, back.data(), back.size(), &backSize);
	const bool same = data.empty() || memcmp(back.data(), data.data(), data.size()) == 0;
	if (a != 0 || b != 0 || backSize != data.size() || !same)
	{
		printf("FALHOU %s (%d, %d, %zu -> %zu -> %zu)\n", name, a, b, data.size(), packedSize, backSize);
		++g_fail;
	}
}

// Fluxos com instruções que o compressor próprio não emite (match de 2 e de 3 bytes por estado) e cópia sobreposta.
struct Vector
{
	const char* name;
	std::vector<unsigned char> stream;
	const char* expected;
};

int main()
{
	std::vector<unsigned char> data;
	RoundTrip("vazio", data);
	unsigned seed = 12345;
	const auto next = [&seed] { return (seed = seed * 1103515245u + 12345u) >> 16; };
	for (size_t size = 1; size <= 70000; size += size < 600 ? 1 : 997)
	{
		for (int kind = 0; kind < 4; ++kind)
		{
			data.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				switch (kind)
				{
				case 0: data[i] = 0; break;
				case 1: data[i] = (unsigned char)next(); break;
				case 2: data[i] = (unsigned char)("mulligan"[i % 8]); break;
				default: data[i] = i > 20000 && next() % 8 ? data[i - 20000] : (unsigned char)(next() % 7); break;
				}
			}
			RoundTrip("corpus", data);
		}
	}
	data.assign(300000, 0);
	for (size_t i = 40000; i < data.size(); ++i)
		data[i] = i % 7 ? data[i - 40000] : (unsigned char)next();
	RoundTrip("distancia longa", data);

	const Vector vectors[] = {
		{ "literais curtos", { 0x15, 'a', 'b', 'c', 'd', 0x11, 0, 0 }, "abcd" },
		{ "sobreposicao", { 0x12, 'a', 0x20 | 7, 0, 0, 0x11, 0, 0 }, "aaaaaaaaaa" },
		{ "match de 2 bytes", { 0x13, 'a', 'b', 0x04, 0x00, 0x11, 0, 0 }, "abab" },
	};
	for (const Vector& v : vectors)
	{
		char out[64] = { 0 };
		size_t size = 0;
		const int r = Lzo1xDecompress(v.stream.data(), v.stream.size(), (unsigned char*)out, sizeof(out) - 1, &size);
		if (r != LZO1X_OK || strcmp(out, v.expected) != 0)
		{
			printf("FALHOU vetor %s: %d '%s'\n", v.name, r, out);
			++g_fail;
		}
	}

	const unsigned char truncated[] = { 0x15, 'a', 'b' };
	const unsigned char badMatch[] = { 0x12, 'a', 0x20 | 7, 0xFC, 0x00, 0x11, 0, 0 };
	size_t size;
	unsigned char out[32];
	if (Lzo1xDecompress(truncated, sizeof(truncated), out, sizeof(out), &size) != LZO1X_ERROR_INPUT
		|| Lzo1xDecompress(badMatch, sizeof(badMatch), out, sizeof(out), &size) != LZO1X_ERROR_MATCH)
	{
		puts("FALHOU entradas invalidas");
		++g_fail;
	}

	puts(g_fail ? "LZO: FALHOU" : "LZO: OK");
	return g_fail != 0;
}
