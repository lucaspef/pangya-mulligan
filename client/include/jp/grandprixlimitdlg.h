#pragma once
// JP 983: unidade 0059e260–0059f126 (sessão B65): FrRoomListSelector (escolha entre o lobby normal e o do Grand Prix; vtables
// 00d0cb6c / 00d0cb60 / 00d0cb58, sizeof 0x130 pelo `new` de 0059e440) e FrGrandPrixLimitDlg (aviso "por que não posso entrar",
// janela "gplimitdlg" aberta pelo CGrandPrixHandler; vtables 00d0cc6c / 00d0cc60 / 00d0cc58, sizeof 0x130 pelo `new` de 0059e6c0).
// Nomes dos campos e dos métodos são meus.

#include "frform.h"
#include "grandprix983.h"

class FrButton;
class FrArea;
class FrEdit;
class FrStatic;
class Bitmap;

class FrRoomListSelector : public FrForm
{
	DECLARE_OBJECT(FrRoomListSelector)

public:
	FrRoomListSelector(); // 0059e270
	virtual ~FrRoomListSelector() { } // 0059e2b0 (0059e4d0 com delete)

protected:
	void OnNormalLobbyInit(int param); // 0059e500
	void OnNormalLobby(); // 0059e9f0
	void OnGrandPrixLobbyInit(int param); // 0059e570
	void OnGrandPrixLobby(); // 0059efd0
	void OnExpl(); // 0059e2d0: troca as imagens entre "start_img_*" e "start_expl_*"
	void OnImgMultiInit(int param); // 0059e5e0
	void OnImgGpInit(int param); // 0059e650

	FrButton* m_pNormalLobby; // +0x110
	FrButton* m_pGrandPrixLobby; // +0x114
	FrArea* m_pImgMulti; // +0x118
	FrArea* m_pImgGp; // +0x11c
	bool m_bExpl; // +0x120: mostrando as imagens (true) ou as explicações
	char m_121[0x130 - 0x121]; // não iniciado no construtor; uso não visto

	DECLARE_FRESH_MSGMAP()
};

class FrGrandPrixLimitDlg : public FrForm
{
	DECLARE_OBJECT(FrGrandPrixLimitDlg)

public:
	FrGrandPrixLimitDlg(); // 0059e360
	virtual ~FrGrandPrixLimitDlg() { } // 0059e3b0 (0059e750 com delete)

	void Init983(sGPInfo983* pInfo, unsigned long deny); // 0059e3d0 (o handler passa pInfo e flags da linha)

protected:
	virtual bool OnInit(); // 0059ec80

	void OnDetailInit(int param); // 0059e780
	void OnOk(); // 0059e3f0
	void OnTitleInit(int param); // 0059e7f0
	void OnConditionInit(int param); // 0059e860
	void OnConditionDraw(); // 0059e8e0: selo "gp_limited" quando algum motivo de nível/tempo pegou
	void OnConditionTxtInit(int param); // 0059e980

	FrEdit* m_pDetail; // +0x110
	FrEdit* m_pConditionTxt; // +0x114
	FrStatic* m_pTitle; // +0x118
	FrArea* m_pCondition; // +0x11c
	const Bitmap* m_pLimitedBmp; // +0x120
	sGPInfo983* m_pInfo; // +0x124
	union
	{
		unsigned long m_deny; // +0x128
		sGPDeny983 m_denyBits;
	};
	bool m_bLimited; // +0x12c

	DECLARE_FRESH_MSGMAP()
};
