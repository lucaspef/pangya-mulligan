#pragma once

#include <string>
#include <list>
#include <map>
#include "rssmanager.h"

// 983 (s105): 4 tipos - Init 009abb70 (emblema no 1, UCC no 0 copiado para o 3), CGhostManager 00828c7e (ghost = 2),
// WorkUploadThread 009a92f0 (0/3 = UCC com chave); sem asteca e sem relatorio de erro. sizeof(NetResourceManager) 0x32f8.
enum eResourceType
{
	RESOURCE_UCC_CLOTHES,
	RESOURCE_GUILD_EMBLEM,
	RESOURCE_GHOST,
	RESOURCE_UCC_TEMP_CLOTHES,
	RESOURCE_NUM,
	// so do 645 (ficam fora dos vetores; nenhum codigo do 983 usa)
	RESOURCE_UCC_AZTEC = 0x100,
	RESOURCE_BUG_REPORT
};

struct sResource
{
	eResourceType type;
	std::string filename;
	unsigned long id;
	char key[9];
	unsigned short flag;
	char arg[64];

	sResource()
	{
		id = 0;
		flag = 0;
		memset(arg, 0, sizeof(arg));
		memset(key, 0, sizeof(key));
	}
};

struct sUploadStatus
{
	unsigned long id;
	char key[9];
	eResourceType type;
	unsigned char count;

	sUploadStatus()
	{
		id = 0;
		type = RESOURCE_UCC_CLOTHES;
		count = 0;
		memset(key, 0, sizeof(key));
	}
};

class NetResourceManager : public WSingleton<NetResourceManager>
{
	friend unsigned int __stdcall CreateDownloadThreadFunc(void* param);
	friend unsigned int __stdcall CreateUploadThreadFunc(void* param);

public:
	struct sPostArgument
	{
		std::string name;
		std::string value;
		bool bBinary;

		sPostArgument();
	};

	NetResourceManager();
	virtual ~NetResourceManager();

	void Init();
	void Close();

	const Bitmap* GetEmblemByName(const char* name);
	// 983: 009ab3e0, 59 chamadas; toma o lugar do GetEmblemByName(guildMark) nas telas (nome meu)
	const Bitmap* GetEmblem983(unsigned long guildUID, unsigned long emblem);
	void DownloadUccClothes(unsigned long id, const char* key,
		unsigned short flag, bool bFront);
	void DownloadUccTempClothes(unsigned long id, const char* key,
		unsigned short flag, bool bFront);
	void Download(const sResource& res, const char* url);
	// 983: sem funcao propria (inlinada nos chamadores)
	bool IsDownloadRequested(eResourceType type, unsigned long id,
		const char* key)
	{
		sResource res;
		res.type = type;
		res.id = id;
		strncpy(res.key, key, 9);
		return IsDownloadRequested(res);
	}
	void UploadUccClothes(unsigned long id, const char* key, const char* arg);
	void UploadUccTempClothes(unsigned long id, const char* key);
	bool SetUploadStatus(unsigned long id, const char* key, eResourceType type);
	bool IsFailedToUpload(unsigned long id, const char* key,
		eResourceType type);
	bool Upload(const sResource& res, const char* url);
	void RequestGuildRSS(unsigned long guildId,
		void(__fastcall* callback)(int));
	void RequestGuildRSS_JP(unsigned long guildId,
		void(__fastcall* callback)(int), const char* arg1, const char* arg2);
	void UploadBugReportData(const char* filename, int mainType, int subType,
		bool bAttachFile);

	bool IsRSSDownLoadComplete() { return m_rss.IsComplete(); }

	void ResetGuildRSS() { m_rss.ResetVars(); }

	void SetClearGarbage() { m_bClearGarbage = true; }
	char* GetUploadAddress(eResourceType type) { return m_uploadAddress[type]; }

protected:
	int WorkDownloadThread();
	int WorkUploadThread();
	bool _RealDownLoad(const sResource& res);
	bool LoadBitmapA(const sResource& res);
	const Bitmap* AddToDownloadList(const sResource& res, int bLoad,
		bool bFront);
	void AddToUploadList(const sResource& res);
	bool IsDownloadRequested(const sResource& res);
	bool IsUploadRequested(const sResource& res);
	void ClearGarbage();

private:
	struct sDownloadInfo
	{
		char dir[1024];
		char url[1024];
		int reserved;
	};

	RSSManager m_rss;
	std::list<sResource> m_downloadList;
	std::list<sResource> m_uploadList;
	void* m_hDownloadThread;
	void* m_hUploadThread;
	_RTL_CRITICAL_SECTION m_csDownload;
	_RTL_CRITICAL_SECTION m_csUpload;
	bool m_bRunning;
	char m_rootDir[512];
	std::map<std::string, Bitmap*> m_emblem;
	sDownloadInfo m_download[RESOURCE_NUM];
	char m_uploadAddress[RESOURCE_NUM][1024];
	std::list<sUploadStatus> m_uploadStatus;
	bool m_bClearGarbage;
	// 983: sem o m_postArgument do 645 (o construtor 009abd40 so monta 2 listas, o mapa de emblemas e m_uploadStatus)
#ifndef REBANG_SEMANTIC_ONLY
	friend struct
		RbNrmFunclets; // HACK: EH funclets of the naked functions (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbWorkuploadthread2Funclets(); // HACK: EH funclets of the naked WorkUploadThread (nakedify)
#endif
};

inline NetResourceManager* NetResManager()
{
	return NetResourceManager::Instance();
}
