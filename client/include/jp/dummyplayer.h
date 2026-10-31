#pragma once
#include <string>
#include <vector>
#include <list>
#include <memory>
#include "petscriptobject.h"
#include "wscene.h"

namespace lua_system
{
	class lua_script;
}
class CPetFrame;
class CPartTidList;
class FxInterpreter;
struct w_motion_data;
struct sTexturePart; // src/projectg/texturepart.h (frente A)

// JP 983: boneco de jogador do lobby e do pódio (RTTI .?AVIDummyPlayer@@, vtable 00d04bbc: as 14 do IPetScriptObject + 3
// próprias), sizeof 0xe4 (`push 0xe4` antes do `new` em 00567a61). Unidade dummyplayeractor 004d5410–004dabd2; par no KR 839
// (004d9520…). É a IDummyPet com um CPetFrame (boneco de peças) no lugar do WPuppet e uma luz própria. Nomes meus, salvo os
// das virtuais herdadas (frente A, petscriptobject.h). Sessão B32.
class IDummyPlayer : public IPetScriptObject
{
public:
	IDummyPlayer(int kind); // 004d78f0
	virtual ~IDummyPlayer(); // 004d7a30 (com `delete`: 004d8ef0)
	virtual void OnScriptSnd(const char*& p); // 1  004d5500
	virtual void OnScriptHideClub(const char*& p); // 5  004d55a0
	virtual void OnScriptFx(const char*& p, bool bSkip, int cmd); // 8  004d8580 (B33)
	virtual void OnScriptPTex(const char*& p); // 9  004d8f20
	virtual void OnScriptShowBone(const char*& p, int cmd); // 10 004d5f60
	virtual void OnScriptPlayIf(const char*& p); // 11 004d7200
	virtual void OnScriptShowVis(const char*& p, int cmd); // 13 004d5f80
	virtual bool SetMotion(const char* name, bool bFlag); // 14 (+0x38) 004d6f80: zera o estado, cria o FxInterpreter e põe o movimento
	virtual void Process(float dt); // 15 (+0x3c) 004d70c0: anda o movimento e acerta a luz pela sombra na cabeça
	virtual void UpdateTexture983(); // 16 (+0x40) 004d5ed0: aplica a última textura trocada (LobbyPlayer::OnDisplay 00566342)

	void SetPosition(const WVector& pos); // 004d55c0
	void LookAt983(WVector pos); // 004d56c0 (pódio 004da91b)
	void RefreshFx983(bool bAll, bool bFlag, int a, int b); // 004d5fa0 (o último argumento não é lido)
	void Play983(const char* name); // 004d6f30 (pódio 004da74a)
	void ChangeTexture983(const char* texture); // 004d8bf0
	bool IsName983(const char* name); // 004d7230
	// 004d80f0: peças padrão do personagem m_charTid (roteiro em edx; NULL = abre "grandprix_ceremony.lua" só para isto)
	static bool ReadDefault983(IDummyPlayer* pPlayer, lua_system::lua_script* pScript, unsigned long* tids, unsigned long* uids);
	// carregadores estáticos (/Gr: boneco em ecx, 2º argumento em edx)
	static bool Load983(IDummyPlayer* pPlayer, unsigned long* tids, unsigned long* buildUids, unsigned long* uids, int a, int b,
		bool bNoScene, int bFull); // 004d7c60 (`ret 0x18`)
	static bool Build983(IDummyPlayer* pPlayer, const CPartTidList* tids, unsigned long* uids, bool bNoScene); // 004d7fb0

	friend class LobbyPlayer; // chama o SetMotionData983

protected:
	void ParsePuppetScript(char* script, bool bSkip); // 004d4940
	void ScriptProcess(w_motion_data* motion, float from, float to, bool bAll, bool bSkip); // 004d6040
	bool SetMotionData983(const char* name); // 004d6aa0

public:
	std::tr1::shared_ptr<CPetFrame> m_pFrame; // +0x08
	int m_kind; // +0x10 (LobbyPlayer cria com 2, o pódio com 1)
	std::string m_name; // +0x14 (nome do elemento na cena)
	WMatrix m_matrix; // +0x30
	float m_60; // +0x60: pi ao pôr o movimento
	LightSet m_light; // +0x64
	WVector m_pos; // +0x80
	std::vector<int> m_8c; // +0x8c (tipo do elemento NÃO conferido)
	std::vector<int> m_9c; // +0x9c (idem)
	unsigned long m_charTid; // +0xac: lido pelo carregador 004d7c60 (`| 0x4000000`)
	w_motion_data* m_pMotion; // +0xb0
	float m_motionTime; // +0xb4
	std::list<sTexturePart*> m_texList; // +0xb8
	sTexturePart* m_pTexPart; // +0xc4: a última textura trocada
	sTexturePart* m_pTexApplied; // +0xc8: a última aplicada pelo UpdateTexture983
	bool m_bShow; // +0xcc
	bool m_bPlaying; // +0xcd: movimento andando
	bool m_ce; // +0xce: 2º argumento do SetMotion (vai ao RefreshFx983); 1 no construtor
	bool m_bInit; // +0xcf
	FxInterpreter* m_pFx; // +0xd0
	int m_fxId; // +0xd4
	int m_d8[2]; // nenhuma função escrita lê
	bool m_e0; // +0xe0: gravado pelo carregador 004d7c60 (1 = lista de peças 00661fb0), lido pelo OnScriptPTex
};
