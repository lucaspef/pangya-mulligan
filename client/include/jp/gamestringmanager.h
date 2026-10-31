#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>
#include "engine/singleton.h"

// JP 983: textos traduzidos baixados do servidor (classe nova; RTTI GameStringManager, vtable 00d0963c, base
// WSingleton<GameStringManager> 00d09624, instância 00e75184, 0x64 bytes — `push 0x64` do criador 00a66dab). O texto vem por HTTP
// (wininet) em Base64; dentro, pares <TEXT><KEY>id</KEY><SERVICE>texto UTF-8</SERVICE></TEXT>. Nomes de método e de campo meus.
class GameStringManager : public WSingleton<GameStringManager>
{
public:
	GameStringManager(); // 00541800
	virtual ~GameStringManager(); // 005416a0 (slot 0 = 00541910)

	bool Load(const std::string& url); // 005420a0 (url vazia = endereço fixo)
	const char* GetString(int key); // 00540900
	bool IsLoaded() const { return m_bLoaded; } // 983 00a6943c le +0x44 direto; offset fixo so vale no STL do VC10

protected:
	void Clear(); // 00541470
	bool Download(); // 00542020
	bool Parse(const char* text); // 00541b50
	// 00541940: os trechos entre `open` e `close` (não usa o `this`)
	bool Extract(const std::string& src, const std::string& open, const std::string& close, std::vector<std::string>* pOut);
	bool Convert(const std::string& in, std::string* pOut, unsigned int codePage); // 00540550: UTF-8 → página de código

	std::map<int, std::string> m_strings; // +0x28
	std::list<int> m_list38; // +0x38 (só o construtor e o destrutor mexem)
	bool m_bLoaded; // +0x44
	std::string m_url; // +0x48
};

const char* __fastcall GetGameString(int key); // 00540a10 (texto do id; "" se não há; NULL sem gerente ou com id 0)
