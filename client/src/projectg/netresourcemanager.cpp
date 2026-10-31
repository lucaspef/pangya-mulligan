#include "minatl.h"
#include "vc10_layout.h"
#include "uccmanager.h"
#include "netresourcemanager.h"
#include "projectg.h"
#include "commonutil.h"
#include "sea.h"
#include "chttp.h"
#include "jp/inetfile.h"
#include "generichttpclient.h"
#include "packet.h"
#include "taskmanager.h"
#include "actor.h"
#include "guildactor.h"
#include "securitykeymanager.h"
#include "shareddoc.h"
#include "ucclibrary.h"
#include "../shared/localize.h"
#include <process.h>
#include <stdio.h>
#include <string.h>
#include "jp/docofs983.h" // D864: crus do CSharedDoc ancorados no membro (VS 2022)



std::string __fastcall GenerateTextureName_F(unsigned long typeId,
	const char* uccIndex);
std::string __fastcall GenerateTextureFullPath(int side, unsigned long typeId,
	const char* uccIndex);

struct ActorAccess : public IActor
{
	static void Handle(IActor* actor, const MsgObject& msg)
	{
		((ActorAccess*)actor)->HandleMsg(msg);
	}
};

bool __fastcall UnzipUccClothes(unsigned char* data, unsigned long size,
	Bitmap* front, Bitmap* back, Bitmap* icon, sUccHeader* header);
// = ucclibrary.cpp (bool, unsigned int); a declaracao antiga (void, unsigned long) dava outro nome e virava stub no exe
bool __fastcall LoadWarningClothes(unsigned int typeId, Bitmap* front,
	Bitmap* back, Bitmap* icon);
std::string __fastcall GenerateTextureName_B(unsigned long typeId,
	const char* uccIndex);
std::string __fastcall GenerateIconFilename(unsigned long typeId,
	const char* uccIndex);
sItemInfo* __fastcall GetMyItemInfo(unsigned long guid);

void __fastcall G_CreateDirectory(const char* path)
{
	// 983 (009a89a0): cria cada nivel do caminho; `full` vai acumulando os pedacos
	std::string rest = path;
	if (rest.length() != 0)
	{
		std::string part = rest.substr(0, rest.find("\\"));
		rest = rest.substr(rest.find("\\") + 1);
		std::string full;
		for (;;)
		{
			full.append(part);
			full += "\\";
			CreateDirectoryA(full.c_str(), NULL);
			size_t pos = rest.find("\\");
			if (pos == std::string::npos)
			{
				break;
			}
			part = rest.substr(0, pos);
			rest = rest.substr(pos + 1);
		}
		full.append(rest);
		CreateDirectoryA(full.c_str(), NULL);
	}
}

unsigned int __stdcall CreateDownloadThreadFunc(void* param)
{
	NetResourceManager* mgr = (NetResourceManager*)param;
	mgr->WorkDownloadThread();
	return 0;
}

unsigned int __stdcall CreateUploadThreadFunc(void* param)
{
	NetResourceManager* mgr = (NetResourceManager*)param;
	mgr->WorkUploadThread();
	return 0;
}

NetResourceManager::NetResourceManager()
{
	C_ASSERT_VC10(sizeof(NetResourceManager) == 0x32f8); // 983: new de 0x32f8 em 00a69899 (CProjectG)
	Init();
}

NetResourceManager::~NetResourceManager()
{
	Close();
}

void NetResourceManager::Init()
{
	// 983 009abb70: enderecos do JP (sem upload de emblema e sem relatorio de erro)
	strcpy(m_download[RESOURCE_GUILD_EMBLEM].dir, "emblem");
	strcpy(m_download[RESOURCE_GUILD_EMBLEM].url, "http://www5.pangya.jp/upload/club/mark/");
#ifdef JP983_SERVER_CONFIG
	// sem rugburn nada desvia o www5 (rugburn do robo: -> localhost:30080, mesmo caminho, gglog a212-1; client/COMPILAR.md
	// item 5): JP983_WWW=http://host:porta, senao o web 30080 do servidor local. SelfDesign/Report o rugburn nao desvia.
	{
		char www[96];
		if (!GetEnvironmentVariableA("JP983_WWW", www, sizeof(www)))
			strcpy(www, "http://127.0.0.1:30080");
		sprintf(m_download[RESOURCE_GUILD_EMBLEM].url, "%s/upload/club/mark/", www);
	}
#endif
	m_download[RESOURCE_GUILD_EMBLEM].reserved = 32;
	m_download[RESOURCE_UCC_CLOTHES].dir[0] = '\0';
	strcpy(m_download[RESOURCE_UCC_CLOTHES].url, "http://157.7.172.65:50007/_Files/SelfDesign/");
	strcpy(m_uploadAddress[RESOURCE_UCC_CLOTHES], "http://157.7.172.65:50007/SelfDesign/UploadFiles.aspx");
	strcpy(m_download[RESOURCE_UCC_TEMP_CLOTHES].url,
		m_download[RESOURCE_UCC_CLOTHES].url);
	strcpy(m_uploadAddress[RESOURCE_UCC_TEMP_CLOTHES],
		m_uploadAddress[RESOURCE_UCC_CLOTHES]);
	m_downloadList.clear();
	m_bRunning = true;
	m_bClearGarbage = false;
	InitializeCriticalSection(&m_csDownload);
	InitializeCriticalSection(&m_csUpload);
	unsigned int downloadThreadId;
	unsigned int uploadThreadId;
	m_hDownloadThread = (void*)_beginthreadex(NULL, 0, CreateDownloadThreadFunc,
		this, 0, &downloadThreadId);
	m_hUploadThread = (void*)_beginthreadex(NULL, 0, CreateUploadThreadFunc,
		this, 0, &uploadThreadId);
	if (m_hDownloadThread != (void*)-1 && m_hUploadThread != (void*)-1)
	{
		memset(m_rootDir, 0, sizeof(m_rootDir));
		strcpy(m_rootDir, g_executeDirectory);
		strcat(m_rootDir, "\\");
		char path[1024];
		memset(path, 0, sizeof(path));
		sprintf(path, "%s\\%s", m_rootDir,
			m_download[RESOURCE_GUILD_EMBLEM].dir);
		G_CreateDirectory(path);
	}
}

void NetResourceManager::Close()
{
	m_bRunning = false;
	WaitForSingleObject(m_hDownloadThread, 3000);
	CloseHandle(m_hDownloadThread);
	WaitForSingleObject(m_hUploadThread, 3000);
	CloseHandle(m_hUploadThread);
	LPCRITICAL_SECTION csDownload = &m_csDownload;
	EnterCriticalSection(csDownload);
	m_downloadList.clear();
	LeaveCriticalSection(csDownload);
	LPCRITICAL_SECTION csUpload = &m_csUpload;
	EnterCriticalSection(csUpload);
	m_uploadList.clear();
	LeaveCriticalSection(csUpload);
	std::map<std::string, Bitmap*>::iterator it;
	for (it = m_emblem.begin(); it != m_emblem.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	m_emblem.clear();
	DeleteCriticalSection(csDownload);
	DeleteCriticalSection(csUpload);
}

int NetResourceManager::WorkDownloadThread()
{
	while (m_bRunning)
	{
		if (m_bClearGarbage)
		{
			m_bClearGarbage = false;
			ClearGarbage();
		}
		EnterCriticalSection(&m_csDownload);
		if (m_downloadList.empty())
		{
			LeaveCriticalSection(&m_csDownload);
			Sleep(1000);
		}
		else
		{
			sResource res = m_downloadList.front();
			m_downloadList.pop_front();
			LeaveCriticalSection(&m_csDownload);
			bool bHandled = false;
			if (res.type == RESOURCE_GUILD_EMBLEM)
			{
				char path[256];
				sprintf(path, "%s%s\\%s", m_rootDir, m_download[res.type].dir,
					res.filename.c_str());
				WIN32_FIND_DATAA findData;
				HANDLE hFind = FindFirstFileA(path, &findData);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					FindClose(hFind);
					LoadBitmapA(res);
					bHandled = true;
				}
			}
			if (!bHandled)
			{
				// 983 (009ab8c0): tipos 0 e 3 (roupa UCC e temporaria) completam o
				// download na UccManager; tipo 1 (emblema) carrega o bitmap; 2 nada
				if (_RealDownLoad(res))
				{
					switch (res.type)
					{
					case RESOURCE_UCC_CLOTHES:
					case RESOURCE_UCC_TEMP_CLOTHES:
						if (UccManager()->SetDownloadComplete(res.id, res.key))
						{
							UccManager()->AddRefreshItem(res.id, res.key);
						}
						break;
					case RESOURCE_GUILD_EMBLEM:
						LoadBitmapA(res);
						break;
					}
				}
			}
		}
	}
	return 0;
}

// 983 009a92f0 (pedido C34): so UCC (tipos 0 e 3, Self Design) com chave; os outros tipos mandam arg1/arg2.
// Sem o emblema de guilda e sem o relatorio de erro do 645; resposta "0" = sucesso; erro -> mensagem 0x28e ao ator principal.
int NetResourceManager::WorkUploadThread()
{
	while (m_bRunning)
	{
		EnterCriticalSection(&m_csUpload);
		if (m_uploadList.empty())
		{
			LeaveCriticalSection(&m_csUpload);
			Sleep(1000);
			continue;
		}
		sResource res = m_uploadList.front();
		m_uploadList.pop_front();
		LeaveCriticalSection(&m_csUpload);
		char cwd[1024] = { 0 };
		GetCurrentDirectoryA(0x400, cwd);
		std::string filePath = cwd;
		filePath.append("\\", 1);
		filePath += res.filename;
		GenericHTTPClient http;
		http.InitilizePostArguments();
		if (res.type == RESOURCE_UCC_CLOTHES || res.type == RESOURCE_UCC_TEMP_CLOTHES)
		{
			sItemInfo* item = GetMyItemInfo(res.id, res.key, 0);
			if (!item)
			{
				// "falha no upload. tente de novo.(codigo:%d)"
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)MakeStr(K2L_Compatibility("\xBE\xF7\xB7\xCE\xB5\xE5\xB0\xA1 \xBD\xC7\xC6\xD0\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9. \xB4\xD9\xBD\xC3 \xBD\xC3\xB5\xB5\xC7\xD8\xC1\xD6\xBC\xBC\xBF\xE4.(\xC4\xDA\xB5\xE5:%d)"), 6),
					0, 0, 0, 0));
				continue;
			}
			std::string key = SecurityKeyManager()->GetKey((eSKContent)1, item->guid);
			if (key.size() < 0x14)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x23), (int)"Fail : invalid key received", 0, 0, 0, 0));
				continue;
			}
			http.AddPostArguments("selfDesignUID", *(unsigned long*)DOCPTR983(0x8cb));
			http.AddPostArguments("selfDesignItemID", item->guid);
			char folder[4] = { 0 };
			sprintf_s(folder, "%c", res.key[0]);
			http.AddPostArguments("selfDesignFolder", folder, 0);
			http.AddPostArguments("selfDesignKey", key.c_str(), 0);
			http.AddPostArguments("selfDesignFileName", filePath.c_str(), 1);
		}
		else
		{
			http.AddPostArguments("arg1", res.arg, 0);
			http.AddPostArguments("arg2", filePath.c_str(), 1);
		}
		int bOk = http.Request(NetResManager()->GetUploadAddress(res.type), 3, "MERONG(0.9/;p)");
		const char* response = http.QueryHTTPResponse();
		if (bOk && strcmp(response, "0") == 0)
		{
			if (res.type >= 0 && (res.type <= 1 || res.type == RESOURCE_UCC_TEMP_CLOTHES) && SetUploadStatus(res.id, res.key, res.type))
			{
				EnterCriticalSection(&m_csUpload);
				std::list<sUploadStatus>::iterator it;
				for (it = m_uploadStatus.begin(); it != m_uploadStatus.end(); it++)
				{
					if ((*it).type == res.type && (*it).id == res.id && strcmp((*it).key, res.key) == 0)
						break;
				}
				if (it != m_uploadStatus.end())
					m_uploadStatus.erase(it);
				LeaveCriticalSection(&m_csUpload);
				if (res.type == RESOURCE_UCC_CLOTHES)
				{
					WSendPacket packet(0xb9);
					packet.Encode1(0);
					packet.Encode4(res.id);
					packet.EncodeStr(std::string(res.key));
					packet.EncodeStr(std::string(res.arg));
					packet.Send(TO_GAME);
				}
				else if (res.type == RESOURCE_UCC_TEMP_CLOTHES)
				{
					WSendPacket packet(0xb9);
					packet.Encode1(3);
					packet.Encode4(res.id);
					packet.EncodeStr(std::string(res.key));
					packet.Send(TO_GAME);
				}
			}
		}
		else
		{
			int code = atoi(response);
			if (res.type == RESOURCE_UCC_CLOTHES || res.type == RESOURCE_UCC_TEMP_CLOTHES)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x213), (int)&res.id, (int)res.key, code, 0, 0));
				EnterCriticalSection(&m_csUpload);
				std::list<sResource>::iterator it = m_uploadList.begin();
				while (it != m_uploadList.end())
				{
					if (res.id == (*it).id && strcmp(res.key, (*it).key) == 0)
					{
						DeleteFileA((*it).filename.c_str());
						it = m_uploadList.erase(it);
					}
					else
						++it;
				}
				LeaveCriticalSection(&m_csUpload);
			}
		}
		DeleteFileA(res.filename.c_str());
	}
	return 0;
}

bool NetResourceManager::_RealDownLoad(const sResource& res)
{
	// 983 (009a9eb0): sem cHttp; o download e feito por HttpDownloadText983 /
	// HttpDownloadFileTo983 (jp/inetfile). A URL usa so o 1o caractere da chave.
	bool bResult = false;
	switch (res.type)
	{
	case RESOURCE_GHOST:
	{
		std::string url = m_download[RESOURCE_GHOST].url;
		url += res.arg[0];
		url += "/";
		url += res.arg;
		url += "/";
		CreateDirectoryA(m_download[res.type].dir, NULL);
		std::string data = HttpDownloadText983(res.filename.c_str(), url.c_str());
		if (data.size() != 0)
		{
			bResult = true;
		}
		break;
	}
	case RESOURCE_GUILD_EMBLEM:
	{
		char path[256];
		memset(path, 0, sizeof(path));
		sprintf(path, "%s\\%s", m_download[RESOURCE_GUILD_EMBLEM].dir,
			res.filename.c_str());
		bResult = HttpDownloadFileTo983(path, res.filename.c_str(), m_rootDir,
			m_download[res.type].url);
		break;
	}
	case RESOURCE_UCC_CLOTHES:
	{
		std::string url = m_download[RESOURCE_UCC_CLOTHES].url;
		url += res.key[0];
		url += "/";
		std::string data = HttpDownloadText983(res.filename.c_str(), url.c_str());
		long width = 0;
		long height = 0;
		IFF_STRUCT::sPart* part = Doc()->m_itemManager.FindPart(res.id);
		if (part)
		{
			Bitmap* base =
				g_resrcmng->LoadBitmapA((const char*)part + 0xf4, 0, false);
			if (base)
			{
				width = base->bi->bmiHeader.biWidth;
				height = base->bi->bmiHeader.biHeight;
				delete base;
			}
		}
		Bitmap front;
		Bitmap back;
		front.Create(width, height, 24);
		back.Create(width, height, 24);
		Bitmap* icon = new Bitmap(64, 84, 32);
		sUccHeader header;
		memset(&header, 0, sizeof(header));
		if (data.size() == 0 ||
			!UnzipUccClothes((unsigned char*)data.c_str(), data.size(), &front,
				&back, icon, &header))
		{
			delete icon;
			return false;
		}
		if (header.flags & 1)
		{
			LoadWarningClothes(res.id, &front, &back, icon);
		}
		std::string frontName = GenerateTextureName_F(res.id, res.key);
		std::string backName = GenerateTextureName_B(res.id, res.key);
		g_resrcmng->UploadTexture(frontName.c_str(), &front, 0, NULL);
		g_resrcmng->UploadTexture(backName.c_str(), &back, 0, NULL);
		std::string iconName = GenerateIconFilename(res.id, res.key);
		if (UccManager()->m_icon.find(iconName) == UccManager()->m_icon.end())
		{
			sUccItem* item;
			if (res.type == RESOURCE_UCC_CLOTHES)
			{
				item = UccManager()->FindClothes(res.id, res.key);
			}
			else
			{
				item = UccManager()->FindAztec(res.id, res.key);
			}
			if (item)
			{
				item->icon = icon;
				UccManager()->m_icon.insert(std::make_pair(iconName, icon));
				bResult = true;
				break;
			}
		}
		ClearTexCache(icon);
		delete icon;
		bResult = true;
		break;
	}
	case RESOURCE_UCC_TEMP_CLOTHES:
	{
		std::string url = m_download[RESOURCE_UCC_TEMP_CLOTHES].url;
		url += res.key[0];
		url += "/";
		std::string data = HttpDownloadText983(res.filename.c_str(), url.c_str());
		long width = 0;
		long height = 0;
		IFF_STRUCT::sPart* part = Doc()->m_itemManager.FindPart(res.id);
		if (part)
		{
			Bitmap* base =
				g_resrcmng->LoadBitmapA((const char*)part + 0xf4, 0, false);
			if (base)
			{
				width = base->bi->bmiHeader.biWidth;
				height = base->bi->bmiHeader.biHeight;
				delete base;
			}
		}
		Bitmap front;
		Bitmap back;
		front.Create(width, height, 32);
		back.Create(width, height, 32);
		if (data.size() == 0 ||
			!UnzipUccClothes((unsigned char*)data.c_str(), data.size(), &front,
				&back, NULL, NULL))
		{
			return false;
		}
		UccManager()->SetUccTempClothes(res.id, res.key, &front, &back);
		bResult = true;
		break;
	}
	}
	return bResult;
}

bool NetResourceManager::LoadBitmapA(const sResource& res)
{
	EnterCriticalSection(&m_csDownload);
	if (res.type == RESOURCE_GUILD_EMBLEM)
	{
		char path[256] = { 0 };
		sprintf(path, "%s%s\\%s", m_rootDir, m_download[res.type].dir,
			res.filename.c_str());
		Bitmap* bitmap = g_resrcmng->LoadBitmapA(path, 0, true);
		if (!bitmap)
		{
			DeleteFileA(path);
			LeaveCriticalSection(&m_csDownload);
			return false;
		}
		m_emblem[res.filename] = bitmap;
	}
	LeaveCriticalSection(&m_csDownload);
	return true;
}

const Bitmap* NetResourceManager::AddToDownloadList(const sResource& res,
	int bLoad, bool bFront)
{
	const Bitmap* ret = NULL;
	EnterCriticalSection(&m_csDownload);
	switch (res.type)
	{
	case RESOURCE_GUILD_EMBLEM:
		if (!bLoad)
		{
			LeaveCriticalSection(&m_csDownload);
			return NULL;
		}
		{
			std::map<std::string, Bitmap*>::const_iterator it =
				m_emblem.find(res.filename);
			if (it != m_emblem.end() && (*it).second != NULL)
			{
				ret = (*it).second;
			}
			else
			{
				if (!IsDownloadRequested(res))
				{
					if (bFront)
					{
						m_downloadList.push_front(res);
					}
					else
					{
						m_downloadList.push_back(res);
					}
					m_emblem[res.filename] = NULL;
				}
				ret = NULL;
			}
		}
		break;
	case RESOURCE_UCC_CLOTHES:
	case RESOURCE_UCC_TEMP_CLOTHES:
		if (!IsDownloadRequested(res))
		{
			if (bFront)
			{
				m_downloadList.push_front(res);
			}
			else
			{
				m_downloadList.push_back(res);
			}
		}
		break;
	default:
		if (bFront)
		{
			m_downloadList.push_front(res);
		}
		else
		{
			m_downloadList.push_back(res);
		}
		break;
	}
	LeaveCriticalSection(&m_csDownload);
	return ret;
}

void NetResourceManager::AddToUploadList(const sResource& res)
{
	if (IsUploadRequested(res))
	{
		// 983 (009aa8c0): avisa o ator principal (M983(0x23)) em vez de
		// enfileirar de novo
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)MakeStr("\xc0\xcc\xb9\xcc \xbe\xf7\xb7\xce\xb5\xe5 \xbf\xe4\xc3\xbb\xc0\xcc \xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. : [%d]",
				res.type),
			0, 0, 0, 0));
		return;
	}
	{
		EnterCriticalSection(&m_csUpload);
		m_uploadList.push_back(res);
		LeaveCriticalSection(&m_csUpload);
	}
}

const Bitmap* NetResourceManager::GetEmblemByName(const char* name)
{
	if (strlen(name) == 0)
	{
		return NULL;
	}
	char buf[13] = { 0 };
	strncpy(buf, name, 12);
	sResource res;
	res.type = RESOURCE_GUILD_EMBLEM;
	res.filename = MakeStr("%s.png", buf);
	return AddToDownloadList(res, 1, false);
}

// 983 009ab3e0 (59 chamadas; era stub no exe): emblema da guilda pelo numero da guilda e a versao do emblema, arquivo
// "<guilda>_<versao>.png"; a versao vai no 2o argumento do AddToDownloadList (o GetEmblemByName passa 1)
const Bitmap* NetResourceManager::GetEmblem983(unsigned long guildUID, unsigned long emblem)
{
	sResource res;
	res.type = RESOURCE_GUILD_EMBLEM;
	res.filename = MakeStr("%d_%d.png", guildUID, emblem);
	return AddToDownloadList(res, emblem, false);
}

void NetResourceManager::DownloadUccClothes(unsigned long id, const char* key,
	unsigned short flag, bool bFront)
{
	sResource res;
	res.filename = GenerateTextureName_F(id, key);
	res.type = RESOURCE_UCC_CLOTHES;
	res.id = id;
	res.flag = flag;
	strncpy(res.key, key, 9);
	AddToDownloadList(res, 0, false);
}

void NetResourceManager::DownloadUccTempClothes(unsigned long id,
	const char* key, unsigned short flag, bool bFront)
{
	sResource res;
	res.filename = GenerateTextureName_F(id, key);
	res.type = RESOURCE_UCC_TEMP_CLOTHES;
	res.id = id;
	res.flag = flag;
	strncpy(res.key, key, 9);
	AddToDownloadList(res, 0, false);
}

void NetResourceManager::Download(const sResource& res, const char* url)
{
	if (url)
	{
		m_download[res.type].reserved = 0;
		strcpy(m_download[res.type].dir, "./ntsaved/");
		strcpy(m_download[res.type].url, url);
	}
	AddToDownloadList(res, 0, false);
}

bool NetResourceManager::IsDownloadRequested(const sResource& res)
{
	// 983 (009a81e0): a busca na fila e feita dentro da secao critica de download
	EnterCriticalSection(&m_csDownload);
	std::list<sResource>::iterator it;
	for (it = m_downloadList.begin(); it != m_downloadList.end(); it++)
	{
		if ((*it).type == res.type && (*it).filename == res.filename)
		{
			LeaveCriticalSection(&m_csDownload);
			return true;
		}
	}
	LeaveCriticalSection(&m_csDownload);
	return false;
}

bool NetResourceManager::IsUploadRequested(const sResource& res)
{
	std::list<sResource>::iterator it;
	for (it = m_uploadList.begin(); it != m_uploadList.end(); it++)
	{
		if ((*it).type == res.type && (*it).filename == res.filename)
		{
			return true;
		}
	}
	return false;
}

void NetResourceManager::UploadUccClothes(unsigned long id, const char* key,
	const char* arg)
{
	sResource res;
	res.filename = GenerateTextureFullPath(0, id, key);
	res.type = RESOURCE_UCC_CLOTHES;
	res.id = id;
	strcpy(res.arg, arg);
	strncpy(res.key, key, 9);
	AddToUploadList(res);
}

void NetResourceManager::UploadUccTempClothes(unsigned long id, const char* key)
{
	sResource res;
	res.filename = GenerateTextureFullPath(0, id, key);
	res.type = RESOURCE_UCC_TEMP_CLOTHES;
	res.id = id;
	strncpy(res.key, key, 9);
	AddToUploadList(res);
}

bool NetResourceManager::SetUploadStatus(unsigned long id, const char* key,
	eResourceType type)
{
	EnterCriticalSection(&m_csUpload);
	unsigned char count;
	std::list<sUploadStatus>::iterator it;
	for (it = m_uploadStatus.begin(); it != m_uploadStatus.end(); it++)
	{
		if ((*it).id == id && strcmp((*it).key, key) == 0 && (*it).type == type)
		{
			++(*it).count;
			count = (*it).count;
			goto done;
		}
	}
	{
		sUploadStatus status;
		status.id = id;
		status.type = type;
		status.count = 1;
		strncpy(status.key, key, 9);
		m_uploadStatus.push_back(status);
		count = 1;
	}
done:
	LeaveCriticalSection(&m_csUpload);
	if ((type == RESOURCE_UCC_CLOTHES || type == RESOURCE_UCC_TEMP_CLOTHES ||
			type == RESOURCE_GUILD_EMBLEM) && // 983 009a8780: sem asteca
		count != 1)
	{
		return false;
	}
	return true;
}

void NetResourceManager::ClearGarbage()
{
	// 983 (009aabc0): sem o teste IsLocalContent; sempre junta as roupas UCC em uso
	std::list<sResource> inUse;
	{
		std::map<unsigned long, sRoomSlot>::iterator slot;
		for (slot = Doc()->m_roomSlotMap.begin();
			slot != Doc()->m_roomSlotMap.end(); ++slot)
		{
			sRoomSlot* kv = &(*slot).second;
			if (kv)
			{
				for (int i = 0; i < 24; i++)
				{
					if (IsUccClothes(kv->partTidList.m_tid[i]))
					{
						sItemInfo* item =
							GetMyItemInfo(kv->partTidList.m_uid[i]);
						if (item && strlen(item->UccIndex) != 0)
						{
							sResource res;
							res.id = kv->partTidList.m_tid[i];
							strncpy(res.key, item->UccIndex, 9);
							inUse.push_back(res);
						}
					}
				}
			}
		}
	}
	EnterCriticalSection(&m_csDownload);
	std::list<sResource>::iterator it = m_downloadList.begin();
	while (it != m_downloadList.end())
	{
		bool bFound = false;
		std::list<sResource>::iterator used;
		for (used = inUse.begin(); used != inUse.end(); used++)
		{
			if ((*it).id == (*used).id && strcmp((*it).key, (*used).key) == 0)
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			it++;
		}
		else
		{
			it = m_downloadList.erase(it);
		}
	}
	LeaveCriticalSection(&m_csDownload);
}

bool NetResourceManager::IsFailedToUpload(unsigned long id, const char* key,
	eResourceType type)
{
	bool bFailed = false;
	bool bNoMatch = true;
	if (type == RESOURCE_UCC_CLOTHES)
	{
		bFailed = UccManager()->FindClothes(id, key) == NULL;
	}
	else if (type == RESOURCE_UCC_TEMP_CLOTHES)
	{
		Bitmap bitmap1;
		Bitmap bitmap2;
		bFailed = !UccManager()->GetUccTempClothes(id, key, bitmap1, bitmap2);
	}
	EnterCriticalSection(&m_csUpload);
	std::list<sResource>::iterator it;
	for (it = m_uploadList.begin(); it != m_uploadList.end(); it++)
	{
		if ((*it).type == type && (*it).id == id && strcmp((*it).key, key) == 0)
		{
			bNoMatch = false;
		}
	}
	LeaveCriticalSection(&m_csUpload);
	if (bFailed && bNoMatch)
	{
		return true;
	}
	return false;
}

bool NetResourceManager::Upload(const sResource& res, const char* url)
{
	if (res.type < RESOURCE_NUM)
	{
		if (url && strlen(url) < 1024)
		{
			strcpy(m_uploadAddress[res.type], url);
		}
		AddToUploadList(res);
		return true;
	}
	return false;
}

void NetResourceManager::RequestGuildRSS(unsigned long guildId,
	void(__fastcall* callback)(int))
{
	m_rss.GetRSSFile(guildId, (void (*)(int))callback);
}

void NetResourceManager::RequestGuildRSS_JP(unsigned long guildId,
	void(__fastcall* callback)(int), const char* arg1, const char* arg2)
{
	m_rss.GetRSSFile_JP(guildId, (void (*)(int))callback, arg1, arg2);
}


// 983: nao existe (nenhum "BugReport" no exe; CollectBallStopData sem par). Vazia para o golfrule.cpp compilar.
void NetResourceManager::UploadBugReportData(const char* filename, int mainType,
	int subType, bool bAttachFile)
{
}
