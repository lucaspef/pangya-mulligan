#pragma once
#include <string>
#include <vector>
#include <list>
#include "petscriptobject.h"

class WPuppet;
class FxInterpreter;
struct w_motion_data;
struct sTexturePart; // src/projectg/texturepart.h (frente A): nome 0x40, textura 0x40, Bitmap*, RECT = 0x94

// JP 983: boneco do lobby (caddie, mascote, taco, jogador). RTTI .?AVIDummyPet@@, vtable 00d046dc (17 posições: as 14 do
// IPetScriptObject + 3 próprias), sizeof 0xc4 (`push 0xc4` antes do `new` em 0056128c). Unidade 004d1190–004d4800; par no KR 839
// (004d5170…). Nomes meus, salvo os das virtuais herdadas (frente A, petscriptobject.h).
class IDummyPet : public IPetScriptObject
{
public:
	IDummyPet(int kind); // 004d37d0
	virtual ~IDummyPet(); // 004d38c0 (com `delete`: 004d3d40)
	virtual void OnScriptSnd(const char*& p); // 1  004d1be0
	virtual void OnScriptHideClub(const char*& p); // 5  004d1c80
	virtual void OnScriptFx(const char*& p, bool bSkip, int cmd); // 8  004d1190 (vazia)
	virtual void OnScriptPTex(const char*& p); // 9  004d3d70
	virtual void OnScriptShowBone(const char*& p, int cmd); // 10 004d11a0
	virtual void OnScriptPlayIf(const char*& p); // 11 004d11c0 (vazia)
	virtual void OnScriptShowVis(const char*& p, int cmd); // 13 004d11d0
	virtual bool SetMotion(const char* name); // 14 (+0x38) 004d2890: zera o estado, cria o FxInterpreter e põe o movimento
	virtual void Process(float dt); // 15 (+0x3c) 004d2980
	virtual void Display(); // 16 (+0x40) 004d2500

	// 004d3fc0 (LobbyCaddie 0056132e, 005613f6): caddie `typeId` com a roupa `skinId` (0 = sem roupa); o .pet sai do nome da
	// textura do corpo da roupa (tabela de modelos: quma, dol, pipin, cadie, ho, maid, Cien, karen)
	static bool Load(IDummyPet* pPet, unsigned long typeId, unsigned long skinId, bool bNoScene);
	static bool LoadClubUi983(IDummyPet* pPet, unsigned long id, bool bNoScene); // 004d23a0 (sem chamador)
	static bool LoadClub983(IDummyPet* pPet, unsigned long id, bool bNoScene); // 004d2450 (LobbyItems 0056311d)
	static bool LoadMascot983(IDummyPet* pPet, unsigned long id, bool bNoScene); // 004d3150 (LobbyMascot 005646da)
	void SetPosition(const WVector& pos); // 004d1e50
	void LookAt983(WVector pos); // 004d1f10 (sem chamador)
	void RefreshFx983(bool bAll, int a, int b); // 004d1200 (os dois últimos argumentos não são lidos)
	void Play983(const char* name); // 004d2850 (sem chamador)
	void ChangeTexture983(const char* name, const char* texture); // 004d3a50 ("face"/"body" do Load e o *ptex( do script)

protected:
	void ParsePuppetScript(char* script, bool bSkip); // 004d11f0
	void ScriptProcess(w_motion_data* motion, float from, float to, bool bAll, bool bSkip); // 004d1ca0
	bool SetMotionData983(const char* name); // 004d25a0
	// 004d34b0: lê "[@movimento] nome textura" do *ptex( e passa a textura pelo CGDSkinItem (o `this` não é usado)
	bool ParsePTex983(w_motion_data* motion, std::string* pName, std::string* pTexture, const char*& p);
	// IPetScriptObject (frente A): as versões que recebem o WPuppet; lá só estão declaradas as do CPetFrame
	void ScriptShowBonePet983(WPuppet* pPet, int cmd, const char*& p); // 005ac9b0
	void ScriptShowVisPet983(WPuppet* pPet, bool bShow, const char*& p); // 005acd10

public:
	WPuppet* m_pPuppet; // +0x08
	int m_kind; // +0x0c: 2 taco, 5 mascote (os outros: ver Load)
	std::string m_name; // +0x10 (nome do elemento na cena)
	WMatrix m_matrix; // +0x2c
	float m_5c; // +0x5c: pi ao pôr o movimento
	int m_hTex; // +0x60: textura da fala do mascote
	WVector m_pos; // +0x64
	std::vector<int> m_70; // +0x70 (tipo do elemento NÃO conferido; nenhuma função da unidade lê)
	std::vector<int> m_80; // +0x80 (idem)
	int m_90;
	w_motion_data* m_pMotion; // +0x94
	float m_motionTime; // +0x98
	std::list<sTexturePart*> m_texList; // +0x9c
	sTexturePart* m_pTexPart; // +0xa8: a última textura trocada
	int m_ac;
	bool m_bMoved; // +0xb0: desenha (nome da B28)
	bool m_bRotated; // +0xb1: movimento andando (nome da B28)
	bool m_bInit; // +0xb2
	char m_b3;
	FxInterpreter* m_pFx; // +0xb4
	int m_fxId; // +0xb8
	char m_bc[8];
};
