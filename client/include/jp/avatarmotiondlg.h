#pragma once
// JP 983: FrAvatarMotionDlg (unidade 0047f2a0–00484010; 0047f220 ainda é da avatarbase, 00484010 já é o ctor do
// CBallDirNotifyController). Lista "motionlist" das animações (gestos) que o boneco do jogador faz no lobby ("LobbyAvatar",
// m_flags bit 1) ou na conversa de avatares ("AvatarChat", bit 0): "기본자세" (pose básica), as animações do conjunto de
// peças que o personagem veste, as "best_motion_name_char%d" do motiondlg.lua, até 14 (12 no lobby) sorteadas da tabela
// de animações do personagem e, por último, "랜덤" (sortear). A janela cresce (OnProc) até caber a lista e encolhe ao fechar.
// Existe no 839 (par 00484160…), não no 645. Sessão B111: classe inteira menos o pedaço de BGM do fim da unidade
// (004835d0–00484000, "BGMlist.lua").
// vtables 00cfecc4 / 00cfecb8 / 00cfecb0; sizeof 0x1f0 (operator new de 004821c0). Nomes dos membros meus.

#include <vector>
#include <string>
#include "frform.h"

class FrListBox;
class Bitmap;
class CPetFrame;
struct FrListItem;

// 0x3c: uma linha da lista (tipo: 0 pose básica, 1 conjunto, 2 best, 3 sorteada da tabela, 4 "랜덤")
struct sAvatarMotion983
{
	sAvatarMotion983() : type(0), name(""), motion("") { }
	sAvatarMotion983(int _type, std::string _name, std::string _motion); // 00480fb0

	int type; // +0x00
	std::string name; // +0x04: texto da lista
	std::string motion; // +0x20: nome da animação
};

// 8 B: par da tabela de animações do personagem (CSharedDoc+0x10c24): animação e texto
struct sAvatarMotionName983
{
	const char* motion;
	const char* name;
};

class FrAvatarMotionDlg : public FrForm
{
	DECLARE_OBJECT(FrAvatarMotionDlg) // GetRTTI 00481ed0; MakeInstance 004821c0; registro 00482230

public:
	FrAvatarMotionDlg(); // 00481dc0
	virtual ~FrAvatarMotionDlg(); // 00481f00 (com `delete`: 00482250; ajustes 00481ee0 / 00481ef0)

	// chamadas de fora (HandleMsg do CLobbyMain 00abc140, do AvatarChat 007acdb0, 007b1860, 00ab8430); nomes meus
	void Refresh983(const unsigned long* pTid); // 00483370: refaz a tabela, a lista e o tamanho
	void SetupMotions983(const unsigned long* pTid); // 00481960 (cópia de chamada em 00481b20)
	void SelectMotion983(const char* motion, const int* pUnused); // 00480d90: marca a linha da animação (-1 = pose básica);
	                                                              // o CLobbyMain passa &msg.param2, sem uso
	void PlayLobby983(const int* pIndex); // 00480590: manda 0x199 (ou 0x196 = parar o sorteio) ao "LobbyAvatar"
	const char* GetMotion983(int index); // 0047fd40: animação da tabela do personagem (NULL fora dela)

	enum
	{
		FLAG_CHAT = 1, // dono "AvatarChat"
		FLAG_LOBBY = 2, // dono "LobbyAvatar"
	};

protected:
	virtual bool OnInit(); // 004832a0
	virtual void OnProc(const float deltaTime); // 00480450: abre/fecha deslizando, realce do mouse, alfa do texto, ESCAPE

	void OnMotionListInit(int param); // 0047f930
	void OnMotionListOwnerDraw(int param); // 00480d70 (-> DrawItem983 00480c30)
	void OnMotionListLBDown(); // 00480e00

	void DrawItem983(FrListItem* pItem); // 00480c30
	void DrawChatMark983(FrListItem* pItem); // 0047fbf0
	void DrawLobbyMark983(FrListItem* pItem); // 0047fca0
	void DrawName983(const int* pIndex, const WPoint& pos); // 00480a60
	void DrawChatName983(const int* pIndex, const WPoint& pos); // 004802b0
	void DrawLobbyName983(const int* pIndex, const WPoint& pos); // 00480360
	void Play983(const int* pIndex); // 00480a90
	void PlayChat983(const int* pIndex); // 00480730
	void SlideOpen983(const float& deltaTime); // 0047fa50
	void SlideClose983(const float& deltaTime); // 0047fb30
	void ProcHover983(const float& deltaTime); // 0047fd70
	void InitHoverRect983(); // 0047f9b0
	void InitIconRect983(); // 0047f9f0
	void CalcTextX983(); // 0047f2b0
	void LoadBitmaps983(); // 0047f2d0
	bool LoadBestMotion983(std::string file); // 00482280
	void FindMotion983(const char* motion, sAvatarMotion983* pOut); // 00481070
	const char* GetSetPartsInfo983(); // 00480270
	void ClearLists983(); // 00482010
	void LoadBasic983(); // 004824c0
	void LoadSet983(); // 004825e0
	void LoadBest983(); // 004827b0
	void LoadRandom983(); // 00482a80
	void LoadShuffle983(); // 00482fd0
	void LoadLists983(); // 00483170
	void AppendList983(const std::vector<sAvatarMotion983>& src); // 004830f0
	void BuildList983(); // 004831a0

	const sAvatarMotion983& GetItem983(int index)
	{
		int n = (int)m_list.size();
		if (index < 0)
			n = 0;
		else if (index <= n)
			n = index;
		return m_list[n];
	}

public:
	std::vector<sAvatarMotionName983> m_allMotions; // +0x110: tabela do personagem (sem as de conjunto)
	FrListBox* m_pMotionList; // +0x120: "motionlist"
	CPetFrame* m_pFrame; // +0x124: esqueleto do boneco (resposta da msg 0x198)
	int m_motionCount; // +0x128
	int m_selected; // +0x12c: linha marcada (-1 = nenhuma)
	float m_textAlpha; // +0x130: 0 → 255
	float m_textX; // +0x134
	float m_hoverAlpha; // +0x138: 0,7 → 1 com o mouse em cima
	WRect m_hoverRect; // +0x13c
	WRect m_iconRect; // +0x14c
	const Bitmap* m_pPlayBmp; // +0x15c: "motionchat_list_play"
	const Bitmap* m_pStopBmp; // +0x160: "motionchat_list_stop"
	const Bitmap* m_pHoverBmp; // +0x164: "motionchat_list_Hover"
	bool m_bPlayed; // +0x168
	bool m_bPlaying; // +0x169: ícone play × stop (escrito pelo dono)
	bool m_b16a; // +0x16a
	bool m_bRandom; // +0x16b: sorteio ligado (endereço vai na msg 0x197)
	unsigned long m_flags; // +0x16c: FLAG_CHAT / FLAG_LOBBY
	unsigned long m_slide; // +0x170: 8 abrindo, 4 fechando, 1 aberta, 2 fechada
	std::vector<sAvatarMotion983> m_basicList; // +0x174: tipo 0
	std::vector<sAvatarMotion983> m_setList; // +0x184: tipo 1
	std::vector<sAvatarMotion983> m_bestList; // +0x194: tipo 2
	std::vector<sAvatarMotion983> m_randomList; // +0x1a4: tipo 3
	std::vector<sAvatarMotion983> m_shuffleList; // +0x1b4: tipo 4
	std::vector<sAvatarMotion983> m_list; // +0x1c4: o que a lista mostra
	std::vector<sAvatarMotion983> m_bestSrc; // +0x1d4: lidas do motiondlg.lua
	char m_pad983_1e4[0x1f0 - 0x1e4]; // +0x1e4: sem uso na unidade

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0047f2a0; mapa 00e037c8 (3 entradas, inicializador 00cc7da0)
};
