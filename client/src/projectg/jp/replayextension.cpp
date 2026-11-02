// JP 983: unidade 006269e0–006274a0, sem classe — bloco "extension" do arquivo da fita (ver include/jp/replayextension.h).
// Faixa com LTCG: 00626b80 recebe a hora em eax, 006270c0 o documento em esi e 00626fd0 o elemento na pilha (aqui saem
// __fastcall/static comuns: DIFERENTE esperado nelas e nas chamadas). As cópias fora de linha 006269e0/006269f0/00626a00
// (getters do CCryptBuffer983), 00626a10/00626a80 (construtores do codificador/decodificador), 00626cf0/00626d00, 00626d40/
// 00626f00 (TiXmlString()), 00626d50 (c_str), 00626d60 (TiXmlDocument::Error), 00626f10/00626f70 (~TiXmlDeclaration) e
// 00626fa0 vêm dos cabeçalhos. Sessão B58.
#include "minatl.h"
#include "projectg.h"
#include "commonutil.h"
#include "tinyxml.h"
#include "s5/utilities.h"
#include "shareddocjp.h"
#include "subgolfrule.h"
#include "replayextension.h"
#include "cryptbuffer.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) + offsetof(sRoomInfo, holeType) == 0x1094f); // A741: era cru Doc()+0x1094f

typedef bool (__fastcall *ExtWriter983)(const char* key, const char* text, unsigned int size, sExtWriter983* w);
typedef bool (__fastcall *ExtReader983)(const char* key, const char* data, unsigned int size, sExtReader983* r);

static const char* s_repeat = "repeat"; // 00e14b14
static const char* s_id = "id"; // 00e14b18

// 00626b80: senha da cifra = data e hora da fita
static const char* MakeExtKey983(const SYSTEMTIME* t)
{
	return MakeStr("%d-%02d-%02d %02d:%02d:%02d", t->wYear, t->wMonth, t->wDay, t->wHour, t->wMinute, t->wSecond);
}

// 00626bb0: versão 0, em claro
static bool __fastcall WriteExtPlain983(const char* key, const char* text, unsigned int size, sExtWriter983* w)
{
	w->Write((void*)text, size);
	return true;
}

// 00626c00
static bool __fastcall ReadExtPlain983(const char* key, const char* data, unsigned int size, sExtReader983* r)
{
	r->Parse(data);
	return true;
}

// 00626c20: versão 1, base64
static bool __fastcall WriteExtBase64983(const char* key, const char* text, unsigned int size, sExtWriter983* w)
{
	int len = S5::UTIL::Base64Encode((char*)text, size, NULL);
	char* buf = new char[len];
	S5::UTIL::Base64Encode((char*)text, size, buf);
	w->Write(buf, len);
	if (buf)
		delete[] buf;
	return true;
}

// 00626ca0
static bool __fastcall ReadExtBase64983(const char* key, const char* data, unsigned int size, sExtReader983* r)
{
	char* buf = new char[size];
	S5::UTIL::Base64Decode((char*)data, (unsigned char*)buf, size);
	r->Parse(buf);
	if (buf)
		delete[] buf;
	return true;
}

static sCryptParam983 s_extCrypt = { "Microsoft Base Cryptographic Provider v1.0", 1, 0x8003, 0x6801, 0x380000 }; // 00e14b44

// 00626d70: versão 4, RC4 com a hora da fita
static bool __fastcall WriteExtCrypt983(const char* key, const char* text, unsigned int size, sExtWriter983* w)
{
	CCryptEncoder983 enc(&s_extCrypt, key, text, size);
	if (enc.IsOk())
	{
		w->Write(enc.GetData(), enc.GetSize());
		return true;
	}
	return false;
}

// 00626e50
static bool __fastcall ReadExtCrypt983(const char* key, const char* data, unsigned int size, sExtReader983* r)
{
	CCryptDecoder983 dec(&s_extCrypt, key, data, size);
	if (dec.IsOk())
	{
		r->Parse((const char*)dec.GetData());
		return true;
	}
	return false;
}

static ExtWriter983 s_extWriters[5] = { WriteExtPlain983, WriteExtBase64983, NULL, NULL, WriteExtCrypt983 }; // 00e14b1c
static ExtReader983 s_extReaders[5] = { ReadExtPlain983, ReadExtBase64983, NULL, NULL, ReadExtCrypt983 }; // 00e14b30

// 00626fd0: <repeat id="buraco fixo"/> na sala de repetição
static void WriteExtRepeat983(TiXmlElement* parent)
{
	if (Doc()->m_roomInfo.holeType == 4)
	{
		TiXmlElement repeat(s_repeat);
		repeat.SetAttribute(s_id, DOCJP()->GetFixHole983());
		parent->InsertEndChild(repeat);
	}
}

// 00627070
static void ReadExtRepeat983(TiXmlNode* ext)
{
	int hole;
	TiXmlNode* node = ext->FirstChild(s_repeat);
	if (node)
	{
		node->ToElement()->Attribute(s_id, &hole);
		DOCJP()->SetFixHole983(hole);
	}
}

// 006270c0
static void BuildExtDoc983(TiXmlDocument* doc)
{
	TiXmlDeclaration decl("1.0", "", "");
	doc->InsertEndChild(decl);
	TiXmlElement ext("extension");
	WriteExtRepeat983(&ext);
	doc->InsertEndChild(ext);
}

// 006271c0
static void ReadExtDoc983(TiXmlDocument* doc)
{
	TiXmlNode* ext = doc->FirstChild("extension");
	if (ext)
		ReadExtRepeat983(ext);
}

// 006271e0
void __fastcall ReplayWriteExtension983(FILE* fp, int version, SYSTEMTIME* pTime)
{
	TiXmlDocument doc;
	BuildExtDoc983(&doc);
	sExtChunk983 chunk("extension");
	sExtWriter983 w(&chunk, fp);
	TiXmlOutStream out;
	out << doc;
	unsigned int len = out.length() + 1;
	if (version > 4)
		version = 4;
	for (int v = version; v >= 1; v--)
	{
		ExtWriter983 fn = s_extWriters[v];
		if (fn)
		{
			chunk.version = (unsigned char)v;
			if (fn(MakeExtKey983(pTime), out.c_str(), len, &w))
				break;
		}
	}
}

// 00627340
void __fastcall ReplayReadExtension983(FILE* fp, SYSTEMTIME* pTime)
{
	sExtChunk983 chunk("EXTENSION");
	if (fread(&chunk, 0x13, 1, fp) && strcmp(chunk.tag, "extension") == 0)
	{
		chunk.data = new char[chunk.size];
		fread(chunk.data, chunk.size, 1, fp);
		ExtReader983 fn;
		if (chunk.version < 5 && (fn = s_extReaders[chunk.version]) != NULL)
		{
			TiXmlDocument doc;
			sExtReader983 r(&doc);
			if (fn(MakeExtKey983(pTime), (const char*)chunk.data, chunk.size, &r) && !doc.Error())
			{
				ReadExtDoc983(&doc);
				TiXmlNode* ext = doc.FirstChild("extension");
				if (ext)
					ReadExtRepeat983(ext);
			}
		}
		if (chunk.data)
			delete[] (char*)chunk.data;
	}
}
