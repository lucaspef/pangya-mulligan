#include "minatl.h"
#include "rssmanager.h"
#include <process.h>
#include <WinInet.h>

bool LoadXMLVer(const char* filename, char* ver, int len)
{
	TiXmlDocument doc;
	if (doc.LoadFile(filename))
	{
		TiXmlNode* rss = doc.FirstChild("rss");

		if (rss == NULL)
			return false;

		TiXmlNode* node = rss->FirstChild("ver");

		if (node == NULL)
			return false;

		node = node->FirstChild();

		if (node == NULL)
			return false;

		const char* value = node->Value();

		strncpy(ver, value, len - 1); // versao vem do XML do servidor; o buffer tem len
		ver[len - 1] = 0;

		return true;
	}

	return false;
}

bool GetUrlFile(const char* url, const char* filename)
{
	char buffer[10240];
	HINTERNET hSession = InternetOpenA("Daemon", 0, NULL, NULL, 0);
	if (hSession == NULL)
		return false;

	HINTERNET hUrl =
		InternetOpenUrlA(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
	if (hUrl == NULL)
	{
		InternetCloseHandle(hSession);
		return false;
	}

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(hSession);
		InternetCloseHandle(hUrl);
		return false;
	}

	unsigned long dwSize = 0;
	unsigned long dwRead = 0;
	unsigned long dwWritten = 0;
	do
	{
		dwRead = 0;
		if (!InternetQueryDataAvailable(hUrl, &dwSize, 0, 0))
			break;
		if (dwSize > sizeof(buffer)) // resposta grande: buffer[10240] na pilha
			dwSize = sizeof(buffer);
		if (!InternetReadFile(hUrl, buffer, dwSize, &dwRead)) // conexao caiu: nao repete para sempre
			break;
		WriteFile(hFile, buffer, dwRead, &dwWritten, NULL);
	} while (dwRead != 0);

	CloseHandle(hFile);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hUrl);
	return true;
}

unsigned int __stdcall RSSThreadFunc(void* pParam)
{
	RSSManager* pManager = (RSSManager*)pParam;
	char szUrl[256] = { 0 };
	// 983 009ac3c0: m_address/m_path de GetRSSFile_JP sao ID e KID; o 645 pedia NoticeRSS.aspx?gIdx= no site QA
	sprintf(szUrl, "%s/rss.aspx?ID=%s&KID=%s", "https://www5.pangya.jp/on_client/club",
		pManager->m_address.c_str(), pManager->m_path.c_str());
	if (GetUrlFile(szUrl, "guildlast.xml"))
	{
		pManager->CallBackFunction(0);
	}
	else
	{
		pManager->m_bNewRss = false;
		pManager->m_bComplete = false;
	}
	return 0;
}

RSSManager::RSSManager()
{
	m_bNewRss = false;
	m_bComplete = false;
}

RSSManager::~RSSManager()
{
}

void RSSManager::GetRSSFile(unsigned long guildIdx, void (*pfnCallback)(int))
{
	if (guildIdx > 0)
	{
		m_guildIdx = guildIdx;
		m_pfnCallback = pfnCallback;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, RSSThreadFunc, this, 0,
			(unsigned int*)&guildIdx);
		if (hThread != INVALID_HANDLE_VALUE)
			CloseHandle(hThread);
	}
}

void RSSManager::GetRSSFile_JP(unsigned long guildIdx, void (*pfnCallback)(int),
	const char* address, const char* path)
{
	if (guildIdx > 0)
	{
		m_guildIdx = guildIdx;
		m_pfnCallback = pfnCallback;
		m_address = address;
		m_path = path;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, RSSThreadFunc, this, 0,
			(unsigned int*)&guildIdx);
		if (hThread != INVALID_HANDLE_VALUE)
			CloseHandle(hThread);
	}
}

void RSSManager::CallBackFunction(int result)
{
	m_bNewRss = false;
	m_bComplete = false;
	if (result != 0)
		return;

	char szLast[100] = { 0 };
	char szNews[100] = { 0 };
	if (!LoadXMLVer("guildlast.xml", szLast, 100))
		return;

	if (!LoadXMLVer("guildnews.xml", szNews, 100))
	{
		DeleteFile("guildnews.xml");
		rename("guildlast.xml", "guildnews.xml");
	}
	else
	{
		if (stricmp(szLast, szNews) == 0)
		{
			DeleteFile("guildnews.xml");
			rename("guildlast.xml", "guildnews.xml");
			m_bComplete = true;
			m_pfnCallback(4);
			return;
		}
		DeleteFile("guildnews.xml");
		rename("guildlast.xml", "guildnews.xml");
		m_pfnCallback(3);
	}
	m_bComplete = true;
	m_bNewRss = true;
}

bool RSSManager::IsNewRss()
{
	return m_bNewRss;
}

const char* RSSManager::LoadFirstId(const char* filename)
{
	TiXmlDocument doc;
	if (doc.LoadFile(filename))
	{
		TiXmlNode* rss = doc.FirstChild("rss");

		if (rss == NULL)
			return NULL;

		TiXmlNode* node = rss->FirstChild("notice");

		if (node == NULL)
			return NULL;

		node = node->FirstChild();

		if (node == NULL)
			return NULL;

		const char* value = node->Value();

		return value;
	}

	return NULL;
}

void RSSManager::ResetVars()
{
	m_bNewRss = false;
	m_bComplete = false;
}
