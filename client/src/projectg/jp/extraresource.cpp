// JP 983: unidade 004dca40–004e11b0 (CExtraResource). Das ~190 funções da faixa, só as daqui são próprias; o resto são
// instâncias do ifstream/filebuf, do std::map<unsigned, std::string>, do unordered_set<std::string>, do vector e do string.
#include "minatl.h"
#include <direct.h>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "extraresource.h"
#include "tinyxml.h"
#include "shareddoc.h"

const char* g_szExtraName = "extracontents"; // 00e06f38
const char* g_szExtraDir = "extrares\\"; // 00e06f3c
const char* g_szExtraXml = "extracontents.xml"; // 00e06f40
const char* g_szExtraTheme = "pangya_default"; // 00e06f44

// 004df070
std::string __fastcall GetExtraUrl()
{
	std::string url = "http://dl.pangya.jp/pangya/season4/patch/";
	if (url == "")
		url = "http://192.168.81.237:8888/client/updatetest/";
	return url + g_szExtraName + "/";
}

static const char* s_szRoot = g_szExtraName; // 00e74d94 (inicializador 00cca530)
static std::string s_extraUrl = GetExtraUrl(); // 00e74d98 (inicializador 00cca540)

// 004dd060
bool CExtraWallpaper::IsEmpty() const
{
	return m_files.size() == 0;
}

// 004dd7b0
const std::string& CExtraWallpaper::Pick(int mode)
{
	std::map<unsigned int, std::string>::iterator it;
	switch (mode)
	{
	case 0:
		it = m_files.lower_bound(m_cur);
		break;
	case 1:
		++m_cur;
		it = m_files.lower_bound(m_cur);
		break;
	case 2:
		{
			unsigned int value = rand() % m_total;
			it = m_files.lower_bound(value);
		}
		break;
	default:
		it = m_files.end();
		break;
	}
	if (it == m_files.end())
		it = m_files.begin();
	m_cur = it->first;
	return it->second;
}

// 004ddbb0
const char* CExtraResource::GetLoading(int mode)
{
	if (m_loading.IsEmpty())
		return "";
	return m_loading.Pick(mode).c_str();
}

// 004ddbe0
const char* CExtraResource::GetLobby(int mode)
{
	if (m_lobby.IsEmpty())
		return "";
	return m_lobby.Pick(mode).c_str();
}

// 004ddc10 (ninguém chama)
std::string __fastcall GetExtraRandomQuery()
{
	char buf[128];
	sprintf(buf, "?ID=%d", rand());
	return buf;
}

// 004de830
bool CExtraResource::Download(const std::string& file, const std::string& url)
{
	m_xml = HttpDownloadText983(file.c_str(), url.c_str());
	return m_xml.size() != 0;
}

// 004df170
std::string CExtraResource::GetFile(TiXmlElement* pFile, bool bExt)
{
	if (!pFile)
		return "";

	std::string name = pFile->Attribute("name");
	if (name == "")
		return "";

	std::string version = pFile->Attribute("version");
	std::string url = pFile->Attribute("url");
	if (url == "")
		url = m_themeUrl;
	if (*(url.end() - 1) != '/')
		url.append(1, '/');

	std::string remote = name;
	if (version.size() > 0)
		remote += "." + version;

	std::string path = m_dir + remote;
	if (HttpDownloadFile983(url + remote, path))
	{
		if (bExt)
			name = "!e!x!t!" + name;
		((WResourceManagerJp*)g_resrcmng)->AddExtraFile983(name.c_str(), path.c_str());
	}
	else if (_stricmp(g_resrcmng->FindMatchFile(name.c_str()), name.c_str()) == 0)
	{
		// não baixou e o gerente não tem outro com esse nome: só serve se o arquivo solto existe
		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA(name.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE)
			return std::string("");
		FindClose(hFind);
	}
	return name;
}

// 00b59fe0 (WResourceManagerJp = emprestimo do WResourceManager): registra nome minusculo -> caminho em disco na lista +0x1e4.
// Um bloco malloc: "caminho\0nome\0"; a lista guarda o ponteiro do bloco e a chave (nome) aponta para dentro dele.
void WResourceManagerJp::AddExtraFile983(const char* name, const char* path)
{
	WResourceManager* self = (WResourceManager*)this;
	char lower[0x100];
	memset(lower, 0, 0xff);
	lower[0] = 0;
	const char* key = name;
	if (self->m_matchDirectory)
	{
		self->StrcpyLower(lower, name);
		key = lower;
	}
	size_t pathLen = strlen(path);
	if (pathLen == 0)
		return;
	char* block = (char*)malloc(strlen(key) + pathLen + 2);
	strcpy(block, path);
	strcpy(block + pathLen + 1, key);
	self->m_list1e4_983.AddItem(block, block + pathLen + 1, false);
}

// 004df570
void CExtraResource::LoadReplacement(TiXmlElement* pElem)
{
	if (!pElem)
		return;

	for (TiXmlElement* pFile = pElem->FirstChildElement("file"); pFile; pFile = pFile->NextSiblingElement("file"))
		GetFile(pFile, false);
}

// 004dfd40
std::string CExtraResource::GetThemeName()
{
	std::ifstream file;
	file.open("ex_theme.txt");
	if (file.is_open())
	{
		std::string line;
		while (!file.eof())
		{
			std::getline(file, line);
			line = line.erase(line.find_last_not_of(" \t\n") + 1);
			line.erase(0, line.find_first_not_of(" \t\n"));
			if (line.size() != 0 && line[0] != ';')
				return line;
		}
	}
	return g_szExtraTheme;
}

// 004e0070
bool CExtraResource::LoadTheme()
{
	if (!Download(g_szExtraXml, s_extraUrl))
		return false;

	TiXmlDocument doc;
	if (doc.Parse(m_xml.c_str()))
	{
		TiXmlElement* pRoot = doc.FirstChildElement(s_szRoot);
		if (pRoot)
		{
			TiXmlElement* pThemes = pRoot->FirstChildElement("themes");
			if (pThemes)
			{
				TiXmlElement* pTheme = pThemes->FirstChildElement(GetThemeName().c_str());
				if (!pTheme)
					pTheme = pThemes->FirstChildElement();
				if (pTheme)
				{
					m_themeSrc = pTheme->Attribute("src");
					m_themeUrl = pTheme->Attribute("url");
					if (m_themeUrl == "")
						m_themeUrl = s_extraUrl;
					if (*(m_themeUrl.end() - 1) != '/')
						m_themeUrl.append(1, '/');
					return true;
				}
			}
		}
	}
	return false;
}

// 004e02b0
bool CExtraResource::LoadXml()
{
	if (!LoadTheme())
		return false;
	return Download(m_themeSrc, m_themeUrl);
}

// 004e0860
void CExtraWallpaper::Add(const std::string& file, int prob)
{
	if (prob == 0)
		return;

	m_total += prob;
	m_files.insert(std::pair<unsigned int, std::string>(m_total - 1, file));
}

// 004e09f0
CExtraResource::~CExtraResource()
{
	if (CIconManager::Instance())
		delete CIconManager::Instance();
}

// 004e0ba0
void CExtraResource::LoadLoading(TiXmlElement* pElem)
{
	if (!pElem)
		return;

	for (TiXmlElement* pPaper = pElem->FirstChildElement("wallpaper"); pPaper; pPaper = pPaper->NextSiblingElement("wallpaper"))
	{
		int prob; // o 983 não zera: sem o atributo fica o que estava na pilha
		pPaper->Attribute("prob", &prob);
		TiXmlElement* pFile = pPaper->FirstChildElement("file");
		if (pFile)
		{
			std::string file = GetFile(pFile, true);
			if (file.size() > 0)
				m_loading.Add(file, prob);
		}
	}
}

// 004e0ca0
void CExtraResource::LoadLobby(TiXmlElement* pElem)
{
	if (!pElem)
		return;

	for (TiXmlElement* pPaper = pElem->FirstChildElement("wallpaper"); pPaper; pPaper = pPaper->NextSiblingElement("wallpaper"))
	{
		int prob;
		pPaper->Attribute("prob", &prob);
		TiXmlElement* pFile = pPaper->FirstChildElement("file");
		if (pFile)
		{
			std::string file = GetFile(pFile, true);
			if (file.size() > 0)
				m_lobby.Add(file, prob);
		}
	}
}

// 004e0da0
CExtraWallpaper::CExtraWallpaper()
{
	m_total = 0;
	m_cur = 0;
	srand((unsigned int)time(NULL));
}

// 004e0e30
CExtraResource::CExtraResource()
	: m_dir(g_szExtraDir)
{
	_mkdir(m_dir.c_str());
	new CIconManager;
}

// 004e0f90
void CExtraResource::LoadNotice(TiXmlElement* pElem)
{
	if (!pElem)
		return;

	for (TiXmlElement* pFile = pElem->FirstChildElement("file"); pFile; pFile = pFile->NextSiblingElement("file"))
	{
		std::string file = GetFile(pFile, true);
		if (file.size() > 0)
			m_notices.push_back(file);
	}
}

// 004dc530 (fica antes das instâncias de iostream da unidade)
void CExtraResource::LoadTopIcons(TiXmlElement* pElem)
{
	CIconManager::Instance()->LoadExtra983(pElem);
}

// 004e1060
void CExtraResource::Load()
{
	if (!LoadXml())
		return;

	TiXmlDocument doc;
	if (doc.Parse(m_xml.c_str()))
	{
		TiXmlElement* pRoot = doc.FirstChildElement(s_szRoot);
		if (pRoot)
		{
			LoadReplacement(pRoot->FirstChildElement("replacement"));
			LoadTopIcons(pRoot->FirstChildElement("topicons"));
			if (Doc()->m_bChannelLogin)
				LoadNotice(pRoot->FirstChildElement("weblogin_notice"));
			else
				LoadNotice(pRoot->FirstChildElement("notice"));
			LoadLoading(pRoot->FirstChildElement("loading"));
			LoadLobby(pRoot->FirstChildElement("lobby"));
		}
	}
}
