// JP 983: unidade 0053f3a0–005420f0 (GameStringManager). As três funções antes dela (0053ef40, 0053f010, 0053f0d0 — lista
// "oneshot_hio1_1…6", chamadas da sala de jogo) são de outra unidade. O grosso da faixa são instâncias do std::map<int,
// std::string>, do std::vector<std::string> e do std::string.
#include "minatl.h"
#include <wininet.h>
#include <stdlib.h>
#include "gamestringmanager.h"
#include "s5/utilities.h"

// 0053f3a0
int __fastcall HttpStatusCode(HINTERNET hRequest)
{
	if (!hRequest)
		return 0;

	int result = 0;
	char* pBuf = new char[0x100];
	DWORD len = 0xff;
	DWORD index = 0;
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE, pBuf, &len, &index))
	{
		pBuf[len] = 0;
		result = atoi(pBuf);
	}
	if (pBuf)
		delete[] pBuf;
	return result;
}

// 0053f410
int __fastcall HttpContentLength(HINTERNET hRequest)
{
	if (!hRequest)
		return 0;

	int result = 0;
	char* pBuf = new char[0x100];
	DWORD len = 0xff;
	DWORD index = 0;
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH, pBuf, &len, &index))
	{
		pBuf[len] = 0;
		result = atoi(pBuf);
	}
	if (pBuf)
		delete[] pBuf;
	return result;
}

// 0053f480 (ninguém chama)
int __fastcall HttpReferer(HINTERNET hRequest, char* pOut)
{
	if (!hRequest)
		return 0;

	char* pBuf = new char[0x100];
	DWORD len = 0xff;
	DWORD index = 0;
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_REFERER, pBuf, &len, &index))
	{
		pBuf[len] = 0;
		if (pOut)
			strcpy(pOut, pBuf);
		if (pBuf)
			delete[] pBuf;
		return len;
	}
	if (pBuf)
		delete[] pBuf;
	*pOut = 0;
	return 0;
}

// 0053f9c0: baixa a página inteira; quem chama apaga o que volta. Sem "Content-Length" o 983 lê num bloco de 0x400 bytes
// sem conferir o tamanho e o heap estoura (chunked, HTTP/1.0: D850); aqui o bloco cresce.
char* __fastcall HttpGet(const char* url, int* pSize)
{
	if (pSize)
		*pSize = 0;

	HINTERNET hNet = InternetOpenA("Mozila/4.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hNet)
		return NULL;

	HINTERNET hUrl = InternetOpenUrlA(hNet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hUrl)
	{
		Sleep(1000);
		hUrl = InternetOpenUrlA(hNet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
		if (!hUrl)
		{
			InternetCloseHandle(hNet);
			return NULL;
		}
	}
	int status = HttpStatusCode(hUrl);
	if (status != 200)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hNet);
		return NULL;
	}

	int capacity = HttpContentLength(hUrl);
	if (capacity <= 0 || capacity > 0x1000000)
		capacity = 0x400;
	char* pBuf = new char[capacity];

	int total = 0;
	DWORD avail = 0, read = 0;
	while (InternetQueryDataAvailable(hUrl, &avail, 0, 0) && avail)
	{
		if (total + (int)avail > capacity)
		{
			capacity = max(capacity * 2, total + (int)avail);
			char* pNew = new char[capacity];
			memcpy(pNew, pBuf, total);
			delete[] pBuf;
			pBuf = pNew;
		}
		if (!InternetReadFile(hUrl, pBuf + total, avail, &read) || !read)
			break;
		total += read;
	}
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hNet);

	if (pSize)
		*pSize = total;
	return pBuf;
}

// 0053fea0: o mesmo, com o zero no fim
char* __fastcall HttpGetString(const char* url)
{
	int size;
	char* pData = HttpGet(url, &size);
	if (!pData)
		return NULL;

	char* pText = new char[size + 1];
	memcpy(pText, pData, size);
	pText[size] = 0;
	delete[] pData;
	return pText;
}

bool GameStringManager::Convert(const std::string& in, std::string* pOut, unsigned int codePage)
{
	if (in.size() == 0)
		return true;

	pOut->clear();
	std::string src(in.c_str());
	int len = src.size() + 1;
	if (src.size() <= 0x104)
		len = 0x104;
	char* pMulti = new char[len];
	wchar_t* pWide = new wchar_t[len];
	memset(pMulti, 0, len);
	memset(pWide, 0, len * 2);
	MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, pWide, len);
	WideCharToMultiByte(codePage, 0, pWide, -1, pMulti, len, NULL, NULL);
	*pOut = pMulti;
	if (pMulti)
		delete[] pMulti;
	if (pWide)
		delete[] pWide;

	{
		// "\n" escrito com barra vira quebra de linha
		size_t pos = 0;
		std::string from("\\n");
		std::string to("\n");
		while ((pos = pOut->find(from, pos)) != std::string::npos)
			pOut->replace(pos, from.size(), to);
	}
	size_t pos = 0;
	do
	{
		pos = pOut->find('\r', pos);
		if (pos == std::string::npos)
			break;
		pOut->replace(pos, 1, "");
	} while (++pos != std::string::npos);
	return pOut->size() != 0;
}

const char* GameStringManager::GetString(int key)
{
	std::map<int, std::string>::iterator it = m_strings.find(key);
	if (it != m_strings.end())
		return it->second.c_str();
	return NULL;
}

const char* __fastcall GetGameString(int key)
{
	if (key != 0 && GameStringManager::Instance() != NULL)
	{
		const char* pText = GameStringManager::Instance()->GetString(key);
		if (pText == NULL)
			return "";
		return pText;
	}
	return NULL;
}

void GameStringManager::Clear()
{
	m_bLoaded = false;
	m_strings.clear();
}

GameStringManager::~GameStringManager()
{
	Clear();
}

GameStringManager::GameStringManager()
{
	Clear();
}

bool GameStringManager::Extract(const std::string& src, const std::string& open, const std::string& close,
	std::vector<std::string>* pOut)
{
	if (src.size() == 0 || pOut == NULL)
		return false;

	size_t begin = src.find(open);
	size_t end = src.find(close);
	while (begin != std::string::npos && end != std::string::npos)
	{
		std::string part = src.substr(begin + open.size(), end - (begin + open.size()));
		pOut->push_back(std::string(part.c_str()));
		begin = src.find(open, end);
		end = src.find(close, begin);
	}
	return true;
}

bool GameStringManager::Parse(const char* text)
{
	if (text == NULL)
		return false;

	std::string unused;
	std::string src = text;
	std::vector<std::string> texts;
	bool bFlag = Extract(src, std::string("<TEXT>"), std::string("</TEXT>"), &texts);
	if (bFlag)
	{
		for (unsigned int i = 0; i < texts.size(); i++)
		{
			std::vector<std::string> fields;
			bFlag = !Extract(texts[i], std::string("<KEY>"), std::string("</KEY>"), &fields);
			if (!bFlag)
			{
				bFlag = !Extract(texts[i], std::string("<SERVICE>"), std::string("</SERVICE>"), &fields);
				if (!bFlag && fields.size() >= 2) // 983 lê fields[1] sem conferir: <TEXT> sem <SERVICE> caía
				{
					int key = atoi(fields[0].c_str());
					if (key != 0)
					{
						std::string value;
						Convert(std::string(fields[1].c_str()), &value, 932);
						if (m_strings.find(key) != m_strings.end())
							return false;
						m_strings[key] = value;
					}
				}
			}
		}
	}
	return true;
}

bool GameStringManager::Download()
{
	char* pText = HttpGetString(m_url.c_str());
	if (pText)
	{
		unsigned char* pDecoded = new unsigned char[strlen(pText) + 1];
		pDecoded[0] = 0; // Base64Decode não termina a saída vazia
		S5::UTIL::Base64Decode(pText, pDecoded, strlen(pText));
		m_bLoaded = Parse((const char*)pDecoded);
		if (pDecoded)
			delete[] pDecoded;
		delete[] pText;
	}
#ifdef JP983_SERVER_CONFIG
	// E488: servidor sem a tabela, com ela parcial ou com texto em branco não para o jogo em "string load failed."
	static const struct { int key; const char* text; } s_defaults[] = {
#include "gamestrings_default.inc"
	};
	for (int i = 0; i < _countof(s_defaults); i++)
	{
		std::string& value = m_strings[s_defaults[i].key];
		if (value.empty())
			Convert(std::string(s_defaults[i].text), &value, 932);
	}
	m_bLoaded = true;
#endif
	return m_bLoaded;
}

bool GameStringManager::Load(const std::string& url)
{
	Clear();
	if (url.size() == 0)
	{
#ifdef JP983_SERVER_CONFIG
		// A214: sem rugburn nada desvia o 157.7.172.65:50007 (rugburn: -> localhost:30091, gglog a212-1) e o Ready
		// para em "string load failed." (a213-1/002): JP983_WEB=http://host:porta, senao o web 30091 do robo
		// C642: "localhost" como o rugburn (gglog vs2022: 127.0.0.1:30091 = string load failed; localhost:30091 carrega)
		char web[128];
		m_url = GetEnvironmentVariableA("JP983_WEB", web, sizeof(web)) - 1u < sizeof(web) - 1 ? web : "http://localhost:30091";
		m_url += "/Translation/Read.aspx";
#else
		m_url = "http://157.7.172.65:50007/Translation/Read.aspx";
#endif
		return Download();
	}
	m_url = url;
	return Download();
}
