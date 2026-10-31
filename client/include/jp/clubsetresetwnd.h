#pragma once
#include "vc10_layout.h"
// JP 983: inicialização (reset) do taco na oficina — unidade 00499800–0049c430 (src/projectg/jp/clubsetresetwnd.cpp). Existe no 839
// (par 0049d120…), não no 645. Fluxo: o meu quarto (CRealMyRoomMain 0061aae0) cria a FrClubSetResetWnd e chama Start983 com o
// item de reset (0x1a000247 / 0x1a00024b) → escolha do taco ("selectclubset") → pergunta ("clubsetwork_okcancel") → andamento
// ("newRandomBox2") → pacote 0x16d → resposta do servidor (CTask::OnPacketCommon 009c20a0) → resultado ("clubsetwork_ok").
// Os resultados dos diálogos voltam por métodos do CRealMyRoomMain (0060ac10 / 0060ac30 / 0060ac50 / 00605eb0, frente A) que
// repassam ao s_pInstance. Nomes dos métodos e dos campos são meus (sessão B89).
#include <vector>
#include <string>
#include "clubsetworkshopform.h"
#include "frrandomboxprogress.h"

class CClubSetInfoFont;
class FrArea;
class FrCmdTarget;
class WReceivedPacket;

// controlador (não é janela; vtable 00d006a8 só com o destrutor; sizeof 0x24 pelo `operator new` de 0049a230)
class FrClubSetResetWnd
{
public:
	static FrClubSetResetWnd* s_pInstance; // 00e74254 (a frente A chama de g_pMyRoomPopup983)
	static FrCmdTarget* s_pOwner; // 00e74258: dono dos diálogos, guardado pela Create983 e lido pelo construtor

	// 0049a230 (chamada em 0061acb4)
	static FrClubSetResetWnd* __fastcall Create983(FrCmdTarget* pOwner);

	FrClubSetResetWnd(); // 00499eb0
	// 0049a250 (com `delete`: 0049a680): avisa o ator principal (0x4a do 983) do taco escolhido
	virtual ~FrClubSetResetWnd();

	// 0049bee0 (chamada em 0061aca4 / 0061acc4): sem taco abre a escolha; com taco, direto a pergunta
	void Start983(unsigned long itemTid, unsigned long clubGuid);
	bool OnSelectResult983(int result, FrForm* pForm); // 0049bc00 (por 0060ac10)
	bool OnQuestionResult983(int result, FrForm* pForm); // 0049c380 (por 0060ac30)
	bool OnProgressResult983(int result, FrForm* pForm); // 00499ac0 (por 0060ac50): manda o pacote 0x16d
	bool OnCloseQuestion983(int result, FrForm* pForm); // 00499ef0 (por 0060ac70, sem chamador no 983)
	// 0049bc40 (CTask::OnPacketCommon 009d5a86 / 009d5aa4): resposta do servidor; NULL = falhou
	void OnResetResult983(WReceivedPacket* pPacket);
	// 00498b60 (unidade da FrClubSetInfoDlg; CTask::OnPacketCommon 009d5a71 / 009d5a90 testa antes de usar o s_pInstance)
	static bool IsOpen983();
	int GetState983() const { return m_state; } // a FrClubSetInfoDlg lê o +0x20 (00498c39)

protected:
	void Clear983(); // 00499a60
	bool SetClub983(unsigned long clubGuid); // 00499a80
	void OpenSelector983(); // 0049b550
	void OpenQuestion983(); // 0049bb50

	FrCmdTarget* m_pOwner; // +0x4
	unsigned long m_itemTid; // +0x8: item de reset
	unsigned long m_clubGuid; // +0xc
	unsigned long m_clubTid; // +0x10
	int m_level; // +0x14: passo (nível) do taco antes do reset
	int m_rank; // +0x18: grau (rank) antes do reset
	bool m_bDirect; // +0x1c: Start983 já veio com o taco (não volta à escolha)
	int m_state; // +0x20: 0 nada, 1 escolha, 2 pergunta, 3 andamento, 4 resultado
};
C_ASSERT(sizeof(FrClubSetResetWnd) == 0x24);

// base da pergunta e do resultado: o taco antes → depois, com o nível e o rank desenhados sobre as figuras
// (vtables 00d00728…; RTTI 00e74264; sizeof 0x180 pelo `operator new` de 0049afe0)
class FrClubSetResetDlgBase : public ClubSetWorkShopForm
{
	DECLARE_OBJECT(FrClubSetResetDlgBase) // GetRTTI 0049ab00; MakeInstance 0049afe0; registro 00cc8ba0

public:
	FrClubSetResetDlgBase(); // 0049aa30
	virtual ~FrClubSetResetDlgBase(); // 0049ab30 (com `delete`: 0049b1b0; ajustes 0049ab10 / 0049ab20)

	virtual void DrawBitmaps983(); // 0049bcf0

protected:
	void LoadLayout983(); // 0049a6b0: clubset_workshop.lua, tabela "clubset_workshop_ui_selector"

	BitmapInfoPtr m_before; // +0x140: figura do taco antes
	BitmapInfoPtr m_after; // +0x148: figura do taco depois
	CClubSetInfoFont* m_pFont[2]; // +0x150: antes, depois
	float m_levelPos[2]; // +0x158: "level_img_pos"
	float m_rankPos[2]; // +0x160: "rank_img_pos"
	int m_level[2]; // +0x168: antes, depois
	int m_rank[2]; // +0x170: antes, depois (5 = S: sem nível)
	unsigned char m_pad983_178[8]; // 983 sem acesso (0049aa30..0049c380)
};
C_ASSERT_VC10(sizeof(FrClubSetResetDlgBase) == 0x180);

// "clubsetwork_okcancel" (RTTI 00e74270; s_pInstance 00e7425c)
class FrClubSetResetQuestionDlg : public FrClubSetResetDlgBase
{
	DECLARE_OBJECT(FrClubSetResetQuestionDlg) // GetRTTI 0049ad50; MakeInstance 0049b070; registro 00cc8be0

public:
	static FrClubSetResetQuestionDlg* s_pInstance; // 00e7425c

	FrClubSetResetQuestionDlg() { s_pInstance = this; } // 0049ad20
	virtual ~FrClubSetResetQuestionDlg() { s_pInstance = NULL; } // com `delete`: 0049adb0 (ajustes 0049ad60 / 0049ad70)

	// 0049b530: CreateForm<FrClubSetResetQuestionDlg>(pManager, pCmdDest, "clubsetwork_okcancel", NULL) (0049b250)
	static FrClubSetResetQuestionDlg* __fastcall Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest);
	// 004997c0 / 004997d0 (unidade da FrClubSetInfoDlg; dois corpos iguais, sem chamador no 983; nomes meus)
	static FrClubSetResetQuestionDlg* GetInstance983();
	static FrClubSetResetQuestionDlg* GetInstanceB983();

	// 0049b5e0: taco do jogador (guid) × o mesmo taco do .iff (tid); texto pelo item de reset
	bool SetClub983(unsigned long clubGuid, unsigned long clubTid, unsigned long itemTid);

protected:
	virtual void OnOK(); // 00499ae0: só repassa
	virtual FrWnd* OnCtrlCreated983(IObject* pObj); // 00499af0: AddCtrlByType983
};

// "clubsetwork_ok" (RTTI 00e74278; s_pInstance 00e74260)
class FrClubSetResetResultDlg : public FrClubSetResetDlgBase
{
	DECLARE_OBJECT(FrClubSetResetResultDlg) // GetRTTI 0049ae20; MakeInstance 0049b110; registro 00cc8c20

public:
	static FrClubSetResetResultDlg* s_pInstance; // 00e74260

	FrClubSetResetResultDlg() { s_pInstance = this; } // 0049adf0
	virtual ~FrClubSetResetResultDlg() { s_pInstance = NULL; } // com `delete`: 0049ae80 (ajustes 0049ae30 / 0049ae40)

	// 0049b540: CreateForm<FrClubSetResetResultDlg>(pManager, pCmdDest, "clubsetwork_ok", NULL) (0049b340)
	static FrClubSetResetResultDlg* __fastcall Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest);

	// 0049b830: nível/rank de antes (guardados pela FrClubSetResetWnd) × os do .iff (depois do reset)
	bool SetResult983(int level, int rank, unsigned long clubTid);

protected:
	virtual FrWnd* OnCtrlCreated983(IObject* pObj); // 00499b00: AddCtrlByType983
};

// andamento "newRandomBox2" (vtables 00d00c6c…; sem RTTI própria: GetRTTI 0049bf70 devolve a da FrRandomBoxProgress 00e7df34;
// sizeof 0x150 pelo `operator new` de 0049c020). A base do 983 tem 0x130 com o "image" (FrArea, +0x118) que esta anima; a da
// frente A ainda está no layout do 645 (pedido B89 em out/PEDIDOS.md): até ela trocar, a figura não anda.
#ifdef FRRANDOMBOXPROGRESS_983
#define RESETPROGRESS_IMAGE983 m_pImage983
#else
#define RESETPROGRESS_IMAGE983 ((FrArea*)NULL)
#endif
class FrClubSetResetProgress : public FrRandomBoxProgress
{
public:
	FrClubSetResetProgress(); // 0049bf20
	virtual ~FrClubSetResetProgress(); // 0049bfa0 (com `delete`: 0049c0f0; ajustes 0049bf80 / 0049bf90)

	// 0049c120: texto pelo nome do item e as duas figuras da animação do item de reset
	void SetItem983(unsigned long itemTid);

protected:
	virtual bool OnInit(); // 00499b10: 3 segundos
	virtual void OnProc(const float deltaTime); // 0049a2f0: barra e figuras a cada meio segundo

	unsigned long m_itemTid; // +0x130
	std::vector<std::string> m_anims; // +0x134
	int m_animIndex; // +0x144
	float m_animTime; // +0x148
	unsigned char m_pad983_14c[4]; // 983 sem acesso (+0x14c da unidade = m_after da DlgBase)
};
#ifdef FRRANDOMBOXPROGRESS_983
C_ASSERT_VC10(sizeof(FrClubSetResetProgress) == 0x150);
#endif
