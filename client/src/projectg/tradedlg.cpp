#include "minatl.h"
#include "vc10_layout.h"
#include "tradedlg.h"
#include "frdesktop.h"
#include "frbutton.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frarea.h"
#include "projectg.h"
#include "ucclibrary.h"
#include "gatewayactor.h"
#include "chatmsg.h"
#include "iteminfodlg.h"
#include "taskmanager.h"
#include "itemstorage.h"
#include "binstr.h"
#include "commonutil.h"
#include "../shared/localize.h"

// 983: layouts medidos na sessao 82 (out/F3.5/s82/RELATORIO.md)
C_ASSERT(sizeof(sTradeItem) == 0xac && offsetof(sTradeItem, CharacterSlotNum) == 0x67 && offsetof(sTradeItem, ItemName) == 0x6d);
C_ASSERT(sizeof(sRoomSlot) == 0x2dc && offsetof(sRoomSlot, charInfo) == 0xd0 && offsetof(sRoomSlot, dwUserUID) == 0x2d4);
C_ASSERT(offsetof(sMyInfo, classicMapStat) - offsetof(sMyInfo, mapStat) == 22 * sizeof(sMapStatistics) && sizeof(sMyInfo) == 0x2f52);


// shareddoc.h inlines (declared there; defined here until shareddoc.h can carry them)
// HACK: in the original they are PCH inlines emitted right after WTL::CString::TrimRight; the dead reference
// makes them wait for it
inline const char* CSharedDoc::GetTradeTitle()
{
	if (0)
		((WTL::CString*)0)->TrimRight();
	return m_tradeTitle.c_str();
}

inline int CSharedDoc::GetCountVisitor()
{
	if (0)
		((WTL::CString*)0)->TrimRight();
	return m_countVisitor;
}

inline __int64 CSharedDoc::GetTradeIncome()
{
	if (0)
		((WTL::CString*)0)->TrimRight();
	return m_tradeIncome;
}

// HACK: std::list<sTradeItem>::begin()/end() const are emitted one wave late in the original (after the last
// function of the unit); explicit specializations, defined at the end of this file, reproduce that
#if _MSC_VER <= 1600 // toolset novo: begin/end do list sao noexcept, a especializacao nao declara
namespace std
{
	template <>
	list<sTradeItem>::const_iterator list<sTradeItem>::begin() const;
	template <>
	list<sTradeItem>::const_iterator list<sTradeItem>::end() const;
} // namespace std
#endif

extern Fresh* g_pFresh;

// <numeric> is not part of the toolchain copy; this is its VC7.1 accumulate.
namespace std
{
	template <class _InIt, class _Ty, class _Fn2>
	inline _Ty accumulate(_InIt _First, _InIt _Last, _Ty _Val, _Fn2 _Func)
	{
		for (; _First != _Last; ++_First)
			_Val = _Func(_Val, *_First);
		return (_Val);
	}
}

template <class T>
struct _TradeItemSort
{
	bool operator()(const T& a, const T& b) const
	{
		if ((a.dwTid >> 26) == 2 && (b.dwTid >> 26) != 2)
			return false;
		if ((a.dwTid >> 26) == 2 && (b.dwTid >> 26) == 2)
		{
			int maskA = a.dwTid & 0x3ffffff;
			int maskB = b.dwTid & 0x3ffffff;
			if (maskA < maskB)
				return true;
			if (maskA > maskB)
				return false;
		}
		if (a.dwTid < b.dwTid)
			return true;
		if (a.dwTid > b.dwTid)
			return false;
		return a.dwGuid < b.dwGuid;
	}
};

int __fastcall float2int(float);

void __fastcall DrawAbility(const sTradeItem& item, FrListItem* listItem)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	unsigned long tid = item.dwTid;
	struct sAbility
	{
		short v[5];
	};
	sAbility attr;
	sAbility slot = { 0 };
	unsigned long type = tid >> 26;
	bool draw = true;
	switch (type)
	{
	case 8:
	{
		IFF_STRUCT::sCadItem* cadItem = ItemManager()->FindCadItem(tid);
		if (!cadItem)
			return;
		attr = *(sAbility*)cadItem->COM;
		break;
	}
	case 7:
	{
		IFF_STRUCT::sCaddie* caddie = ItemManager()->FindCaddie(tid);
		if (!caddie)
			return;
		attr = *(sAbility*)caddie->Attr;
		break;
	}
	case 2:
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(tid);
		if (!part)
			return;
		attr = *(sAbility*)part->Attr;
		slot = *(sAbility*)part->Slot;
		break;
	}
	default:
		draw = false;
		break;
	}
	if (draw)
	{
		float y = 0.0f;
		const char* name = "";
		for (int i = 0, left = 5; left != 0; ++i, --left)
		{
			if (attr.v[i] != 0 || slot.v[i] != 0)
			{
				switch (i)
				{
				case 0:
					name = K2L_Compatibility("\xc6\xc4\xbf\xf6");
					break;
				case 1:
					name = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1");
					break;
				case 2:
					name = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5");
					break;
				case 3:
					name = K2L_Compatibility("\xbd\xba\xc7\xc9");
					break;
				case 4:
					name = K2L_Compatibility("\xc4\xbf\xba\xea");
					break;
				}
				if (attr.v[i] > 0 || slot.v[i] > 0)
				{
					gdi->SetTextColor(0xff0000ff, 0xffffffff);
					const char* text;
					if (attr.v[i] == 0)
						text = MakeStr(K2L_Compatibility("%s\xbd\xbd\xb7\xd4%+d"), name, // 983 008f2ade
							(int)slot.v[i]);
					else if (slot.v[i] == 0)
						text = MakeStr("%s%+d", name, (int)attr.v[i]);
					else
						text = MakeStr(K2L_Compatibility("%s%+d/\xbd\xbd\xb7\xd4%+d"), name, // 983 008f2b29
							(int)attr.v[i], (int)slot.v[i]);
					gdi->Print(listItem->pos + WPoint(182.0f, y + 36.0f), 2,
						text);
					y += 14.0f;
				}
			}
		}
		return;
	}
	if (type == 4)
	{
		gdi->SetTextColor(0xff009000, 0xffffffff);
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 36.0f), 2,
			K2L_Compatibility("\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xc7\xd1 \xba\xf1\xbf\xeb:")); // 983 008f0f04
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 50.0f), 2,
			MakeStr("%d\xc6\xce", item.dwUpgradeCost));
	}
}


void __fastcall SetTextColorByPrice(__int64 price)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		if (price > 1000000) // 983 008ee800
		{
			device->SetTextColor(0xfff50018, 0xffffffff);
		}
		else if (price > 100000)
		{
			device->SetTextColor(0xffd5009b, 0xffffffff);
		}
		else if (price > 10000)
		{
			device->SetTextColor(0xffc79500, 0xffffffff);
		}
		else if (price > 1000)
		{
			device->SetTextColor(0xff0e0e0e, 0xffffffff);
		}
		else
		{
			device->SetTextColor(0xff626262, 0xffffffff);
		}
	}
}

const char* __fastcall MakeStatusString(int type, int level, int slot)
{
	// HACK: dead reference to a late PCH inline; defers this function one wave (emission order).
	if (0)
		g_pFresh->GetManager();
	// power, control, accuracy, spin, curve
	const char* names[5] = { K2L_Compatibility("\xc6\xc4\xbf\xf6"), K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1"),
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5"), K2L_Compatibility("\xbd\xba\xc7\xc9"), K2L_Compatibility("\xc4\xbf\xba\xea") };
	// 983 (008edcb0): "slot" traduzido uma vez, antes dos testes
	const char* slotName = K2L_Compatibility("\xbd\xbd\xb7\xd4");
	if (level)
	{
		if (slot)
		{
			return MakeStr("%s+%d/%s+%d", names[type], level, slotName, slot);
		}
		return MakeStr("%s+%d", names[type], level);
	}
	if (slot)
	{
		return MakeStr("%s%s+%d", names[type], slotName, slot);
	}
	return "";
}

const char* __fastcall MakeCardStatusString(int type, int level)
{
	// HACK: dead reference to a late PCH inline; defers this function one wave (emission order).
	if (0)
		g_pFresh->GetManager();
	const char* names[5] = { K2L_Compatibility("\xc6\xc4\xbf\xf6\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xbd\xba\xc7\xc9\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xbf\xba\xea\xbd\xbd\xb7\xd4") };
	if (level)
	{
		return MakeStr("%s+%d", names[type], level);
	}
	return "";
}

IMPLEMENT_OBJECT(FrTradeDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeDlg, FrForm)

ON_FRESH_VI("tradedlg", FRCMD_INIT, FrTradeDlg::OnTradeDlgInit)
ON_FRESH_VI("visitor_back", FRCMD_INIT, FrTradeDlg::OnVisitorBackInit)
ON_FRESH_VV("visitor_back", FRCMD_OWNERDRAW, FrTradeDlg::OnVisitorBackOwnerDraw)
ON_FRESH_VI("mypang", FRCMD_INIT, FrTradeDlg::OnMyPangInit)
ON_FRESH_VV("mypang", FRCMD_OWNERDRAW, FrTradeDlg::OnMyPangOwnerDraw)
ON_FRESH_VI("trade_item_list", FRCMD_INIT, FrTradeDlg::OnTradeItemListInit)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONDOWN,
	FrTradeDlg::OnTradeItemListLClick)
ON_FRESH_VV("trade_item_list", FRCMD_RBUTTONDOWN,
	FrTradeDlg::OnTradeItemListRClick)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	FrTradeDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("noitem01", FRCMD_INIT, FrTradeDlg::OnNoItem01Init)
ON_FRESH_VI("noitem02", FRCMD_INIT, FrTradeDlg::OnNoItem02Init)
ON_FRESH_VI("visitor", FRCMD_INIT, FrTradeDlg::OnVisitorInit)
ON_FRESH_VI("userid", FRCMD_INIT, FrTradeDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, FrTradeDlg::OnTitleInit)
ON_FRESH_VI("edit", FRCMD_INIT, FrTradeDlg::OnEditBnInit)
ON_FRESH_VV("edit", FRCMD_LBUTTONUP, FrTradeDlg::OnEditBnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrTradeDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrTradeDlg::OnCloseBnUp)
ON_FRESH_VI("hide", FRCMD_INIT, FrTradeDlg::OnHideBnInit)
ON_FRESH_VV("hide", FRCMD_LBUTTONUP, FrTradeDlg::OnHideBnUp)
ON_FRESH_VI("exit", FRCMD_INIT, FrTradeDlg::OnExitBnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, FrTradeDlg::OnExitBnUp)

END_FRESH_MSGMAP()

FrTradeDlg::FrTradeDlg()
{
	m_pVisitorBack = NULL;
	m_pTradeItemList = NULL;
	m_pNoItem01 = NULL;
	m_pNoItem02 = NULL;
	m_pVisitor = NULL;
	m_pUserId = NULL;
	m_pTitle = NULL;
	m_pEditBn = NULL;
	m_pCloseBn = NULL;
	m_pHideBn = NULL;
	m_pExitBn = NULL;
	m_pItemBase[0] = g_pFresh->GetBitmap("item_base_n");
	m_pItemBase[1] = g_pFresh->GetBitmap("item_base_o");
	m_pItemBase[2] = g_pFresh->GetBitmap("item_base_d");
	m_pItemBase[3] = g_pFresh->GetBitmap("item_base_empty");
	m_pUccState[0] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccState[1] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccState[2] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	static char* s_fontWind[] = { "[font_wind.jpg" };
	WTITLEFONT timeFont;
	memset(&timeFont, 0, sizeof(timeFont));
	timeFont.filename = s_fontWind;
	timeFont.fontw = 16;
	timeFont.fonth = 16;
	timeFont.numPages = 1;
	timeFont.texw = 128;
	timeFont.texh = 64;
	timeFont.pCharSet = "1234567890ym%-.?/:,";
	m_pFontTime = g_resrcmng->GetTitleFont();
	m_pFontTime->Create(&timeFont);
	static char* s_fontBong[] = { "[font_bong.jpg" };
	WTITLEFONT priceFont;
	memset(&priceFont, 0, sizeof(priceFont));
	priceFont.filename = s_fontBong;
	priceFont.fontw = 9;
	priceFont.fonth = 16;
	priceFont.numPages = 1;
	priceFont.texw = 128;
	priceFont.texh = 32;
	priceFont.pCharSet = "1234567890,-";
	m_pFontPrice = g_resrcmng->GetTitleFont();
	m_pFontPrice->Create(&priceFont);
	m_bOwner = false;
	m_tradeUID = -1;
	memset(m_unused168, 0, sizeof(m_unused168));
	m_bSendingServer = false;
	m_unused18c = 0.0f;
	m_buyBtn[0].pBitmap = g_pFresh->GetBitmap("buy_btn_n");
	m_buyBtn[0].rect = WRect(144.0f, 66.0f, m_buyBtn[0].pBitmap->Width(),
		m_buyBtn[0].pBitmap->Height());
	m_buyBtn[0].index = -1;
	m_buyBtn[1].pBitmap = g_pFresh->GetBitmap("buy_btn_o");
	m_buyBtn[1].rect = WRect(144.0f, 66.0f, m_buyBtn[1].pBitmap->Width(),
		m_buyBtn[1].pBitmap->Height());
	m_buyBtn[1].index = -1;
	m_clockIcon.pBitmap = g_pFresh->GetBitmap("clock");
	m_clockIcon.rect = WRect(50.0f, 35.0f, m_clockIcon.pBitmap->Width(),
		m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetBitmap("delete");
	m_deleteIcon.rect = WRect(50.0f, 35.0f, m_deleteIcon.pBitmap->Width(),
		m_deleteIcon.pBitmap->Height());
}

// 983: o estado do lobby (singleton 00e78ca8) e da frente B (include/jp/lobbyjp.h, CLobbyState983); aqui so o que o
// destrutor usa. 005fa910 recebe o nome do botao por valor e repassa a 005fa340 (nome do metodo e meu).
class CLobbyState983a
{
public:
	void EnableButton983(std::wstring name, int enable); // 005fa910
	static CLobbyState983a* ms_pInstance; // 00e78ca8
};

FrTradeDlg::~FrTradeDlg()
{
	if (m_pFontPrice)
		m_pFontPrice->Erase();
	if (m_pFontTime)
		m_pFontTime->Erase();
	if (g_resrcmng)
	{
		if (m_pFontPrice)
		{
			g_resrcmng->Release(m_pFontPrice);
			m_pFontPrice = NULL;
		}
		if (g_resrcmng && m_pFontTime)
		{
			g_resrcmng->Release(m_pFontTime);
			m_pFontTime = NULL;
		}
	}
	// 983 (008f7ff0): quem visita so avisa a saida com tarefa e documento vivos; fechada a loja (dono ou visita que
	// avisou), o botao "btn_exit" do lobby volta a valer (objeto de 00e78ca8, 005fa910).
	if (!m_bOwner)
	{
		if (!CTaskManager::Instance() ||
			!CTaskManager::Instance()->GetCurrentTask() ||
			!CSharedDoc::IsInstantiated())
			return;
		Doc()->SendTradeExitShop(m_tradeUID);
	}
	if (CLobbyState983a::ms_pInstance)
		CLobbyState983a::ms_pInstance->EnableButton983(L"btn_exit", 1);
}

void FrTradeDlg::OnTradeDlgInit(int param)
{
}

void FrTradeDlg::OnVisitorBackInit(int param)
{
	m_pVisitorBack = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTradeDlg::OnVisitorBackOwnerDraw()
{
}

void FrTradeDlg::OnMyPangInit(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTradeDlg::OnMyPangOwnerDraw()
{
	DrawPang();
}

void FrTradeDlg::OnTradeItemListInit(int param)
{
	m_pTradeItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pTradeItemList)
		m_pTradeItemList->UseRightButton(true);
}

void FrTradeDlg::OnTradeItemListLClick()
{
	FrListItem* listItem = m_pTradeItemList->GetItemUnderCursor();
	if (!listItem)
		return;

	if (!listItem->selected)
		m_pTradeItemList->ToggleItem(listItem);

	if (!m_bOwner && !m_bSendingServer)
	{
		sTradeItem* item = (sTradeItem*)listItem->pData;
		Doc()->m_itemManager.FindCommonItem(item->dwTid);
		if ((item->wFlagTradeLimit & 3) == 0) // 983: bits 1|2, como o botao Comprar
		{
			WRect rc(listItem->pos.x + m_buyBtn[0].rect.x,
				listItem->pos.y + m_buyBtn[0].rect.y, m_buyBtn[0].rect.w,
				m_buyBtn[0].rect.h);
			if (rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				m_buyBtn[0].index = listItem->idx;
				item = (sTradeItem*)listItem->pData;
				if (item->i64Price > Doc()->m_myInfo.stat.i64Pang)
				{
					SetTradeResultCode(0x17, 0xe3, NULL);
					return;
				}
				if (Doc()->m_myInfo.stat.Level == 0)
				{
					SetTradeResultCode(0x19, 0xe3, NULL);
					return;
				}
				FrTradeBuyDlg* buyDlg = CreateForm<FrTradeBuyDlg>(
					g_pFresh->GetManager(), this, "tradebuydlg", NULL);
				if (buyDlg)
				{
					buyDlg->Open(
						(FRESH_PFN_RESULT)&FrTradeDlg::OnTradeBuyDlgResult,
						0x41); // 983 008f6a9d
					buyDlg->SetFadeout(false);
					buyDlg->SetBuyItem(item);
					buyDlg->SetControlsBuyItem(item);
				}
			}
		}
	}
}

void FrTradeDlg::OnTradeItemListRClick()
{
	FrListItem* listItem = m_pTradeItemList->GetItemUnderCursor();
	if (!listItem)
		return;

	sTradeItem* item = (sTradeItem*)listItem->pData;
	if (item)
	{
		FrItemInfoDlg* info = CreateForm<FrItemInfoDlg>(g_pFresh->GetManager(),
			this, "iteminfo", NULL);
		if (info)
		{
			info->Open(NULL, 3);
			info->SetItemInfo(item);
			if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/) &&
				(item->dwTid >> 26) == 2)
			{
				FrEdit* edit =
					DYNAMIC_CAST(FrEdit, info->FindChildByName("avility"));
				if (edit)
				{
					IFF_STRUCT::sPart* part =
						Doc()->m_itemManager.FindPart(item->dwTid);
					if (part)
					{
						for (int i = 0; i < 5; i++)
						{
							if (part->Attr[i] != 0 || part->Slot[i] != 0)
							{
								bool b1 = true;
								bool b2 = false;
								const char* status = MakeStatusString(i,
									part->Attr[i], part->Slot[i]);
								edit->AddText(
									MakeStr(
										K2L_Compatibility("\\c0xff0000ff\\c%s\\c0xff000000\\c"),
										status),
									b2, b1);
							}
						}
					}
					unsigned long* attachCard = &item->attachCardTid[0][0];
					for (int n = 8; n != 0; n--)
					{
						unsigned long tid = *attachCard;
						if (tid != 0)
						{
							unsigned long type = (tid >> 22) & 0xf;
							if (type == 0)
							{
								IFF_STRUCT::sCard* card =
									Doc()->m_itemManager.FindCard(tid);
								if (card)
								{
									for (int i = 0; i < 5; i++)
									{
										if (card->COM[i] > 0)
										{
											bool b1 = true;
											bool b2 = false;
											const char* status =
												MakeCardStatusString(i,
													card->COM[i]);
											edit->AddText(
												MakeStr(
													K2L_Compatibility("\\c0xff60ee0e\\c%s\\c0xff000000\\c"),
													status),
												b2, b1);
										}
									}
								}
							}
							else if (type == 1)
							{
								IFF_STRUCT::sDesc* desc =
									Doc()->m_itemManager.FindDesc(tid);
								if (desc)
								{
									char text[512];
									const char* src = desc->Desc;
									char c;
									int n2 = 0;
									do
									{
										c = *src;
										text[n2++] = c;
										src++;
									} while (c != 0);
									for (int i = 0; i < 512; i++)
									{
										if (text[i] == '\n')
										{
											text[i] = 0;
											break;
										}
									}
									edit->AddText(
										MakeStr(
											K2L_Compatibility("\\c0xffff0000\\c%s\\c0xff000000\\c"),
											text),
										false, true);
								}
							}
						}
						attachCard++;
					}
				}
			}
		}
	}
	if (!listItem->selected)
		m_pTradeItemList->ToggleItem(listItem);
}

// HACK: the original reads g_pFresh->manager->m_istate.mousePos directly (no inline accessors: GetManager()/
// GetMousePos() shift the scratch registers of the buy button block); m_istate is protected.
struct RbWndManager : FrWndManager
{
	using FrWndManager::m_istate;
};

void FrTradeDlg::OnTradeItemListOwnerDraw(int param)
{
	FrListItem* listItem = (FrListItem*)param;
	if (!listItem)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	// 983 008f16d0: para quem compra, aviso em vermelho na posicao da janela (+200, +22/+37)
	if (!m_bOwner)
	{
		float x = GetRect().x;
		float y = GetRect().y;
		gdi->SetTextColor(0xffff0000, 0xffffffff);
		float px = x + 200.0;
		gdi->Print(WPoint(px, y + 22.0), 0,
			K2L_Compatibility("\xc1\xa6""3\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb \xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xc0\xd6\xb4\xc2"));
		gdi->Print(WPoint(px, y + 37.0), 0,
			K2L_Compatibility("\xb0\xd4\xc0\xd3\xb3\xbb \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xb8\xc5\xb8\xc5\xb4\xc2 \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
	}
	if (!listItem->pData)
		return;
	const Bitmap* base = m_pItemBase[0];
	unsigned long color = 0xffffffff;
	gdi->SetTextColor(0xff000000, 0xffffffff);
	sTradeItem item = *(sTradeItem*)listItem->pData;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(item.dwTid);
	if (!common)
		return;
	// 983: bit 2 do flag (alem do 1) tambem deixa o item cinza
	if (!m_bOwner)
	{
		if (item.wFlagTradeLimit & 1)
			color = 0xff909090;
		if ((item.wFlagTradeLimit & 1) || (item.wFlagTradeLimit & 2))
			color = 0xff909090;
	}
	unsigned long baseColor = color;
	gdi->SetTextStyle(0);
	if (base)
	{
		WRect rect(listItem->pos.x, listItem->pos.y, (float)base->Width(),
			(float)base->Height());
		gdi->DrawTexture(base, rect, baseColor, 0);
	}

	unsigned long type = item.dwTid >> 26;
	if (type == 2)
	{
		IFF_STRUCT::sChar* chara =
			ItemManager()->FindChar(((item.dwTid >> 18) & 0xff) | 0x4000000);
		if (chara)
		{
			const Bitmap* icon =
				g_pFresh->GetBitmap(MakeStr("s_%s", chara->c.Icon));
			if (icon)
			{
				WRect rect(listItem->pos.x + 3.0f, listItem->pos.y + 3.0f,
					(float)icon->Width(), (float)icon->Height());
				gdi->DrawTexture(icon, rect, 0xffffffff, 0);
			}
		}
	}

	const Bitmap* bitmap = g_pFresh->GetBitmap(common->Icon);
	if (bitmap)
	{
		float offX;
		float offY;
		switch (type)
		{
		default:
			offX = 16.0f;
			offY = 27.0f;
			break;
		case 2:
			offX = 7.0f;
			offY = 2.0f;
			break;
		}
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item.status & 1) &&
			IsUccClothes(item.dwTid))
			bitmap = GetUccItemIcon(item.dwGuid);
		if (bitmap)
		{
			WRect rect(offX + listItem->pos.x, offY + listItem->pos.y,
				(float)bitmap->Width(), (float)bitmap->Height());
			gdi->DrawTexture(bitmap, rect, color, 0);
		}
	}

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && IsUccClothes(item.dwTid))
	{
		if ((item.status & 1) && item.Seq == 1 && m_pUccState[0])
		{
			gdi->DrawTexture(m_pUccState[0],
				WRect(listItem->pos.x + 50.0f, listItem->pos.y + 52.0f,
					(float)m_pUccState[0]->Width(),
					(float)m_pUccState[0]->Height()),
				0xffffffff, 0);
		}
		else if ((item.status & 1) && item.Seq > 1 && m_pUccState[1])
		{
			gdi->DrawTexture(m_pUccState[1],
				WRect(listItem->pos.x + 50.0f, listItem->pos.y + 52.0f,
					(float)m_pUccState[1]->Width(),
					(float)m_pUccState[1]->Height()),
				0xffffffff, 0);
		}
		else if ((item.status & 2) && m_pUccState[2])
		{
			gdi->DrawTexture(m_pUccState[2],
				WRect(listItem->pos.x + 50.0f, listItem->pos.y + 52.0f,
					(float)m_pUccState[2]->Width(),
					(float)m_pUccState[2]->Height()),
				0xffffffff, 0);
		}
	}

	const char* name;
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && IsUccClothes(item.dwTid))
		name = GetUccItemName(item.dwGuid);
	else
		name = common->Name;
	gdi->SetTextStyle(1);
	WPoint& pos = listItem->pos;
	gdi->Print(WPoint(listItem->pos.x + 182.0f, pos.y + 7.0f), 2, name);
	gdi->SetTextStyle(0);

	char buf[32] = "";
	MakePriceStrA(buf, item.i64Price);
	SetTextColorByPrice(item.i64Price);
	int count = item.iNum;
	if (count > 1)
	{
		const char* text = MakeStr("%d", count);
		float x = pos.x + 54.0f;
		for (count = 0; count < strlen(text); ++count)
			x -= m_pFontTime->GetCharWidth(g_view, text[count] - '0');
		m_pFontTime->Print(g_view, x, listItem->pos.y + 55.0f, text, 0,
			0xffffffff, NULL);
		gdi->Print(WPoint(pos.x + 182.0f, pos.y + 23.0f), 2,
			MakeStr(K2L_Compatibility("\xb0\xb3\xb4\xe7 %s\xc6\xce"), buf));
	}
	else
	{
		gdi->Print(WPoint(pos.x + 182.0f, pos.y + 23.0f), 2,
			MakeStr(K2L_Compatibility("%s\xc6\xce"), buf));
	}
	gdi->SetTextColor(0xff000000, 0xffffffff);
	DrawAbility(item, listItem);

	if (!m_bOwner && !(*(unsigned char*)&item.wFlagTradeLimit & 3))
	{
		if (listItem->underCursor &&
			(m_buyBtn[0].rect + pos)
				.IsInRect(((const RbWndManager*)g_pFresh->manager)
						->m_istate.mousePos))
			gdi->DrawTexture(m_buyBtn[1].pBitmap, m_buyBtn[1].rect + pos,
				0xffffffff, 0);
		else
			gdi->DrawTexture(m_buyBtn[0].pBitmap, m_buyBtn[0].rect + pos,
				0xffffffff, 0);
	}

	switch (item.dwTid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 16:
	case 28:
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (common->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(pos.x + 40.0f -
					(float)float2int((float)level->Width() * 0.5f),
				listItem->pos.y + 51.0f -
					(float)float2int((float)level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, color, 0);
		}
		break;
	}
}

void FrTradeDlg::OnNoItem01Init(int param)
{
	m_pNoItem01 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pNoItem01)
		m_pNoItem01->SetVisible(false);
}

void FrTradeDlg::OnNoItem02Init(int param)
{
	m_pNoItem02 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pNoItem02)
		m_pNoItem02->SetVisible(false);
}

void FrTradeDlg::OnVisitorInit(int param)
{
	m_pVisitor = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	UpdateCountVisitor();
}

void FrTradeDlg::OnUserIdInit(int param)
{
	m_pUserId = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	UpdateTradeTitle();
}

void FrTradeDlg::OnEditBnInit(int param)
{
	m_pEditBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008f8120): sem a mensagem ao gateway; a janela de edicao sobe 25 pontos e abre com 0xc1
void FrTradeDlg::OnEditBnUp()
{
	FrTradeEditDlg* editDlg = CreateForm<FrTradeEditDlg>(g_pFresh->GetManager(),
		this, "tradeeditdlg", NULL);
	if (editDlg)
	{
		WRect rect = editDlg->GetRect();
		rect.y -= 25.0;
		editDlg->SetRect(rect);
		editDlg->Open(NULL, 0xc1);
		editDlg->SetFadeout(false);
		Doc()->SendTradeEditShop();
		OnFreshOkay();
	}
}

void FrTradeDlg::OnCloseBnInit(int param)
{
	m_pCloseBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008ede30): chama o virtual OnCancel (posicao 34), que ficou com o corpo
void FrTradeDlg::OnCloseBnUp()
{
	OnCancel();
}

// 983 (008f6af0): virtual novo; codigos 0x14/0x15 e tipo 0xe3 (no 645: 0x12/0x13 e 0xe1, dentro do OnCloseBnUp)
void FrTradeDlg::OnCancel()
{
	int state = Doc()->m_stateTrade;
	if (state == STATE_TRADE_OUTOFSTOCK)
		SetTradeResultCode(state + 0x12, 0xe3, NULL);
	else if (state == STATE_TRADE_ENTER)
		OnExitBnUp();
	else
		SetTradeResultCode(0x14, 0xe3, NULL);
}

void FrTradeDlg::OnHideBnInit(int param)
{
	m_pHideBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008ede40): so fecha (sem a mensagem 0x273 ao gateway)
void FrTradeDlg::OnHideBnUp()
{
	OnFreshOkay();
}

void FrTradeDlg::OnExitBnInit(int param)
{
	m_pExitBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008ede50): so fecha
void FrTradeDlg::OnExitBnUp()
{
	OnFreshOkay();
}

bool FrTradeDlg::OnInit()
{
	EnableDrag(false);
	if (Doc()->m_stateTrade == STATE_TRADE_OPEN ||
		Doc()->m_stateTrade == STATE_TRADE_OUTOFSTOCK ||
		Doc()->m_stateTrade == STATE_TRADE_ENTER)
	{
		// 983 (008f1fb0): pelas duas auxiliares, fora de linha
		if (Doc()->m_listTradeItem.size() <= 0)
		{
			ShowStaticOutOfStock(true);
			EnableHideBn(false);
		}
		else
		{
			ShowStaticOutOfStock(false);
			EnableHideBn(true);
		}
	}
	return true;
}

static const char s_thanksForVisiting[] =
	"\xc0\xcc\xbf\xeb\xc7\xd8 \xc1\xd6\xbc\xc5\xbc\xad \xb0\xa8\xbb\xe7\xc7\xd5\xb4\xcf\xb4\xd9.";

void FrTradeDlg::OpenTradeShop(unsigned long uid)
{
	if (uid == (unsigned long)-1)
	{
		OnFreshOkay();
		return;
	}

	m_tradeUID = uid;
	if (uid == Doc()->m_myInfo.info.dwUID)
	{
		m_bOwner = true;
		SetListTradeItem(false);
		if (m_pVisitorBack)
			m_pVisitorBack->SetVisible(true);
		if (m_pVisitor)
			m_pVisitor->SetVisible(true);
		if (m_pEditBn)
			m_pEditBn->SetVisible(true);
		if (m_pCloseBn)
			m_pCloseBn->SetVisible(true);
		if (m_pHideBn)
			m_pHideBn->SetVisible(true);
		if (m_pExitBn)
			m_pExitBn->SetVisible(false);
	}
	else
	{
		m_bOwner = false;
		if (m_pVisitorBack)
			m_pVisitorBack->SetVisible(false);
		if (m_pVisitor)
			m_pVisitor->SetVisible(false);
		if (m_pEditBn)
			m_pEditBn->SetVisible(false);
		if (m_pCloseBn)
			m_pCloseBn->SetVisible(false);
		if (m_pHideBn)
			m_pHideBn->SetVisible(false);
		if (m_pExitBn)
			m_pExitBn->SetVisible(true);
	}

	if (m_pUserId)
	{
		char buf[64] = "";
		sprintf(buf, K2L_Compatibility("\xb3\xaa\xc0\xc7 \xbb\xf3\xc1\xa1"));
		if (!m_bOwner)
		{
			std::list<sSlotInfo>::iterator it;
			for (it = Doc()->m_slotList.begin(); it != Doc()->m_slotList.end();
				it++)
			{
				if ((*it).dwUserUID == Doc()->m_tradeUID)
				{
					sprintf(buf,
						MakeStr(K2L_Compatibility("%s \xb4\xd4\xc0\xc7 \xbb\xf3\xc1\xa1"),
							(*it).sNick));
					break;
				}
			}
		}
		m_pUserId->SetLine(1, buf, 0, false, 0);
	}

	if (!m_bOwner)
	{
		m_pNoItem01->SetCaption(
			K2L_Compatibility("\xb8\xf0\xb5\xe7 \xbb\xf3\xc7\xb0\xc0\xcc "
			"\xc7\xb0\xc0\xfd\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf"
			"\xb4\xd9."));
		m_pNoItem02->SetCaption(K2L_Compatibility(s_thanksForVisiting));
	}
}

void FrTradeDlg::SetListTradeItem(bool bEmpty)
{
	if (!m_pTradeItemList)
		return;

	m_pTradeItemList->ClearItem();
	if (!bEmpty)
	{
		std::list<sTradeItem>& tradeList = Doc()->m_listTradeItem;
		for (std::list<sTradeItem>::iterator it = tradeList.begin();
			it != tradeList.end(); ++it)
		{
			IFF_ITEM_COMMON* common =
				Doc()->m_itemManager.FindCommonItem((*it).dwTid);
			if (common)
			{
				(*it).wFlagTradeLimit = 0;
				if (common->Level > Doc()->m_myInfo.stat.Level)
					(*it).wFlagTradeLimit |= 1;
				// 983 008f5ea0: peca que ja tenho tambem fica bloqueada (bit 2)
				if (Doc()->IsOverlapPart((*it).dwTid))
					(*it).wFlagTradeLimit |= 2;
				m_pTradeItemList->AddItem(&*it);
			}
		}
	}
}

void FrTradeDlg::DrawPang()
{
	if (!g_pFresh->GetManager()->GetGDI())
		return;
	if (!m_pFontPrice)
		return;

	char buf[32] = "";
	WRect rect = GetRect();
	static int s_fontWidth = m_pFontPrice->GetFontInfo()->fontw;
	static float s_width;
	if (m_bOwner)
		MakePriceStrA(buf, Doc()->GetTradeIncome());
	else
		MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
	s_width = (s_fontWidth - 1.0f) * (strlen(buf) - 1);
	m_pFontPrice->Print(g_view, rect.x + 130.0f - s_width, rect.y + 380.0f, buf,
		0, 0xffffffff, NULL);
}

void FrTradeDlg::UpdateTradeTitle()
{
	if (m_pTitle)
	{
		m_pTitle->ClearLine();
		const char* title = Doc()->GetTradeTitle();
		m_pTitle->SetLine(1, title, 0, false, 0);
	}
}

void FrTradeDlg::UpdateCountVisitor()
{
	if (m_pVisitor)
	{
		char buf[32] = "";
		MakePriceStrA(buf, Doc()->GetCountVisitor());
		m_pVisitor->SetLine(1, buf, 0, false, 0);
	}
}

bool FrTradeDlg::OnTradeBuyDlgResult(int result, FrForm* form)
{
	if (result == 1)
		EnableSendingServer(true);
	return true;
}

void FrTradeDlg::EnableSendingServer(bool enable)
{
	m_bSendingServer = enable;
	if (m_pExitBn)
		m_pExitBn->Enable(!enable);
}

void FrTradeDlg::ShowStaticOutOfStock(bool show)
{
	if (m_pNoItem01)
		m_pNoItem01->SetVisible(show);
	if (m_pNoItem02)
		m_pNoItem02->SetVisible(show);
}

void FrTradeDlg::EnableHideBn(bool enable)
{
	if (m_pHideBn)
		m_pHideBn->Enable(enable);
}

IMPLEMENT_OBJECT(FrTradeBuyDlg, FrForm)

// 983 (s84): tamanhos pelo push antes do operator new de cada MakeInstance (008f4f20, 008f21e0, 008f2cd0, 008f8460, 008f5a50)
C_ASSERT_VC10(sizeof(FrTradeDlg) == 0x210);
C_ASSERT_VC10(sizeof(FrTradeBuyDlg) == 0x200);
C_ASSERT_VC10(sizeof(FrTradeResultDlg) == 0x1e0);
C_ASSERT_VC10(sizeof(FrTradeEditDlg) == 0x280);
C_ASSERT_VC10(sizeof(FrTradeFinishEditDlg) == 0x1a0);

// HACK: instantiates CreateForm<FrTradeFinishEditDlg> before CreateForm<FrTradeWarningDlg> (original COMDAT order)
typedef char rb_createform_order[sizeof(
	CreateForm<FrTradeFinishEditDlg>(NULL, NULL, NULL, NULL))];

BEGIN_FRESH_MSGMAP(FrTradeBuyDlg, FrForm)

ON_FRESH_VI("tradebuydlg", FRCMD_INIT, FrTradeBuyDlg::OnTradeBuyDlgInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrTradeBuyDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrTradeBuyDlg::OnOkBnUp)
ON_FRESH_VI("buyitem_icon", FRCMD_INIT, FrTradeBuyDlg::OnBuyItemIconInit)
ON_FRESH_VV("buyitem_icon", FRCMD_OWNERDRAW,
	FrTradeBuyDlg::OnBuyItemIconOwnerDraw)
ON_FRESH_VI("buyitem_name", FRCMD_INIT, FrTradeBuyDlg::OnBuyItemNameInit)
ON_FRESH_VI("buyitem_ability", FRCMD_INIT, FrTradeBuyDlg::OnBuyItemAbilityInit)
ON_FRESH_VI("my_pang", FRCMD_INIT, FrTradeBuyDlg::OnMyPangInit)
ON_FRESH_VI("price", FRCMD_INIT, FrTradeBuyDlg::OnPriceInit)
ON_FRESH_VI("pang_left", FRCMD_INIT, FrTradeBuyDlg::OnPangLeftInit)
ON_FRESH_VI("num", FRCMD_INIT, FrTradeBuyDlg::OnNumberInit)
ON_FRESH_VI("num_next", FRCMD_INIT, FrTradeBuyDlg::OnNumberNextBnInit)
ON_FRESH_VV("num_next", FRCMD_LBUTTONUP, FrTradeBuyDlg::OnNumberNextBnUp)
ON_FRESH_VI("num_prev", FRCMD_INIT, FrTradeBuyDlg::OnNumberPrevBnInit)
ON_FRESH_VV("num_prev", FRCMD_LBUTTONUP, FrTradeBuyDlg::OnNumberPrevBnUp)

END_FRESH_MSGMAP()

FrTradeBuyDlg::FrTradeBuyDlg()
{
	// HACK: dead reference to a late PCH inline; defers this function one wave (emission order).
	if (0)
		g_pFresh->GetManager();
	m_pBuyItemIcon = NULL;
	m_pBuyItemName = NULL;
	m_pBuyItemAbility = NULL;
	m_pMyPang = NULL;
	m_pPrice = NULL;
	m_pPangLeft = NULL;
	m_pNumberNext = NULL;
	m_pNumberPrev = NULL;
	m_pNumber = NULL;
	memset(&m_buyItem, 0, sizeof(m_buyItem));
	m_pBuyItem = NULL;
	m_bClosing = false;
	static char* s_fontWind[] = { "[font_wind.jpg" };
	WTITLEFONT font;
	memset(&font, 0, sizeof(font));
	font.filename = s_fontWind;
	font.fontw = 16;
	font.fonth = 16;
	font.numPages = 1;
	font.texw = 128;
	font.texh = 64;
	font.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&font);
	m_pUccState[0] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccState[1] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccState[2] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	m_count = 0;
}

FrTradeBuyDlg::~FrTradeBuyDlg()
{
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrTradeBuyDlg::OnTradeBuyDlgInit(int param)
{
}

void FrTradeBuyDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeBuyDlg::OnOkBnUp()
{
	bool bOk = true;
	if (!m_pBuyItem || !m_pNumber)
	{
		SetTradeResultCode(0, 0x7d, NULL);
		OnFreshCancel();
		return;
	}

	m_buyItem = *m_pBuyItem;
	std::list<sTradeItem>& items = Doc()->m_listTradeItem;
	std::list<sTradeItem>::iterator it;
	for (it = items.begin(); it != items.end(); it++)
	{
		if ((*it).iIndex == m_buyItem.iIndex)
			break;
	}
	if (it == items.end())
	{
		SetTradeResultCode(0xe, 0xe3, NULL);
		OnFreshCancel();
		return;
	}

	m_buyItem.iNum = atoi(m_pNumber->GetLine(1, false));
	if (m_buyItem.iNum >= m_pBuyItem->iNum)
		m_buyItem.iNum = m_pBuyItem->iNum;
	if (m_buyItem.iNum <= 1)
		m_buyItem.iNum = 1;
	if (m_buyItem.i64Price * m_buyItem.iNum > Doc()->m_myInfo.stat.i64Pang)
	{
		SetTradeResultCode(0x17, 0xe3, NULL);
		bOk = false;
	}

	if (!Doc()->ConfirmInsertItem(m_buyItem.dwTid, m_buyItem.iNum))
	{
		FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify", NULL);
		WRect rect = form->GetRect();
		form->SetRect(rect);
		form->SetTimeLimit(12.0f);
		form->SetMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 "
			"\xba\xb8\xc0\xaf\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3"
			"\xc0\xcc\xb9\xc7\xb7\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb "
			"\xb1\xb8\xc0\xd4 \xb6\xc7\xb4\xc2 \xbf\xc5\xb1\xe6 \xbc\xf6 "
			"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		form->Open(NULL, 0);
		OnFreshCancel();
		return;
	}

	if (!bOk)
	{
		OnFreshCancel();
		return;
	}

	FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
		"notify_okcancel", NULL);
	form->SetMessage(K2L_Compatibility("\\c0xffff0000\\c\xc1\xa4\xb8\xbb \xb1\xb8\xc0\xd4\xc7\xcf"
					 "\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	form->Open((FRESH_PFN_RESULT)&FrTradeBuyDlg::OnConfirmResult, 3);
}

bool __fastcall CheckTradeItem(unsigned long uid, const sTradeItem& item)
{
	if (Doc()->m_stateTrade == STATE_TRADE_ENTER && uid != 0 &&
		uid != (unsigned long)-1 && item.dwGuid != 0 && item.dwTid != 0 &&
		item.i64Price != 0)
		return (bool)(item.iNum > 0);
	return false;
}

bool FrTradeBuyDlg::OnConfirmResult(int result, FrForm* form)
{
	if (result != FrOK || !form)
		return true;

	// 983 (008f6b30): uid lido antes da copia do item; Close em cada ramo
	CSharedDoc* doc = Doc();
	unsigned long uid = doc->m_tradeUID;
	if (!doc->SendTradeBuyItem(uid, m_buyItem))
	{
		SetTradeResultCode(0, 0x7d, NULL);
		SetClosing(true);
		Close(FrCANCEL, false);
	}
	else
	{
		SetClosing(true);
		Close(FrOK, false);
	}
	return true;
}

void FrTradeBuyDlg::OnBuyItemIconInit(int param)
{
	m_pBuyItemIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTradeBuyDlg::OnBuyItemIconOwnerDraw()
{
	if (!m_pBuyItem)
		return;

	WPoint pos(m_pBuyItemIcon->GetRect().x, m_pBuyItemIcon->GetRect().y);
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	if (m_pBuyItem->dwTid == 0)
		return;

	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(m_pBuyItem->dwTid);
	if (!common)
		return;

	float offX = 0.0f;
	float offY = 0.0f;
	const Bitmap* bitmap = g_pFresh->GetBitmap(common->Icon);
	if ((m_pBuyItem->dwTid & 0xfc000000) == 0x8000000)
	{
		offX = -12.0f;
		offY = -20.0f;
	}
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (m_pBuyItem->status & 1) &&
		IsUccClothes(m_pBuyItem->dwTid))
		bitmap = GetUccItemIcon(m_pBuyItem->dwGuid);

	if (bitmap)
	{
		WRect rect(pos.x + offX + 14.0f, pos.y + offY + 11.0f,
			(float)bitmap->Width(), (float)bitmap->Height());
		gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(4);
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
			IsUccClothes(m_pBuyItem->dwTid))
		{
			if ((m_pBuyItem->status & 1) && m_pBuyItem->Seq == 1 &&
				m_pUccState[0])
			{
				gdi->DrawTexture(m_pUccState[0],
					WRect(pos.x + 51.0f, pos.y + 43.0f,
						(float)m_pUccState[0]->Width(),
						(float)m_pUccState[0]->Height()),
					0xffffffff, 0);
			}
			else if ((m_pBuyItem->status & 1) && m_pBuyItem->Seq > 1 &&
				m_pUccState[1])
			{
				gdi->DrawTexture(m_pUccState[1],
					WRect(pos.x + 51.0f, pos.y + 43.0f,
						(float)m_pUccState[1]->Width(),
						(float)m_pUccState[1]->Height()),
					0xffffffff, 0);
			}
			else if ((m_pBuyItem->status & 2) && m_pUccState[2])
			{
				gdi->DrawTexture(m_pUccState[2],
					WRect(pos.x + 51.0f, pos.y + 43.0f,
						(float)m_pUccState[2]->Width(),
						(float)m_pUccState[2]->Height()),
					0xffffffff, 0);
			}
			// 983 008f2270: aviso em vermelho na posicao da janela (+16, +120/+135)
			float x = GetRect().x;
			float y = GetRect().y;
			gdi->SetTextColor(0xffff0000, 0xffffffff);
			float px = x + 16.0;
			gdi->Print(WPoint(px, y + 120.0), 0,
				K2L_Compatibility("\xc1\xa6""3\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb"));
			gdi->Print(WPoint(px, y + 135.0), 0,
				K2L_Compatibility("\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb4\xc2 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce \xb1\xb8\xc0\xd4\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
		}
		int count = m_pBuyItem->iNum;
		if (count > 1)
		{
			const char* text = MakeStr("%d", count);
			float x = pos.x + 54.0f;
			for (count = 0; count < strlen(text); ++count)
				x -= m_pFont->GetCharWidth(g_view, text[count] - '0');
			m_pFont->Print(g_view, x, pos.y + 42.0f, text, 0, 0xffffffff, NULL);
		}
	}

	switch (m_pBuyItem->dwTid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 0x10:
	case 0x1c:
	{
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (common->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(pos.x + 38.0f -
					(float)float2int((float)level->Width() * 0.5f),
				pos.y + 35.0f - (float)float2int((float)level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, 0xffffffff, 0);
		}
		break;
	}
	}
}

void FrTradeBuyDlg::OnBuyItemNameInit(int param)
{
	m_pBuyItemName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeBuyDlg::OnBuyItemAbilityInit(int param)
{
	m_pBuyItemAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeBuyDlg::OnMyPangInit(int param)
{
	m_pMyPang = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMyPang)
	{
		char buf[32] = "";
		MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
		m_pMyPang->SetLine(1, buf, 0, false, 0);
	}
}

void FrTradeBuyDlg::OnPriceInit(int param)
{
	m_pPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeBuyDlg::OnPangLeftInit(int param)
{
	m_pPangLeft = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeBuyDlg::OnNumberInit(int param)
{
	m_pNumber = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeBuyDlg::OnNumberNextBnInit(int param)
{
	m_pNumberNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNumberNext)
	{
		m_pNumberNext->SetPushDelay(0.0f);
		m_pNumberNext->UseDblClick(false); // 983 008ef1e0: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrTradeBuyDlg::OnNumberNextBnUp()
{
	if (!m_pNumberNext || !m_pNumber || !m_pBuyItem)
		return;

	int num = atoi(m_pNumber->GetLine(1, false)) + 1;
	int max = m_pBuyItem->iNum;
	if (num >= max)
	{
		num = max;
		m_pNumberNext->Enable(false);
	}
	m_pNumberPrev->Enable(true);
	m_pNumber->ClearLine();
	m_pNumber->SetLine(1, MakeStr("%d", num), 0, false, 0);
	UpdatePrice();
}

void FrTradeBuyDlg::OnNumberPrevBnInit(int param)
{
	m_pNumberPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNumberPrev)
	{
		m_pNumberPrev->SetPushDelay(0.0f);
		m_pNumberPrev->UseDblClick(false); // 983 008ef270: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrTradeBuyDlg::OnNumberPrevBnUp()
{
	if (!m_pNumberPrev || !m_pNumber || !m_pBuyItem)
		return;

	int num = atoi(m_pNumber->GetLine(1, false)) - 1;
	if (num <= 1)
	{
		num = 1;
		m_pNumberPrev->Enable(false);
	}
	m_pNumberNext->Enable(true);
	m_pNumber->ClearLine();
	m_pNumber->SetLine(1, MakeStr("%d", num), 0, false, 0);
	UpdatePrice();
}

bool FrTradeBuyDlg::OnInit()
{
	EnableDrag(false);
	return true;
}

void FrTradeBuyDlg::OnProc(const float deltaTime)
{
	if (!m_pBuyItem || !m_pNumberNext || !m_pNumberPrev)
		return;

	int num = atoi(m_pNumber->GetLine(1, false));
	if (num <= 1)
	{
		m_pNumberNext->Enable(true);
		m_pNumberPrev->Enable(false);
		m_pNumber->ClearLine();
		m_pNumber->SetLine(1, MakeStr("%d", 1), 0, false, 0);
		UpdatePrice();
		return;
	}

	int max = m_pBuyItem->iNum;
	if (num >= max)
	{
		m_pNumberNext->Enable(false);
		m_pNumberPrev->Enable(true);
		m_pNumber->ClearLine();
		m_pNumber->SetLine(1, MakeStr("%d", max), 0, false, 0);
		UpdatePrice();
		return;
	}

	m_pNumberNext->Enable(true);
	m_pNumberPrev->Enable(true);
	UpdatePrice();
}

void FrTradeBuyDlg::SetBuyItem(sTradeItem* item)
{
	m_pBuyItemName->ClearLine();
	m_pBuyItemAbility->ClearLine();
	m_pBuyItem = item;
	if (!item)
		return;

	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(item->dwTid);
	if (!common || !m_pBuyItemName)
		return;

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item->status & 1) &&
		IsUccClothes(item->dwTid))
	{
		const char* name = GetUccItemName(item->dwGuid);
		m_pBuyItemName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), name), false, true);
	}
	else
		m_pBuyItemName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), common->Name),
			false, true);

	short attr[5];
	short slot[5] = { 0 };
	switch (item->dwTid >> 26)
	{
	case 2:
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->dwTid);
		if (!part)
			return;
		memcpy(attr, part->Attr, sizeof(attr));
		memcpy(slot, part->Slot, sizeof(slot));
		break;
	}
	case 7:
	{
		IFF_STRUCT::sCaddie* caddie = ItemManager()->FindCaddie(item->dwTid);
		if (!caddie)
			return;
		memcpy(attr, caddie->Attr, sizeof(attr));
		break;
	}
	case 8:
	{
		IFF_STRUCT::sCadItem* cadItem = ItemManager()->FindCadItem(item->dwTid);
		if (!cadItem)
			return;
		memcpy(attr, cadItem->COM, sizeof(attr));
		break;
	}
	default:
		return;
	}

	// power, control, accuracy, spin, curve
	const char* label = "";
	for (int i = 0, left = 5; left != 0; ++i, --left)
	{
		if (attr[i] == 0 && slot[i] == 0)
			continue;

		switch (i)
		{
		case 0:
			label = K2L_Compatibility("\xc6\xc4\xbf\xf6");
			break;
		case 1:
			label = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1");
			break;
		case 2:
			label = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5");
			break;
		case 3:
			label = K2L_Compatibility("\xbd\xba\xc7\xc9");
			break;
		case 4:
			label = K2L_Compatibility("\xc4\xbf\xba\xea");
			break;
		}

		if (attr[i] <= 0 && slot[i] <= 0)
			continue;

		if (attr[i] == 0)
			m_pBuyItemAbility->AddText(
				MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
					label, slot[i]),
				false, true);
		else if (slot[i] == 0)
			m_pBuyItemAbility->AddText(
				MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c", label,
					attr[i]),
				false, true);
		else
			m_pBuyItemAbility->AddText(
				MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xbd\xbd\xb7\xd4%+d"
						"\\c0xff000000\\c"),
					label, attr[i], slot[i]),
				false, true);
	}
}

void FrTradeBuyDlg::SetControlsBuyItem(sTradeItem* item)
{
	if (!m_pPrice || !m_pPangLeft || !m_pNumberNext || !m_pNumberPrev ||
		!m_pNumber)
		return;

	m_pPrice->ClearLine();
	m_pPangLeft->ClearLine();
	m_pNumber->ClearLine();
	if (!item)
	{
		m_pNumberNext->Enable(false);
		m_pNumberPrev->Enable(false);
		m_pNumber->Enable(false);
		return;
	}

	m_pNumber->SetLine(1, "1", 0, false, 0);
	UpdatePrice();
	if (item->iNum == 1)
	{
		m_pNumberNext->Enable(false);
		m_pNumberPrev->Enable(false);
		m_pNumber->Enable(false);
		return;
	}
	m_pNumberNext->Enable(true);
	m_pNumberPrev->Enable(true);
	m_pNumber->Enable(true);
}

void FrTradeBuyDlg::UpdatePrice()
{
	if (!m_pPrice || !m_pNumber || !m_pBuyItem)
		return;

	char price[32] = "";
	char left[32] = "";
	int num = atoi(m_pNumber->GetLine(1, false));
	if (m_count != num)
	{
		MakePriceStrA(price, m_pBuyItem->i64Price * num);
		m_pPrice->SetLine(1, price, 0, false, 0);
		MakePriceStrA(left,
			Doc()->m_myInfo.stat.i64Pang - m_pBuyItem->i64Price * num);
		m_pPangLeft->SetLine(1, left, 0, false, 0);
		m_count = num;
	}
}

IMPLEMENT_OBJECT(FrTradeResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeResultDlg, FrForm)

ON_FRESH_VI("traderesultdlg", FRCMD_INIT,
	FrTradeResultDlg::OnTradeResultDlgInit)
ON_FRESH_VI("resultitem_icon", FRCMD_INIT,
	FrTradeResultDlg::OnResultItemIconInit)
ON_FRESH_VV("resultitem_icon", FRCMD_OWNERDRAW,
	FrTradeResultDlg::OnResultItemIconOwnerDraw)
ON_FRESH_VI("resultitem_name", FRCMD_INIT,
	FrTradeResultDlg::OnResultItemNameInit)
ON_FRESH_VI("resultitem_ability", FRCMD_INIT,
	FrTradeResultDlg::OnResultItemAbilityInit)

END_FRESH_MSGMAP()

FrTradeResultDlg::FrTradeResultDlg()
{
	m_pResultItemIcon = NULL;
	m_pResultItemName = NULL;
	m_pResultItemAbility = NULL;
	memset(&m_resultItem, 0, sizeof(m_resultItem));
	static char* s_fontWind[] = { "[font_wind.jpg" };
	WTITLEFONT font;
	memset(&font, 0, sizeof(font));
	font.filename = s_fontWind;
	font.fontw = 16;
	font.fonth = 16;
	font.numPages = 1;
	font.texw = 128;
	font.texh = 64;
	font.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&font);
	m_pUccState[0] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccState[1] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccState[2] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
}

FrTradeResultDlg::~FrTradeResultDlg()
{
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrTradeResultDlg::OnTradeResultDlgInit(int param)
{
}

void FrTradeResultDlg::OnResultItemIconInit(int param)
{
	m_pResultItemIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTradeResultDlg::OnResultItemIconOwnerDraw()
{
	if (m_resultItem.dwTid == 0)
		return;
	if (!m_pResultItemIcon)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(m_resultItem.dwTid);
	if (!common)
		return;

	float offX = 0.0f;
	float offY = 0.0f;
	const Bitmap* bitmap = g_pFresh->GetBitmap(common->Icon);
	if ((m_resultItem.dwTid & 0xfc000000) == 0x8000000)
	{
		offX = -12.0f;
		offY = -20.0f;
	}
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (m_resultItem.status & 1) &&
		IsUccClothes(m_resultItem.dwTid))
		bitmap = GetUccItemIcon(m_resultItem.dwGuid);
	if (!bitmap)
		return;

	WPoint pos(m_pResultItemIcon->GetRect().x, m_pResultItemIcon->GetRect().y);
	WRect rect(pos.x + offX + 14.0f, pos.y + offY + 11.0f,
		(float)bitmap->Width(), (float)bitmap->Height());
	gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(4);
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
		IsUccClothes(m_resultItem.dwTid))
	{
		if ((m_resultItem.status & 1) && m_resultItem.Seq == 1 &&
			m_pUccState[0])
		{
			WPoint pt(m_pResultItemIcon->GetRect().x,
				m_pResultItemIcon->GetRect().y);
			gdi->DrawTexture(m_pUccState[0],
				WRect(pt.x + 51.0f, pt.y + 43.0f,
					(float)m_pUccState[0]->Width(),
					(float)m_pUccState[0]->Height()),
				0xffffffff, 0);
		}
		else if ((m_resultItem.status & 1) && m_resultItem.Seq > 1 &&
			m_pUccState[1])
		{
			WPoint pt(m_pResultItemIcon->GetRect().x,
				m_pResultItemIcon->GetRect().y);
			gdi->DrawTexture(m_pUccState[1],
				WRect(pt.x + 51.0f, pt.y + 43.0f,
					(float)m_pUccState[1]->Width(),
					(float)m_pUccState[1]->Height()),
				0xffffffff, 0);
		}
		else if ((m_resultItem.status & 2) && m_pUccState[2])
		{
			WPoint pt(m_pResultItemIcon->GetRect().x,
				m_pResultItemIcon->GetRect().y);
			gdi->DrawTexture(m_pUccState[2],
				WRect(pt.x + 51.0f, pt.y + 43.0f,
					(float)m_pUccState[2]->Width(),
					(float)m_pUccState[2]->Height()),
				0xffffffff, 0);
		}
	}
	int count = m_resultItem.iNum;
	if (count > 1)
	{
		const char* text = MakeStr("%d", count);
		float x = pos.x + 54.0f;
		for (count = 0; count < strlen(text); ++count)
			x -= m_pFont->GetCharWidth(g_view, text[count] - '0');
		m_pFont->Print(g_view, x, pos.y + 42.0f, text, 0, 0xffffffff, NULL);
	}

	switch (m_resultItem.dwTid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 0x10:
	case 0x1c:
	{
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (common->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(pos.x + 38.0f -
					(float)float2int((float)level->Width() * 0.5f),
				pos.y + 35.0f - (float)float2int((float)level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, 0xffffffff, 0);
		}
		break;
	}
	}
}

void FrTradeResultDlg::OnResultItemNameInit(int param)
{
	m_pResultItemName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeResultDlg::OnResultItemAbilityInit(int param)
{
	m_pResultItemAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrTradeResultDlg::OnInit()
{
	EnableDrag(false);
	return true;
}

void FrTradeResultDlg::SetResultItem(sTradeItem* item)
{
	m_pResultItemName->ClearLine();
	m_pResultItemAbility->ClearLine();
	if (item)
	{
		m_resultItem = *item;

		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(item->dwTid);
		if (!common || !m_pResultItemName)
			return;

		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item->status & 1) &&
			IsUccClothes(item->dwTid))
		{
			const char* name = GetUccItemName(item->dwGuid);
			m_pResultItemName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), name), false,
				true);
		}
		else
			m_pResultItemName->AddText(
				MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), common->Name), false, true);

		short attr[5];
		short slot[5] = { 0 };
		switch (item->dwTid >> 26)
		{
		case 2:
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->dwTid);
			if (!part)
				return;
			memcpy(attr, part->Attr, sizeof(attr));
			memcpy(slot, part->Slot, sizeof(slot));
			break;
		}
		case 7:
		{
			IFF_STRUCT::sCaddie* caddie =
				ItemManager()->FindCaddie(item->dwTid);
			if (!caddie)
				return;
			memcpy(attr, caddie->Attr, sizeof(attr));
			break;
		}
		case 8:
		{
			IFF_STRUCT::sCadItem* cadItem =
				ItemManager()->FindCadItem(item->dwTid);
			if (!cadItem)
				return;
			memcpy(attr, cadItem->COM, sizeof(attr));
			break;
		}
		default:
			return;
		}

		// power, control, accuracy, spin, curve
		const char* label = "";
		for (int i = 0, left = 5; left != 0; ++i, --left)
		{
			if (attr[i] == 0 && slot[i] == 0)
				continue;

			switch (i)
			{
			case 0:
				label = K2L_Compatibility("\xc6\xc4\xbf\xf6");
				break;
			case 1:
				label = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1");
				break;
			case 2:
				label = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5");
				break;
			case 3:
				label = K2L_Compatibility("\xbd\xba\xc7\xc9");
				break;
			case 4:
				label = K2L_Compatibility("\xc4\xbf\xba\xea");
				break;
			}

			if (attr[i] <= 0 && slot[i] <= 0)
				continue;

			if (attr[i] == 0)
				m_pResultItemAbility->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xff0000ff\\c%s\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
						label, slot[i]),
					false, true);
			else if (slot[i] == 0)
				m_pResultItemAbility->AddText(
					MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c", label,
						attr[i]),
					false, true);
			else
				m_pResultItemAbility->AddText(
					MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xbd\xbd\xb7\xd4%+d"
							"\\c0xff000000\\c"),
						label, attr[i], slot[i]),
					false, true);
		}
	}
	else
		memset(&m_resultItem, 0, sizeof(m_resultItem));
}

IMPLEMENT_OBJECT(FrTradeEditDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeEditDlg, FrForm)

ON_FRESH_VI("tradeeditdlg", FRCMD_INIT, FrTradeEditDlg::OnTradeEditDlgInit)
ON_FRESH_VI("my_item_list", FRCMD_INIT, FrTradeEditDlg::OnMyItemListInit)
ON_FRESH_VV("my_item_list", FRCMD_LBUTTONDOWN,
	FrTradeEditDlg::OnMyItemListLClick)
ON_FRESH_VV("my_item_list", FRCMD_RBUTTONDOWN,
	FrTradeEditDlg::OnMyItemListRClick)
ON_FRESH_VI("my_item_list", FRCMD_OWNERDRAW,
	FrTradeEditDlg::OnMyItemListOwnerDraw)
ON_FRESH_VI("trade_item_list", FRCMD_INIT, FrTradeEditDlg::OnTradeItemListInit)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONDOWN,
	FrTradeEditDlg::OnTradeItemListLClick)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	FrTradeEditDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("visitor", FRCMD_INIT, FrTradeEditDlg::OnVisitorInit)
ON_FRESH_VI("userid", FRCMD_INIT, FrTradeEditDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, FrTradeEditDlg::OnTitleInit)
ON_FRESH_VI("trade_title_edit", FRCMD_INIT, FrTradeEditDlg::OnTitleEditInit)
ON_FRESH_BI("trade_title_edit", FRCMD_ENTERKEY,
	FrTradeEditDlg::OnTitleEditEnterKey)
ON_FRESH_VI("trade_title_edit_bn", FRCMD_INIT,
	FrTradeEditDlg::OnTitleEditBnInit)
ON_FRESH_VV("trade_title_edit_bn", FRCMD_LBUTTONUP,
	FrTradeEditDlg::OnTitleEditBnUp)
ON_FRESH_VI("trade_myitem_icon", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemIconInit)
ON_FRESH_VV("trade_myitem_icon", FRCMD_OWNERDRAW,
	FrTradeEditDlg::OnTradeMyItemIconOwnerDraw)
ON_FRESH_VI("trade_myitem_name", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemNameInit)
ON_FRESH_VI("trade_myitem_ability", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemAbilityInit)
ON_FRESH_VI("trade_myitem_number", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemNumberInit)
ON_FRESH_VI("trade_myitem_price", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemPriceInit)
ON_FRESH_VV("trade_myitem_price", FRCMD_LBUTTONUP,
	FrTradeEditDlg::OnTradeMyItemPriceClick)
ON_FRESH_VI("trade_myitem_ok_bn", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemOkBnInit)
ON_FRESH_VV("trade_myitem_ok_bn", FRCMD_LBUTTONUP,
	FrTradeEditDlg::OnTradeMyItemOkBnUp)
ON_FRESH_VI("trade_myitem_cancel_bn", FRCMD_INIT,
	FrTradeEditDlg::OnTradeMyItemCancelBnInit)
ON_FRESH_VV("trade_myitem_cancel_bn", FRCMD_LBUTTONUP,
	FrTradeEditDlg::OnTradeMyItemCancelBnUp)
ON_FRESH_VI("open", FRCMD_INIT, FrTradeEditDlg::OnOpenBnInit)
ON_FRESH_VV("open", FRCMD_LBUTTONUP, FrTradeEditDlg::OnOpenBnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrTradeEditDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrTradeEditDlg::OnCloseBnUp)
ON_FRESH_VI("exit", FRCMD_INIT, FrTradeEditDlg::OnExitBnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, FrTradeEditDlg::OnExitBnUp)
END_FRESH_MSGMAP()

FrTradeEditDlg::FrTradeEditDlg()
{
	m_pTradeItemList = NULL;
	m_pMyItemList = NULL;
	m_pVisitor = NULL;
	m_pUserId = NULL;
	m_pTitle = NULL;
	m_pTitleEdit = NULL;
	m_pMyItemIcon = NULL;
	m_pMyItemName = NULL;
	m_pMyItemAbility = NULL;
	m_pMyItemNumber = NULL;
	m_pMyItemPrice = NULL;
	m_pMyItemOkBn = NULL;
	m_pMyItemCancelBn = NULL;
	m_pOpenBn = NULL;
	m_pCloseBn = NULL;
	m_pExitBn = NULL;
	static char* s_fontWind[] = { "[font_wind.jpg" };
	WTITLEFONT timeFont;
	memset(&timeFont, 0, sizeof(timeFont));
	timeFont.filename = s_fontWind;
	timeFont.fontw = 16;
	timeFont.fonth = 16;
	timeFont.numPages = 1;
	timeFont.texw = 128;
	timeFont.texh = 64;
	timeFont.pCharSet = "1234567890ym%-.?/:,";
	m_pFontTime = g_resrcmng->GetTitleFont();
	m_pFontTime->Create(&timeFont);
	static char* s_fontBong[] = { "[font_bong.jpg" };
	WTITLEFONT priceFont;
	memset(&priceFont, 0, sizeof(priceFont));
	priceFont.filename = s_fontBong;
	priceFont.fontw = 9;
	priceFont.fonth = 16;
	priceFont.numPages = 1;
	priceFont.texw = 128;
	priceFont.texh = 32;
	priceFont.pCharSet = "1234567890,-";
	m_pFontPrice = g_resrcmng->GetTitleFont();
	m_pFontPrice->Create(&priceFont);
	m_pBase[0] = g_pFresh->GetBitmap("trade_myitem_base_click");
	m_pBase[1] = g_pFresh->GetBitmap("trade_myitem_base_over");
	m_pBase[2] = g_pFresh->GetBitmap("trade_item_base_normal");
	m_pBase[3] = g_pFresh->GetBitmap("trade_item_base_over");
	m_pBase[4] = g_pFresh->GetBitmap("trade_item_base_click");
	m_pBase[5] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pBase[6] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pBase[7] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	memset(m_unused190, 0, sizeof(m_unused190));
	memset(m_title, 0, sizeof(m_title));
	m_listMyItem.clear();
	m_listTradeItem.clear();
	m_pBuyItem = NULL;
	m_btn[0].pBitmap = g_pFresh->GetBitmap("trade_item_del_bn_normal");
	m_btn[0].rect = WRect(120.0f, 30.0f, m_btn[0].pBitmap->Width(),
		m_btn[0].pBitmap->Height());
	m_btn[1].pBitmap = g_pFresh->GetBitmap("trade_item_del_bn_over");
	m_btn[1].rect = WRect(120.0f, 30.0f, m_btn[1].pBitmap->Width(),
		m_btn[1].pBitmap->Height());
	m_btn[2].pBitmap = g_pFresh->GetBitmap("trade_item_edit_bn_normal");
	m_btn[2].rect = WRect(80.0f, 30.0f, m_btn[2].pBitmap->Width(),
		m_btn[2].pBitmap->Height());
	m_btn[3].pBitmap = g_pFresh->GetBitmap("trade_item_edit_bn_over");
	m_btn[3].rect = WRect(80.0f, 30.0f, m_btn[3].pBitmap->Width(),
		m_btn[3].pBitmap->Height());
	m_btn[5].pBitmap = g_pFresh->GetBitmap("clock");
	m_btn[5].rect = WRect(50.0f, 35.0f, m_btn[5].pBitmap->Width(),
		m_btn[5].pBitmap->Height());
	m_btn[4].pBitmap = g_pFresh->GetBitmap("delete");
	m_btn[4].rect = WRect(50.0f, 35.0f, m_btn[4].pBitmap->Width(),
		m_btn[4].pBitmap->Height());
	SetOpenAfterTitle983(false); // 983: 008f71e3
}

FrTradeEditDlg::~FrTradeEditDlg()
{
	// HACK: dead reference, defers the destructor one wave as in the original
	if (0)
		DelListMyItem(NULL);
	if (m_pFontPrice)
		m_pFontPrice->Erase();
	if (m_pFontTime)
		m_pFontTime->Erase();
	if (g_resrcmng)
	{
		if (m_pFontPrice)
		{
			g_resrcmng->Release(m_pFontPrice);
			m_pFontPrice = NULL;
		}
		if (g_resrcmng && m_pFontTime)
		{
			g_resrcmng->Release(m_pFontTime);
			m_pFontTime = NULL;
		}
	}
}

void FrTradeEditDlg::OnTradeEditDlgInit(int param)
{
}

void FrTradeEditDlg::OnMyItemListInit(int param)
{
	m_pMyItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pMyItemList)
		m_pMyItemList->UseRightButton(true);
}

void FrTradeEditDlg::OnMyItemListLClick()
{
	if (m_listTradeItem.size() < 6)
	{
		FrListItem* listItem = m_pMyItemList->GetItemUnderCursor();
		if (listItem)
		{
			if (!listItem->selected)
			{
				m_pMyItemList->ToggleItem(listItem);
				return;
			}
			sTradeItem* item = (sTradeItem*)listItem->pData;
			SetTradeMyItem(item);
			SetControlsTradeMyItem(item, false);
		}
	}
}

void FrTradeEditDlg::OnMyItemListRClick()
{
	if (m_listTradeItem.size() < 6)
	{
		FrListItem* listItem = m_pMyItemList->GetItemUnderCursor();
		if (listItem)
		{
			sTradeItem* item = (sTradeItem*)listItem->pData;
			if (item)
			{
				FrItemInfoDlg* info = CreateForm<FrItemInfoDlg>(
					g_pFresh->GetManager(), this, "iteminfo", NULL);
				if (info)
				{
					info->Open(NULL, 3);
					info->SetItemInfo(item);
					if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
					{
						FrEdit* edit = DYNAMIC_CAST(FrEdit,
							info->FindChildByName("avility"));
						if (edit)
						{
							IFF_STRUCT::sPart* part =
								Doc()->m_itemManager.FindPart(item->dwTid);
							if (part)
							{
								for (int i = 0; i < 5; i++)
								{
									if (part->Attr[i] != 0 ||
										part->Slot[i] != 0)
									{
										bool b1 = true;
										bool b2 = false;
										const char* status = MakeStatusString(i,
											part->Attr[i], part->Slot[i]);
										edit->AddText(
											MakeStr(
												K2L_Compatibility("\\c0xff0000ff\\c%s\\c0xff000000\\c"),
												status),
											b2, b1);
									}
								}
							}
							unsigned long* attachCard =
								&item->attachCardTid[0][0];
							for (int n = 8; n != 0; n--)
							{
								unsigned long tid = *attachCard;
								if (tid != 0)
								{
									unsigned long type = (tid >> 22) & 0xf;
									if (type == 0)
									{
										IFF_STRUCT::sCard* card =
											Doc()->m_itemManager.FindCard(tid);
										if (card)
										{
											for (int i = 0; i < 5; i++)
											{
												if ((*card).COM[i] > 0)
												{
													bool b1 = true;
													bool b2 = false;
													const char* status =
														MakeCardStatusString(i,
															(*card).COM[i]);
													edit->AddText(
														MakeStr(
															K2L_Compatibility("\\c0xff60ee0e\\c%s\\c0xff000000\\c"),
															status),
														b2, b1);
												}
											}
										}
									}
									else if (type == 1)
									{
										IFF_STRUCT::sDesc* desc =
											Doc()->m_itemManager.FindDesc(tid);
										if (desc)
										{
											char text[512];
											const char* src = desc->Desc;
											char c;
											int n2 = 0;
											do
											{
												c = *src;
												text[n2++] = c;
												src++;
											} while (c != 0);
											for (int i = 0; i < 512; i++)
											{
												if (text[i] == '\n')
												{
													text[i] = 0;
													break;
												}
											}
											edit->AddText(
												MakeStr(
													K2L_Compatibility("\\c0xffff0000\\c%s\\c0xff000000\\c"),
													text),
												false, true);
										}
									}
								}
								attachCard++;
							}
						}
					}
				}
			}
			if (!listItem->selected)
			{
				m_pMyItemList->ToggleItem(listItem);
				return;
			}
			SetTradeMyItem(item);
			SetControlsTradeMyItem(item, false);
		}
	}
}

void FrTradeEditDlg::OnMyItemListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	FrListItem* listItem = (FrListItem*)param;
	if (!listItem)
		return;

	sTradeItem* item = (sTradeItem*)listItem->pData;
	const Bitmap* base = NULL;
	if (listItem->selected)
	{
		base = m_pBase[0];
		gdi->SetTextColor(0xffff0000, 0xffffffff);
	}
	else if (listItem->underCursor)
	{
		base = m_pBase[1];
		gdi->SetTextColor(0xff000000, 0xffffffff);
	}
	else
	{
		gdi->SetTextColor(0xff000000, 0xffffffff);
	}
	if (base)
	{
		WRect rect(listItem->pos.x + 2.0f, listItem->pos.y,
			(float)base->Width(), (float)base->Height());
		gdi->DrawTexture(base, rect, 0xffffffff, 0);
	}
	if ((item->dwTid & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sChar* chara =
			ItemManager()->FindChar(((item->dwTid >> 18) & 0xff) | 0x4000000);
		if (chara)
		{
			const Bitmap* icon =
				g_pFresh->GetBitmap(MakeStr("s_%s", chara->c.Icon));
			if (icon)
			{
				WRect rect(listItem->pos.x + 7.0f, listItem->pos.y + 5.0f,
					(float)icon->Width(), (float)icon->Height());
				gdi->DrawTexture(icon, rect, 0xffffffff, 0);
			}
		}
	}
	WPoint pt(listItem->pos.x + 13.0f, listItem->pos.y + 20.0f);
	// 983 (008f5770): so o DrawItem; sem o desenho dos encaixes de carta (DrawPartsSlot)
	DrawItem(pt, item, 0);
}

void FrTradeEditDlg::OnTradeItemListInit(int param)
{
	m_pTradeItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeItemListLClick()
{
	FrListItem* listItem = m_pTradeItemList->GetItemUnderCursor();
	if (listItem)
	{
		sTradeItem* item = (sTradeItem*)listItem->pData;
		// 983 008f7440: rect do botao montado em local (pos + rect)
		WRect rc(listItem->pos.x + m_btn[0].rect.x,
			listItem->pos.y + m_btn[0].rect.y, m_btn[0].rect.w,
			m_btn[0].rect.h);
		if (rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			m_pTradeItemList->SelectItem(m_pTradeItemList->GetItemUnderCursor(),
				true);
			SetTradeResultCode(0x16, 0xe3, NULL);
			return;
		}
		bool enable = false;
		std::list<sTradeItem>::iterator it = m_listMyItem.begin();
		for (; it != m_listMyItem.end(); ++it)
		{
			if (it->dwGuid == item->dwGuid)
			{
				if (it->iNum + item->iNum > 1)
					enable = true;
				break;
			}
		}
		SetTradeMyItem(item);
		SetControlsTradeMyItem(item, enable);
	}
}

void FrTradeEditDlg::OnTradeItemListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	FrListItem* listItem = (FrListItem*)param;
	if (!listItem)
		return;

	sTradeItem* item = (sTradeItem*)listItem->pData;
	IFF_ITEM_COMMON* common = Doc()->m_itemManager.FindCommonItem(item->dwTid);
	if (m_pBase[2])
	{
		WRect rect(listItem->pos.x + 1.0f, listItem->pos.y + 1.0f,
			(float)m_pBase[2]->Width(), (float)m_pBase[2]->Height());
		gdi->DrawTexture(m_pBase[2], rect, 0xffffffff, 0);
	}

	char buf[32] = "";
	MakePriceStrA(buf, item->i64Price);
	const char* price;
	if (item->iNum == 1)
		price = MakeStr(K2L_Compatibility("%s\xc6\xce"), buf);
	else
		price = MakeStr(K2L_Compatibility("\xb0\xb3\xb4\xe7%s\xc6\xce"), buf);

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item->status & 1) &&
		IsUccClothes(item->dwTid))
		gdi->Print(WPoint(listItem->pos.x + 10.0f, listItem->pos.y + 13.0f), 0,
			"%s", GetUccItemName(item->dwGuid));
	else
		gdi->Print(WPoint(listItem->pos.x + 10.0f, listItem->pos.y + 13.0f), 0,
			"%s", common->Name);

	SetTextColorByPrice(item->i64Price);
	gdi->Print(WPoint(listItem->pos.x + 12.0f, listItem->pos.y + 32.0f), 0,
		price);
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->DrawTexture(m_btn[0].pBitmap,
		WRect(listItem->pos.x + m_btn[0].rect.x,
			listItem->pos.y + m_btn[0].rect.y, m_btn[0].rect.w,
			m_btn[0].rect.h),
		0xffffffff, 0);
}

void FrTradeEditDlg::OnVisitorInit(int param)
{
	m_pVisitor = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pVisitor)
	{
		char buf[32] = "";
		MakePriceStrA(buf, Doc()->GetCountVisitor());
		m_pVisitor->SetLine(1, buf, 0, false, 0);
	}
}

void FrTradeEditDlg::OnUserIdInit(int param)
{
	m_pUserId = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pUserId)
		m_pUserId->SetLine(1, K2L_Compatibility("\xb3\xaa\xc0\xc7 \xbb\xf3\xc1\xa1"), 0, false, 0);
}

void FrTradeEditDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	UpdateTradeTitle();
}

void FrTradeEditDlg::OnTitleEditInit(int param)
{
	m_pTitleEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrTradeEditDlg::OnTitleEditEnterKey(int param)
{
	int result = SendTradeTitle();
	if (result != 1)
		SetTradeResultCode(result, 0xe3, NULL);
	return true;
}

void FrTradeEditDlg::OnTitleEditBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeEditDlg::OnTitleEditBnUp()
{
	int result = SendTradeTitle();
	if (result != 1)
		SetTradeResultCode(result, 0xe3, NULL);
}

void FrTradeEditDlg::OnTradeMyItemIconInit(int param)
{
	m_pMyItemIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemIconOwnerDraw()
{
	if (m_pBuyItem)
	{
		WPoint pos(m_pMyItemIcon->GetRect().x, m_pMyItemIcon->GetRect().y);
		// 983 (008f5960): sem o desenho dos encaixes de carta (DrawPartsSlot nao existe no 983)
		DrawItem(pos, (sTradeItem*)m_pBuyItem, 0);
	}
	DrawPang();
}

void FrTradeEditDlg::OnTradeMyItemNameInit(int param)
{
	m_pMyItemName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemAbilityInit(int param)
{
	m_pMyItemAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemNumberInit(int param)
{
	m_pMyItemNumber = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemPriceInit(int param)
{
	// 983 (008ef9d0): sem o limite de 8 caracteres do 645
	m_pMyItemPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemPriceClick()
{
	if (m_pMyItemPrice)
		m_pMyItemPrice->SetKeyFocus(true);
}

void FrTradeEditDlg::OnTradeMyItemOkBnInit(int param)
{
	m_pMyItemOkBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeEditDlg::OnTradeMyItemOkBnUp()
{
	if (!m_pBuyItem)
		return;

	sTradeItem item = *m_pBuyItem;
	int num = atoi(m_pMyItemNumber->GetLine(1, false));
	char digits[10] = "";
	const char* priceText = m_pMyItemPrice->GetLine(1, false);
	int i = 0;
	char* out = digits;
	for (; i < 9; i++)
	{
		char c = priceText[i];
		if (c == '\0')
			break;
		if (c < '0' || c > '9')
		{
			if (c != ',')
			{
				digits[0] = '\0';
				break;
			}
		}
		else
		{
			*out++ = c;
		}
	}
	__int64 price = atoi(digits);
	int have = item.iNum;

	std::list<sTradeItem>::iterator it = m_listTradeItem.begin();
	for (; it != m_listTradeItem.end(); ++it)
	{
		if (&*it == m_pBuyItem)
		{
			have = it->iNum;
			std::list<sTradeItem>::iterator my = m_listMyItem.begin();
			for (; my != m_listMyItem.end(); ++my)
			{
				if (my->dwGuid == m_pBuyItem->dwGuid)
				{
					have += my->iNum;
					break;
				}
			}
			break;
		}
	}

	if (num < 1 || num > have)
	{
		SetTradeResultCode(0xb, 0xe3, NULL);
		return;
	}
	else if (price <= 0)
	{
		// 983 (008f8677): 0xc = preco errado; o limite de 30 milhoes do 645 nao existe
		SetTradeResultCode(0xc, 0xe3, NULL);
		return;
	}
	else
	{
		item.iNum = num;
		item.i64Price = price;
		DelListMyItem(&item);
		AddListTradeItem(&item);
		SetTradeMyItem(NULL);
		// 983 (008f86bb): pelas duas auxiliares, fora de linha
		SetControlsTradeMyItem(NULL, false);
		SetControlsButton(true, Doc()->m_listTradeItem.size() > 0);
		if (m_pMyItemList)
			m_pMyItemList->UnselectAllItem();
		return;
	}
}

void FrTradeEditDlg::OnTradeMyItemCancelBnInit(int param)
{
	m_pMyItemCancelBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008f59b0): os quatro controles passam pelo SetControlsTradeMyItem
void FrTradeEditDlg::OnTradeMyItemCancelBnUp()
{
	SetTradeMyItem(NULL);
	SetControlsTradeMyItem(NULL, false);
}

void FrTradeEditDlg::OnOpenBnInit(int param)
{
	m_pOpenBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008f8730): abre com 0xc1; o nome da loja vem de SetTradeUserId (008ee390); codigo 8 manda o titulo; o
// SetTradeResultCode tambem e chamado com 1 (sem efeito: 1 so vale com o pacote 0xec).
void FrTradeEditDlg::OnOpenBnUp()
{
	int result = CheckTradeEdit();
	if (result == 1)
	{
		FrTradeFinishEditDlg* dlg = CreateForm<FrTradeFinishEditDlg>(
			g_pFresh->GetManager(), this, "tradefinisheditdlg", NULL);
		if (dlg)
		{
			dlg->Open(NULL, 0xc1);
			dlg->SetFadeout(false);
			dlg->SetListTradeItem(&m_listTradeItem);
			dlg->SetTradeUserId();
			dlg->SetTradeTitle(m_title);
		}
	}
	else if (result == 8)
	{
		int sent = SendTradeTitle();
		if (sent != 1)
			SetTradeResultCode(sent, 0xe3, NULL);
		return;
	}
	SetTradeResultCode(result, 0xe3, NULL);
}

void FrTradeEditDlg::OnCloseBnInit(int param)
{
	m_pCloseBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeEditDlg::OnCloseBnUp()
{
	SetTradeResultCode(0x14, 0xe3, NULL);
}

void FrTradeEditDlg::OnExitBnInit(int param)
{
	m_pExitBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008ee0f0): so chama o virtual OnCancel (posicao 34)
void FrTradeEditDlg::OnExitBnUp()
{
	OnCancel();
}

// 983 (008f3890): virtual novo; o corpo do OnExitBnUp do 645 (abre/fecha a loja no servidor) e FrForm::OnCancel.
// Sem ele o ESC fechava a edicao sem mandar a loja ao servidor.
void FrTradeEditDlg::OnCancel()
{
	if (Doc()->m_listTradeItem.size() > 0)
		Doc()->SendTradeOpenShop();
	else
		Doc()->SendTradeCloseShop();
	FrForm::OnCancel();
}

// 983 (008f9870): os controles passam pelos dois auxiliares e o byte de +0x1e0 e zerado nos dois ramos
bool FrTradeEditDlg::OnInit()
{
	EnableDrag(false);
	SetListEnableTradingMyItem();
	SetListTradingItem();
	SetListMyItem();
	SetListTradeItem();
	SetTradeMyItem(NULL);
	SetControlsTradeMyItem(NULL, false);
	if (Doc()->m_listTradeItem.size() > 0)
	{
		SetControlsButton(true, true);
		SetOpenAfterTitle983(false);
	}
	else
	{
		SetControlsButton(false, false);
		SetOpenAfterTitle983(false);
	}
	return true;
}

// 983 (008ee290 / 008ee2a0): fora de linha, chamados tambem pelo CTask::OnPacketCommon
void FrTradeEditDlg::SetOpenAfterTitle983(bool b)
{
	m_bOpenAfterTitle983 = b;
}

bool FrTradeEditDlg::IsOpenAfterTitle983()
{
	return m_bOpenAfterTitle983;
}

void FrTradeEditDlg::SetListEnableTradingMyItem()
{
	m_listMyItem.clear();
	sTradeItem item;

	for (std::list<sItemInfo>::iterator it = Doc()->m_partsList.begin();
		it != Doc()->m_partsList.end(); ++it)
	{
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->tid);
		if (common && (common->IsSalable == 1 || common->IsSalable == 3))
		{
			item.dwGuid = it->guid;
			item.dwTid = it->tid;
			item.iNum = 1 - GetNumTradingItem(it->guid);
			item.i64Price = 0;
			// 983 008f8a32: sem o IsLocalContent das cartas neste laco
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->tid);
			if (part)
			{
				item.CharacterSlotNum = part->CharacterSlot;
				item.CaddieSlotNum = part->CaddieSlot;
				item.NpcSlotNum983 = part->NpcSlot983; // 983: 008f8a32 / 008f8ba0
				memset(item.attachCardTid, 0, sizeof(item.attachCardTid));
				for (std::list<sSCardAvilityPeriodInfo>::iterator card =
						 Doc()->m_cardAbilityList[0].begin();
					card != Doc()->m_cardAbilityList[0].end(); ++card)
				{
					if ((*card).partsTid == it->tid &&
						(*card).partsUid == it->guid)
					{
						item.iNum = 0;
						break;
					}
				}
			}
			if (item.iNum > 0)
				m_listMyItem.push_back(item);
		}
	}

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
	{
		for (std::list<sItemInfo>::iterator it = Doc()->m_uccItemList.begin();
			it != Doc()->m_uccItemList.end(); ++it)
		{
			if (GetNumTradingItem(it->guid) > 0)
				continue;
			IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->tid);
			if (common && (common->IsSalable == 1 || common->IsSalable == 3))
			{
				bool equipped = false;
				for (std::map<unsigned int, sCharacterInfo>::iterator chr =
						 Doc()->m_charMap.begin();
					chr != Doc()->m_charMap.end(); ++chr)
				{
					if (chr->second.tid ==
							(((it->tid >> 18) & 0xff) | 0x4000000) &&
						chr->second.ItemIdList[(it->tid >> 13) & 0x1f] ==
							it->guid)
					{
						equipped = true;
						break;
					}
				}
				if (equipped)
					continue;
				item.dwGuid = it->guid;
				item.dwTid = it->tid;
				strncpy(item.UccIndex, it->UccIndex, sizeof(item.UccIndex));
				item.Seq = it->Seq;
				item.iNum = 1;
				item.i64Price = 0;
				item.status = it->status;
				if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
				{
					IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->tid);
					if (part)
					{
						item.CharacterSlotNum = part->CharacterSlot;
						item.CaddieSlotNum = part->CaddieSlot;
						item.NpcSlotNum983 = part->NpcSlot983; // 983: 008f8a32 / 008f8ba0
						memset(item.attachCardTid, 0,
							sizeof(item.attachCardTid));
						for (std::list<sSCardAvilityPeriodInfo>::iterator card =
								 Doc()->m_cardAbilityList[0].begin();
							card != Doc()->m_cardAbilityList[0].end(); ++card)
						{
							if ((*card).partsTid == it->tid &&
								(*card).partsUid == it->guid)
							{
								item.iNum = 0;
								break;
							}
						}
					}
				}
				if (item.iNum > 0)
					m_listMyItem.push_back(item);
			}
		}
	}

	for (std::list<sItemInfo>::iterator it = Doc()->m_auxPartList.begin();
		it != Doc()->m_auxPartList.end(); ++it)
	{
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->tid);
		if (common && (common->IsSalable == 1 || common->IsSalable == 3))
		{
			item.dwGuid = it->guid;
			item.dwTid = it->tid;
			item.iNum = it->Common[0] - GetNumTradingItem(it->guid);
			item.i64Price = 0;
			if (item.iNum > 0)
				m_listMyItem.push_back(item);
		}
	}

	SetListEquipParts();
	for (std::list<sTradeItem>::iterator it = m_listMyItem.begin();
		it != m_listMyItem.end();)
	{
		if (IsEquipParts(it->dwTid, it->dwGuid))
			it = m_listMyItem.erase(it);
		else
			++it;
	}

	for (std::list<sItemInfo>::iterator it = Doc()->m_myItemList.begin();
		it != Doc()->m_myItemList.end(); ++it)
	{
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->tid);
		if (common && (common->IsSalable == 1 || common->IsSalable == 3))
		{
			item.dwGuid = it->guid;
			item.dwTid = it->tid;
			int trading = GetNumTradingItem(it->guid);
			item.iNum = it->Common[0] - trading - GetNumEquipItem(it->tid);
			item.i64Price = 0;
			if (item.iNum > 0)
				m_listMyItem.push_back(item);
		}
	}

	for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
		it != Doc()->m_ballList.end(); ++it)
	{
		if (it->tid == Doc()->m_myInfo.userEquip.tidBall)
			continue;
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->tid);
		if (common && (common->IsSalable == 1 || common->IsSalable == 3))
		{
			item.dwGuid = it->guid;
			item.dwTid = it->tid;
			item.iNum = it->Common[0] - GetNumTradingItem(it->guid);
			item.i64Price = 0;
			if (item.iNum > 0)
				m_listMyItem.push_back(item);
		}
	}

	for (std::map<unsigned int, sCaddieInfo>::iterator it =
			 Doc()->m_caddieMap.begin();
		it != Doc()->m_caddieMap.end(); ++it)
	{
		if (it->second.guid != Doc()->m_myInfo.userEquip.guidCaddie)
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem(it->second.tid);
			if (common && (common->IsSalable == 1 || common->IsSalable == 3))
			{
				item.dwGuid = it->second.guid;
				item.dwTid = it->second.tid;
				item.iNum = 1 - GetNumTradingItem(it->second.guid);
				item.i64Price = 0;
				if (item.iNum > 0)
					m_listMyItem.push_back(item);
			}
		}
	}

	for (std::map<unsigned int, sItemInfo>::iterator it =
			 Doc()->m_clubSetMap.begin();
		it != Doc()->m_clubSetMap.end(); ++it)
	{
		if (it->second.guid != Doc()->m_myInfo.userEquip.guidClubSet)
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem(it->second.tid);
			if (common && (common->IsSalable == 1 || common->IsSalable == 3))
			{
				memset(&item, 0, sizeof(item));
				item.dwGuid = it->second.guid;
				item.dwTid = it->second.tid;
				item.iNum = 1 - GetNumTradingItem(it->second.guid);
				item.i64Price = 0;
				// 983 008f9200: encaixes de carta do taco e taco com carta em uso nao entra
				IFF_STRUCT::sClubSet* clubSet =
					ItemManager()->FindClubSet(it->second.tid);
				if (clubSet)
				{
					item.CharacterSlotNum = clubSet->CharacterSlot983;
					item.CaddieSlotNum = clubSet->CaddieSlot983;
					memset(item.attachCardTid, 0, sizeof(item.attachCardTid));
					for (std::list<sSCardAvilityPeriodInfo>::iterator card =
							 Doc()->m_cardAbilityList[0].begin();
						card != Doc()->m_cardAbilityList[0].end(); ++card)
					{
						if ((*card).partsTid == it->second.tid &&
							(*card).partsUid == it->second.guid)
						{
							item.iNum = 0;
							break;
						}
					}
				}
				// 983: 5 shorts em sItemInfo+0xaa (slotUp983); algum != 0 = taco fora da lista
				const short* lock983 = it->second.slotUp983;
				int n;
				for (n = 0; n < 5; n++)
				{
					if (lock983[n] != 0)
						break;
				}
				if (n < 5)
					continue;
				item.dwUpgradeCost = 0;
				for (int k = 0; k < 5; k++)
					item.arrEnchant[k] = it->second.Common[k];
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < item.arrEnchant[i]; j++)
					{
						IFF_STRUCT::sEnchant* enchant =
							ItemManager()->FindEnchant(((0x340 | i) << 20) | j);
						if (enchant)
							item.dwUpgradeCost += enchant->ReqdPang;
					}
				}
				if (item.iNum > 0)
					m_listMyItem.push_back(item);
			}
		}
	}

	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
	{
		for (std::list<sCards>::iterator it = Doc()->m_cardList.begin();
			it != Doc()->m_cardList.end(); ++it)
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem((*it).tid);
			if (common && (common->IsSalable == 1 || common->IsSalable == 3))
			{
				item.dwGuid = (*it).uid;
				item.dwTid = (*it).tid;
				item.iNum = (*it).cardCount - GetNumTradingItem((*it).uid);
				item.i64Price = 0;
				if (item.iNum > 0)
					m_listMyItem.push_back(item);
			}
		}
	}
}

void FrTradeEditDlg::SetListEquipParts()
{
	m_listEquipParts.clear();
	for (std::map<unsigned int, sCharacterInfo>::iterator it =
			 Doc()->m_charMap.begin();
		it != Doc()->m_charMap.end(); ++it)
	{
		for (int i = 0; i < 24; i++)
		{
			if (it->second.tidParts[i] != 0)
			{
				sPartEquipInfo info;
				info.m_part = it->second.tidParts[i];
				info.m_tid = it->second.ItemIdList[i];
				m_listEquipParts.push_back(info);
			}
		}
	}
}

bool FrTradeEditDlg::IsEquipParts(unsigned long part, unsigned long tid)
{
	// 983 008f6410: uma saida so, com o resultado em local
	bool found = false;
	for (std::list<sPartEquipInfo>::iterator it = m_listEquipParts.begin();
		it != m_listEquipParts.end(); ++it)
	{
		if ((it->m_tid == 0 && it->m_part == part) ||
			(it->m_tid != 0 && it->m_part == part && it->m_tid == tid))
		{
			m_listEquipParts.erase(it);
			found = true;
			break;
		}
	}
	return found;
}

int FrTradeEditDlg::GetNumEquipItem(unsigned long guid)
{
	int count = 0;
	if (guid == Doc()->m_myInfo.userEquip.tidItemSlot[0])
		count = 1;
	for (int i = 1; i < 10; i++)
	{
		if (guid == Doc()->m_myInfo.userEquip.tidItemSlot[i])
			count++;
	}
	return count;
}

int FrTradeEditDlg::GetNumTradingItem(unsigned long guid)
{
	int count = 0;
	for (std::list<sTradeItem>::iterator it = Doc()->m_listTradeItem.begin();
		it != Doc()->m_listTradeItem.end(); ++it)
	{
		if (it->dwGuid == guid)
			count += it->iNum;
	}
	return count;
}

void FrTradeEditDlg::SetListTradingItem()
{
	// 983 (008f76b0): lista do doc lida uma vez
	std::list<sTradeItem>& docList = Doc()->m_listTradeItem;
	for (std::list<sTradeItem>::iterator it = docList.begin(); it != docList.end(); ++it)
		m_listTradeItem.push_back(*it);
}

void FrTradeEditDlg::SetListMyItem()
{
	if (m_pMyItemList)
	{
		m_pMyItemList->ClearItem();
		m_listMyItem.sort(_TradeItemSort<sTradeItem>());
		// 983 (008f97d0): ficam de fora as cartas (tipo 0x1f) e as pecas com carta aplicada
		for (std::list<sTradeItem>::iterator it = m_listMyItem.begin();
			it != m_listMyItem.end(); ++it)
		{
			if ((it->dwTid & 0xfc000000) == 0x7c000000)
				continue;
			bool hasCard = false;
			for (std::list<sSCardAvilityPeriodInfo>::iterator card =
					 Doc()->m_cardAbilityList[0].begin();
				card != Doc()->m_cardAbilityList[0].end(); ++card)
			{
				if (card->partsUid == it->dwGuid)
				{
					hasCard = true;
					break;
				}
			}
			if (!hasCard)
				m_pMyItemList->AddItem(&*it);
		}
	}
}

void FrTradeEditDlg::SetListTradeItem()
{
	if (m_pTradeItemList)
	{
		m_pTradeItemList->ClearItem();
		for (std::list<sTradeItem>::iterator it = m_listTradeItem.begin();
			it != m_listTradeItem.end(); ++it)
			m_pTradeItemList->AddItem(&*it);
	}
}

void FrTradeEditDlg::SetTradeMyItem(sTradeItem* item)
{
	if (m_pMyItemName)
		m_pMyItemName->ClearLine();
	if (m_pMyItemAbility)
		m_pMyItemAbility->ClearLine();
	m_pBuyItem = item;
	IFF_ITEM_COMMON* common;
	if (!item)
		return;
	common = ItemManager()->FindCommonItem(item->dwTid);
	if (!common || !m_pMyItemName)
		return;

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item->status & 1) &&
		IsUccClothes(item->dwTid))
	{
		const char* name = GetUccItemName(item->dwGuid);
		m_pMyItemName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), name), false, true);
	}
	else
		m_pMyItemName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), common->Name), false,
			true);

	short attr[5];
	short slot[5] = { 0 };
	bool draw = true;
	unsigned long type = item->dwTid >> 26;
	switch (type)
	{
	case 8:
	{
		IFF_STRUCT::sCadItem* cadItem = ItemManager()->FindCadItem(item->dwTid);
		if (!cadItem)
			return;
		memcpy(attr, cadItem->COM, sizeof(attr));
		break;
	}
	case 7:
	{
		IFF_STRUCT::sCaddie* caddie = ItemManager()->FindCaddie(item->dwTid);
		if (!caddie)
			return;
		memcpy(attr, caddie->Attr, sizeof(attr));
		break;
	}
	case 2:
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->dwTid);
		if (!part)
			return;
		memcpy(attr, part->Attr, sizeof(attr));
		memcpy(slot, part->Slot, sizeof(slot));
		break;
	}
	default:
		draw = false;
		break;
	}
	if (draw)
	{
		// power, control, accuracy, spin, curve
		const char* label = "";
		for (int i = 0, left = 5; left != 0; i++, left--)
		{
			if (attr[i] == 0 && slot[i] == 0)
				continue;

			switch (i)
			{
			case 0:
				label = K2L_Compatibility("\xc6\xc4\xbf\xf6");
				break;
			case 1:
				label = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1");
				break;
			case 2:
				label = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5");
				break;
			case 3:
				label = K2L_Compatibility("\xbd\xba\xc7\xc9");
				break;
			case 4:
				label = K2L_Compatibility("\xc4\xbf\xba\xea");
				break;
			}

			if (attr[i] <= 0 && slot[i] <= 0)
				continue;

			if (attr[i] == 0)
				m_pMyItemAbility->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xff0000ff\\c%s\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
						label, slot[i]),
					false, true);
			else if (slot[i] == 0)
				m_pMyItemAbility->AddText(
					MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c", label,
						attr[i]),
					false, true);
			else
				m_pMyItemAbility->AddText(
					MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xbd\xbd\xb7\xd4%+d"
							"\\c0xff000000\\c"),
						label, attr[i], slot[i]),
					false, true);
		}
		return;
	}
	if (type == 4)
		m_pMyItemAbility->AddText(
			MakeStr(
				K2L_Compatibility("\\c0xff009000\\c\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xc7\xd1 \xba\xf1\xbf\xeb:\n%d\xc6\xce\\c0xff000000\\c"),
				item->dwUpgradeCost),
			false, true);
}

void FrTradeEditDlg::AddListTradeItem(sTradeItem* item)
{
	if (!item || !m_pBuyItem)
		return;

	std::list<sTradeItem>::iterator end = m_listTradeItem.end();
	std::list<sTradeItem>::iterator it = m_listTradeItem.begin();
	for (; it != end; ++it)
	{
		if (&*it == m_pBuyItem)
			break;
	}
	if (it == end || &*it != m_pBuyItem)
	{
		m_listTradeItem.insert(end, *item);
		m_pTradeItemList->AddItem(&*--end);
	}
	else
	{
		*it = *item;
	}
}

void FrTradeEditDlg::DelListTradeItem(sTradeItem* item)
{
	// HACK: dead reference, defers this function one wave as in the original
	if (0)
		CheckTradeEdit();
	if (item)
	{
		for (std::list<sTradeItem>::iterator it = m_listTradeItem.begin();
			it != m_listTradeItem.end(); ++it)
		{
			if (&*it == item)
			{
				m_pTradeItemList->DelItem(&*it);
				m_listTradeItem.erase(it);
				return;
			}
		}
	}
}

void FrTradeEditDlg::AddListMyItem(sTradeItem* item)
{
	if (!item)
		return;

	std::list<sTradeItem>::iterator end = m_listMyItem.end();
	std::list<sTradeItem>::iterator it = m_listMyItem.begin();
	while (it != end && it->dwGuid != item->dwGuid)
		++it;
	if (it == end)
	{
		m_listMyItem.insert(end, *item);
		m_pMyItemList->AddItem(&*--end);
	}
	else if (item->iNum > 0)
	{
		it->iNum += item->iNum;
	}
}

void FrTradeEditDlg::DelListMyItem(sTradeItem* item)
{
	// HACK: dead reference, defers this function one wave as in the original
	if (0)
		CheckTradeEdit();
	if (item && m_pBuyItem)
	{
		std::list<sTradeItem>::iterator it = m_listMyItem.begin();
		for (; it != m_listMyItem.end(); ++it)
		{
			if (it->dwGuid == item->dwGuid)
				break;
		}
		if (it != m_listMyItem.end())
		{
			int num = 0;
			for (std::list<sTradeItem>::iterator t = m_listTradeItem.begin();
				t != m_listTradeItem.end(); ++t)
			{
				if (&*t == m_pBuyItem)
				{
					num = t->iNum;
					break;
				}
			}
			if (&*it == m_pBuyItem)
			{
				int cur = it->iNum;
				num = item->iNum;
				if (cur == num)
				{
					m_pMyItemList->DelItem(&*it);
					m_listMyItem.erase(it);
				}
				else if (cur > num)
				{
					it->iNum -= num;
				}
			}
			else
			{
				it->iNum += num - item->iNum;
			}
		}
	}
}

void FrTradeEditDlg::CancelSelectedTradeItem()
{
	if (m_pTradeItemList && m_pTradeItemList->GetSelected())
	{
		sTradeItem* item = (sTradeItem*)m_pTradeItemList->GetSelected()->pData;
		if (item)
		{
			if (item == m_pBuyItem)
			{
				SetTradeMyItem(NULL);
				SetControlsTradeMyItem(NULL, false);
			}
			AddListMyItem(item);
			DelListTradeItem(item);
			if (m_listTradeItem.size() <= 0)
			{
				if (Doc()->m_listTradeItem.size() > 0)
					SetControlsButton(false, true);
				else
					SetControlsButton(false, false);
			}
		}
	}
}

void FrTradeEditDlg::DrawPang()
{
	if (!g_pFresh->GetManager()->GetGDI() || !m_pFontPrice)
		return;

	char buf[32] = "";
	WRect rect = GetRect();
	static int s_fontWidth = m_pFontPrice->GetFontInfo()->fontw;
	static float s_width;
	MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
	s_width = (s_fontWidth - 1.0f) * (strlen(buf) - 1);
	m_pFontPrice->Print(g_view, rect.x + 130.0f - s_width, rect.y + 483.0f, buf,
		0, 0xffffffff, 0);
}

void FrTradeEditDlg::DrawItem(WPoint pos, sTradeItem* item, unsigned char flag)
{
	if (!item)
		return;

	unsigned long tid = item->dwTid;
	int num = item->iNum;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	if (tid == 0)
		return;

	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(tid);
	if (!common)
		return;

	float offX = 0.0f;
	float offY = 0.0f;
	const Bitmap* bitmap = g_pFresh->GetBitmap(common->Icon);
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item->status & 1) &&
		IsUccClothes(item->dwTid))
		bitmap = GetUccItemIcon(item->dwGuid);

	if (tid >> 26 == 2)
	{
		offX = -12.0f;
		offY = -20.0f;
	}
	if (bitmap)
	{
		WRect rect(pos.x + offX + 14.0f, pos.y + offY + 11.0f,
			(float)bitmap->Width(), (float)bitmap->Height());
		gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(4);
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
			IsUccClothes(item->dwTid))
		{
			if ((item->status & 1) && item->Seq == 1 && m_pBase[5])
			{
				gdi->DrawTexture(m_pBase[5],
					WRect(pos.x + 46.0f, pos.y + 48.0f,
						(float)m_pBase[5]->Width(),
						(float)m_pBase[5]->Height()),
					0xffffffff, 0);
			}
			else if ((item->status & 1) && item->Seq > 1 && m_pBase[6])
			{
				gdi->DrawTexture(m_pBase[6],
					WRect(pos.x + 46.0f, pos.y + 48.0f,
						(float)m_pBase[6]->Width(),
						(float)m_pBase[6]->Height()),
					0xffffffff, 0);
			}
			else if ((item->status & 2) && m_pBase[7])
			{
				gdi->DrawTexture(m_pBase[7],
					WRect(pos.x + 46.0f, pos.y + 48.0f,
						(float)m_pBase[7]->Width(),
						(float)m_pBase[7]->Height()),
					0xffffffff, 0);
			}
		}
		if (num > 1)
		{
			const char* text = MakeStr("%d", num);
			float x = pos.x + 54.0f;
			for (num = 0; num < strlen(text); ++num)
				x -= m_pFontTime->GetCharWidth(g_view, text[num] - '0');
			m_pFontTime->Print(g_view, x, pos.y + 42.0f, text, 0, 0xffffffff,
				NULL);
		}
	}

	switch (tid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 0x10:
	case 0x1c:
	{
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (common->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(pos.x + 38.0f -
					(float)float2int((float)level->Width() * 0.5f),
				pos.y + 35.0f - (float)float2int((float)level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, 0xffffffff, 0);
		}
		break;
	}
	}
}

void FrTradeEditDlg::SetControlsButton(bool bOpen, bool bClose)
{
	m_pOpenBn->Enable(bOpen);
	m_pCloseBn->Enable(bClose);
	m_pExitBn->Enable(true);
}

void FrTradeEditDlg::SetControlsTradeMyItem(sTradeItem* item, bool enable)
{
	if (m_pMyItemNumber)
		m_pMyItemNumber->ClearLine();
	if (m_pMyItemPrice)
		m_pMyItemPrice->ClearLine();
	if (!item)
	{
		m_pMyItemNumber->Enable(false);
		m_pMyItemPrice->Enable(false);
		m_pMyItemOkBn->Enable(false);
		m_pMyItemCancelBn->Enable(false);
		return;
	}
	m_pMyItemNumber->Enable(true);
	m_pMyItemPrice->Enable(true);
	m_pMyItemOkBn->Enable(true);
	m_pMyItemCancelBn->Enable(true);
	if (item->iNum == 1)
	{
		m_pMyItemNumber->SetLine(1, "1", 0, false, 0);
		m_pMyItemNumber->Enable(enable);
	}
	if (item->i64Price > 0)
	{
		m_pMyItemNumber->SetLine(1, MakeStr("%d", item->iNum), 0, false, 0);
		m_pMyItemPrice->SetLine(1, MakeStr("%I64d", item->i64Price), 0, false,
			0);
	}
}

// 983 (008f7900): 9 = sem itens (8 no 645), 0x13 = titulo em branco (0x11); codigo novo 8 = o titulo foi digitado na
// caixa mas ainda nao foi confirmado: quem chama manda o titulo e a loja abre quando o servidor responder.
int FrTradeEditDlg::CheckTradeEdit()
{
	memset(m_title, 0, sizeof(m_title));
	if (m_listTradeItem.size() <= 0)
		return 9;

	const char* title = Doc()->GetTradeTitle();
	if (strlen(title) >= 0x20)
		return 6;

	char buf[0x100] = "";
	strcpy(buf, title);
	if (Doc()->m_chatManager.Filtering(buf))
		return 7;

	int len = strlen(title);
	if (len == 0)
	{
		const char* docTitle = Doc()->GetTradeTitle();
		len = strlen(docTitle);
		if (len == 0)
		{
			if (strlen(m_pTitleEdit->GetLine(1, false)) != 0)
			{
				SetOpenAfterTitle983(true);
				return 8;
			}
			return 5;
		}
	}

	WTL::CString trimmed(title);
	trimmed.TrimRight();
	if (trimmed.GetLength() == 0)
		return 0x13;
	strcpy(m_title, title);
	return 1;
}

void FrTradeEditDlg::UpdateTradeTitle()
{
	if (m_pTitle)
	{
		m_pTitle->ClearLine();
		const char* title = Doc()->GetTradeTitle();
		m_pTitle->SetLine(1, title, 0, false, 0);
	}
}

void FrTradeEditDlg::UpdateTradeTitleEdit()
{
	if (m_pTitleEdit)
	{
		m_pTitleEdit->ClearLine();
		const char* title = Doc()->GetTradeTitle();
		m_pTitleEdit->SetLine(1, title, 0, false, 0);
	}
}

eResultTrade FrTradeEditDlg::SendTradeTitle()
{
	eResultTrade result = RESULT_TRADE_OK;
	if (m_pTitleEdit)
	{
		WTL::CString trimmed(m_pTitleEdit->GetLine(1, false));
		trimmed.TrimRight();
		if (trimmed.GetLength() == 0)
		{
			result = (eResultTrade)5;
		}
		else
		{
			const char* line = m_pTitleEdit->GetLine(1, false);
			int len = strlen(line);
			char buf[0x100] = "";
			strcpy(buf, line);
			if (Doc()->m_chatManager.Filtering(buf))
			{
				result = (eResultTrade)7;
			}
			else if (len > 0 && len < 0x20)
			{
				if (strcmp(line, Doc()->GetTradeTitle()) != 0)
					Doc()->SendTradeTitle(line);
			}
			m_pTitleEdit->ClearLine();
		}
	}
	return result;
}

IMPLEMENT_OBJECT(FrTradeFinishEditDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeFinishEditDlg, FrForm)

ON_FRESH_VI("tradefinisheditdlg", FRCMD_INIT,
	FrTradeFinishEditDlg::OnTradeFinishEditDlgInit)
ON_FRESH_VI("trade_item_list", FRCMD_INIT,
	FrTradeFinishEditDlg::OnTradeItemListInit)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	FrTradeFinishEditDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("userid", FRCMD_INIT, FrTradeFinishEditDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, FrTradeFinishEditDlg::OnTitleInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrTradeFinishEditDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrTradeFinishEditDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrTradeFinishEditDlg::FrTradeFinishEditDlg()
{
	m_pTradeItemList = NULL;
	m_pUserId = NULL;
	m_pTitle = NULL;
	m_pBase[0] = g_pFresh->GetBitmap("item_base_n");
	m_pBase[1] = g_pFresh->GetBitmap("item_base_o");
	m_pBase[2] = g_pFresh->GetBitmap("item_base_d");
	m_pBase[3] = g_pFresh->GetBitmap("item_base_empty");
	m_pBase[4] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pBase[5] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pBase[6] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	static char* s_fontWind[] = { "[font_wind.jpg" };
	WTITLEFONT font;
	memset(&font, 0, sizeof(font));
	font.filename = s_fontWind;
	font.fontw = 16;
	font.fonth = 16;
	font.numPages = 1;
	font.texw = 128;
	font.texh = 64;
	font.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&font);
	memset(m_title, 0, sizeof(m_title));
	m_pList = NULL;
	m_titleSource = false;
	m_clockIcon.pBitmap = g_pFresh->GetBitmap("clock");
	m_clockIcon.rect = WRect(50.0f, 35.0f, m_clockIcon.pBitmap->Width(),
		m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetBitmap("delete");
	m_deleteIcon.rect = WRect(50.0f, 35.0f, m_deleteIcon.pBitmap->Width(),
		m_deleteIcon.pBitmap->Height());
}

FrTradeFinishEditDlg::~FrTradeFinishEditDlg()
{
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrTradeFinishEditDlg::OnTradeFinishEditDlgInit(int param)
{
}

void FrTradeFinishEditDlg::OnUserIdInit(int param)
{
	m_pUserId = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeFinishEditDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrTradeFinishEditDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeFinishEditDlg::OnOkBnUp()
{
	int index = 0;
	if (m_pList->size() <= 0 || m_pList->size() > 6 || m_titleSource <= 0)
	{
		SetTradeResultCode(0, 0xe3, NULL);
		Doc()->SendTradeCloseShop();
	}
	else
	{
		if (m_titleSource == 2)
			Doc()->SendTradeTitle(m_title);
		WSendPacket packet((enumClientPacket)0x7c);
		packet.Encode4(m_pList->size());
		for (std::list<sTradeItem>::iterator it = m_pList->begin();
			it != m_pList->end(); ++it)
		{
			sTradeItem* tradeItem = &*it;
			tradeItem->iIndex = index;
			packet.EncodeBuffer(tradeItem, sizeof(sTradeItem));
			index++;
		}
		packet.Send(TO_GAME);
	}
	OnFreshOkay();
}

bool FrTradeFinishEditDlg::OnInit()
{
	EnableDrag(false);
	return true;
}

void FrTradeFinishEditDlg::OnTradeItemListInit(int param)
{
	m_pTradeItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrTradeFinishEditDlg::OnTradeItemListOwnerDraw(int param)
{
	FrListItem* listItem = (FrListItem*)param;
	if (!listItem)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	// 983 008f44a0: aviso em vermelho na posicao da janela (+130, +22/+37)
	float x = GetRect().x;
	float y = GetRect().y;
	gdi->SetTextColor(0xffff0000, 0xffffffff);
	float px = x + 130.0;
	gdi->Print(WPoint(px, y + 22.0), 0,
		K2L_Compatibility("\xc1\xa6""3\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb"));
	gdi->Print(WPoint(px, y + 37.0), 0,
		K2L_Compatibility("\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xc0\xd6\xb4\xc2 \xb0\xd4\xc0\xd3\xb3\xbb \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xb8\xc5\xb8\xc5\xb4\xc2 \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
	const Bitmap* base;
	if (!listItem->pData)
	{
		base = m_pBase[3];
	}
	else
	{
		if (listItem->selected)
		{
			base = m_pBase[2];
			gdi->SetTextColor(0xffff0000, 0xffffffff);
		}
		else if (listItem->underCursor)
		{
			base = m_pBase[1];
			gdi->SetTextColor(0xff000000, 0xffffffff);
		}
		else
		{
			base = m_pBase[0];
			gdi->SetTextColor(0xff000000, 0xffffffff);
		}
	}
	gdi->SetTextStyle(0);
	if (base)
	{
		unsigned long baseColor = 0xffffffff;
		WRect rect(listItem->pos.x, listItem->pos.y, (float)base->Width(),
			(float)base->Height());
		gdi->DrawTexture(base, rect, baseColor, 0);
	}
	if (!listItem->pData)
		return;
	sTradeItem item = *(sTradeItem*)listItem->pData;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(item.dwTid);
	if (!common)
		return;
	unsigned long type = item.dwTid >> 26;
	if (type == 18 &&
		(((item.dwTid >> 11) & 0x7ff) == 12 ||
			((item.dwTid >> 11) & 0x7ff) == 11))
		return;
	if (item.dwTid >> 26 == 2)
	{
		IFF_STRUCT::sChar* chara =
			ItemManager()->FindChar(((item.dwTid >> 18) & 0xff) | 0x4000000);
		if (chara)
		{
			const Bitmap* icon =
				g_pFresh->GetBitmap(MakeStr("s_%s", chara->c.Icon));
			if (icon)
			{
				WRect rect(listItem->pos.x + 3.0f, listItem->pos.y + 3.0f,
					(float)icon->Width(), (float)icon->Height());
				gdi->DrawTexture(icon, rect, 0xffffffff, 0);
			}
		}
	}

	const Bitmap* bitmap = g_pFresh->GetBitmap(common->Icon);
	if (bitmap)
	{
		float offX;
		float offY;
		switch (type)
		{
		default:
			offX = 16.0f;
			offY = 27.0f;
			break;
		case 2:
			offX = 7.0f;
			offY = 2.0f;
			break;
		}
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item.status & 1) &&
			IsUccClothes(item.dwTid))
			bitmap = GetUccItemIcon(item.dwGuid);
		if (bitmap)
		{
			WRect rect(offX + listItem->pos.x, offY + listItem->pos.y,
				(float)bitmap->Width(), (float)bitmap->Height());
			gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
		}

		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
			IsUccClothes(item.dwTid))
		{
			if ((item.status & 1) && item.Seq == 1 && m_pBase[4])
			{
				gdi->DrawTexture(m_pBase[4],
					WRect(listItem->pos.x + 49.0f, listItem->pos.y + 54.0f,
						(float)m_pBase[4]->Width(),
						(float)m_pBase[4]->Height()),
					0xffffffff, 0);
			}
			else if ((item.status & 1) && item.Seq > 1 && m_pBase[5])
			{
				gdi->DrawTexture(m_pBase[5],
					WRect(listItem->pos.x + 49.0f, listItem->pos.y + 54.0f,
						(float)m_pBase[5]->Width(),
						(float)m_pBase[5]->Height()),
					0xffffffff, 0);
			}
			else if ((item.status & 2) && m_pBase[6])
			{
				gdi->DrawTexture(m_pBase[6],
					WRect(listItem->pos.x + 49.0f, listItem->pos.y + 54.0f,
						(float)m_pBase[6]->Width(),
						(float)m_pBase[6]->Height()),
					0xffffffff, 0);
			}
		}
	}

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && (item.status & 1) &&
		IsUccClothes(item.dwTid))
	{
		const char* name = GetUccItemName(item.dwGuid);
		gdi->SetTextStyle(1);
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 7.0f), 2,
			name);
		gdi->SetTextStyle(0);
	}
	else
	{
		gdi->SetTextStyle(1);
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 7.0f), 2,
			common->Name);
		gdi->SetTextStyle(0);
	}

	char buf[32] = "";
	MakePriceStrA(buf, item.i64Price);
	SetTextColorByPrice(item.i64Price);
	int count = item.iNum;
	if (count > 1)
	{
		const char* text = MakeStr("%d", count);
		float x = listItem->pos.x + 54.0f;
		for (count = 0; count < strlen(text); ++count)
			x -= m_pFont->GetCharWidth(g_view, text[count] - '0');
		m_pFont->Print(g_view, x, listItem->pos.y + 55.0f, text, 0, 0xffffffff,
			NULL);
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 23.0f), 2,
			MakeStr(K2L_Compatibility("\xb0\xb3\xb4\xe7 %s\xc6\xce"), buf));
	}
	else
	{
		gdi->Print(WPoint(listItem->pos.x + 182.0f, listItem->pos.y + 23.0f), 2,
			MakeStr(K2L_Compatibility("%s\xc6\xce"), buf));
	}
	gdi->SetTextColor(0xff000000, 0xffffffff);
	DrawAbility(item, listItem);

	switch (item.dwTid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 16:
	case 28:
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (common->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(listItem->pos.x + 40.0f -
					(float)float2int((float)level->Width() * 0.5f),
				listItem->pos.y + 51.0f -
					(float)float2int((float)level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, 0xffffffff, 0);
		}
		break;
	}
}

void FrTradeFinishEditDlg::SetTradeUserId()
{
	// HACK: dead reference to a late PCH inline; defers this function one wave (emission order).
	if (0)
		g_pFresh->GetManager();
	if (m_pUserId)
		m_pUserId->SetLine(1, K2L_Compatibility("\xb3\xaa\xc0\xc7 \xbb\xf3\xc1\xa1"), 0, false, 0);
}

void FrTradeFinishEditDlg::SetTradeTitle(char* title)
{
	if (!title)
		return;
	if (strlen(title) >= 0x20)
		return;
	m_titleSource = 0;
	int len = strlen(title);
	const char* docTitle = Doc()->GetTradeTitle();
	int docLen = strlen(docTitle);
	if (len > 0)
	{
		if (len < 0x20)
		{
			strcpy(m_title, title);
			m_titleSource = 2;
			goto setline;
		}
		if (docLen <= 0)
			goto setline;
	}
	else if (docLen <= 0)
	{
		m_titleSource = 0;
		return;
	}
	if (docLen < 0x20)
	{
		strcpy(m_title, Doc()->GetTradeTitle());
		m_titleSource = 1;
	}
setline:
	if (m_pTitle)
		m_pTitle->SetLine(1, m_title, 0, false, 0);
}

void FrTradeFinishEditDlg::SetListTradeItem(std::list<sTradeItem>* list)
{
	if (m_pTradeItemList)
	{
		m_pList = list;
		m_pTradeItemList->ClearItem();
		for (std::list<sTradeItem>::iterator it = m_pList->begin();
			it != m_pList->end(); ++it)
			m_pTradeItemList->AddItem(&*it);
	}
}

IMPLEMENT_OBJECT(FrTradeWarningDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeWarningDlg, FrForm)

ON_FRESH_VI("TradeWarningDlg", FRCMD_INIT,
	FrTradeWarningDlg::OnTradeWarningDlgInit)
ON_FRESH_VI("msg01", FRCMD_INIT, FrTradeWarningDlg::OnMsg01Init)
ON_FRESH_VI("msg02", FRCMD_INIT, FrTradeWarningDlg::OnMsg02Init)
ON_FRESH_VI("ok", FRCMD_INIT, FrTradeWarningDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrTradeWarningDlg::OnOkBnUp)
ON_FRESH_VI("cancel", FRCMD_INIT, FrTradeWarningDlg::OnCancelBnInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrTradeWarningDlg::OnCancelBnUp)

END_FRESH_MSGMAP()

FrTradeWarningDlg::FrTradeWarningDlg()
{
	m_pMsg01 = NULL;
	m_pMsg02 = NULL;
	m_type = 1;
}

FrTradeWarningDlg::~FrTradeWarningDlg()
{
}

void FrTradeWarningDlg::OnTradeWarningDlgInit(int param)
{
}

void FrTradeWarningDlg::OnMsg01Init(int param)
{
	m_pMsg01 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrTradeWarningDlg::OnMsg02Init(int param)
{
	m_pMsg02 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrTradeWarningDlg::OnOkBnInit(int param)
{
	m_pOkBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (008f7dc0): os codigos andaram 2 (0x14/0x15 fecham a loja, 0x16 cancela o item marcado) e nao ha a mensagem
// 0x273 ao gateway.
void FrTradeWarningDlg::OnOkBnUp()
{
	int type = m_type;
	if (type >= 0x14)
	{
		if (type > 0x15)
		{
			if (type == 0x16)
			{
				FrWnd* wnd =
					g_pFresh->GetManager()->GetDesktop()->FindChildByName(
						"tradeeditdlg");
				if (wnd == NULL)
				{
					__rtti_obj = NULL;
					OnFreshOkay();
					return;
				}
				((FrTradeEditDlg*)wnd)->CancelSelectedTradeItem();
				OnFreshOkay();
				return;
			}
		}
		else
		{
			Doc()->SendTradeCloseShop();
		}
	}
	OnFreshOkay();
}

void FrTradeWarningDlg::OnCancelBnInit(int param)
{
	m_pCancelBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeWarningDlg::OnCancelBnUp()
{
	OnFreshCancel();
}

void FrTradeWarningDlg::SetControls(int type)
{
	// HACK: dead reference to a late PCH inline; defers this function one wave (emission order).
	if (0)
		g_pFresh->GetManager();
	WPoint pos;
	FrWnd* wnd;
	if (type == 0)
	{
		m_pCancelBn->SetVisible(false);
		pos.x = m_rect.x + 92.0f;
		wnd = m_pOkBn;
	}
	else
	{
		m_pOkBn->SetVisible(true);
		m_pCancelBn->SetVisible(true);
		pos.x = m_rect.x + 50.0f;
		pos.y = m_rect.y + 137.0f;
		m_pOkBn->MoveWindow(pos);
		pos.x = m_rect.x + 135.0f;
		wnd = m_pCancelBn;
	}
	pos.y = m_rect.y + 137.0f;
	wnd->MoveWindow(pos);
}

// 983 (008f0180): numeracao propria (a do 645 anda 1 a partir do 8, 2 a partir do 0x11; o limite de 30 milhoes, o
// canal, a sala de conversa e a venda em pacote nao existem), todo texto passa por K2L_Compatibility, e ha 3 avisos
// novos (0x12 item na caixa de correio, 0x1d item que o jogador nao tem, 0x1e item vinculado).
void FrTradeWarningDlg::SetWarningMessageType(int type, unsigned char flag)
{
	m_type = type;
	m_pMsg01->SetTextColor(0xff000000, 0xffffffff);
	switch (m_type)
	{
	case 0x0:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbf\xc0\xb7\xf9"));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xfc\xb7\xc3\xb5\xc8 \xb5\xa5\xc0\xcc\xc5\xcd\xb0\xa1 \xc3\xca\xb1\xe2\xc8\xad\xb5\xcb\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x2:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbf\xc0\xb7\xf9"));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xc0\xcc\xb9\xcc \xbb\xf3\xc1\xa1\xc0\xcc \xb0\xb3\xbc\xb3\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x3:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbb\xf3\xc1\xa1\xc0\xcc \xb4\xdd\xc7\xfb\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xd9\xc0\xbd \xb1\xe2\xc8\xb8\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."));
		SetControls(0);
		return;
	case 0x4:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbb\xf3\xc1\xa1\xc0\xcc \xc6\xed\xc1\xfd\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xbc\xb3 \xc8\xc4 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."));
		SetControls(0);
		return;
	case 0x5:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xb8\xed\xc0\xcc \xc0\xd4\xb7\xc2\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xc0\xd4\xb7\xc2 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x6:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xb8\xed\xc0\xcc \xc0\xdf\xb8\xf8\xc0\xd4\xb7\xc2\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xc1\xa4\xbb\xf3 \xc0\xd4\xb7\xc2 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x13:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbb\xf3\xc1\xa1\xb8\xed\xc0\xbb \xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc5 \xc8\xc4 \xc8\xae\xc0\xce \xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf\xbc\xad"));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9\xc7\xd1 \xbb\xf3\xc1\xa1\xb8\xed\xc0\xce\xc1\xf6 \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."));
		SetControls(0);
		return;
	case 0x7:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xb8\xed\xbf\xa1 \xc0\xfb\xb4\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba"));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xdc\xbe\xee\xb0\xa1 \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x9:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb5\xee\xb7\xcf\xc7\xcf\xbd\xc5 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xc0\xbb \xb0\xb3\xbc\xb3\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0xa:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbc\xd2\xc0\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xc0\xbb \xb0\xb3\xbc\xb3\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0xb:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbe\xc6\xc0\xcc\xc5\xdb \xbc\xf6\xb7\xae\xc0\xcc \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xd9\xbd\xc3 \xc7\xd1\xb9\xf8 \xc8\xae\xc0\xce\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0xc:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbe\xc6\xc0\xcc\xc5\xdb \xb0\xa1\xb0\xdd\xc0\xcc \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xd9\xbd\xc3 \xc7\xd1\xb9\xf8 \xc8\xae\xc0\xce\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0xe:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xba\xbb \xbb\xf3\xc7\xb0\xc0\xba \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xd9\xb8\xa5 \xbb\xf3\xc7\xb0\xc0\xbb \xb1\xb8\xb8\xc5\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."));
		SetControls(0);
		return;
	case 0xf:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbb\xf3\xc1\xa1 \xc0\xcc\xbf\xeb\xc0\xda\xb0\xa1 \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x14:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb5\xee\xb7\xcf\xb5\xc8 \xb8\xf0\xb5\xe7 \xb9\xb0\xc7\xb0\xc0\xcc \xc3\xeb\xbc\xd2\xb5\xcb\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xc0\xbb \xb4\xdd\xc0\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
		SetControls(1);
		return;
	case 0x15:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb8\xf0\xb5\xe7 \xbb\xf3\xc7\xb0\xc0\xcc \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1\xc0\xbb \xb4\xdd\xc0\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
		SetControls(1);
		return;
	case 0x16:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbc\xb1\xc5\xc3\xc7\xcf\xbd\xc5 \xb9\xb0\xc7\xb0\xc0\xbb \xb5\xee\xb7\xcf"));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xc3\xeb\xbc\xd2\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
		SetControls(1);
		return;
	case 0x17:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbc\xd2\xc1\xf6\xc7\xcf\xbd\xc5 \xb1\xdd\xbe\xd7\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb9\xb0\xc7\xb0\xc0\xbb \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x18:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb8\xf0\xb5\xe7 \xbb\xf3\xc7\xb0\xc0\xcc \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xc0\xcc\xbf\xeb\xc7\xd8 \xc1\xd6\xbc\xc5\xbc\xad \xb0\xa8\xbb\xe7\xc7\xd5\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x10:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(flag
				? "\xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1 \xc0\xcc\xbf\xeb \xc1\xa6\xc0\xe7 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."
				: "\xc0\xcc\xbf\xeb \xc1\xa6\xc0\xe7\xc1\xdf\xc0\xce \xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1\xc0\xd4\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x1a:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb7\xc1\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb3\xca\xb9\xab \xb8\xb9\xc0\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pMsg02->SetCaption(K2L_Compatibility(
			"\xb4\xf5 \xc0\xcc\xbb\xf3 \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x11:
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x12:
		// 983: sem SetControls (008f04e4)
		m_pMsg01->SetTextColor(0xffff0000, 0xffffffff);
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xc0\xcc \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xec\xc6\xed\xc7\xd4\xbf\xa1 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		return;
	case 0x1d:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	case 0x1e:
		m_pMsg01->SetCaption(K2L_Compatibility(
			"\xb1\xcd\xbc\xd3\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9."));
		SetControls(0);
		return;
	default:
		OnFreshCancel();
		return;
	}
}

void FrTradeWarningDlg::SetWarningMessageForPackage(int type)
{
	if (type >= 1900001 && type <= 0x1cfe07)
	{
		const char* msg = MakeStr(
			"\xb9\xad\xc0\xbd\xc6\xc7\xb8\xc5 \xbf\xc0\xb7\xf9 [%d]", type);
		m_pMsg01->SetCaption(msg);
		SetControls(0);
		return;
	}
	SetWarningMessageType(type, 0);
}

IMPLEMENT_OBJECT(FrTradeLevelLimitDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeLevelLimitDlg, FrForm)

ON_FRESH_VI("ok", FRCMD_INIT, FrTradeLevelLimitDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrTradeLevelLimitDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrTradeLevelLimitDlg::FrTradeLevelLimitDlg()
{
}

FrTradeLevelLimitDlg::~FrTradeLevelLimitDlg()
{
}

void FrTradeLevelLimitDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTradeLevelLimitDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrTradeLevelLimitDlg::OnInit()
{
	return true;
}

// 983 (008f65f0): aviso de manutencao 0x10 (0xf no 645), resultado da compra pelo pacote 0xec (0xea), limite de
// nivel 0x19 (0x17); os dois textos do chat passam por K2L_Compatibility.
void __fastcall SetTradeResultCode(int code, int type, sTradeItem* item)
{
	if (code == 0x10)
	{
		FrWnd* wnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"tradewarningdlg");
		if (wnd == NULL)
		{
			__rtti_obj = NULL;
			FrTradeWarningDlg* dlg = CreateForm<FrTradeWarningDlg>(
				g_pFresh->GetManager(), NULL, "tradewarningdlg", NULL);
			if (dlg)
			{
				dlg->Open(NULL, 1);
				dlg->SetWarningMessageType(0x10, (unsigned char)type);
				dlg->SetFadeout(false);
				return;
			}
		}
	}
	else if (code == 1)
	{
		if (type == 0xec)
		{
			CItemStorage* storage = WSingleton<CItemStorage>::Instance();
			if (!IsLocalContent(S4_ITEM_STORAGE) || storage == NULL ||
				!storage->IsVisible983())
			{
				FrTradeResultDlg* dlg = CreateForm<FrTradeResultDlg>(
					g_pFresh->GetManager(), NULL, "traderesultdlg", NULL);
				if (dlg)
				{
					dlg->Open(NULL, 1);
					dlg->SetFadeout(false);
					dlg->SetResultItem(item);
					return;
				}
			}
		}
	}
	else if (code == 0x19)
	{
		FrTradeLevelLimitDlg* dlg = CreateForm<FrTradeLevelLimitDlg>(
			g_pFresh->GetManager(), NULL, "tradelevellimitdlg", NULL);
		if (dlg)
		{
			dlg->Open(NULL, 0x40);
		}
	}
	else
	{
		FrWnd* wnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"traderesultdlg");
		if (wnd == NULL)
		{
			__rtti_obj = NULL;
			wnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"tradewarningdlg");
			if (wnd == NULL)
			{
				__rtti_obj = NULL;
				FrTradeWarningDlg* dlg = CreateForm<FrTradeWarningDlg>(
					g_pFresh->GetManager(), NULL, "tradewarningdlg", NULL);
				if (dlg)
				{
					dlg->Open(NULL, 1);
					dlg->SetWarningMessageType(code, 0);
					dlg->SetFadeout(false);
				}
			}
		}
		else if (code == 3)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(K2L_Compatibility(
					"\xbb\xf3\xc1\xa1\xc0\xcc \xb4\xdd\xc7\xfb\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xc0\xbd \xb1\xe2\xc8\xb8\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.")),
				0xffff, false, false);
		}
		else if (code == 4)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(K2L_Compatibility(
					"\xbb\xf3\xc1\xa1\xc0\xcc \xc6\xed\xc1\xfd\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xb0\xb3\xbc\xb3 \xc8\xc4 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.")),
				0xffff, false, false);
		}
	}
}

// VC10: sem as especializações de list<sTradeItem>::begin/end do VC7.1

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?EnableButton983@CLobbyState983a@@QAEXV?$basic_string@_WU?$char_traits@_W@std@@V?$allocator@_W@2@@std@@H@Z=?EnableButton983@RFLobbyDesktop@@QAEXV?$basic_string@_WU?$char_traits@_W@std@@V?$allocator@_W@2@@std@@_N@Z") // mesma VA 005fa910; int x bool
#pragma comment(linker, "/alternatename:?ms_pInstance@CLobbyState983a@@2PAV1@A=?ms_pInstance@CLobbyState983@@2PAV1@A") // dado, mesma VA 00e78ca8
