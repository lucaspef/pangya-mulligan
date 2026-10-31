#include "minatl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "ijl.h"
#include "exception.h"
#include "registry.h"
#include "ccrc32.h"
#include "chttp.h"
#include "../shared/localize.h"
#include "tinyxml.h"
#include "westpak.h"
#include "wresrcmng.h"
#include <list>
#include <functional>
#include "jp/uiframesetmanager.h"
#include "shareddoc.h" // F4 (C84): 983 GetRegIntegratedPakName
#include "wlocalize.h"
#include "jp/inetfile.h"


extern HINSTANCE g_instance;
extern WResourceManager* g_resrcmng;

int g_iLoadTotalNum = 100;
int g_iLoadCurNum = 0;
int g_iLoadNextNum = 0;
bool g_bExitDrawLoading = false;
WResourceManager* WFile::m_pResMng = 0;

// F4 (C84): 983 key (00d3d60c, passed to DecodeTEA_NBYTE 00bc6560 at 0098e676);
// 645 QA: { 0x0485b576, 0x05148e02, 0x05141d96, 0x028fa9d6 }.
static const unsigned int g_updateListKey[4] = { 0x020a5fd4, 0x01eebdff,
	0x02b3c6a0, 0x04f6a3e1 };

void SaveJPG(const char* filename, const Bitmap* bitmap, int quality)
{
	JPEG_CORE_PROPERTIES jcprops;
	memset(&jcprops, 0, sizeof(jcprops));
	if (ijlInit(&jcprops) == IJL_OK)
	{
		jcprops.DIBBytes = bitmap->vram;
		jcprops.DIBWidth = bitmap->bi->bmiHeader.biWidth;
		jcprops.DIBHeight = bitmap->bi->bmiHeader.biHeight;
		jcprops.DIBColor = IJL_BGR;
		jcprops.DIBChannels = 3;
		jcprops.DIBPadBytes = bitmap->pitch - bitmap->bi->bmiHeader.biWidth * 3;
		jcprops.JPGHeight = jcprops.DIBHeight;
		jcprops.JPGWidth = jcprops.DIBWidth;
		jcprops.JPGFile = filename;
		jcprops.JPGColor = IJL_YCBCR;
		jcprops.JPGChannels = 3;
		jcprops.JPGSubsampling = 1;
		jcprops.jquality = quality;
		ijlWrite(&jcprops, IJL_JFILE_WRITEWHOLEIMAGE);
		ijlFree(&jcprops);
	}
}

void SaveBMP(const char* filename, const Bitmap* bitmap)
{
	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = 0;
	info.biHeight = 0;
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biCompression = 0;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	BITMAPFILEHEADER header;
	header.bfType = 0x4d42;
	header.bfSize = 0xe;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfOffBits = 0x36;

	FILE* fp = fopen(filename, "wb");
	if (fp)
	{
		info.biWidth = bitmap->bi->bmiHeader.biWidth;
		info.biHeight = bitmap->bi->bmiHeader.biHeight;
		fwrite(&header, 1, sizeof(header), fp);
		fwrite(&info, 1, sizeof(info), fp);
		unsigned int bytespp = bitmap->bi->bmiHeader.biBitCount >> 3;
		int width = info.biWidth;
		for (int y = info.biHeight - 1; y >= 0; --y)
		{
			int offset = 0;
			for (int x = 0; x < width; ++x)
			{
				const unsigned char* p =
					bitmap->vram + offset + bitmap->pitch * y;
				fputc(p[0], fp);
				fputc(p[1], fp);
				fputc(p[2], fp);
				offset += bytespp;
			}
			int pad = ((width * 3 + 3) & ~3) - width * 3;
			for (int i = 0; i < pad; ++i)
				fputc(0, fp);
		}
		fclose(fp);
	}
}

const char* LoadStr(unsigned int id)
{
	static char s_buf[0x200];
	LoadStringA(g_instance, id, s_buf, sizeof(s_buf));
	return s_buf;
}

bool IsPakFile(const char* name)
{
	int len = strlen(name);
	if (len < 5)
		return false;
	return strcmpi(name + len - 4, ".pak") == 0;
}

struct PAKINFO
{
	char name[260];
	unsigned long crc;
	unsigned long size;
	char sortName[260]; // 983 PAKINFO+0x10c: chave do less (0098c030)
};

namespace std
{
	template <>
	struct less<PAKINFO> : public binary_function<PAKINFO, PAKINFO, bool>
	{
		bool operator()(PAKINFO a, PAKINFO b)
		{
			if (strcmpi("ProjectG.exe", a.name) == 0)
				return false;
			if (strcmpi("ProjectG.exe", b.name) == 0)
				return true;
			return strcmpi(a.sortName, b.sortName) < 0;
		}
	};
}

bool IsIntegratedPakFile(const char* name)
{
	int len = strlen(name);
	if (len < 6)
		return false;
	return strcmpi(name + len - 5, "+.pak") == 0;
}

bool IsExeFile(const char* name)
{
	int len = strlen(name);
	if (len < 5)
		return false;
	return strcmpi(name + len - 4, ".exe") == 0;
}

void DeleteReadOnlyFile(const char* filename)
{
	DWORD attr = GetFileAttributesA(filename);
	if (attr & FILE_ATTRIBUTE_READONLY)
		SetFileAttributesA(filename, attr & ~FILE_ATTRIBUTE_READONLY);
	DeleteFileA(filename);
}

void TrimExt(const char* src, char* dst)
{
	strcpy(dst, src);
	char* dot = strrchr(dst, '.');
	if (dot)
		*dot = 0;
}

void GetModuleName(char* name)
{
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	char* slash = strrchr(path, '\\');
	if (slash)
		strcpy(name, slash + 1);
	else
		strcpy(name, path);
}

bool DownloadUpdateList(std::list<PAKINFO>* paks, char* version);

int DecodeTEA_NBYTE(char* dest, int size, const char* src,
	const unsigned int* key);

// 983 0098e2b0: "ProjectG801j.pak" -> "projectg0801.pak" (ordem numerica); sem numero apos o 8o caractere, o proprio nome
static std::string PakSortName(const char* name)
{
	try
	{
		std::string num = std::string(name).substr(8);
		char buf[260];
		sprintf(buf, "projectg%04d.pak", std::stoi(num));
		return buf;
	}
	catch (...)
	{
		return name;
	}
}

bool DownloadUpdateList(std::list<PAKINFO>* paks, char* version)
{
	// 983 0098e4a0 (D230): the list comes from HttpDownloadText983 (00a4cb40, CINetBuffer), not cHttp; an empty
	// answer shows "<msg> : Code[<HttpGetLastError983>]" (MB_ICONINFORMATION)
	std::string url = "http://dl.pangya.jp";
#ifdef JP983_SERVER_CONFIG
	// sem rugburn nada reescreve a URL: JP983_WEB=http://host:porta, senao o web 30091 do robo (client/COMPILAR.md item 5)
	// C642: "localhost", nao 127.0.0.1 - o web do PC so atende Host localhost (gglog vs2022: 127.0.0.1 parou, rugburn -> localhost segue)
	char web[128];
	url = GetEnvironmentVariable("JP983_WEB", web, sizeof(web)) ? web : "http://localhost:30091";
	// A214: o rugburn reescreve dl.pangya.jp/pangya/season4/patch/updatelist para <web>/patch/updatelist (gglog a212-1)
	std::string file = "/patch/updatelist";
#else
	std::string file = "/pangya/season4/patch/updatelist";
#endif
	std::string body = HttpDownloadText983(file, url);
	if (body.empty())
	{
		unsigned long code = HttpGetLastError983();
		MessageBox(NULL,
			MakeStr("%s : Code[%d]",
				K2L_Compatibility("\xbc\xad\xb9\xf6\xb7\xce\xba\xce\xc5\xcd \xc6\xc4\xc0\xcf \xb8\xf1\xb7\xcf\xc0\xbb \xb0\xa1\xc1\xae\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				code),
			K2L_Compatibility("\xbe\xcb\xb8\xb2"), MB_ICONINFORMATION);
		return false;
	}

	unsigned long size = (unsigned long)body.size();
	char* xml = new char[size + 8];
	memcpy(xml, body.c_str(), size + 1);
	if (IsLocalContent(S3_ENCRYPTION_UPDATELIST))
	{
		int padded = size + ((size & 7) ? 8 - (size & 7) : 0);
		char* dec = new char[padded + 1];
		memset(dec, 0, padded + 1);
		DecodeTEA_NBYTE(dec, padded, xml, g_updateListKey);
		delete[] xml;
		xml = dec;
	}

	TiXmlDocument doc;
	TiXmlString text;
	if (size)
		text.reserve(size);
	text += xml;
	doc.Parse(text.c_str());
	if (xml)
		delete[] xml;
	if (doc.Error())
	{
		MessageBox(NULL,
			K2L_Compatibility("XML \xc6\xc4\xbd\xcc\xc1\xdf \xbf\xa1\xb7\xaf\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
			K2L_Compatibility("\xbe\xcb\xb8\xb2"), MB_ICONINFORMATION);
		return false;
	}

	std::string tag[5];
	if (IsLocalContent(S3_ENCRYPTION_UPDATELIST))
	{
		tag[0] = "updatefiles";
		tag[1] = "fileinfo";
		tag[2] = "fname";
		tag[3] = "fsize";
		tag[4] = "fcrc";
	}
	else
	{
		tag[0] = "filelist";
		tag[1] = "file";
		tag[2] = "filename";
		tag[3] = "size";
		tag[4] = "crc";
	}

	TiXmlNode* root = doc.FirstChild(tag[0].c_str());
	if (!root)
	{
		MessageBox(NULL,
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xb8\xf1\xb7\xcf\xc0\xbb \xc0\xd0\xb4\xc2 \xc1\xdf \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xbf\xa1\xb7\xaf"), MB_ICONINFORMATION);
		return false;
	}

	for (TiXmlNode* node = root->FirstChild(tag[1].c_str()); node;
		node = node->NextSibling())
	{
		TiXmlElement* element = node->ToElement();
		int dummy;
		PAKINFO info;
		memset(&info, 0, sizeof(info));
		strcpy(info.name, element->Attribute(tag[2].c_str(), &dummy));
		size_t len = strlen(info.name);
		if (len < 5 || strcmpi(info.name + len - 4, ".pak") != 0)
		{
			if (strcmpi("ProjectG.exe", info.name) == 0)
			{
				info.size = atoi(node->ToElement()->Attribute("fsize", &dummy));
				info.crc = atoi(node->ToElement()->Attribute("fcrc", &dummy));
				paks->push_back(info);
			}
			continue;
		}
		char base[64] = "\0";
		strcpy(base, info.name);
		char* dot = strrchr(base, '.');
		if (dot)
			*dot = '\0';
		if (strcmpi(base, version) == 0 ||
			(strcmpi(base, version) > 0 && !IsIntegratedPakFile(info.name)))
		{
			strcpy(info.sortName, PakSortName(info.name).c_str());
			info.size = atoi(element->Attribute(tag[3].c_str(), &dummy));
			info.crc = atoi(element->Attribute(tag[4].c_str(), &dummy));
			paks->push_back(info);
		}
	}
	return true;
}

// F4 (frente C, C84): 983 0098dc80 - HKLM "Software\\Ntreev\\Pangya_Jp" (the 645 QA read Pangya_QA, which does not
// exist on a JP install: empty name -> "file error" box in InitPakedFile2). With m_bChannelLogin the key comes from the
// suffix of the command line id (m_commandLineID, lowered in place like the 983 _strlwr): @hg / @gr / @lv.
WTL::CString __stdcall GetRegIntegratedPakName()
{
	const char* key = "Software\\Ntreev\\Pangya_Jp";
	CSharedDoc* doc = CSharedDoc::Instance();
	if (doc->m_bChannelLogin)
	{
		std::string id(_strlwr(const_cast<char*>(doc->m_commandLineID.c_str())));
		if (id.find("@hg") != std::string::npos)
			key = "Software\\Ntreev\\PangYa_Hangame";
		else if (id.find("@gr") != std::string::npos)
			key = "Software\\Ntreev\\PangYa_Atgames";
		else if (id.find("@lv") != std::string::npos)
			key = "Software\\Ntreev\\PangYa_Lievo";
	}
	CRegistry registry;
	registry.SetRootKey(HKEY_LOCAL_MACHINE);
	registry.SetKey(key, true);
	WTL::CString name = "";
	name = registry.ReadString("IntegratedPak", "");
	return name;
}

namespace std
{
	template <>
	struct greater<PAKINFO> : public binary_function<PAKINFO, PAKINFO, bool>
	{
		bool operator()(const PAKINFO& a, const PAKINFO& b) const
		{
			return a.size > b.size;
		}
	};
}

int __stdcall IntegrityPakCRC(std::list<PAKINFO>& paks, const char* version,
	std::string& errName)
{
	SYSTEM_INFO si;
	memset(&si, 0, sizeof(si));
	GetSystemInfo(&si);
	bool multiCpu = si.dwNumberOfProcessors > 1;

	CRC_32 crc;
	std::list<PAKINFO> sorted;
	std::list<PAKINFO>::iterator it = paks.begin();
	for (; it != paks.end(); ++it)
	{
		PAKINFO& pak = *it;
		if (strcmpi("ProjectG.exe", pak.name) == 0)
		{
			char path[260] = "\0";
			GetModuleName(path);
			if (strcmpi(path, pak.name) != 0 ||
				crc.CalcCRC(pak.name, NULL) != pak.crc)
			{
				errName = "ProjectG.exe";
				g_bExitDrawLoading = true;
				return 0;
			}
		}
		else
		{
			char base[260] = "\0";
			strcpy(base, pak.name);
			char* dot = strrchr(base, '.');
			if (dot)
				*dot = '\0';
			if (strcmpi(version, base) <= 0)
			{
				if (!multiCpu || strstr(pak.name, "_"))
					sorted.push_back(pak);
				HookPAKFile(pak.name);
			}
		}
	}

	float total = (float)sorted.size();
	sorted.sort(std::greater<PAKINFO>());
	float done = 0.0f;
	for (it = sorted.begin(); it != sorted.end(); ++it)
	{
		if (crc.CalcCRC((*it).name, NULL) != (*it).crc)
		{
			errName = (*it).name;
			return 0;
		}
		done += 1.0f;
		g_iLoadNextNum = (int)((done / total) * (float)g_iLoadTotalNum);
	}
	g_iLoadNextNum = g_iLoadTotalNum;
	return 1;
}

bool __stdcall GetCRCCheckList(
	std::list<std::pair<std::string, unsigned long> >& checkList)
{
	std::list<PAKINFO> paks;
	WTL::CString pakName = GetRegIntegratedPakName();
	char version[64] = "\0";
	if (pakName.GetLength() == 0)
	{
		MessageBox(NULL,
			K2L_Compatibility("\xc6\xc4\xc0\xcf\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbc\xb3\xc4\xa1\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."), // 983 0098d8fe
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xbf\xa1\xb7\xaf"), MB_ICONEXCLAMATION);
		return false;
	}
	strcpy(version, pakName);
	char* dot = strrchr(version, '.');
	if (dot)
		*dot = '\0';
	if (!DownloadUpdateList(&paks, version))
		return false;
	paks.sort(std::less<PAKINFO>());
	for (std::list<PAKINFO>::iterator it = paks.begin(); it != paks.end(); ++it)
	{
		PAKINFO& pak = *it;
		if (strcmpi("ProjectG.exe", pak.name) == 0)
			continue;
		char base[260] = "\0";
		strcpy(base, pak.name);
		char* p = strrchr(base, '.');
		if (p)
			*p = '\0';
		if (strcmpi(version, base) <= 0 && !strstr(pak.name, "_"))
		{
			std::string name(pak.name);
			checkList.push_back(
				std::pair<std::string, unsigned long>(name, pak.crc));
		}
	}
	return true;
}

// 983 0098f8c0 (D230): the empty-name message and the final box go through K2L_Compatibility (00b4ec60); a failed check
// only shows the box when PANGYA_ARG_SUB != "noWindow"; version is TrimExt(name) (0098c1e0); the success path
// calls SetAutoMatchDirectory("data") (00b5bc20).
bool InitPakedFile2()
{
	std::list<PAKINFO> paks;
	WTL::CString pakName = GetRegIntegratedPakName();
	char version[64] = "\0";
	if (pakName.GetLength() == 0)
	{
		MessageBox(NULL,
			K2L_Compatibility("\xc0\xce\xbd\xba\xc5\xe7 \xb9\xf6\xc1\xaf\xc0\xbb \xbc\xb3\xc4\xa1, \xc8\xa4\xc0\xba \xb7\xb1\xc3\xc4\xb8\xa6 \xb8\xd5\xc0\xfa \xbd\xc7\xc7\xe0\xc7\xcf\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."),
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xbf\xa1\xb7\xaf"), MB_ICONEXCLAMATION);
		return false;
	}
	TrimExt(pakName, version);
	if (!DownloadUpdateList(&paks, version))
		return false;
	paks.sort(std::less<PAKINFO>());
	std::string errName;
	if (IntegrityPakCRC(paks, version, errName))
	{
		g_resrcmng->SetAutoMatchDirectory("data");
		return true;
	}
	char arg[64] = "\0";
	GetEnvironmentVariableA("PANGYA_ARG_SUB", arg, sizeof(arg));
	if (strcmp(arg, "noWindow") != 0)
	{
		MessageBox(NULL,
			MakeStr(
				K2L_Compatibility("%s \xc6\xc4\xc0\xcf\xbf\xa1 \xbc\xd5\xbb\xf3\xc0\xcc \xb9\xdf\xb0\xdf\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc7\xc7\xe0\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
				errName.c_str()),
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xbf\xa1\xb7\xaf"), MB_ICONEXCLAMATION);
	}
	return false;
}

// 983 0098d6a0: Catch_All 0098d8f1 cobre a funcao toda: aviso "reinstale" e rethrow
void InitPakedFile()
{
	try
	{
		WList<char*> list(16, 16);
		WIN32_FIND_DATA fd;
		HANDLE find = FindFirstFile("projectg*.pak", &fd);
		if (find != INVALID_HANDLE_VALUE)
		{
			do
			{
				char* name = new char[strlen(fd.cFileName) + 1];
				strcpy(name, fd.cFileName);
				list += name;
			} while (FindNextFile(find, &fd));
		}
		FindClose(find);

		while (true)
		{
			char* first = list.Start();
			if (!first)
			{
				g_resrcmng->SetAutoMatchDirectory("data");
				return;
			}
			char* smallest = first;
			for (char* cur = first; cur; cur = list.Next())
			{
				if (strcmp(cur, smallest) < 0)
					smallest = cur;
			}
			list -= smallest;
			HookPAKFile(smallest);
			delete[] smallest;
		}
	}
	catch (...)
	{
		MessageBox(NULL, K2L_Compatibility("\xc6\xc4\xc0\xcf\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbc\xb3\xc4\xa1\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
			K2L_Compatibility("\xc6\xc4\xc0\xcf \xbf\xa1\xb7\xaf"), MB_ICONINFORMATION);
		throw;
	}
}

void LineParse(const char* line, std::vector<std::string>& tokens)
{
	char c = *line;
	while (true)
	{
		if (c == '\0' || *line == '\r')
			return;
		std::string token;
		if (*line == '"')
		{
			c = line[1];
			while (line = line + 1, c != '"')
			{
				if (c == '\0')
				{
					if (*line != '"')
						goto push;
					break;
				}
				token += c;
				c = line[1];
			}
			line = line + 1;
		}
		else
		{
			int sp = isspace(*line);
			for (; sp == 0 && *line != '\0'; line = line + 1)
			{
				token += *line;
				sp = isspace(line[1]);
			}
		}
push:
		tokens.push_back(token);
		int sp = isspace(*line);
		while (sp != 0)
		{
			const char* n = line + 1;
			line = line + 1;
			sp = isspace(*n);
		}
		c = *line;
	}
}

char* GetStr(char* src, char* dst)
{
	while (*src == ' ' || *src == '\t')
	{
		if (*src == 0)
			goto done;
		++src;
	}
	if (*src)
	{
		if (*src++ == '"')
		{
			while (*src)
			{
				if (*src == '"')
				{
					if (*src)
						++src;
					goto done;
				}
				*dst++ = *src++;
			}
		}
	}
done:
	*dst = 0;
	return src;
}

const char* GetStrConst(const char* src, char* dst)
{
	while (*src == ' ' || *src == '\t')
	{
		if (*src == 0)
			goto done;
		++src;
	}
	if (*src)
	{
		if (*src++ == '"')
		{
			while (*src)
			{
				if (*src == '"')
				{
					if (*src)
						++src;
					goto done;
				}
				*dst++ = *src++;
			}
		}
	}
done:
	*dst = 0;
	return src;
}

const char* GetCharsTillSpace(const char* src, char* dst)
{
	while (*src == ' ' || *src == '\t')
	{
		if (*src == 0)
			goto fail;
		++src;
	}
	char c = *src;
	if (c != 0)
		goto loop;
fail:
	*dst = 0;
	return NULL;
	for (;;)
	{
		++src;
		*dst++ = c;
		c = *src;
		if (c == 0)
			break;
loop:
		if (c == ' ')
			break;
	}
	*dst = 0;
	return src;
}

char* FindTag(char* str, char* end, char* tag)
{
	if (str && tag)
	{
		char* found = strstr(str, tag);
		if (found && found < end)
			return found + strlen(tag);
	}
	return NULL;
}

void LimitText(char* text, int limit)
{
	if (0)
		Random(*(float*)0, *(float*)0, *(int*)0);
	if (text)
	{
		if (strlen(text) > (unsigned int)limit)
		{
			int i = 0;
			if (0 < limit)
			{
				int count = 1;
				do
				{
					// 983 0098cbb0 (D230): Shift-JIS lead bytes 0x80..0x9f / 0xe0..0xfc (the 645 tested only the high bit)
					unsigned char c = (unsigned char)text[i];
					if ((c >= 0x80 && c < 0xa0) || (unsigned char)(c + 0x20) <= 0x1c)
					{
						if (count >= limit)
							break;
						++i;
						++count;
					}
					++i;
					++count;
				} while (i < limit);
			}
			text[i] = 0;
		}
	}
}

char* __cdecl MakeStr(const char* format, ...)
{
	static __declspec(thread) char s_strBuf[64][0x240];
	static __declspec(thread) unsigned char s_strIndex;
	_vsnprintf(s_strBuf[s_strIndex & 0x3f], 0x240, format,
		(char*)(&format + 1));
	return s_strBuf[s_strIndex++ & 0x3f];
}

std::string GetMyComputerName()
{
	char name[20];
	DWORD size = 17;
	GetComputerNameA(name, &size);
	return std::string(name);
}

float Random(float min, float max, int seed)
{
	if (seed < 0)
		seed = rand();
	return (max - min) * (float)(seed % 0x3ff) * (1.0f / 1024.0f) + min;
}

unsigned long htoi(const char* str)
{
	char buf[256];
	unsigned long value;
	if (str)
	{
		value = strlen(str);
		if (value)
		{
			sprintf(buf, "0x%s", str);
			sscanf(buf, "%X", &value);
			return value;
		}
	}
	return 0;
}

void RemakeNumber(int number, char* out)
{
	char buf[32];
	sprintf(buf, "%d", number);
	int len = strlen(buf);
	int remain = len;
	int o = 0;
	int i = 0;
	if (0 < len)
	{
		while (true)
		{
			--remain;
			out[o] = buf[i];
			++i;
			++o;
			if (len <= i)
				break;
			if (remain % 3 == 0)
				out[o++] = ',';
		}
	}
	out[o] = 0;
}

unsigned long ModifyColor(unsigned long color, float f)
{
	if (0)
		LimitText(*(char**)0, *(int*)0);
	float r = (float)((color >> 16) & 0xff) * f;
	if (r > 255.0f)
		r = 255.0f;
	unsigned char cr = (unsigned char)r;
	float g = (float)((color >> 8) & 0xff) * f;
	if (g > 255.0f)
		g = 255.0f;
	unsigned char cg = (unsigned char)g;
	float b = (float)(color & 0xff) * f;
	if (b > 255.0f)
		b = 255.0f;
	unsigned char cb = (unsigned char)b;
	return (cr << 16) | (cg << 8) | cb;
}

bool str_Replace(std::string& str, const std::string& from,
	const std::string& to)
{
	size_t pos = str.find(from);
	if (pos == std::string::npos)
		return false;
	do
	{
		str.replace(str.begin() + pos, str.begin() + pos + from.size(), to);
		pos = str.find(from, pos + to.size());
	} while (pos != std::string::npos);
	return true;
}

WFile::WFile()
	: m_filename(NULL), m_pFile(NULL)
{
}

cFile* WFile::Open(const char* filename, unsigned int option)
{
	Close();
	m_pFile = m_pResMng->GetCFile(filename, option);
	return m_pFile;
}

WFile::WFile(const char* filename)
	: m_filename(filename), m_pFile(NULL)
{
	m_pFile = m_pResMng->GetCFile(filename, -1);
	if (!m_pFile)
		throw WAppException(
			K2L_Compatibility("\xc6\xc4\xc0\xcf\xc0\xbb \xbf\xad \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
}

WFile::~WFile()
{
	Close();
}

void WFile::Close()
{
	if (m_pFile)
		CloseCFile(m_pFile);
	m_pFile = NULL;
}

int WFile::GetByte()
{
	if (m_pFile)
		return m_pFile->GetByte();
	return -1;
}

int WFile::Tell()
{
	if (m_pFile)
		return m_pFile->Tell();
	return -1;
}

int WFile::Read(void* buffer, int size)
{
	if (m_pFile)
		return m_pFile->Read(buffer, size);
	return -1;
}

void WFile::Seek(int offset, int origin)
{
	if (m_pFile)
		m_pFile->Seek(offset, origin);
}

int WFile::Length()
{
	if (0)
		((WFile*)0)->WFile::~WFile();
	if (m_pFile)
		return m_pFile->m_nLen;
	return -1;
}

void WFile::SetResourceManager(WResourceManager* pResMng)
{
	m_pResMng = pResMng;
}

// JP 983 (0098edc0)
float ScaleByViewHeight983(WView* pView, float value)
{
	CUiFrameSetManager::Size scl(1.0f, pView->GetHeight() / 480.0);
	CUiFrameSetManager::Dim dim;
	dim.Set(value, 4);
	return dim.Calc(scl);
}

// JP 983 (0098cd30): divide pela escala da altura (altura / 480, arredondada para baixo sem o m_bA4 do CUiFrameSetManager);
// a escala sai do ScaleByViewHeight983 de 1.0 (mesma medida scltype 4)
float UnscaleByViewHeight983(WView* pView, float value)
{
	return value / ScaleByViewHeight983(pView, 1.0f);
}
