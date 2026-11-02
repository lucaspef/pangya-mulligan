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
			delete m_korea;
			m_korea = 0;
		}
		if (m_japan)
		{
			delete m_japan;
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
		*data = new char[size];
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

// 0x1005bbd0
void WLangTable::Load()
{
	memset(m_cacheKey, 0, sizeof(m_cacheKey));
	memset(m_cacheValue, 0, sizeof(m_cacheValue));
	m_cachePos = 0;
	int size = LoadFile("korea.dat", &m_korea);
	LoadFile("japan.dat", &m_japan);
	int k = 0;
	int j = 0;
	while (k < size)
	{
		const char* korea = m_korea + k;
		while (m_korea[k++])
			;
		const char* japan = m_japan + j;
		while (m_japan[j++])
			;
		if (m_table.find(korea) == m_table.end())
			m_table.insert(Table::value_type(korea, japan));
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
