#include "minatl.h"
#include "vc10_layout.h"
#include "shopmain.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // D820
#include "avatar_avatar.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frgroupbox.h"
#include "frtabbutton.h"
#include "frcombobox.h"
#include "frtextbutton.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "projectg.h"
#include "woverlay.h"
#include "tooltip.h"
#include "shareddoc.h"
#include "uccmanager.h"
#include "giftdlg.h"
#include "checkoutdlg.h"
#include "upgradecaddiedlg.h"
#include "messengerdlg.h"
#include "repaydlg.h"
#include "caddieinfodlg.h"
#include "setitemdlg.h"
#include "partsinfo.h"
#include "rankingdlg.h"
#include "user_info.h"
#include "parttidlist.h"
#include "cardsystem.h"
#include "lobbytask.h"
#include "jp/rftooltiphelper.h"
#include "s5/utilities.h"
#include "gatewayactor.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10
// HACK: only this static member is needed here. mascot.h (npc.h + inline bodies) would add IL to this file, which is already
// IL-heavy from the naked functions, and IObject::DynamicCast of the PCH would then be compiled before the CreateForm<> templates
class WPuppet;
class CMascot
{
public:
	static int ChangeMascotMsgInTex(const char* msg,
		const IFF_STRUCT::sMascot* pMascot, WPuppet* pPuppet,
		const char* texName);
};
#include "mathconsts.h"
#include "jp/statushelper.h"
#include "jp/taskwidget.h"
#include "jp/taskform.h" // FrTaskForm (CTask::GetWidgetForm983)
#include "jp/itemsearchdlg.h" // sItemSearchResult983 (0x2d3)
#include "../shared/localize.h"
#include "jp/refresh/rflobbydesktop.h" // 983 00a8ca90: m_bChanged do desktop
#include "jp/refresh/rflobbycharacterselectwnd.h" // gaveta de personagens (00e78228)


// 983: so o que a loja usa do estado do lobby (singleton 00e78ca8; a classe cheia e da frente B, include/jp/lobbyjp.h)
extern int g_lobbyModal983; // 00e78cac

class CLobbyState983a
{
public:
	void SetShopMode983(int mode); // 005fbe70
	void EndShopMode983(); // 005f8020
	static CLobbyState983a* ms_pInstance; // 00e78ca8
};
// E148: eram stubs vazios no exe (jp983_stubs.log); a funcao real e do RFLobbyDesktop (this = 00e78ca8)
#pragma comment(linker, "/alternatename:?SetShopMode983@CLobbyState983a@@QAEXH@Z=?SetScreen983@RFLobbyDesktop@@QAEXH@Z")
#pragma comment(linker, "/alternatename:?EndShopMode983@CLobbyState983a@@QAEXXZ=?UpdateLobbyActors983@RFLobbyDesktop@@QAEXXZ")



// HACK: 156.0f is defined in helpmsg.cpp as a function-local static aliased to __real@431c0000 (helpmsg.obj is linked
// earlier and owns it in the original); the literal here would emit a competing COMDAT in this object's .rdata
extern "C" __declspec(selectany) const float rb_real_431c0000 = 156.0f; // C64.4: o estático de função do rebang não existe mais

extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

extern Fresh* g_pFresh;
extern const WRect g_rectUnitW983; // 00e4296c (retangulo de origem padrao)
std::string __fastcall GenerateTextureName_F(unsigned long typeId,
	const char* uccIndex);
std::string __fastcall GenerateTextureName_B(unsigned long typeId,
	const char* uccIndex);
// 983: no CShopMain o SetSubTabImg esta inline (3x MakeStr com "stext_%d_n"), nao chama 00604b00
static __forceinline void SetSubTabImg(const char* names[3], int n)
{
	names[0] = MakeStr("stext_%d_n", n);
	names[1] = MakeStr("stext_%d_n", n);
	names[2] = MakeStr("stext_%d_n", n);
}
int float2int(float f);

// CAnotherHand::m_csUser/m_csQueue/m_curWork/m_vacation/m_aWork are protected; the macros below reach them through this
// never-instantiated view
struct RbHandView : public CAnotherHand
{
	using CAnotherHand::m_csUser;
	using CAnotherHand::m_csQueue;
	using CAnotherHand::m_curWork;
	using CAnotherHand::m_vacation;
	using CAnotherHand::m_aWork;
};

// macros for naked functions, not inline functions: a call in the dead copy of a naked function would force an out-of-line COMDAT
// that the original does not have
#define LOCK_USER(hand) \
	do \
	{ \
		EnterCriticalSection(&((RbHandView*)(hand))->m_csUser); \
		(hand)->m_bUserLock = true; \
	} while (0)

#define UNLOCK_USER(hand) \
	do \
	{ \
		(hand)->m_bUserLock = false; \
		LeaveCriticalSection(&((RbHandView*)(hand))->m_csUser); \
	} while (0)

// the original inlines "queue empty or current work key != 0" here.
#define HAND_QUERY_IDLE(hand, idle) \
	do \
	{ \
		RbHandView* pView = (RbHandView*)(hand); \
		EnterCriticalSection(&pView->m_csQueue); \
		(hand)->m_bQueseLock = true; \
		(idle) = pView->m_curWork == pView->m_vacation || \
			pView->m_aWork[pView->m_curWork].key != 0; \
		(hand)->m_bQueseLock = false; \
		LeaveCriticalSection(&pView->m_csQueue); \
	} while (0)

IMPLEMENT_OBJECT(CShopMain, FrTaskForm)

// 983 00a92d70 / 00cea580: criador do form "shopmain" na fabrica de forms
static FrForm* CreateShopMain983(FrWndManager* pManager, const char* name)
{
	return CreateForm<CShopMain>(pManager, NULL, name);
}
static FrFormRegistrar983 s_shopMainRegistrar("shopmain", CreateShopMain983);

BEGIN_FRESH_MSGMAP(CShopMain, FrForm)
ON_FRESH_VI("shopview", FRCMD_INIT,
	CShopMain::OnSortOptionComboInit)
ON_FRESH_VV("shopview", FRCMD_LBUTTONDOWN,
	CShopMain::OnSortOptionComboDown)
ON_FRESH_VI("searchgoods", FRCMD_INIT,
	CShopMain::OnSearchGoodsButtonInit)
ON_FRESH_VV("searchgoods", FRCMD_LBUTTONUP,
	CShopMain::OnSearchGoodsLButtonDown)
ON_FRESH_VI("exportselectlist", FRCMD_INIT,
	CShopMain::OnExportSelectListInit)
ON_FRESH_VV("exportselectlist", FRCMD_LBUTTONUP,
	CShopMain::OnExportSelectListUp)
ON_FRESH_VI("main_bg1", FRCMD_INIT,
	CShopMain::OnMainBg1Init)
ON_FRESH_VI("main_bg2", FRCMD_INIT,
	CShopMain::OnMainBg2Init)
ON_FRESH_VI("main_bg3", FRCMD_INIT,
	CShopMain::OnMainBg3Init)
ON_FRESH_VI("main_bg4", FRCMD_INIT,
	CShopMain::OnMainBg4Init)
ON_FRESH_VI("item_list", FRCMD_INIT,
	CShopMain::OnItemListInit)
ON_FRESH_VI("item_list", FRCMD_OWNERDRAW,
	CShopMain::OnItemListOwnerDraw)
ON_FRESH_VV("item_list", FRCMD_LBUTTONDOWN,
	CShopMain::OnItemListBtnDown)
ON_FRESH_VV("item_list", FRCMD_RBUTTONDOWN,
	CShopMain::OnItemListRBtnDown)
ON_FRESH_VI("tab_main", FRCMD_INIT,
	CShopMain::OnTabMainInit)
ON_FRESH_VV("tab_main", FRCMD_LBUTTONUP,
	CShopMain::OnTabMainButtonUp)
ON_FRESH_VI("tab_sub", FRCMD_INIT,
	CShopMain::OnTabSubInit)
ON_FRESH_VV("tab_sub", FRCMD_LBUTTONUP,
	CShopMain::OnTabSubButtonUp)
ON_FRESH_VI("item_search_btn", FRCMD_INIT,
	CShopMain::OnItemSearchBtnInit)
ON_FRESH_VV("item_search_btn", FRCMD_LBUTTONUP,
	CShopMain::OnItemSearchBtnUp)
ON_FRESH_VI("buy_all", FRCMD_INIT,
	CShopMain::OnBuyAllInit)
ON_FRESH_VV("buy_all", FRCMD_LBUTTONUP,
	CShopMain::OnBuyAllUp)
ON_FRESH_VI("buy_all", FRCMD_HOVERON,
	CShopMain::OnBuyHoverOn)
ON_FRESH_VI("buy_all", FRCMD_HOVEROFF,
	CShopMain::OnBuyHoverOff)
ON_FRESH_VI("buy_reset", FRCMD_INIT,
	CShopMain::OnBuyResetInit)
ON_FRESH_VV("buy_reset", FRCMD_LBUTTONUP,
	CShopMain::OnBuyResetUp)
ON_FRESH_VI("buy_reset", FRCMD_HOVERON,
	CShopMain::OnBuyHoverOn)
ON_FRESH_VI("buy_reset", FRCMD_HOVEROFF,
	CShopMain::OnBuyHoverOff)
ON_FRESH_VI("sidetab_buy", FRCMD_INIT,
	CShopMain::OnSideTabBuyInit)
ON_FRESH_VV("sidetab_buy", FRCMD_LBUTTONDOWN,
	CShopMain::OnSideTabBuyUp)
ON_FRESH_VI("sidetab_rental", FRCMD_INIT,
	CShopMain::OnSideTabRentalInit)
ON_FRESH_VV("sidetab_rental", FRCMD_LBUTTONDOWN,
	CShopMain::OnSideTabRentalUp)
ON_FRESH_VV("close_wnd", FRCMD_LBUTTONUP,
	CShopMain::OnCloseWndUp)
ON_FRESH_VI("tabtxt2", FRCMD_INIT,
	CShopMain::OnTabTxt2Init)
ON_FRESH_VI("tabtxt2", FRCMD_INIT,
	CShopMain::OnTabTxt3Init)
ON_FRESH_VI("pet", FRCMD_INIT,
	CShopMain::OnPetInit)
ON_FRESH_VI("pet", FRCMD_OWNERDRAW,
	CShopMain::OnPetOwnerDraw)
ON_FRESH_VI("skin_pet", FRCMD_INIT,
	CShopMain::OnSkinPetInit)
ON_FRESH_VI("skin_pet2", FRCMD_INIT,
	CShopMain::OnSkinPet2Init)
ON_FRESH_VI("item_desc_bg", FRCMD_INIT,
	CShopMain::OnItemDescBgInit)
ON_FRESH_VI("item_desc_bg", FRCMD_OWNERDRAW,
	CShopMain::OnItemDescBgOwnerDraw)
ON_FRESH_VI("item_desc", FRCMD_INIT,
	CShopMain::OnItemDescInit)
ON_FRESH_VI("part_cart", FRCMD_INIT,
	CShopMain::OnPartCartListInit)
ON_FRESH_VI("part_cart", FRCMD_OWNERDRAW,
	CShopMain::OnPartCartListOwnerDraw)
ON_FRESH_VI("auxpart_cart", FRCMD_INIT,
	CShopMain::OnAuxPartCartListInit)
ON_FRESH_VI("auxpart_cart", FRCMD_OWNERDRAW,
	CShopMain::OnAuxPartCartListOwnerDraw)
ON_FRESH_VI("scroll_up", FRCMD_INIT,
	CShopMain::OnScrollUpInit)
ON_FRESH_VV("scroll_up", FRCMD_LBUTTONUP,
	CShopMain::OnScrollUpUp)
ON_FRESH_VI("scroll_down", FRCMD_INIT,
	CShopMain::OnScrollDownInit)
ON_FRESH_VV("scroll_down", FRCMD_LBUTTONUP,
	CShopMain::OnScrollDownUp)
ON_FRESH_VI("item_slot_frame1", FRCMD_INIT,
	CShopMain::OnItemSlotFrame1Init)
ON_FRESH_VI("item_slot_frame2", FRCMD_INIT,
	CShopMain::OnItemSlotFrame2Init)
ON_FRESH_VI("item_slot", FRCMD_INIT,
	CShopMain::OnItemSlotInit)
ON_FRESH_VI("item_slot", FRCMD_OWNERDRAW,
	CShopMain::OnItemSlotOwnerDraw)
ON_FRESH_VI("stat_group", FRCMD_INIT,
	CShopMain::OnStatBoxInit)
ON_FRESH_VI("stat_group.arrow_m", FRCMD_INIT,
	CShopMain::OnStatArrowInit)
ON_FRESH_VI("stat_group.epower", FRCMD_INIT,
	CShopMain::OnPowerEditInit)
ON_FRESH_VI("stat_group.econtrol", FRCMD_INIT,
	CShopMain::OnControlEditInit)
ON_FRESH_VI("stat_group.eimpact", FRCMD_INIT,
	CShopMain::OnImpactEditInit)
ON_FRESH_VI("stat_group.espin", FRCMD_INIT,
	CShopMain::OnSpinEditInit)
ON_FRESH_VI("stat_group.ecurve", FRCMD_INIT,
	CShopMain::OnCurveEditInit)
END_FRESH_MSGMAP()

void __fastcall AddSellMiniButton(IFF_ITEM_COMMON& item, FrListItem*& pItem)
{
	if (item.InStock == 1 || item.InStock == 3)
		pItem->AddNoRBtnIdx(1);
	if (item.InStock == 1 || item.InStock == 2)
		pItem->AddNoRBtnIdx(2);
}

// 00a93300 (slot 32, FrForm::OnInit): monta a loja ao abrir o form
bool CShopMain::OnInit()
{
	BuildVisibleCaddieList();
	m_pPetArea = m_pPet;
	unsigned int key = CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie;
	unsigned long caddieTid;
	if (key == 0)
		caddieTid = 0;
	else
		caddieTid = CSharedDoc::Instance()->m_caddieMap[key].tid;
	key = CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot;
	unsigned long mascotTid;
	if (key == 0)
		mascotTid = 0x40000000;
	else
		mascotTid = CSharedDoc::Instance()->m_mascotMap[key].tid;
	jp983_rastro("A803 loja OnInit u524=%08x guidChar=%08x tid=%08x", m_u524,
		CSharedDoc::Instance()->m_myInfo.userEquip.guidChar,
		CSharedDoc::Instance()->m_charMap[CSharedDoc::Instance()->m_myInfo.userEquip.guidChar].tid);
	if (m_u524)
		ChangeCharacter(m_u524);
	else
	{
		key = CSharedDoc::Instance()->m_myInfo.userEquip.guidChar;
		ChangeCharacter(CSharedDoc::Instance()->m_charMap[key].tid);
	}
	ChangeCaddie(caddieTid);
	key = CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet;
	ChangeClubset(CSharedDoc::Instance()->m_clubSetMap[key].tid);
	ChangeAztec(CSharedDoc::Instance()->m_myInfo.userEquip.tidBall);
	ChangeMascot(mascotTid);
	CSharedDoc* pDoc = CSharedDoc::Instance();
	unsigned long* pTid = m_skinTid;
	memcpy(pTid, pDoc->m_myInfo.userEquip.tidSkin, sizeof(m_skinTid));
	ChangeMainTab(m_mainTab, true);
	ChangeSubTab(m_subTab, true);
	m_statTab = (statTab_t)(m_mainTab > 3);
	pDoc = CSharedDoc::Instance();
	memcpy(pTid, pDoc->m_myInfo.userEquip.tidSkin, sizeof(m_skinTid));
	for (int i = 6; i != 0; --i, ++pTid)
	{
		if (*pTid)
		{
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(*pTid);
			if (pSkin && (*pTid & 0x3c00000) == 0)
			{
				m_u724 = pSkin->HScroll;
				m_u728 = pSkin->VScroll;
				m_f71c = 0;
				m_f720 = 0;
			}
		}
	}
	if (CSharedDoc::Instance()->m_bRefreshCamera)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xce, 1, 0, 0, 0, 0));
	}
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 | 4);
	}
	return FrForm::OnInit();
}

void CShopMain::OnSortOptionComboInit(int param)
{
	FrComboBox* pWnd = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
	m_pSortOptionCombo = pWnd;
	if (pWnd)
	{
		pWnd->SetVisible(false);
		pWnd->Enable(false);
	}
}

void CShopMain::OnSortOptionComboDown()
{
}

void CShopMain::OnSearchGoodsButtonInit(int param)
{
	FrTextButton* pWnd = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
	m_pSearchGoodsButton = pWnd;
	if (pWnd)
	{
		pWnd->SetVisible(false);
		pWnd->Enable(false);
	}
}

void CShopMain::OnSearchGoodsLButtonDown()
{
}

// 983 00a822f0 / 00a822e0 / 00a822d0: so "ret"
void CShopMain::OnExportSelectListInit(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pBtn)
	{
		pBtn->SetVisible(false);
		pBtn->Enable(false);
	}
}

void CShopMain::OnExportSelectListUp()
{
}

void CShopMain::OnMainBg1Init(int param)
{
	m_pMainBg[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnMainBg2Init(int param)
{
	m_pMainBg[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnMainBg3Init(int param)
{
	m_pMainBg[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnMainBg4Init(int param)
{
	m_pMainBg[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnItemSearchBtnInit(int param)
{
	m_pItemSearchBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00a824e0
void CShopMain::OnCloseWndUp()
{
	Close(true);
}

void CShopMain::OnStatArrowInit(int param)
{
	m_pStatArrow = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 00a82300: so o dialogo de checkout (os outros dialogos do 645 nao existem no CShopMain do 983)
void CShopMain::InitShopDialogs()
{
	m_pCheckoutDlg = 0;
}

// 983 00a82310: zera os ponteiros/valores da loja (os campos do 645 fora do sizeof 0x6c0 nao sao tocados)
void CShopMain::InitShopControls()
{
	// 983 00a82310: ordem e campos do 983 (zera m_f544/m_f548 e m_u55c; m_descOffset vem de SetItemDescRect e m_prevMainTab de OnInit)
	m_pTabMain = 0;
	m_pTabSub = 0;
	m_pBuyAll = 0;
	m_pBuyReset = 0;
	m_pItemList = 0;
	m_pPartCartList = 0;
	m_pAuxPartCartList = 0;
	m_pTabTxt2 = 0;
	m_pTabTxt3 = 0;
	m_n154 = 0;
	m_pPetArea = 0;
	m_pExhibition = 0;
	m_pStatBox = 0;
	m_pStatArrow = 0;
	memset(m_pStatEdit, 0, sizeof(m_pStatEdit));
	m_f128 = -10.0f;
	m_n124 = 0;
	memset(m_skinTid, 0, sizeof(m_skinTid));
	memset(&m_u724, 0, 8); // m_u724/m_u728
	memset(&m_f71c, 0, 8); // m_f71c/m_f720 (983: memset inline, mov de eax)
	m_f544 = 0;
	m_f548 = 0;
	m_pPet = 0;
	m_pSkinPet = 0;
	m_pItemSlotFrame1 = 0;
	m_pItemSlotFrame2 = 0;
	m_u55c = 0;
	m_pBmpSelArrow = 0;
	memset(m_partGuid983, 0, sizeof(m_partGuid983));
	m_pBmpMarkBegin = 0;
	m_nRookieLv1 = 0;
	m_nRookieLv = 0;
	m_pBmpSelectMark = 0;
}

void CShopMain::CloseAllDialogs()
{
	FrWnd* pWnd = DYNAMIC_CAST(FrWnd,
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop"));
	if (pWnd)
		pWnd->Close(true);
}

// 00a84790 (slot 6, FrWnd::Enable): a loja some junto quando desabilitada e leva o quadro de status (+0x15c)
void CShopMain::Enable(bool enable)
{
	FrWnd::Enable(enable);
	FrWnd::SetVisible(enable);
	if (m_pStatBox)
	{
		m_pStatBox->Enable(enable);
		m_pStatBox->SetVisible(enable);
	}
}

void CShopMain::TurnOnItemStatus()
{
	m_pStatBox->SetVisible(true);
	SetLevelBar();
}

void CShopMain::TurnOffItemStatus()
{
	m_pStatBox->SetVisible(false);
	SetLevelBar();
}

// 009 a89000 (JP 983): comparador da lista de pecas. Diferencas p/ o 645: (1) tids do set em CSharedDoc+0x113e4
// vao para o fim/inicio; (2) item com DateFlag == 2 so conta se estiver no periodo de venda (IsInSalePeriod);
// (3) tid especial (tabela de 35 em 00d45768) antes dos periodos; (4) nivel do jogador (+0x91d) e IsUnderLvl.
bool IsUccClothes(unsigned long tid);
// E422: parte especial: o 983 (a93799, a913d9, a8cf5b) le um dword ("guid") e uma string ("UccIndex") logo DEPOIS do
// sPart (0x238), no rabo do no do std::map<uint,sPart> do ItemManager. Nao e campo: VC10 = _Color/_Isnil e fora do bloco
// (D808); cl 14.44 = fora do bloco. Lixo nos dois compiladores; nao vem do servidor nem do IFF.
#define SPARTTAIL983(p) ((const char*)(p) + sizeof(IFF_STRUCT::sPart))
static bool IsSpecialPartTid983(unsigned long tid) // 00a82ce0
{
	static const unsigned long tbl[35] = {
		0x08006055, 0x0800a035, 0x08044085, 0x08048025, 0x080440b3,
		0x08084044, 0x0808a025, 0x080c408b, 0x080ca023, 0x080c40b2,
		0x0810404f, 0x08108030, 0x08144084, 0x0814a02c, 0x081440bf,
		0x08184087, 0x08188020, 0x081840c7, 0x081c404b, 0x081ca01f,
		0x0820404a, 0x0820a00e, 0x08204091, 0x08244064, 0x0824a004,
		0x0824408f, 0x08284026, 0x08284027, 0x0828a012, 0x0830401b,
		0x0830401e, 0x0830a00c, 0x0838400e, 0x08384011, 0x0838a006,
	};
	for (unsigned int i = 0; i < 35; i++)
		if (tbl[i] == tid)
			return true;
	return false;
}

// 0062fd70 (CSharedDoc): como o BuildMyPartTidListDefault, mas mantem as partes atuais e so zera as 5 primeiras que forem
// especiais (tid e uid); nao aplica o padrao nem limpa os acessorios
static bool StripSpecialPartTids983(unsigned long typeId)
{
	CSharedDoc* doc = CSharedDoc::Instance();
	IFF_STRUCT::sChar* pChar = doc->m_itemManager.FindChar(typeId);
	if (!pChar)
		return false;
	CPartTidList& tids = doc->GetMyPartTidList();
	tids.m_charTid = typeId;
	tids.m_defPartNum = pChar->nParts;
	tids.m_partNum = pChar->nParts + pChar->nAcsries;
	tids.m_hairColor = 0;
	tids.m_shirtsColor = 0;
	for (int i = 0; i < 5; i++)
	{
		if (IsSpecialPartTid983(tids.m_tid[i]))
		{
			tids.m_tid[i] = 0;
			tids.m_uid[i] = 0;
		}
	}
	return true;
}

bool __fastcall PartCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;
	if (b == NULL)
		return true;

	const IFF_ITEM_COMMON* pA = (const IFF_ITEM_COMMON*)a;
	const IFF_ITEM_COMMON* pB = (const IFF_ITEM_COMMON*)b;

	// tids fixados = mapa do pacote 0x20e (CSharedDoc+0x113e0, cabeca em +0x113e4): o primeiro que bate decide
	for (std::map<unsigned long, sShopSaleState983>::const_iterator it =
			 CSharedDoc::Instance()->m_shopSaleState983.begin();
		it != CSharedDoc::Instance()->m_shopSaleState983.end(); ++it)
	{
		// o 983 le o tid gravado no valor (no +0x10), igual a chave
		unsigned long tid = it->second.typeId;
		if (pA->TypeId == tid)
			return true;
		if (pB->TypeId == tid)
			return false;
	}

	if (pB->DateFlag == 2 &&
		ItemManager()->IsInSalePeriod(pB->TypeId, CSharedDoc::Instance()->GetServerTime()))
		return false;

	if (UccManager()->IsDisplayUccClothes(pA) &&
		!UccManager()->IsDisplayUccClothes(pB))
		return true;
	if (UccManager()->IsDisplayUccClothes(pB) &&
		!UccManager()->IsDisplayUccClothes(pA))
		return false;
	if (UccManager()->IsDisplayUccClothes(pA) &&
		UccManager()->IsDisplayUccClothes(pB))
	{
		const sDisplayUccClothes* pUccA = (const sDisplayUccClothes*)a;
		const sDisplayUccClothes* pUccB = (const sDisplayUccClothes*)b;
		if (pUccA->rank < pUccB->rank)
			return true;
		if (pUccA->rank == pUccB->rank)
			return pUccA->partTypeId < pUccB->partTypeId;
		if (pUccA->rank > pUccB->rank)
			return false;
	}

	if (IsSpecialPartTid983(pA->TypeId) == true && IsSpecialPartTid983(pB->TypeId) == false)
		return true;

	_SYSTEMTIME* pStartA = NULL;
	_SYSTEMTIME* pEndA = NULL;
	_SYSTEMTIME* pStartB = NULL;
	_SYSTEMTIME* pEndB = NULL;
	bool bSaleA = ItemManager()->HasSalePeriod(pA->TypeId, &pStartA, &pEndA);
	bool bSaleB = ItemManager()->HasSalePeriod(pB->TypeId, &pStartB, &pEndB);
	// item com DateFlag == 2 fora do periodo de venda nao conta como "em promocao"
	if (bSaleA && pA->DateFlag == 2 &&
		!ItemManager()->IsInSalePeriod(pA->TypeId, CSharedDoc::Instance()->GetServerTime()))
		bSaleA = false;
	if (bSaleB && pB->DateFlag == 2 &&
		!ItemManager()->IsInSalePeriod(pB->TypeId, CSharedDoc::Instance()->GetServerTime()))
		bSaleB = false;
	if (!bSaleA)
	{
		if (bSaleB)
			return false;
	}
	else
	{
		if (!bSaleB)
			return true;
		if ((pStartA->wYear != 0 || pEndA->wYear != 0) && pStartB->wYear == 0 &&
			pEndB->wYear == 0)
			return true;
		if (pStartA->wYear == 0 && pEndA->wYear == 0 &&
			(pStartB->wYear != 0 || pEndB->wYear != 0))
			return false;
	}

	if (pA->New && !pB->New)
		return true;
	if (pB->New && !pA->New)
		return false;
	if (pA->Hit && !pB->Hit)
		return true;
	if (pB->Hit && !pA->Hit)
		return false;

	if (pA->SalePrice != 0 && pA->SalePrice < pA->Price &&
		(pB->SalePrice == 0 || pB->SalePrice >= pB->Price))
		return true;
	if (pB->SalePrice != 0 && pB->SalePrice < pB->Price &&
		(pA->SalePrice == 0 || pA->SalePrice >= pA->Price))
		return false;

	unsigned char myLevel = CSharedDoc::Instance()->m_myInfo.stat.Level;
	if (!pA->IsUnderLvl)
	{
		if (pA->Level > myLevel)
			return false;
	}
	else if (!pB->IsUnderLvl && pB->Level > myLevel)
		return true;
	if (pA->IsUnderLvl == 1)
	{
		if (pA->Level < myLevel)
			return false;
	}
	else if (pB->IsUnderLvl && pB->Level < myLevel)
		return true;

	if ((pA->TypeId & 0xfc000000) == 0x8000000 &&
		(pB->TypeId & 0xfc000000) == 0x8000000)
	{
		const IFF_STRUCT::sPart* pPartA = (const IFF_STRUCT::sPart*)a;
		const IFF_STRUCT::sPart* pPartB = (const IFF_STRUCT::sPart*)b;
		if (pPartA->Category < pPartB->Category)
			return true;
		if (pPartA->Category > pPartB->Category)
			return false;
	}
	return pB->TypeId < pA->TypeId;
}

bool __fastcall SpecialHitNewCompare(const void* a, const void* b)
{
	const IFF_ITEM_COMMON* pA = (const IFF_ITEM_COMMON*)a;
	const IFF_ITEM_COMMON* pB = (const IFF_ITEM_COMMON*)b;
	// 983 00a85400: item com DateFlag == 2 dentro do periodo de venda vai depois; fora dele nao conta como "em promocao"
	if (pB->DateFlag == 2 &&
		ItemManager()->IsInSalePeriod(pB->TypeId, CSharedDoc::Instance()->GetServerTime()))
		return false;
	_SYSTEMTIME* pStartA = NULL;
	_SYSTEMTIME* pEndA = NULL;
	_SYSTEMTIME* pStartB = NULL;
	_SYSTEMTIME* pEndB = NULL;
	bool bSaleA = ItemManager()->HasSalePeriod(pA->TypeId, &pStartA, &pEndA);
	bool bSaleB = ItemManager()->HasSalePeriod(pB->TypeId, &pStartB, &pEndB);
	if (bSaleA && pA->DateFlag == 2 &&
		!ItemManager()->IsInSalePeriod(pA->TypeId, CSharedDoc::Instance()->GetServerTime()))
		bSaleA = false;
	if (bSaleB && pB->DateFlag == 2 &&
		!ItemManager()->IsInSalePeriod(pB->TypeId, CSharedDoc::Instance()->GetServerTime()))
		bSaleB = false;
	if (!bSaleA)
	{
		if (bSaleB)
			return false;
	}
	else
	{
		if (!bSaleB)
			return true;
		if ((pStartA->wYear != 0 || pEndA->wYear != 0) && pStartB->wYear == 0 &&
			pEndB->wYear == 0)
			return true;
		if (pStartA->wYear == 0 && pEndA->wYear == 0 &&
			(pStartB->wYear != 0 || pEndB->wYear != 0))
			return false;
	}

	if (pA->New && !pB->New)
		return true;
	if (pB->New && !pA->New)
		return false;
	if (pA->Hit && !pB->Hit)
		return true;
	if (pB->Hit && !pA->Hit)
		return false;
	// 983 00a85400: itens com nivel acima do jogador (CSharedDoc+0x91d) vao para o fim
	unsigned char lv = CSharedDoc::Instance()->m_myInfo.stat.Level;
	if (!pA->IsUnderLvl)
	{
		if (pA->Level > lv)
			return false;
	}
	else if (!pB->IsUnderLvl && pB->Level > lv)
		return true;
	if (pA->IsUnderLvl)
	{
		if (pA->Level < lv)
			return false;
	}
	else if (pB->IsUnderLvl && pB->Level < lv)
		return true;
	return pB->TypeId < pA->TypeId;
}

bool __fastcall SkinCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;
	if (b == NULL)
		return true;
	return ((const unsigned long*)a)[1] < ((const unsigned long*)b)[1];
}

bool __fastcall ShopEmptyCompare(const void* a, const void* b)
{
	if (a != NULL && b == NULL)
		return true;
	return false;
}

// 983 a82470: itens com DateFlag == 2 vao depois; senao ordena pelo TypeId (+4)
// (o 983 compara o bit New com 2, sempre diferente: com B de periodo o resultado e sempre false)
bool __fastcall CaddieCompare(const void* a, const void* b)
{
	const IFF_ITEM_COMMON* pA = (const IFF_ITEM_COMMON*)a;
	const IFF_ITEM_COMMON* pB = (const IFF_ITEM_COMMON*)b;
	if (pA->DateFlag == 2 && pB->DateFlag != 2)
		return true;
	if (pB->DateFlag == 2 && pA->New != 2)
		return false;
	return pA->TypeId < pB->TypeId;
}

void CShopMain::OnPetInit(int param)
{
	m_pPet = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnSkinPetInit(int param)
{
	m_pSkinPet = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnSkinPet2Init(int param)
{
	m_pSkinPet2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::DrawSkinFrame()
{
	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	if (!gfx)
		return;
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBmp;
	CItemManager* pItemMgr = ItemManager();
	if (m_skinTid[0] && (pSkin = pItemMgr->FindSkin(m_skinTid[0])) != NULL &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		// 983 00a85610: ladrilha o fundo a partir do deslocamento (m_f71c, m_f720); cada ladrilho usa
		// dst.w == src.w e dst.h == src.h (o 645 deixava dst.w/src.h do primeiro e esticava)
		WRect area = m_pPetArea->GetRect();
		float x0 = area.x;
		float y = area.y;
		float w = area.w;
		float h = area.h;
		float bw = (float)pBmp->Width();
		float bh = (float)pBmp->Height();
		if (m_f71c > bw)
			m_f71c = m_f71c - bw;
		else if (m_f71c < 0.0f)
			m_f71c = bw + m_f71c;
		if (m_f720 > bh)
			m_f720 = m_f720 - bh;
		else if (m_f720 < 0.0f)
			m_f720 = bh + m_f720;
		WRect src(m_f71c, m_f720, bw - m_f71c, bh - m_f720);
		WRect dst(x0, y, src.w, src.h);
		float yEnd = h + y;
		if (dst.y < yEnd)
		{
			float xEnd = w + x0;
			do
			{
				while (dst.x < xEnd)
				{
					gfx->DrawTexture(pBmp, src, dst, 0xffffffff, 0);
					dst.x = dst.w + dst.x;
					float rest = xEnd - dst.x;
					dst.w = (rest < bw) ? rest : bw;
					src.x = 0;
					src.w = dst.w;
				}
				dst.y = dst.h + dst.y;
				src.w = bw - m_f71c;
				float rest = yEnd - dst.y;
				dst.h = (rest < bh) ? rest : bh;
				src.x = m_f71c;
				src.y = 0;
				src.h = dst.h;
				dst.x = x0;
				dst.w = src.w;
			} while (dst.y < yEnd);
		}
	}
	// 983 00a85610: a vitrine (CExhibition) so e desenhada nas sub-abas de skin 0xe..0x10
	if ((m_subTab == SUBTAB_SKIN || m_subTab == 0xf || m_subTab == 0x10) && !m_hand.IsWorking(0))
	{
		LOCK_USER(&m_hand);
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		UNLOCK_USER(&m_hand);
	}
	for (int i = 1; i <= 2; ++i)
	{
		IFF_STRUCT::sSkin* pS;
		const Bitmap* pB;
		pItemMgr = ItemManager();
		if (m_skinTid[i] && (pS = pItemMgr->FindSkin(m_skinTid[i])) != NULL &&
			(pB = g_pFresh->GetBitmap(pS->Tex)) != NULL)
			gfx->DrawTexture(pB, m_pPetArea->GetRect(), 0xffffffff, 0);
	}
}

// 983 00e35138: 15 entradas (indice 0..0xe)
__declspec(selectany) unsigned long g_slotCategory[15] = { 3, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 0, 0, 0, 0 };

void CShopMain::DrawSkinSlot()
{
	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	if (!gfx)
		return;
	// 983 00a85a00: origem (116, 308) somada ao rect do form da widget do task atual (GetWidgetForm983);
	// na Decoration o form fica em (640, 150) no 983 (C326.1 mediu 185,387 -> 825,537)
	float x = 116.0f;
	float y = 308.0f;
	if (FrTaskForm* pTaskForm = AfxGetTask()->GetWidgetForm983())
	{
		x = pTaskForm->GetRect().x + 116.0;
		y = pTaskForm->GetRect().y + 308.0;
	}
	gfx->DrawTexture(m_pBmpTeamBase, WRect(x, y, (float)m_pBmpTeamBase->Width(),
		(float)m_pBmpTeamBase->Height()), 0xffffffff, 0);
	IFF_STRUCT::sSkin* pSkin;
	IFF_STRUCT::sChar* pChar;
	const Bitmap* pBmp;
	if (m_skinTid[3] &&
		(pSkin = ItemManager()->FindSkin(m_skinTid[3])) != NULL &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		gfx->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()),
			0xffffffff, 0);
	}
	pChar = ItemManager()->FindChar(m_u524);
	if (pChar && (pBmp = g_pFresh->GetBitmap(pChar->c.Icon)) != NULL)
	{
		gfx->DrawTexture(pBmp, WRect(x + 8.0, y + 36.0, (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
	}
	if (m_skinTid[4] &&
		(pSkin = ItemManager()->FindSkin(m_skinTid[4])) != NULL &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		gfx->DrawTexture(pBmp, WRect(x + 8.0, y + 36.0, (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
	}
	// 983 00a85c7a: sexo = bit 3 do byte logo apos dwEmblemVer (nossa sPangYaUserInfo o tem no bit 1)
	const Bitmap* pGender = (&m_pBmpMale)[
		(((unsigned char*)&CSharedDoc::Instance()->m_myInfo.info.dwEmblemVer)[4] >> 3) & 1];
	gfx->DrawTexture(pGender, WRect(x + 5.0, y + 98.0, (float)pGender->Width(),
		(float)pGender->Height()), 0xffffffff, 0);
	const char* levelName;
	if (m_skinTid[5] == 0 || (pSkin = ItemManager()->FindSkin(m_skinTid[5])) == NULL)
		levelName = MakeStr("level_%03d", CSharedDoc::Instance()->m_myInfo.stat.Level + 1);
	else
		levelName = pSkin->c.Icon;
	pBmp = g_pFresh->GetBitmap(levelName);
	if (pBmp)
	{
		gfx->DrawTexture(pBmp, WRect(
			(float)((x + 90.0) - (int)(pBmp->Width() * 0.5)),
			(float)((y + 78.0) - (int)(pBmp->Height() * 0.5)),
			(float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	}
	gfx->DrawTexture(m_pBmpMasterSign, WRect(
		(float)(int)((x + 63.5) - m_pBmpMasterSign->Width() * 0.5), y + 5.0,
		(float)m_pBmpMasterSign->Width(), (float)m_pBmpMasterSign->Height()), 0xffffffff, 0);
	gfx->SetTextColor(0xffffffff, 0xff808080);
	gfx->SetTextStyle(2);
	WPoint pt(x + 23.0, y + 107.0);
	g_pFresh->GetManager()->PrintText(pt, 0,
		CSharedDoc::Instance()->m_myInfo.info.sNick, -1.0f, 0xffffffff);
}

void CShopMain::DrawCutinFrame()
{
	IFF_ITEM_COMMON* pItem;
	if (m_subTab == 0x22 && m_pItemList->GetSelected() &&
		(pItem = (IFF_ITEM_COMMON*)m_pItemList->GetSelected()->pData) != NULL)
	{
		WOverlay* pImg = GetCutinImg(pItem->TypeId);
		if (pImg)
		{
			// 983 00a909d0: origem (72, 66) somada ao rect do form da widget do task atual (GetWidgetForm983);
			// tamanho preenchido depois, origem = g_rectUnitW983
			WRect rc;
			rc.x = 72.0f;
			rc.y = 66.0f;
			if (FrTaskForm* pTaskForm = AfxGetTask()->GetWidgetForm983())
			{
				rc.x += pTaskForm->GetRect().x;
				rc.y += pTaskForm->GetRect().y;
			}
			rc.w = (float)pImg->GetWidth();
			rc.h = (float)pImg->GetHeight();
			pImg->Render(g_view, g_rectUnitW983, rc, 0, 0xffffffff, 0, 0);
		}
	}
}

WOverlay* CShopMain::GetCutinImg(unsigned long tid)
{
	WOverlay* pImg;
	std::map<unsigned int, WOverlay*>::iterator it = m_cutinImgMap.find(tid);
	if (it == m_cutinImgMap.end())
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
		if (pSkin)
		{
			char name[64];
			sprintf(name, "%s.jpg", pSkin->Tex);
			pImg = g_resrcmng->GetOverlay(name, 0);
			if (pImg)
				m_cutinImgMap.insert(std::map<unsigned int, WOverlay*>::value_type(
					pSkin->c.TypeId, pImg));
		}
		else
			return NULL;
	}
	else
		pImg = (*it).second;
	return pImg;
}

void CShopMain::DeleteCutinImages()
{
	for (std::map<unsigned int, WOverlay*>::iterator it = m_cutinImgMap.begin();
		it != m_cutinImgMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
}

void CShopMain::OnPetOwnerDraw(int param)
{
	// A691: 983 00a90aa0 nao desenha nada com o lobby travado (RFLobbyDesktop 00e78ca8, +0x240)
	RFLobbyDesktop* desk = RFLobbyDesktop::Instance983();
	if (desk && desk->m_bLocked)
		return;
	if (m_mainTab == 5)
	{
		switch (m_subTab)
		{
		case 0xe:
		case 0xf:
		case 0x10:
			DrawSkinFrame();
			return;
		case 0x11:
		case 0x23:
			DrawSkinSlot();
			return;
		case 0x22:
			DrawCutinFrame();
			return;
		}
	}
	// A293: 983 00a90b54 testa a flag da loja do bongdari no CSharedDoc (+0x11410). O +0x122 do 645 aqui cai DENTRO do vptr do
	// CHandOwner (FrTaskForm = 0x120): byte sempre != 0 -> a previa de taco/bola/item da loja nunca era desenhada.
	else if (!m_hand.IsWorking(0) && !Doc()->m_bBongdariShopOpen &&
		!CRankingInfo::Instance()->GetDlg() &&
		(!CUserInfo::Instance() || !CUserInfo::Instance()->GetDlg()))
	{
		m_hand.Enter();
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		m_hand.Leave();
	}
}

void CShopMain::OnItemDescBgInit(int param)
{
	m_pItemDescBg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnItemDescBgOwnerDraw(int param)
{
	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	if (gfx && m_pItemDesc->GetLineNum() > 0)
	{
		float w1 = (float)m_pBmpItemDesc1->Width();
		float h1 = (float)m_pBmpItemDesc1->Height();
		float w2 = (float)m_pBmpItemDesc2->Width();
		float h2 = (float)m_pBmpItemDesc2->Height();
		float y = m_descOffset.y + 320.0f;
		gfx->DrawTexture(m_pBmpItemDesc1, WRect(m_descOffset.x + 6.0f, y, w1, h1), 0xffffffff, 0);
		gfx->DrawTexture(m_pBmpItemDesc2, WRect(m_descOffset.x + 262.0f, y, w2, h2), 0xffffffff, 0);
	}
}

void CShopMain::OnItemDescInit(int param)
{
	m_pItemDesc = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pItemDesc)
	{
		m_pItemDesc->SetVisible(false);
		m_f554 = m_pItemDesc->GetRect().x;
		m_f558 = m_pItemDesc->GetRect().h + m_pItemDesc->GetRect().y;
	}
}

void CShopMain::SetItemDescRect()
{
	m_pItemDesc->ClearLine();

	switch ((int)m_subTab)
	{
	case 7:
	case 0xb:
		m_descOffset = WPoint(0.0f, 30.0f);
		break;
	case 8:
	case 0xd:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
		m_descOffset = WPoint(0.0f, 120.0f);
		break;
	default:
		m_descOffset = WPoint(0.0f, 0.0f);
		break;
	}
}

void CShopMain::ShowItemDesc(unsigned long tid)
{
	IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(tid);
	if (pDesc)
	{
		m_pItemDesc->ClearLine();
		m_pItemDesc->AddText(pDesc->Desc, false, true);
		int lines = m_pItemDesc->GetLineNum();
		if (lines > 2)
		{
			// 983 00a85ee0: conta das linhas a apagar, linhas -= conta, do/while
			int n = lines - 2;
			lines -= n;
			do
				m_pItemDesc->DeleteEndLine();
			while (--n);
		}
		float h = (float)m_pItemDesc->GetLineHeight() * (float)lines;
		float w = m_pItemDesc->GetRect().w;
		m_pItemDesc->SetRect(WRect(m_f554 + m_descOffset.x, (m_f558 + m_descOffset.y) - h, w, h));
		m_pItemDesc->GetScrollBar()->ScrollToFirst();
	}
}


// 983 00f02ed4: periodos de venda {texto ja passado pelo K2L, dias}, montados no inicializador 00ceb350
struct sShopPeriod983
{
	const char* name;
	int days;
};
static sShopPeriod983 g_shopPeriod983[6] = {
	{ K2L_Compatibility("1\xc0\xcf"), 1 },
	{ K2L_Compatibility("7\xc0\xcf"), 7 },
	{ K2L_Compatibility("15\xc0\xcf"), 15 },
	{ K2L_Compatibility("30\xc0\xcf"), 30 },
	{ K2L_Compatibility("365\xc0\xcf"), 365 },
	{ K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), 0 }
};

void CShopMain::DrawItemPrice(FrGraphicInterface* gfx, float x, float y,
	IFF_ITEM_COMMON& item, FrListItem* pItem)
{
	if (!pItem || item.InStock == 4)
		return;

	// 983 00a85fe0: com DateFlag == 2 manda o periodo de venda (IsInSalePeriod); senao, SalePrice < Price
	bool sale;
	if (item.DateFlag == 2)
	{
		CSharedDoc* pDoc = CSharedDoc::Instance();
		sale = ItemManager()->IsInSalePeriod(item.TypeId, pDoc->GetServerTime());
	}
	else
		sale = item.SalePrice != 0 && item.SalePrice < item.Price;

	const char* str;
	if (sale)
	{
		gfx->SetTextColor(0xffff0000, 0xffffffff);
		if (item.IsCash)
			str = MakeStr(K2L_Compatibility("%d\xc4\xed\xc5\xb0"), item.SalePrice);
		else
			str = MakeStr(K2L_Compatibility("%d\xc6\xce"), item.SalePrice);
		float w1 = gfx->Print11(WPoint(pItem->pos.x + x, pItem->pos.y + y), 2, str);
		gfx->SetTextColor(0xff000000, 0xffffffff);
		float x2 = x - w1;
		gfx->SetTextColor(0xff000000, 0xffffffff);
		x2 = (float)(x2 - 7.0);
		if (item.IsCash)
			str = MakeStr(K2L_Compatibility("%d\xc4\xed\xc5\xb0"), item.Price);
		else
			str = MakeStr(K2L_Compatibility("%d\xc6\xce"), item.Price);
		float w2 = gfx->Print11(WPoint(pItem->pos.x + x2, pItem->pos.y + y), 2, str);
		gfx->SetTextColor(0xff000000, 0xffffffff);
		float xr = (float)(x2 + 1.0);
		float yy = float2int((float)(gfx->GetFontHeight11() * 0.5)) + y;
		WPoint to(pItem->pos.x + xr, yy + pItem->pos.y);
		float left = (float)(x2 - float2int(w2) - 1.0);
		yy = float2int((float)(gfx->GetFontHeight11() * 0.5)) + y;
		WPoint from(pItem->pos.x + left, yy + pItem->pos.y);
		g_view->DrawLine2D(from, to, 0xff000000, 0xff000000, 0);
		return;
	}

	unsigned long tid = item.TypeId;
	if ((tid & 0xfc000000) == 0x38000000 && ((tid >> 22) & 0xf) != 6 &&
		((tid >> 22) & 0xf) != 5)
	{
		// 983: skin com precos por periodo (SkinCom[5] = precos de 1/7/15/30 dias e ilimitado)
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
		if (!pSkin)
			return;
		float maxw = 0.0f;
		for (int i = 0; i < 5; ++i)
		{
			if (pSkin->COM[i] > 0)
			{
				float w = gfx->GetTextExtendOld983(g_shopPeriod983[i].name);
				if (maxw < w)
					maxw = w;
			}
		}
		float yoff = -5.0f;
		for (int i = 0; i < 5; ++i)
		{
			if (pSkin->COM[i] <= 0)
				continue;
			const char* fmt = K2L_Compatibility(
				item.IsCash ? "\xc4\xed\xc5\xb0 %d\xb0\xb3 / " : "%d\xc6\xce / ");
			str = MakeStr(fmt, (int)pSkin->COM[i]);
			float yp = yoff + y;
			gfx->Print(WPoint(pItem->pos.x + (x - maxw), pItem->pos.y + yp), 2, str);
			gfx->Print(WPoint(pItem->pos.x + x, pItem->pos.y + yp), 2, g_shopPeriod983[i].name);
			yoff = (float)(yoff + 13.0);
		}
		return;
	}

	if (item.IsCash)
		str = MakeStr(K2L_Compatibility("%d\xc4\xed\xc5\xb0"), item.Price);
	else
		str = MakeStr(K2L_Compatibility("%d\xc6\xce"), item.Price);
	gfx->Print(WPoint(pItem->pos.x + x, pItem->pos.y + y), 2, str);
}


void CShopMain::OnStatBoxInit(int param)
{
	m_pStatBox = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
}


void CShopMain::OnTabTxt2Init(int param)
{
	m_pTabTxt2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CShopMain::OnTabTxt3Init(int param)
{
	m_pTabTxt3 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}


void CShopMain::OnTabMainInit(int param)
{
	FrTabButton* pWnd = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	m_pTabMain = pWnd;
	if (pWnd)
	{
		// 983 00a83790: indice 1 = stab_7 (Clothes), indices 2..5 = stab_2..stab_5
		const char* names[3] = { 0, 0, 0 };
		names[0] = MakeStr("stab_%d_n", 1);
		names[1] = MakeStr("stab_%d_d", 1);
		names[2] = MakeStr("stab_%d_d", 1);
		m_pTabMain->AddTab(names, 0);
		names[0] = MakeStr("stab_%d_n", 7);
		names[1] = MakeStr("stab_%d_d", 7);
		names[2] = MakeStr("stab_%d_d", 7);
		m_pTabMain->AddTab(names, 1);
		for (int i = 2; i < 6; ++i)
		{
			names[0] = MakeStr("stab_%d_n", i);
			names[1] = MakeStr("stab_%d_d", i);
			names[2] = MakeStr("stab_%d_d", i);
			m_pTabMain->AddTab(names, i);
		}
	}
}

// 983 00a92e20: as abas principais do 983 sao 0 personagem, 1 (novo, mesma ChangeCharacter), 2 taco, 3 caddie, 4 mascote, 5 skin
void CShopMain::OnTabMainButtonUp()
{
	TButtonItem* pSel = m_pTabMain->GetSelected();
	if (pSel && (mainTab_t)pSel->index != m_mainTab)
	{
		ChangeMainTab((mainTab_t)pSel->index, false);
		switch (pSel->index)
		{
		case 0:
			ChangeSubTab((subTab_t)0, false);
			ChangeCharacter(m_u524);
			break;
		case 1:
			ChangeSubTab(SUBTAB_SET, false);
			ChangeCharacter(m_u524);
			break;
		case 2:
			ChangeSubTab(SUBTAB_CLUBSET, false);
			ChangeClubset(m_clubsetTid);
			break;
		case 3:
			ChangeSubTab(SUBTAB_CADDIE, false);
			ChangeCaddie(m_u528);
			break;
		case 4:
			ChangeSubTab(SUBTAB_MASCOT, false);
			ChangeMascot(m_mascotTid);
			break;
		case 5:
			ChangeSubTab(SUBTAB_SKIN, false);
			ChangeSkin(m_u524);
			break;
		}
	}
}

void CShopMain::OnTabSubInit(int param)
{
	m_pTabSub = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
}

void CShopMain::OnTabSubButtonUp()
{
	TButtonItem* pSel = m_pTabSub->GetSelected();
	if (pSel)
		ChangeSubTab((subTab_t)pSel->index, false);
}

void CShopMain::OnItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pItemList)
	{
		m_pItemList->UseRightButton(true);
		m_pItemList->UseDummy(true);
		m_pItemList->SetPushSound(NULL);
		m_pItemList->AddNoRButtonRect(m_rowBtn983[ROWBTN983_BUY].Rc(), 1);
		m_pItemList->AddNoRButtonRect(m_rowBtn983[ROWBTN983_GIFT].Rc(), 2);
	}
}

void CShopMain::DrawStatus(FrGraphicInterface* gfx, FrListItem* item,
	short* const a, short* const b, float y)
{
	// 983 00a83a10: x de partida 86 (d13f6c); primeiro a[i] com [i][0], depois b[i] com [i][1]
	float yy = 4.0f;

	for (int i = 0; i < 5; i++)
	{
		Bitmap* bmpA = m_pStatBmp[i][0];
		Bitmap* bmpB = m_pStatBmp[i][1];
		int na = a[i];
		int nb = b[i];

		if (nb != 0 || na != 0)
		{
			float x = 86.0f;

			for (int j = 0; j < na; j++)
			{
				WRect rc(x + item->pos.x + y, item->pos.y + yy + y, 5.0f, 6.0f);
				gfx->DrawTexture(bmpA, rc, 0xffffffff, 0);
				x -= 6.0;
			}

			for (int j = 0; j < nb; j++)
			{
				WRect rc(x + item->pos.x + y, item->pos.y + yy + y, 5.0f, 6.0f);
				gfx->DrawTexture(bmpB, rc, 0xffffffff, 0);
				x -= 6.0;
			}

			yy += 6.0;
		}
	}
}

C_ASSERT(offsetof(IFF_STRUCT::sClubSet, Attr) == 0xd0 && offsetof(IFF_STRUCT::sClubSet, Slot) == 0xda);
C_ASSERT(offsetof(IFF_STRUCT::sCaddie, Attr) == 0xec);
C_ASSERT(offsetof(IFF_STRUCT::sMascot, Attr) == 0x115);

// 983 00a89380 (+ texto de b30340): botao "Stats" do item; sob o mouse mostra os 5 atributos (a[i]/b[i]) como dica da lista
void CShopMain::DrawStatView983(FrGraphicInterface* gfx, FrListItem* item,
	const short* stats)
{
	WRect rc(m_rowBtn983[ROWBTN983_STATE].Rc().x + item->pos.x, item->pos.y + m_rowBtn983[ROWBTN983_STATE].Rc().y,
		m_rowBtn983[ROWBTN983_STATE].Rc().w, m_rowBtn983[ROWBTN983_STATE].Rc().h);
	if (item->underCursor && rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		gfx->DrawTexture(m_rowBtn983[ROWBTN983_STATE].pBmpO, rc, 0xffffffff, 0);
		std::string text;
		for (int i = 0; i < 5; ++i)
		{
			const char* line = MakeStatusString(i, stats[i], stats[i + 5]);
			if (*line)
			{
				text += line;
				text += '\n';
			}
		}
		text.erase(text.find_last_not_of('\n') + 1);
		text.erase(0, text.find_first_not_of('\n'));
		if (!text.empty())
		{
			m_pItemList->m_nFlags.Turn(FWF_TOOLTIPS, true);
			m_pItemList->SetToolTipText(text);
			m_n6a8 = item->idx;
		}
		return;
	}
	gfx->DrawTexture(m_rowBtn983[ROWBTN983_STATE].pBmpN, rc, 0xffffffff, 0);
	if (m_n6a8 == item->idx)
	{
		m_pItemList->SetToolTipText(std::string());
		m_pItemList->m_nFlags.Turn(FWF_TOOLTIPS, false);
		m_n6a8 = -1;
	}
}

void CShopMain::DrawStatus2(FrGraphicInterface* gfx, FrListItem* item,
	short* const a, short* const b, float y)
{
	float yy = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		if (a[i] != 0 || b[i] != 0)
		{
			gfx->SetTextColor(0xff0000ff, 0xffffffff);
			WPoint off(y + 182.0f, (y + 36.0f) + yy);
			WPoint pt(item->pos.x + off.x, item->pos.y + off.y);
			gfx->Print11(pt, 2, MakeStatusString(i, a[i], b[i]));
			yy += 14.0f;
		}
	}
}

void CShopMain::DrawStatusCard(FrGraphicInterface* gfx, FrListItem* item,
	short* const a, float y)
{
	float yy = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		if (a[i] != 0)
		{
			gfx->SetTextColor(0xff0000ff, 0xffffffff);
			WPoint off(y + 182.0f, (y + 36.0f) + yy);
			WPoint pt(item->pos.x + off.x, item->pos.y + off.y);
			gfx->Print11(pt, 2, MakeCardStatusString(i, a[i]));
			yy += 14.0f;
		}
	}
}

// 983: sem funcao propria (inline no chamador, a94305). So a marca de iniciante: os encaixes de carta
// (00b2fca0) sao das janelas de carta (007fef10, 00807010, 00807960), nunca da lista da loja
__forceinline void CShopMain::DrawPartsSlot(FrGraphicInterface* gfx, FrListItem* pItem)
{
	IFF_STRUCT::sPart* pPart = (IFF_STRUCT::sPart*)pItem->pData;
	if (pPart && pPart->Beginner && m_pBmpMarkBegin)
	{
		WRect dst(pItem->pos.x + 5.0f, pItem->pos.y + 65.0f,
			(float)m_pBmpMarkBegin->Width(), (float)m_pBmpMarkBegin->Height());
		gfx->DrawTexture(m_pBmpMarkBegin, dst, 0xffffffff, 0);
	}
}

void CShopMain::OnItemListOwnerDraw(int param)
{
	FrListItem* li = (FrListItem*)param;
	if (li == NULL)
		return;

	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	IFF_ITEM_COMMON* item = (IFF_ITEM_COMMON*)li->pData;
	Bitmap* bmp;
	if (item == NULL)
	{
		bmp = m_pItemBase[3];
	}
	else
	{
		if (li->selected)
		{
			bmp = m_pItemBase[2];
			gfx->SetTextColor(0xffff0000, 0xffffffff);
			gfx->SetTextStyle(0);
		}
		else if (li->underCursor)
		{
			bmp = m_pItemBase[1];
			gfx->SetTextColor(0xff000000, 0xffffffff);
			gfx->SetTextStyle(0);
		}
		else
		{
			bmp = m_pItemBase[0];
			gfx->SetTextColor(0xff000000, 0xffffffff);
			gfx->SetTextStyle(0);
		}
	}
	// 983 a826d0/a82700: item fora da faixa de nivel do jogador desenha escurecido
	bool locked = item &&
		(item->IsUnderLvl ? item->Level < m_nRookieLv
						  : item->Level >= m_nRookieLv1);
	if (bmp)
	{
		WRect rc(li->pos.x, li->pos.y, (float)bmp->Width(),
			(float)bmp->Height());
		gfx->DrawTexture(bmp, rc, locked ? 0x50b0b0b0 : 0xffffffff, 0);
	}
	if (item == NULL)
		return;

	if ((item->TypeId & 0xfc000000) == 0x38000000 &&
		(item->TypeId & 0x3c00000) == 0x1800000)
	{
		const Bitmap* pBg = g_pFresh->GetBitmap("Title_BG");
		WRect rc(li->pos.x, li->pos.y, (float)pBg->Width(), // 983 a936a0: Title_BG em (0,0)
			(float)pBg->Height());
		gfx->DrawTexture(pBg, rc, 0xffffffff, 0);
	}

	const Bitmap* icon = GetItemIcon(item->TypeId);
	if (UccManager()->IsDisplayUccClothes(item))
	{
		sDisplayUccClothes* pUcc = (sDisplayUccClothes*)li->pData;
		icon = GetUccItemIcon(pUcc->itemId);
	}
	// 983 a93799: parte especial (00a82ce0) monta um sItemInfo UCC (guid de +0x238, UccIndex de +0x23c)
	// e tira dele o nome (007e96a0) e o icone (007e9620)
	bool bSpecial = false;
	char specialName[64] = "";
	if (IsSpecialPartTid983(item->TypeId) == true)
	{
		sItemInfo info;
		memset(&info, 0, sizeof(info));
		info.guid = *(const unsigned long*)SPARTTAIL983(item);
		info.tid = item->TypeId;
		info.status = 1;
		// D808: +0x23c passa do fim do no do std::map (0x24c, ver checkoutdlg D/00aa2dd6) = lixo do heap; o strcpy
		// do 983 pode estourar os 9 bytes (por quadro, no OwnerDraw). Copia limitada: mesmo resultado quando ha NUL.
		strncpy(info.UccIndex, SPARTTAIL983(item) + 4, sizeof(info.UccIndex) - 1);
		strcpy(specialName, GetUccItemName(&info));
		icon = GetUccItemIcon(&info);
		bSpecial = true;
	}
	if (icon)
	{
		float x;
		float y;
		switch (m_subTab)
		{
		case 0:
		case 1:
			if ((item->TypeId & 0xfc000000) == 0x24000000)
			{
				x = 7.0f;
				y = 2.0f;
			}
			else
			{
				x = 16.0f;
				y = 27.0f;
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 0xd:
		case 0x1b:
			x = 7.0f;
			y = 2.0f;
			break;
		case 0xb:
		case 0xc:
			if ((item->TypeId & 0xfc000000) == 0x20000000)
			{
				x = 7.0f;
				y = 2.0f;
			}
			else
			{
				x = 16.0f;
				y = 27.0f;
			}
			break;
		case 0x10:
			x = 7.0f;
			y = 12.0f;
			break;
		case 0x1c:
		case 0x22:
			x = 16.0f;
			y = 12.0f;
			break;
		case 0x1d:
		case 0x1e:
		case 0x1f:
		case 0x20:
		case 0x21:
			x = 21.0f;
			y = 12.0f;
			break;
		case 0x23:
		{
			// 983 a936a0: conjuntos Memorial centram o icone pelo Title_BG
			const Bitmap* pBg = g_pFresh->GetBitmap("Title_BG");
			x = pBg->Width() * 0.5f + 7.0f - icon->Width() * 0.5f;
			y = 27.0f;
			break;
		}
		default:
			// 983: default desloca (7,2) se bola fora do padrao 0x3e00000/0x1f8000
			if ((item->TypeId & 0xfc000000) == 0x24000000 &&
				!((item->TypeId & 0x3e00000) == 0x800000 &&
					(item->TypeId & 0x1fe000) == 0x1f8000))
			{
				x = 7.0f;
				y = 2.0f;
			}
			else
			{
				x = 16.0f;
				y = 27.0f;
			}
			break;
		}
		WRect rc(x + li->pos.x, y + li->pos.y, (float)icon->Width(),
			(float)icon->Height());
		gfx->DrawTexture(icon, rc, 0xffffffff, 0);
	}

	if (UccManager()->IsDisplayUccClothes(item) && m_pItemBase[4])
	{
		gfx->DrawTexture(m_pItemBase[4],
			WRect(li->pos.x, li->pos.y, (float)m_pItemBase[4]->Width(),
				(float)m_pItemBase[4]->Height()),
			0xffffffff, 0);
	}

	gfx->SetTextStyle(0);
	switch (m_subTab)
	{
	case 8:
	{
		IFF_STRUCT::sBall* pBall = (IFF_STRUCT::sBall*)item;
		if ((item->TypeId & 0xfc000000) == 0x24000000)
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2, "%s",
				pBall->c.Name);
		else
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				K2L_Compatibility("%s(%d\xb0\xb3)"), pBall->c.Name, pBall->COM[0]);
		break;
	}
	case 9:
	case 0xa:
	{
		// 983 a93e..a94210: so mostra a quantidade se nao for tid 0x18100000/0x1fc000 e COM[0] > 0 e COM[1] <= 0
		IFF_STRUCT::sItem* pIt = (IFF_STRUCT::sItem*)item;
		short num = pIt->COM[0];
		if (((item->TypeId & 0xfc000000) == 0x18000000 &&
				(item->TypeId & 0x1f00000) == 0x100000) ||
			(item->TypeId & 0x1fe000) == 0x1fc000 || pIt->COM[1] > 0)
			num = 0;
		if (num > 0)
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				K2L_Compatibility("%s(%d\xb0\xb3)"), item->Name, num);
		else
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				item->Name);
		break;
	}
	case 0x1c:
		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				item->Name);
		break;
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x21:
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		{
			if (((IFF_STRUCT::sFurniture*)item)->COM[0] > 0)
				gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
					K2L_Compatibility("%s(%d\xb0\xb3)"), item->Name,
					((IFF_STRUCT::sFurniture*)item)->COM[0]);
			else
				gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
					item->Name);
		}
		break;
	case 0x20:
		if ((item->TypeId & 0xfc000000) == 0x48000000)
		{
			if (((IFF_STRUCT::sFurniture*)item)->COM[0] > 0)
				gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
					K2L_Compatibility("%s(%d\xb0\xb3)"), item->Name,
					((IFF_STRUCT::sFurniture*)item)->COM[0]);
			else
				gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
					item->Name);
		}
		break;
	default:
		if (UccManager()->IsDisplayUccClothes(item))
		{
			sDisplayUccClothes* pUcc = (sDisplayUccClothes*)li->pData;
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				GetUccItemName(pUcc->itemId));
			g_pFresh->GetManager()->PrintText(
				WPoint(li->pos.x + 182.0f, li->pos.y + 23.0f), 2,
				pUcc->name.c_str(), -1.0f, 0xffffffff);
		}
		else if (bSpecial == true)
		{
			gfx->Print(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				specialName);
		}
		else
		{
			gfx->Print11(WPoint(li->pos.x + 182.0f, li->pos.y + 7.0f), 2,
				item->Name);
		}
		break;
	}

	if (IsSelectedRentalTab())
		DrawRentalPrice(182.0f, 23.0f, *item, li);
	else
		DrawItemPrice(gfx, 182.0f, 23.0f, *item, li);

	if (m_subTab >= 2 && (m_subTab <= 5 || m_subTab == 0x1b))
	{
		DrawStatus2(gfx, li, ((IFF_STRUCT::sPart*)li->pData)->Attr,
			((IFF_STRUCT::sPart*)li->pData)->Slot, 0.0f);
		DrawPartsSlot(gfx, li);
	}

	// 983 a942c7: subabas taco (7), caddie (0xb) e mascote (0xd) mostram o botao Stats
	short stats[10] = { 0 };
	bool bStats = false;
	if (m_subTab == SUBTAB_CLUBSET)
	{
		// 983 a9439b: item de conjunto mostra os atributos do taco, 1o elemento do conjunto (+0xc4)
		unsigned long clubTid = item->TypeId;
		if ((clubTid & 0x3e00000) == 0x800000 && (clubTid & 0x1fe000) == 0x1f8000)
			clubTid = ((IFF_STRUCT::sSetItem*)item)->ElemIds[0];
		IFF_STRUCT::sClubSet* pSet = clubTid ? ItemManager()->FindClubSet(clubTid) : NULL;
		if (pSet)
		{
			memcpy(stats, pSet->Attr, sizeof(pSet->Attr));
			memcpy(stats + 5, pSet->Slot, sizeof(pSet->Slot));
			bStats = true;
		}
	}
	else if (m_subTab == SUBTAB_CADDIE)
	{
		IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(item->TypeId);
		if (pCaddie)
		{
			memcpy(stats, pCaddie->Attr, sizeof(pCaddie->Attr));
			bStats = true;
		}
	}
	else if (m_subTab == SUBTAB_MASCOT)
	{
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(item->TypeId);
		if (pMascot)
		{
			for (int i = 0; i < 5; ++i)
				stats[i] = pMascot->Attr[i];
			bStats = true;
		}
	}
	if (bStats)
		DrawStatView983(gfx, li, stats);

	if (li->selected &&
		item->Level > CSharedDoc::Instance()->m_myInfo.stat.Level)
	{
		const Bitmap* pLevel =
			g_pFresh->GetBitmap(MakeStr("level_%03d", item->Level + 1));
		if (pLevel)
		{
			WRect rc(li->pos.x + 10.0f, li->pos.y + 4.0f,
				(float)pLevel->Width(), (float)pLevel->Height());
			gfx->DrawTexture(pLevel, rc, 0xffffffff, 0);
		}
	}

	// 983 a94639..a948a5: marca de periodo (Surprise Sale) so se estiver no periodo e nada mais;
	// senao Limited (estado de venda), Lim (periodo no item), depois New e Hot
	const Bitmap* pMark = NULL;
	bool bFlags = true;
	if (item->DateFlag == 2)
	{
		bFlags = false;
		if (ItemManager()->IsInSalePeriod(item->TypeId,
				CSharedDoc::Instance()->GetServerTime()))
			pMark = m_pBmpMarkSurprise;
	}
	else if (ShopSaleState983(item->TypeId) != 0 && m_pBmpMarkLimited)
	{
		// 983 a94788: Limited vai sozinho, sem New/Hot
		bFlags = false;
		pMark = m_pBmpMarkLimited;
	}
	else if (m_pBmpMarkLim &&
		(item->saleStart.wYear != 0 || item->saleEnd.wYear != 0))
		pMark = m_pBmpMarkLim;
	if (pMark)
	{
		gfx->DrawTexture(pMark,
			WRect(li->pos.x, li->pos.y, (float)pMark->Width(),
				(float)pMark->Height()),
			0xffffffff, 0);
	}
	if (bFlags && item->New && m_pBmpMarkNew)
	{
		gfx->DrawTexture(m_pBmpMarkNew,
			WRect(li->pos.x, li->pos.y, (float)m_pBmpMarkNew->Width(),
				(float)m_pBmpMarkNew->Height()),
			0xffffffff, 0);
	}
	if (bFlags && item->Hit && m_pBmpMarkHot)
	{
		gfx->DrawTexture(m_pBmpMarkHot,
			WRect(li->pos.x, li->pos.y, (float)m_pBmpMarkHot->Width(),
				(float)m_pBmpMarkHot->Height()),
			0xffffffff, 0);
	}

	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	// 983 a948bd: comprar = estoque 1/3/7 ou parte especial; presente = so IsCash e estoque 1/2/7
	bool bBuy = item->InStock == 1 || item->InStock == 3 || item->InStock == 7 ||
		IsSpecialPartTid983(item->TypeId);
	Title* pTitle;
	const Bitmap* pNoBuy;
	if ((item->TypeId & 0xfc000000) == 0x38000000 &&
		(item->TypeId & 0x3c00000) == 0x1800000)
	{
		pTitle = g_TitleManager->GetTitle((eTitle)(item->TypeId & 0x3fffff));
		if (pTitle != NULL && !pTitle->CanEquip() &&
			(pNoBuy = g_pFresh->GetBitmap("do not purchase")) != NULL)
		{
			gfx->DrawTexture(pNoBuy,
				WRect(WPoint(li->pos.x + 1.0f, li->pos.y + 1.0f),
					pNoBuy->Width(), pNoBuy->Height()),
				0xffffffff, 0);
			bBuy = false;
		}
	}
	else if (locked && (pNoBuy = g_pFresh->GetBitmap("do not purchase")) != NULL)
	{
		gfx->DrawTexture(pNoBuy,
			WRect(WPoint(li->pos.x + 1.0f, li->pos.y + 1.0f), pNoBuy->Width(),
				pNoBuy->Height()),
			0xffffffff, 0);
		bBuy = false;
	}
	// 983 63f280: estado 3 (mapa CSharedDoc+0x113e0, pacote 0x20e) esconde comprar e presente
	unsigned long saleState = 0;
	std::map<unsigned long, sShopSaleState983>::iterator itState =
		Doc()->m_shopSaleState983.find(item->TypeId);
	if (itState != Doc()->m_shopSaleState983.end())
		saleState = itState->second.state;
	if (saleState != 3 && bBuy)
	{
		if (!m_rowBtn983[ROWBTN983_BUY].bOn)
		{
			gfx->DrawTexture(m_rowBtn983[ROWBTN983_BUY].pBmpN,
				m_rowBtn983[ROWBTN983_BUY].Rc() + li->pos + WPoint(0.0f, 0.0f), 0x88ffffff, 0);
		}
		else
		{
			if (li->underCursor)
			{
				if ((m_rowBtn983[ROWBTN983_BUY].Rc() + li->pos).IsInRect(mouse))
				{
					gfx->DrawTexture(m_rowBtn983[ROWBTN983_BUY].pBmpO,
						(m_rowBtn983[ROWBTN983_BUY].Rc() + li->pos) + WPoint(0.0f, 0.0f),
						0xffffffff, 0);
					if (CIconToolTip::GetInstance()->GetCurTipIndex() == -1) // 983 a94b84
						CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_BUY);
					goto GiftButton;
				}
			}
			gfx->DrawTexture(m_rowBtn983[ROWBTN983_BUY].pBmpN,
				m_rowBtn983[ROWBTN983_BUY].Rc() + li->pos + WPoint(0.0f, 0.0f), 0xffffffff, 0);
		}
	}

GiftButton:
	bool bGift = item->IsCash &&
		(item->InStock == 1 || item->InStock == 2 || item->InStock == 7) &&
		!IsSpecialPartTid983(item->TypeId) && saleState != 3;
	if ((!m_pSideTabRental ||
			m_pSideTabRental->GetStatus() != FrButton::PRESSED ||
			m_mainTab != 1) &&
		bGift)
	{
		if (!m_rowBtn983[ROWBTN983_GIFT].bOn)
		{
			gfx->DrawTexture(m_rowBtn983[ROWBTN983_GIFT].pBmpN, m_rowBtn983[ROWBTN983_GIFT].Rc() + li->pos, 0x88ffffff, 0);
		}
		else
		{
			if (li->underCursor)
			{
				if ((m_rowBtn983[ROWBTN983_GIFT].Rc() + li->pos).IsInRect(mouse))
				{
					gfx->DrawTexture(m_rowBtn983[ROWBTN983_GIFT].pBmpO, m_rowBtn983[ROWBTN983_GIFT].Rc() + li->pos,
						0xffffffff, 0);
					if (CIconToolTip::GetInstance()->GetCurTipIndex() == -1) // 983 a94d1e
						CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_GIFT);
					goto LimitMark;
				}
			}
			gfx->DrawTexture(m_rowBtn983[ROWBTN983_GIFT].pBmpN, m_rowBtn983[ROWBTN983_GIFT].Rc() + li->pos, 0xffffffff, 0);
		}
	}

LimitMark:
	// 983 00a94d38: tipo da venda limitada (estado 1/2/outro -> mark_limite_type_01..03) no pe do item
	if (saleState != 0)
	{
		Bitmap* pType = m_pBmp1fc[saleState == 1 ? 0 : (saleState == 2 ? 1 : 2)];
		float y = (float)m_pItemBase[0]->Height() + li->pos.y + 0.0f -
			(float)m_pBmpMarkLimited->Height() * 0.5 - 5.0;
		float x = (float)m_pBmpMarkLimited->Width() * 0.5 + li->pos.x + 0.0f + 15.0;
		gfx->DrawTexture(pType,
			WRect(x, y, (float)pType->Width(), (float)pType->Height()), 0xffffffff, 0);
	}
	// 983 00a94e59: marca do item escolhido na busca (0x2d7)
	if (m_pBmpSelectMark && m_n2e4 && m_n2e4 == item->TypeId)
	{
		gfx->DrawTexture(m_pBmpSelectMark,
			WRect(li->pos.x, li->pos.y, (float)m_pItemList->GetItemWidth(),
				(float)m_pItemList->GetItemHeight()),
			0xffffffff, 0);
	}
}

unsigned long CShopMain::ShopSaleState983(unsigned long tid)
{
	// 983 63f280: estado de venda (mapa CSharedDoc+0x113e0, pacote 0x20e)
	std::map<unsigned long, sShopSaleState983>::iterator it =
		Doc()->m_shopSaleState983.find(tid);
	if (it == Doc()->m_shopSaleState983.end())
		return 0;
	return it->second.state;
}

bool CShopMain::ShopItemLocked983(const IFF_ITEM_COMMON* item)
{
	// 983 a826d0/a82700: fora da faixa de nivel do jogador
	return item->IsUnderLvl ? item->Level < m_nRookieLv
							: item->Level >= m_nRookieLv1;
}

void CShopMain::OnItemListBtnDown()
{
	FrListItem* pItem = m_pItemList->GetItemUnderCursor();
	if (pItem != NULL)
	{
		bool bEnable = false;
		if (m_hand.IsWorking(0))
		{
			m_pItemList->ToggleItem(pItem);
			return;
		}
		IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)pItem->pData;
		CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
		if (UccManager()->IsDisplayUccClothes(pData))
		{
			sDisplayUccClothes* pUcc = (sDisplayUccClothes*)pItem->pData;
			IFF_STRUCT::sPart* pPart =
				ItemManager()->FindPart(pUcc->partTypeId);
			sUccClothes* pClothes = UccManager()->FindClothes(pUcc->itemId);
			if (pPart != NULL && pClothes != NULL)
			{
				pUcc->part.PosMask = pPart->PosMask;
				pUcc->part.HideMask = pPart->HideMask;
				strcpy(pUcc->part.Data, pPart->Data);
				strcpy(pUcc->part.Tex[0],
					GenerateTextureName_F(pClothes->typeId, pClothes->uccIndex)
						.c_str());
				strcpy(pUcc->part.Tex[1],
					GenerateTextureName_B(pClothes->typeId, pClothes->uccIndex)
						.c_str());
				strcpy(pUcc->part.OrgTex[0], pPart->Tex[0]);
				strcpy(pUcc->part.OrgTex[1], pPart->Tex[1]);
				pUcc->part.SubPart[0] = pPart->SubPart[0];
				pUcc->part.SubPart[1] = pPart->SubPart[1];
			}
		}

		// 983 00a90e0c: titulo que nao pode equipar nao reage ao clique
		if ((pData->TypeId & 0xfc000000) == 0x38000000 &&
			(pData->TypeId & 0x3c00000) == 0x1800000)
		{
			Title* pTitle = g_TitleManager->GetTitle((eTitle)(pData->TypeId & 0x3fffff));
			if (pTitle != NULL && !pTitle->CanEquip())
				return;
		}

		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		WRect rc(pItem->pos.x + m_rowBtn983[ROWBTN983_BUY].Rc().x, pItem->pos.y + m_rowBtn983[ROWBTN983_BUY].Rc().y,
			m_rowBtn983[ROWBTN983_BUY].Rc().w, m_rowBtn983[ROWBTN983_BUY].Rc().h);
		if (rc.IsInRect(mouse))
		{
			// 983 a90bf0: comprar = estoque 1/3/7 ou parte especial; fora da venda (estado 3 do 0x20e) ou
			// fora da faixa de nivel nao compra (cai na selecao)
			// (a parte especial compra mesmo sem m_rowBtn983[ROWBTN983_BUY].bOn)
			bool bGift = pData->InStock == 1 || pData->InStock == 3 ||
				pData->InStock == 7;
			if (!m_rowBtn983[ROWBTN983_BUY].bOn)
				bGift = false;
			if (IsSpecialPartTid983(pData->TypeId) == true)
				bGift = true;
			// 983 00a90eb4: item 0x1a0000fd ja no inventario (m_myItemList, Common[0] != 0) -> "notify"
			// "´õÀÌ»ó °¡Áú ¼ö ¾ø½À´Ï´Ù." e nao compra
			if (pData->TypeId == 0x1a0000fd)
			{
				std::list<sItemInfo>& items = CSharedDoc::Instance()->m_myItemList;
				std::list<sItemInfo>::iterator end = items.end();
				for (std::list<sItemInfo>::iterator it = items.begin(); it != end; ++it)
				{
					if ((*it).tid == 0x1a0000fd && (*it).Common[0] != 0)
					{
						FrForm* pForm = CreateForm<FrForm>(
							g_pFresh->GetManager(), this, "notify", NULL);
						if (pForm != NULL)
						{
							pForm->SetMessage(K2L_Compatibility("\xb4\xf5\xc0\xcc\xbb\xf3 "
								"\xb0\xa1\xc1\xfa \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), false);
							pForm->Open(NULL, 3);
						}
						bGift = false;
					}
				}
			}
			if (bGift && (ShopSaleState983(pData->TypeId) == 3 ||
							 ShopItemLocked983(pData)))
				bGift = false;
			if (bGift)
			{
				ResetItemList(false);
				ClearCartList(false, pTids);
				m_oldItems.clear();
				m_pItemList->SelectItem(pItem, true);
				unsigned long type = pData->TypeId >> 26;
				if (type == 0xf && !HaveCharacter(m_u524))
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 \xb0\xa1\xc1\xf6\xb0\xed "
							 "\xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0));
					return;
				}
				if (type == 1 && HaveCharacter(m_u524))
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 \xc0\xcc\xb9\xcc "
							 "\xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0));
					// 983 00a910c7: o personagem ja comprado ainda e selecionado (vestido na previa)
					SelectItemList(pItem);
					return;
				}
				if (!SelectItemList(pItem))
					return;
				if ((pData->TypeId & 0xfc000000) == 0x20000000 &&
					(pData->TypeId & 0x1fe000) == 0x6000)
				{
					UpgradeCaddie(m_u528);
					return;
				}
				Checkout();
				return;
			}
		}
		else
		{
			WRect rc2(pItem->pos.x + m_rowBtn983[ROWBTN983_GIFT].Rc().x,
				pItem->pos.y + m_rowBtn983[ROWBTN983_GIFT].Rc().y, m_rowBtn983[ROWBTN983_GIFT].Rc().w, m_rowBtn983[ROWBTN983_GIFT].Rc().h);
			if (rc2.IsInRect(mouse))
			{
				// 983 a90bf0: presente = IsCash e estoque 1/2/7, nao parte especial
				bool bGift = pData->IsCash &&
					(pData->InStock == 1 || pData->InStock == 2 ||
						pData->InStock == 7) &&
					!IsSpecialPartTid983(pData->TypeId);
				// 983 00a911e0: presente so testa o estado de venda (sem faixa de nivel)
				if (bGift && ShopSaleState983(pData->TypeId) == 3)
					bGift = false;
				if (m_rowBtn983[ROWBTN983_GIFT].bOn && bGift)
				{
					ResetItemList(false);
					m_pItemList->SelectItem(pItem, true);
					if (!SelectItemList(pItem))
						return;
					SendGift(*pData);
					return;
				}
			}
		}

		if (!pItem->selected)
		{
			unsigned long type = pData->TypeId >> 26;
			if (type == 2 || (type > 0xd && type < 0x10))
				UnselectItemList(pData->TypeId);
			else
				m_pItemList->ToggleItem(pItem);
		}
		else
		{
			unsigned long tid = pData->TypeId;
			if ((tid >> 26) == 0x1f)
				m_u72c = tid;
			else if ((tid >> 26) == 9 && (tid & 0x1fe000) == 0x1fa000)
				m_u72c = tid;
			else
				m_u72c = 0;
			SelectItemList(pItem);
		}

		if (m_mainTab == 1 || (m_mainTab == 5 && m_subTab != 0x23))
		{
			m_pBuyAll->Enable(!m_pPartCartList->m_itemList.empty() ||
				!m_pAuxPartCartList->m_itemList.empty());
			if (!m_pPartCartList->m_itemList.empty() ||
				!m_pAuxPartCartList->m_itemList.empty())
				bEnable = true;
			m_pBuyReset->Enable(bEnable);
		}
	}
}

void CShopMain::OnItemListRBtnDown()
{
	FrListItem* pItem;
	if (m_pItemList && (pItem = m_pItemList->GetItemUnderCursor()) != NULL)
	{
		if (m_hand.IsWorking(0))
		{
			m_pItemList->ToggleItem(pItem);
			return;
		}
		if (pItem->selected)
			SelectItemList(pItem);
		m_pItemList->SelectItem(pItem, true);
		IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)pItem->pData;
		if (pData && !UccManager()->IsDisplayUccClothes(pData))
		{
			// 983 a913d9: parte especial passa o guid (+0x238) e o UccIndex (+0x23c) do UCC
			unsigned long uccGuid = 0;
			char uccIndex[9] = "";
			if (IsSpecialPartTid983(pData->TypeId) == true)
			{
				uccGuid = *(const unsigned long*)SPARTTAIL983(pData);
				// D808: +0x23c = lixo do heap apos o no do mapa (983 strcpy em 9 bytes da pilha, cookie em ebp-4)
				strncpy(uccIndex, SPARTTAIL983(pData) + 4, sizeof(uccIndex) - 1);
			}
			OpenInformation(pData->TypeId, uccGuid, uccIndex);
			// 983 00a91340: no direito so a aba principal (1 ou 5), sem o teste
			// de sub-aba do esquerdo (00a90bf0)
			if (m_mainTab == 1 || m_mainTab == 5)
			{
				m_pBuyAll->Enable(!m_pPartCartList->m_itemList.empty() ||
					!m_pAuxPartCartList->m_itemList.empty());
				m_pBuyReset->Enable(!m_pPartCartList->m_itemList.empty() ||
					!m_pAuxPartCartList->m_itemList.empty());
			}
		}
	}
}

void CShopMain::OnBuyAllInit(int param)
{
	m_pBuyAll = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pBuyAll)
		m_pBuyAll->EnableHover(true, 0.0f);
}

void CShopMain::OnBuyAllUp()
{
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem = (IFF_ITEM_COMMON*)(*it)->pData;
		unsigned long type = pItem->TypeId >> 26;
		if (type == 1)
		{
			if (HaveCharacter(pItem->TypeId))
				it = m_pPartCartList->_DelItem(it);
			break;
		}
		if (type == 9)
			break;
	}

	// 983 00a914c0: sem o personagem e sem personagem no carrinho -> aviso
	if (!HaveCharacter(m_u524) && it == m_pPartCartList->m_itemList.end())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 "
				"\xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9"), 0, 0, 0, 0));
		return;
	}
	Checkout();
	OnBuyHoverOff(0);
}

void CShopMain::OnBuyResetInit(int param)
{
	m_pBuyReset = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pBuyReset)
		m_pBuyReset->EnableHover(true, 0.0f);
}

void CShopMain::OnBuyResetUp()
{
	g_audio->PlaySfx("cloth_off");
	if (m_mainTab == 1)
	{
		std::map<unsigned int, sCharacterInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_charMap.begin();
			it != CSharedDoc::Instance()->m_charMap.end() &&
			it->second.tid != m_u524;
			++it)
		{
		}
		if (m_subTab == 1 || m_subTab == 2 || m_subTab == 3 || m_subTab == 4 ||
			m_subTab == 5 || m_subTab == 6 || m_subTab == 0x1b)
		{
			if (it != CSharedDoc::Instance()->m_charMap.end())
			{
				CSharedDoc::Instance()->BuildMyPartTidList(
					(sCharacterInfo&)it->second);
				m_pParts = (unsigned long*)it->second.tidParts;
				m_pAuxParts = (unsigned long*)it->second.tidAuxParts;
				m_pCharPCL = (char*)it->second.PCL;
				m_b520 = it->second.hairClr;
			}
			else
			{
				CSharedDoc::Instance()->BuildMyPartTidListDefault(m_u524);
			}
			ClearCartList(true, &CSharedDoc::Instance()->GetMyPartTidList());
			BuildDisplayStand(m_subTab);
			// 983 00a930a5: o boneco da loja e o do "LobbyAvatar": 0x188 (limpa) e 0x187 (&tid, 1), sem o m_hand do 645
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << MsgObject(NULL, 0x188, 0, 0, 0, 0, 0);
			pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << MsgObject(NULL, 0x187, (int)&m_u524, 1, 0, 0, 0);
			TurnOnItemStatus();
		}
	}
	else if (m_mainTab == 5)
	{
		ResetItemList(false);
		ClearCartList(false, NULL);
		unsigned long* pTid = m_skinTid;
		memcpy(pTid, CSharedDoc::Instance()->m_myInfo.userEquip.tidSkin,
			sizeof(m_skinTid));
		for (int i = 6; i != 0; --i, ++pTid)
		{
			if (*pTid)
			{
				IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(*pTid);
				if (pSkin && (*pTid & 0x3c00000) == 0)
				{
					m_u724 = pSkin->HScroll;
					m_u728 = pSkin->VScroll;
					m_f71c = 0;
					m_f720 = 0;
				}
			}
		}
	}
	m_pBuyAll->Enable(false);
	m_pBuyReset->Enable(false);
	OnBuyHoverOff(0);
}

// 983 00a89640: dica "\xb8\xf0\xb5\xce\xb1\xb8\xb8\xc5" / "\xbf\xf8\xb7\xa1\xb4\xeb\xb7\xce" pelo RFTooltipHelper (nao mais o CIconToolTip)
void CShopMain::OnBuyHoverOn(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pBtn == m_pBuyAll)
	{
		if (RFTooltipHelper::Instance())
			RFTooltipHelper::Instance()->Show983(
				0, std::string(K2L_Compatibility("\xb8\xf0\xb5\xce\xb1\xb8\xb8\xc5")), 1, 0xb);
	}
	else if (pBtn == m_pBuyReset)
	{
		if (RFTooltipHelper::Instance())
			RFTooltipHelper::Instance()->Show983(
				0, std::string(K2L_Compatibility("\xbf\xf8\xb7\xa1\xb4\xeb\xb7\xce")), 1, 0xb);
	}
}

// 983 00a863d0: sempre esconde a dica
void CShopMain::OnBuyHoverOff(int param)
{
	if (RFTooltipHelper::Instance())
		RFTooltipHelper::Instance()->Hide983();
}

void CShopMain::OnSideTabBuyInit(int param)
{
	m_pSideTabBuy = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pSideTabBuy)
	{
		m_pSideTabBuy->SetPushDelay(0.2f);
		m_pSideTabBuy->SetStatus(FrButton::PRESSED);
	}
}

void CShopMain::OnSideTabBuyUp()
{
	if (m_pSideTabBuy && m_pSideTabRental &&
		m_pSideTabBuy->GetStatus() != FrButton::PRESSED)
	{
		m_pSideTabBuy->SetStatus(FrButton::PRESSED);
		m_pSideTabRental->SetStatus(FrButton::NORMAL);
		m_rowBtn983[ROWBTN983_GIFT].bOn = true;
		ClearCartList(true, &CSharedDoc::Instance()->GetMyPartTidList());
		SetLevelBar();
		BuildDisplayStand(m_subTab);
		m_hand.Work(this, 0, &m_u524, true, -1);
	}
}

void CShopMain::OnSideTabRentalInit(int param)
{
	m_pSideTabRental = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pSideTabRental)
	{
		m_pSideTabRental->SetPushDelay(0.2f);
		m_pSideTabRental->SetStatus(FrButton::NORMAL);
	}
}

void CShopMain::OnSideTabRentalUp()
{
	if (m_pSideTabBuy && m_pSideTabRental &&
		m_pSideTabRental->GetStatus() != FrButton::PRESSED)
	{
		m_pSideTabRental->SetStatus(FrButton::PRESSED);
		m_pSideTabBuy->SetStatus(FrButton::NORMAL);
		m_rowBtn983[ROWBTN983_GIFT].bOn = false;
		ClearCartList(false, &CSharedDoc::Instance()->GetMyPartTidList());
		SetLevelBar();
		BuildDisplayRental(m_subTab);
		m_hand.Work(this, 0, &m_u524, true, -1);
	}
}

namespace S5
{
	bool ISCutinItem(unsigned long typeId);
	void SysTimeToSec983(const SYSTEMTIME* pTime, __int64* pOut); // 00b30830 (D648)
	bool UnderCategory(unsigned long charTid, unsigned long typeId,
		unsigned long flag);
}

bool IsAngelWing(unsigned long typeId);
bool IsMannerPlayer();

void CShopMain::SetVisibleSideTab(bool visible)
{
	if (m_pSideTabBuy)
		m_pSideTabBuy->SetVisible(visible);
	if (m_pSideTabRental)
		m_pSideTabRental->SetVisible(visible);
}

void CShopMain::BuildDisplayRental(subTab_t tab)
{
	m_pItemList->ClearItem();
	// 983 00a8c310: sub-abas de roupa 2..5 (numeracao do 983; o 645 usava 1..4)
	if (tab >= 2 && tab <= 5)
	{
		unsigned long flag = GetCategoryFlag(tab);
		PartIterator it;
		for (it = ItemManager()->m_PartMap.begin();
			it != ItemManager()->m_PartMap.end(); it++)
		{
			IFF_STRUCT::sPart& part = (*it).second;
			IFF_ITEM_COMMON& item = part.c;
			if (part.RentalPrice > 0 &&
				S5::UnderCategory(m_u524, item.TypeId, flag) &&
				item.InStock != 0)
			{
				CItemManager* pItemMgr = ItemManager();
				if (pItemMgr->IsInSalePeriod(item.TypeId,
						CSharedDoc::Instance()->GetServerTime()) &&
					(!IsAngelWing(item.TypeId) ||
						(IsMannerPlayer() &&
							CSharedDoc::Instance()->m_myInfo.stat.Level >= 21)))
				{
					FrListItem* pItem = m_pItemList->AddItem(&item);
					if (pItem && IsItemInCart(pItem))
						m_pItemList->SelectItem(pItem, true);
					if (item.InStock == 1 || item.InStock == 3)
						pItem->AddNoRBtnIdx(1);
				}
			}
		}
		m_pItemList->SortItem(PartCompare);
	}
	int n = m_pItemList->GetCurrentItemSize(true);
	if (n < 8)
	{
		int i = 8 - n;
		do
		{
			m_pItemList->AddItem(NULL);
			--i;
		} while (i != 0);
	}
}

void CShopMain::DrawRentalPrice(float x, float y, IFF_ITEM_COMMON& item,
	FrListItem* pItem)
{
	if (pItem && item.InStock != 4 && (item.TypeId & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(item.TypeId);
		if (pPart)
		{
			FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
			const char* str = MakeStr(K2L_Compatibility("%d\xc6\xce(%d\xc0\xcf)"),
				pPart->RentalPrice, 7);
			WPoint pt(pItem->pos.x + x, pItem->pos.y + y);
			gfx->Print11(pt, 2, str);
		}
	}
}

bool CShopMain::IsSelectedRentalTab()
{
	if (m_mainTab == 1 && m_pSideTabRental != NULL &&
		m_pSideTabRental->GetStatus() == FrButton::PRESSED)
		return true;
	return false;
}

void CShopMain::OnPartCartListInit(int param)
{
	m_pPartCartList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

// 983: WRect zerado do .bss (00f14668), o mesmo "retangulo vazio" do recycledlg.cpp
// 00a82590: comparacao fora de linha (thiscall, ecx = este rect vazio), chamada por a8aa50/a8b040
struct EmptyRect983 : WRect
{
	__declspec(noinline) bool operator==(const WRect& r) const
	{
		return r.x == x && r.y == y && r.w == w && r.h == h;
	}
};
static EmptyRect983 s_rcEmpty983;

// a8aa50/a8b040: com [this+0x6b0] vazio (a82590) pede o rect do boneco na tela ao "LobbyAvatar" (msg 0x194 ->
// LobbyPlayer::GetScreenRect983) e chama 00a84da0: part_cart em rect.x-90, auxpart_cart em rect.x+rect.w+10 (sem passar
// de main_bg3.x-120). Original: part_cart 260 / aux 647 nas abas, part_cart 648 na Decoration (camera da vitrine)
void CShopMain::PlacePartCartLists983()
{
	if (m_pPartCartList)
	{
		// so o x muda (983 a84da0): o y fica o do layout (279 no original em todas as abas, Decoration inclusive)
		WRect r = m_pPartCartList->GetRect();
		r.x = m_rc6b0.x - 90.0f;
		m_pPartCartList->SetRect(r);
	}
	if (m_pAuxPartCartList)
	{
		WRect r = m_pAuxPartCartList->GetRect();
		r.x = m_rc6b0.x + m_rc6b0.w + 10.0f;
		if (m_pMainBg[2] && m_pMainBg[2]->GetRect().x < r.x + r.w + 10.0f)
			r.x = m_pMainBg[2]->GetRect().x - 120.0f;
		m_pAuxPartCartList->SetRect(r);
		CheckAuxCartScroll(); // 983 00a84e3a: as setas acompanham a lista que acabou de andar
	}
}

void CShopMain::OnPartCartListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	if (!gfx)
		return;
	IFF_STRUCT::sPart* pPart = (IFF_STRUCT::sPart*)pItem->pData;
	if (!pPart)
		return;
	unsigned long tid = pPart->c.TypeId;
	short attr[5] = {0};
	short slotAttr[5] = {0};
	bool found = false;
	int slot;
	// 983 a8aa50: so tipos 2, 9 e 0xf
	switch (tid >> 26)
	{
	case 2:
		for (int i = 0; i < 5; ++i)
		{
			if (pPart->Attr[i] != 0 || pPart->Slot[i] != 0)
			{
				memcpy(attr, pPart->Attr, sizeof(attr));
				memcpy(slotAttr, pPart->Slot, sizeof(slotAttr));
				found = true;
				break;
			}
		}
		switch (pPart->Category)
		{
		case 0:
			slot = 1;
			break;
		case 1:
			slot = 2;
			break;
		case 4:
			slot = 3;
			break;
		case 7:
		case 8:
		case 9:
		{
			unsigned int n = tid >> 18 & 0xff;
			if (n > 0xe)
				slot = 0;
			else
				slot = ((tid >> 13 & 0x1f) != g_slotCategory[n]) + 1;
			break;
		}
		default:
			return;
		}
		break;
	case 9:
	{
		unsigned int sub = tid >> 21 & 0x1f;
		if (sub == 6 || sub == 5 || sub == 4 || sub == 8)
			return;
		slot = 1;
		break;
	}
	case 0xf:
		slot = 0;
		break;
	default:
		return;
	}
	float w = (float)m_pBmpFa02->Width();
	float h = (float)m_pBmpFa02->Height();
	float iconW = (float)m_pSiconBmp[0]->Width();
	float iconH = (float)m_pSiconBmp[0]->Height();
	float btnW = (float)m_pBmpCancelN->Width();
	float btnH = (float)m_pBmpCancelN->Height();
	// 983 a8aa50: relativo a lista part_cart (+0x34/+0x38); 8,129 sem lista
	float x = 8.0f;
	float y = 129.0f;
	if (s_rcEmpty983 == m_rc6b0)
	{
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x194, (int)&m_rc6b0, 0, 0, 0, 0);
		PlacePartCartLists983();
	}
	if (m_pPartCartList)
	{
		x = m_pPartCartList->GetRect().x;
		y = m_pPartCartList->GetRect().y;
	}
	y += h * (float)slot;
	gfx->DrawTexture(m_pBmpFa02, WRect(x, y, w, h), 0xffffffff, 0);
	gfx->DrawTexture(m_pSiconBmp[slot], WRect(x + 4.0f, y + 5.0f, iconW, iconH),
		0xffffffff, 0);
	gfx->SetTextColor(0xffffffff, 0xff36779a);
	gfx->SetTextStyle(4);
	// 983 a8aa50: parte especial (00a82ce0) usa o nome do UCC (007e96a0), que volta "" (ver OpenInformation)
	const char* name = IsSpecialPartTid983(tid) ? "" : pPart->c.Name;
	float tx = x + 8.0f;
	gfx->Print11(WPoint(tx, y + 27.0f), 0, name);
	gfx->SetTextColor(0xffffffff, 0xffffffff);
	gfx->SetTextStyle(0);
	if (found)
		DrawStatus3(gfx, tx, y + 41.0f, attr, slotAttr);
	WRect btn(x + 104.0f, y + 12.0f, btnW, btnH);
	if (!btn.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		gfx->DrawTexture(m_pBmpCancelN, btn, 0xffffffff, 0);
	}
	else
	{
		gfx->DrawTexture(m_pBmpCancelO, btn, 0xffffffff, 0);
		if (g_input->GetButton(LEFT_BUTTON) == 2)
			m_tidList.push_back(pPart->c.TypeId);
	}
}

void CShopMain::DrawStatus3(FrGraphicInterface* gfx, float x, float y,
	short* const a, short* const b)
{
	float yy = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		if (a[i] != 0 || b[i] != 0)
		{
			WPoint pt(x, yy + y);
			gfx->Print11(pt, 0, MakeStatusString(i, a[i], b[i]));
			yy += 12.0f;
		}
	}
}

void CShopMain::OnAuxPartCartListInit(int param)
{
	m_pAuxPartCartList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CShopMain::OnAuxPartCartListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	FrGraphicInterface* gfx;
	IFF_STRUCT::sPart* pPart;
	if (pItem && (gfx = g_pFresh->GetManager()->GetGDI()) != NULL &&
		(pPart = (IFF_STRUCT::sPart*)pItem->pData) != NULL)
	{
		short attr[5] = { 0, 0, 0, 0, 0 };
		short slot[5] = { 0, 0, 0, 0, 0 };
		bool found = false;
		// 983 a8b040: tipo 0xe nao procura atributos
		if ((pPart->c.TypeId & 0xfc000000) != 0x38000000)
		{
			for (int i = 0; i < 5; ++i)
			{
				if (pPart->Attr[i] != 0 || pPart->Slot[i] != 0)
				{
					memcpy(attr, pPart->Attr, sizeof(attr));
					memcpy(slot, pPart->Slot, sizeof(slot));
					found = true;
					break;
				}
			}
		}
		float x = 250.0f;
		float y = 129.0f;
		float w = (float)m_pBmpFa02->Width();
		float h = (float)m_pBmpFa02->Height();
		float iconW = (float)m_pSiconBmp[0]->Width();
		float iconH = (float)m_pSiconBmp[0]->Height();
		float btnW = (float)m_pBmpCancelN->Width();
		float btnH = (float)m_pBmpCancelN->Height();
		// 983 a8b040: posiciona as listas (a84da0) e desenha relativo a lista auxpart_cart (+0x34/+0x38); 250,129 sem lista
		if (s_rcEmpty983 == m_rc6b0)
		{
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x194, (int)&m_rc6b0, 0, 0, 0, 0);
			PlacePartCartLists983();
		}
		if (m_pAuxPartCartList)
		{
			// 983 a8b040: na Decoration x = main_bg3 [this+0x698].x-120 (d0b478), y da lista (original: main_bg3 x=995 ->
			// 875, auxpart_cart y=279 = medido C326.1 em c326-1)
			if (m_mainTab == 5 && m_pMainBg[2])
				x = m_pMainBg[2]->GetRect().x - 120.0f;
			else
				x = m_pAuxPartCartList->GetRect().x;
			y = m_pAuxPartCartList->GetRect().y;
		}
		y += h * (float)pItem->no;
		gfx->DrawTexture(m_pBmpFa02, WRect(x, y, w, h), 0xffffffff, 0);
		gfx->DrawTexture(m_pSiconBmp[4], WRect(x + 4.0f, y + 5.0f, iconW, iconH),
			0xffffffff, 0);
		gfx->SetTextColor(0xffffffff, 0xff36779a);
		gfx->SetTextStyle(4);
		float tx = x + 8.0f;
		gfx->Print11(WPoint(tx, y + 27.0f), 0, pPart->c.Name);
		gfx->SetTextColor(0xffffffff, 0xffffffff);
		gfx->SetTextStyle(0);
		if (found)
			DrawStatus3(gfx, tx, y + 41.0f, attr, slot);
		WRect btn(x + 104.0f, y + 12.0f, btnW, btnH);
		if (!btn.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			gfx->DrawTexture(m_pBmpCancelN, btn, 0xffffffff, 0);
		}
		else
		{
			gfx->DrawTexture(m_pBmpCancelO, btn, 0xffffffff, 0);
			if (g_input->GetButton(LEFT_BUTTON) == 2)
				m_tidList.push_back(pPart->c.TypeId);
		}
	}
}

const char* dot_files[5][2] = {
	{ "ab_power",    "ab_power_slot"    },
    { "ab_control",  "ab_control_slot"  },
	{ "ab_accuracy", "ab_accuracy_slot" },
    { "ab_spin",     "ab_spin_slot"     },
	{ "ab_curve",    "ab_curve_slot"    }
};

#define REGBMP(name) ((Bitmap*)g_pFresh->RegisterBitmap(name))

void CShopMain::CheckAuxCartScroll()
{
	FrScrollBar* pBar = m_pAuxPartCartList->GetScrollBar();
	int top = pBar->GetCurTopRow_Int();
	if (pBar->GetRowCapacity() < pBar->GetItemNum())
	{
		m_pScrollUp->SetVisible(true);
		m_pScrollDown->SetVisible(true);
		if (top == 0)
		{
			m_pScrollUp->Enable(false);
			m_pScrollDown->Enable(true);
		}
		else
		{
			m_pScrollUp->Enable(true);
			m_pScrollDown->Enable(false);
		}
		if (m_pBuyReset)
		{
			// 983 00a840b0: as setas se posicionam pela lista do carrinho auxiliar (0x148), nao pelo botao Reset
			WRect rcList;
			WRect rcDown;
			WRect rcUp;
			m_pAuxPartCartList->GetClientRect(rcList);
			m_pScrollDown->GetClientRect(rcDown);
			m_pScrollUp->GetClientRect(rcUp);
			rcDown.x = (rcList.x - rcDown.w) + 118.0;
			rcUp.x = (rcDown.x - rcUp.w) - 2.0;
			float dh = rcList.h - rcDown.h;
			rcUp.y = (dh * 0.5 + rcList.y) - 156.0;
			rcDown.y = rcUp.y;
			m_pScrollUp->SetClientRect(rcUp);
			m_pScrollDown->SetClientRect(rcDown);
		}
	}
	else
	{
		m_pScrollUp->SetVisible(false);
		m_pScrollDown->SetVisible(false);
	}
}

void CShopMain::TurnOffAuxCartScroll()
{
	m_pScrollUp->SetVisible(false);
	m_pScrollDown->SetVisible(false);
}

void CShopMain::TurnOnAuxCartScroll()
{
	m_pScrollUp->SetVisible(true);
	m_pScrollDown->SetVisible(true);
}

void CShopMain::OnScrollUpInit(int param)
{
	m_pScrollUp = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pScrollUp)
		m_pScrollUp->SetVisible(false);
}

void CShopMain::OnScrollUpUp()
{
	m_pAuxPartCartList->GetScrollBar()->ScrollUp(-1);
	CheckAuxCartScroll();
}

void CShopMain::OnScrollDownInit(int param)
{
	m_pScrollDown = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pScrollDown)
		m_pScrollDown->SetVisible(false);
}

void CShopMain::OnScrollDownUp()
{
	m_pAuxPartCartList->GetScrollBar()->ScrollDown(-1);
	CheckAuxCartScroll();
}

void CShopMain::OnItemSlotFrame1Init(int param)
{
	m_pItemSlotFrame1 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pItemSlotFrame1)
		m_pItemSlotFrame1->SetVisible(false);
}

void CShopMain::OnItemSlotFrame2Init(int param)
{
	m_pItemSlotFrame2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pItemSlotFrame2)
		m_pItemSlotFrame2->SetVisible(false);
}

// 983: sem funcao propria (inline no chamador)
// 983 006336a0 (sem nome): item escondido do jogador (igual ao equipdlg.cpp)
static bool __stdcall IsHiddenItem983(unsigned long tid)
{
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1e00000)
		return true;
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1f00000)
		return !(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 0x2804);
	return ItemManager()->IsNonVisibleItem(tid) == true;
}

void CShopMain::BuildMyItemSlot(bool bAll)
{
	m_pItemSlot->ClearItem();

	std::list<sItemInfo>::iterator it =
		CSharedDoc::Instance()->m_myItemList.begin();
	if (it == CSharedDoc::Instance()->m_myItemList.end())
		return;

	do
	{
		sItemInfo& info = *it;
		if (ItemManager()->GetCouponKind(info.tid) != 0)
			continue;
		bool hidden = IsHiddenItem983(info.tid) == true;
		IFF_STRUCT::sItem* pFound = ItemManager()->FindItem(info.tid);
		if (pFound && pFound->Artifact != 0)
			continue;
		if (hidden == true || info.tid == 0x1a000042)
			continue;

		// 983: a contagem dos 10 slots e um bloco so, usado pelo bAll com
		// tid 0x2000000 e pelo caminho sem conteudo local sem esse bit
		int num = 0;
		if (bAll)
		{
			if ((info.tid & 0x2000000) == 0)
				continue;
		}
		else if (IsLocalContent((localContentType_t)0x81 /*983: era 0x84; interpolado*/))
		{
			CCardManager::Instance()->SetPlayerIndex(0xff);
			CCardManager::Instance()->CalcCardPeriodAndStatus();
			int limit = CSharedDoc::Instance()->GetNumAddItemSlotByMascot() + 8;
			int period = CCardManager::Instance()->GetCardPeriodSlot();
			if (period > 0)
				limit += period;

			if ((info.tid & 0x2000000) != 0)
				continue;
			CSharedDoc* pDoc = CSharedDoc::Instance();
			unsigned long tid = info.tid;
			int i = 0;
			do
			{
				if (i < limit)
				{
					if (tid == pDoc->m_myInfo.userEquip.tidItemSlot[i])
						num++;
				}
				else if (tid == pDoc->m_myInfo.userEquip.tidItemSlot[i])
				{
					pDoc->m_myInfo.userEquip.tidItemSlot[i] = 0;
					pDoc = CSharedDoc::Instance();
				}
				i++;
			} while (i < 10);
			if (info.Common[0] - num > 0)
				m_pItemSlot->AddItem(&info);
			continue;
		}
		else if ((info.tid & 0x2000000) != 0)
			continue;

		{
			unsigned long tid = info.tid;
			const unsigned long* slot =
				CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot;
			if (tid == slot[0]) num++;
			if (tid == slot[1]) num++;
			if (tid == slot[2]) num++;
			if (tid == slot[3]) num++;
			if (tid == slot[4]) num++;
			if (tid == slot[5]) num++;
			if (tid == slot[6]) num++;
			if (tid == slot[7]) num++;
			if (tid == slot[8]) num++;
			if (tid == slot[9]) num++;
		}
		if (info.Common[0] - num > 0)
			m_pItemSlot->AddItem(&info);
	} while (++it != CSharedDoc::Instance()->m_myItemList.end());
}

void CShopMain::OnItemSlotInit(int param)
{
	m_pItemSlot = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pItemSlot)
	{
		m_pItemSlot->SetVisible(false);
		m_pItemSlot->SetMouseEvent(false);
	}
}

void CShopMain::OnItemSlotOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item == NULL)
		return;

	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	if (gfx == NULL)
		return;

	if (m_pBmpItemSlotN)
	{
		WRect rc(item->pos.x, item->pos.y, (float)m_pBmpItemSlotN->Width(),
			(float)m_pBmpItemSlotN->Height());
		gfx->DrawTexture(m_pBmpItemSlotN, rc, 0xffffffff, 0);
	}

	sItemInfo* info = (sItemInfo*)item->pData;
	if (info == NULL)
		return;

	CItemManager* pItemMgr = ItemManager();
	IFF_STRUCT::sItem* pItem = pItemMgr->FindItem(info->tid);
	if (pItem == NULL)
		return;

	const Bitmap* icon = g_pFresh->GetBitmap(pItem->c.Icon);
	if (icon)
	{
		WRect rc(item->pos.x + 2.0f, item->pos.y + 4.0f, (float)icon->Width(),
			(float)icon->Height());
		gfx->DrawTexture(icon, rc, 0xffffffff, 0);
	}

	// 983 00a86530: slots de item (sUserEquip::tidItemSlot) que ja tem este tid, um if por slot
	unsigned long used = 0;
	if ((info->tid & 0x2000000) == 0)
	{
		const unsigned long* slot = CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot;
		if (info->tid == slot[0]) ++used;
		if (info->tid == slot[1]) ++used;
		if (info->tid == slot[2]) ++used;
		if (info->tid == slot[3]) ++used;
		if (info->tid == slot[4]) ++used;
		if (info->tid == slot[5]) ++used;
		if (info->tid == slot[6]) ++used;
		if (info->tid == slot[7]) ++used;
		if (info->tid == slot[8]) ++used;
		if (info->tid == slot[9]) ++used;
	}
	// quantidade com a WTITLEFONT da loja (+0x6ac), alinhada a direita em (+45, +40), espaco 1.0
	float x = item->pos.x + 45.0f;
	float y = item->pos.y + 40.0f;
	gfx->PrintTitleFont983(m_pTitleFont, x, y,
		MakeStr("%d", (int)info->Common[0] - (int)used), 1.0f, 2, 1.0f, 0xffffffff);
}

void CShopMain::OnPowerEditInit(int param)
{
	m_pStatEdit[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CShopMain::OnControlEditInit(int param)
{
	m_pStatEdit[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CShopMain::OnImpactEditInit(int param)
{
	m_pStatEdit[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CShopMain::OnSpinEditInit(int param)
{
	m_pStatEdit[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CShopMain::OnCurveEditInit(int param)
{
	m_pStatEdit[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

bool CShopMain::OnBuyDlgResult(int result, FrForm* dlg)
{
	if (result != 1)
	{
		m_pCheckoutDlg = 0;
		return true;
	}
	FrListBox::ITEM_LIST::iterator it = m_pPartCartList->m_itemList.begin();
	while (it != m_pPartCartList->m_itemList.end())
	{
		IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
		if ((pData->TypeId & 0xfc000000) == 0x8000000)
		{
			if (IsUccClothes(pData->TypeId))
				RestoreShoppersOldItem(pData->TypeId,
					CSharedDoc::Instance()->GetMyPartTidList());
		}
		++it;
	}
	ResetItemList(false);
	ClearCartList(false, NULL);
	m_oldItems.clear();
	m_pCheckoutDlg = 0;
	return true;
}

CShopMain::miniBtn_t::miniBtn_t()
{
}

bool CShopMain::OnSellDlgResult(int result, FrForm* dlg)
{
	if (result == 1)
	{
		FrRepayDlg* pDlg = DYNAMIC_CAST(FrRepayDlg, dlg);
		if (pDlg)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			int type;
			switch (pDlg->GetTid() >> 26)
			{
			case 1:
				type = 0;
				break;
			case 7:
				type = 1;
				break;
			default:
				type = 2;
				break;
			}
			WSendPacket packet((enumClientPacket)0x1e);
			packet.Encode1(type);
			packet.Encode1(1);
			packet.Encode4(pDlg->GetTid());
			packet.Encode4(pDlg->GetGuid());
			packet.Send(TO_GAME);
		}
	}
	return true;
}

bool CShopMain::OnReemployeeResult(int result, FrForm* dlg)
{
	if (result == 1)
	{
		std::map<unsigned int, sCaddieInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_caddieMap.begin();
			it != CSharedDoc::Instance()->m_caddieMap.end(); it++)
		{
		}
	}
	return true;
}

// MakeStatusString (b2e010) e MakeCardStatusString (b2e260): definidas no tradedlg.cpp (globais fastcall)

// 983 b2e180: a==0 so mostra "0/b"; a!=0 e b!=0 -> "%d(-%d)/%d" (a, c, b) ou "%d/%d" (a, b)
char const* __fastcall MakeStatusString2(int a, int b, int c)
{
	if (a != 0)
	{
		if (b != 0)
		{
			if (c != 0)
				return MakeStr("%d(-%d)/%d", a, c, b);
			return MakeStr("%d/%d", a, b);
		}
		return MakeStr("%d", a);
	}
	if (b != 0)
		return MakeStr("%d/%d", 0, b);
	return "";
}

// 00a876f0 (inlinada no ctor em outros pontos): nivel do jogador (CSharedDoc +0x91d) e +1
void CShopMain::InitRookieLevel983()
{
	m_nRookieLv1 = Doc()->m_myInfo.stat.Level + 1;
	m_nRookieLv = Doc()->m_myInfo.stat.Level;
}

// 983 00a86740: mensagem 0x2d8 ao host (abre a busca de itens)
void CShopMain::OnItemSearchBtnUp()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d8, 0, 0, 0, 0, 0));
}

CShopMain::CShopMain() // 00a91ec0
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
	m_u72c = 0;
	m_cutinImgMap.clear();
	InitShopControls();
	InitShopDialogs();
	m_u528 = 0;
	m_u524 = 0;
	m_mascotTid = 0x40000000;
	m_clubsetTid = 0x10000000;
	m_aztecTid = 0x14000000;
	m_pCharPCL = NULL;
	m_pParts = NULL;
	m_pAuxParts = NULL;
	m_b520 = false;
	m_tidList.clear();
	m_statTab = (statTab_t)0;
	m_mainTab = (mainTab_t)0;
	m_prevMainTab = (mainTab_t)0;
	m_f544 = 0;
	m_subTab = (subTab_t)0;
	m_rc6b0 = s_rcEmpty983;
	m_pSortOptionCombo = NULL;
	m_pSearchGoodsButton = NULL;
	m_pItemSearchBtn = 0;
	InitRookieLevel983();
	InitalizeUpgradeCaddieInfo();
	m_n6a8 = -1;
	m_n2e4 = 0;

	static char* s_fontWind[1] = { "[font_wind.jpg" };
	WTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	fontInfo.filename = s_fontWind;
	fontInfo.numPages = 1;
	fontInfo.fontw = 16;
	fontInfo.fonth = 16;
	fontInfo.texw = 128;
	fontInfo.texh = 64;
	fontInfo.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&fontInfo);
}

CShopMain::~CShopMain() // 00a91640
{
	DeleteCardImages(); // 983 00a88dd0 (o nosso nao soltava as cartas)
	DeleteCutinImages();
	ReleaseExhibition(true);
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
	m_hand.BreakWorkingThread();
	if (CTaskManager::Instance())
	{
		if (CTask* pTask = AfxGetTask())
			pTask->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		if (CTaskManager::Instance())
			CTaskManager::Instance()->PostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);
	}
	// 983 a9172a: na conversa de avatar desfaz o que a loja ligou (SetScreen m_layer = 1; OnInit estado | 4)
	if (g_lobbyModal983 == 1)
	{
		if (RFLOBBYDESKTOP983)
			RFLOBBYDESKTOP983->m_layer = 0;
		if (ntAvatarManager::m_pInstance)
		{
			ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
			if (pMyAvatar)
				pMyAvatar->SetState(pMyAvatar->m_state983 & ~4);
		}
	}
	if (g_resrcmng && m_pTitleFont) // 983 a91764: a WTitleFont do ctor (+0x6ac)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

// 00a868e0: carrega as imagens da loja e avisa a janela "LobbyAvatar" com a mensagem 0x187 (personagem ativo)
bool CShopMain::OnOpenWidget983()
{
	LoadShopImages();
	sCharacterInfo* ch = Doc()->FindChar983(0xffffffff);
	if (ch)
	{
		IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
		if (pActor)
			pActor << MsgObject(NULL, 0x187, (int)ch, 1, 0, 0, 0);
	}
	return true;
}

// 00a86960: ao fechar a widget reconstroi a lista de partes do personagem ativo
bool CShopMain::OnCloseWidget983()
{
	FrTaskForm::OnCloseWidget983();
	sCharacterInfo* ch = Doc()->FindChar983(0xffffffff);
	if (ch)
		Doc()->BuildMyPartTidList(*ch);
	return true;
}

void CShopMain::OnProc(const float deltaTime) // 00a91950
{
	// 983: IsWorking(0) aberto (o inline dele piora a ordem dos saltos); membros via friend do CAnotherHand
	CRITICAL_SECTION* pCs = &m_hand.m_csQueue;
	EnterCriticalSection(pCs);
	m_hand.m_bQueseLock = true;
	if (m_hand.m_curWork == m_hand.m_vacation ||
		m_hand.m_aWork[m_hand.m_curWork].key != 0)
	{
		m_hand.m_bQueseLock = false;
		LeaveCriticalSection(pCs);
		LOCK_USER(&m_hand);
		bool bInput;
		switch (m_mainTab)
		{
		case 2:
		case 5:
			bInput = false;
			break;
		default:
			bInput = true;
			break;
		}
		if (m_pExhibition)
			m_pExhibition->Process(deltaTime, bInput);
		UNLOCK_USER(&m_hand);
	}
	else
	{
		m_hand.m_bQueseLock = false;
		LeaveCriticalSection(pCs);
		Sleep(10);
	}
	if (m_f128 > 0.0f)
	{
		m_f128 -= deltaTime;
		if (m_f128 < 0.1f)
		{
			m_f128 = -10.0f;
			if (m_n124)
				((FrWnd*)m_n124)->SetVisible(false);
		}
	}
	if (m_pCheckoutDlg)
		m_pCheckoutDlg->Process();
	if (m_mainTab == 5)
	{
		int i = 0;
		do
		{
			float speed;
			if (i == 0)
				speed = 138.0f;
			else
				speed = 228.0f;
			switch ((&m_u724)[i])
			{
			case 0:
				(&m_f71c)[i] = 0;
				break;
			case 1:
				(&m_f71c)[i] -= speed * deltaTime * 0.25f;
				break;
			case 2:
				(&m_f71c)[i] += speed * deltaTime * 0.25f;
				break;
			}
			i++;
		} while (i < 2);
	}
	if (g_input->GetDown("ESCAPE", true) == 1 &&
		!g_pFresh->GetManager()->HasEscKeyWindow())
		CMessengerInfo::Instance()->Close(false);
	if (!m_tidList.empty())
	{
		CSharedDoc::Instance()->GetMyPartTidList();
		for (std::list<unsigned long>::iterator it = m_tidList.begin();
			it != m_tidList.end(); ++it)
			UnselectItemList(*it);
		m_tidList.clear();
		if (m_mainTab == 1 || m_mainTab == 5)
		{
			m_pBuyAll->Enable(!m_pPartCartList->m_itemList.empty() ||
				!m_pAuxPartCartList->m_itemList.empty());
			m_pBuyReset->Enable(!m_pPartCartList->m_itemList.empty() ||
				!m_pAuxPartCartList->m_itemList.empty());
		}
	}
	// 983 00a91950: dica de Buy/Gift (0x1b/0x1c) apagada a cada quadro
	int tip = TOOLTIP()->GetCurTipIndex();
	if (tip == TOOLTIP_BUY || tip == TOOLTIP_GIFT)
		TOOLTIP()->ClearToolTip();
	m_f548 += deltaTime;
	m_f544 = fabsf(sinf(m_f548 * 4.0f)) * 5.0f;
}

struct sAttr5
{
	short v[5];
};

// HandleMsg zero-fills its sItemInfo locals with a plain memset; the constructor never runs on them.
struct sRawItemInfo
{
	char raw[sizeof(sItemInfo)];
};

// JP 983 (00a87980, metodo da CShopMain no 983 que nao le o this): pacote 0x140 ao jogo e mensagem 1 ao ator principal.
// Fim das mensagens 0xe9/0xea (so do 983) que o CShopTask::OnPacket manda no resultado de compra 0x35 e depois de todo
// resultado (lote 11 da C-PKT). Metodo desde a s101 da A.
void CShopMain::SendShopRefresh983()
{
	WSendPacket packet((enumClientPacket)0x140);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}

// 983 00a950f0..00a962f0 (13 copias do mesmo functor, uma por mapa do CItemManager): item a venda na loja entra na lista da
// busca como {tid, 0xffffffff, Icon, Name, 0} (00a94fa0 -> ctor 00616500)
template <class MAP>
static void CollectSearchItems983(MAP& m, std::list<sItemSearchResult983>& out, int group)
{
	for (typename MAP::iterator it = m.begin(); it != m.end(); ++it)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second;
		unsigned long tid = item.TypeId;
		unsigned long lv = CSharedDoc::Instance()->m_myInfo.stat.Level;
		if (ItemManager()->IsLevelLocked(&item, &lv) || item.InStock == 0)
			continue;
		if (!ItemManager()->IsInSalePeriod(tid, CSharedDoc::Instance()->GetServerTime()))
			continue;
		switch (tid >> 26)
		{
		case 6:
			if (tid == 0x24dfe00a)
				continue;
			break;
		case 7:
			if (!ItemManager()->IsSellableItem(&item, CSharedDoc::Instance()->GetServerTime()))
				continue;
			break;
		case 9:
			if (group == 0)
			{
				if ((tid & 0x3e00000) != 0x200000)
					continue;
			}
			else if (group == 1)
			{
				unsigned long sub = tid >> 21 & 0x1f;
				if (sub < 5 || (sub > 6 && sub != 8) || (tid >> 13 & 0xff) < 0xfd)
					continue;
			}
			break; // grupo 4 cai no caso 0xe, que so filtra tid 0x38 (009da070): set entra
		case 0xe:
			if ((tid & 0x3c00000) == 0x1400000 && !ItemManager()->FindCutinInformation(tid))
				continue;
			break;
		case 0x12:
			if (tid & 0x2000000)
				continue;
			break;
		}
		sItemSearchResult983 r;
		r.typeId = tid;
		r.uid = 0xffffffff;
		r.text = item.Icon;
		r.name = item.Name;
		r.linkedTypeId = 0;
		out.push_back(r);
	}
}

// 983 00a82730 (0x2d6, D639): aba principal do item pelo tipo do tid
CShopMain::mainTab_t CShopMain::MainTabOfTid983(unsigned long tid)
{
	if (tid != 0)
	{
		switch (tid >> 0x1a)
		{
		case 1:
			return (mainTab_t)0;
		case 2:
		case 9:
			switch ((tid >> 0x15) & 0x1f)
			{
			case 5:
			case 6:
			case 8:
				return (mainTab_t)2;
			}
			break;
		case 4:
		case 5:
		case 6:
		case 0x1f:
			return (mainTab_t)2;
		case 7:
		case 8:
			return (mainTab_t)3;
		case 0xe:
			return (mainTab_t)5;
		case 0x10:
			return (mainTab_t)4;
		}
	}
	return (mainTab_t)1;
}

// 983 00a87720 (0x2d6, D639): sub-aba do item pelo tipo do tid (roupa pela Category do Part)
CShopMain::subTab_t CShopMain::SubTabOfTid983(unsigned long tid)
{
	if (tid != 0)
	{
		switch (tid >> 0x1a)
		{
		case 1:
			return (subTab_t)1;
		case 2:
			switch (ItemManager()->FindPart(tid)->Category)
			{
			case 1:
				return (subTab_t)3;
			case 2:
			case 3:
			case 5:
				return (subTab_t)5;
			case 4:
				return (subTab_t)4;
			case 6:
				return (subTab_t)6;
			case 7:
			case 8:
			case 9:
				return (subTab_t)0x1b;
			}
			break;
		case 4:
			return (subTab_t)7;
		case 5:
			return (subTab_t)8;
		case 6:
			return (subTab_t)(((tid & 0x2000000) != 0) + 9);
		case 7:
		case 8:
			return (subTab_t)0xb;
		case 9:
			switch ((tid >> 0x15) & 0x1f)
			{
			default:
				return (subTab_t)1;
			case 5:
				return (subTab_t)8;
			case 6:
				return (subTab_t)0xa;
			case 8:
				return (subTab_t)0x1c;
			}
		case 0xe:
			switch ((tid >> 0x16) & 0xf)
			{
			case 0:
				return (subTab_t)0xe;
			case 1:
				return (subTab_t)0xf;
			case 2:
				return (subTab_t)0x10;
			case 3:
				return (subTab_t)0x11;
			case 4:
			case 5:
				return (subTab_t)0x22;
			case 6:
				return (subTab_t)0x23;
			}
			break;
		case 0xf:
			return (subTab_t)6;
		case 0x10:
			return (subTab_t)0xd;
		case 0x12:
			switch ((tid >> 0xb) & 0x7ff)
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				return (subTab_t)0x20;
			default:
				return (subTab_t)0x1d;
			case 8:
				return (subTab_t)0x1f;
			case 9:
				return (subTab_t)0x1e;
			}
		case 0x1c:
			return (subTab_t)5;
		case 0x1f:
			return (subTab_t)0x1c;
		}
	}
	return (subTab_t)2;
}

// 983 00a96f00 (0x2d3 da FrItemSearchDlg, D638): junta os itens a venda do grupo do combo (0 roupa, 1 item, 2 caddie, 3 mascote,
// 4 skin, 5 mobilia) e devolve os que tem a palavra no nome (strstr)
void CShopMain::SearchItems983(std::list<sItemSearchResult983>& results, int group, const std::string& word)
{
	CItemManager* im = ItemManager();
	std::list<sItemSearchResult983> all;
	switch (group)
	{
	case 0:
		CollectSearchItems983(im->m_CharMap, all, group);
		CollectSearchItems983(im->m_PartMap, all, group);
		CollectSearchItems983(im->m_SetItemMap, all, group);
		CollectSearchItems983(im->m_HairStyleMap, all, group);
		break;
	case 1:
		CollectSearchItems983(im->m_SetItemMap, all, group);
		CollectSearchItems983(im->m_ClubSetMap, all, group);
		CollectSearchItems983(im->m_BallMap, all, group);
		CollectSearchItems983(im->m_ItemMap, all, group);
		CollectSearchItems983(im->m_CardMap, all, group);
		break;
	case 2:
	{
		CollectSearchItems983(im->m_CaddieMap, all, group);
		CollectSearchItems983(im->m_CadItemMap, all, group);
		// 00a935a0: caddie (7) e item de caddie (8) so ficam se o caddie esta no m_caddieIffMap (+0x1c0)
		for (std::list<sItemSearchResult983>::iterator it = all.begin(); it != all.end();)
		{
			unsigned long tid = it->typeId;
			unsigned long kind = tid >> 26;
			if (tid != 0 && (kind == 7 || kind == 8))
			{
				if (kind == 8)
					tid = ((tid >> 21 & 0x1f) + (tid >> 16 & 0x1f)) | 0x1c000000;
				if (m_caddieIffMap.find(tid) == m_caddieIffMap.end())
				{
					it = all.erase(it);
					continue;
				}
			}
			++it;
		}
		break;
	}
	case 3:
		CollectSearchItems983(im->m_MascotMap, all, group);
		break;
	case 4:
		CollectSearchItems983(im->m_SkinMap, all, group);
		break;
	case 5:
		CollectSearchItems983(im->m_FurnitureMap, all, group);
		break;
	}
	for (std::list<sItemSearchResult983>::iterator it = all.begin(); it != all.end(); ++it)
		if (strstr(it->name.c_str(), word.c_str()))
			results.push_back(*it);
}

// 983 00a82d00: partes com o prazo do pacote em horas (as outras vem em dias)
static bool IsHourPart983(unsigned long tid)
{
	switch (tid)
	{
	case 0x801680c: case 0x805880c: case 0x809880c: case 0x80dc80c: case 0x811880c: case 0x816080c:
	case 0x819080c: case 0x81e280c: case 0x821480c: case 0x825480c: case 0x839c80a:
		return true;
	}
	return false;
}

bool CShopMain::OnMsg983(const MsgObject& msg) // 00a971f0
{
	switch (msg.message)
	{
	case 0xe9 /*so do 983*/:
		// 983 (00a97271): aviso pelo codigo do servidor (limite de venda do dia, preco) e depois o fim de 00a87980
		switch (msg.param1)
		{
		case 0x301881:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xdd\xc0\xcf(\xc8\xb8) \xc1\xd8\xba\xf1\xb5\xc8 \xbc\xf6\xb7\xae\xc0\xcc \xb8\xf0\xb5\xce \xb8\xc5\xc1\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			break;
		case 0x2f4d77:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)GetGameString(0x13886), 0, 0, 0, 0));
			break;
		case 0x301882:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xdd\xc0\xcf(\xc8\xb8) \xb1\xb8\xb8\xc5\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xc7\xd1\xb5\xb5\xb0\xa1 \xb8\xf0\xb5\xce \xbc\xd2\xc1\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			break;
		case 0x301883:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb0\xa1\xb0\xdd\xc0\xbb \xb4\xd9\xbd\xc3 \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0));
			break;
		default:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /*so do 983: aviso pelo codigo*/, msg.param1, 0, 0, 0, 0));
			break;
		}
		SendShopRefresh983();
		break;
	case 0xea /*so do 983*/:
		// 983 (00a974b9)
		SendShopRefresh983();
		break;
	case 0x2d3: // 983 00a99131: busca da FrItemSearchDlg (p1 grupo, p2 palavra, p3 lista de resultados)
		SearchItems983(*(std::list<sItemSearchResult983>*)msg.param3, msg.param1, *(const std::string*)msg.param2);
		return true;
	case 0x2d6: // 983 00a9914b: clique num resultado da busca (p1 tid, p3 tid ligado) -> vai a aba/sub-aba do item e o mostra (D639)
	{
		unsigned long tid = msg.param1;
		if (m_hand.IsWorking(0) || tid == 0)
			break;
		unsigned long linked = msg.param3;
		mainTab_t oldMain = m_mainTab;
		unsigned long type = tid >> 0x1a;
		mainTab_t mainTab = MainTabOfTid983(tid);
		subTab_t subTab = SubTabOfTid983(tid);
		if (m_mainTab != mainTab)
		{
			m_pTabMain->Select(mainTab);
			ChangeMainTab(mainTab, false);
		}
		if (m_subTab != subTab)
		{
			if (subTab == 0xc)
				subTab = SUBTAB_CADDIE;
			ChangeSubTab(subTab, false);
		}
		switch (mainTab)
		{
		case 1:
		{
			unsigned long charTid;
			if (subTab == 1)
			{
				if (type == 1)
					charTid = tid;
				else if (((tid >> 0xd) & 0xff) != 0xfc)
					charTid = ((tid >> 0xd) & 0xff) | 0x4000000;
				else
					charTid = ((linked >> 0x12) & 0xff) | 0x4000000;
			}
			else if (subTab == 5 && type == 0x1c)
				charTid = m_u524;
			else if (subTab == SUBTAB_HAIR)
				charTid = ItemManager()->FindHairStyle(tid)->cCharID | 0x4000000;
			else
				charTid = ((tid >> 0x12) & 0xff) | 0x4000000;
			if (m_u524 != charTid)
			{
				m_u524 = charTid;
				ChangeCharacter(charTid);
			}
			if (oldMain != mainTab && mainTab == 1)
				ChangeCharacter(m_u524);
			if (subTab == 1)
				ChangeSubTab(subTab, false);
			break;
		}
		case 2:
		case 3:
			if (m_subTab != subTab)
			{
				if (subTab == 0xc)
					subTab = SUBTAB_CADDIE;
				ChangeSubTab(subTab, false);
			}
			if (type == 8)
				ChangeCaddie((((tid >> 0x15) & 0x1f) + ((tid >> 0x10) & 0x1f)) | 0x1c000000);
			else
				ChangeCaddie((tid & 0x3ffffff) | 0x1c000000);
			break;
		case 5:
			if (m_subTab == SUBTAB_CUTIN)
			{
				IFF_STRUCT::S5::sCutinInformation* pCutin = ItemManager()->FindCutinInformation(tid);
				if (pCutin)
				{
					if ((unsigned long)pCutin->CharIndex != ((tid >> 0x12) & 0xff))
						m_u524 = pCutin->CharIndex | 0x4000000;
					ChangeCharacter(m_u524);
				}
			}
			ChangeSkin(m_u524);
			break;
		}
		return true;
	}
	case 0x2d7: // 983 00a9937a: escolha de um resultado da busca (p1 tid) -> marca, seleciona e rola ate o item, veste o boneco e fecha a busca (D640)
	{
		m_pItemList->UnselectAllItem();
		unsigned long tid = msg.param1;
		if (tid == 0)
		{
			m_n2e4 = 0;
			return true;
		}
		m_n2e4 = tid;
		switch (tid >> 0x1a)
		{
		case 6:
		case 9:
		case 0xe:
		case 0x10:
		case 0x12:
		{
			// 983 00a993bc: item da lista com o tid; sem ele, nada mais
			FrListItem* pFound = NULL;
			FrListBox::ITEM_LIST::iterator it = m_pItemList->m_itemList.begin();
			for (; it != m_pItemList->m_itemList.end(); ++it)
			{
				IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
				if (pData && pData->TypeId == tid)
				{
					pFound = *it;
					break;
				}
			}
			if (pFound == NULL)
				return true;
			if (m_subTab != 1)
			{
				if (m_subTab == 0x22)
					m_pItemList->SelectItem(pFound, true);
				else
					SelectItemList(pFound);
			}
			break;
		}
		}
		// 983 00a99422: personagem do tid ao "LobbyAvatar" (0x187, p2 0) e fecha a gaveta de personagens
		unsigned long charTid = ((tid >> 0x12) & 0xff) | 0x4000000;
		if (ItemManager()->FindChar(charTid))
		{
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << MsgObject(NULL, 0x187, (int)&charTid, 0, 0, 0, 0);
			if (RFLobbyCharacterSelectAnimWnd::ms_pInstance && RFLobbyCharacterSelectAnimWnd::ms_pInstance->m_bOpen)
				RFLobbyCharacterSelectAnimWnd::ms_pInstance->Select983(2, false);
		}
		ScrollToTid983(tid);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d9, 0, 0, 0, 0, 0));
		return true;
	}
	case M983(0x6a):
	{
		if (m_mainTab != msg.param1)
			break;
		std::map<unsigned int, sCharacterInfo>::iterator cit =
			CSharedDoc::Instance()->m_charMap.begin();
		for (; cit != CSharedDoc::Instance()->m_charMap.end(); ++cit)
		{
			if (m_u524 == cit->second.tid)
			{
				if (CSharedDoc::Instance()->m_myInfo.userEquip.guidChar !=
					cit->second.guid)
				{
					WSendPacket packet((enumClientPacket)0x20);
					packet.Encode1(5);
					packet.Encode4(cit->second.guid);
					packet.Send(TO_GAME);
				}
				break;
			}
		}
		if (cit == CSharedDoc::Instance()->m_charMap.end())
			break;
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_u524);
		if (!pChar)
			break;
		sCharacterInfo info = cit->second;
		bool bChanged = false;
		for (int i = 0; i < pChar->nAcsries + pChar->nParts; i++)
		{
			if (info.tidParts[i] == m_pParts[i])
				continue;
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(m_pParts[i]);
			if (pPart)
			{
				unsigned char myLevel =
					CSharedDoc::Instance()->m_myInfo.stat.Level;
				if (pPart->c.IsUnderLvl == 0 && pPart->c.Level > myLevel)
					continue;
				if (pPart->c.IsUnderLvl == 1 && pPart->c.Level < myLevel)
					continue;
			}
			if (IsUccClothes(m_pParts[i]))
				continue;
			info.tidParts[i] = m_pParts[i];
			info.ItemIdList[i] = m_partGuid983[i];
			bChanged = true;
		}
		for (int i = pChar->nAcsries + pChar->nParts; i < 24; i++)
			info.tidParts[i] = 0;
		for (int i = 0; i < 5; i++)
		{
			if (info.tidAuxParts[i] == m_pAuxParts[i])
				continue;
			IFF_STRUCT::sAuxPart* pAux =
				ItemManager()->FindAuxPart(m_pAuxParts[i]);
			if (pAux &&
				pAux->c.Level > CSharedDoc::Instance()->m_myInfo.stat.Level)
				continue;
			info.tidAuxParts[i] = m_pAuxParts[i];
			bChanged = true;
		}
		if (info.hairClr != (unsigned char)m_b520)
			info.hairClr = m_b520;
		else if (!bChanged)
			break;
		{
			WSendPacket packet((enumClientPacket)0x20);
			packet.Encode1(0);
			packet.EncodeBuffer(&info, sizeof(info));
			packet.Send(TO_GAME);
		}
		break;
	}
	case M983(0xc4):
		if (msg.param1 < 6)
		{
			m_mainTab = (mainTab_t)msg.param1;
			if (msg.param2 < 0x25)
				m_subTab = (subTab_t)msg.param2;
			if (m_mainTab == 1 && m_subTab > 6)
				m_subTab = (subTab_t)1;
		}
		else
		{
			m_mainTab = (mainTab_t)0;
			m_subTab = (subTab_t)0;
		}
		break;
	case M983(0xc3):
		*(int*)msg.param1 = m_mainTab;
		*(int*)msg.param2 = m_subTab;
		break;
	case 0:
		OpenLayout((const char*)msg.param1);
		break;
	case 0xe8 /*so do 983*/: // 00a9725c
		BuildDisplayStand(m_subTab);
		break;
	case M983(0xc6):
		if (msg.param1 == 3)
		{
			m_mainTab = (mainTab_t)1;
			m_subTab = (subTab_t)0x1b;
			ChangeMainTab((mainTab_t)1, true);
			ChangeSubTab((subTab_t)0x1b, true);
			unsigned long tid = msg.param2;
			if (!tid)
				tid = m_u524;
			ChangeCharacter(tid);
			break;
		}
		if (msg.param1 == 4)
		{
			if (!IsLocalContent(S4_CARD_SYSTEM))
				break;
			m_mainTab = (mainTab_t)2;
			m_subTab = (subTab_t)0x1c;
			ChangeMainTab((mainTab_t)2, true);
			ChangeSubTab((subTab_t)0x1c, true);
			ReleaseExhibition(true);
			break;
		}
		// 983 00a9792f: abre na aba de um item em promocao ao acaso (0063b960)
		if (msg.param1 == 5)
		{
			std::map<unsigned long, IFF_ITEM_COMMON>& promo = CSharedDoc::Instance()->m_promoItemsNow983;
			if (promo.empty())
				break;
			std::map<unsigned long, IFF_ITEM_COMMON>::iterator it = promo.begin();
			std::advance(it, rand() % promo.size());
			unsigned long tid = it->second.TypeId;
			if (!tid)
				break;
			if (tid >> 26 == 2)
			{
				m_mainTab = (mainTab_t)1;
				m_subTab = (subTab_t)2;
				m_u524 = (tid >> 18 & 0xff) | 0x4000000;
			}
			else if (tid >> 26 == 6)
			{
				m_mainTab = (mainTab_t)2;
				m_subTab = (subTab_t)10;
			}
			else
			{
				m_subTab = (subTab_t)2;
				m_mainTab = (mainTab_t)1;
				ChangeCharacter(0x4000000);
			}
		}
		break;
	// 983 00a971f0: sem caso 0xec (M983(0xc7)); o do 645 fechava m_pNotifyOneDlg (membro do 645 sem par, removido C699)
	case M983(0x49):
	{
		const unsigned long* pInfo = (const unsigned long*)msg.param2;
		CTask* pTask = CTaskManager::Instance()->GetPreservedTask();
		jp983_rastro("A803 loja 0x49 tid=%08x guid=%08x u524=%08x avatarchat=%d", pInfo[0], pInfo[1], m_u524,
			pTask && pTask->GetActor("AvatarChat") ? 1 : 0);
		if (pTask && pTask->GetActor("AvatarChat"))
		{
			pTask->GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x49), msg.param1, msg.param2, 0, 0, 0);
		}
		else
		{
			CSharedDoc::Instance()->m_myInfo.userEquip.guidChar = pInfo[1];
		}
		ChangeCharacter(pInfo[0]);
		break;
	}
	case M983(0x4a):
	{
		unsigned long* pIds = (unsigned long*)msg.param2;
		if (pIds[1])
			ChangeCaddie(pIds[1]);
		CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie = pIds[0];
		break;
	}
	case M983(0x4b):
	{
		unsigned long* pIds = (unsigned long*)msg.param2;
		ChangeClubset(pIds[1]);
		CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet = pIds[0];
		break;
	}
	case M983(0x4c):
		ChangeAztec(msg.param2);
		CSharedDoc::Instance()->m_myInfo.userEquip.tidBall = msg.param2;
		break;
	case 0x5a /*so do 983*/:
		// 983: resolucao mudou (CClientSetting posta 0x5a ao Lobby); na aba de roupa pede de novo o rect do
		// boneco ao LobbyAvatar e recoloca as listas do carrinho (00a84da0); fora dela zera o rect (00f14668)
		if (m_mainTab == 1)
		{
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x194, (int)&m_rc6b0, 0, 0, 0, 0);
			PlacePartCartLists983();
			return true;
		}
		else
			m_rc6b0 = s_rcEmpty983;
		return true;
	case M983(0x4e):
	{
		unsigned long* pIds = (unsigned long*)msg.param2;
		if (pIds[1])
			ChangeMascot(pIds[1]);
		CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot = pIds[0];
		CSharedDoc::Instance()->SendEquipItemSlot(); // 983 00a97b1a
		break;
	}
	case M983(0x68):
		// 983 (0x75): na aba de roupa tira da lista do documento so as partes especiais (0062fd70), com a cor de cabelo da
		// loja, e recarrega boneco e vitrine
		if (m_mainTab == 1)
		{
			StripSpecialPartTids983(m_u524);
			CPartTidList& tids = CSharedDoc::Instance()->GetMyPartTidList();
			tids.m_hairColor = m_b520;
			m_pParts = tids.m_tid;
			SafeLoadPet(&m_u524, true);
			BuildDisplayStand(m_subTab);
			return true;
		}
		break;
	case M983(0xb0):
		break;
	case M983(0xb5):
	{
		sCharacterInfo info;
		memset(&info, 0, sizeof(info));
		info.guid = msg.param1;
		info.tid = msg.param2;
		ItemManager()->GetDefCombo(info.tid, info.tidParts);
		CSharedDoc::Instance()->m_charMap.insert(
			std::pair<const unsigned int, sCharacterInfo>(info.guid, info));
		CSharedDoc::Instance()->m_myInfo.userEquip.guidChar = info.guid;
		// 983 00a97bc5: boneco comprado vai ao "LobbyAvatar" (0x188 limpa, 0x187 &info) e a gaveta de personagens refaz a lista
		IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
		if (pActor)
			pActor << MsgObject(NULL, 0x188, 0, 0, 0, 0, 0);
		pActor = AfxGetTask()->GetActor("LobbyAvatar");
		if (pActor)
			pActor << MsgObject(NULL, 0x187, (int)&info, 0, 0, 0, 0);
		RFLobbyCharacterSelectAnimWnd* pDrawer = RFLobbyCharacterSelectAnimWnd::ms_pInstance;
		if (pDrawer)
			pDrawer->SetChar983(0xf);
		if (info.guid != (unsigned long)-1 && info.guid != 0 && pDrawer)
			pDrawer->SetButton983(info.tid & 0x3ffffff);
		break;
	}
	case M983(0xb6):
	{
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = msg.param1;
		item.tid = msg.param2;
		if (msg.param4 == 6 || msg.param4 == 2)
		{
			item.HourRemain = IsHourPart983(item.tid) ? msg.param3 : msg.param3 * 24; // 983 00a97d54
			item.ItemType = msg.param4;
		}
		m_partGuid983[item.tid >> 13 & 0x1f] = item.guid;
		if (IsUccClothes(item.tid)) // 983 00a97d78: sem IsLocalContent (o S4_UCC e do 0xef, que nao vem para a loja)
		{
			strncpy(item.UccIndex, (const char*)msg.param5 + 0x1d, 9);
			item.Seq = 1;
			CSharedDoc::Instance()->m_uccItemList.push_back(item);
			break;
		}
		CSharedDoc::Instance()->AddMyPartsList(item);
		break;
	}
	case M983(0xb7):
	{
		sCaddieInfo info;
		memset(&info, 0, sizeof(info));
		info.guid = msg.param1;
		info.tid = msg.param2;
		info.Rent_flag = 1;
		info.Remain_Date = 30;
		if (info.tid >= 0x1c00000c && info.tid <= 0x1c00000e)
		{
			info.Level = (unsigned char)msg.param4;
			info.Exp = msg.param3;
		}
		CSharedDoc::Instance()->m_caddieMap.insert(
			std::pair<const unsigned int, sCaddieInfo>(info.guid, info));
		m_u528 = info.tid;
		BuildVisibleCaddieList();
		break;
	}
	case M983(0xb8):
	{
		unsigned long guid = msg.param1;
		unsigned long tid = msg.param2;
		int hour = msg.param3;
		std::map<unsigned int, sCaddieInfo>::iterator cit =
			CSharedDoc::Instance()->m_caddieMap.find(guid);
		_ReadWriteBarrier();
		if (cit == CSharedDoc::Instance()->m_caddieMap.end())
			break;
		if (!ItemManager()->FindCadItem(tid))
			break;
		unsigned char days = (unsigned short)hour / 24;
		sCaddieInfo& info = cit->second;
		info.tidPart = tid;
		info.Remain_Partdate = (short)hour;
		if (info.Remain_Date < days)
			info.Remain_Date = days;
		break;
	}
	case M983(0xb9):
	{
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = msg.param1;
		item.tid = msg.param2;
		int hour = msg.param3;
		item.ItemType = msg.param4;
		item.HourRemain = hour;
		item.Common[0] = *(short*)((char*)msg.param5 + 0xb);
		// D648 (983 00a97f88): caddie 0x181xxxxx guarda o fim do prazo em segundos (00b30830) em item+0x20 = os 8 bytes de
		// ItemDate.wHour..wMilliseconds
		if ((item.tid & 0xfc000000) == 0x18000000 && (item.tid & 0x1f00000) == 0x100000)
			S5::SysTimeToSec983((const SYSTEMTIME*)((char*)msg.param5 + 0xd), (__int64*)&item.ItemDate.wHour);
		ItemManager()->FindItem(item.tid);
		// 983 00a98003: caddie 0x181xxxxx ja na lista (mesmo guid) so troca tipo, prazo e data
		if ((item.tid & 0xfc000000) == 0x18000000 && (item.tid & 0x1f00000) == 0x100000)
		{
			std::list<sItemInfo>& items = CSharedDoc::Instance()->m_myItemList;
			std::list<sItemInfo>::iterator it = items.begin();
			for (; it != items.end() && it->guid != item.guid; ++it)
			{
			}
			if (it == items.end())
				items.push_back(item);
			else
			{
				it->ItemType = item.ItemType;
				it->HourRemain = item.HourRemain;
				memcpy(&it->ItemDate.wHour, &item.ItemDate.wHour, 8);
			}
			SetMyItemSlot();
			break;
		}
		bool bFound = false;
		if (item.ItemType != 0)
		{
			switch (item.ItemType)
			{
			case 3:
				hour *= 1;
				break;
			case 4:
				hour *= 24;
				item.HourRemain = hour;
				break;
			case 5:
				hour *= 720;
				item.HourRemain = hour;
				break;
			}
		}
		if (!item.ItemType)
		{
			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_myItemList.begin();
				it != CSharedDoc::Instance()->m_myItemList.end(); ++it)
			{
				if (it->tid == item.tid)
				{
					it->HourRemain += hour;
					// 983 00a980cb: bilhetes de gacha ficam com a contagem do Doc
					if (item.tid == 0x1a000083)
						it->Common[0] = (short)CSharedDoc::Instance()->m_gachaTicket[1];
					else if (item.tid == 0x1a000080)
						it->Common[0] = (short)CSharedDoc::Instance()->m_gachaTicket[0];
					else
						it->Common[0] = item.Common[0];
					bFound = true;
				}
			}
		}
		if (item.ItemType && hour > 0)
			Net_StartPeriodItem(item.guid);
		if (!bFound)
			CSharedDoc::Instance()->m_myItemList.push_back(item);
	}
	// fall through: 0xc5 is the tail of 0xb9
	case M983(0xc5):
		SetMyItemSlot();
		break;
	case M983(0xba):
	{
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = msg.param1;
		item.tid = msg.param2;
		CSharedDoc::Instance()->m_clubSetMap.insert(
			std::pair<const unsigned int, sItemInfo>(item.guid, item));
		break;
	}
	case M983(0xbb):
	{
		unsigned int guid = msg.param1;
		const char* pData = (const char*)msg.param5;
		unsigned long tid = msg.param2;
		IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(tid);
		if (!pBall)
			break;
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_ballList.begin();
		for (; it != CSharedDoc::Instance()->m_ballList.end(); ++it)
		{
			if (it->tid == tid)
			{
				if (*(const unsigned short*)(pData + 0xb) > 0)
					it->Common[0] = *(const unsigned short*)(pData + 0xb);
				else
					it->Common[0] += pBall->COM[0];
				break;
			}
		}
		if (it != CSharedDoc::Instance()->m_ballList.end())
			break;
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = guid;
		item.tid = tid;
		if (*(const unsigned short*)(pData + 0xb) > 0)
			item.Common[0] = *(const unsigned short*)(pData + 0xb);
		else
			item.Common[0] = pBall->COM[0];
		CSharedDoc::Instance()->m_ballList.push_back(item);
		break;
	}
	case M983(0x3):
		if (!m_n124)
			break;
		if (!((FrEdit*)m_n124)->IsVisible())
		{
			g_audio->PlaySfx("notice_dingdong", NULL, 0, NULL, NULL, 0.5f,
				200.0f);
			((FrEdit*)m_n124)->SetVisible(true);
		}
		m_f128 = 7.0f;
		((FrEdit*)m_n124)
			->AddLine((const char*)msg.param1,
				CChatMsg::Instance()->ConvertToUIColor(msg.param2), false);
		break;
	case M983(0xbc):
	{
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = msg.param1;
		item.tid = msg.param2;
		item.ItemType = msg.param4;
		item.HourRemain = msg.param3 * 24;
		CSharedDoc::Instance()->m_cutinList.push_back(item);
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_cutinList.begin();
		for (; it != CSharedDoc::Instance()->m_cutinList.end(); ++it)
		{
		}
		break;
	}
	case M983(0xbd):
	{
		IFF_STRUCT::sHairStyle* pHair =
			ItemManager()->FindHairStyle(msg.param2);
		if (!pHair)
			break;
		std::map<unsigned int, sCharacterInfo>::iterator cit =
			CSharedDoc::Instance()->m_charMap.begin();
		for (; cit != CSharedDoc::Instance()->m_charMap.end(); ++cit)
		{
			if ((cit->second.tid & 0x3ffffff) == pHair->cCharID)
			{
				cit->second.hairClr = pHair->cHairID;
				break;
			}
		}
		// 983 00a98a8b: com S4_MAILBOX_FOR_REAL_MYROOM o cabelo novo vai ao boneco do lobby (SetHair, 0x18c)
		if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << MsgObject(NULL, 0x18c, (int)&CSharedDoc::Instance()->GetMyPartTidList(),
					(int)&pHair->cHairID, 0, 0, 0);
		}
		break;
	}
	case M983(0xbe):
	{
		sRawItemInfo rawItem;
		memset(&rawItem, 0, sizeof(rawItem));
		sItemInfo& item = *(sItemInfo*)&rawItem;
		item.guid = msg.param1;
		item.tid = msg.param2;
		int hour = msg.param3;
		item.ItemType = msg.param4;
		item.HourRemain = hour;
		item.Common[0] = *(short*)((char*)msg.param5 + 0xb);
		ItemManager()->FindItem(item.tid);
		bool bFound = false;
		if (item.ItemType != 0)
		{
			switch (item.ItemType)
			{
			case 3:
				hour *= 1;
				break;
			case 4:
				hour *= 24;
				item.HourRemain = hour;
				break;
			case 5:
				hour *= 720;
				item.HourRemain = hour;
				break;
			}
		}
		if (item.ItemType == 0)
		{
			std::list<sItemInfo>::iterator it =
				CSharedDoc::Instance()->m_auxPartList.begin();
			for (; it != CSharedDoc::Instance()->m_auxPartList.end(); ++it)
			{
				if (it->tid == item.tid)
				{
					it->HourRemain += hour;
					it->Common[0] = item.Common[0];
					bFound = true;
				}
			}
		}
		if (item.ItemType != 0 && hour > 0)
			Net_StartPeriodItem(item.guid);
		if (!bFound)
			CSharedDoc::Instance()->m_auxPartList.push_back(item);
		break;
	}
	case M983(0xbf):
	{
		sMascotInfo info;
		memset(&info, 0, sizeof(info));
		info.guid = msg.param1;
		info.tid = msg.param2;
		strcpy(info.szMsg, "PANGYA!");
		const char* pSrc = (const char*)msg.param5;
		info.Remain_Date = *(unsigned short*)(pSrc + 8);
		memcpy(&info.endDate, pSrc + 0xd, 16);
		CSharedDoc::Instance()->AddMascot(&info);
		break;
	}
	case M983(0xc0):
	{
		// D621: 983 00a97249 caso 0xe2: 8 botoes {WRect, N, O, bool} de 0x1c com o bool em +0x300..+0x3c4
		// (Buy +0x31c, Gift +0x338) = m_rowBtn983[]; aba Rental (+0x1a4, m_status +0xf4) PRESSED -> Gift desligado.
		// O nosso usava RAW 645: 7 bools a partir de +0x3d4 (dentro de m_itemBmpMap/m_cutinImgMap) e +0x29c+0xf8.
		C_ASSERT_VC10(offsetof(CShopMain, m_rowBtn983) == 0x2e8 && sizeof(CShopMain::sRowBtn983) == 0x1c);
		C_ASSERT_VC10(offsetof(CShopMain, m_pSideTabRental) == 0x1a4);
		bool bOn = msg.param1 != 0;
		for (int i = 0; i < ROWBTN983_N; i++)
			m_rowBtn983[i].bOn = bOn;
		if (m_pSideTabRental && m_pSideTabRental->GetStatus() == FrButton::PRESSED)
			m_rowBtn983[ROWBTN983_GIFT].bOn = false;
		break;
	}
	case M983(0x241):
	{
		FrWnd* pNotify =
			g_pFresh->GetManager()->GetDesktop()->FindChildForm("notify");
		if (pNotify)
			((FrForm*)pNotify)->SetMessage((const char*)msg.param1, false);
		break;
	}
	case M983(0x218):
	{
		unsigned long tid = msg.param2;
		unsigned long uid = msg.param1;
		const char* pData = (const char*)msg.param5;
		unsigned int count = *(const unsigned short*)(pData + 0xb);
		if (*(const unsigned short*)(pData + 0xb) <= 0)
			count = 1;
		if (ItemManager()->FindCard(tid))
		{
			sCards card;
			memset(&card, 0, sizeof(card));
			card.uid = uid;
			card.tid = tid;
			card.cardCount = count;
			std::list<sCards>::iterator it =
				std::find_if(CSharedDoc::Instance()->m_cardList.begin(),
					CSharedDoc::Instance()->m_cardList.end(), _FindByUid(uid));
			if (it == CSharedDoc::Instance()->m_cardList.end())
				CSharedDoc::Instance()->m_cardList.push_back(card);
			else
				(*it).cardCount = count;
		}
		break;
	}
	case M983(0x217):
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			break;
		unsigned long id = msg.param1;
		unsigned long tid = msg.param2;
		IFF_STRUCT::sFurniture* pFur = ItemManager()->FindFurniture(tid);
		if (!pFur)
			break;
		sFurniture_List furniture;
		memset(&furniture, 0, sizeof(furniture));
		furniture.id = id;
		furniture.typeId = tid;
		furniture.x = pFur->Pos[0];
		furniture.y = pFur->Pos[1];
		furniture.z = pFur->Pos[2];
		furniture.r = pFur->Pos[3];
		furniture.bArrange = false;
		CSharedDoc::Instance()->AddFurniture(furniture);
		break;
	}
	case M983(0x24f):
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage((const char*)msg.param1, false);
		pForm->Open(NULL, 3);
		pForm->SetTopmost(true);
		pForm->FindNextTopFocus(false);
		BuildVisibleCaddieList();
		ChangeCaddie(msg.param2);
		SetLevelBar();
		break;
	}
	case 0x27b /*so do 983*/: // repassa ao ator principal a mensagem 2 sem parametros
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		break;
	case 0x27c /*so do 983*/: // so consome (00a99787); o caso do presente do 645 (M983(0x1ff) -> GiftProgress) nao existe no 983
		break;
	default:
		return false;
	}
	return true;
}

void CShopMain::OnAnotherHand(int key, void* param)
{
	if (key == 0)
		ShowPet(*(unsigned long*)param);
}

void CShopMain::OpenLayout(const char* name)
{
	WSendPacket packet1(0x5c);
	packet1.Send(TO_GAME);
	WSendPacket packet2(0x3d);
	packet2.Send(TO_GAME);
	// 983 (00a86990): terceiro pacote 0xfe com o dword em CSharedDoc+0x11498 (m_listVersion da lista de mercadorias da loja)
	unsigned long commodity = Doc()->m_sdCommodityList983.m_listVersion;
	WSendPacket packet3((enumClientPacket)0xfe);
	packet3.Encode4(commodity);
	packet3.Send(TO_GAME);
	g_pFresh->OpenLayout(name, this, false);
}

void CShopMain::SetPetCtrl(unsigned long tid)
{
	// 983 00a86a80
	WRect area = m_pPetArea->GetRect();
	CExhibition* pExhibition = NULL;
	bool bOk = false;
	switch (tid >> 26)
	{
	case 1:
	{
		if (m_subTab != 0xe && m_subTab != 0xf && m_subTab != 0x10)
		{
			// 983: roupa avulsa nao cria boneco na loja; manda 0x187 ao "LobbyAvatar" (&tid)
			m_pExhibition = NULL;
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << MsgObject(NULL, 0x187, (int)&tid, 0, 0, 0, 0);
			return;
		}
		// 983 00604750: uid = 1 (chave do CMotionAdvisor), o 0xffffffff padrao e dos outros bonecos
		pExhibition = new CExhibitionBySoftwareTnL(area.x + area.w * 0.5f,
			area.y + area.h * 0.5f, 0.35f, true, 1);
		pExhibition->SetArea(area.w, area.h);
		pExhibition->SetDistanceRange(16.0f, 20.0f);
		pExhibition->SetDistance(18.0f, true, false);
		pExhibition->SetControlFlag(0x1f);
		bOk = pExhibition->SetModel(CSharedDoc::Instance()->GetMyPartTidList(),
			CSharedDoc::Instance()->GetMyAuxPartTidList(), NULL, 3.0f, 0);
		break;
	}
	case 7:
	case 8:
		m_pExhibition = NULL;
		return;
	case 5:
		pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
			area.y + area.h * 0.5f, 0.0f, true);
		pExhibition->SetArea(area.w, area.h);
		pExhibition->SetDistance(1.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 4:
		pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
			area.y + area.h / 3.0f, 0.0f, true);
		pExhibition->SetArea(area.w, area.h);
		pExhibition->SetDistance(7.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 16:
	{
		pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
			area.y + area.h * 0.5f + 15.0f, 0.35f, true);
		pExhibition->SetArea(area.w, area.h);
		pExhibition->SetDistanceRange(4.0f, 8.0f);
		pExhibition->SetDistance(6.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
		// 983 00a86a80: texto passa pelo K2L_Compatibility (00b4ec60) antes
		CMascot::ChangeMascotMsgInTex(K2L_Compatibility("\xb8\xde\xbc\xbc\xc1\xf6"), pMascot,
			pExhibition->GetModel(), "default");
		break;
	}
	case 6:
		pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
			area.y + area.h * 0.5f, 0.0f, true);
		pExhibition->SetArea(area.w, area.h);
		pExhibition->SetDistance(6.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 31:
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
		if (!pCard)
			break;
		if ((tid & 0x3c00000) == 0x1000000)
		{
			pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
				area.y + area.h * 0.5f - 150.0f, 0.35f, true);
			pExhibition->SetArea(area.w, area.h);
			pExhibition->SetDistanceRange(4.0f, 8.0f);
			pExhibition->SetDistance(6.0f, true, false);
			bOk = pExhibition->SetModel(MakeStr("%s", pCard->Image),
				(eExhibitType)12);
		}
		else
		{
			pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
				area.y + area.h * 0.5f, 0.0f, true);
			pExhibition->SetArea(area.w, area.h);
			pExhibition->SetDistanceRange(4.0f, 8.0f);
			pExhibition->SetDistance(5.0f, true, false);
			bOk = pExhibition->SetModel(MakeStr("%s", pCard->Image),
				(eExhibitType)12);
		}
		pExhibition->SetMotion("default", true, 0.0f);
		break;
	}
	case 9:
	{
		unsigned int sub = tid >> 21 & 0x1f;
		if (sub == 6 || sub == 5 || sub == 4 || sub == 8)
		{
			pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
				area.y + area.h * 0.5f, 0.0f, true);
			pExhibition->SetArea(area.w, area.h);
			pExhibition->SetDistance(6.0f, true, false);
			if ((tid & 0x3e00000) == 0x800000 && (tid & 0x1fe000) == 0x1f8000)
			{
				IFF_STRUCT::sSetItem* pSet = ItemManager()->FindSetItem(tid);
				if (pSet)
				{
					IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(pSet->ElemIds[0]);
					if (pClub)
						tid = pClub->c.TypeId;
				}
			}
			bOk = pExhibition->SetModel(tid);
		}
		break;
	}
	case 18:
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			pExhibition = new CExhibitionByHardwareTnL(area.x + area.w * 0.5f,
				area.y + area.h * 0.5f, 0.0f, true);
			pExhibition->SetArea(area.w * 0.5f, area.h * 0.5f);
			pExhibition->SetDistance(15.0f, true, false);
			bOk = pExhibition->SetModel(tid);
		}
		break;
	default:
		return;
	}
	if (!bOk)
	{
		if (pExhibition)
			delete pExhibition;
	}
	else if (pExhibition) // 983 00a872fc
		m_pExhibition = pExhibition;
}

void CShopMain::ShowPet(unsigned long tid)
{
	if (tid)
	{
		// 983 00a87370: saida unica, ReleaseExhibition(false) chamado
		m_hand.Enter();
		if (m_pPetArea)
		{
			ReleaseExhibition(false);
			SetPetCtrl(tid);
		}
		m_hand.Leave();
	}
}

void CShopMain::HidePet(bool wait)
{
	if (wait)
		m_hand.WaitUntilWorkDone(9999);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}


inline WRect WRect::operator+(WPoint& point) const
{
	return WRect(x + point.x, y + point.y, w, h);
}

void CShopMain::LoadShopImages()
{
	m_pBmpMarkNew = REGBMP("mark_new");
	m_pBmpMarkHot = REGBMP("mark_hot");
	m_pBmpMarkLim = REGBMP("mark_lim");
	m_pBmpMarkLimited = REGBMP("mark_limitetag"); // 983 a84833
	for (int i = 0; i < 3; ++i)
		m_pBmp1fc[i] = REGBMP(MakeStr("mark_limite_type_%02d", i + 1));
	m_pBmpMarkSurprise = REGBMP("mark_surprise_sale");
	m_pBmpMarkBegin = REGBMP("mark_begin");
	Bitmap** pStat = &m_pStatBmp[0][0];
	const char** pName = &dot_files[0][0];
	do
	{
		int n = 2;
		do
		{
			*pStat = REGBMP(*pName);
			pName++;
			pStat++;
			n--;
		} while (n != 0);
	} while ((int)pName < (int)(&dot_files[0][0] + 10));
	m_pSelectBmp[0] = REGBMP("select_up");
	m_pSelectBmp[1] = REGBMP("select_mid");
	m_pSelectBmp[2] = REGBMP("select_low");
	m_pSelectBmp[3] = REGBMP("select_empty");
	m_pSelectBmp[4] = REGBMP("select_cursor1");
	m_pSelectBmp[5] = REGBMP("select_cursor0");
	m_pStatCharBmp[0] = REGBMP("stat_char_n");
	m_pStatCharBmp[1] = REGBMP("stat_char_o");
	m_pStatCharBmp[2] = REGBMP("stat_char_d");
	m_pStatCharBmp[3] = REGBMP("stat_club_n");
	m_pStatCharBmp[4] = REGBMP("stat_club_o");
	m_pStatCharBmp[5] = REGBMP("stat_club_d");
	m_pStatCharBmp[6] = REGBMP("stat_caddie_n");
	m_pStatCharBmp[7] = REGBMP("stat_caddie_o");
	m_pStatCharBmp[8] = REGBMP("stat_caddie_d");
	m_pStatAllBmp[0] = REGBMP("stat_all_none");
	m_pStatAllBmp[1] = REGBMP("stat_all_o");
	m_pStatAllBmp[2] = REGBMP("stat_all_d");
	int i = 0;
	Bitmap** pIcon = &m_pSiconBmp[0];
	do
	{
		i++;
		*pIcon = REGBMP(MakeStr("sicon%d", i));
		pIcon++;
	} while (i < 5);
	m_pBmpFa02 = REGBMP("it_fa02");
	m_pBmpCancelN = REGBMP("cancel_btn_n");
	m_pBmpCancelO = REGBMP("cancel_btn_o");
	m_rowBtn983[ROWBTN983_BUY].pBmpN = REGBMP("buy_btn_n");
	m_rowBtn983[ROWBTN983_BUY].pBmpO = REGBMP("buy_btn_o");
	float w = (float)m_rowBtn983[ROWBTN983_BUY].pBmpN->Width();
	float h = (float)m_rowBtn983[ROWBTN983_BUY].pBmpN->Height();
	m_rowBtn983[ROWBTN983_BUY].Rc() = WRect(144.0f, 66.0f, w, h);
	m_rowBtn983[ROWBTN983_BUY].bOn = true;
	m_rowBtn983[ROWBTN983_GIFT].pBmpN = REGBMP("gf_btn_n");
	m_rowBtn983[ROWBTN983_GIFT].pBmpO = REGBMP("gf_btn_o");
	w = (float)m_rowBtn983[ROWBTN983_GIFT].pBmpN->Width();
	h = (float)m_rowBtn983[ROWBTN983_GIFT].pBmpN->Height();
	m_rowBtn983[ROWBTN983_GIFT].Rc() = WRect(103.0f, 66.0f, w, h);
	m_rowBtn983[ROWBTN983_GIFT].bOn = true;
	m_rowBtn983[ROWBTN983_STATE].pBmpN = REGBMP("state_view_n");
	m_rowBtn983[ROWBTN983_STATE].pBmpO = REGBMP("state_view_o");
	m_rowBtn983[ROWBTN983_STATE].Rc() = WRect(103.0f, 35.0f, (float)m_rowBtn983[ROWBTN983_STATE].pBmpN->Width(),
		(float)m_rowBtn983[ROWBTN983_STATE].pBmpN->Height());
	m_pItemBase[0] = REGBMP("item_base_n");
	m_pItemBase[1] = REGBMP("item_base_o");
	m_pItemBase[2] = REGBMP("item_base_d");
	m_pItemBase[3] = REGBMP("item_base_empty");
	m_pItemBase[4] = REGBMP("item_base_award");
	m_pBmpMale = REGBMP("i_male");
	m_pBmpFemale = REGBMP("i_female");
	m_pBmpMasterSign = REGBMP("master_sign");
	m_pBmpTeamBase = REGBMP("team_base");
	m_pBmpSelArrow = REGBMP("selection_arrow");
	m_pBmpItemSlotN = REGBMP("item_slot_n");
	m_pBmpItemDesc1 = REGBMP("itemdesc_01");
	m_pBmpItemDesc2 = REGBMP("itemdesc_02");
	m_pBmpSelectMark = REGBMP("SelectItem_Mark"); // 983 00a84d86
}

#define IsSellableCaddie(caddie) \
	(((IFF_ITEM_COMMON&)(caddie).c).InStock != 0 && \
		((IFF_ITEM_COMMON&)(caddie).c).InStock != 5 && \
		((IFF_ITEM_COMMON&)(caddie).c).InStock != 6 && \
		CSharedDoc::Instance()->m_itemManager.IsInSalePeriod( \
			((IFF_ITEM_COMMON&)(caddie).c).TypeId, \
			CSharedDoc::Instance()->GetServerTime()))

bool CShopMain::HaveCharacter(unsigned long tid)
{
	std::map<unsigned int, sCharacterInfo>::iterator it;
	for (it = CSharedDoc::Instance()->m_charMap.begin();
		it != CSharedDoc::Instance()->m_charMap.end(); it++)
	{
		if (tid == (*it).second.tid)
			return true;
	}
	return false;
}

void CShopMain::BuildDisplayStand(subTab_t tab)
{
	if (m_mainTab == 1) // 983 00a923c0: SetVisibleSideTab (00a824f0) chamada, nao inline
	{
		SetVisibleSideTab(true);
		if (IsSelectedRentalTab())
		{
			BuildDisplayRental(tab);
			return;
		}
	}
	else
		SetVisibleSideTab(false);
	m_pItemList->ClearItem();
	switch (tab) // numeracao do 983 (00a923c0): 1 e 0xc sao abas novas (00a91d20 / 00a8f790)
	{
	case SUBTAB_SET:
		Build_Set(SUBTAB_SET, 0);
		break;
	case SUBTAB_PACKAGE:
		Build_Package(SUBTAB_PACKAGE, 0);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 0x1b:
		Build_Parts(tab, 0);
		break;
	case SUBTAB_HAIR:
		Build_Hair(SUBTAB_HAIR, 0);
		break;
	case SUBTAB_CLUBSET:
		Build_Clubset(SUBTAB_CLUBSET, 0);
		break;
	case SUBTAB_AZTEC:
		Build_Aztec(SUBTAB_AZTEC, 0);
		break;
	case SUBTAB_EQUIP:
		Build_Equip(SUBTAB_EQUIP, 0);
		break;
	case SUBTAB_NOEQUIP:
		Build_NoEquip(SUBTAB_NOEQUIP, 0);
		break;
	case SUBTAB_CADDIE:
		Build_Caddie(SUBTAB_CADDIE, 0);
		break;
	case 0xc:
		Build_CaddieItem(0xc, 0);
		break;
	case SUBTAB_MASCOT:
		Build_Mascot(SUBTAB_MASCOT, 0);
		break;
	case 0xe:
	case 0xf:
	case 0x10:
	case 0x11:
	case 0x23:
		Build_Decorate(tab, 0);
		break;
	case SUBTAB_CARD:
		Build_Card(SUBTAB_CARD, 0);
		break;
	case 0x1d:
	case 0x1e:
	case 0x1f:
		Build_Interior(tab, 0);
		break;
	case SUBTAB_FURNITURE:
		Build_Furniture(SUBTAB_FURNITURE, 0);
		break;
	case SUBTAB_CUTIN:
		Build_Cutin(SUBTAB_CUTIN, 0);
		break;
	}
	int n = m_pItemList->GetCurrentItemSize(true);
	if (n < 8)
	{
		int i = 8 - n;
		do
		{
			m_pItemList->AddItem(NULL);
			--i;
		} while (i != 0);
	}
}

void CShopMain::Net_StartPeriodItem(unsigned long tid)
{
	WSendPacket packet(0x20);
	packet.Encode1(7);
	packet.Encode4(tid);
	packet.Send(TO_GAME);
}

void CShopMain::SetLevelBar() // 00a8b410
{
	CStatusHelper helper;
	int i;
	for (i = 0; i < 5; ++i)
	{
		int cur = helper.GetStatus983(i);
		int max = helper.GetMyFreeSlot983(i) + cur;
		if (m_pStatEdit[i])
			m_pStatEdit[i]->SetCaption11(MakeStr("%d/%d", cur, max));
	}

	switch (m_subTab)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 0x1b:
		m_u55c = 0;
		break;
	case 7:
		m_u55c = 1;
		break;
	case 0xb:
	case 0xc:
		m_u55c = 2;
		break;
	case 0xd:
		break;
	default:
		return;
	}

	// 983 00a8b410: *(list+0x104) == 0 = m_pItemUnderCursor (nao o selecionado +0x108)
	bool bDraw = m_pItemList->GetItemUnderCursor() != NULL;
	sCharacterInfo* pChar = Doc()->FindChar983(0xffffffff);
	for (i = 0; i < 5; ++i)
	{
		const unsigned long* pAux = pChar ? Doc()->GetMyAuxPartTidList() : NULL;
		int cur = helper.GetStatusOf983(i, m_u524, m_pParts, pAux, m_clubsetTid,
			m_u528, m_mascotTid);
		int free = helper.GetFreeSlotOf983(i, m_pParts, m_u524,
			pChar ? (int)pChar->mastery : 0);
		int max = free + cur;
		if (!bDraw)
			cur = max = 0;
		if (m_pStatEdit[i])
		{
			const char* pOld = m_pStatEdit[i]->GetCaption();
			if (bDraw)
				m_pStatEdit[i]->SetCaption11(MakeStr("%s     %d/%d", pOld, cur, max));
			else
				m_pStatEdit[i]->SetCaption11(MakeStr("%s          ", pOld));
		}
	}
}

unsigned long __fastcall ShopCategoryFlag983(int cat)
{
	// 983 00b2e310: numeracao das sub-abas deslocada de +1 em relacao ao 645 (1 e 0xc novas)
	switch (cat)
	{
	case 1:
	case 0xe:
	case 0x17:
		return 0;
	case 2:
	case 0xf:
		return 1;
	case 3:
	case 0x10:
		return 2;
	case 4:
		return 0x10;
	case 5:
		return 0x2c;
	case 6:
		return 0x40;
	case 0x11:
		return 3;
	case 0x16:
		return 0x3bf;
	case 0x1b:
		return 0x380;
	case 0x1d:
		return 7;
	case 0x1e:
		return 9;
	case 0x1f:
		return 8;
	case 0x23:
		return 6;
	}
	return 0;
}

// 983: a cada item das listas de Build_*, 009d8a80 (nivel do jogador = CSharedDoc +0x91d) antes do InStock
static __forceinline bool IsLevelLocked983(IFF_ITEM_COMMON& item)
{
	unsigned long lv = CSharedDoc::Instance()->m_myInfo.stat.Level;
	return ItemManager()->IsLevelLocked(&item, &lv);
}

void CShopMain::Build_Package(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sChar>::iterator it;
	for (it = ItemManager()->m_CharMap.begin();
		it != ItemManager()->m_CharMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item))
		{
			CItemManager* pItemMgr = ItemManager();
			if ((item.InStock != 0 &&
					pItemMgr->IsInSalePeriod(item.TypeId,
						CSharedDoc::Instance()->GetServerTime())) ||
				item.DateFlag == 2)
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
}

void CShopMain::Build_Hair(subTab_t tab, int)
{
	std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator it;
	for (it = ItemManager()->m_HairStyleMap.begin();
		it != ItemManager()->m_HairStyleMap.end(); it++)
	{
		const IFF_STRUCT::sHairStyle& hair = (*it).second;
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)hair.c;
		if (!IsLevelLocked983(item) && (m_u524 & 0x3ffffff) == hair.cCharID)
		{
			CItemManager* pItemMgr = ItemManager();
			// 983 00a8e0fd: DateFlag == 2 entra mesmo sem InStock/fora da venda (como Build_Furniture)
			if ((item.InStock != 0 && pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime())) || item.DateFlag == 2)
			{
				FrListItem* pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
				if (IsItemInCart(pItem))
					m_pItemList->SelectItem(pItem, true);
			}
		}
	}
	m_pItemList->SortItem(SpecialHitNewCompare);
}

// 983 00a91d20: aba Set (Clothes > Packages): conjuntos do personagem atual (+0x2b0 = m_SetItemMap; C561: os tids
// 0x2420c0xx do Arin no SetItem.iff tem sub 1 e char 6 em >>13; o m_PartMap deixava a lista vazia)
void CShopMain::Build_Set(subTab_t, int)
{
	unsigned long charKey = m_u524;
	IFF_ITEM_COMMON& chr = (IFF_ITEM_COMMON&)ItemManager()->m_CharMap[charKey].c;
	if (chr.InStock != 0 &&
		ItemManager()->IsInSalePeriod(chr.TypeId, CSharedDoc::Instance()->GetServerTime()))
	{
		SetItemIterator it;
		for (it = ItemManager()->m_SetItemMap.begin();
			it != ItemManager()->m_SetItemMap.end(); it++)
		{
			IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
			if (IsLevelLocked983(item) || item.InStock == 0)
				continue;
			unsigned long sub = (item.TypeId >> 0x15) & 0x1f;
			if ((sub == 1 || sub == 2) &&
				((item.TypeId >> 0xd) & 0xff) == (m_u524 & 0x3ffffff) &&
				ItemManager()->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				FrListItem* pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Parts(subTab_t tab, int)
{
	unsigned long flag = GetCategoryFlag(tab);
	if (tab == 27)
	{
		std::list<sDisplayUccClothes>& clothes = UccManager()->m_displayClothes;
		std::list<sDisplayUccClothes>::iterator itUcc;
		for (itUcc = clothes.begin(); itUcc != clothes.end(); itUcc++)
		{
			sDisplayUccClothes* pClothes = &(*itUcc);
			if (((pClothes->partTypeId >> 18) & 0xff) == (m_u524 & 0x3ffffff))
				m_pItemList->AddItem(pClothes);
		}
	}

	PartIterator it;
	for (it = ItemManager()->m_PartMap.begin();
		it != ItemManager()->m_PartMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (*it).second.c;
		if (!S5::UnderCategory(m_u524, item.TypeId, flag))
			continue;
		CItemManager* pItemMgr = ItemManager();
		if (!((item.InStock != 0 && pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime())) ||
				item.DateFlag == 2))
			continue;
		if (IsAngelWing(item.TypeId) &&
			!(IsMannerPlayer() && CSharedDoc::Instance()->m_myInfo.stat.Level >= 21))
			continue;
		if (IsLevelLocked983(item))
			continue;
		FrListItem* pItem = m_pItemList->AddItem(&item);
		AddSellMiniButton(item, pItem);
		if (pItem && IsItemInCart(pItem))
			m_pItemList->SelectItem(pItem, true);
	}

	// 983: lista de mercadorias do Self Design (CSharedDoc+0x1147c), so com m_listVersion != 0
	sd_shop::CCommodityList& commodity = CSharedDoc::Instance()->m_sdCommodityList983;
	if (commodity.m_listVersion)
	{
		std::map<unsigned long, sd_shop::sCommodity>::iterator itCom;
		for (itCom = commodity.m_map.begin(); itCom != commodity.m_map.end(); itCom++)
		{
			void* pCom = &(*itCom).second;
			if (!S5::UnderCategory(m_u524, ((sd_shop::sCommodity*)pCom)->part.c.TypeId, flag))
				continue;
			FrListItem* pItem = m_pItemList->AddItem(pCom);
			if (pItem)
			{
				if (IsItemInCart(pItem))
					m_pItemList->SelectItem(pItem, true);
				pItem->noRBtnIdxList.push_back(1);
			}
		}
	}
	m_pItemList->SortItem(PartCompare);
}

void CShopMain::Build_Clubset(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sClubSet>::iterator it;
	for (it = ItemManager()->m_ClubSetMap.begin();
		it != ItemManager()->m_ClubSetMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item))
		{
			CItemManager* pItemMgr = ItemManager();
			// 983 00a8e569: DateFlag == 2 entra mesmo sem InStock/fora da venda
			if ((item.InStock != 0 && pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime())) || item.DateFlag == 2)
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}

	SetItemIterator itSet;
	for (itSet = ItemManager()->m_SetItemMap.begin();
		itSet != ItemManager()->m_SetItemMap.end(); itSet++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*itSet).second.c;
		if (!IsLevelLocked983(item) && item.InStock != 0 && (item.TypeId & 0x3e00000) == 0x800000)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Aztec(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sBall>::iterator it;
	for (it = ItemManager()->m_BallMap.begin();
		it != ItemManager()->m_BallMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item) && item.InStock != 0)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}

	SetItemIterator itSet;
	for (itSet = ItemManager()->m_SetItemMap.begin();
		itSet != ItemManager()->m_SetItemMap.end(); itSet++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*itSet).second.c;
		if (item.InStock != 0 && (item.TypeId & 0x3e00000) == 0xa00000)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Equip(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sItem>::iterator it;
	for (it = ItemManager()->m_ItemMap.begin();
		it != ItemManager()->m_ItemMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item) && item.InStock != 0 && (item.TypeId & 0x2000000) == 0)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}

	SetItemIterator itSet;
	for (itSet = ItemManager()->m_SetItemMap.begin();
		itSet != ItemManager()->m_SetItemMap.end(); itSet++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*itSet).second.c;
		if (item.InStock != 0 && (item.TypeId & 0x3e00000) == 0xc00000 &&
			(item.TypeId & 0x1fe000) == 0x1fc000)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_NoEquip(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sItem>::iterator it;
	// 983: item de tipo 6 com subtipo 0x1f so aparece com a flag 0x2804 de CSharedDoc+0x812; 24dfe00a nunca entra
	// (os AddNoRBtnIdx de InStock 1/2/3 do 645 viraram codigo morto no 983: al==1|3 e depois al==7; mantidos como no 983)
	for (it = ItemManager()->m_ItemMap.begin();
		it != ItemManager()->m_ItemMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!((item.TypeId & 0xfc000000) == 0x18000000 && (item.TypeId & 0x1f00000) == 0x1f00000 &&
				!(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 0x2804)) &&
			!IsLevelLocked983(item) && item.InStock != 0 && (item.TypeId & 0x2000000) != 0)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()) &&
				item.TypeId != 0x24dfe00a)
			{
				pItem = m_pItemList->AddItem(&item);
				if ((item.InStock == 1 || item.InStock == 3) && item.InStock == 7)
					pItem->AddNoRBtnIdx(1);
				if ((item.InStock == 1 || item.InStock == 2) && item.IsCash && item.InStock == 7)
					pItem->AddNoRBtnIdx(2);
			}
		}
	}

	SetItemIterator itSet;
	for (itSet = ItemManager()->m_SetItemMap.begin();
		itSet != ItemManager()->m_SetItemMap.end(); itSet++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*itSet).second.c;
		if (item.InStock != 0 && (item.TypeId & 0x3e00000) == 0xc00000 &&
			(item.TypeId & 0x1fe000) == 0x1fe000)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()) &&
				item.TypeId != 0x24dfe00a)
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Card(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sCard>::iterator it;
	for (it = ItemManager()->m_CardMap.begin();
		it != ItemManager()->m_CardMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item))
		{
			CItemManager* pItemMgr = ItemManager();
			// 983 00a8ee43: DateFlag == 2 entra mesmo sem InStock/fora da venda
			if ((item.InStock != 0 && (item.TypeId & 0x2000000) == 0 &&
					pItemMgr->IsInSalePeriod(item.TypeId,
						CSharedDoc::Instance()->GetServerTime())) || item.DateFlag == 2)
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}

	SetItemIterator itSet;
	for (itSet = ItemManager()->m_SetItemMap.begin();
		itSet != ItemManager()->m_SetItemMap.end(); itSet++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*itSet).second.c;
		if (item.InStock != 0 && (item.TypeId & 0x3e00000) == 0x1000000)
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime()))
			{
				pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Interior(subTab_t tab, int)
{
	unsigned long flag = GetCategoryFlag(tab);
	std::map<unsigned int, IFF_STRUCT::sFurniture>::iterator it;
	for (it = ItemManager()->m_FurnitureMap.begin();
		it != ItemManager()->m_FurnitureMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item))
		{
			CItemManager* pItemMgr = ItemManager();
			// 983 00a8f061: DateFlag == 2 entra mesmo sem InStock/fora da venda
			if (((item.InStock != 0 && (item.TypeId & 0x2000000) == 0 &&
					pItemMgr->IsInSalePeriod(item.TypeId,
						CSharedDoc::Instance()->GetServerTime())) || item.DateFlag == 2) &&
				((item.TypeId >> 11) & 0x7ff) == flag)
			{
				FrListItem* pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Furniture(subTab_t, int)
{
	std::map<unsigned int, IFF_STRUCT::sFurniture>::iterator it;
	for (it = ItemManager()->m_FurnitureMap.begin();
		it != ItemManager()->m_FurnitureMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (IsLevelLocked983(item))
			continue;
		// 983 00a8f170: item com DateFlag == 2 entra mesmo sem InStock/fora da venda
		{
			CItemManager* pItemMgr = ItemManager();
			if (!((item.InStock != 0 && (item.TypeId & 0x2000000) == 0 &&
					pItemMgr->IsInSalePeriod(item.TypeId,
						CSharedDoc::Instance()->GetServerTime())) ||
				item.DateFlag == 2))
				continue;
			int sub = (item.TypeId >> 11) & 0x7ff;
			if ((sub >= 0 && sub < 7) || sub >= 10)
			{
				FrListItem* pItem = m_pItemList->AddItem(&item);
				if (pItem)
				{
					if (item.InStock == 1 || item.InStock == 3)
						pItem->noRBtnIdxList.push_back(1);
					if (item.InStock == 1 || item.InStock == 2)
						pItem->noRBtnIdxList.push_back(2);
				}
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Decorate(subTab_t tab, int)
{
	unsigned long flag = GetCategoryFlag(tab);
	std::map<unsigned int, IFF_STRUCT::sSkin>::iterator it;
	for (it = ItemManager()->m_SkinMap.begin();
		it != ItemManager()->m_SkinMap.end(); it++)
	{
		if (!IsLevelLocked983((IFF_ITEM_COMMON&)(*it).second.c) &&
			(flag == 5 || (((*it).second.c.TypeId >> 22) & 0xf) == flag) &&
			(*it).second.c.InStock != 0)
		{
			FrListItem* pItem = m_pItemList->AddItem((void*)&(*it).second.c);
			AddSellMiniButton((IFF_ITEM_COMMON&)(*it).second.c, pItem);
			if (IsItemInCart(pItem))
			{
				m_pItemList->SelectItem(pItem, true);
				if (((*it).second.c.TypeId & 0x3c00000) == 0 && m_skinTid[0])
				{
					m_u724 = (*it).second.HScroll;
					m_u728 = (*it).second.VScroll;
					m_f71c = 0;
					m_f720 = 0;
				}
			}
		}
	}
	m_pItemList->SortItem(SkinCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Cutin(subTab_t, int)
{
	std::map<unsigned int, IFF_STRUCT::sSkin>::iterator it;
	for (it = ItemManager()->m_SkinMap.begin();
		it != ItemManager()->m_SkinMap.end(); it++)
	{
		if (S5::ISCutinItem((*it).second.c.TypeId) &&
			(*it).second.c.InStock != 0)
		{
			IFF_STRUCT::S5::sCutinInformation* pInfo =
				ItemManager()->FindCutinInformation((*it).second.c.TypeId);
			if (pInfo &&
				!IsLevelLocked983((IFF_ITEM_COMMON&)(*it).second.c) &&
				((m_u524 & 0x3ffffff) == (unsigned int)pInfo->CharIndex ||
					pInfo->CharIndex >= 100))
			{
				FrListItem* pItem =
					m_pItemList->AddItem((void*)&(*it).second.c);
				AddSellMiniButton((IFF_ITEM_COMMON&)(*it).second.c, pItem);
				if (IsItemInCart(pItem))
					m_pItemList->SelectItem(pItem, true);
			}
		}
	}
	m_pItemList->SortItem(SkinCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Caddie(subTab_t, int)
{
	FrListItem* pItem;
	std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator it;
	for (it = m_caddieIffMap.begin(); it != m_caddieIffMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		CItemManager* pItemMgr = ItemManager();
		if (pItemMgr->IsSellableItem(&item,
				CSharedDoc::Instance()->GetServerTime()))
		{
			pItem = m_pItemList->AddItem(&item);
			AddSellMiniButton(item, pItem);
		}
	}
	m_pItemList->SortItem(CaddieCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

// 983 00a8f790: aba 0xc (roupas do caddie): CadItem.iff do caddie atual (m_u528) a venda, ou item de periodo
void CShopMain::Build_CaddieItem(int, int)
{
	std::map<unsigned int, IFF_STRUCT::sCadItem>::iterator it;
	for (it = ItemManager()->m_CadItemMap.begin();
		it != ItemManager()->m_CadItemMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		// 983 00a8f790: item com DateFlag == 2 testado so depois do caso normal
		if ((item.InStock != 0 &&
				(((item.TypeId >> 21) & 0x1f) + ((item.TypeId >> 16) & 0x1f)) ==
					(m_u528 & 0x3ffffff) &&
				ItemManager()->IsInSalePeriod(item.TypeId,
					CSharedDoc::Instance()->GetServerTime())) ||
			item.DateFlag == 2)
		{
			FrListItem* pItem = m_pItemList->AddItem(&item);
			AddSellMiniButton(item, pItem);
		}
	}
	m_pItemList->SortItem(CaddieCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

void CShopMain::Build_Mascot(subTab_t, int)
{
	std::map<unsigned int, IFF_STRUCT::sMascot>::iterator it;
	for (it = ItemManager()->m_MascotMap.begin();
		it != ItemManager()->m_MascotMap.end(); it++)
	{
		IFF_ITEM_COMMON& item = (IFF_ITEM_COMMON&)(*it).second.c;
		if (!IsLevelLocked983(item))
		{
			CItemManager* pItemMgr = ItemManager();
			if (pItemMgr->IsSellableItem(&item,
					CSharedDoc::Instance()->GetServerTime()))
			{
				FrListItem* pItem = m_pItemList->AddItem(&item);
				AddSellMiniButton(item, pItem);
			}
		}
	}
	m_pItemList->SortItem(ShopEmptyCompare);
	m_pItemList->SortItem(SpecialHitNewCompare);
}

bool CShopMain::IsItemInCart(FrListItem* pItem)
{
	// 983 00a88c00: o item procurado fica em local antes dos lacos
	IFF_ITEM_COMMON* pFind = (IFF_ITEM_COMMON*)pItem->pData;
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		IFF_ITEM_COMMON* pCart = (IFF_ITEM_COMMON*)(*it)->pData;
		if (pCart && pFind->TypeId == pCart->TypeId)
			return true;
	}
	for (it = m_pAuxPartCartList->m_itemList.begin();
		it != m_pAuxPartCartList->m_itemList.end(); ++it)
	{
		IFF_ITEM_COMMON* pCart = (IFF_ITEM_COMMON*)(*it)->pData;
		if (pCart && pFind->TypeId == pCart->TypeId)
			return true;
	}
	return false;
}

void CShopMain::RestoreShoppersOldItem(unsigned long tid, CPartTidList& tids)
{
	std::list<oldItem_t>::iterator it = m_oldItems.begin();
	while (it != m_oldItems.end())
	{
		if ((*it).tid == tid)
		{
			// 983 00a8a390: mensagem 0x18a ao "LobbyAvatar" (lista, &oldTid, &oldUid)
			MsgObject msg(NULL, 0x18a, (int)&tids, (int)&(*it).oldTid,
				(int)&(*it).oldUid, 0, 0);
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << msg;
			it = m_oldItems.erase(it);
		}
		else
			++it;
	}
}

void CShopMain::ReleaseExhibition(bool bWait)
{
	if (bWait)
		m_hand.WaitUntilWorkDone(9999);
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
}

void CShopMain::ChangeSubTab(subTab_t tab, bool bForce)
{
	if (!bForce && m_subTab == tab)
		return;

	int old = m_subTab;
	bool bLoadPet = false;
	if (((old <= 6 || old == 0x11) && (tab == 0xe || tab == 0xf || tab == 0x10)) ||
		(old == 0x22 && (tab == 0xe || tab == 0xf || tab == 0x10)) ||
		((old == 0xe || old == 0xf || old == 0x10 || old == 0x22) &&
			m_mainTab == 0) ||
		(m_mainTab == 1 && old == 0x11 && tab == 1))
		bLoadPet = true;

	m_subTab = tab;
	m_pItemList->SetMultiSelect(tab == 5 || tab == 0x1b);
	BuildDisplayStand(m_subTab);
	m_pTabSub->Select(m_subTab);

	switch (m_subTab)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 0x1b:
		TurnOnItemStatus();
		if (bLoadPet)
			SafeLoadPet(&m_u524, bLoadPet);
		break;
	case 5:
		TurnOnItemStatus();
		if (bLoadPet)
			SafeLoadPet(&m_u524, bLoadPet);
		break;
	case 7:
		TurnOnItemStatus();
		SafeLoadPet(&m_clubsetTid, false);
		break;
	case 8:
		TurnOffItemStatus();
		SafeLoadPet(&m_aztecTid, false);
		break;
	case 0x1c:
		if (IsLocalContent((localContentType_t)0x52))
		{
			TurnOffItemStatus();
			ReleaseExhibition(true);
		}
		break;
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
		if (IsLocalContent((localContentType_t)0x57) &&
			IsLocalContent((localContentType_t)0x62))
		{
			TurnOffItemStatus();
			ReleaseExhibition(true);
		}
		break;
	case 9:
	case 0xa:
		TurnOffItemStatus();
		ReleaseExhibition(true);
		break;
	case 0xb:
		TurnOnItemStatus();
		ReleaseExhibition(true);
		break;
	case 0xd:
		TurnOffItemStatus();
		break;
	// 983 00a92590: tres cases com o mesmo corpo, separados como no 983
	case 0xe:
	case 0x11:
		TurnOffItemStatus();
		SafeLoadPet(&m_u524, bLoadPet);
		break;
	case 0xf:
	case 0x23:
		TurnOffItemStatus();
		SafeLoadPet(&m_u524, bLoadPet);
		break;
	case 0x10:
	case 0x22:
		TurnOffItemStatus();
		SafeLoadPet(&m_u524, bLoadPet);
		break;
	}

	SetMyItemSlot();
	SetItemDescRect();
	SetPetAreaRect();
	SetAutoScroll();
}

void CShopMain::SetPetAreaRect()
{
	switch ((int)m_subTab)
	{
	case 0xe:
	case 0xf:
	case 0x10:
		m_pPetArea = m_pSkinPet;
		break;
	case 0x11:
	case 0x22:
	case 0x23:
		m_pPetArea = m_pSkinPet2;
		break;
	default:
		m_pPetArea = m_pPet;
		break;
	}
}

void CShopMain::SetMyItemSlot()
{
	switch (m_subTab)
	{
	case 9:
		BuildMyItemSlot(false);
		break;
	case 0xa:
		BuildMyItemSlot(true);
		break;
	default:
		m_pItemSlotFrame1->SetVisible(false);
		m_pItemSlotFrame2->SetVisible(false);
		m_pItemSlot->SetVisible(false);
		return;
	}
	m_pItemSlotFrame1->SetVisible(true);
	m_pItemSlotFrame2->SetVisible(true);
	m_pItemSlot->SetVisible(true);
}

void CShopMain::SetAutoScroll()
{
	// 983 00a88d00: a lista so fica em local na contagem; depois le m_pItemList de novo
	FrListBox* pList = m_pItemList;
	FrListItem* pSel = pList->GetSelected();
	int row = 0;
	if (pSel)
	{
		FrListBox::ITEM_LIST::iterator it = pList->m_itemList.begin();
		FrListBox::ITEM_LIST::iterator itEnd = pList->m_itemList.end();
		for (; it != itEnd && *it != pSel; ++it)
			++row;
	}
	m_pItemList->ResetTopRow();
	m_pItemList->GetScrollBar()->SetCurTopRow(row / 2);
}

// 983 00a88d60 (0x2d7, D640): rola a lista ate o item do tid (sem achar = linha do fim)
void CShopMain::ScrollToTid983(unsigned long tid)
{
	if (tid == 0)
		return;
	int row = 0;
	FrListBox::ITEM_LIST::iterator it = m_pItemList->m_itemList.begin();
	for (; it != m_pItemList->m_itemList.end(); ++it, ++row)
	{
		IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
		if (pData && pData->TypeId == tid)
			break;
	}
	m_pItemList->ResetTopRow();
	m_pItemList->GetScrollBar()->SetCurTopRow(row / 2);
}

// 983: o envio da mensagem ao quadro do lobby esta inline em cada ramo da aba (ChangeMainTab)
static __forceinline void SendLobbyCmd983(const char* name, unsigned long cmd)
{
	IActor* pActor = AfxGetTask()->GetActor(name);
	if (pActor)
		pActor << MsgObject(NULL, cmd, 1, 0, 0, 0, 0);
}

void CShopMain::ChangeMainTab(mainTab_t tab, bool bSelect) // 00a8fa30
{
	if (bSelect)
		m_pTabMain->Select(m_mainTab);
	else if (m_mainTab == tab)
		return;

	m_mainTab = tab;
	// A693: o 983 (00a8fa30..00a900fc) nao toca [this+0x6b0] ao trocar de aba; o rect so zera na 0x5a fora da aba 1
	if (tab != 1)
	{
		if (!m_pSideTabBuy)
			return;
		if (!m_pSideTabRental)
			return;
		if (m_pSideTabBuy->GetStatus() != FrButton::PRESSED)
		{
			m_pSideTabBuy->SetStatus(FrButton::PRESSED);
			m_pSideTabRental->SetStatus(FrButton::NORMAL);
			m_rowBtn983[ROWBTN983_GIFT].bOn = true;
		}
	}

	m_pTabSub->ClearTab();
	const char* names[3] = { NULL, NULL, NULL };
	switch (m_mainTab)
	{
	case 0:
		m_pBuyAll->SetVisible(false);
		m_pBuyReset->SetVisible(false);
		SetSubTabImg(names, 0xc);
		m_pTabSub->AddTab(names, 0);
		break;
	case 1:
	{
		m_pBuyAll->SetVisible(true);
		m_pBuyAll->Enable(false);
		m_pBuyReset->SetVisible(true);
		m_pBuyReset->Enable(false);
		for (unsigned long i = 1; (int)i <= 6; ++i)
		{
			SetSubTabImg(names, i);
			m_pTabSub->AddTab(names, i);
		}
		SetSubTabImg(names, 0x1b);
		m_pTabSub->AddTab(names, 0x1b);
		break;
	}
	case 2:
	{
		m_pBuyAll->SetVisible(false);
		m_pBuyReset->SetVisible(false);
		for (unsigned long i = 7; (int)i <= 0xa; ++i)
		{
			SetSubTabImg(names, i);
			m_pTabSub->AddTab(names, i);
		}
		if (IsLocalContent((localContentType_t)0x52))
		{
			SetSubTabImg(names, 0x1c);
			m_pTabSub->AddTab(names, 0x1c);
		}
		break;
	}
	case 3:
		m_pBuyAll->SetVisible(false);
		m_pBuyReset->SetVisible(false);
		SetSubTabImg(names, 0xb);
		m_pTabSub->AddTab(names, 0xb);
		SetSubTabImg(names, 0x27);
		m_pTabSub->AddTab(names, 0xc);
		break;
	case 4:
		m_pBuyAll->SetVisible(false);
		m_pBuyReset->SetVisible(false);
		SetSubTabImg(names, 0xd);
		m_pTabSub->AddTab(names, 0xd);
		break;
	case 5:
	{
		m_pBuyAll->SetVisible(true);
		m_pBuyReset->SetVisible(true);
		for (unsigned long i = 0xe; (int)i <= 0x11; ++i)
		{
			SetSubTabImg(names, i);
			m_pTabSub->AddTab(names, i);
		}
		SetSubTabImg(names, 0x22);
		m_pTabSub->AddTab(names, 0x22);
		SetSubTabImg(names, 0x23);
		m_pTabSub->AddTab(names, 0x23);
		break;
	}
	}

	m_pItemDesc->SetVisible(false);
	m_pItemDescBg->SetVisible(false);

	if (CLobbyState983a::ms_pInstance)
	{
		// 983: modo do lobby (005fbe70) e mensagens 0x1ce/0x1cf a "LobbyAvatar"/"LobbyCaddie" conforme a aba
		// (a8fe82: abas 0/1 = 2, aba 3 (Caddie) = 3 = CaddieCamera, demais (Item/Mascot/Decoration) = 4; reconferido C172.1)
		unsigned long cmdAvatar = 0x1cf, cmdCaddie = 0x1cf;
		int mode = 3;
		if (m_mainTab == 0 || m_mainTab == 1)
		{
			mode = 2;
			cmdAvatar = 0x1ce;
		}
		else if (m_mainTab == 3)
			cmdCaddie = 0x1ce;
		else
			mode = 4;
		CLobbyState983a::ms_pInstance->SetShopMode983(mode);
		SendLobbyCmd983("LobbyAvatar", cmdAvatar);
		SendLobbyCmd983("LobbyCaddie", cmdCaddie);
		CLobbyState983a::ms_pInstance->EndShopMode983();
	}
	m_prevMainTab = m_mainTab;
	TurnOffItemStatus();
	m_pScrollUp->SetVisible(false);
	m_pScrollDown->SetVisible(false);
	ClearCartList(false, NULL);
}

void CShopMain::ChangeStatTab(statTab_t tab)
{
	m_statTab = tab;
}

void CShopMain::ChangeCharacter(unsigned long tid) // 00a92810
{
	std::map<unsigned int, sCharacterInfo>::iterator it;
	for (it = CSharedDoc::Instance()->m_charMap.begin();
		it != CSharedDoc::Instance()->m_charMap.end() && it->second.tid != tid;
		++it)
	{
	}
	m_pBuyAll->Enable(false);
	m_pBuyReset->Enable(false);
	if (m_u524 != tid)
	{
		m_u524 = tid;
		m_pPartCartList->ClearItem();
		m_pAuxPartCartList->ClearItem();
	}

	// 983: subtabs de roupa testados um a um; o ramo de fora (sub 0 / skin) vem primeiro
	int sub = m_subTab;
	if (sub != 1 && sub != 2 && sub != 3 && sub != 4 && sub != 5 && sub != 6 && sub != 0x1b)
	{
		if (sub == 0)
		{
			if (it != CSharedDoc::Instance()->m_charMap.end())
			{
				CSharedDoc::Instance()->BuildMyPartTidList(
					(sCharacterInfo&)it->second);
				m_pParts = (unsigned long*)it->second.tidParts;
				m_pCharPCL = (char*)it->second.PCL;
				m_pAuxParts = (unsigned long*)it->second.tidAuxParts;
				m_b520 = it->second.hairClr;
			}
			else
			{
				CSharedDoc::Instance()->BuildMyPartTidListDefault(tid);
				m_b520 = 0;
			}
			ChangeSubTab((subTab_t)0, false);
			m_pTabSub->Select(m_subTab);
		}
		else
		{
			if (m_mainTab == 5)
			{
				ChangeSkin(m_u524);
				CheckAuxCartScroll(); // 983 00a840b0
				CSharedDoc::Instance()->BuildMyPartTidList((sCharacterInfo&)it->second);
				BuildDisplayStand(m_subTab);
				SafeLoadPet(&m_u524, false);
			}
			SetAutoScroll();
			return;
		}
	}
	else
	{
		bool bWinner = false;
		if (sub == 0x1b && tid == UccManager()->GetWinnerCharacterTypeId())
			bWinner = IS_EXACTKINDOF(CLobbyTask, AfxGetTask());
		if (it != CSharedDoc::Instance()->m_charMap.end())
		{
			CSharedDoc::Instance()->BuildMyPartTidList(
				(sCharacterInfo&)it->second);
			m_pParts = (unsigned long*)it->second.tidParts;
			m_pCharPCL = (char*)it->second.PCL;
			m_pAuxParts = (unsigned long*)it->second.tidAuxParts;
			m_b520 = it->second.hairClr;
			if (m_subTab == 1)
			{
				ChangeSubTab((subTab_t)2, false);
				m_pTabSub->Select(m_subTab);
			}
		}
		else
		{
			CSharedDoc::Instance()->BuildMyPartTidListDefault(tid);
			if (!bWinner)
				ChangeSubTab((subTab_t)1, false);
			m_pTabSub->Select(m_subTab);
			m_b520 = 0;
		}
	}
	ClearCartList(true, &CSharedDoc::Instance()->GetMyPartTidList());
	BuildDisplayStand(m_subTab);
	SafeLoadPet(&m_u524, m_u524 == tid);
	TurnOnItemStatus();
	SetAutoScroll();
}

// 983 00a92b20
void CShopMain::ChangeCaddie(unsigned long tid)
{
	m_u528 = tid;
	if (m_subTab == SUBTAB_CADDIE)
	{
		BuildDisplayStand(SUBTAB_CADDIE);
		SafeLoadPet(&m_u528, false);
		TurnOnItemStatus();
	}
}

// 983 00a92b60
void CShopMain::ChangeMascot(unsigned long tid)
{
	m_mascotTid = tid;
	if (m_subTab == SUBTAB_MASCOT)
	{
		BuildDisplayStand(SUBTAB_MASCOT);
		SafeLoadPet(&m_mascotTid, false);
		TurnOffItemStatus();
	}
}

void CShopMain::ChangeSkin(unsigned long tid)
{
	m_u524 = tid;
	SafeLoadPet(&m_u524, false);
	memcpy(m_skinTid, CSharedDoc::Instance()->m_myInfo.userEquip.tidSkin,
		sizeof(m_skinTid));
	unsigned long* pTid = m_skinTid;
	for (int i = 0; i < 6; ++i, ++pTid)
	{
		if (*pTid)
		{
			CItemManager* pItemMgr = ItemManager();
			IFF_STRUCT::sSkin* pSkin = pItemMgr->FindSkin(*pTid);
			if (pSkin && (*pTid & 0x3c00000) == 0)
			{
				m_u724 = pSkin->HScroll;
				m_u728 = pSkin->VScroll;
				m_f71c = 0;
				m_f720 = 0;
			}
		}
	}
}

// 983 00a92ba0
void CShopMain::ChangeClubset(unsigned long tid)
{
	m_clubsetTid = tid;
	if (m_subTab == SUBTAB_CLUBSET)
	{
		BuildDisplayStand(SUBTAB_CLUBSET);
		SafeLoadPet(&m_clubsetTid, false);
		TurnOnItemStatus();
	}
}

// 983 00a92be0
void CShopMain::ChangeAztec(unsigned long tid)
{
	m_aztecTid = tid;
	if (m_subTab == SUBTAB_AZTEC)
	{
		BuildDisplayStand(SUBTAB_AZTEC);
		SafeLoadPet(&m_aztecTid, false);
		TurnOffItemStatus();
	}
}

void CShopMain::TurnOnItemSelect()
{
	m_pScrollUp->SetVisible(true);
	m_pScrollDown->SetVisible(true);
}

void CShopMain::TurnOffItemSelect()
{
	m_pScrollUp->SetVisible(false);
	m_pScrollDown->SetVisible(false);
}

const Bitmap* CShopMain::GetItemIcon(unsigned long tid)
{
	// 983 00a90120: sem iniciar pBmp; insere e devolve dentro do if, como o 983
	std::map<unsigned long, const Bitmap*>::iterator it =
		m_itemBmpMap.find(tid);
	if (it == m_itemBmpMap.end())
	{
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
		if (pItem)
		{
			const Bitmap* pBmp = g_pFresh->RegisterBitmap(pItem->Icon);
			if (pBmp)
			{
				m_itemBmpMap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(
						pItem->TypeId, pBmp));
				return pBmp;
			}
		}
	}
	else
		return (*it).second;
	return NULL;
}


void CShopMain::DrawSelArrow(FrGraphicInterface* gfx)
{
}


void CShopMain::SafeLoadPet(unsigned long* pTid, bool bForce)
{
	m_hand.WaitUntilWorkDone(9999);
	if (bForce || !m_pExhibition || m_pExhibition->GetTypeId() != *pTid)
		m_hand.Work(this, 0, pTid, true, -1);
}

// 983 00a8c080: so repassa para SetLevelBar (o modelo e atualizado pela mao/exhibition)
void CShopMain::ShowMainModel(CPartTidList& tids)
{
	SetLevelBar();
}

void CShopMain::LoadCardImages()
{
	std::map<unsigned int, IFF_STRUCT::sCard>::iterator it;
	for (it = ItemManager()->m_CardMap.begin();
		it != ItemManager()->m_CardMap.end(); it++)
	{
		const IFF_STRUCT::sCard& card = (*it).second;
		if (m_cardImgMap.find(card.c.TypeId) == m_cardImgMap.end())
		{
			char name[64];
			sprintf(name, "%s.png", card.Image);
			WOverlay* pImg = g_resrcmng->GetOverlay(name, 0x80000000);
			if (pImg)
				m_cardImgMap.insert(
					std::map<unsigned int, WOverlay*>::value_type(card.c.TypeId,
						pImg));
		}
	}
}

void CShopMain::DeleteCardImages()
{
	for (std::map<unsigned int, WOverlay*>::iterator it = m_cardImgMap.begin();
		it != m_cardImgMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
}

WOverlay* CShopMain::GetCardImg(unsigned long tid)
{
	// 983 00a90320: ramo do nao-achado primeiro, como o 983
	std::map<unsigned int, WOverlay*>::iterator it = m_cardImgMap.find(tid);
	if (it == m_cardImgMap.end())
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
		if (pCard)
		{
			char name[64];
			sprintf(name, "%s.png", pCard->Image);
			WOverlay* pImg = g_resrcmng->GetOverlay(name, 0);
			if (pImg)
				m_cardImgMap.insert(
					std::map<unsigned int, WOverlay*>::value_type(
						pCard->c.TypeId, pImg));
			return pImg;
		}
	}
	else
		return (*it).second;
	return NULL;
}

void CShopMain::DrawCard()
{
	if (m_u72c)
	{
		IFF_STRUCT::sCard* pCard;
		unsigned long tid;
		if (m_u72c >> 26 == 0x1f)
		{
			CItemManager* pItemMgr = ItemManager();
			pCard = pItemMgr->FindCard(m_u72c);
			tid = m_u72c;
		}
		else
		{
			if (m_u72c >> 26 != 9 || (m_u72c & 0x1fe000) != 0x1fa000)
				return;
			CItemManager* pItemMgr = ItemManager();
			IFF_STRUCT::sSetItem* pSet = pItemMgr->FindSetItem(m_u72c);
			if (!pSet)
				return;
			tid = pSet->ElemIds[0];
			pItemMgr = ItemManager();
			pCard = pItemMgr->FindCard(tid);
		}
		if (pCard && tid)
		{
			WOverlay* pImg = GetCardImg(tid);
			if (pImg)
				pImg->Render(g_view, WRect(0, 0, 1.0f, 1.0f),
					WRect(92.0f, 117.0f, 248.0f, 499.0f), 0, 0xffffffff, 0, 0);
		}
	}
}

void CShopMain::InitalizeUpgradeCaddieInfo()
{
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000001UL, 0x1c000010UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000002UL, 0x1c000011UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000003UL, 0x1c000012UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000004UL, 0x1c000013UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000005UL, 0x1c000014UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000006UL, 0x1c000015UL));
	m_upgradeMap.insert(std::map<unsigned long, unsigned long>::value_type(
		0x1c000007UL, 0x1c000016UL));
}

void CShopMain::BuildVisibleCaddieList()
{
	m_caddieIffMap.clear();

	std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator it;
	for (it = ItemManager()->m_CaddieMap.begin();
		it != ItemManager()->m_CaddieMap.end(); it++)
	{
		IFF_STRUCT::sCaddie* pCaddie = &(*it).second;
		unsigned long baseTid = 0;
		unsigned long upgTid = 0;
		bool bOwnUpgraded = false;

		std::map<unsigned long, unsigned long>::iterator itUp;
		for (itUp = m_upgradeMap.begin(); itUp != m_upgradeMap.end(); itUp++)
		{
			baseTid = (*itUp).first;
			upgTid = (*itUp).second;
			if (pCaddie->c.TypeId == baseTid || pCaddie->c.TypeId == upgTid)
				break;
		}

		bool bAdd = false;
		if (itUp == m_upgradeMap.end())
		{
			bAdd = true;
		}
		else
		{
			std::map<unsigned int, sCaddieInfo>::const_iterator itOwn =
				CSharedDoc::Instance()->m_caddieMap.begin();
			if (pCaddie->c.TypeId == baseTid)
			{
				for (; itOwn != CSharedDoc::Instance()->m_caddieMap.end();
					++itOwn)
				{
					if ((*itOwn).second.tid == baseTid)
					{
						bAdd = true;
						break;
					}
				}
			}
			else if (pCaddie->c.TypeId == upgTid)
			{
				if (itOwn != CSharedDoc::Instance()->m_caddieMap.end())
				{
					for (; itOwn != CSharedDoc::Instance()->m_caddieMap.end();
						++itOwn)
					{
						if ((*itOwn).second.tid == baseTid)
							break;
						if ((*itOwn).second.tid == upgTid)
						{
							bOwnUpgraded = true;
							break;
						}
					}
					if (itOwn == CSharedDoc::Instance()->m_caddieMap.end() ||
						bOwnUpgraded)
						bAdd = true;
				}
				else
				{
					bAdd = true;
				}
			}
		}

		if (bAdd)
			m_caddieIffMap.insert(
				std::map<unsigned int, IFF_STRUCT::sCaddie>::value_type(
					(*it).first, *pCaddie));
	}
}

void CShopMain::UpgradeCaddie(unsigned long tid)
{
	unsigned long newTid = 0;
	std::map<unsigned long, unsigned long>::iterator itUp =
		m_upgradeMap.find(tid);
	if (itUp != m_upgradeMap.end())
	{
		newTid = (*itUp).second;
	}
	else
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc4\xb3\xb5\xf0\xb4\xc2 \xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xb0\xa1 "
				 "\xba\xd2\xb0\xa1\xb4\xc9 \xc7\xd5\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0));
	}
	unsigned int oldPrice = 0;
	unsigned int newPrice = 0;
	std::map<unsigned int, IFF_STRUCT::sCaddie>& caddies =
		ItemManager()->m_CaddieMap;
	std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator itOld =
		caddies.find(tid);
	std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator itNew =
		caddies.find(newTid);
	sAttr5 oldAttr;
	sAttr5 newAttr;
	if (itOld != caddies.end())
	{
		oldAttr = *(sAttr5*)(*itOld).second.Attr;
		oldPrice = (*itOld).second.MonthlyFee;
	}
	if (itNew != caddies.end())
	{
		newAttr = *(sAttr5*)(*itNew).second.Attr;
		newPrice = (*itNew).second.MonthlyFee;
	}
	int stat[5];
	stat[0] = newAttr.v[0] - oldAttr.v[0];
	stat[1] = newAttr.v[1] - oldAttr.v[1];
	stat[2] = newAttr.v[2] - oldAttr.v[2];
	stat[3] = newAttr.v[3] - oldAttr.v[3];
	stat[4] = newAttr.v[4] - oldAttr.v[4];
	m_pUpgradeDlg = CreateForm<FrUpgradeCaddieDlg>(g_pFresh->GetManager(), this,
		"upgradecaddie", NULL);
	if (m_pUpgradeDlg)
	{
		m_pUpgradeDlg->SetUpgradeInfo(tid, stat, oldPrice, newPrice);
		m_pUpgradeDlg->Open(NULL, 3);
	}
}

// 983 00a897c0 (trecho a89cb5..a89f8c): periodo/quantidade de venda no fim do texto "name" da janela de informacao
void CShopMain::AddSaleInfo983(FrEdit* pEdit, IFF_ITEM_COMMON* pItem)
{
	unsigned long tid = pItem->TypeId;
	unsigned long saleState = 0;
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned long, sShopSaleState983>::iterator itState =
		pDoc->m_shopSaleState983.find(tid);
	if (itState != pDoc->m_shopSaleState983.end())
		saleState = itState->second.state;

	IFF983::sShopLimitItem* pLimit =
		saleState != 0 ? ItemManager()->FindShopLimitItem983(tid) : NULL;
	if (pLimit)
	{
		_SYSTEMTIME* s0 = &pLimit->saleStart;
		_SYSTEMTIME* s1 = &pLimit->saleEnd;
		if (s1->wYear == 0)
		{
			pEdit->AddText("\n", false, true);
			return;
		}
		unsigned long total = pLimit->total;
		unsigned long perDay = pLimit->perDay;
		unsigned long hours = pLimit->hours;
		std::string qty;
		if (hours != 0)
			qty = MakeStr(K2L_Compatibility("%d\xc0\xcf %d \xb0\xb3\xbe\xbf"),
				(int)(hours / 24), (int)perDay);
		else
			qty = MakeStr(K2L_Compatibility("\xc3\xd1 %d \xb0\xb3"), (int)total);
		pEdit->AddText(
			MakeStr(K2L_Compatibility(
						"\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf\\c0xff000000\\c"),
				s0->wMonth, s0->wDay, s1->wMonth, s1->wDay),
			false, true);
		pEdit->AddText("\n", false, true);
		if ((tid & 0xfc000000) == 0x18000000)
			pEdit->AddText(
				MakeStr(K2L_Compatibility(
							"\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xbc\xf6\xb7\xae: %s\\c0xff000000\\c"),
					qty.c_str()),
				false, true);
		return;
	}

	_SYSTEMTIME* s0 = NULL;
	_SYSTEMTIME* s1 = NULL;
	if (!ItemManager()->HasSalePeriod(tid, &s0, &s1) || !s0 || !s1 ||
		s0->wYear == 0 || s1->wYear == 0)
	{
		pEdit->AddText("\n", false, true);
		return;
	}
	bool bDiscount = pItem->DateFlag == 2 &&
		ItemManager()->IsInSalePeriod(tid, pDoc->GetServerTime());
	pEdit->AddText("\n\n\n", false, true);
	pEdit->AddText(
		MakeStr(K2L_Compatibility(bDiscount
					? "\\c0xffff0000\\c\xc7\xd2\xc0\xce\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf\\c0xff000000\\c"
					: "\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf\\c0xff000000\\c"),
			s0->wMonth, s0->wDay, s1->wMonth, s1->wDay),
		false, true);
}

void CShopMain::OpenInformation(unsigned long tid, unsigned long uccGuid, const char* uccIndex)
{
	FrWnd* pWnd = DYNAMIC_CAST(FrWnd,
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("information"));
	if (pWnd)
		return;
	pWnd = DYNAMIC_CAST(FrWnd,
		g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"information_setitem"));
	if (pWnd)
		return;

	CItemManager* pItemMgr = ItemManager();
	IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem(tid);
	// 983 a8988a: com guid de UCC (parte especial, ver o chamador a91340) monta um sItemInfo e tira dele
	// o icone (007e9620) e o nome (007e96a0)
	const Bitmap* uccIcon = NULL;
	char uccName[41] = "";
	if (uccGuid != 0 && uccIndex != NULL)
	{
		sItemInfo info;
		memset(&info, 0, sizeof(info));
		info.tid = tid;
		info.guid = uccGuid;
		info.status = 1;
		strcpy(info.UccIndex, uccIndex);
		uccIcon = GetUccItemIcon(&info);
		strcpy(uccName, GetUccItemName(&info));
	}
	if (!pItem)
		return;

	// 983 a897c0: conjunto abre direto no seu ramo
	if ((pItem->TypeId & 0xfc000000) == 0x24000000)
	{
		FrSetItemDlg* pSetDlg = CreateForm<FrSetItemDlg>(g_pFresh->GetManager(),
			this, "information_setitem", NULL);
		if (!pSetDlg)
			return;
		pSetDlg->SetSetItem(pItem->TypeId);
		pSetDlg->Open(NULL, 3);
		return;
	}
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
		"information", NULL);
	if (!pForm)
		return;
	{

		FrArea* pPortrait =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
		if (pPortrait)
		{
			if (IsSpecialPartTid983(pItem->TypeId) == true)
			{
				if (uccIcon)
					pPortrait->SetBgImg(uccIcon);
			}
			else
				pPortrait->SetBgImg(pItem->Icon);
		}
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
		if (pName && pLevel)
		{
			const char* suffix = "";
			if (pItem->TypeId != 0x1c00001a)
			{
				switch (pItem->InStock)
				{
				case 2:
					suffix = K2L_Compatibility("(\xbc\xb1\xb9\xb0\xc0\xfc\xbf\xeb)");
					break;
				case 3:
					suffix = K2L_Compatibility("(\xb1\xb8\xc0\xd4\xc0\xfc\xbf\xeb)");
					break;
				case 4:
					suffix = K2L_Compatibility("(\xc0\xfc\xbd\xc3\xbf\xeb)");
					break;
				}
			}
			// 983 a89b05: parte especial usa o nome do UCC e, vazio, pula nome + "\n"
			const char* name = pItem->Name;
			if (IsSpecialPartTid983(pItem->TypeId) == true)
				name = uccName[0] ? uccName : NULL;
			if (name)
			{
				pName->AddText(
					MakeStr(K2L_Compatibility(
								"\xc0\xcc\xb8\xa7: %s\\c0xffff0000\\c%s\\c0xff000000\\c"),
						name, suffix),
					false, true);

				pName->AddText("\n", false, true);
			}

			const char* text;
			if (pItem->IsUnderLvl)
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));
				text = "\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf";
			}
			else
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));
				text = "\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3";
			}
			pName->AddText(MakeStr(K2L_Compatibility(text), pItem->Name), false, true);
			pName->AddText("\n\n", false, true);
		}

		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			FrEdit* pAbility =
				DYNAMIC_CAST(FrEdit, pForm->FindChildByName("avility"));
			if (pAbility)
			{
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(tid);
				if (pPart)
				{
					for (int i = 0; i < 5; ++i)
					{
						if (pPart->Attr[i] != 0 || pPart->Slot[i] != 0)
						{
							const char* str = MakeStatusString(i,
								pPart->Attr[i], pPart->Slot[i]);
							pAbility->AddText(
								MakeStr(K2L_Compatibility(
											"\\c0xff0000ff\\c%s\\c0xff000000\\c"),
									str),
								false, true);
							// 983 a89ca5: o "\n" vai para o texto "name" (esi), nao para "avility"
							pName->AddText("\n", false, true);
						}
					}
				}
			}
		}

		// 983 a89cb5: periodo/quantidade no mesmo texto "name"
		if (pName)
			AddSaleInfo983(pName, pItem);

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->TypeId);
		if (pDesc)
		{
			// 983 a89fa0..a8a2d1: desc + aviso de direitos autorais p/ roupa UCC a venda (Category 7/9 = copia/compra);
			// as duas linhas do aviso passam por K2L (00b4ec60 em a8a06b e a8a16b/a8a23b)
			char buf[0x400];
			strncpy(buf, pDesc->Desc, sizeof(buf) - 1);
			buf[sizeof(buf) - 1] = 0;
			if (IsLocalContent(S4_UCC) && IsUccClothes(pItem->TypeId) &&
				!IsSpecialPartTid983(pItem->TypeId))
			{
				IFF_STRUCT::sPart* pUcc = ItemManager()->FindPart(pItem->TypeId);
				bool bCopy = pUcc && (pUcc->Category == 7 || pUcc->Category == 9);
				size_t n = strlen(buf);
				_snprintf(buf + n, sizeof(buf) - 1 - n,
					"\n\\c0xffff0000\\c%s\\c0xff000000\\c\n\\c0xffff0000\\c%s\\c0xff000000\\c",
					K2L_Compatibility("\xc1\xa6\x33\xc0\xda\xc0\xc7\x20\xb1\xc7\xb8\xae\xb8\xa6\x20\xc4\xa7\xc7\xd8\x2f\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb"),
					K2L_Compatibility(bCopy ? "\xc7\xd8\xc4\xa5\x20\xbf\xec\xb7\xc1\xb0\xa1\x20\xc0\xd6\xb4\xc2\x20\xb5\xf0\xc0\xda\xc0\xce\xc0\xc7\x20\xba\xb9\xbb\xe7\x20\xb9\xd7\x20\xb1\xb8\xc0\xd4\xc0\xba\x20\xc7\xc7\xc7\xd8\x20\xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0" : "\xc7\xd8\xc4\xa5\x20\xbf\xec\xb7\xc1\xb0\xa1\x20\xc0\xd6\xb4\xc2\x20\xb5\xf0\xc0\xda\xc0\xce\xc0\xba\x20\xc7\xc7\xc7\xd8\x20\xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
				buf[sizeof(buf) - 1] = 0;
			}
			if (buf[0])
				pForm->SetMessage(buf, false);
		}
	}
	// 983 a8a2db: esconde reset e icon_clubsetworkshop antes do Open
	FrButton* pReset = DYNAMIC_CAST(FrButton, pForm->FindChildByName("reset"));
	if (pReset)
		pReset->SetVisible(false);
	FrButton* pClub = DYNAMIC_CAST(FrButton,
		pForm->FindChildByName("icon_clubsetworkshop"));
	if (pClub)
		pClub->SetVisible(false);
	pForm->Open(NULL, 3);
}

void CShopMain::OpenCaddieInfo(const sCaddieInfo& info)
{
	FrWnd* pWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("caddie_info");
	if (pWnd)
		return;
	FrCaddieInfoDlg* pDlg = DYNAMIC_CAST(FrCaddieInfoDlg, pWnd);
	if (!pDlg)
		pDlg = CreateForm<FrCaddieInfoDlg>(g_pFresh->GetManager(), this,
			"caddie_info", NULL);
	if (pDlg)
	{
		pDlg->SetCaddieInfo(info);
		pDlg->Open((FRESH_PFN_RESULT)&CShopMain::OnReemployeeResult, 3);
	}
}

void CShopMain::ResetItemList(bool bKeepChar)
{
	if (m_pItemList)
	{
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pItemList->m_itemList.begin();
			it != m_pItemList->m_itemList.end(); ++it)
		{
			FrListItem* pItem = *it;
			if (pItem->selected &&
				(!bKeepChar ||
					(((IFF_ITEM_COMMON*)pItem->pData)->TypeId & 0xfc000000) !=
						0x4000000))
				m_pItemList->UnselectItem(pItem);
		}
	}
}

int CShopMain::GetItemNumInMyPocket(unsigned long tid, bool bNoCash)
{
	int count = 0;
	if (tid == 0 || (tid & 0xfc000000) == 0x4000000)
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			CSharedDoc::Instance()->m_charMap.begin();
		if (it != CSharedDoc::Instance()->m_charMap.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem =
					pItemMgr->FindCommonItem((*it).second.tid);
				if (pItem && (tid == 0 || (*it).second.tid == tid) &&
					(!bNoCash || !pItem->IsCash))
					count++;
				++it;
			} while (it != CSharedDoc::Instance()->m_charMap.end());
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x8000000)
	{
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_partsList.begin();
		if (it != CSharedDoc::Instance()->m_partsList.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem((*it).tid);
				if (pItem && (tid == 0 || (*it).tid == tid) &&
					(!bNoCash || !pItem->IsCash))
					count++;
				++it;
			} while (it != CSharedDoc::Instance()->m_partsList.end());
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x18000000)
	{
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_myItemList.begin();
		if (it != CSharedDoc::Instance()->m_myItemList.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem((*it).tid);
				if (pItem && (tid == 0 || (*it).tid == tid) &&
					(!bNoCash || !pItem->IsCash))
				{
					// 983 00a8872b: subtipo 1 de 0x18 conta por unidade, mesmo com ItemType
					if (((tid & 0xfc000000) == 0x18000000 && (tid & 0x1f00000) == 0x100000) ||
						(*it).ItemType == 0)
						count++;
					else
						count = 100000;
				}
				++it;
			} while (it != CSharedDoc::Instance()->m_myItemList.end());
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x1c000000)
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			CSharedDoc::Instance()->m_caddieMap.begin();
		if (it != CSharedDoc::Instance()->m_caddieMap.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem =
					pItemMgr->FindCommonItem((*it).second.tid);
				if (pItem && (tid == 0 || (*it).second.tid == tid) &&
					(!bNoCash || !pItem->IsCash))
					count++;
				++it;
			} while (it != CSharedDoc::Instance()->m_caddieMap.end());
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x14000000)
	{
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_ballList.begin();
		if (it != CSharedDoc::Instance()->m_ballList.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem((*it).tid);
				if (pItem && (tid == 0 || (*it).tid == tid) &&
					(!bNoCash || !pItem->IsCash))
					count++;
				++it;
			} while (it != CSharedDoc::Instance()->m_ballList.end());
		}
	}
	unsigned long type = tid >> 26;
	if (type == 9 && (tid & 0x3e00000) == 0x200000)
	{
		CItemManager* pItemMgr = ItemManager();
		IFF_STRUCT::sSetItem* pSet = pItemMgr->FindSetItem(tid);
		{
			for (unsigned char n = 0; n < pSet->nElems; n++)
			{
				if (tid == 0 || (pSet->ElemIds[n] & 0xfc000000) != 0x4000000)
				{
					std::list<sItemInfo>::iterator it =
						CSharedDoc::Instance()->m_partsList.begin();
					if (it != CSharedDoc::Instance()->m_partsList.end())
					{
						do
						{
							CItemManager* pItemMgr = ItemManager();
							IFF_ITEM_COMMON* pItem =
								pItemMgr->FindCommonItem((*it).tid);
							if (pItem &&
								(tid == 0 || (*it).tid == pSet->ElemIds[n]) &&
								(!bNoCash || !pItem->IsCash))
								count++;
							++it;
						} while (
							it != CSharedDoc::Instance()->m_partsList.end());
					}
				}
			}
		}
	}
	if (tid == 0 || type == 0xe)
	{
		std::list<sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_cutinList.begin();
		if (it != CSharedDoc::Instance()->m_cutinList.end())
		{
			do
			{
				CItemManager* pItemMgr = ItemManager();
				IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem((*it).tid);
				if (pItem && (tid == 0 || (*it).tid == tid) &&
					(!bNoCash || !pItem->IsCash))
					count++;
				++it;
			} while (it != CSharedDoc::Instance()->m_cutinList.end());
		}
	}
	if (tid == 0 || type == 0xf)
		count = 1;
	if (type == 4)
	{
		std::map<unsigned int, sItemInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_clubSetMap.begin();
			it != CSharedDoc::Instance()->m_clubSetMap.end(); it++)
		{
			CItemManager* pItemMgr = ItemManager();
			IFF_ITEM_COMMON* pItem = pItemMgr->FindCommonItem((*it).second.tid);
			if (pItem && (tid == 0 || (*it).second.tid == tid) &&
				(!bNoCash || !pItem->IsCash))
				count++;
		}
	}
	return count;
}

void CShopMain::Checkout()
{
	if (CSharedDoc::Instance()->IsControlServerService(4))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbb\xf3\xc1\xa1 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
		return;
	}
	if (m_pPartCartList->m_itemList.size() +
			m_pAuxPartCartList->m_itemList.size() ==
		0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xb1\xb8\xb8\xc5\xc7\xd2 \xbb\xf3\xc7\xb0\xc0\xbb "
				 "\xbc\xb1\xc5\xc3\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"),
			0, 0, 0, 0));
		return;
	}
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		unsigned char stock = ((IFF_ITEM_COMMON*)(*it)->pData)->InStock;
		// 983 00a8c68a: a parte especial (00a82ce0) compra mesmo fora de venda (so neste carrinho)
		if ((stock == 0 || stock == 2 || stock == 4) &&
			!IsSpecialPartTid983(((IFF_ITEM_COMMON*)(*it)->pData)->TypeId))
			goto CannotBuy;
	}
	for (it = m_pAuxPartCartList->m_itemList.begin();
		it != m_pAuxPartCartList->m_itemList.end(); ++it)
	{
		unsigned char stock = ((IFF_ITEM_COMMON*)(*it)->pData)->InStock;
		if (stock == 0 || stock == 2 || stock == 4)
		{
CannotBuy:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xb8\xb8\xc5\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xbb\xf3\xc7\xb0\xc0\xcc "
					 "\xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			return;
		}
	}
	CSharedDoc* pDoc = CSharedDoc::Instance();
	FrCheckoutDlg::CartItem cart;
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		cart.pItem = (IFF_STRUCT::sItem*)(*it)->pData;
		if ((((IFF_ITEM_COMMON*)cart.pItem)->TypeId & 0xfc000000) == 0x48000000)
		{
			std::list<sFurniture_List>::iterator fit;
			for (fit = pDoc->m_furnitureList.begin();
				fit != pDoc->m_furnitureList.end(); ++fit)
			{
				IFF_STRUCT::sFurniture* pFur =
					pDoc->m_itemManager.FindFurniture((*fit).typeId);
				pDoc = CSharedDoc::Instance();
				if (pFur == NULL)
					break;
				if (pFur->IsOwn == 0 &&
					(*fit).typeId == ((IFF_ITEM_COMMON*)cart.pItem)->TypeId)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb "
							 "\xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0));
					return;
				}
			}
		}
	}
	m_pCheckoutDlg = CreateForm<FrCheckoutDlg>(g_pFresh->GetManager(), this,
		"checkout", NULL);
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		cart.pItem = (IFF_STRUCT::sItem*)(*it)->pData;
		unsigned long tid = ((IFF_ITEM_COMMON*)cart.pItem)->TypeId;
		if ((tid & 0xfc000000) == 0x24000000)
		{
			IFF_STRUCT::sSetItem* pSet = ItemManager()->FindSetItem(tid);
			int i = 0;
			if (pSet != NULL && (cart.count = 0, pSet->nElems > 0))
			{
				do
				{
					unsigned long elem = pSet->ElemIds[i];
					if (elem != 0 && (elem >> 26) != 1 && (elem >> 26) != 6)
						cart.count += GetItemNumInMyPocket(elem, false);
					i++;
				} while (i < (int)pSet->nElems);
			}
		}
		else
		{
			cart.count = GetItemNumInMyPocket(tid, false);
		}
		cart.option = 5;
		m_pCheckoutDlg->Cart().push_back(cart);
	}
	for (it = m_pAuxPartCartList->m_itemList.begin();
		it != m_pAuxPartCartList->m_itemList.end(); ++it)
	{
		cart.pItem = (IFF_STRUCT::sItem*)(*it)->pData;
		cart.count =
			GetItemNumInMyPocket(((IFF_ITEM_COMMON*)cart.pItem)->TypeId, false);
		cart.option = 5;
		m_pCheckoutDlg->Cart().push_back(cart);
	}
	m_pCheckoutDlg->SetCaddieIffMap(m_caddieIffMap); // 983 a8ca0f (00aa25b0)
	bool bRental = IsSelectedRentalTab(); // 983 a8ca16 (a83f20)
	m_pCheckoutDlg->Checkout(CSharedDoc::Instance()->m_myInfo.stat.i64Pang,
		CSharedDoc::Instance()->m_cookie, bRental);
	m_pCheckoutDlg->Open((FRESH_PFN_RESULT)&CShopMain::OnBuyDlgResult, 0x81);
}

void CShopMain::SendGift(const IFF_ITEM_COMMON& item)
{
	if (CSharedDoc::Instance()->IsControlServerService(12))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xd4 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
	}
	else if ((bool)((CSharedDoc::Instance()->m_myInfo.info.dwIdentity >> 8) &
				 1))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xcf\xb1\xe2\xb8\xa6 \xc7\xd2 \xbc\xf6 "
				 "\xbe\xf8\xb4\xc2 \xb0\xe8\xc1\xa4\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
	}
	else if (m_pPartCartList)
	{
		if (m_pPartCartList->m_itemList.empty() &&
			m_pAuxPartCartList->m_itemList.empty())
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xb8\xb8\xc5\xc7\xd2 \xbb\xf3\xc7\xb0\xc0\xbb \xbc\xb1\xc5\xc3"
					 "\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"),
				0, 0, 0, 0));
			return;
		}
		if (item.InStock == 0 || item.InStock == 3 || item.InStock == 4)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 "
					 "\xbb\xf3\xc7\xb0\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		// 983 00a875e6: 0x4d ao ator principal, que cria e guarda a janela (CTaskMain 0065e0b5)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x4d, (int)&item, 0, 0, 0, 0));
	}
}

void CShopMain::Repay(const IFF_ITEM_COMMON& item, unsigned long guid)
{
	FrRepayDlg* pDlg =
		CreateForm<FrRepayDlg>(g_pFresh->GetManager(), this, "repay", NULL);
	if (pDlg)
	{
		pDlg->Repay(item, guid, CSharedDoc::Instance()->m_myInfo.stat.i64Pang,
			CSharedDoc::Instance()->m_cookie);
		pDlg->Open((FRESH_PFN_RESULT)&CShopMain::OnSellDlgResult, 0x81);
	}
}

void CShopMain::ClearCartList(bool bKeepChar, CPartTidList* pTids)
{
	if (!m_pPartCartList)
		return;

	FrListBox::ITEM_LIST::iterator it = m_pPartCartList->m_itemList.begin();
	if (it != m_pPartCartList->m_itemList.end())
	{
		do
		{
			IFF_ITEM_COMMON* pItem = (IFF_ITEM_COMMON*)(*it)->pData;
			unsigned long type = pItem->TypeId >> 26;
			if (type == 1)
			{
				if (!bKeepChar)
					it = m_pPartCartList->_DelItem(it);
				else
					++it;
			}
			else
			{
				if (type == 2 && pTids)
				{
					if (pTids->IsEquipped(pItem->TypeId, 0) &&
						!m_hand.IsWorking(0))
					{
						// 983 00a8b690: 0x18a ao "LobbyAvatar" (lista, &TypeId), montada antes do GetActor
						MsgObject msg(NULL, 0x18a, (int)pTids, (int)&pItem->TypeId, 0, 0, 0);
						IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
						if (pActor)
							pActor << msg;
						RestoreShoppersOldItem(pItem->TypeId, *pTids);
					}
				}
				else if (type == 0xf && pTids && !m_hand.IsWorking(0))
				{
					// 983 00a8b690: personagem da gaveta do lobby (005f6430), nao o da PetFrame
					unsigned long tidChar = 0;
					if (RFLobbyDesktop::Instance983())
						tidChar = RFLobbyDesktop::Instance983()->GetCharTypeId983();
					std::map<unsigned int, sCharacterInfo>::iterator itChar =
						CSharedDoc::Instance()->m_charMap.begin();
					while (itChar != CSharedDoc::Instance()->m_charMap.end())
					{
						if (itChar->second.tid == tidChar)
						{
							m_b520 = itChar->second.hairClr;
							// 983: 0x18c ao "LobbyAvatar" (lista, &cor, 0)
							MsgObject msg(NULL, 0x18c,
								(int)&CSharedDoc::Instance()->GetMyPartTidList(),
								(int)&m_b520, 0, 0, 0);
							IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
							if (pActor)
								pActor << msg;
							goto cart_deleted;
						}
						++itChar;
					}
					m_b520 = 0;
				}
cart_deleted:
				it = m_pPartCartList->_DelItem(it);
			}
		} while (it != m_pPartCartList->m_itemList.end());
	}

	FrListBox::ITEM_LIST::iterator itAux =
		m_pAuxPartCartList->m_itemList.begin();
	if (itAux != m_pAuxPartCartList->m_itemList.end())
	{
		do
		{
			IFF_ITEM_COMMON* pItem = (IFF_ITEM_COMMON*)(*itAux)->pData;
			if ((pItem->TypeId & 0xfc000000) == 0x8000000 && pTids &&
				pTids->IsEquipped(pItem->TypeId, 0))
			{
				bool bHandIdle;
				HAND_QUERY_IDLE(&m_hand, bHandIdle);
				if (bHandIdle)
				{
					// 983 manda (int)&pTids aqui (lea [ebp+0xc], 00a8b944); o 0x18a le CPartTidList*: mantido pTids
					MsgObject msg(NULL, 0x18a, (int)pTids, (int)&pItem->TypeId, 0, 0, 0);
					IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
					if (pActor)
						pActor << msg;
					RestoreShoppersOldItem(pItem->TypeId, *pTids);
				}
			}
			itAux = m_pAuxPartCartList->_DelItem(itAux);
		} while (itAux != m_pAuxPartCartList->m_itemList.end());
	}
	CheckAuxCartScroll();
}

std::list<FrListItem*>::iterator CShopMain::DeleteItemFromCartList(
	unsigned long tid)
{
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pPartCartList->m_itemList.begin();
		it != m_pPartCartList->m_itemList.end(); ++it)
	{
		if (((IFF_ITEM_COMMON*)(*it)->pData)->TypeId == tid)
			return m_pPartCartList->_DelItem(it);
	}
	return m_pPartCartList->m_itemList.end();
}

std::list<FrListItem*>::iterator CShopMain::DeleteItemFromAuxCartList(
	unsigned long tid)
{
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pAuxPartCartList->m_itemList.begin();
		it != m_pAuxPartCartList->m_itemList.end(); ++it)
	{
		if (((IFF_ITEM_COMMON*)(*it)->pData)->TypeId == tid)
		{
			FrListBox::ITEM_LIST::iterator next =
				m_pAuxPartCartList->_DelItem(it);
			CheckAuxCartScroll();
			return next;
		}
	}
	return m_pAuxPartCartList->m_itemList.end();
}

bool CShopMain::SelectItemList(unsigned long tid)
{
	if (!m_pItemList)
		return false;
	FrListBox::ITEM_LIST::iterator it = m_pItemList->m_itemList.begin();
	if (it != m_pItemList->m_itemList.end())
	{
		while (!(*it)->pData || ((IFF_ITEM_COMMON*)(*it)->pData)->TypeId != tid)
		{
			++it;
			if (it == m_pItemList->m_itemList.end())
				return false;
		}
		if (*it)
		{
			SelectItemList(*it);
			return true;
		}
	}
	return false;
}

bool CShopMain::SelectItemList(FrListItem* pListItem)
{
	IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)pListItem->pData;
	unsigned int* pTypeId = &pData->TypeId;
	unsigned long tid = *pTypeId;
	char msg[256];
	if ((tid & 0x3e00000) == 0x800000 && (tid & 0x1fe000) == 0x1f8000)
	{
		IFF_STRUCT::sSetItem* pSetItem = ItemManager()->FindSetItem(tid);
		if (pSetItem != NULL)
			tid = pSetItem->ElemIds[0];
	}
	switch (tid >> 26)
	{
	case 1:
	{
		CSharedDoc::Instance()->BuildMyPartTidListDefault(*pTypeId);
		CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
		pTids->m_hairColor = m_b520;
		m_u524 = *pTypeId;
		m_pParts = pTids->m_tid;
		// 983 00a8ca90: 0x187 ao "LobbyAvatar" (&tid) no lugar de SafeLoadPet
		{
			IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
			if (pAv)
				pAv << MsgObject(NULL, 0x187, (int)&m_u524, 0, 0, 0, 0);
		}
		// 983 00a8ca90: com a gaveta de personagens aberta, fecha-a; o desktop fica marcado como alterado sempre
		if (RFLobbyCharacterSelectAnimWnd::ms_pInstance && RFLobbyCharacterSelectAnimWnd::ms_pInstance->m_bOpen)
			RFLobbyCharacterSelectAnimWnd::ms_pInstance->Select983(2, false);
		if (RFLOBBYDESKTOP983)
			RFLOBBYDESKTOP983->m_bChanged = true;
		IFF_ITEM_COMMON* pHair = NULL;
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pPartCartList->m_itemList.begin();
			it != m_pPartCartList->m_itemList.end(); ++it)
		{
			pHair = (IFF_ITEM_COMMON*)(*it)->pData;
			if ((pHair->TypeId & 0xfc000000) == 0x3c000000)
				break;
			pHair = NULL;
		}
		m_pAuxPartCartList->ClearItem();
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		if (pHair != NULL)
			m_pPartCartList->AddItem(pHair);
		TurnOnItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		break;
	}
	case 2:
	{
		IFF_ITEM_COMMON* pPart = (IFF_ITEM_COMMON*)pListItem->pData;
		CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
		unsigned long overlapped = pTids->GetOverlapped(pPart->TypeId);
		if (!pTids->CanSetPart(pPart->TypeId))
		{
			m_pItemList->UnselectItem(pListItem);
			if (((IFF_STRUCT::sPart*)pData)->EquippableWith[0] != 0)
			{
				// 983 00a8ca90: "so pode vestir com %s" (nome em EquippableWith), por K2L_Compatibility
				sprintf(msg,
					K2L_Compatibility("\xba\xbb \xbb\xf3\xc7\xb0\xc0\xba \\c0xffff0000\\c%s\xc0\xbb(\xb8\xa6) "
					"\xc0\xd4\xc0\xba \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb8\xb8\\c0xff000000\\c "
					"\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					((IFF_STRUCT::sPart*)pData)->EquippableWith);
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
				return false;
			}
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\xbb\xf3\xc7\xb0\xc0\xbb \xc2\xf8\xbf\xeb\xc7\xd2 "
					 "\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			return false;
		}
		TakeOff_PartNCartList(*pTids, overlapped);
		TakeOff_AuxPartNCartList(*pTids, overlapped);
		unsigned long oldTid = 0;
		IFF_ITEM_COMMON* pCur = (IFF_ITEM_COMMON*)pListItem->pData;
		if (UccManager()->IsDisplayUccClothes(pData))
		{
			unsigned long mask = *pTypeId;
			for (std::list<oldItem_t>::iterator oit = m_oldItems.begin();
				oit != m_oldItems.end();)
			{
				if ((((*oit).tid >> 13) & 0x1f) == ((mask >> 13) & 0x1f))
				{
					UnselectItemFromItemList((*oit).tid);
					oit = m_oldItems.erase(oit);
				}
				else
					++oit;
			}
			FrListBox::ITEM_LIST::iterator it;
			for (it = m_pItemList->m_itemList.begin();
				it != m_pItemList->m_itemList.end(); ++it)
			{
				FrListItem* p = *it;
				if (p != pListItem && p->pData != NULL &&
					((((IFF_ITEM_COMMON*)p->pData)->TypeId ^ mask) & 0x3e000) ==
						0)
					m_pItemList->UnselectItem(p);
			}
			oldTid = ((sDisplayUccClothes*)pCur)->itemId;
		}
		// 983 00a8cf5b: parte especial (00a82ce0) passa o guid do UCC (+0x238)
		if (IsSpecialPartTid983(*pTypeId) == true)
			oldTid = *(const unsigned long*)SPARTTAIL983(pListItem->pData);
		PutOnNBackupShoppersOldItem(pPart->TypeId, *pTids, oldTid);
		if (!UccManager()->IsDisplayUccClothes(pCur))
		{
			unsigned char cat = ((IFF_STRUCT::sPart*)pData)->Category;
			if (cat == 0 || cat == 1 || cat == 4 || cat == 7 || cat == 8 ||
				cat == 9)
				m_pPartCartList->AddItem(pData);
			else
			{
				m_pAuxPartCartList->AddItem(pData);
				CheckAuxCartScroll();
			}
		}
		m_pParts = pTids->m_tid;
		// 983 00a8cfd0: religa os efeitos do boneco (00873a30)
		if (m_pExhibition)
			m_pExhibition->AttachFx();
		ShowMainModel(*pTids);
		if ((unsigned char)m_b520 != pTids->m_hairColor)
		{
			// 983: 0x18c ao "LobbyAvatar" (lista, &cor, 0)
			IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
			if (pAv)
				pAv << MsgObject(NULL, 0x18c, (int)pTids, (int)&m_b520, 0, 0, 0);
		}
		TurnOnItemStatus();
		g_audio->PlaySfx("item_select_cloth");
		break;
	}
	case 9:
	{
		unsigned long sub = (tid >> 21) & 0x1f;
		if (sub != 6 && sub != 5 && sub != 4 && sub != 8)
		{
			LARGE_INTEGER d820t0; // D820: tempo total do clique no pacote (item 11 dono10)
			QueryPerformanceCounter(&d820t0);
			m_pPartCartList->ClearItem();
			IFF_STRUCT::sSetItem* pSet =
				(IFF_STRUCT::sSetItem*)pListItem->pData;
			CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
			if ((pSet->c.TypeId & 0x3e00000) == 0x200000 ||
				(pSet->ElemIds[0] & 0xfc000000) == 0x4000000)
			{
				IFF_STRUCT::sChar* pChar;
				if ((pSet->ElemIds[0] & 0xfc000000) == 0x4000000)
					pChar = ItemManager()->FindChar(pSet->ElemIds[0]);
				else
					pChar = ItemManager()->FindChar(
						((pSet->c.TypeId >> 13) & 0xff) | 0x4000000);
				if (pChar != NULL)
				{
					for (int i = 0; i < pChar->nParts + pChar->nAcsries; i++)
					{
						unsigned long t = pTids->m_tid[i];
						if (t != 0 && (t & 0x600) == 0)
						{
							// 983 00a8d1cb: 0x18a ao "LobbyAvatar" (lista, &tid) no lugar do SetPart do 645
							IActor* pLav = AfxGetTask()->GetActor("LobbyAvatar");
							if (pLav)
								pLav << MsgObject(NULL, 0x18a, (int)pTids,
									(int)&pTids->m_tid[i], 0, 0, 0);
						}
					}
				}
			}
			for (int i = 0; i < pSet->nElems; i++)
			{
				if ((pSet->ElemIds[i] & 0xfc000000) == 0x8000000)
					PutOnNBackupShoppersOldItem(pSet->ElemIds[i], *pTids, 0);
			}
			m_pParts = pTids->m_tid;
			{
				IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
				if (pAv)
					pAv << MsgObject(NULL, 0x18c, (int)pTids, (int)&m_b520, 0, 0, 0);
			}
			IFF_ITEM_COMMON* pHair = NULL;
			FrListBox::ITEM_LIST::iterator it;
			for (it = m_pPartCartList->m_itemList.begin();
				it != m_pPartCartList->m_itemList.end(); ++it)
			{
				pHair = (IFF_ITEM_COMMON*)(*it)->pData;
				if ((pHair->TypeId & 0xfc000000) == 0x3c000000)
					break;
				pHair = NULL;
			}
			ShowMainModel(*pTids);
			m_pAuxPartCartList->ClearItem();
			m_pPartCartList->ClearItem();
			m_pPartCartList->AddItem(pSet);
			if (pHair != NULL)
				m_pPartCartList->AddItem(pHair);
			m_pStatBox->SetVisible(true);
			SetLevelBar();
			g_audio->PlaySfx("item_select_cloth");
			{
				LARGE_INTEGER f, n;
				QueryPerformanceFrequency(&f);
				QueryPerformanceCounter(&n);
				jp983_rastro("D820 set tid=%08x n=%d total=%.1f ms", pSet->c.TypeId, (int)pSet->nElems,
					(double)(n.QuadPart - d820t0.QuadPart) * 1000.0 / (double)f.QuadPart);
			}
			break;
		}
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		SafeLoadPet((unsigned long*)pTypeId, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	}
	case 0x12:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		SafeLoadPet((unsigned long*)pTypeId, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	case 0xf:
	{
		CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
		{
			IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
			if (pAv)
				pAv << MsgObject(NULL, 0x18c, (int)pTids,
					(int)&((IFF_STRUCT::sHairStyle*)pData)->cHairID, 0, 0, 0);
		}
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pPartCartList->m_itemList.begin();
			it != m_pPartCartList->m_itemList.end();)
		{
			IFF_ITEM_COMMON* pItem = (IFF_ITEM_COMMON*)(*it)->pData;
			if ((pItem->TypeId & 0xfc000000) == 0x3c000000)
			{
				UnselectItemFromItemList(pItem->TypeId);
				it = DeleteItemFromCartList(pItem->TypeId);
			}
			else
				++it;
		}
		m_pPartCartList->AddItem(pData);
		m_b520 = ((IFF_STRUCT::sHairStyle*)pData)->cHairID;
		TurnOnItemStatus();
		g_audio->PlaySfx("item_select_hair");
		break;
	}
	case 4:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		m_clubsetTid = *pTypeId;
		SafeLoadPet(&m_clubsetTid, false);
		TurnOnItemStatus();
		g_audio->PlaySfx("item_select_club");
		ShowItemDesc(m_clubsetTid);
		break;
	case 5:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		m_aztecTid = *pTypeId;
		SafeLoadPet(&m_aztecTid, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("item_select_aztec");
		ShowItemDesc(m_aztecTid);
		break;
	case 7:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		m_u528 = *pTypeId;
		{
			// 983: 0x1a7 ao "LobbyCaddie" (&tid)
			IActor* pCad = AfxGetTask()->GetActor("LobbyCaddie");
			if (pCad)
				pCad << MsgObject(NULL, 0x1a7, (int)&m_u528, 0, 0, 0, 0);
		}
		// 983 00a8d68e: desktop +0x259 marcado
		if (RFLOBBYDESKTOP983)
			RFLOBBYDESKTOP983->m_259 = true;
		TurnOnItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	case 8:
		if ((tid & 0x1fe000) != 0x6000)
		{
			m_pPartCartList->ClearItem();
			m_pPartCartList->AddItem(pData);
			// 983 00a8d6af: 0x1a7 ao "LobbyCaddie" (pTypeId) no lugar de SafeLoadPet
			IActor* pCad = AfxGetTask()->GetActor("LobbyCaddie");
			if (pCad)
				pCad << MsgObject(NULL, 0x1a7, (int)pTypeId, 0, 0, 0, 0);
			// 983 00a8d76d: desktop +0x259 marcado
			if (RFLOBBYDESKTOP983)
				RFLOBBYDESKTOP983->m_259 = true;
			TurnOnItemStatus();
		}
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	case 0x10:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		m_mascotTid = *pTypeId;
		SafeLoadPet(&m_mascotTid, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	case 0xe:
	{
		eEquipSkin skin;
		if (S5::UTIL::GetEquipSkinIndex(skin, tid))
		{
			if (m_skinTid[skin] != 0)
				DeleteItemFromAuxCartList(m_skinTid[skin]);
			m_skinTid[skin] = *pTypeId;
			IFF_STRUCT::sSkin* pSkin = (IFF_STRUCT::sSkin*)pListItem->pData;
			if ((*pTypeId & 0x3c00000) == 0)
			{
				m_u724 = pSkin->HScroll;
				m_u728 = pSkin->VScroll;
				m_f71c = 0;
				m_f720 = 0;
			}
		}
		else if ((tid & 0x3c00000) == 0x1400000)
		{
			FrListBox::ITEM_LIST::iterator it;
			for (it = m_pAuxPartCartList->m_itemList.begin();
				it != m_pAuxPartCartList->m_itemList.end(); ++it)
			{
				if ((((IFF_ITEM_COMMON*)(*it)->pData)->TypeId & 0x3c00000) ==
					0x1400000)
				{
					DeleteItemFromAuxCartList(
						((IFF_ITEM_COMMON*)(*it)->pData)->TypeId);
					break;
				}
			}
		}
		else
			m_pAuxPartCartList->ClearItem();
		m_pAuxPartCartList->AddItem(pData);
		CheckAuxCartScroll();
		g_audio->PlaySfx("ui_desktop_icon_click");
		{
			// 983 a8d8d9: titulo (tid & 0x3fffff) sem CanEquip desliga comprar/limpar
			Title* pTitle = g_TitleManager->GetTitle((eTitle)(*pTypeId & 0x3fffff));
			if (pTitle != NULL && m_pBuyAll != NULL)
			{
				bool bOk = pTitle->CanEquip();
				m_pBuyAll->Enable(bOk);
				m_pBuyReset->Enable(bOk);
			}
		}
		break;
	}
	case 6:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		SafeLoadPet((unsigned long*)pTypeId, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	case 0x1f:
		m_pPartCartList->ClearItem();
		m_pPartCartList->AddItem(pData);
		SafeLoadPet((unsigned long*)pTypeId, false);
		TurnOffItemStatus();
		g_audio->PlaySfx("ui_desktop_icon_click");
		ShowItemDesc(*pTypeId);
		break;
	}
	pListItem->selected = true;
	return true;
}

void CShopMain::UnselectItemList(unsigned long tid)
{
	CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
	switch (tid >> 26)
	{
	case 2:
	{
		DeleteItemFromCartList(tid);
		{
			FrListBox::ITEM_LIST::iterator it = DeleteItemFromAuxCartList(tid);
		}
		UnselectItemFromItemList(tid);
		unsigned long oldTids[24];
		memcpy(oldTids, pTids->m_tid, sizeof(oldTids));
		if (pTids->IsEquipped(tid, 0))
		{
			// 983 00a8da20: 0x18a ao "LobbyAvatar" (lista, &tid, &bool falso) no lugar do SetPart do 645;
			// a mensagem e montada antes do GetActor (o nosso mandava param3 = 0)
			bool bFalse = false;
			MsgObject msg(NULL, 0x18a, (int)pTids, (int)&tid, (int)&bFalse, 0, 0);
			IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
			if (pActor)
				pActor << msg;
			RestoreShoppersOldItem(tid, *pTids);
			m_pParts = pTids->m_tid;
		}
		for (int i = 0; i < 24; ++i)
		{
			unsigned long old = oldTids[i];
			if (old != 0 && tid != old && pTids->m_tid[i] != old && old != 2 &&
				old != 3)
			{
				UnselectItemFromItemList(old);
				DeleteItemFromCartList(old);
				DeleteItemFromAuxCartList(old);
			}
		}
		pTids->m_hairColor = m_b520;
		ShowMainModel(*pTids);
		return;
	}
	default:
		// 983 00a8da20: sem esconder o m_pStatBox (o 645 escondia)
		UnselectItemFromItemList(tid);
		SetLevelBar();
		break;
	case 7:
	case 0x1f:
		break;
	case 9:
	{
		DeleteItemFromCartList(tid);
		UnselectItemFromItemList(tid);
		CItemManager* pItemMgr = ItemManager();
		IFF_STRUCT::sSetItem* pSet = pItemMgr->FindSetItem(tid);
		for (int i = 0; i < pSet->nElems; ++i)
		{
			unsigned long elem = pSet->ElemIds[i];
			if (pTids->IsEquipped(elem, 0))
			{
				// 983 00a8da20 (caso 9): param3 = &bool falso, como no caso 2
				bool bFalse = false;
				MsgObject msg(NULL, 0x18a, (int)pTids, (int)&elem, (int)&bFalse, 0, 0);
				IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
				if (pActor)
					pActor << msg;
				RestoreShoppersOldItem(elem, *pTids);
				m_pParts = pTids->m_tid;
			}
		}
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pItemList->m_itemList.begin();
			it != m_pItemList->m_itemList.end(); ++it)
		{
			if ((*it)->pData &&
				(((IFF_ITEM_COMMON*)(*it)->pData)->TypeId & 0xfc000000) ==
					0x4000000)
			{
				SelectItemList(*it);
				m_pItemList->SelectItem(*it, true);
				break;
			}
		}
		pTids->m_hairColor = m_b520;
		ShowMainModel(*pTids);
		return;
	}
	case 0xf:
	{
		DeleteItemFromCartList(tid);
		UnselectItemFromItemList(tid);
		char hairClr = 0;
		CPartTidList* pTids = &CSharedDoc::Instance()->GetMyPartTidList();
		std::map<unsigned int, sCharacterInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_charMap.begin();
			it != CSharedDoc::Instance()->m_charMap.end(); ++it)
		{
			// 983 00a8da20: personagem da gaveta do lobby (005f6430), nao o da PetFrame
			if (it->second.tid == (RFLobbyDesktop::Instance983() ?
				RFLobbyDesktop::Instance983()->GetCharTypeId983() : 0))
			{
				hairClr = it->second.hairClr;
				break;
			}
		}
		m_b520 = hairClr;
		pTids->m_hairColor = hairClr;
		// 983 00a8da20: 0x18c ao "LobbyAvatar" (lista, &cor, 0), mensagem montada antes do GetActor
		MsgObject msg(NULL, 0x18c, (int)pTids, (int)&hairClr, 0, 0, 0);
		IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
		if (pActor)
			pActor << msg;
		ShowMainModel(*pTids);
		return;
	}
	case 0xe:
	{
		// 983 00a8da20: skin sai da lista auxiliar; no fim reabilita Buy All/Reset
		DeleteItemFromAuxCartList(tid);
		UnselectItemFromItemList(tid);
		eEquipSkin skin;
		if (S5::UTIL::GetEquipSkinIndex(skin, tid) != 0)
		{
			m_skinTid[skin] =
				CSharedDoc::Instance()->m_myInfo.userEquip.tidSkin[skin];
			if ((tid & 0x3c00000) == 0)
			{
				if (m_skinTid[skin] == 0)
				{
					m_u724 = 0;
					m_u728 = 0;
				}
				else
				{
					CItemManager* pItemMgr = ItemManager();
					IFF_STRUCT::sSkin* pSkin = pItemMgr->FindSkin(m_skinTid[skin]);
					m_u724 = pSkin->HScroll;
					m_u728 = pSkin->VScroll;
				}
				m_f71c = 0;
				m_f720 = 0;
			}
		}
		if (m_pBuyAll)
		{
			m_pBuyAll->Enable(true);
			m_pBuyReset->Enable(true);
			return;
		}
		break;
	}
	}
}

bool CShopMain::UnselectItemFromItemList(unsigned long tid)
{
	bool bFound = false;
	if (m_pItemList)
	{
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pItemList->m_itemList.begin();
			it != m_pItemList->m_itemList.end(); ++it)
		{
			IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
			if (pData && pData->TypeId == tid)
			{
				m_pItemList->UnselectItem(*it);
				bFound = true;
			}
		}
	}
	return bFound;
}

void CShopMain::TakeOff_PartNCartList(CPartTidList& tids, unsigned long mask)
{
	FrListBox::ITEM_LIST::iterator it = m_pPartCartList->m_itemList.begin();
	if (it != m_pPartCartList->m_itemList.end())
	{
		do
		{
			IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
			unsigned int group = pData->TypeId >> 26;
			if (group == 2)
			{
				// 983 00a8b9e0: remocao no fall-through, ++it no else
				if ((mask & tids.GetOverlapped(pData->TypeId)) != 0)
				{
					UnselectItemFromItemList(pData->TypeId);
					it = DeleteItemFromCartList(pData->TypeId);
					if (tids.IsEquipped(pData->TypeId, 0))
					{
						MsgObject msg(NULL, 0x18a, (int)&tids, (int)&pData->TypeId, 0, 0, 0);
						IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
						if (pActor)
							pActor << msg;
					}
					RestoreShoppersOldItem(pData->TypeId, tids);
					UnselectItemList(pData->TypeId);
				}
				else
					++it;
			}
			else if (group == 9)
			{
				IFF_STRUCT::sSetItem* pSet = (IFF_STRUCT::sSetItem*)pData;
				int i = 0;
				if (pSet->nElems != 0)
				{
					unsigned int* pElem = pSet->ElemIds;
					do
					{
						unsigned long tid = *pElem;
						if (tids.IsEquipped(tid, 0))
						{
							MsgObject msg(NULL, 0x18a, (int)&tids, (int)&tid, 0, 0, 0);
							IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
							if (pActor)
								pActor << msg;
						}
						RestoreShoppersOldItem(tid, tids);
						++pElem;
						++i;
					} while (i < pSet->nElems);
				}
				UnselectItemFromItemList(pData->TypeId);
				it = DeleteItemFromCartList(pData->TypeId); // 983 a8bb72-a8bb87
			}
			else
				++it;
		} while (it != m_pPartCartList->m_itemList.end());
	}
}

void CShopMain::TakeOff_AuxPartNCartList(CPartTidList& tids, unsigned long mask)
{
	FrListBox::ITEM_LIST::iterator it = m_pAuxPartCartList->m_itemList.begin();
	if (it != m_pAuxPartCartList->m_itemList.end())
	{
		do
		{
			IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
			if ((pData->TypeId & 0xfc000000) == 0x8000000 &&
				(mask & tids.GetOverlapped(pData->TypeId)) != 0)
			{
				UnselectItemFromItemList(pData->TypeId);
				it = DeleteItemFromAuxCartList(pData->TypeId);
				if (tids.IsEquipped(pData->TypeId, 0))
				{
					MsgObject msg(NULL, 0x18a, (int)&tids, (int)&pData->TypeId, 0, 0, 0);
					IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar");
					if (pActor)
						pActor << msg;
				}
				RestoreShoppersOldItem(pData->TypeId, tids);
			}
			else
				++it;
		} while (it != m_pAuxPartCartList->m_itemList.end());
	}
}

void CShopMain::PutOnNBackupShoppersOldItem(unsigned long tid,
	CPartTidList& tids, unsigned long uid)
{
	// 983 00a8be80: 0x18a ao "LobbyAvatar" (lista, &tid, &uid, &changed), sem o CExhibition/CPetFrame do 645
	// 1a 0x18a com param3 = &bool verdadeiro (00a8bed9, byte [ebp-0xcd]); cada MsgObject montada antes do GetActor
	CPartTidList backup = tids;
	if (tids.IsEquipped(tid, 0))
	{
		bool bTrue = true;
		MsgObject msg(NULL, 0x18a, (int)&tids, (int)&tid, (int)&bTrue, 0, 0);
		IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
		if (pAv)
			pAv << msg;
	}
	unsigned long changed = 0;
	{
		MsgObject msg(NULL, 0x18a, (int)&tids, (int)&tid, (int)&uid, (int)&changed, 0);
		IActor* pAv = AfxGetTask()->GetActor("LobbyAvatar");
		if (pAv)
			pAv << msg;
	}
	for (int i = 0; i < tids.m_partNum; i++, changed >>= 1)
	{
		if ((changed & 1) && (backup.m_tid[i] & 0x600) == 0 &&
			(backup.m_tid[i] & 0xfc000000) == 0x8000000)
		{
			oldItem_t item;
			item.tid = tid;
			item.oldTid = backup.m_tid[i];
			item.oldUid = backup.m_uid[i];
			m_oldItems.push_back(item);
		}
	}
}
