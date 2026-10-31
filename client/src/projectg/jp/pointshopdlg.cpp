// JP 983: unidade 0084b8d0–008515c0 (loja de pontos do Tiki: FrPointShopHelp, FrPointShopMessageDlg, FrPointShopNoticeDlg e
// FrPointShopDlg). O corte da F3.7.1 começava em 0084b7fe: 0084b7e0/0084b810 são da ShotReview (unidade anterior, inicializador
// 00cddec0). Existe no 839, não no 645; ver include/jp/pointshopdlg.h. Sessão B106: as três janelas pequenas e a FrPointShopDlg
// menos as listas; sessão B107: as listas (FindListItem983, CanExchange983, AddListItem983, RefreshList983, DrawListItem983, os 3
// pintores e os 2 cliques) e o DrawBitmap983.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "frviewer.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frarea.h"
#include "frscrollbar.h"
#include "commonutil.h"
#include "../../shared/itemmanager.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "titlefontex.h"
#include "pointshopdlg.h"
#include "binstr.h"
#include "createcharacteroption.h"

extern Fresh* g_pFresh; // 00f02340

// ---------------------------------------------------------------------------------------------------------------------
// FrPointShopHelp

// 0084b8d0 (GetMessageMap); mapa 00e21c10 (1 entrada)
BEGIN_FRESH_MSGMAP(FrPointShopHelp, FrForm)
ON_FRESH_VI("desc_view", FRCMD_INIT, FrPointShopHelp::OnDescViewInit983)
END_FRESH_MSGMAP()

// 0084b8e0
FrPointShopHelp::FrPointShopHelp()
	: m_pDescView(NULL)
{
}

IMPLEMENT_OBJECT(FrPointShopHelp, FrForm)

// 0084b920
FrPointShopHelp::~FrPointShopHelp()
{
}

// 0084c0a0
void FrPointShopHelp::OnDescViewInit983(int param)
{
	m_pDescView = KINDOF_CAST983(FrViewer, param);
	m_pDescView->ShowScrollBar(true);
	m_pDescView->Open("pointshop_explanation.jpg");
}

// ---------------------------------------------------------------------------------------------------------------------
// FrPointShopMessageDlg

// 0084b940 (GetMessageMap); mapa 00e21c18 (1 entrada)
BEGIN_FRESH_MSGMAP(FrPointShopMessageDlg, FrForm)
ON_FRESH_VI("background", FRCMD_INIT, FrPointShopMessageDlg::OnBackgroundInit983)
END_FRESH_MSGMAP()

IMPLEMENT_OBJECT(FrPointShopMessageDlg, FrForm)

// 0084c130
void FrPointShopMessageDlg::OnBackgroundInit983(int param)
{
	m_pBackground = KINDOF_CAST983(FrArea, param);
}

// 0084d9d0: fundo (falha ou não) e o texto em vermelho
void FrPointShopMessageDlg::OnDraw()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	WRect rc = m_rect;
	if (m_pBackground)
	{
		const Bitmap* pBmp = m_bSuccess ? m_pBgBmp : m_pFailBmp;
		if (pBmp)
			pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
	pGI->SetTextColor(0xff000000, 0xffffffff);
	pGI->SetTextStyle(2);
	pGI->SetTextColor(0xffff0000, 0xffffffff);
	pGI->Print(WPoint(rc.x + 127.0, rc.y + 116.0), 1, m_msg.c_str());
}

// 0084de70
FrPointShopMessageDlg::~FrPointShopMessageDlg()
{
	m_pBgBmp = NULL;
	m_pFailBmp = NULL;
}

// 0084e820
FrPointShopMessageDlg::FrPointShopMessageDlg()
	: m_msg("")
{
	m_bSuccess = 1;
	m_pBgBmp = g_pFresh->RegisterBitmap("tikipoint_popup_bg");
	m_pFailBmp = g_pFresh->RegisterBitmap("tikipoint_popup_fail");
}

// ---------------------------------------------------------------------------------------------------------------------
// FrPointShopNoticeDlg

// 0084b960 (GetMessageMap); mapa 00e21c20 (6 entradas)
BEGIN_FRESH_MSGMAP(FrPointShopNoticeDlg, FrForm)
ON_FRESH_VI("background", FRCMD_INIT, FrPointShopNoticeDlg::OnBackgroundInit983)
ON_FRESH_VI("tag", FRCMD_INIT, FrPointShopNoticeDlg::OnTagInit983)
ON_FRESH_VV("tag", FRCMD_OWNERDRAW, FrPointShopNoticeDlg::OnTagDraw983)
ON_FRESH_VI("desc", FRCMD_INIT, FrPointShopNoticeDlg::OnDescInit983)
ON_FRESH_VV("Apply", FRCMD_LBUTTONUP, FrPointShopNoticeDlg::OnApplyUp983)
ON_FRESH_VV("Exit", FRCMD_LBUTTONUP, FrPointShopNoticeDlg::OnExitUp983)
END_FRESH_MSGMAP()

// 0084b970
FrPointShopNoticeDlg::FrPointShopNoticeDlg()
{
	m_pShop = NULL;
	m_pTag = NULL;
	m_pDesc = NULL;
	m_pTec1Bmp = g_pFresh->RegisterBitmap("tikipoint_popup_tec1");
	m_pTec2Bmp = g_pFresh->RegisterBitmap("tikipoint_popup_tec2");
	m_pBgBmp = g_pFresh->RegisterBitmap("tikipoint_popup_bg");
}

// 0084ba30
FrPointShopNoticeDlg::~FrPointShopNoticeDlg()
{
	m_pTec1Bmp = NULL;
	m_pTec2Bmp = NULL;
	m_pBgBmp = NULL;
	m_pShop = NULL;
}

IMPLEMENT_OBJECT(FrPointShopNoticeDlg, FrForm)

// 0084c260
void FrPointShopNoticeDlg::OnBackgroundInit983(int param)
{
	m_pBackground = KINDOF_CAST983(FrArea, param);
}

// 0084c2d0
void FrPointShopNoticeDlg::OnTagInit983(int param)
{
	m_pTag = KINDOF_CAST983(FrArea, param);
}

// 0084c340
void FrPointShopNoticeDlg::OnTagDraw983()
{
	if (m_pTag && m_pShop)
	{
		const Bitmap* pBmp;
		switch (m_pShop->m_mode)
		{
		case 1: pBmp = m_pTec1Bmp; break;
		case 2: pBmp = m_pTec2Bmp; break;
		default: return;
		}
		FrGraphicInterface* pGI;
		if (pBmp && (pGI = g_pFresh->GetManager()->GetGDI()) != NULL)
		{
			WRect rc = m_pTag->GetRect();
			rc.w = (float)pBmp->Width();
			rc.h = (float)pBmp->Height();
			pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
		}
	}
}

// 0084c410
void FrPointShopNoticeDlg::OnDescInit983(int param)
{
	m_pDesc = KINDOF_CAST983(FrEdit, param);
}

// 0084dac0: na compra, sem TP bastante, fecha e avisa (mensagem 0x2e1); senão manda o pacote e avisa o CTaskMain (mensagem 1)
void FrPointShopNoticeDlg::OnApplyUp983()
{
	if (m_pShop)
	{
		if (m_pShop->m_mode == 2 && m_pShop->GetTotal983() > *(int*)&Doc()->m_pointShopTP983)
		{
			Close(true);
			// "구매에 필요한 포인트가 부족합니다." (faltam pontos para a compra)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e1,
				(int)K2L_Compatibility("\xb1\xb8\xb8\xc5\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xf7\xc0\xce\xc6\xae\xb0\xa1 "
					"\xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
			return;
		}
		m_pShop->Send983();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	}
	Close(true);
}

// 0084fbd0: troca ("%s(%d) x %d" por item, centrado na altura, + total e aviso) ou compra (ícone + "%s(%d) : %dTP" + total)
void FrPointShopNoticeDlg::OnDraw()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	WRect rc = m_rect;
	if (m_pBackground && m_pBgBmp)
		pGI->DrawTexture(m_pBgBmp, rc, 0xffffffff, 0);
	if (!m_pShop)
		return;
	switch (m_pShop->m_mode)
	{
	case 1:
		{
			pGI->SetTextColor(0xffa34706, 0xffffffff);
			pGI->SetTextStyle(1);
			CPointShop::ItemMap items = m_pShop->GetItems983();
			float y = (float)(5 - items.size()) * 6.0 + 75.0;
			for (CPointShop::ItemMap::iterator it = items.begin(); it != items.end(); ++it)
			{
				CPointShop::sItem item = it->second;
				const char* name = ItemManager()->GetItemName(item.typeId);
				int unit = m_pShop->GetUnitCount983(item.typeId);
				m_pShop->GetPoint983(item.typeId);
				pGI->Print(WPoint(rc.x + 127.0, rc.y + y), 1, MakeStr("%s(%d) x %d", name, unit, item.count));
				y += 15.0;
			}
			pGI->SetTextColor(0xff8a19a5, 0xffffffff);
			pGI->SetTextStyle(2);
			// "총 %d포인트로 교환 하시겠습니까?" (trocar por %d pontos no total?)
			pGI->Print(WPoint(rc.x + 127.0, rc.y + 180.0), 1,
				MakeStr(K2L_Compatibility("\xc3\xd1 %d\xc6\xf7\xc0\xce\xc6\xae\xb7\xce \xb1\xb3\xc8\xaf \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					m_pShop->GetTotal983()));
			pGI->SetTextColor(0xffa34706, 0xffffffff);
			pGI->SetTextStyle(0);
			// "[주의] 교환한 아이템은 복원할 수 없습니다." (o item trocado não volta)
			pGI->Print(WPoint(rc.x + 127.0, rc.y + 230.0), 1,
				K2L_Compatibility("[\xc1\xd6\xc0\xc7] \xb1\xb3\xc8\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xba\xb9\xbf\xf8\xc7\xd2 \xbc\xf6 "
					"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		}
		break;
	case 2:
		{
			pGI->SetTextColor(0xffa34706, 0xffffffff);
			pGI->SetTextStyle(1);
			CPointShop::ItemMap items = m_pShop->GetItems983();
			for (CPointShop::ItemMap::iterator it = items.begin(); it != items.end(); ++it)
			{
				CPointShop::sItem item = it->second;
				const char* name = ItemManager()->GetItemName(item.typeId);
				IFF983::sPointShop* pShop = ItemManager()->FindPointShop983(item.typeId);
				if (pShop)
				{
					IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(item.typeId);
					if (pItem)
					{
						const Bitmap* pIcon = g_pFresh->GetBitmap(pItem->Icon);
						if (pIcon)
							pGI->DrawTexture(pIcon, WRect(rc.x + 95.0, rc.y + 60.0, (float)pIcon->Width(), (float)pIcon->Height()),
								0xffffffff, 0);
						pGI->Print(WPoint(rc.x + 127.0, rc.y + 120.0), 1,
							MakeStr("%s(%d) : %dTP", name, *(int*)&pShop->raw[4], *(int*)&pShop->raw[0]));
					}
				}
			}
			pGI->SetTextColor(0xff8a19a5, 0xffffffff);
			pGI->SetTextStyle(2);
			// "구매 하시겠습니까?" (comprar?)
			pGI->Print(WPoint(rc.x + 127.0, rc.y + 180.0), 1,
				MakeStr(K2L_Compatibility("\xb1\xb8\xb8\xc5 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), m_pShop->GetTotal983()));
		}
		break;
	}
}

// 008500a0
void FrPointShopNoticeDlg::SetShop983(CPointShop* pShop)
{
	m_pShop = pShop;
	if (pShop && pShop->m_mode == 2)
	{
		CPointShop::ItemMap items = pShop->GetItems983();
		for (CPointShop::ItemMap::iterator it = items.begin(); it != items.end(); ++it)
		{
			CPointShop::sItem item = it->second;
			IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(item.typeId);
			if (pDesc && strlen(pDesc->Desc) != 0)
			{
				m_pDesc->ClearLine();
				m_pDesc->AddText(pDesc->Desc, false, true, 0, 0);
				m_pDesc->GetScrollBar()->ScrollToFirst();
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FrPointShopDlg

// 0084ba80 (GetMessageMap); mapa 00e21c28 (42 entradas)
BEGIN_FRESH_MSGMAP(FrPointShopDlg, FrForm)
ON_FRESH_VI("MyPoint", FRCMD_INIT, FrPointShopDlg::OnMyPointInit983)
ON_FRESH_VV("MyPoint", FRCMD_OWNERDRAW, FrPointShopDlg::OnMyPointDraw983)
ON_FRESH_VI("PointShop_Exit", FRCMD_INIT, FrPointShopDlg::OnExitInit983)
ON_FRESH_VV("PointShop_Exit", FRCMD_LBUTTONUP, FrPointShopDlg::OnExitUp983)
ON_FRESH_VI("PointShop_Main", FRCMD_INIT, FrPointShopDlg::OnMainInit983)
ON_FRESH_VV("PointShop_Main", FRCMD_LBUTTONUP, FrPointShopDlg::OnMainUp983)
ON_FRESH_VI("PointShop_Help", FRCMD_INIT, FrPointShopDlg::OnHelpInit983)
ON_FRESH_VV("PointShop_Help", FRCMD_LBUTTONUP, FrPointShopDlg::OnHelpUp983)
ON_FRESH_VI("GotoMainBg", FRCMD_INIT, FrPointShopDlg::OnGotoMainBgInit983)
ON_FRESH_VI("Intro_bg1", FRCMD_INIT, FrPointShopDlg::OnIntroBg1Init983)
ON_FRESH_VI("Intro_bg2", FRCMD_INIT, FrPointShopDlg::OnIntroBg2Init983)
ON_FRESH_VI("Intro_notice", FRCMD_INIT, FrPointShopDlg::OnIntroNoticeInit983)
ON_FRESH_VV("Intro_notice", FRCMD_OWNERDRAW, FrPointShopDlg::OnIntroNoticeDraw983)
ON_FRESH_VI("Exchange", FRCMD_INIT, FrPointShopDlg::OnExchangeInit983)
ON_FRESH_VV("Exchange", FRCMD_LBUTTONUP, FrPointShopDlg::OnExchangeUp983)
ON_FRESH_VI("Buy", FRCMD_INIT, FrPointShopDlg::OnBuyInit983)
ON_FRESH_VV("Buy", FRCMD_LBUTTONUP, FrPointShopDlg::OnBuyUp983)
ON_FRESH_VI("Exchange_ListBG1", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG1Init983)
ON_FRESH_VI("Exchange_ListBG2", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG2Init983)
ON_FRESH_VI("Exchange_ListBG3", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG3Init983)
ON_FRESH_VI("Exchange_ListBG4", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG4Init983)
ON_FRESH_VI("Exchange_ListBG5", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG5Init983)
ON_FRESH_VI("Exchange_ListBG6", FRCMD_INIT, FrPointShopDlg::OnExchangeListBG6Init983)
ON_FRESH_VI("PrePoint", FRCMD_INIT, FrPointShopDlg::OnPrePointInit983)
ON_FRESH_VV("PrePoint", FRCMD_OWNERDRAW, FrPointShopDlg::OnPrePointDraw983)
ON_FRESH_VI("Exchange_ok", FRCMD_INIT, FrPointShopDlg::OnOkInit983)
ON_FRESH_VV("Exchange_ok", FRCMD_LBUTTONUP, FrPointShopDlg::OnOkUp983)
ON_FRESH_VI("Own_item", FRCMD_INIT, FrPointShopDlg::OnOwnItemInit983)
ON_FRESH_VI("Own_item", FRCMD_OWNERDRAW, FrPointShopDlg::OnOwnItemDraw983)
ON_FRESH_VV("Own_item", FRCMD_LBUTTONUP, FrPointShopDlg::OnOwnItemUp983)
ON_FRESH_VI("Exchange_item", FRCMD_INIT, FrPointShopDlg::OnExchangeItemInit983)
ON_FRESH_VI("Exchange_item", FRCMD_OWNERDRAW, FrPointShopDlg::OnExchangeItemDraw983)
ON_FRESH_VV("Exchange_item", FRCMD_LBUTTONUP, FrPointShopDlg::OnExchangeItemUp983)
ON_FRESH_VI("Shop_ListBG1", FRCMD_INIT, FrPointShopDlg::OnShopListBG1Init983)
ON_FRESH_VI("Shop_ListBG2", FRCMD_INIT, FrPointShopDlg::OnShopListBG2Init983)
ON_FRESH_VI("Shop_ListBG3", FRCMD_INIT, FrPointShopDlg::OnShopListBG3Init983)
ON_FRESH_VI("Shop_ListBG4", FRCMD_INIT, FrPointShopDlg::OnShopListBG4Init983)
ON_FRESH_VI("Shop_ListBG5", FRCMD_INIT, FrPointShopDlg::OnShopListBG5Init983)
ON_FRESH_VI("Shop_ListBG6", FRCMD_INIT, FrPointShopDlg::OnShopListBG6Init983)
ON_FRESH_VI("Shop_item", FRCMD_INIT, FrPointShopDlg::OnShopItemInit983)
ON_FRESH_VI("Shop_item", FRCMD_OWNERDRAW, FrPointShopDlg::OnShopItemDraw983)
ON_FRESH_VV("Shop_item", FRCMD_LBUTTONUP, FrPointShopDlg::OnShopItemUp983)
END_FRESH_MSGMAP()

// 0084ba90: abre na página 3 (abertura)
bool FrPointShopDlg::OnInit()
{
	m_page = 3;
	m_shop.SetMode983(3);
	m_pMyOverBmp = g_pFresh->RegisterBitmap("tikipoint_btn_pointget_list_my_o");
	m_pUseOverBmp = g_pFresh->RegisterBitmap("tikipoint_btn_pointget_list_use_o");
	m_pListNormalBmp = g_pFresh->RegisterBitmap("tikipoint_pointuse_list_n");
	m_pListOverBmp = g_pFresh->RegisterBitmap("tikipoint_pointuse_list_o");
	m_pBuyBmp = g_pFresh->RegisterBitmap("tikipoint_buy");
	return true;
}

// 0084c480
void FrPointShopDlg::DrawBitmap983(const Bitmap* pBmp, WRect rc, unsigned long color, bool bFit)
{
	FrGraphicInterface* pGI;
	if (pBmp && (pGI = g_pFresh->GetManager()->GetGDI()) != NULL)
	{
		WRect dst = rc;
		if (bFit)
		{
			dst.w = (float)pBmp->Width();
			dst.h = (float)pBmp->Height();
		}
		pGI->SetTextStyle(0);
		pGI->DrawTexture(pBmp, dst, color, 0);
	}
}

// 0084c530
void FrPointShopDlg::OnMyPointInit983(int param)
{
	m_pMyPoint = KINDOF_CAST983(FrArea, param);
}

// 0084c5a0
void FrPointShopDlg::OnMainInit983(int param)
{
	m_pMain = KINDOF_CAST983(FrButton, param);
	if (m_pMain)
		m_pMain->SetVisible(false);
	if (m_pMain)
		m_pMain->SetPushDelay(0.01f);
	if (m_pMain)
		m_pMain->EnableBlink(true);
}

// 0084c650
void FrPointShopDlg::OnHelpInit983(int param)
{
	m_pHelp = KINDOF_CAST983(FrButton, param);
	if (m_pHelp)
		m_pHelp->SetVisible(true);
	if (m_pHelp)
		m_pHelp->SetPushDelay(0.01f);
}

// 0084c6f0
void FrPointShopDlg::OnExitInit983(int param)
{
	m_pExit = KINDOF_CAST983(FrButton, param);
	if (m_pExit)
		m_pExit->SetVisible(true);
	if (m_pExit)
		m_pExit->SetPushDelay(0.01f);
}

// 0084c790
void FrPointShopDlg::OnGotoMainBgInit983(int param)
{
	m_pGotoMainBg = KINDOF_CAST983(FrArea, param);
	if (m_pGotoMainBg)
		m_pGotoMainBg->SetVisible(false);
}

// 0084c810
void FrPointShopDlg::OnIntroBg1Init983(int param)
{
	m_pIntroBg1 = KINDOF_CAST983(FrArea, param);
	if (m_pIntroBg1)
		m_pIntroBg1->SetVisible(true);
}

// 0084c890
void FrPointShopDlg::OnIntroBg2Init983(int param)
{
	m_pIntroBg2 = KINDOF_CAST983(FrArea, param);
	if (m_pIntroBg2)
		m_pIntroBg2->SetVisible(true);
}

// 0084c910
void FrPointShopDlg::OnIntroNoticeInit983(int param)
{
	m_pIntroNotice = KINDOF_CAST983(FrViewer, param);
	if (m_pIntroNotice)
		m_pIntroNotice->SetVisible(true);
	m_pIntroNotice->ShowScrollBar(true);
}

// 0084c9b0
void FrPointShopDlg::OnIntroNoticeDraw983()
{
	if (m_pIntroNotice && m_pIntroNotice->IsVisible())
		m_pIntroNotice->Open("pointshop_notice.jpg");
}

// 0084c9d0
void FrPointShopDlg::OnExchangeInit983(int param)
{
	m_pExchange = KINDOF_CAST983(FrButton, param);
	if (m_pExchange)
		m_pExchange->SetVisible(true);
	if (m_pExchange)
		m_pExchange->SetPushDelay(0.01f);
	if (m_pExchange)
		m_pExchange->EnableHover(true, 0.0f);
}

// 0084ca80
void FrPointShopDlg::OnBuyInit983(int param)
{
	m_pBuy = KINDOF_CAST983(FrButton, param);
	if (m_pBuy)
		m_pBuy->SetVisible(true);
	if (m_pBuy)
		m_pBuy->SetPushDelay(0.01f);
}

// 0084cb20
void FrPointShopDlg::OnExchangeListBG1Init983(int param)
{
	m_pExchangeListBG[0] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[0])
		m_pExchangeListBG[0]->SetVisible(false);
}

// 0084cba0
void FrPointShopDlg::OnExchangeListBG2Init983(int param)
{
	m_pExchangeListBG[1] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[1])
		m_pExchangeListBG[1]->SetVisible(false);
}

// 0084cc20
void FrPointShopDlg::OnExchangeListBG3Init983(int param)
{
	m_pExchangeListBG[2] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[2])
		m_pExchangeListBG[2]->SetVisible(false);
}

// 0084cca0
void FrPointShopDlg::OnExchangeListBG4Init983(int param)
{
	m_pExchangeListBG[3] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[3])
		m_pExchangeListBG[3]->SetVisible(false);
}

// 0084cd20
void FrPointShopDlg::OnExchangeListBG5Init983(int param)
{
	m_pExchangeListBG[4] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[4])
		m_pExchangeListBG[4]->SetVisible(false);
}

// 0084cda0
void FrPointShopDlg::OnExchangeListBG6Init983(int param)
{
	m_pExchangeListBG[5] = KINDOF_CAST983(FrArea, param);
	if (m_pExchangeListBG[5])
		m_pExchangeListBG[5]->SetVisible(false);
}

// 0084ce20
void FrPointShopDlg::OnPrePointInit983(int param)
{
	m_pPrePoint = KINDOF_CAST983(FrArea, param);
	if (m_pPrePoint)
		m_pPrePoint->SetVisible(false);
}

// 0084cea0: TP da troca escolhida
void FrPointShopDlg::OnPrePointDraw983()
{
	if (m_pPrePoint && m_pFont && m_pPrePoint->IsVisible())
	{
		m_pFont->SetColor(0xffd31b91);
		FrArea* pArea = m_pPrePoint;
		m_pFont->Print(pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", m_shop.GetTotal983()), (eTITLEFONTALINE)2, 0.9f,
			1.0f);
	}
}

// 0084cf20
void FrPointShopDlg::OnOkInit983(int param)
{
	m_pOk = KINDOF_CAST983(FrButton, param);
	if (m_pOk)
		m_pOk->SetVisible(false);
}

// 0084cfa0
void FrPointShopDlg::OnOwnItemInit983(int param)
{
	m_pOwnItem = KINDOF_CAST983(FrListBox, param);
	if (m_pOwnItem)
		m_pOwnItem->SetVisible(false);
	m_pOwnItem->EnableHover(true, 0.1f);
}

// 0084d030
void FrPointShopDlg::OnExchangeItemInit983(int param)
{
	m_pExchangeItem = KINDOF_CAST983(FrListBox, param);
	if (m_pExchangeItem)
		m_pExchangeItem->SetVisible(false);
}

// 0084d0b0
void FrPointShopDlg::OnShopListBG1Init983(int param)
{
	m_pShopListBG[0] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[0])
		m_pShopListBG[0]->SetVisible(false);
}

// 0084d130
void FrPointShopDlg::OnShopListBG2Init983(int param)
{
	m_pShopListBG[1] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[1])
		m_pShopListBG[1]->SetVisible(false);
}

// 0084d1b0
void FrPointShopDlg::OnShopListBG3Init983(int param)
{
	m_pShopListBG[2] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[2])
		m_pShopListBG[2]->SetVisible(false);
}

// 0084d230
void FrPointShopDlg::OnShopListBG4Init983(int param)
{
	m_pShopListBG[3] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[3])
		m_pShopListBG[3]->SetVisible(false);
}

// 0084d2b0
void FrPointShopDlg::OnShopListBG5Init983(int param)
{
	m_pShopListBG[4] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[4])
		m_pShopListBG[4]->SetVisible(false);
}

// 0084d330
void FrPointShopDlg::OnShopListBG6Init983(int param)
{
	m_pShopListBG[5] = KINDOF_CAST983(FrArea, param);
	if (m_pShopListBG[5])
		m_pShopListBG[5]->SetVisible(false);
}

// 0084d3b0
void FrPointShopDlg::OnShopItemInit983(int param)
{
	m_pShopItem = KINDOF_CAST983(FrListBox, param);
	if (m_pShopItem)
		m_pShopItem->SetVisible(false);
}

// 0084dbc0: TP que eu tenho (CSharedDoc +0x3860)
void FrPointShopDlg::OnMyPointDraw983()
{
	if (m_pMyPoint && m_pFont)
	{
		m_pFont->SetColor(0xffd31b91);
		FrArea* pArea = m_pMyPoint;
		m_pFont->Print(pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", *(int*)&Doc()->m_pointShopTP983), (eTITLEFONTALINE)2,
			1.0f, 1.0f);
	}
}

// 0084dc30
void FrPointShopDlg::OnHelpUp983()
{
	if (m_pHelp && m_pHelp->IsVisible())
	{
		FrPointShopHelp* pDlg = CreateForm<FrPointShopHelp>(g_pFresh->GetManager(), this, "PointShopHelp", NULL);
		if (pDlg)
			pDlg->Open(NULL, 3);
	}
}

// 0084e320
sPointShopListItem983* FrPointShopDlg::FindListItem983(unsigned long typeId)
{
	sPointShopListItem983* pFound = NULL;
	for (std::list<sPointShopListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
	{
		if (it->typeId == typeId)
		{
			pFound = &*it;
			break;
		}
	}
	return pFound;
}

// 0084e350: só itens com TP de troca (TikiPoints); peça/taco/bola/item fora de uso e sem carta; pelo menos uma unidade inteira
bool FrPointShopDlg::CanExchange983(unsigned long typeId, void* pData)
{
	bool ok = false;
	if (!pData)
		return false;
	unsigned long kind = typeId >> 26;
	IFF_ITEM_COMMON* pIff;
	switch (kind)
	{
	case 2:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindPart(typeId);
		break;
	case 4:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(typeId);
		break;
	case 5:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindBall(typeId);
		break;
	case 6:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindItem(typeId);
		break;
	case 0x1f:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindCard(typeId);
		break;
	default:
		return false;
	}
	if (!pIff || pIff->TikiPoints == 0)
		return false;
	switch (kind)
	{
	case 2:
	case 4:
	case 5:
	case 6:
	{
		sItemInfo* pInfo = (sItemInfo*)pData;
		if (!Doc()->IsEquipParts(pInfo->guid, pInfo->typeId))
		{
			ok = true;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it = Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (it->partsUid == pInfo->guid)
				{
					ok = false;
					break;
				}
			}
		}
		unsigned int unit = m_shop.GetUnitCount983(pInfo->typeId);
		if (unit == 0)
			return false;
		if (kind == 6)
		{
			unsigned int equipped = 0;
			for (int i = 0; i < 10; ++i)
			{
				if (typeId == Doc()->m_myInfo.userEquip.tidItemSlot[i])
					++equipped;
			}
			if ((pInfo->Common[0] - equipped) / unit < 1)
				return false;
			return ok;
		}
		if (kind == 5)
		{
			if (pInfo->Common[0] / unit < 1)
				return false;
			if (Doc()->m_myInfo.userEquip.tidBall == typeId)
				ok = false;
		}
		return ok;
	}
	case 0x1f:
	{
		sCards* pCard = (sCards*)pData;
		if (pCard->partTid == 0 && pCard->partUid == 0)
			ok = true;
		unsigned int unit = m_shop.GetUnitCount983(pCard->tid);
		if (unit == 0)
			return false;
		if (pCard->cardCount / unit < 1)
			ok = false;
		return ok;
	}
	}
	return false;
}

// 0084e5d0
void FrPointShopDlg::FillListBox983()
{
	FrListBox* pList;
	switch (m_page)
	{
	case 1: pList = m_pOwnItem; break;
	case 2: pList = m_pShopItem; break;
	default: return;
	}
	if (pList)
	{
		for (std::list<sPointShopListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
			pList->AddItem(&*it);
	}
}

// 0084e620: item meu para a troca (uma unidade por clique)
void FrPointShopDlg::OnOwnItemUp983()
{
	FrListItem* pItem = m_pOwnItem->GetItemUnderCursor();
	if (!pItem)
		return;
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sPointShopListItem983* p = FindListItem983(typeId);
	if (!p || p->uids.begin() == p->uids.end())
		return;
	CPointShop::sItem key;
	key.uid = 0;
	key.count = 0;
	key.uid = p->uids.begin()->first;
	key.typeId = typeId;
	CPointShop::sItem* pAdded = m_shop.Add983(key);
	if (!pAdded)
		return;
	unsigned int unit = m_shop.GetUnitCount983(typeId);
	p->count -= unit;
	if ((unsigned int)p->count < unit)
		m_pOwnItem->DelItem(pItem->pData);
	if (!m_pExchangeItem->FindItem(&pAdded->typeId))
		m_pExchangeItem->AddItem(&pAdded->typeId);
}

// 0084e6e0: tira uma unidade da troca
void FrPointShopDlg::OnExchangeItemUp983()
{
	FrListItem* pItem = m_pExchangeItem->GetItemUnderCursor();
	if (!pItem)
		return;
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sPointShopListItem983* p = FindListItem983(typeId);
	if (!p || p->uids.begin() == p->uids.end())
		return;
	CPointShop::sItem key;
	key.uid = 0;
	key.count = 0;
	key.uid = p->uids.begin()->first;
	key.typeId = typeId;
	if (m_shop.Remove983(key) == 0)
		m_pExchangeItem->DelItem(pItem->pData);
	p->count += m_shop.GetUnitCount983(typeId);
	if (!m_pOwnItem->FindItem(p))
		m_pOwnItem->AddItem(p);
}

// constantes (00d05ac0–00d05ad8, as mesmas da alchemympbreakdlg)
static const double ITEM_ICON_OFFSET_Y983 = 4.5;
static const double ITEM_ICON_SCALE983 = 1.4f; // figura = altura da linha / 1,4
static const double ITEM_NAME_OFFSET_X983 = 1.6f;
static const double ITEM_TEXT_OFFSET_Y983 = 0.35f;

// 0084eda0: uma linha das listas (OWNERDRAW): figura (peça: antes o rosto do personagem; taco/bola com "bar_"), nome cortado em
// maxLen ("...") com " (<unidade>)" na troca, "x<quantidade>" (troca: unidades inteiras; compra: a do PointShop.iff) e o TP
// ("<tp>TP": IFF +0x88 na troca, preço na compra)
void FrPointShopDlg::DrawListItem983(FrListItem* pItem, float h, unsigned int maxLen, unsigned int count)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pItem || !pGI)
		return;
	pGI->SetTextColor(0xff000000, 0xffffffff);
	pGI->SetTextStyle(0);
	unsigned long* pTypeId = (unsigned long*)pItem->pData;
	std::string icon("");
	std::string name("");
	int charIndex = 0;
	int point = 0;
	unsigned int shown = 0;
	int unit = 0;
	IFF_ITEM_COMMON* pIff;
	switch (*pTypeId >> 26)
	{
	case 2:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindPart(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			point = pIff->TikiPoints;
			charIndex = (*pTypeId >> 18) & 0xff;
		}
		break;
	case 4:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			point = pIff->TikiPoints;
		}
		if (!icon.empty())
			icon = std::string("bar_") + icon;
		break;
	case 5:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindBall(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			point = pIff->TikiPoints;
		}
		if (!icon.empty())
			icon = std::string("bar_") + icon;
		break;
	case 6:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindItem(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			point = pIff->TikiPoints;
		}
		break;
	case 0x1f:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindCard(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			point = pIff->TikiPoints;
		}
		break;
	}
	if (m_page == 2)
	{
		IFF983::sPointShop* pShop = ItemManager()->FindPointShop983(*pTypeId);
		if (pShop)
		{
			point = *(int*)&pShop->raw[0];
			shown = *(unsigned int*)&pShop->raw[4];
			unit = 0;
		}
	}
	else if (m_page == 1)
	{
		unit = m_shop.GetUnitCount983(*pTypeId);
		shown = count;
		if (unit)
			shown = count / unit;
	}
	WRect rc;
	rc.x = pItem->pos.x + 5.0;
	rc.y = pItem->pos.y + ITEM_ICON_OFFSET_Y983;
	rc.w = 0.0f;
	rc.h = 0.0f;
	if ((*pTypeId & 0xfc000000) == 0x08000000)
	{
		std::string face("");
		face = CreateCharacterOption::Instance()->GetBarIcon983(charIndex);
		if (!face.empty())
		{
			const Bitmap* pBmp = g_pFresh->GetBitmap(face.c_str());
			if (pBmp)
			{
				float scale = h / (pBmp->Height() * ITEM_ICON_SCALE983);
				scale = scale < 1.0f ? scale : 1.0f;
				if (m_page == 2)
					rc.y -= 2.0;
				rc.w = pBmp->Width() * scale;
				rc.h = pBmp->Height() * scale;
				pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
				if (m_page == 2)
					rc.y += 2.0;
			}
		}
	}
	if (!icon.empty())
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(icon.c_str());
		if (pBmp)
		{
			float scale = h / (pBmp->Height() * ITEM_ICON_SCALE983);
			scale += (*pTypeId & 0xfc000000) == 0x18000000 ? scale * 0.5 : 0.0;
			scale = scale < 1.0f ? scale : 1.0f;
			if (m_page == 2)
				rc.y -= 2.0;
			rc.x = scale + scale + rc.w + rc.x;
			rc.w = pBmp->Width() * scale;
			rc.h = pBmp->Height() * scale;
			pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
			if (m_page == 2)
				rc.y += 2.0;
		}
	}
	if (!name.empty())
	{
		if (name.length() > maxLen)
		{
			char buf[32] = {0};
			mbscut(buf, name.c_str(), maxLen - 1);
			name = buf;
			name += "...";
		}
		if (unit)
			name += MakeStr(" (%d)", unit);
		float nameX = h * ITEM_NAME_OFFSET_X983;
		rc.x = nameX + pItem->pos.x;
		pGI->Print11(WPoint(rc.x, (h - 11.0) * ITEM_TEXT_OFFSET_Y983 + rc.y), 0, name.c_str());
	}
	pGI->SetTextColor(0xff871ba4, 0xffffffff);
	pGI->Print(WPoint((float)(maxLen * 11) + pItem->pos.x, (h - 11.0) * ITEM_TEXT_OFFSET_Y983 + rc.y), 2, MakeStr("x%d", shown));
	pGI->SetTextStyle(1);
	pGI->SetTextColor(0xffd31b9f, 0xffffffff);
	float pointX = (float)(maxLen * 13);
	pGI->Print(WPoint((pointX + pItem->pos.x) - 4.0, rc.y + (h - 11.0) * ITEM_TEXT_OFFSET_Y983), 2, MakeStr("%d", point));
	pGI->SetTextColor(0xff871ba4, 0xffffffff);
	pGI->Print(WPoint(pointX + pItem->pos.x, rc.y + (h - 11.0) * ITEM_TEXT_OFFSET_Y983), 0, "TP");
}

// 0084f700 ("Own_item" OWNERDRAW)
void FrPointShopDlg::OnOwnItemDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	float w = (float)m_pOwnItem->GetItemWidth();
	float h = (float)m_pOwnItem->GetItemHeight();
	if (m_pOwnItem->GetItemUnderCursor() == pItem)
		DrawBitmap983(m_pMyOverBmp, WRect(pItem->pos.x + 2.0, pItem->pos.y + 2.0, w, h), 0xffffffff, true);
	sPointShopListItem983* p = FindListItem983(*(unsigned long*)pItem->pData);
	if (p)
		DrawListItem983(pItem, h + 1.0, 0x10, p->count);
}

// 0084f7c0 ("Exchange_item" OWNERDRAW): quantidade = unidades escolhidas × unidade do item
void FrPointShopDlg::OnExchangeItemDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	float w = (float)m_pExchangeItem->GetItemWidth();
	float h = (float)m_pExchangeItem->GetItemHeight();
	if (m_pExchangeItem->GetItemUnderCursor() == pItem)
		DrawBitmap983(m_pUseOverBmp, WRect(pItem->pos.x + 2.0, pItem->pos.y + 2.0, w, h), 0xffffffff, true);
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sPointShopListItem983* p = FindListItem983(typeId);
	if (p && p->uids.begin() != p->uids.end())
	{
		CPointShop::sItem key;
		key.uid = p->uids.begin()->first;
		key.typeId = typeId;
		key.count = 0;
		CPointShop::sItem* pFound = m_shop.Find983(key);
		int unit = m_shop.GetUnitCount983(typeId);
		if (pFound && unit)
			DrawListItem983(pItem, h + 1.0, 0x10, pFound->count * unit);
	}
}

// 0084f8d0 ("Shop_item" OWNERDRAW): fundo da linha, botão "tikipoint_buy" e o item
void FrPointShopDlg::OnShopItemDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	float w = (float)m_pShopItem->GetItemWidth();
	float h = (float)m_pShopItem->GetItemHeight();
	if (m_pShopItem->GetItemUnderCursor() == pItem)
		DrawBitmap983(m_pListOverBmp, WRect(pItem->pos.x, pItem->pos.y, w, h), 0xffffffff, false);
	DrawBitmap983(m_pBuyBmp, WRect(pItem->pos.x + 412.0, pItem->pos.y + 10.0, w, h), 0xffffffff, true);
	DrawListItem983(pItem, h, 0x1e, m_shop.GetUnitCount983(*(unsigned long*)pItem->pData));
}

// 00850200
void FrPointShopDlg::OnProc(const float deltaTime)
{
	if (m_pOk && m_page == 1)
	{
		CPointShop::ItemMap items = m_shop.GetItems983();
		m_pOk->Enable(items.size() ? true : false);
	}
}

// 008502b0: "Exchange_ok" (troca) → aviso de confirmação
void FrPointShopDlg::OnOkUp983()
{
	CPointShop::ItemMap items = m_shop.GetItems983();
	if (items.size() == 0)
		return;
	FrPointShopNoticeDlg* pDlg = CreateForm<FrPointShopNoticeDlg>(g_pFresh->GetManager(), this, "PointShopNoticeDlg", NULL);
	pDlg->SetShop983(&m_shop);
	pDlg->Open(NULL, 0);
}

// 008503a0: clique na lista de compra → escolhe só esse item e abre o aviso de confirmação
void FrPointShopDlg::OnShopItemUp983()
{
	FrListItem* pItem = m_pShopItem->GetItemUnderCursor();
	if (!pItem)
		return;
	m_shop.Clear983();
	CPointShop::sItem item = { 0, ((sPointShopListItem983*)pItem->pData)->typeId, 0 };
	if (m_shop.Add983(item))
	{
		FrPointShopNoticeDlg* pDlg = CreateForm<FrPointShopNoticeDlg>(g_pFresh->GetManager(), this, "PointShopNoticeDlg", NULL);
		if (pDlg)
		{
			pDlg->SetShop983(&m_shop);
			pDlg->Open(NULL, 0);
		}
	}
}

// 00850850
FrPointShopDlg::FrPointShopDlg()
{
	m_pGotoMainBg = NULL;
	m_pIntroBg1 = NULL;
	m_pIntroBg2 = NULL;
	m_pIntroNotice = NULL;
	m_pMain = NULL;
	m_pHelp = NULL;
	m_pExit = NULL;
	m_pExchange = NULL;
	m_pBuy = NULL;
	for (int i = 0; i < 6; ++i)
		m_pExchangeListBG[i] = NULL;
	for (int i = 0; i < 6; ++i)
		m_pShopListBG[i] = NULL;
	m_pMyPoint = NULL;
	m_pPrePoint = NULL;
	m_pOk = NULL;
	m_pOwnItem = NULL;
	m_pExchangeItem = NULL;
	m_pShopItem = NULL;
	m_pMyOverBmp = NULL;
	m_pUseOverBmp = NULL;
	m_pListNormalBmp = NULL;
	m_pListOverBmp = NULL;
	m_pBuyBmp = NULL;
	m_pFont = NULL;
	m_pFont = new CWindFont;
}

IMPLEMENT_OBJECT(FrPointShopDlg, FrForm)

// 00850a30
FrPointShopDlg::~FrPointShopDlg()
{
	m_pMyOverBmp = NULL;
	m_pUseOverBmp = NULL;
	m_pListNormalBmp = NULL;
	m_pListOverBmp = NULL;
	m_pBuyBmp = NULL;
	for (std::list<sPointShopListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
		it->uids.clear();
	m_list.clear();
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
}

// 00850da0: soma no item do tipo (ou cria um novo, no começo ou no fim da lista); guarda o uid com a quantidade
int FrPointShopDlg::AddListItem983(unsigned long typeId, unsigned long uid, int count, bool bFront)
{
	sPointShopListItem983* p = FindListItem983(typeId);
	if (!p)
	{
		sPointShopListItem983 item;
		item.typeId = typeId;
		item.count = count;
		if (uid)
			item.uids[uid] = count;
		if (bFront)
			m_list.push_front(item);
		else
			m_list.push_back(item);
		return count;
	}
	p->count += count;
	if (uid)
		p->uids[uid] = count;
	return p->count;
}

// 00850f10: troca → inventário que rende TP (peças, bolas, itens, tacos, cartas); compra → PointShop.iff (ativos)
void FrPointShopDlg::RefreshList983()
{
	for (std::list<sPointShopListItem983>::iterator lt = m_list.begin(); lt != m_list.end(); ++lt)
		lt->uids.clear();
	m_list.clear();
	switch (m_page)
	{
	case 1:
		{
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_partsList.begin(); it != Doc()->m_partsList.end(); ++it)
			{
				sItemInfo* pInfo = &*it;
				if (pInfo->typeId && pInfo->ItemType == 0 && CanExchange983(pInfo->typeId, pInfo))
					AddListItem983(pInfo->typeId, pInfo->guid, 1, false);
			}
			for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end(); ++it)
			{
				sItemInfo* pInfo = &*it;
				if (pInfo->typeId && pInfo->ItemType == 0 && CanExchange983(pInfo->typeId, pInfo))
					AddListItem983(pInfo->typeId, pInfo->guid, pInfo->Common[0], false);
			}
			for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); ++it)
			{
				sItemInfo* pInfo = &*it;
				if (pInfo->typeId && pInfo->ItemType == 0 && CanExchange983(pInfo->typeId, pInfo))
				{
					unsigned long typeId = pInfo->typeId;
					int count = pInfo->Common[0];
					if ((typeId & 0x2000000) == 0)
					{
						int equipped = 0;
						for (int i = 0; i < 10; ++i)
						{
							if (typeId == Doc()->m_myInfo.userEquip.tidItemSlot[i])
								++equipped;
						}
						count -= equipped;
					}
					AddListItem983(typeId, pInfo->guid, count, false);
				}
			}
			for (std::map<unsigned int, sItemInfo>::iterator mt = Doc()->m_clubSetMap.begin(); mt != Doc()->m_clubSetMap.end();
				++mt)
			{
				sItemInfo* pInfo = &mt->second;
				if (pInfo->typeId && pInfo->ItemType == 0 && CanExchange983(pInfo->typeId, pInfo))
					AddListItem983(pInfo->typeId, pInfo->guid, 1, false);
			}
			for (std::list<sCards>::iterator ct = Doc()->m_cardList.begin(); ct != Doc()->m_cardList.end(); ++ct)
			{
				sCards* pCard = &*ct;
				if (pCard->tid && CanExchange983(pCard->tid, pCard))
					AddListItem983(pCard->tid, pCard->uid, pCard->cardCount, false);
			}
		}
		break;
	case 2:
		{
			std::map<unsigned int, IFF983::sPointShop>& shop = ItemManager()->m_PointShopMap983;
			for (std::map<unsigned int, IFF983::sPointShop>::iterator st = shop.begin(); st != shop.end(); ++st)
			{
				if ((unsigned char)st->second.active && st->second.TypeId)
				{
					if (st->second.raw[8] & 1)
						AddListItem983(st->second.TypeId, 0, *(int*)&st->second.raw[4], true);
					else
						AddListItem983(st->second.TypeId, 0, *(int*)&st->second.raw[4], false);
				}
			}
		}
		break;
	}
}

// 00851310: mostra os controles da página e refaz a lista
void FrPointShopDlg::SetPage983(int page)
{
	m_page = page;
	m_shop.SetMode983(page);
	bool bIntro = false;
	bool bExchange = false;
	bool bShop = false;
	if (page == 1)
		bExchange = true;
	else if (page == 2)
		bShop = true;
	else if (page == 3)
		bIntro = true;
	if (m_pGotoMainBg)
		m_pGotoMainBg->SetVisible(!bIntro);
	if (m_pMain)
		m_pMain->SetVisible(!bIntro);
	if (m_pIntroBg1)
		m_pIntroBg1->SetVisible(bIntro);
	if (m_pIntroBg2)
		m_pIntroBg2->SetVisible(bIntro);
	if (m_pIntroNotice)
		m_pIntroNotice->SetVisible(bIntro);
	if (m_pExchange)
		m_pExchange->SetVisible(bIntro);
	if (m_pBuy)
		m_pBuy->SetVisible(bIntro);
	if (m_pExchangeListBG[0])
		m_pExchangeListBG[0]->SetVisible(bExchange);
	if (m_pExchangeListBG[1])
		m_pExchangeListBG[1]->SetVisible(bExchange);
	if (m_pExchangeListBG[2])
		m_pExchangeListBG[2]->SetVisible(bExchange);
	if (m_pExchangeListBG[3])
		m_pExchangeListBG[3]->SetVisible(bExchange);
	if (m_pExchangeListBG[4])
		m_pExchangeListBG[4]->SetVisible(bExchange);
	if (m_pExchangeListBG[5])
		m_pExchangeListBG[5]->SetVisible(bExchange);
	if (m_pPrePoint)
		m_pPrePoint->SetVisible(bExchange);
	if (m_pOk)
		m_pOk->SetVisible(bExchange);
	if (m_pOwnItem)
		m_pOwnItem->SetVisible(bExchange);
	if (m_pOwnItem)
		m_pOwnItem->ClearItem();
	if (m_pExchangeItem)
		m_pExchangeItem->SetVisible(bExchange);
	if (m_pExchangeItem)
		m_pExchangeItem->ClearItem();
	if (m_pShopListBG[0])
		m_pShopListBG[0]->SetVisible(bShop);
	if (m_pShopListBG[1])
		m_pShopListBG[1]->SetVisible(bShop);
	if (m_pShopListBG[2])
		m_pShopListBG[2]->SetVisible(bShop);
	if (m_pShopListBG[3])
		m_pShopListBG[3]->SetVisible(bShop);
	if (m_pShopListBG[4])
		m_pShopListBG[4]->SetVisible(bShop);
	if (m_pShopListBG[5])
		m_pShopListBG[5]->SetVisible(bShop);
	if (m_pShopItem)
		m_pShopItem->SetVisible(bShop);
	if (m_pShopItem)
		m_pShopItem->ClearItem();
	RefreshList983();
	FillListBox983();
}

// 008515a0
void FrPointShopDlg::Refresh983()
{
	SetPage983(m_page);
	m_shop.Clear983();
}
