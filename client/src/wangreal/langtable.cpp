// Unidade nova do JP 983 (0x10059990..0x1005bf50; não existe no 645 nem no 839): tradução coreano -> japonês das
// mensagens da engine. korea.dat e japan.dat são listas paralelas de strings terminadas em zero; a tabela é um
// stdext::hash_map<std::string, std::string>. Nome do arquivo e da classe são provisórios (sem RTTI/PDB);
// K2L_Compatibility é o nome do 645 (lá era inline e devolvia o próprio texto).
#include <io.h>
#include <new>
#include <stdio.h>
#include <string.h>
#include <string>
#include <hash_map>
#include "wlocalize.h"

class WLangTable
{
public:
	typedef stdext::hash_map<std::string, std::string> Table;
	enum { CACHE_SIZE = 30 };

	// 0x1005be40 (expandido em K2L_Compatibility)
	WLangTable() : m_korea(0), m_japan(0)
	{
		Load();
	}
	// 0x1005baf0 (no corpo da classe: é o que põe a função depois de hash_map() e emplace_front no .obj)
	~WLangTable()
	{
		if (m_korea)
		{
			delete[] m_korea;
			m_korea = 0;
		}
		if (m_japan)
		{
			delete[] m_japan;
			m_japan = 0;
		}
	}

	size_t LoadFile(const char* filename, char** data);
	void Load();
	const char* Find(const char* str);

	char* m_korea;                      // +0x00 korea.dat inteiro
	char* m_japan;                      // +0x04 japan.dat inteiro
	const char* m_cacheKey[CACHE_SIZE]; // +0x08 só lido no 983 (ninguém grava)
	const char* m_cacheValue[CACHE_SIZE]; // +0x80
	int m_cachePos;                     // +0xf8
	Table m_table;                      // +0xfc
};

// 0x1006e558: pares de korea.dat repetidos (só contado)
int g_langDuplicated;

// 0x10059990
size_t WLangTable::LoadFile(const char* filename, char** data)
{
	FILE* file = fopen(filename, "rb");
	size_t size = 0;
	if (file)
	{
		size = _filelength(_fileno(file));
		*data = new char[size + 1];
		(*data)[size] = 0; // ultima string sem zero no fim
		fread(*data, size, 1, file);
		fclose(file);
	}
	return size;
}

// 0x100599f0 (sem chamador na dll): copia um caractere Shift-JIS (1 ou 2 bytes) de src para dst; devolve o tamanho (0 = fim)
int GetSjisChar(const unsigned char* src, unsigned char* dst)
{
	unsigned char c = src[0];
	if (c == 0)
		return 0;
	if ((c >= 0x80 && c < 0xa0) || (c >= 0xe0 && c <= 0xfc))
	{
		if (src[1] == 0)
			return 0;
		dst[0] = c;
		dst[1] = src[1];
		dst[2] = 0;
		return 2;
	}
	dst[0] = c;
	dst[2] = 0;
	return 1;
}

// 0x1005af30
const char* WLangTable::Find(const char* str)
{
	int start;
	if (m_cachePos < CACHE_SIZE / 2)
		start = CACHE_SIZE - m_cachePos;
	else
		start = m_cachePos - CACHE_SIZE / 2;
	for (int i = start; i < start + CACHE_SIZE; ++i)
	{
		if (m_cacheKey[i % CACHE_SIZE] && strcmp(m_cacheKey[i % CACHE_SIZE], str) == 0)
			return m_cacheValue[i % CACHE_SIZE];
	}
	const char* result = str;
	Table::iterator it = m_table.find(str);
	if (it != m_table.end())
	{
		const std::string& s = it->second;
		const char* text = s.c_str();
		result = text;
	}
	return result;
}

// A930: proxima conversao de printf do texto ('s', 'd', 'f' ou 'n'); 0 no fim. '%' sem conversao valida nao conta
static char NextConversion(const char*& p)
{
	while (*p)
	{
		if (*p++ != '%')
			continue;
		if (*p == '%')
		{
			++p;
			continue;
		}
		const char* q = p;
		while (*q == '-' || *q == '+' || *q == '#' || *q == '0' || *q == '*' || *q == '.' || (*q >= '0' && *q <= '9'))
			++q;
		if (q[0] == 'I' && q[1] == '6' && q[2] == '4')
			q += 3;
		else if (*q == 'h' || *q == 'l' || *q == 'w')
			++q;
		switch (*q)
		{
		case 's': case 'S':
			p = q + 1;
			return 's';
		case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'c': case 'C':
			p = q + 1;
			return 'd';
		case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
			p = q + 1;
			return 'f';
		case 'n':
			p = q + 1;
			return 'n';
		}
	}
	return 0;
}

// A930: a chave pode ser formato com argumentos no sprintf do chamador. Traducao com %s no lugar de %d, ou com
// conversao a mais, derrubaria; nesse caso fica o texto da chave, que casa com os argumentos.
static bool SameFormat(const char* korea, const char* japan)
{
	for (;;)
	{
		char c = NextConversion(japan);
		if (!c)
			return true; // conversao a menos: argumento sobra, nao derruba
		if (c != NextConversion(korea))
			return false;
	}
}

// 0x1005bbd0
void WLangTable::Load()
{
	memset(m_cacheKey, 0, sizeof(m_cacheKey));
	memset(m_cacheValue, 0, sizeof(m_cacheValue));
	m_cachePos = 0;
	int size = LoadFile("korea.dat", &m_korea);
	int japanSize = LoadFile("japan.dat", &m_japan); // A930: ausente ou mais curto que korea.dat: o que falta fica sem traducao
	int k = 0;
	int j = 0;
	while (k < size && j < japanSize)
	{
		const char* korea = m_korea + k;
		while (m_korea[k++])
			;
		const char* japan = m_japan + j;
		while (m_japan[j++])
			;
		if (m_table.find(korea) == m_table.end())
			m_table.insert(Table::value_type(korea, SameFormat(korea, japan) ? japan : korea));
		else
			++g_langDuplicated;
	}
}

// 0x1005beb0
const char* K2L_Compatibility(const char* str)
{
	static WLangTable table;
	return table.Find(str);
}
