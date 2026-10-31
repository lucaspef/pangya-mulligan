#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetRankUpWnd (unidade 004c6280–004ca630; vtables 00d034f0 / 00d034e8; s_pInstance 00e74cd4; sizeof 0xd4 pelo
// `operator new` de 004c7490). Aba "subir rank" da FrClubSetWorkshopMainDlg: escolhe o taco (FrClubSetWorkshopSelector), mostra a
// experiência que falta (edit_need_exp), a carta escolhe o atributo que ganha a casa nova (static_addslot_stat), confirma (0x167).
// Layout medido no construtor 004c6810 (sessão B81). Nomes dos métodos e dos campos são meus. A LoadLayout983 (004c86e0, 5.104 B)
// está na mesma unidade (escrita na B81, como a da LevelUp).
#include <vector>
#include "clubsetworkshopchildwndbase.h"

class FrForm;
class WReceivedPacket;
class FrClubSetInfoWnd;

class FrClubSetRankUpWnd : public ClubSetWorkshopChildWndBase, public ClubSetWorkshopHelper
{
public:
	static FrClubSetRankUpWnd* s_pInstance; // 00e74cd4
	// criador: FrClubSetWorkshopMainDlg::CreateRankUp983 (004c7490)

	FrClubSetRankUpWnd(ClubSetWorkShopForm* pForm); // 004c6810
	virtual ~FrClubSetRankUpWnd(); // 004c6910 (com `delete`: 004c7500; ajuste do Helper 004c6900)

	virtual void SetShow983(bool bShow); // 004c8690 (+4)
	virtual void OnBeginWork983(); // 004c6dd0 (+8)
	virtual void OnEndWork983(); // 004c7ce0 (+0xc)
	virtual void OnDrawNpcImg983(); // 004c7e60 (+0x14)
	virtual void OnDrawLogo983(); // 004c7f70 (+0x1c)
	virtual bool Init983(); // 004c69d0 (+0x20): chamado pelo OnInit da MainDlg
	virtual void Proc983(float deltaTime); // 004c6280 (+0x24): chamado pelo OnProc da MainDlg
	void LoadLayout983(); // 004c86e0: posições do clubset_workshop.lua (B81)

	// controles do mapa de mensagens da MainDlg (repassados por ela)
	void OnDrawClubinfo983(); // 004c8080
	void OnDrawItem983(); // 004c8140
	void OnConfirmBtnUp983(); // 004c7000
	void OnClubselectBtnUp983(); // 004c7530
	void OnCardAddBtnUp983(); // 004c8270
	void OnCardSubBtnUp983(); // 004c8290
	void OnDrawCardNum983(); // 004c82b0
	void OnDrawNpcSpeech983(); // 004c83a0
	bool OnClubSelected983(int result, FrForm* pForm); // 004c8500
	bool OnConfirmResult983(int result, FrForm* pForm); // 004c7250

	// respostas do servidor (chamadas pelo 009d57b0 com o s_pInstance; NULL = falhou). Nomes provisórios.
	void OnRankUpStart983(); // 004c6470: o trabalho começou
	void OnRankUpResult983(WReceivedPacket* pPacket); // 004c64c0: novo rank e taco
	void OnRankUpPacketA983(WReceivedPacket* pPacket); // 004c7360
	void OnRankUpPacketB983(WReceivedPacket* pPacket); // 004c8540
	void OnRankUpCancel983(); // 004c8640: chamado pela FrClubSetWorkshopProgressWnd (004a3975, 004a39ce)

	// B85: a FrClubSetWorkshopProgressWnd e o quadro do resultado leem m_pInfoWnd / m_selStat e chamam SelectClub983
	friend class FrClubSetWorkshopProgressWnd;
	friend class FrClubSetWorkshopProgressResultWnd;

protected:
	void ResetInfo983(unsigned long guid, int mode); // 004c62b0 (os argumentos não são usados)
	bool IsStatMax983(int index); // 004c62f0: o atributo escolhido pela carta já está no máximo
	void UpdateNeedExp983(); // 004c75c0: "edit_need_exp"
	void UpdateCards983(); // 004c7760
	void UpdateConfirm983(); // 004c7a60: botão confirmar e o motivo no balão
	void SelectClub983(unsigned long guid); // 004c8480

	std::vector<unsigned long> m_v78; // +0x78 (como na LevelUp; tipo a conferir)
	FrClubSetInfoWnd* m_pInfoWnd; // +0x88
	const char* m_itemBgName; // +0x8c: figura do fundo "area_item_rankup"
	int m_cardIconOverlay; // +0x90 (índice no Helper)
	float m_cardIconX; // +0x94
	float m_cardIconY; // +0x98
	unsigned long m_clubGuid; // +0x9c: taco escolhido
	unsigned long m_clubRank; // +0xa0
	unsigned long m_useTypeId; // +0xa4: carta usada (0x167)
	int m_exp; // +0xa8: experiência do taco (Fx, +0x5c do slot 1)
	int m_cardCount; // +0xac: 0…4 (escolhe o atributo)
	int m_cardMax; // +0xb0: cartas que o jogador tem
	int m_needExp; // +0xb4: experiência pedida pelo ClubSetWorkShopRankUpExp
	const char* m_rankImgName[6]; // +0xb8: figura de cada rank (aviso de confirmação: rank → seta → rank + 1)
	bool m_bSending; // +0xd0: 0x167 enviado, à espera da resposta
};
C_ASSERT_VC10(sizeof(FrClubSetRankUpWnd) == 0xd4);
