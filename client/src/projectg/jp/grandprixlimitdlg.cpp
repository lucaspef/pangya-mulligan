// JP 983: unidade 0059e260–0059f126 (FrRoomListSelector + FrGrandPrixLimitDlg). Fonte reconstruída do 983 (classes novas, sem par no
// 645). Sessão B65: construtores, destrutores, registros, os dois mapas de mensagens (7 + 6 entradas: 00e0f308 / 00e0f408, ponteiros
// gravados pelos inicializadores 00cd13d0 / 00cd1500), OnInit e os 13 métodos.
#include "minatl.h"
#include "projectg.h"
#include "grandprixlimitdlg.h"
#include "shareddocjp.h"
#include "gamestringmanager.h"
#include "taskmanager.h"
#include "lobbytask.h"
#include "packet.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frstatic.h"
#include "bitmap.h"
#include "../../shared/itemmanager.h"
#include <vector>

extern Fresh* g_pFresh;

// JP 983: empréstimos (pedido B → A da B65; nomes meus)
// 00bfb1d0: o `FrEdit::AddText` do 983 tem 5 argumentos (fredit.h da A declara 3)
class FrEditJp983 : public FrEdit
{
public:
	// A312: inline -> FrEdit::AddText de 5 argumentos (fredit.h); antes sem corpo = stub no exe (texto nunca aparecia)
	void AddText983(const char* text, bool a, bool b, unsigned long c, unsigned long d) { AddText(text, a, b, c, d); }
};
#define EDIT983(p) ((FrEditJp983*)(p))
// 00a17b90: lista de `GrandPrixConditionEquip.iff` do Grand Prix (mapa CItemManager+0x514); NULL se não há
class CItemManagerGpJp
{
public:
	std::vector<IFF983::sGrandPrixConditionEquip>* FindGrandPrixConditionEquip983(unsigned long tid);
};
#define ITEMMANAGERGP983() ((CItemManagerGpJp*)&Doc()->m_itemManager)

#include "levelnametable.h"

static CLevelNameTable983 s_levelNames; // 00e75f0c

// 0059ebd0: objeto vazio criado na pilha só para chamar o método (`lea ecx, [ebp-1]`). Nome meu.
struct sGrandPrixLobbyRequest983
{
	void Send(); // pacote 0x176 + mensagem 1 ao ator principal
};

void sGrandPrixLobbyRequest983::Send()
{
	WSendPacket packet((enumClientPacket)0x176);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// ------------------------------------------------------------------------------------------------------------------------
// FrRoomListSelector

IMPLEMENT_OBJECT(FrRoomListSelector, FrForm)

FrRoomListSelector::FrRoomListSelector()
	: m_bExpl(false)
{
}

BEGIN_FRESH_MSGMAP(FrRoomListSelector, FrForm)

ON_FRESH_VI("normallobby", FRCMD_INIT, FrRoomListSelector::OnNormalLobbyInit)
ON_FRESH_VV("normallobby", FRCMD_LBUTTONUP, FrRoomListSelector::OnNormalLobby)
ON_FRESH_VI("grandprixlobby", FRCMD_INIT, FrRoomListSelector::OnGrandPrixLobbyInit)
ON_FRESH_VV("grandprixlobby", FRCMD_LBUTTONUP, FrRoomListSelector::OnGrandPrixLobby)
ON_FRESH_VV("expl", FRCMD_LBUTTONUP, FrRoomListSelector::OnExpl)
ON_FRESH_VI("img_multi", FRCMD_INIT, FrRoomListSelector::OnImgMultiInit)
ON_FRESH_VI("img_gp", FRCMD_INIT, FrRoomListSelector::OnImgGpInit)

END_FRESH_MSGMAP()

void FrRoomListSelector::OnExpl()
{
	if (m_bExpl)
	{
		m_pImgMulti->SetBgImg(g_pFresh->GetBitmap("start_img_multi"));
		m_pImgGp->SetBgImg(g_pFresh->GetBitmap("start_img_grandprix"));
	}
	else
	{
		m_pImgMulti->SetBgImg(g_pFresh->GetBitmap("start_expl_multi"));
		m_pImgGp->SetBgImg(g_pFresh->GetBitmap("start_expl_grandprix"));
	}
	m_bExpl = m_bExpl == false;
}

void FrRoomListSelector::OnNormalLobbyInit(int param)
{
	m_pNormalLobby = DYNAMIC_CAST(FrButton, param);
}

void FrRoomListSelector::OnGrandPrixLobbyInit(int param)
{
	m_pGrandPrixLobby = DYNAMIC_CAST(FrButton, param);
}

void FrRoomListSelector::OnImgMultiInit(int param)
{
	m_pImgMulti = DYNAMIC_CAST(FrArea, param);
}

void FrRoomListSelector::OnImgGpInit(int param)
{
	m_pImgGp = DYNAMIC_CAST(FrArea, param);
}

void FrRoomListSelector::OnNormalLobby()
{
	// 983: lê +0x10b28 = m_bReturnLobbyNotice (o byte novo +0x10b27 do CSharedDoc, C38.1, mostrou que não é o m_bRebuildUnderBar)
	if (!Doc()->m_bReturnLobbyNotice)
	{
		WSendPacket packet((enumClientPacket)0x81);
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	}
	else if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
	{
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		AfxPostMsg(NULL, "Lobby", 0x8b, 4, 0, 0, 0);
	}
	else
	{
		MsgObject msg(NULL, 0x8b, 4, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0, (int)"TOPPAGE", 0, 0, 0, 0) << msg;
	}
	Close(FrOK, true);
}

void FrRoomListSelector::OnGrandPrixLobby()
{
	if (Doc()->m_bReturnLobbyNotice)
	{
		if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		{
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		}
		else if (strcmpi(g_pFresh->GetManager()->GetLayoutID(), "TOPPAGE") == 0 || DOCJP()->GetState983() == 2)
		{
			MsgObject msg(NULL, 0x8b, 4, 1, 0, 0, 0);
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0, (int)"TOPPAGE", 0, 0, 0, 0) << msg;
			Close(FrOK, true);
			return;
		}
	}
	sGrandPrixLobbyRequest983().Send();
	Close(FrOK, true);
}

// ------------------------------------------------------------------------------------------------------------------------
// FrGrandPrixLimitDlg

IMPLEMENT_OBJECT(FrGrandPrixLimitDlg, FrForm)

FrGrandPrixLimitDlg::FrGrandPrixLimitDlg()
	: m_pDetail(NULL), m_pLimitedBmp(NULL), m_pInfo(NULL), m_deny(0), m_bLimited(false)
{
}

BEGIN_FRESH_MSGMAP(FrGrandPrixLimitDlg, FrForm)

ON_FRESH_VI("detail", FRCMD_INIT, FrGrandPrixLimitDlg::OnDetailInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrGrandPrixLimitDlg::OnOk)
ON_FRESH_VI("title", FRCMD_INIT, FrGrandPrixLimitDlg::OnTitleInit)
ON_FRESH_VI("condition", FRCMD_INIT, FrGrandPrixLimitDlg::OnConditionInit)
ON_FRESH_VV("condition", FRCMD_OWNERDRAW, FrGrandPrixLimitDlg::OnConditionDraw)
ON_FRESH_VI("contition_txt", FRCMD_INIT, FrGrandPrixLimitDlg::OnConditionTxtInit)

END_FRESH_MSGMAP()

void FrGrandPrixLimitDlg::Init983(sGPInfo983* pInfo, unsigned long deny)
{
	m_pInfo = pInfo;
	m_deny = deny;
}

void FrGrandPrixLimitDlg::OnOk()
{
	Close(FrOK, true);
}

void FrGrandPrixLimitDlg::OnDetailInit(int param)
{
	m_pDetail = DYNAMIC_CAST(FrEdit, param);
}

void FrGrandPrixLimitDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrStatic, param);
}

void FrGrandPrixLimitDlg::OnConditionInit(int param)
{
	m_pCondition = DYNAMIC_CAST(FrArea, param);
	m_pLimitedBmp = g_pFresh->GetBitmap("gp_limited");
}

void FrGrandPrixLimitDlg::OnConditionDraw()
{
	const WRect& rc = m_pCondition->GetRect();
	if (m_pLimitedBmp && m_bLimited)
	{
		g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pLimitedBmp,
			WRect(rc.x, rc.y,
				(float)m_pLimitedBmp->Width(), (float)m_pLimitedBmp->Height()),
			0xffffffff, 0);
	}
}

void FrGrandPrixLimitDlg::OnConditionTxtInit(int param)
{
	m_pConditionTxt = DYNAMIC_CAST(FrEdit, param);
}

bool FrGrandPrixLimitDlg::OnInit()
{
	m_pDetail->SetReadOnly(true);
	m_pConditionTxt->SetReadOnly(true);
	m_bLimited = false;
	if (!m_pInfo)
		return false;

	m_pTitle->SetCaption(m_pInfo->name);
	if (m_pInfo->maxLevel > 0)
	{
		EDIT983(m_pDetail)->AddText983(MakeStr(GetGameString(0x754a),
			K2L_Compatibility(s_levelNames.Get(m_pInfo->minLevel).name),
			K2L_Compatibility(s_levelNames.Get(m_pInfo->maxLevel).name)), false, true, 0, 0);
	}
	if (m_pInfo->m_9c > 0)
	{
		if (m_pInfo->m_9c == 0xb4)
			EDIT983(m_pDetail)->AddText983(MakeStr(GetGameString(0x7556), m_pInfo->m_98), false, true, 0, 0);
		else if (m_pInfo->m_98 == 0)
			EDIT983(m_pDetail)->AddText983(MakeStr(GetGameString(0x754c), m_pInfo->m_9c), false, true, 0, 0);
		else
			EDIT983(m_pDetail)->AddText983(MakeStr(GetGameString(0x754b), m_pInfo->m_98, m_pInfo->m_9c), false, true, 0, 0);
	}
	std::vector<IFF983::sGrandPrixConditionEquip>* pEquip =
		ITEMMANAGERGP983()->FindGrandPrixConditionEquip983(m_pInfo->ticket);
	if (pEquip)
		EDIT983(m_pDetail)->AddText983((const char*)&pEquip->at(0).raw[4], false, true, 0, 0);

	sGPDeny983 deny = m_denyBits; // o 983 testa com `shr n; test 1` (não reproduzido: o nosso cl junta em `test bl, 1<<n`)
	if (deny.level0 || deny.level1)
	{
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7532), false, true, 0, 0);
		m_bLimited = true;
	}
	if (deny.b2)
	{
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7534), false, true, 0, 0);
		m_bLimited = true;
	}
	if (deny.b3)
	{
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7533), false, true, 0, 0);
		m_bLimited = true;
	}
	if (deny.b4)
	{
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7536), false, true, 0, 0);
		m_bLimited = true;
	}
	if (deny.b5)
	{
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7538), false, true, 0, 0);
		EDIT983(m_pConditionTxt)->AddText983(GetGameString(0x7550), false, true, 0, 0);
	}
	if (deny.condition)
		EDIT983(m_pDetail)->AddText983(MakeStr(GetGameString(0x7537)), false, true, 0, 0);
	if (deny.prev)
	{
		IFF983::sGrandPrixData* pPrev = Doc()->m_itemManager.FindGrandPrixData983(m_pInfo->prevTypeId);
		if (pPrev)
		{
			EDIT983(m_pConditionTxt)->AddText983(MakeStr(GetGameString(0x753f), (const char*)pPrev + 0x12), false, true,
				0, 0);
		}
	}
	return FrForm::OnInit();
}
