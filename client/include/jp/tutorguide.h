#pragma once
// JP 983: CTutorGuide (unidade 006b1ee0–006b2bf0; vtables 00d19e14 / 00d19e00 (CRenderFuncPtr); RTTI 00e7904c; `new` de
// 0x710 em 006b2b40). Classe nova do 983 (existe no KR 839, 006b1eb0…): até quatro marcas do tutorial presas a pontos do
// campo — um indicador ("tutor_Indicator_NN.png") que balança em cima de uma marca de jogador ("player_mark_NN.png"),
// desenhados como quadrado projetado (ou como WOverlay no modo 0x200 do jogo). Mensagens 0x310 (põe a marca N num ponto),
// 0x311 (apaga todas), 0x1ce (mostra), 0x97 (zera o relógio das marcas) e 0x30f (manda o pacote 0x12e com o ponto).
// O corte da F3.7.1 começava em 006b1f60: as quatro primeiras funções (006b1ee0–006b1f50) são da marca. Sessão B55; nomes meus.

#include "actor.h"
#include "scenemanager.h"
#include "wvideo.h"

class WOverlay;

// 0x1bc bytes; nome meu
struct sTutorMark
{
	sTutorMark() { Init(); } // cópia fora de linha 006b28e0 (construtor do vetor)
	~sTutorMark() { Release(); } // 006b20c0 (salto para a Release)

	void Release(); // 006b1ee0
	void ResetTime(); // 006b1f30
	void Show(); // 006b1f40
	void Hide(); // 006b1f50
	void Load(int index); // 006b20d0
	void Reset(); // 006b21e0
	void Set(const WVector& pos, int index); // 006b2210
	void UpdateScreenPos(); // 006b2240
	void Draw(int hTex, const WRect* rc, unsigned long color); // 006b2290
	void Init(); // 006b2500
	void Update(float delta); // 006b2540
	void DrawIndicator(); // 006b25e0
	void DrawMark(); // 006b2650
	void Display(); // 006b28f0

	WVector m_pos; // +0x00
	int m_hIndicator; // +0x0c
	int m_hMark; // +0x10
	bool m_bShowIndicator; // +0x14
	bool m_bShowMark; // +0x15
	WOverlay* m_pIndicator; // +0x18
	WRect m_rcIndicator; // +0x1c
	WOverlay* m_pMark; // +0x2c
	WRect m_rcMark; // +0x30
	WTVertex* m_pVtx[5]; // +0x40 leque 0, 1, 3, 2 e o fim
	WTVertex m_vtx[4]; // +0x54
	float m_time; // +0x1b4
	bool m_bActive; // +0x1b8
};

class CTutorGuide : public IActor, public CRenderFuncPtr
{
	friend IObject* CTutorGuideMakeInstance();

public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 006b29f0

	enum { MAX_MARK = 4 };

	CTutorGuide(); // 006b2960
	virtual ~CTutorGuide(); // 006b2a00 (com `delete`: 006b2a80)

	virtual void OnPreLoadInit(); // 006b2490
	virtual void OnLoad(); // 006b24c0
	virtual void OnInit(); // 006b1f60
	virtual void OnProcess(float delta); // 006b26e0
	virtual void OnDisplay(); // 006b1fa0
	virtual void OnDestroy(); // 006b1f70
	virtual void Display(); // 006b2b10

protected:
	virtual void HandleMsg(const MsgObject& msg); // 006b2740

private:
	void ResetTimeAll(); // 006b1fb0
	void ShowAll(); // 006b1fd0
	void HideAll(); // 006b1ff0 (sem chamador no 983)
	bool IsBefore(float t); // 006b2010
	void ResetAll(); // 006b24e0
	void InitAll(); // 006b28c0

	sTutorMark m_mark[MAX_MARK]; // +0x1c
	float m_time; // +0x70c tempo desde o último pacote 0x12e
};

C_ASSERT(sizeof(sTutorMark) == 0x1bc);
C_ASSERT(sizeof(CTutorGuide) == 0x710);
