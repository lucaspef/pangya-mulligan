#pragma once
// JP 983: bloco "extension" do arquivo da fita (replay), unidade 006269e0–006274a0 (sem classe; nomes meus). Sessão B58.
// O bloco é um pedaço de arquivo com cabeçalho de 0x13 bytes (`sExtChunk983`) seguido de um XML:
//   <?xml version="1.0"?><extension><repeat id="N"/></extension>
// (`repeat` só na sala de repetição, byte +0x1094f do CSharedDoc == 4; N = buraco fixo, CSharedDocJp::GetFixHole983).
// O XML vai em claro (versão 0), em base64 (1) ou cifrado com RC4 (4; senha = data/hora da fita "%d-%02d-%02d %02d:%02d:%02d").
// Gravador: 006884f0 (golfrule, frente A) chama ReplayWriteExtension983(fp, 4, &hora): tenta da versão pedida para baixo.
// Leitor: CSubGolfRuleBase (00650072, src/projectg/jp/subgolfrulebase.cpp) chama ReplayReadExtension983(fp, &hora).
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "tinyxml.h"

#pragma pack(push, 1)
struct sExtChunk983 // 0x13
{
	char tag[10];          // +0x00 "extension" (o leitor começa com "EXTENSION" e compara com strcmp)
	unsigned char version; // +0x0a índice nas tabelas de gravadores/leitores (0–4)
	unsigned int size;     // +0x0b
	void* data;            // +0x0f (no arquivo vai o ponteiro; o leitor sobrescreve)

	// 00626d10
	sExtChunk983(const char* name)
		: version(0), size(0), data(NULL)
	{
		strncpy(tag, name, 10);
	}
};
#pragma pack(pop)

// Contexto dos gravadores (8 bytes)
struct sExtWriter983
{
	sExtChunk983* chunk;
	FILE* fp;

	// 00626af0
	sExtWriter983(sExtChunk983* c, FILE* f)
		: chunk(c), fp(f)
	{
	}
	// 00626b10: cabeçalho + dados
	void Write(void* data, unsigned int size)
	{
		chunk->data = data;
		chunk->size = size;
		fwrite(chunk, 0x13, 1, fp);
		fwrite(chunk->data, chunk->size, 1, fp);
	}
};

// Contexto dos leitores (4 bytes)
struct sExtReader983
{
	TiXmlDocument* doc;

	// 00626b60
	sExtReader983(TiXmlDocument* d)
		: doc(d)
	{
	}
	// 00626b70 (jmp LoadFileFromMemoryEx)
	bool Parse(const char* text)
	{
		return doc->LoadFileFromMemoryEx(text);
	}
};

// 006271e0: grava o bloco (versão máxima 4; da maior para a menor até um gravador aceitar)
void __fastcall ReplayWriteExtension983(FILE* fp, int version, SYSTEMTIME* pTime);
// 00627340: lê o bloco, se o próximo pedaço do arquivo for "extension"
void __fastcall ReplayReadExtension983(FILE* fp, SYSTEMTIME* pTime);

C_ASSERT(sizeof(sExtChunk983) == 0x13);
