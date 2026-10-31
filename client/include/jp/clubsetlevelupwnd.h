#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetLevelUpWnd (unidade 004ea7a0–004ec7b0, e a leitura do Lua 004ec7b0 na unidade seguinte; vtables 00d052d4 /
// 00d052cc; s_pInstance 00e74dc0; sizeof 0xd8 pelo `operator new` de 004eb1c0). Aba "subir nível" da FrClubSetWorkshopMainDlg:
// escolhe o taco (FrClubSetWorkshopSelector), mostra o item de subir nível e até 5 cartas de reforço, confirma (0x164).
// Layout medido no construtor 004ea900 (sessão B79). Nomes dos métodos e dos campos são meus.
#include <vector>
#include "clubsetworkshopchildwndbase.h"

class FrForm;
class FrArea;
class WReceivedPacket;
class FrClubSetInfoWnd;

class FrClubSetLevelUpWnd : public ClubSetWorkshopChildWndBase, public ClubSetWorkshopHelper
{
public:
	static FrClubSetLevelUpWnd* s_pInstance; // 00e74dc0
	// criador: FrClubSetWorkshopMainDlg::CreateLevelUp983 (004eb1c0)

	FrClubSetLevelUpWnd(ClubSetWorkShopForm* pForm); // 004ea900
	virtual ~FrClubSetLevelUpWnd(); // 004eaa10 (com `delete`: 004eb230; ajuste do Helper 004eaa00)

	virtual void SetShow983(bool bShow); // 004ec760 (+4)
	virtual void OnBeginWork983(); // 004eac10 (+8)
	virtual void OnEndWork983(); // 004ebe00 (+0xc)
	virtual void OnDrawNpcImg983(); // 004eae40 (+0x14)
	virtual void OnDrawLogo983(); // 004eaeb0 (+0x1c)
	virtual bool Init983(); // 004eb2f0 (+0x20): chamado pelo OnInit da MainDlg
	virtual void Proc983(float deltaTime); // 004ea7a0 (+0x24): chamado pelo OnProc da MainDlg
	void LoadLayout983(); // 004ec7b0 (unidade própria, clubsetlevelupwndlayout.cpp, B80): clubset_workshop.lua

	// controles do mapa de mensagens da MainDlg (repassados por ela)
	void OnDrawClubinfo983(); // 004ebf20
	void OnDrawItem983(); // 004ebff0
	void OnConfirmBtnUp983(); // 004eaf20
	void OnClubselectBtnUp983(); // 004eb260
	void OnDrawItemNum983(); // 004ec160
	void OnCardAddBtnUp983(); // 004ec290
	void OnCardSubBtnUp983(); // 004ec2c0
	void OnDrawCardNum983(); // 004ec2f0
	void OnDrawNpcSpeech983(); // 004ec410
	bool OnClubSelected983(int result, FrForm* pForm); // 004ec690
	bool OnConfirmResult983(int result, FrForm* pForm); // 004ec4f0

	// respostas do servidor (chamadas pelo 009c20a0 com o s_pInstance; NULL = falhou)
	void OnLevelUpStart983(WReceivedPacket* pPacket); // 004ea850
	void OnLevelUpResult983(WReceivedPacket* pPacket); // 004eb130
	void OnLevelUpCancel983(WReceivedPacket* pPacket); // 004ec6d0

	// B85: a FrClubSetWorkshopProgressWnd e o quadro do resultado leem m_pInfoWnd / m_selStat e chamam SelectClub983
	friend class FrClubSetWorkshopProgressWnd;
	friend class FrClubSetWorkshopProgressResultWnd;

protected:
	void UpdateUseItem983(); // 004ea7d0: carta (se há) ou item de subir nível
	void ResetInfo983(unsigned long guid, int mode); // 004ea820 (os argumentos não são usados)
	void UpdateItemIcon983(); // 004eaad0
	void UpdateCards983(); // 004eb830
	void UpdateConfirm983(); // 004ebbd0: botão confirmar e o motivo no balão
	void SelectClub983(unsigned long guid); // 004ec600

	std::vector<unsigned long> m_v78; // +0x78 (tipo a conferir; a LoadLayout983 enche o m_typeIds do Helper, +0x2c, não este)
	FrClubSetInfoWnd* m_pInfoWnd; // +0x88
	const char* m_itemBgName; // +0x8c: figura do fundo "area_item_levelup" (item_bg_filename)
	int m_confirmState; // +0x90: −1 pode confirmar; 0 sem taco; 1 sem item; 2 no máximo
	FrArea* m_pNpcImg; // +0x94: "area_npc_img1"
	FrArea* m_pLogo; // +0x98: "area_logo_levelup"
	int m_itemIconOverlay; // +0x9c (índice no Helper)
	int m_cardIconOverlay; // +0xa0
	float m_itemIconX; // +0xa4 (item_select_bg_pos[1] do Lua, B80)
	float m_itemIconY; // +0xa8
	float m_cardIconX; // +0xac (card_select_bg_pos[1])
	float m_cardIconY; // +0xb0
	unsigned long m_clubGuid; // +0xb4: taco escolhido
	unsigned long m_clubRank; // +0xb8
	unsigned long m_useTypeId; // +0xbc: item ou carta usado (0x164)
	int m_useCount; // +0xc0
	int m_unusedc4983; // +0xc4: só o construtor zera (004ea9cb)
	int m_itemCount; // +0xc8: quantos itens de subir nível o jogador tem
	int m_cardCount; // +0xcc: cartas postas (0…5)
	int m_cardMax; // +0xd0: cartas que o jogador tem
	bool m_bSending; // +0xd4: 0x164 enviado, à espera da resposta
};
C_ASSERT_VC10(sizeof(FrClubSetLevelUpWnd) == 0xd8);
