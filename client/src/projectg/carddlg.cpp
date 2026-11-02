#include "minatl.h"
#define REBANG_CARDDLG_CPP
#include "carddlg.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10
#include "ccardbook.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frstatic.h"
#include "frlistbox.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "shareddoc.h"
#include "uccmanager.h"
#include "ucclibrary.h"
#include "netresourcemanager.h"
#include "commonutil.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "wview.h"
#include "projectg.h"
#include "jp/shareddocjp.h"
#include "jp/lobbyjp.h"
#include "msg983.h"
#include <atltime.h>
#include "jp/docofs983.h" // D864: crus do CSharedDoc ancorados no membro (VS 2022)
namespace S5 { void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); } // 00b307f0
// layout do 983 (FrItemAttachCardDlg: sizeof 0x3b0 por alinhamento 16)
#define CARD_OFF(m, o) typedef char CardOff_##m[(offsetof(FrItemAttachCardDlg, m) == (o)) ? 1 : -1]
#ifdef REBANG_LAYOUT_CHECK
void FrItemAttachCardDlg::CheckLayout983()
{
	CARD_OFF(m_slot, 0x124);
	CARD_OFF(m_slots, 0x150);
	CARD_OFF(m_pSelectParts, 0x33c);
	CARD_OFF(m_pSelectClub, 0x340);
	CARD_OFF(m_selectMode983, 0x344);
	CARD_OFF(m_pCardAttachDlg, 0x34c);
	CARD_OFF(m_pSelectedItem, 0x358);
	CARD_OFF(m_myItemList, 0x35c);
	CARD_OFF(m_equipParts, 0x368);
	CARD_OFF(m_cardSlotMove, 0x374);
	CARD_OFF(m_pEmptySlotBmp, 0x388);
	CARD_OFF(m_bItemListFolded, 0x398);
	CARD_OFF(m_pUccStateBmp, 0x39c);
	typedef char CardSize[(sizeof(FrItemAttachCardDlg) == 0x3b0) ? 1 : -1];
}
#endif



// HACK: the original emits the "%s + %d" literal with FrCardUseDlg::OnItemOwnerDraw (its first user, now naked asm),
// so the literal is a local static of that function and the C users reach it through an alias.
#define RB_F91 91.0f
#define STAT_PLUS_FMT "%s + %d"

extern Fresh* g_pFresh;
int float2int(float f);

// sItemInfo names its guid field differently from the types _FindByGuid is used with elsewhere.
template <>
inline bool _FindByGuid::operator()(const sItemInfo& t) const
{
	return m_guid == t.guid;
}

unsigned long FrSPCardBuffDlg::m_SpecialBuffStartTick;

IMPLEMENT_OBJECT(FrSpCardUseDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrSpCardUseDlg, FrForm)
ON_FRESH_VI("item", FRCMD_INIT, FrSpCardUseDlg::OnItemInit)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrSpCardUseDlg::OnItemOwnerDraw)
ON_FRESH_VI("text1", FRCMD_INIT, FrSpCardUseDlg::OnText1Init) // 983 00e1d508: 3 entradas, sem "text2"
END_FRESH_MSGMAP()

FrSpCardUseDlg::FrSpCardUseDlg()
{
	m_spCardTid = 0;
	m_overlappedCard = 0;
}

FrSpCardUseDlg::~FrSpCardUseDlg()
{
}

void FrSpCardUseDlg::SetSpCardInfo(unsigned long& spCardTid)
{
	m_spCardTid = spCardTid;
	m_overlappedCard = GetOverlappedCard();
}

unsigned long FrSpCardUseDlg::GetOverlappedCard()
{
	// A special card of the same effect already running (not this card,
	// not expired) is the one the new card would overlap.
	for (std::list<sSCardAvilityPeriodInfo>::iterator it =
			 CSharedDoc::Instance()->m_cardAbilityList[0].begin();
		it != CSharedDoc::Instance()->m_cardAbilityList[0].end(); ++it)
	{
		if ((*it).tid == m_spCardTid)
		{
			continue;
		}

		if ((*it).cardType != 2)
		{
			continue;
		}

		if (ItemManager()->CompareSystemTime(
				CSharedDoc::Instance()->GetServerTime(), (*it).useEndTime) == 1)
		{
			continue;
		}

		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_spCardTid);

		if ((*it).Avility == pCard->Avility)
		{
			return (*it).tid;
		}
	}

	return 0;
}

void FrSpCardUseDlg::OnItemInit(int param)
{
	m_pItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrSpCardUseDlg::OnItemOwnerDraw(int nIndex)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	char desc[512] = { 0 };

	if (m_overlappedCard != 0)
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 11.0f),
			0,
			K2L_Compatibility("\xC7\xF6\xC0\xE7 \xC0\xFB\xBF\xEB\xC1\xDF\xC0\xCE \xC4\xAB\xB5\xE5 \xC8\xBF\xB0\xFA\xB0\xA1 \xBB\xE8\xC1\xA6\xB5\xC7\xB0\xED"));
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 28.0f),
			0,
			K2L_Compatibility("\xBB\xF5\xB7\xCE\xBF\xEE \xC4\xAB\xB5\xE5 \xC8\xBF\xB0\xFA\xB0\xA1 \xC0\xFB\xBF\xEB\xB5\xCB\xB4\xCF\xB4\xD9."));

		IFF_STRUCT::sCard* pOldCard = ItemManager()->FindCard(m_overlappedCard);
		if (pOldCard == 0)
		{
			return;
		}

		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(1);
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 50.0f),
			0, "%s", pOldCard->c.Name);
		pGDI->SetTextStyle(0);

		IFF_STRUCT::sDesc* pOldDesc = ItemManager()->FindDesc(m_overlappedCard);
		if (pOldDesc != 0)
		{
			strcpy(desc, pOldDesc->Desc);

			for (int i = 0; i < 512; i++)
			{
				if (desc[i] == '\n')
				{
					desc[i] = 0;
					break;
				}
			}

			pGDI->Print(WPoint(m_pItem->GetRect().x + 20.0f,
							m_pItem->GetRect().y + 70.0f),
				0, "%s", desc);
		}

		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_spCardTid);
		if (pCard == 0)
		{
			return;
		}

		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(1);
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 110.0f),
			0, "%s", pCard->c.Name);
		pGDI->SetTextStyle(0);

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_spCardTid);
		if (pDesc != 0)
		{
			strcpy(desc, pDesc->Desc);

			for (int i = 0; i < 512; i++)
			{
				if (desc[i] == '\n')
				{
					desc[i] = 0;
					break;
				}
			}

			pGDI->Print(WPoint(m_pItem->GetRect().x + 20.0f,
							m_pItem->GetRect().y + 130.0f),
				0, "%s", desc);
		}

		const Bitmap* pArrow = g_pFresh->GetBitmap("Overlapping_Arrow");
		if (pArrow != 0)
		{
			WRect rect(m_pItem->GetRect().x + 90.0f,
				m_pItem->GetRect().y + 90.0f, (float)pArrow->Width(),
				(float)pArrow->Height());
			pGDI->DrawTexture(pArrow, rect, 0xFFFFFFFF, 0);
		}
	}
	else
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 11.0f),
			0,
			"\xC0\xCC \xC4\xAB\xB5\xE5\xB8\xA6 \xBB\xE7\xBF\xEB\xC7\xD8\xBC\xAD \xBE\xC6\xC0\xCC\xC5\xDB\xBF\xA1");
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 28.0f),
			0,
			K2L_Compatibility("\xB4\xD9\xC0\xBD \xC6\xAF\xBC\xF6\xC8\xBF\xB0\xFA\xB8\xA6 \xBE\xF2\xC0\xB8\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"));

		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_spCardTid);
		if (pCard == 0)
		{
			return;
		}

		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->SubIcon);
		if (pBitmap != 0)
		{
			// 983 00802ee0: icone em (+220, +10) (como o FrCardUseDlg)
			WRect rect(m_pItem->GetRect().x + 220.0f,
				m_pItem->GetRect().y + 10.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		}

		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(1);
		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 50.0f),
			0, "%s", pCard->c.Name);
		pGDI->SetTextStyle(0);

		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_spCardTid);
		if (pDesc != 0)
		{
			strcpy(desc, pDesc->Desc);

			for (int i = 0; i < 512; i++)
			{
				if (desc[i] == '\n')
				{
					desc[i] = 0;
					break;
				}
			}

			pGDI->Print(WPoint(m_pItem->GetRect().x + 20.0f,
							m_pItem->GetRect().y + 70.0f),
				0, "%s", desc);
		}
	}
}

void FrSpCardUseDlg::OnText1Init(int param)
{
	m_pText1 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);

	if (m_pText1)
	{
		m_pText1->Enable(false);
		m_pText1->SetVisible(false);
	}
}

IMPLEMENT_OBJECT(FrCardUseDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrCardUseDlg, FrForm)
ON_FRESH_VI("item", FRCMD_INIT, FrCardUseDlg::OnItemInit)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrCardUseDlg::OnItemOwnerDraw)
ON_FRESH_VI("card_desc", FRCMD_INIT, FrCardUseDlg::OnCardDescInit)
END_FRESH_MSGMAP()

FrCardUseDlg::FrCardUseDlg()
{
	m_tid = 0;
	m_guid = 0;

	m_partsGuid = 0;
	m_partsTid = 0;
}

FrCardUseDlg::~FrCardUseDlg()
{
}

void FrCardUseDlg::OnItemInit(int param)
{
	m_pItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrCardUseDlg::OnCardDescInit(int param)
{
	m_pCardDesc = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

// 983 00803b70: cartas tipo 1/2/5 poem a descricao do IFF no "card_desc", linha a linha
void FrCardUseDlg::SetCardDesc983()
{
	char desc[512] = { 0 };
	unsigned long cardType = (m_tid >> 22) & 0xf;
	if (cardType == 2 || cardType == 1 || cardType == 5)
	{
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_tid);
		if (pDesc != 0)
		{
			strcpy(desc, pDesc->Desc);
			char line[128] = { 0 };
			int start = 0;
			for (int i = 0; i < 512; i++)
			{
				if (desc[i] == '\n' || desc[i] == '\0')
				{
					memset(line, 0, sizeof(line));
					strncpy(line, desc + start, i - start);
					m_pCardDesc->AddText(line, false, true, 0, 0);
				}
				if (desc[i] == '\0')
					break;
				if (desc[i] == '\n')
					start = i + 1;
			}
		}
	}
}

bool FrCardUseDlg::OnInit()
{
	SetCardDesc983();
	return FrForm::OnInit();
}

void FrCardUseDlg::OnItemOwnerDraw(int nIndex)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	char desc[512] = { 0 };
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_tid);
	if (pCard == 0)
	{
		return;
	}

	int cardType = (m_tid >> 22) & 0xF;
	const Bitmap* pBitmap =
		g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->SubIcon);
	if (pBitmap != 0 && cardType != 0)
	{
		WRect rect(m_pItem->GetRect().x + 220.0f, m_pItem->GetRect().y + 10.0f,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
	}

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	pGDI->Print(
		WPoint(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 50.0f), 0,
		"%s", pCard->c.Name);
	pGDI->SetTextStyle(0);

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(0);

	if (cardType == 1 || cardType == 5)
	{
		// 983: so copia a descricao (nao desenha)
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_tid);
		if (pDesc != 0)
		{
			strcpy(desc, pDesc->Desc);
		}
	}
	else if (cardType == 0)
	{
		static const char* const iconNames[5] = { "card_sub_char_07",
			"card_sub_char_08", "card_sub_char_09", "card_sub_char_10",
			"card_sub_char_06" };
		float iconOffset = 0.0f;
		float offset = 0.0f;
		for (int i = 0; i < 5; i++)
		{
			if (pCard->COM[i] > 0)
			{
				char icon[256] = { 0 };

				if (i == 0)
				{
					sprintf(desc, "%s + %d",
						K2L_Compatibility("\xC6\xC4\xBF\xF6\xBD\xBD\xB7\xD4"),
						pCard->COM[0]);
				}
				else if (i == 1)
				{
					sprintf(desc, "%s + %d",
						K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1\xBD\xBD\xB7\xD4"),
						pCard->COM[1]);
				}
				else if (i == 2)
				{
					sprintf(desc, "%s + %d",
						K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5\xBD\xBD\xB7\xD4"),
						pCard->COM[2]);
				}
				else if (i == 3)
				{
					sprintf(desc, "%s + %d",
						K2L_Compatibility("\xBD\xBA\xC7\xC9\xBD\xBD\xB7\xD4"),
						pCard->COM[3]);
				}
				else if (i == 4)
				{
					sprintf(desc, "%s + %d",
						K2L_Compatibility("\xC4\xBF\xBA\xEA\xBD\xBD\xB7\xD4"),
						pCard->COM[4]);
				}
				sprintf(icon, "%s", iconNames[i]);
				iconOffset += 1.0f;

				const Bitmap* pIcon =
					g_pFresh->GetManager()->GetBitmap("ITEMS", icon);
				if (pIcon != 0)
				{
					WRect rect(m_pItem->GetRect().x + 30.0f + iconOffset * 20.0f,
						m_pItem->GetRect().y + 100.0f, (float)pIcon->Width(),
						(float)pIcon->Height());
					pGDI->DrawTexture(pIcon, rect, 0xFFFFFFFF, 0);
				}

				pGDI->Print(WPoint(m_pItem->GetRect().x + 20.0f,
								m_pItem->GetRect().y + (offset + 70.0f)),
					0, "%s", desc);
				offset += 15.0f;
			}
		}

		if (pCard->Avility == 1)
		{
			sprintf(desc, "%s - %d",
				K2L_Compatibility("\xBA\xF1\xB0\xC5\xB8\xAE"),
				pCard->AvilityValue);
			pGDI->Print(WPoint(m_pItem->GetRect().x + 20.0f,
							m_pItem->GetRect().y + (offset + 70.0f)),
				0, "%s", desc);
		}
	}
}

IMPLEMENT_OBJECT(FrOpenCardPackDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrOpenCardPackDlg, FrForm)
ON_FRESH_VI("item", FRCMD_INIT, FrOpenCardPackDlg::OnItemInit)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrOpenCardPackDlg::OnItemOwnerDraw)
ON_FRESH_VI("levelImg", FRCMD_INIT, FrOpenCardPackDlg::OnLevelImgInit)
END_FRESH_MSGMAP()

FrOpenCardPackDlg::FrOpenCardPackDlg()
{
	m_tid = 0;
	m_guid = 0;
}

FrOpenCardPackDlg::~FrOpenCardPackDlg()
{
}

void FrOpenCardPackDlg::OnLevelImgInit(int param)
{
	m_pLevelImg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrOpenCardPackDlg::OnItemInit(int param)
{
	m_pItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrOpenCardPackDlg::OnItemOwnerDraw(int nIndex)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	char desc[512] = { 0 };
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_tid);
	if (pCard == 0)
	{
		return;
	}

	int cardType = (m_tid >> 22) & 0xF;
	const Bitmap* pBitmap =
		g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->c.Icon);
	if (pBitmap != 0)
	{
		WRect rect(m_pItem->GetRect().x + 20.0f, m_pItem->GetRect().y + 50.0f,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
	}

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	pGDI->Print(
		WPoint(m_pItem->GetRect().x + 210.0f, m_pItem->GetRect().y + 55.0f), 0, // 983 00803cd0: nome em (+210, +55)
		"%s", pCard->c.Name);
	pGDI->SetTextStyle(0);

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	pGDI->Print(
		WPoint(m_pItem->GetRect().x + 160.0f, m_pItem->GetRect().y + 55.0f), 0, // 983: (+160, +55)
		"%s", K2L_Compatibility("\xC0\xCC\xB8\xA7: "));
	pGDI->SetTextStyle(0);

	char text[100] = { 0 };

	// The level text goes through a pointer (the original copies it byte by byte).
	if (pCard->c.IsUnderLvl)
	{
		const char* levelText =
			K2L_Compatibility("\xB7\xB9\xBA\xA7:                \xC0\xCC\xC7\xCF");
		strcpy(text, levelText);
	}
	else
	{
		const char* levelText =
			K2L_Compatibility("\xB7\xB9\xBA\xA7:                \xC0\xCC\xBB\xF3");
		strcpy(text, levelText);
	}

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	pGDI->Print(
		WPoint(m_pItem->GetRect().x + 160.0f, m_pItem->GetRect().y + 75.0f), 0, // 983: (+160, +75)
		"%s", text);
	pGDI->SetTextStyle(0);

	if (m_pLevelImg != 0)
	{
		m_pLevelImg->SetBgImg(MakeStr("level_%03d", pCard->c.Level + 1));
	}

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(0);

	if (cardType == 3)
	{
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_tid);
		if (pDesc == 0)
		{
			return;
		}

		strcpy(desc, pDesc->Desc);

		for (int i = 0; i < 512; i++)
		{
			if (desc[i] == '\n')
			{
				desc[i] = 0;
				break;
			}
		}

		pGDI->Print(
			WPoint(m_pItem->GetRect().x + 25.0f, m_pItem->GetRect().y + 155.0f),
			0, "%s", desc);
	}
}

IMPLEMENT_OBJECT(FrOpenCardPackResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrOpenCardPackResultDlg, FrForm)
ON_FRESH_VI("item", FRCMD_INIT, FrOpenCardPackResultDlg::OnItemInit)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrOpenCardPackResultDlg::OnItemOwnerDraw)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrOpenCardPackResultDlg::OnItemOwnerDraw)
ON_FRESH_VI("cardpack_frm_num", FRCMD_INIT,
	FrOpenCardPackResultDlg::OnCardFrmNumInit)
END_FRESH_MSGMAP()

FrOpenCardPackResultDlg::FrOpenCardPackResultDlg()
{
	for (int i = 0; i < 3; i++)
	{
		m_cardTid[i] = 0;
	}
}

FrOpenCardPackResultDlg::~FrOpenCardPackResultDlg()
{
}

void FrOpenCardPackResultDlg::OnItemInit(int param)
{
	m_pItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrOpenCardPackResultDlg::OnCardFrmNumInit(int param)
{
	FrArea* pFrmNum = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pCardFrmNum = pFrmNum;

	if (m_cardTid[1] && m_cardTid[2] && m_cardTid4)
	{
		pFrmNum->SetBgImg("cardpack_frm_num_3");
	}
	else
	{
		pFrmNum->SetBgImg("cardpack_frm_num_1");
	}
}

void FrOpenCardPackResultDlg::OnItemOwnerDraw(int nIndex)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	char buffer[32]; // unused; the original's frame keeps its /GS cookie

	if (m_cardTid[0] != 0)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_cardTid[0]);
		if (pCard == 0)
		{
			return;
		}

		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->c.Icon);
		if (pBitmap != 0)
		{
			WRect rect(m_pItem->GetRect().x + 30.0f,
				m_pItem->GetRect().y + 85.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		}

		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->PrintText(
			WPoint(m_pItem->GetRect().x + 16.0f, m_pItem->GetRect().y + 160.0f),
			0, MakeStr("%s", pCard->c.Name), 0);
	}

	if (m_cardTid[1] != 0)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_cardTid[1]);
		if (pCard == 0)
		{
			return;
		}

		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->c.Icon);
		if (pBitmap != 0)
		{
			WRect rect(m_pItem->GetRect().x + 126.0f,
				m_pItem->GetRect().y + 94.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		}
	}

	if (m_cardTid[2] != 0 && m_cardTid4 != 0)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_cardTid[2]);
		if (pCard == 0)
		{
			return;
		}

		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->c.Icon);
		if (pBitmap != 0)
		{
			WRect rect(m_pItem->GetRect().x + 178.0f,
				m_pItem->GetRect().y + 94.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		}

		pCard = ItemManager()->FindCard(m_cardTid4);
		if (pCard == 0)
		{
			return;
		}

		pBitmap = g_pFresh->GetManager()->GetBitmap("ITEMS", pCard->c.Icon);
		if (pBitmap != 0)
		{
			WRect rect(m_pItem->GetRect().x + 230.0f,
				m_pItem->GetRect().y + 94.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		}
	}
}

IMPLEMENT_OBJECT(FrCardAttachDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrCardAttachDlg, FrForm)
ON_FRESH_VI("item", FRCMD_INIT, FrCardAttachDlg::OnItemInit)
ON_FRESH_VI("item", FRCMD_OWNERDRAW, FrCardAttachDlg::OnItemOwnerDraw)
ON_FRESH_VI("cardavility", FRCMD_INIT, FrCardAttachDlg::OnCardAvilityInit)
ON_FRESH_VI("cardavilityold", FRCMD_INIT, FrCardAttachDlg::OnCardAvilityOldInit)
END_FRESH_MSGMAP()

FrCardAttachDlg::FrCardAttachDlg()
{
	m_cardTid = 0;
	m_cardGuid = 0;
	m_unused12c = 0;
	m_unused130 = 0;

	m_charSlot = 0;
	m_caddieSlot = 0;
	m_npcSlot = 0;

	m_pCardAvility = 0;
	m_pCardAvilityOld = 0;
	m_unused140 = 0;
}

FrCardAttachDlg::~FrCardAttachDlg()
{
}

// 983 00806a10: para taco, icone e quantidade do "clube card patch" (0x1a00018f) que o OnItemOwnerDraw 00804420 desenha
bool FrCardAttachDlg::OnInit()
{
	if ((m_partTid & 0xfc000000) == 0x10000000)
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(0x1a00018f);
		m_unused140 = (unsigned long)g_pFresh->GetBitmap(pItem->c.Icon);
		const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
		std::list<sItemInfo>::const_iterator it;
		for (it = items.begin(); it != items.end(); ++it)
		{
			if ((*it).typeId == 0x1a00018f)
				break;
		}
		if (it != items.end())
			m_str148 = MakeStr(K2L_Compatibility("%d\xb0\xb3"), (*it).Common[0]);
	}
	return FrForm::OnInit();
}

void FrCardAttachDlg::OnCardAvilityInit(int param)
{
	m_pCardAvility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrCardAttachDlg::OnCardAvilityOldInit(int param)
{
	m_pCardAvilityOld = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrCardAttachDlg::OnItemInit(int param)
{
	m_pItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 00804420: o 645 nao desenhava nada; o 983 escreve o aviso (-ÁÖÀÇ-) e, com o bitmap +0x140, a quantidade.
void FrCardAttachDlg::OnItemOwnerDraw(int nIndex)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	float x = m_rect.x;
	float y = m_rect.y;
	const Bitmap* bitmap = (const Bitmap*)m_unused140;
	if (bitmap)
	{
		WRect rect(x + 40.0f, y + 145.0f, (float)bitmap->Width(), (float)bitmap->Height());
		gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
		gdi->Print(WPoint(x + 90.0f, y + 155.0f), 0,
			K2L_Compatibility("\307\366\300\347\274\366\267\256"));
		gdi->Print(WPoint(x + 100.0f, y + 176.0f), 0, m_str148.c_str());
		gdi->Print(WPoint(x + 280.0f, y + 155.0f), 1,
			K2L_Compatibility("'\305\254\267\264 \304\253\265\345\306\320\304\241' "
			"\276\306\300\314\305\333\300\273 \273\347\277\353\307\317\277\251"));
		gdi->Print(WPoint(x + 280.0f, y + 176.0f), 1,
			K2L_Compatibility("\264\311\267\302\300\273 \271\331\262\331\275\303\260\332\275\300\264\317\261\356?"));
		y += 30.0f;
	}
	y += 175.0f;
	gdi->SetTextColor(0xffff0000, 0xffffffff);
	gdi->Print(WPoint(x + 190.0f, y + 10.0f), 1, K2L_Compatibility("-\301\326\300\307-"));
	gdi->Print(WPoint(x + 200.0f, y + 26.0f), 1,
		K2L_Compatibility("\300\314\271\314 \304\253\265\345\260\241 \300\345\302\370\265\310 "
		"\275\275\267\324\277\241 \273\365 \304\253\265\345\270\246"));
	gdi->Print(WPoint(x + 200.0f, y + 39.0f), 1,
		K2L_Compatibility("\265\244\276\356 \276\272\277\354\270\351 \261\342\301\270 "
		"\304\253\265\345\260\241 \273\347\266\363\301\366\260\324 \265\313\264\317\264\331."));
}

CTFrameMove::CTFrameMove()
	: m_count(3)
{
	m_frames.reserve(m_count);
}

CTFrameMove::CTFrameMove(unsigned char count)
	: m_count(count)
{
	m_frames.reserve(count);
}

CTFrameMove::~CTFrameMove()
{
	Reset();
}

void CTFrameMove::AddFrame(FrArea* pFrame)
{
	if (pFrame != 0)
	{
		m_frames.push_back(pFrame);
	}
}

bool CTFrameMove::SetFrameWidth(float width)
{
	unsigned char start;
	unsigned char end;

	if (0.0f != width)
	{
		unsigned char count = (unsigned char)m_frames.size();

		if (count == 3)
		{
			start = count;
			end = 6;
		}
		else if (count == 9)
		{
			start = 0;
			end = 3;
		}

		for (unsigned char i = start; i < end; i++)
		{
			WRect rect;
			rect = m_frames[i]->GetRect();
			rect.w += width;
			m_frames[i]->SetRect(rect);
		}

		return true;
	}

	return false;
}

bool CTFrameMove::SetFrameHeight(float height)
{
	if (0.0f != height && (unsigned char)m_frames.size() == 6)
	{
		unsigned char i;

		for (i = 0; i < 3; i++)
		{
			WRect rect;
			rect = m_frames[i]->GetRect();
			rect.h += height;
			m_frames[i]->SetRect(rect);
		}

		for (i = 3; i < 6; i++)
		{
			WRect rect;
			rect = m_frames[i]->GetRect();
			rect.y += height;
			m_frames[i]->SetRect(rect);
		}

		return true;
	}

	return false;
}

bool FrCardAttachDlg::OnSetDlgTxt()
{
	if (m_cardTid == 0)
	{
		return false;
	}

	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_cardTid);
	if (pCard == 0)
	{
		return false;
	}

	char text[512] = { 0 };
	int cardType = (m_cardTid >> 22) & 0xF;
	if (cardType == 0)
	{
		for (int i = 0; i < 5; i++)
		{
			if (pCard->COM[i] > 0)
			{
				if (i == 0)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC6\xC4\xBF\xF6\xBD\xBD\xB7\xD4"), pCard->COM[0]);
				}
				else if (i == 1)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1\xBD\xBD\xB7\xD4"),
						pCard->COM[1]);
				}
				else if (i == 2)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5\xBD\xBD\xB7\xD4"),
						pCard->COM[2]);
				}
				else if (i == 3)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xBD\xBA\xC7\xC9\xBD\xBD\xB7\xD4"), pCard->COM[3]);
				}
				else if (i == 4)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xBF\xBA\xEA\xBD\xBD\xB7\xD4"), pCard->COM[4]);
				}

				m_pCardAvility->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text),
					false, true);
			}
		}
		if (pCard->Avility == 1)
		{
			sprintf(text, "%s - %d", K2L_Compatibility("\xBA\xF1\xB0\xC5\xB8\xAE"), pCard->AvilityValue);
			m_pCardAvility->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text), false, true);
		}
	}
	else if (cardType == 1 || cardType == 5)
	{
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_cardTid);
		if (pDesc != 0)
		{
			strcpy(text, pDesc->Desc);

			for (int i = 0; i < 512; i++)
			{
				if (text[i] == '\n')
				{
					text[i] = 0;
					break;
				}
			}

			m_pCardAvility->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text), false,
				true);
		}
	}

	if ((int)m_charSlot > 0 || (int)m_caddieSlot > 0 || (int)m_npcSlot > 0)
	{
		std::list<sSCardAvilityPeriodInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_cardAbilityList[0].begin();
			it != CSharedDoc::Instance()->m_cardAbilityList[0].end(); ++it)
		{
			unsigned long oldTid = (*it).tid;
			int oldType = (oldTid >> 22) & 0xF;
			if ((*it).partsTid == m_partTid && (*it).partsUid == m_partGuid)
			{
				if (oldType == 0)
				{
					if (m_charSlot == (*it).slotNum)
					{
						IFF_STRUCT::sCard* pOldCard =
							ItemManager()->FindCard(oldTid);
						if (pOldCard != 0)
						{
							for (int i = 0; i < 5; i++)
							{
								if (pOldCard->COM[i] > 0)
								{
									memset(text, 0, sizeof(text));
									if (i == 0)
									{
										sprintf(text, STAT_PLUS_FMT,
											K2L_Compatibility("\xC6\xC4\xBF\xF6\xBD\xBD\xB7\xD4"),
											pOldCard->COM[0]);
									}
									else if (i == 1)
									{
										sprintf(text, STAT_PLUS_FMT,
											K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1\xBD\xBD\xB7\xD4"),
											pOldCard->COM[1]);
									}
									else if (i == 2)
									{
										sprintf(text, STAT_PLUS_FMT,
											K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5\xBD\xBD\xB7\xD4"),
											pOldCard->COM[2]);
									}
									else if (i == 3)
									{
										sprintf(text, STAT_PLUS_FMT,
											K2L_Compatibility("\xBD\xBA\xC7\xC9\xBD\xBD\xB7\xD4"),
											pOldCard->COM[3]);
									}
									else if (i == 4)
									{
										sprintf(text, STAT_PLUS_FMT,
											K2L_Compatibility("\xC4\xBF\xBA\xEA\xBD\xBD\xB7\xD4"),
											pOldCard->COM[4]);
									}

									m_pCardAvilityOld->AddText(
										MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text),
										false, true);
								}
							}
							if (pOldCard->Avility == 1)
							{
								sprintf(text, "%s - %d", K2L_Compatibility("\xBA\xF1\xB0\xC5\xB8\xAE"), pOldCard->AvilityValue);
								m_pCardAvilityOld->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text), false, true);
							}
						}
					}
				}
				else if (oldType == 1)
				{
					if (m_caddieSlot == (*it).slotNum)
					{
						IFF_STRUCT::sDesc* pDesc =
							ItemManager()->FindDesc(oldTid);
						if (pDesc != 0)
						{
							strcpy(text, pDesc->Desc);

							for (int i = 0; i < 512; i++)
							{
								if (text[i] == '\n')
								{
									text[i] = 0;
									break;
								}
							}

							m_pCardAvilityOld->AddText(
								MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text), false, true);
						}
					}
				}
				else if (oldType == 5)
				{
					if (m_npcSlot == (*it).slotNum)
					{
						IFF_STRUCT::sDesc* pDesc =
							ItemManager()->FindDesc(oldTid);
						if (pDesc != 0)
						{
							strcpy(text, pDesc->Desc);

							for (int i = 0; i < 512; i++)
							{
								if (text[i] == '\n')
								{
									text[i] = 0;
									break;
								}
							}

							m_pCardAvilityOld->AddText(
								MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), text), false, true);
						}
					}
				}
			}
		}
	}

	return true;
}

IMPLEMENT_OBJECT(FrItemAttachCardDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrItemAttachCardDlg, FrForm)
// 983 00e1d808 (52 entradas): charbtn_1-3, caddiebtn_1-3, npcbtn_1-2 (os _4 do 645 nao existem no layout do 983);
// clique das casas = FRCMD_LBUTTONUP (0x4); select_parts/select_club/ok = entradas 47-51.
ON_FRESH_VI("attach_item_list", FRCMD_INIT,
	FrItemAttachCardDlg::OnMyItemListInit)
ON_FRESH_VV("attach_item_list", FRCMD_LBUTTONDOWN,
	FrItemAttachCardDlg::OnMyItemListLClick)
ON_FRESH_VV("attach_item_list", FRCMD_RBUTTONDOWN,
	FrItemAttachCardDlg::OnMyItemListRClick)
ON_FRESH_VI("attach_item_list", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnMyItemListOwnerDraw)
ON_FRESH_VI("attach_card_icon", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachCardIconInit)
ON_FRESH_VV("attach_card_icon", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnAttachCardIconOwnerDraw)
ON_FRESH_VI("attach_card_name", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachCardNameInit)
ON_FRESH_VI("attach_card_ability", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachCardAbilityInit)
ON_FRESH_VI("attach_item_icon", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachItemIconInit)
ON_FRESH_VV("attach_item_icon", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnAttachItemIconOwnerDraw)
ON_FRESH_VI("attach_item_name", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachItemNameInit)
ON_FRESH_VI("attach_item_ability", FRCMD_INIT,
	FrItemAttachCardDlg::OnAttachItemAbilityInit)
ON_FRESH_VI("attach_char_img", FRCMD_INIT,
	FrItemAttachCardDlg::OnAreaCharImgInit)
ON_FRESH_VI("foldbtn", FRCMD_INIT, FrItemAttachCardDlg::OnItemListFoldInit)
ON_FRESH_VV("foldbtn", FRCMD_LBUTTONDOWN,
	FrItemAttachCardDlg::OnItemListFoldLClick)
ON_FRESH_VI("unfoldbtn", FRCMD_INIT, FrItemAttachCardDlg::OnItemListUnfoldInit)
ON_FRESH_VV("unfoldbtn", FRCMD_LBUTTONDOWN,
	FrItemAttachCardDlg::OnItemListUnfoldLClick)
ON_FRESH_VI("card_frm_04", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_04Init)
ON_FRESH_VI("card_frm_05", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_05Init)
ON_FRESH_VI("card_frm_06", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_06Init)
ON_FRESH_VI("card_frm_07", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_07Init)
ON_FRESH_VI("card_frm_08", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_08Init)
ON_FRESH_VI("card_frm_09", FRCMD_INIT, FrItemAttachCardDlg::OnDlgFrm_09Init)
ON_FRESH_VI("charbtn_1", FRCMD_INIT, FrItemAttachCardDlg::OnCharSlotBtnInit_1)
ON_FRESH_VV("charbtn_1", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCharSlotBtnDown_1)
ON_FRESH_VV("charbtn_1", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_1)
ON_FRESH_VI("charbtn_2", FRCMD_INIT, FrItemAttachCardDlg::OnCharSlotBtnInit_2)
ON_FRESH_VV("charbtn_2", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCharSlotBtnDown_2)
ON_FRESH_VV("charbtn_2", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_2)
ON_FRESH_VI("charbtn_3", FRCMD_INIT, FrItemAttachCardDlg::OnCharSlotBtnInit_3)
ON_FRESH_VV("charbtn_3", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCharSlotBtnDown_3)
ON_FRESH_VV("charbtn_3", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_3)
ON_FRESH_VI("caddiebtn_1", FRCMD_INIT, FrItemAttachCardDlg::OnCharSlotBtnInit_4)
ON_FRESH_VV("caddiebtn_1", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCharSlotBtnDown_4)
ON_FRESH_VV("caddiebtn_1", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_4)
ON_FRESH_VI("caddiebtn_2", FRCMD_INIT,
	FrItemAttachCardDlg::OnCaddieSlotBtnInit_1)
ON_FRESH_VV("caddiebtn_2", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCaddieSlotBtnDown_1)
ON_FRESH_VV("caddiebtn_2", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_1)
ON_FRESH_VI("caddiebtn_3", FRCMD_INIT,
	FrItemAttachCardDlg::OnCaddieSlotBtnInit_2)
ON_FRESH_VV("caddiebtn_3", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCaddieSlotBtnDown_2)
ON_FRESH_VV("caddiebtn_3", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_2)
ON_FRESH_VI("npcbtn_1", FRCMD_INIT,
	FrItemAttachCardDlg::OnCaddieSlotBtnInit_3)
ON_FRESH_VV("npcbtn_1", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCaddieSlotBtnDown_3)
ON_FRESH_VV("npcbtn_1", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_3)
ON_FRESH_VI("npcbtn_2", FRCMD_INIT,
	FrItemAttachCardDlg::OnCaddieSlotBtnInit_4)
ON_FRESH_VV("npcbtn_2", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnCaddieSlotBtnDown_4)
ON_FRESH_VV("npcbtn_2", FRCMD_OWNERDRAW,
	FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_4)
ON_FRESH_VI("select_parts", FRCMD_INIT, FrItemAttachCardDlg::OnSelectPartsInit)
ON_FRESH_VV("select_parts", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnSelectPartsLClick)
ON_FRESH_VI("select_club", FRCMD_INIT, FrItemAttachCardDlg::OnSelectClubInit)
ON_FRESH_VV("select_club", FRCMD_LBUTTONUP,
	FrItemAttachCardDlg::OnSelectClubLClick)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrItemAttachCardDlg::OnOkLClick983)
END_FRESH_MSGMAP()

void FrItemAttachCardDlg::DrawSlotTxt983(float x, float y, int idx, int kind)
{
	std::string txt;
	switch (kind)
	{
	case 0:
		txt = m_slots[idx].txt;
		break;
	case 1:
		txt = m_slots[3 + idx].txt;
		break;
	case 5:
		txt = m_slots[6 + idx].txt;
		break;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->Print11(WPoint(x + 20.0f, y + 6.0f), 0, "%s", txt.c_str());
}

void FrItemAttachCardDlg::DrawEmptySlot983(float x, float y, int idx)
{
	if (idx < 0)
		idx = 0;
	else if (idx > 3)
		idx = 3;
	const Bitmap* pBitmap = m_pEmptySlotBmp[idx];
	if (pBitmap)
	{
		g_pFresh->GetManager()->GetGDI()->DrawTexture(pBitmap,
			WRect(x + 5.0f, y + 4.0f, pBitmap->Width(), pBitmap->Height()),
			0xFFFFFFFF, 0);
	}
}


FrItemAttachCardDlg::FrItemAttachCardDlg()
{
	m_pMyItemList = 0;
	m_pAttachCardIcon = 0;
	m_pAttachCardName = 0;
	m_pAttachCardAbility = 0;
	m_pAttachItemIcon = 0;
	m_pAttachItemName = 0;
	m_pAttachItemAbility = 0;

	int i;
	for (i = 0; i < 3; i++)
	{
		m_slots[i].pBtn = 0;
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}
	for (i = 3; i < 6; i++)
	{
		m_slots[i].pBtn = 0;
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}
	for (i = 6; i < 8; i++)
	{
		m_slots[i].pBtn = 0;
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}

	m_cardTid = 0;
	m_cardGuid = 0;
	m_partTid = 0;
	m_partGuid = 0;

	m_pLineOverlay = g_resrcmng->GetOverlay("line.tga", 0);

	m_pCardAttachDlg = 0;

	m_pMyItemBaseClickBmp = g_pFresh->GetBitmap("trade_myitem_base_click");
	m_pMyItemBaseOverBmp = g_pFresh->GetBitmap("trade_myitem_base_over");

	m_myItemList.clear();

	m_pSelectedItem = 0;
	initCardSlotBtnTxt();

	m_slot = 0xFF;

	m_bItemListFolded = false;

	m_pEmptySlotBmp[0] = g_pFresh->GetBitmap("card_character_empty");
	m_pEmptySlotBmp[1] = g_pFresh->GetBitmap("card_caddie_empty");
	m_pEmptySlotBmp[2] = g_pFresh->GetBitmap("card_npc_empty");

	m_pUccStateBmp[0] = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccStateBmp[1] = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccStateBmp[2] = g_pFresh->RegisterBitmap("ucc_state_temp_saved");

	m_pSelectParts = 0;
	m_pSelectClub = 0;
	m_selectMode983 = 0;
}

FrItemAttachCardDlg::~FrItemAttachCardDlg()
{
	if (g_resrcmng && m_pLineOverlay)
	{
		g_resrcmng->Release(m_pLineOverlay);
		m_pLineOverlay = 0;
	}
}

bool FrItemAttachCardDlg::OnInit()
{
	EnableDrag(false);

	SetListCardUsableMyItem();
	SetListMyItem();
	SetSelectMyItem(0);
	SetCardInfo(m_cardTid);
	m_pSelectParts->SetStatus(FrButton::PRESSED); // 983 00809261: abre com a aba de partes marcada

	return true;
}

// 983: inline nos chamadores (LTCG), sem corpo proprio
void FrItemAttachCardDlg::initCardSlotBtnTxt()
{
	int i;
	for (i = 0; i < 3; i++)
	{
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}
	for (i = 3; i < 6; i++)
	{
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}
	for (i = 6; i < 8; i++)
	{
		memset(m_slots[i].txt, 0, sizeof(m_slots[i].txt));
		m_slots[i].attached = false;
		m_slots[i].card = 0;
	}
}

// 983 00800cd0: slot (1..8) -> tipo (0 personagem, 1 caddie, 5 npc)
int FrItemAttachCardDlg::GetSlotKind983(int slot)
{
	int kind = -1;
	if (slot <= 3)
		return 0;
	if (slot <= 6)
		return 1;
	if (slot <= 8)
		kind = 5;
	return kind;
}

// 983 00800d00: tipo -> indice do slot dentro do grupo (m_slot - base)
int FrItemAttachCardDlg::GetSlotIndex983(int kind)
{
	int idx = -1;
	switch (kind)
	{
	case 0:
		idx = m_slot;
		break;
	case 1:
		return m_slot - 3;
	case 5:
		return m_slot - 6;
	}
	return idx;
}

// 983 00804ee0
void FrItemAttachCardDlg::SetSlotInfo983(int kind, int idx, unsigned long card, bool attached)
{
	switch (kind)
	{
	case 0:
		m_slots[idx].attached = attached;
		m_slots[idx].card = card;
		SetCharacterAttatchBtnTxt(card, m_slots[idx].txt);
		break;
	case 1:
		m_slots[3 + idx].attached = attached;
		m_slots[3 + idx].card = card;
		SetCaddieAttatchBtnTxt(card, m_slots[3 + idx].txt);
		break;
	case 5:
		m_slots[6 + idx].attached = attached;
		m_slots[6 + idx].card = card;
		SetCaddieAttatchBtnTxt(card, m_slots[6 + idx].txt);
		break;
	}
}

// 983 00800d40
bool FrItemAttachCardDlg::IsInvalidSlot983(int slot)
{
	int kind = GetSlotKind983(slot);
	int idx = GetSlotIndex983(kind);
	if (idx < 0)
		return true;
	bool invalid = false;
	switch (kind)
	{
	case 0:
		if (idx > 3)
			invalid = true;
		break;
	case 1:
		if (idx > 3)
			invalid = true;
		break;
	case 5:
		if (idx > 2)
			invalid = true;
		break;
	}
	return invalid;
}

// 983 008028d0
unsigned long FrItemAttachCardDlg::GetSlotCard983(int slot)
{
	if (IsInvalidSlot983(slot))
		return 0;

	unsigned long card = 0;
	int kind = GetSlotKind983(m_slot);
	int idx = GetSlotIndex983(kind) - 1;
	switch (kind)
	{
	case 0:
		card = m_slots[idx].card;
		break;
	case 1:
		return m_slots[3 + idx].card;
	case 5:
		return m_slots[6 + idx].card;
	}
	return card;
}

// 983 00802960
bool FrItemAttachCardDlg::IsSlotAttached983(int slot)
{
	if (IsInvalidSlot983(slot))
		return false;

	bool attached = false;
	int kind = GetSlotKind983(slot);
	int idx = GetSlotIndex983(kind) - 1;
	switch (kind)
	{
	case 0:
		attached = m_slots[idx].attached;
		break;
	case 1:
		return m_slots[3 + idx].attached;
	case 5:
		return m_slots[6 + idx].attached;
	}
	return attached;
}

void FrItemAttachCardDlg::OnAttachCardIconInit(int param)
{
	m_pAttachCardIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnAttachCardIconOwnerDraw()
{
	if (m_cardTid)
	{
		WPoint pt(m_pAttachCardIcon->GetRect().x,
			m_pAttachCardIcon->GetRect().y);

		DrawItem(pt, m_cardTid, m_cardGuid);
	}
}

void FrItemAttachCardDlg::OnAttachCardNameInit(int param)
{
	m_pAttachCardName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnAttachCardAbilityInit(int param)
{
	m_pAttachCardAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnAttachItemIconInit(int param)
{
	m_pAttachItemIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 b2fca0 (Part: 3 fileiras, typeId>>26 == 2) / b2fcd0 (ClubSet: 2 fileiras, typeId>>26 == 4); o 983 escolhe
// pela tabela de politicas b2ed60..b2ee60. Fileira = (qtde, cardType, y, bitmap de slot vazio); x0 = 5, passo 16.
// So o uid do item e comparado (983 b2f4b4: [edi+4] x [esi+0x14]); a carta e achada por FindCard (a17a10).
// Livre: o 983 tambem chama b2fca0/b2fcd0 da lista do FrCardRemove (007fef10, em pItem->pos).
void DrawPartsSlot983(WPoint pt, unsigned long typeId, unsigned long id,
	FrGraphicInterface* pGDI)
{
	struct sSlotRow
	{
		int count;
		int cardType;
		float y;
		const char* empty;
	};

	sSlotRow rows[3];
	int nRows = 0;

	if ((typeId >> 26) == 2)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(typeId);
		if (pPart == 0)
		{
			return;
		}
		rows[0].count = pPart->CharacterSlot;
		rows[0].cardType = 0;
		rows[0].y = 38.0f;
		rows[0].empty = "card_character_empty";
		rows[1].count = pPart->CaddieSlot;
		rows[1].cardType = 1;
		rows[1].y = 54.0f;
		rows[1].empty = "card_caddie_empty";
		rows[2].count = pPart->NpcSlot983;
		rows[2].cardType = 5;
		rows[2].y = 70.0f;
		rows[2].empty = "card_npc_empty";
		nRows = 3;
	}
	else if ((typeId >> 26) == 4)
	{
		IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(typeId);
		if (pClubSet == 0)
		{
			return;
		}
		rows[0].count = pClubSet->CharacterSlot983;
		rows[0].cardType = 0;
		rows[0].y = 38.0f;
		rows[0].empty = "card_club_char_empty";
		rows[1].count = pClubSet->CaddieSlot983;
		rows[1].cardType = 1;
		rows[1].y = 54.0f;
		rows[1].empty = "card_club_caddie_empty";
		nRows = 2;
	}
	else
	{
		return;
	}

	if (pGDI == 0)
	{
		pGDI = g_pFresh->GetManager()->GetGDI();
		if (pGDI == 0)
		{
			return;
		}
	}

	for (int r = 0; r < nRows; r++)
	{
		float x = 5.0f;

		for (int slot = 1; slot < rows[r].count + 1; slot++)
		{
			const Bitmap* pBitmap = 0;
			std::list<sSCardAvilityPeriodInfo>& list =
				CSharedDoc::Instance()->m_cardAbilityList[0];
			std::list<sSCardAvilityPeriodInfo>::iterator it;

			for (it = list.begin(); it != list.end(); ++it)
			{
				if (id != (*it).partsUid)
				{
					continue;
				}

				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard((*it).tid);
				if (slot == (*it).slotNum && (*it).cardType == rows[r].cardType && pCard != 0)
				{
					pBitmap = g_pFresh->GetBitmap(pCard->SlotImg);
					break;
				}
			}

			if (pBitmap == 0)
			{
				pBitmap = g_pFresh->GetBitmap(rows[r].empty);
			}

			if (pBitmap != 0)
			{
				WRect rect(pt.x + x, pt.y + rows[r].y, (float)pBitmap->Width(),
					(float)pBitmap->Height());
				pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
			}
			x += 16.0f;
		}
	}
}

__forceinline void FrItemAttachCardDlg::DrawPartsSlot(WPoint pt, unsigned long typeId,
	unsigned long id, FrGraphicInterface* pGDI)
{
	DrawPartsSlot983(pt, typeId, id, pGDI);
}

void FrItemAttachCardDlg::OnAttachItemIconOwnerDraw()
{
	if (m_pSelectedItem)
	{
		WPoint pt(m_pAttachItemIcon->GetRect().x,
			m_pAttachItemIcon->GetRect().y);

		DrawItem(pt, m_pSelectedItem->typeId, m_pSelectedItem->id);
		DrawPartsSlot(pt, m_pSelectedItem->typeId, m_pSelectedItem->id, 0);
	}
}

void FrItemAttachCardDlg::OnAttachItemNameInit(int param)
{
	m_pAttachItemName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnAttachItemAbilityInit(int param)
{
	m_pAttachItemAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnAreaCharImgInit(int param)
{
	m_pAreaCharImg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnItemListFoldInit(int param)
{
	m_pItemListFold = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pItemListFold->SetVisible(true);
}

void FrItemAttachCardDlg::OnItemListUnfoldInit(int param)
{
	m_pItemListUnfold = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	m_pItemListUnfold->SetVisible(false);
}

void FrItemAttachCardDlg::OnItemListFoldLClick()
{
	WRect rect;
	m_pMyItemList->GetClientRect(rect);
	rect.h += 100.0f;
	m_pMyItemList->SetClientRect(rect);

	m_cardSlotMove.SetFrameHeight(100.0f);

	m_pItemListFold->SetVisible(false);
	m_pItemListUnfold->SetVisible(true);

	m_bItemListFolded = true;
}

void FrItemAttachCardDlg::OnItemListUnfoldLClick()
{
	WRect rect;
	m_pMyItemList->GetClientRect(rect);
	rect.h -= 100.0f;
	m_pMyItemList->SetClientRect(rect);

	m_cardSlotMove.SetFrameHeight(-100.0f);

	m_pItemListFold->SetVisible(true);
	m_pItemListUnfold->SetVisible(false);

	m_bItemListFolded = false;
}

// 983 00802720 / 00802790: guarda os botoes das abas (NULL se nao for FrButton)
void FrItemAttachCardDlg::OnSelectPartsInit(int param)
{
	m_pSelectParts = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrItemAttachCardDlg::OnSelectClubInit(int param)
{
	m_pSelectClub = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00809280: aba das pecas (lista do SetListCardUsableMyItem 00808fd0)
void FrItemAttachCardDlg::OnSelectPartsLClick()
{
	if (m_selectMode983 == 0)
		return;
	m_pSelectParts->SetStatus(FrButton::PRESSED);
	m_pSelectClub->SetStatus(FrButton::NORMAL);
	m_selectMode983 = 0;
	SetListCardUsableMyItem();
	SetListMyItem();
	SetSelectMyItem(0);
}

// 983 00808f70: aba dos tacos; tira o retrato do personagem
void FrItemAttachCardDlg::OnSelectClubLClick()
{
	if (m_selectMode983 == 1)
		return;
	m_pSelectParts->SetStatus(FrButton::NORMAL);
	m_pSelectClub->SetStatus(FrButton::PRESSED);
	m_selectMode983 = 1;
	SetListCardUsableClub983();
	SetListMyItem();
	SetSelectMyItem(0);
	m_pAreaCharImg->SetBgImg((const Bitmap*)0);
}

// 983 00808cb0: tacos com casa de carta (+0xfa/+0xfc), menos o taco em uso (CSharedDoc +0xa2e)
void FrItemAttachCardDlg::SetListCardUsableClub983()
{
	m_myItemList.clear();

	sItemAttachCard item;
	std::map<unsigned int, sItemInfo>& clubs = CSharedDoc::Instance()->m_clubSetMap;
	std::map<unsigned int, sItemInfo>::iterator it;
	for (it = clubs.begin(); it != clubs.end(); ++it)
	{
		IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(it->second.tid);
		if (pClub->CharacterSlot983 > 0 || pClub->CaddieSlot983 > 0)
		{
			item.typeId = it->second.tid;
			item.id = it->second.guid;
			m_myItemList.push_back(item);
		}
	}

	unsigned long clubId = DOCLOBBYJP()->GetClubSetId983();
	if (clubId != (unsigned long)-1)
	{
		std::list<sItemAttachCard>::iterator li;
		for (li = m_myItemList.begin(); li != m_myItemList.end(); ++li)
		{
			if (li->id == clubId)
			{
				m_myItemList.erase(li);
				break;
			}
		}
	}
}

// 983 00806120: taco pede o item "clube card patch" (0x1a00018f); sem ele, aviso pelo ator principal
void FrItemAttachCardDlg::OnOkLClick983()
{
	if (m_selectMode983 == 1 && !Doc()->FindMyItemByTypeId983(0x1a00018f))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x26),
			(int)K2L_Compatibility("\xc5\xac\xb7\xb4\xbc\xc2\xc6\xae\xc0\xc7 \xc4\xab\xb5\xe5 "
				"\xc0\xe5\xc2\xf8\xbf\xa1\xb4\xc2 '\xc5\xac\xb7\xb4 \xc4\xab\xb5\xe5\xc6\xd0\xc4\xa1' "
				"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9."),
			0x1a00018f, 0, 0, 0));
		return;
	}
	Close(FrOK, true);
}

void FrItemAttachCardDlg::OnDlgFrm_04Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrItemAttachCardDlg::OnDlgFrm_05Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrItemAttachCardDlg::OnDlgFrm_06Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrItemAttachCardDlg::OnDlgFrm_07Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrItemAttachCardDlg::OnDlgFrm_08Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrItemAttachCardDlg::OnDlgFrm_09Init(int param)
{
	m_cardSlotMove.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

bool FrItemAttachCardDlg::OnCardAttachResult(int ret, FrForm* pForm)
{
	if (ret == FrOK)
	{
		ResetCardSlotInfo();
	}
	else if (ret == FrCANCEL)
	{
		SetPartsSlotBtnTxt();
		m_slot = 0xff;
	}

	m_pCardAttachDlg = 0;
	return true;
}

void FrItemAttachCardDlg::OnOpenAttachDlg(int charSlot, int caddieSlot, int npcSlot)
{
	m_pCardAttachDlg = CreateForm<FrCardAttachDlg>(g_pFresh->GetManager(), this,
		"attach_card", 0);
	m_pCardAttachDlg->SetCaption(K2L_Compatibility("\xC4\xAB\xB5\xE5\xBB\xE7\xBF\xEB\xC8\xAE\xC0\xCE"));

	m_pCardAttachDlg->m_cardTid = m_cardTid;
	m_pCardAttachDlg->m_cardGuid = m_cardGuid;
	m_pCardAttachDlg->m_partTid = m_partTid;
	m_pCardAttachDlg->m_partGuid = m_partGuid;
	m_pCardAttachDlg->m_charSlot = charSlot;
	m_pCardAttachDlg->m_caddieSlot = caddieSlot;
	m_pCardAttachDlg->m_npcSlot = npcSlot;
	m_pCardAttachDlg->OnSetDlgTxt();

	m_pCardAttachDlg->Open(
		(FRESH_PFN_RESULT)&FrItemAttachCardDlg::OnAgreeAttachingResult, 1);
}

void FrItemAttachCardDlg::SetCharacterAttatchBtnTxt(unsigned long typeId,
	char* text)
{
	if (m_cardTid == 0)
	{
		strcpy(text, "Card Info error...");
	}

	char result[50] = { 0 };
	char stat[255] = { 0 };
	int count = 0;

	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
	if (pCard != 0)
	{
		for (int i = 0; i < 5; i++)
		{
			if (pCard->COM[i] > 0)
			{
				if (count == 1)
				{
					strcat(result, ",");
				}

				memset(stat, 0, sizeof(stat));
				if (i == 0)
				{
					sprintf(stat, STAT_PLUS_FMT,
						K2L_Compatibility("\xC6\xC4\xBF\xF6\xBD\xBD\xB7\xD4"), pCard->COM[0]);
					strcat(result, stat);
					count++;
				}
				else if (i == 1)
				{
					sprintf(stat, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1\xBD\xBD\xB7\xD4"),
						pCard->COM[1]);
					strcat(result, stat);
					count++;
				}
				else if (i == 2)
				{
					sprintf(stat, STAT_PLUS_FMT,
						K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5\xBD\xBD\xB7\xD4"),
						pCard->COM[2]);
					strcat(result, stat);
					count++;
				}
				else if (i == 3)
				{
					sprintf(stat, STAT_PLUS_FMT,
						K2L_Compatibility("\xBD\xBA\xC7\xC9\xBD\xBD\xB7\xD4"), pCard->COM[3]);
					strcat(result, stat);
					count++;
				}
				else if (i == 4)
				{
					sprintf(stat, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xBF\xBA\xEA\xBD\xBD\xB7\xD4"), pCard->COM[4]);
					strcat(result, stat);
					count++;
				}
			}
		}
	}

	strcpy(text, result);
}

void FrItemAttachCardDlg::SetCaddieAttatchBtnTxt(unsigned long typeId,
	char* text)
{
	if (m_cardTid == 0)
	{
		strcpy(text, "Card Info error...");
	}

	char desc[512] = { 0 };

	IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(typeId);
	if (pDesc)
	{
		strcpy(desc, pDesc->Desc);

		for (int i = 0; i < 512; i++)
		{
			if (desc[i] == '\n')
			{
				desc[i] = 0;
				break;
			}
		}
	}

	strncpy(text, desc, 0x31);
}

void FrItemAttachCardDlg::DrawBtnCharSlotimg(float x, float y,
	unsigned long cardTypeId)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(cardTypeId);
	if (pCard)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		if (pGDI)
		{
			const Bitmap* pBitmap = g_pFresh->GetBitmap(pCard->SlotImg);
			if (pBitmap)
			{
				pGDI->DrawTexture(pBitmap,
					WRect(x + 5.0f, y + 4.0f, pBitmap->Width(),
						pBitmap->Height()),
					0xFFFFFFFF, 0);
			}
		}
	}
}

void FrItemAttachCardDlg::DrawBtnCaddieSlotimg(float x, float y,
	unsigned long cardTypeId)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(cardTypeId);
	if (pCard)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		if (pGDI)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap(MakeStr(pCard->SlotImg));
			if (pBitmap)
			{
				pGDI->DrawTexture(pBitmap,
					WRect(x + 5.0f, y + 4.0f, pBitmap->Width(),
						pBitmap->Height()),
					0xFFFFFFFF, 0);
			}
		}
	}
}

void FrItemAttachCardDlg::OnCharSlotBtnInit_1(int param)
{
	m_slots[0].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[0].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCharSlotBtnDown_1()
{
	if (m_cardType == 0)
	{
		OnOpenAttachDlg(1, -1, -1);
		m_slot = 1;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB8\xAF\xC5\xCD \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_1()
{
	const WRect& rect = m_slots[0].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 0, 0);
	if (!m_slots[0].attached)
		DrawEmptySlot983(rect.x, rect.y, 0);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[0].card);
}

void FrItemAttachCardDlg::OnCharSlotBtnInit_2(int param)
{
	m_slots[1].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[1].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCharSlotBtnDown_2()
{
	if (m_cardType == 0)
	{
		OnOpenAttachDlg(2, -1, -1);
		m_slot = 2;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB8\xAF\xC5\xCD \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_2()
{
	const WRect& rect = m_slots[1].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 1, 0);
	if (!m_slots[1].attached)
		DrawEmptySlot983(rect.x, rect.y, 0);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[1].card);
}

void FrItemAttachCardDlg::OnCharSlotBtnInit_3(int param)
{
	m_slots[2].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[2].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCharSlotBtnDown_3()
{
	if (m_cardType == 0)
	{
		OnOpenAttachDlg(3, -1, -1);
		m_slot = 3;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB8\xAF\xC5\xCD \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_3()
{
	const WRect& rect = m_slots[2].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 2, 0);
	if (!m_slots[2].attached)
		DrawEmptySlot983(rect.x, rect.y, 0);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[2].card);
}

void FrItemAttachCardDlg::OnCharSlotBtnInit_4(int param)
{
	m_slots[3].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[3].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCharSlotBtnDown_4()
{
	if (m_cardType == 1)
	{
		OnOpenAttachDlg(0, 1, -1);
		m_slot = 4;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB5\xF0 \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCharSlotBtnOwnerDraw_4()
{
	const WRect& rect = m_slots[3].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 0, 1);
	if (!m_slots[3].attached)
		DrawEmptySlot983(rect.x, rect.y, 1);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[3].card);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnInit_1(int param)
{
	m_slots[4 + 0].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[4 + 0].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnDown_1()
{
	if (m_cardType == 1)
	{
		OnOpenAttachDlg(0, 2, -1);
		m_slot = 5;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB5\xF0 \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_1()
{
	const WRect& rect = m_slots[4].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 1, 1);
	if (!m_slots[4].attached)
		DrawEmptySlot983(rect.x, rect.y, 1);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[4].card);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnInit_2(int param)
{
	m_slots[4 + 1].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[4 + 1].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnDown_2()
{
	if (m_cardType == 1)
	{
		OnOpenAttachDlg(0, 3, -1);
		m_slot = 6;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("\xC4\xB3\xB5\xF0 \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_2()
{
	const WRect& rect = m_slots[5].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 2, 1);
	if (!m_slots[5].attached)
		DrawEmptySlot983(rect.x, rect.y, 1);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[5].card);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnInit_3(int param)
{
	m_slots[4 + 2].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[4 + 2].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnDown_3()
{
	if (m_cardType == 5)
	{
		OnOpenAttachDlg(0, 0, 1);
		m_slot = 7;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("NPC \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_3()
{
	const WRect& rect = m_slots[6].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 0, 5);
	if (!m_slots[6].attached)
		DrawEmptySlot983(rect.x, rect.y, 2);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[6].card);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnInit_4(int param)
{
	m_slots[4 + 3].pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_slots[4 + 3].pBtn->Enable(false);
}

void FrItemAttachCardDlg::OnCaddieSlotBtnDown_4()
{
	if (m_cardType == 5)
	{
		OnOpenAttachDlg(0, 0, 2);
		m_slot = 8;
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			K2L_Compatibility("NPC \xC4\xAB\xB5\xE5\xB8\xB8 \xBB\xE7\xBF\xEB\xC7\xD2 \xBC\xF6 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void FrItemAttachCardDlg::OnCaddieSlotBtnOwnerDraw_4()
{
	const WRect& rect = m_slots[7].pBtn->GetRect();

	DrawSlotTxt983(rect.x, rect.y, 1, 5);
	if (!m_slots[7].attached)
		DrawEmptySlot983(rect.x, rect.y, 2);
	else
		DrawBtnCharSlotimg(rect.x, rect.y, m_slots[7].card);
}

void FrItemAttachCardDlg::OnProc(const float delta)
{
	FrForm::OnProc(delta); // 983: slot = FrForm 00be6300 (fecha por tempo), sem override
}

void FrItemAttachCardDlg::SetListMyItem()
{
	if (m_pMyItemList)
	{
		m_pMyItemList->ClearItem();

		std::list<sItemAttachCard>::iterator it;
		for (it = m_myItemList.begin(); it != m_myItemList.end(); it++)
		{
			m_pMyItemList->AddItem(&*it);
		}
	}
}

void FrItemAttachCardDlg::OnMyItemListInit(int param)
{
	m_pMyItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pMyItemList)
	{
		m_pMyItemList->UseRightButton(true);
	}

	// 983 00802870: "A card attached can be removed with card extract fluid."
	SetDesc(K2L_Compatibility(
		"\xc0\xe5\xc2\xf8\xc7\xd1 \xc4\xab\xb5\xe5\xb4\xc2 \xc4\xab\xb5\xe5\xc3\xdf\xc3\xe2\xbe\xd7\xc0\xb8\xb7\xce \xba\xd0\xb8\xae \xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
}

void FrItemAttachCardDlg::OnMyItemListLClick()
{
	FrListItem* pItem = m_pMyItemList->GetItemUnderCursor();

	if (pItem)
	{
		m_slot = 0xff;

		if (!pItem->selected)
		{
			m_pMyItemList->ToggleItem(pItem);
		}
		else
		{
			SetSelectMyItem((sItemAttachCard*)pItem->pData);
		}
	}
}

void FrItemAttachCardDlg::OnMyItemListRClick()
{
	if (0)
		((FrItemAttachCardDlg*)0)
			->FrItemAttachCardDlg::OnMyItemListInit(*(int*)0);
}

void FrItemAttachCardDlg::OnMyItemListOwnerDraw(int nIndex)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	FrListItem* pItem = (FrListItem*)nIndex;
	if (pItem == 0)
	{
		return;
	}

	sItemAttachCard* pData = (sItemAttachCard*)pItem->pData;
	const Bitmap* pBaseBmp = 0;

	if (pItem->selected)
	{
		pBaseBmp = m_pMyItemBaseClickBmp;
		pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
	}
	else if (pItem->underCursor)
	{
		pBaseBmp = m_pMyItemBaseOverBmp;
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	}
	else
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	}

	if (pBaseBmp != 0)
	{
		WRect rect(pItem->pos.x + 2.0f, pItem->pos.y, (float)pBaseBmp->Width(),
			(float)pBaseBmp->Height());

		pGDI->DrawTexture(pBaseBmp, rect, 0xFFFFFFFF, 0);
	}

	if ((pData->typeId & 0xFC000000) == 0x08000000)
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(
			((pData->typeId >> 18) & 0xFF) | 0x04000000);
		if (pChar != 0)
		{
			const Bitmap* pCharBmp =
				g_pFresh->GetBitmap(MakeStr("s_%s", pChar->c.Icon));
			if (pCharBmp != 0)
			{
				WRect rect(pItem->pos.x + 7.0f, pItem->pos.y + 5.0f,
					(float)pCharBmp->Width(), (float)pCharBmp->Height());
				pGDI->DrawTexture(pCharBmp, rect, 0xFFFFFFFF, 0);
			}
		}
	}

	WPoint pt(pItem->pos.x + 13.0f, pItem->pos.y + 20.0f);
	DrawItem(pt, pData->typeId, pData->id);
	DrawPartsSlot(pt, pData->typeId, pData->id, pGDI);
	DrawPartsLine();
}

void FrItemAttachCardDlg::SetListCardUsableMyItem()
{
	m_myItemList.clear();

	sItemAttachCard item;
	item.typeId = 0;
	item.id = 0;

	for (int partType = 0; partType < 15; partType++)
	{
		std::list<sItemInfo>::iterator it;

		std::list<sItemInfo>& partsList = CSharedDoc::Instance()->m_partsList;
		for (it = partsList.begin(); it != partsList.end(); ++it)
		{
			if (it->ItemType == 6 && it->Expired)
			{
				continue;
			}

			if (partType != ((it->tid >> 18) & 0xFF))
			{
				continue;
			}

			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(it->tid);

			if (!pPart)
			{
				continue;
			}

			if (pPart->CaddieSlot > 0 || pPart->CharacterSlot > 0 || pPart->NpcSlot983 > 0)
			{
				item.typeId = it->tid;
				item.id = it->guid;

				m_myItemList.push_back(item);
			}
		}

		std::list<sItemInfo>& itemList = CSharedDoc::Instance()->m_uccItemList; // 983 0x3878
		for (it = itemList.begin(); it != itemList.end(); ++it)
		{
			if (partType != ((it->tid >> 18) & 0xFF))
			{
				continue;
			}

			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(it->tid);

			if (!pPart)
			{
				continue;
			}

			if ((it->status & 1) &&
				(!UccManager()->FindClothes(it->tid, it->UccIndex) ||
					UccManager()->IsDownloadComplete(it->tid, it->UccIndex) ||
					NetResourceManager::Instance()->IsDownloadRequested(
						RESOURCE_UCC_CLOTHES, it->tid, it->UccIndex)))
			{
				if (it->status & 1)
				{
					item.typeId = it->tid;
					item.id = it->guid;

					m_myItemList.push_back(item);
				}
			}
		}
	}

	SetListEquipParts();

	std::list<sItemAttachCard>::iterator itMy = m_myItemList.begin();
	while (itMy != m_myItemList.end())
	{
		if (IsEquipParts((*itMy).typeId, (*itMy).id))
		{
			itMy = m_myItemList.erase(itMy);
		}
		else
		{
			itMy++;
		}
	}
}

void FrItemAttachCardDlg::DrawItem(WPoint pt, unsigned long typeId,
	unsigned long id)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	if (typeId == 0)
	{
		return;
	}

	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(typeId);
	if (pCommon == 0)
	{
		return;
	}

	float offsetX = 0.0f;
	float offsetY = 0.0f;

	const Bitmap* pBitmap = g_pFresh->GetBitmap(pCommon->Icon);
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && IsUccClothes(typeId))
	{
		pBitmap = GetUccItemIcon(id);
	}

	int itemType = typeId >> 26;
	if (itemType == 2)
	{
		offsetX = -12.0f;
		offsetY = -20.0f;
	}

	if (pBitmap != 0)
	{
		WRect rect(pt.x + offsetX + 14.0f, pt.y + offsetY + 11.0f,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGDI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(4);
	}

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) && IsUccClothes(typeId))
	{
		sItemInfo* pInfo = GetMyItemInfo(id);
		if (pInfo != 0)
		{
			WRect rect(pt.x + 47.0f, pt.y + 43.0f,
				(float)m_pUccStateBmp[0]->Width(),
				(float)m_pUccStateBmp[0]->Height());
			unsigned char ucc = pInfo->status & 1;
			if (ucc && pInfo->Seq == 1 && m_pUccStateBmp[0])
			{
				pGDI->DrawTexture(m_pUccStateBmp[0], rect, 0xFFFFFFFF, 0);
			}
			else if (ucc && pInfo->Seq > 1 && m_pUccStateBmp[1])
			{
				pGDI->DrawTexture(m_pUccStateBmp[1], rect, 0xFFFFFFFF, 0);
			}
			else if ((pInfo->status & 2) && m_pUccStateBmp[2])
			{
				pGDI->DrawTexture(m_pUccStateBmp[2], rect, 0xFFFFFFFF, 0);
			}
		}
	}

	switch (itemType)
	{
	case 2:
	{
		const Bitmap* pLevel = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (pCommon->Level & 0x7F) + 1));
		if (pLevel != 0)
		{
			WRect rect(pt.x + 38.0f - float2int(pLevel->Width() * 0.5f),
				pt.y + 35.0f - float2int(pLevel->Height() * 0.5f),
				(float)pLevel->Width(), (float)pLevel->Height());
			pGDI->DrawTexture(pLevel, rect, 0xFFFFFFFF, 0);
		}

		// 983 00806519: o relogio (item de tempo) so para pecas que nao sao UCC
		if (IsUccClothes(typeId))
		{
			break;
		}

		std::list<sItemInfo>::iterator it =
			std::find_if(CSharedDoc::Instance()->m_partsList.begin(),
				CSharedDoc::Instance()->m_partsList.end(), _FindByGuid(id));
		if (it != CSharedDoc::Instance()->m_partsList.end() &&
			(*it).ItemType == 6)
		{
			const Bitmap* pClock = g_pFresh->GetBitmap("clock");
			pGDI->DrawTexture(pClock,
				WRect(pt.x + 60.0f, pt.y - 15.0f, pClock->Width(),
					pClock->Height()),
				0xFFFFFFFF, 0);
		}
		break;
	}
	}
}

void FrItemAttachCardDlg::SetCardInfo(unsigned long cardTypeId)
{
	if (cardTypeId == 0)
	{
		return;
	}

	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(cardTypeId);
	if (pCard == 0 || (cardTypeId & 0xFC000000) != 0x7C000000)
	{
		return;
	}

	if (m_pAttachCardName != 0)
	{
		m_pAttachCardName->AddText(MakeStr(K2L_Compatibility("\\c0xff000000\\c%s"), pCard->c.Name),
			false, true);
	}

	if (m_pAttachCardAbility != 0)
	{
		for (int i = 0; i < 5; i++)
		{
			if (pCard->COM[i] > 0)
			{
				char text[512];
				memset(text, 0, sizeof(text));

				if (i == 0)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC6\xC4\xBF\xF6\xBD\xBD\xB7\xD4"), pCard->COM[0]);
				}
				else if (i == 1)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1\xBD\xBD\xB7\xD4"),
						pCard->COM[1]);
				}
				else if (i == 2)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5\xBD\xBD\xB7\xD4"),
						pCard->COM[2]);
				}
				else if (i == 3)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xBD\xBA\xC7\xC9\xBD\xBD\xB7\xD4"), pCard->COM[3]);
				}
				else if (i == 4)
				{
					sprintf(text, STAT_PLUS_FMT,
						K2L_Compatibility("\xC4\xBF\xBA\xEA\xBD\xBD\xB7\xD4"), pCard->COM[4]);
				}

				m_pAttachCardAbility->AddText(
					MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s"), text), false, true);
			}
		}

		if (pCard->Avility == 1 && (pCard->c.TypeId & 0x3c00000) == 0)
		{
			char text[512];
			sprintf(text, "%s - %d", K2L_Compatibility("\xBA\xF1\xB0\xC5\xB8\xAE"),
				pCard->AvilityValue);
			m_pAttachCardAbility->AddText(
				MakeStr(K2L_Compatibility("\\c0xff0000ff\\c%s"), text), false, true);
		}
	}
}

void FrItemAttachCardDlg::SetListEquipParts()
{
	m_equipParts.clear();

	for (std::map<unsigned int, sCharacterInfo>::iterator it =
			 CSharedDoc::Instance()->m_charMap.begin();
		it != CSharedDoc::Instance()->m_charMap.end(); ++it)
	{
		for (int i = 0; i < 24; i++)
		{
			if (it->second.tidParts[i] != 0)
			{
				sItemAttachCard part;
				part.typeId = it->second.tidParts[i];
				part.id = it->second.ItemIdList[i];
				m_equipParts.push_back(part);
			}
		}
	}
}

bool FrItemAttachCardDlg::IsEquipParts(unsigned long typeId, unsigned long id)
{
	if (0)
		((FrItemAttachCardDlg*)0)
			->FrItemAttachCardDlg::OnMyItemListOwnerDraw(*(int*)0);
	bool bFound = false;

	std::list<sItemAttachCard>::iterator it;
	for (it = m_equipParts.begin(); it != m_equipParts.end(); it++)
	{
		if (typeId == (*it).typeId && id == (*it).id)
		{
			m_equipParts.erase(it);
			bFound = true;
			break;
		}
	}

	return bFound;
}

void FrItemAttachCardDlg::SetSelectMyItem(sItemAttachCard* pItem)
{
	m_pAttachItemName->ClearLine();
	m_pAttachItemAbility->ClearLine();
	m_pSelectedItem = pItem;

	if (pItem == 0)
	{
		return;
	}

	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pItem->typeId);
	if (pCommon == 0)
	{
		return;
	}

	if (m_pAttachItemName == 0)
	{
		return;
	}

	m_partTid = pItem->typeId;
	m_partGuid = pItem->id;

	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
		IsUccClothes(pCommon->TypeId))
	{
		m_pAttachItemName->AddText(
			MakeStr(K2L_Compatibility("\\c0xff000000\\c%s"), GetUccItemName(pItem->id)), false,
			true);
	}
	else
	{
		m_pAttachItemName->AddText(MakeStr(K2L_Compatibility("\\c0xff000000\\c%s"), pCommon->Name),
			false, true);
	}

	// 983 00807ca0: Part (typeId>>26 == 2: Attr +0x1e4, Slot +0x1ee, contagens +0x228/+0x22a/+0x22c) e ClubSet
	// (== 4: Attr +0xd0, Slot +0xda, contagens +0xfa/+0xfc); outro tipo nao mexe nos slots nem na lista de atributos.
	short baseStat[5];
	short slotStat[5];
	unsigned long itemKind = pItem->typeId >> 26;
	bool bHasStat = false;

	if (itemKind == 4)
	{
		IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(pItem->typeId);
		if (pClubSet != 0)
		{
			memcpy(baseStat, pClubSet->Attr, sizeof(baseStat));
			memcpy(slotStat, pClubSet->Slot, sizeof(slotStat));
			SetPartsSlot(pClubSet->CharacterSlot983, pClubSet->CaddieSlot983, 0);
			bHasStat = true;
		}
	}
	else if (itemKind == 2)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pItem->typeId);
		if (pPart != 0)
		{
			memcpy(baseStat, pPart->Attr, sizeof(baseStat));
			memcpy(slotStat, pPart->Slot, sizeof(slotStat));
			SetPartsSlot(pPart->CharacterSlot, pPart->CaddieSlot, pPart->NpcSlot983);
			bHasStat = true;
		}
	}

	if (bHasStat)
	{
		const char* statName = "";
		for (int i = 0; i < 5; i++)
		{
			if (baseStat[i] == 0 && slotStat[i] == 0)
			{
				continue;
			}

			switch (i)
			{
			case 0:
				statName = K2L_Compatibility("\xC6\xC4\xBF\xF6");
				break;
			case 1:
				statName = K2L_Compatibility("\xC4\xC1\xC6\xAE\xB7\xD1");
				break;
			case 2:
				statName = K2L_Compatibility("\xC1\xA4\xC8\xAE\xB5\xB5");
				break;
			case 3:
				statName = K2L_Compatibility("\xBD\xBA\xC7\xC9");
				break;
			case 4:
				statName = K2L_Compatibility("\xC4\xBF\xBA\xEA");
				break;
			}

			if (baseStat[i] <= 0 && slotStat[i] <= 0)
			{
				continue;
			}

			if (baseStat[i] == 0)
			{
				m_pAttachItemAbility->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xff0000ff\\c%s\xBD\xBD\xB7\xD4%+d\\c0xff000000\\c"),
						statName, slotStat[i]),
					false, true);
			}
			else if (slotStat[i] == 0)
			{
				m_pAttachItemAbility->AddText(
					MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c", statName, baseStat[i]),
					false, true);
			}
			else if (itemKind == 2)
			{
				m_pAttachItemAbility->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xBD\xBD\xB7\xD4%+d\\c0xff000000\\c"),
						statName, baseStat[i], slotStat[i]),
					false, true);
			}
			else
			{
				// ClubSet (983 7f29): slot menor que a base (slot - base < 0) mostra 0
				int slotShown = (slotStat[i] - baseStat[i] > -1) ? slotStat[i] : 0;
				m_pAttachItemAbility->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xBD\xBD\xB7\xD4%+d\\c0xff000000\\c"),
						statName, baseStat[i], slotShown),
					false, true);
			}
		}
	}

	if ((pItem->typeId & 0xFC000000) == 0x08000000)
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(
			((pItem->typeId >> 18) & 0xFF) | 0x04000000);
		if (pChar != 0)
		{
			m_pAreaCharImg->SetBgImg(MakeStr("s_%s", pChar->c.Icon));
		}
	}

	SetPartsSlotBtnTxt();
}

// 983 008028a0 = 00800b80 (desliga as 3+3+2 fileiras: "card_slot_d") + 00800c20 (liga as primeiras n de cada: "cd_slot_n")
void FrItemAttachCardDlg::DisableSlotRows983(int nChar, int nCaddie, int nNpc)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (m_slots[i].pBtn)
		{
			m_slots[i].pBtn->SetButtonImg("card_slot_d", FrButton::NORMAL);
			m_slots[i].pBtn->Enable(false);
		}
	}

	for (i = 0; i < 3; i++)
	{
		if (m_slots[3 + i].pBtn)
		{
			m_slots[3 + i].pBtn->SetButtonImg("card_slot_d", FrButton::NORMAL);
			m_slots[3 + i].pBtn->Enable(false);
		}
	}

	for (i = 0; i < 2; i++)
	{
		if (m_slots[6 + i].pBtn)
		{
			m_slots[6 + i].pBtn->SetButtonImg("card_slot_d", FrButton::NORMAL);
			m_slots[6 + i].pBtn->Enable(false);
		}
	}
}

void FrItemAttachCardDlg::EnableSlotRows983(int nChar, int nCaddie, int nNpc)
{
	int i;

	for (i = 0; i < nChar; i++)
	{
		if (m_slots[i].pBtn)
		{
			m_slots[i].pBtn->SetButtonImg("cd_slot_n", FrButton::NORMAL);
			m_slots[i].pBtn->Enable(true);
		}
	}

	for (i = 0; i < nCaddie; i++)
	{
		if (m_slots[3 + i].pBtn)
		{
			m_slots[3 + i].pBtn->SetButtonImg("cd_slot_n", FrButton::NORMAL);
			m_slots[3 + i].pBtn->Enable(true);
		}
	}

	for (i = 0; i < nNpc; i++)
	{
		if (m_slots[6 + i].pBtn)
		{
			m_slots[6 + i].pBtn->SetButtonImg("cd_slot_n", FrButton::NORMAL);
			m_slots[6 + i].pBtn->Enable(true);
		}
	}
}

void FrItemAttachCardDlg::SetPartsSlot(int nChar, int nCaddie, int nNpc)
{
	DisableSlotRows983(nChar, nCaddie, nNpc);
	EnableSlotRows983(nChar, nCaddie, nNpc);
}

void FrItemAttachCardDlg::SetPartsSlotBtnTxt()
{
	initCardSlotBtnTxt();

	for (std::list<sSCardAvilityPeriodInfo>::iterator it =
			 CSharedDoc::Instance()->m_cardAbilityList[0].begin();
		it != CSharedDoc::Instance()->m_cardAbilityList[0].end(); ++it)
	{
		if ((*it).partsUid != m_partGuid || (*it).partsTid != m_partTid)
		{
			continue;
		}

		unsigned long tid = (*it).tid;
		int slot = (*it).slotNum;
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
		if (!pCard)
		{
			continue;
		}

		slot--;
		if (slot < 0)
			slot = 0;
		else if (slot > 3)
			slot = 3;

		int kind = (*it).cardType;
		if (kind == 0)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pCard->COM[i] > 0)
				{
					SetSlotInfo983(0, slot, (*it).tid, true);
				}
			}
		}
		else if (kind == 1)
		{
			SetSlotInfo983(kind, slot, (*it).tid, true);
		}
		else if (kind == 5)
		{
			SetSlotInfo983(kind, slot, (*it).tid, true);
		}
	}
}

void FrItemAttachCardDlg::DrawPartsLine()
{
	if (0)
		((FrSPCardBuffDlg*)0)->GetSpecialCardNum();

	float x = 0.0f;

	for (int i = 0; i < 3; i++)
	{
		if (m_pLineOverlay)
		{
			m_pLineOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(x + 307.0f, 155.0f, 1.0f, 75.0f), 0, 0xffffffff, 0.0f, 0);

			if (m_bItemListFolded == true)
			{
				m_pLineOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
					WRect(x + 307.0f, 255.0f, 1.0f, 75.0f), 0, 0xffffffff, 0.0f,
					0);
			}
		}

		// HACK: 91.0f; tutorialmain.cpp owns __real@42b60000 as a function-local static (naked asm), the
		// literal here would emit a competing COMDAT (the volatile read keeps the x87 operand order)
		x = *(volatile float*)&x + RB_F91;
	}
}

void FrItemAttachCardDlg::AgreeAttaching()
{
	unsigned long cardTypeId = GetSlotCard983(m_slot);

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", 0);

	// "The effect of the card you were using (%s) is removed. Use the card anyway?"
	pForm->SetMessage(
		MakeStr(
			K2L_Compatibility("\xB1\xE2\xC1\xB8\xBF\xA1 \xBB\xE7\xBF\xEB\xC7\xCF\xBD\xC3\xB4\xF8 \xC4\xAB\xB5\xE5(%s)\xC0\xC7 \xC8\xBF\xB0\xFA\xB0\xA1 \xBB\xE8\xC1\xA6\xB5\xCB\xB4\xCF\xB4\xD9. \xC1\xA4\xB8\xBB\xB7\xCE \xC4\xAB\xB5\xE5\xB8\xA6 \xBB\xE7\xBF\xEB\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			ItemManager()->GetItemName(cardTypeId)),
		false);

	pForm->Open((FRESH_PFN_RESULT)&FrItemAttachCardDlg::OnCardAttachResult, 1);
}

bool FrItemAttachCardDlg::OnAgreeAttachingResult(int ret, FrForm* pForm)
{
	if (ret == FrOK)
	{
		bool bAttached = IsSlotAttached983(m_slot);

		if (bAttached)
		{
			AgreeAttaching();
		}
		else
		{
			ResetCardSlotInfo();
		}
	}
	else if (ret == FrCANCEL)
	{
		SetPartsSlotBtnTxt();
		m_slot = 0xff;
	}

	m_pCardAttachDlg = 0;
	return true;
}

void FrItemAttachCardDlg::ResetCardSlotInfo()
{
	SetPartsSlotBtnTxt();

	int kind = GetSlotKind983(m_slot);
	SetSlotInfo983(kind, GetSlotIndex983(kind) - 1, m_cardTid, true);
}

IMPLEMENT_OBJECT(FrSPCardBuffDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrSPCardBuffDlg, FrForm)
ON_FRESH_VI("spcardlist", FRCMD_INIT, FrSPCardBuffDlg::OnSPCardListInit)
ON_FRESH_VI("spcardlist", FRCMD_OWNERDRAW,
	FrSPCardBuffDlg::OnSPCardListOwnerDraw)
END_FRESH_MSGMAP()

WPoint FrSPCardBuffDlg::m_Pos(-1.0f, 0.0f);

FrSPCardBuffDlg::FrSPCardBuffDlg()
{
	m_pSPCardList = 0;
}

FrSPCardBuffDlg::~FrSPCardBuffDlg()
{
}

// 983: so FrForm::OnInit (00800e30); o registro dos bitmaps dos cards (645) saiu.
bool FrSPCardBuffDlg::OnInit()
{
	FrForm::OnInit();
	return true;
}

// 983 00808140: dica no icone premium da lista; fora dele a dica fica vazia
void FrSPCardBuffDlg::OnProc(const float delta)
{
	if ((Doc()->m_myInfo.info.dwIdentity >> 14) & 1)
	{
		FrListItem* pItem = m_pSPCardList->FindItem(m_premiumPeriodInfo);
		const Bitmap* pBmp = pItem ? g_pFresh->GetBitmap("premium_buff") : NULL;
		if (pBmp)
		{
			WRect rect(pItem->pos.x, pItem->pos.y, (float)pBmp->Width(), (float)pBmp->Height());
			if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				m_pSPCardList->SetToolTipText(std::string(GetGameString(0x13884)));
				return;
			}
		}
	}
	m_pSPCardList->SetToolTipText(std::string(""));
}

void FrSPCardBuffDlg::SetBuffItem(
	const std::list<sSCardAvilityPeriodInfo>& buffList, bool clear)
{
	if (!m_pSPCardList)
	{
		return;
	}

	if (clear)
	{
		m_pSPCardList->ClearItem();
	}

	for (std::list<sSCardAvilityPeriodInfo>::const_iterator it =
			 buffList.begin();
		it != buffList.end(); it++)
	{
		const sSCardAvilityPeriodInfo* pInfo = &*it;
		bool bSPCard =
			(pInfo->tid & 0xFC000000) == 0x7C000000 && pInfo->cardType == 2;

		if (bSPCard || ItemManager()->IsItemBuff(pInfo->tid))
		{
			m_pSPCardList->AddItem((void*)pInfo);
		}
	}

	int rows = (GetSpecialCardNum() - 1) / 4;
	if (rows > 0)
	{
		WRect rect;
		m_pSPCardList->GetClientRect(rect);
		int height = m_pSPCardList->GetItemHeight() * rows;
		rect.h += height;
		m_pSPCardList->SetClientRect(rect);

		rect = GetRect();
		height = m_pSPCardList->GetItemHeight() * rows;
		rect.h += height;
		SetRect(rect);
	}
}

int FrSPCardBuffDlg::GetSpecialCardNum()
{
	if (!m_pSPCardList)
	{
		return 0;
	}

	return m_pSPCardList->GetCurrentItemSize(false);
}

// 983 00806700 (A319, out/F5/cartas/a319/): so premium (bit 14 de +0x812) e com a lista: o item 0x1a100002 da lista do
// jogador vira uma linha (tid em +0x118; inicio/fim = segundos de +0x18/+0x20 do item, layout do sItemInfo provisorio);
// depois a lista e o form crescem altura_do_item * ((cartoes - 1) / 4).
void FrSPCardBuffDlg::UpdatePremium983()
{
	if (!((Doc()->m_myInfo.info.dwIdentity >> 14) & 1) || !m_pSPCardList)
		return;
	const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it;
	for (it = items.begin(); it != items.end(); ++it)
		if (it->typeId == 0x1a100002)
			break;
	if (it != items.end())
	{
		sSCardAvilityPeriodInfo* pInfo = (sSCardAvilityPeriodInfo*)m_premiumPeriodInfo;
		pInfo->tid = it->typeId;
		S5::SecToSysTime983(&pInfo->useStartTime, *(__int64*)((char*)&*it + 0x18));
		S5::SecToSysTime983(&pInfo->useEndTime, *(__int64*)((char*)&*it + 0x20));
		m_pSPCardList->AddItem(m_premiumPeriodInfo);
	}
	int rows = (GetSpecialCardNum() - 1) / 4;
	if (rows > 0)
	{
		WRect rect;
		m_pSPCardList->GetClientRect(rect);
		rect.h += (float)(m_pSPCardList->GetItemHeight() * rows);
		m_pSPCardList->SetClientRect(rect);
		rect = GetRect();
		rect.h += (float)(m_pSPCardList->GetItemHeight() * rows);
		SetRect(rect);
	}
}

void FrSPCardBuffDlg::OnSPCardListInit(int param)
{
	memset(m_premiumPeriodInfo, 0, sizeof(m_premiumPeriodInfo));
	m_pSPCardList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrSPCardBuffDlg::OnSPCardListOwnerDraw(int nIndex)
{
	FrListItem* pItem = (FrListItem*)nIndex;
	if (pItem == 0)
	{
		return;
	}

	const sSCardAvilityPeriodInfo* pInfo =
		(const sSCardAvilityPeriodInfo*)pItem->pData;
	if (pInfo == 0)
	{
		return;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
	{
		return;
	}

	const char* bitmapName;
	if ((pInfo->tid & 0xFC000000) == 0x7C000000)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(pInfo->tid);
		if (pCard == 0)
		{
			return;
		}

		bitmapName = pCard->BuffImg;
	}
	else if ((*(unsigned long*)DOCPTR983(0x812) >> 0xe) & 1)
	{
		bitmapName = "premium_buff"; // 983: usuario premium mostra o icone unico
	}
	else
	{
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pInfo->tid);
		if (pCommon == 0)
		{
			return;
		}

		bitmapName = MakeStr("%s_buff", pCommon->Icon);
	}

	const Bitmap* pBitmap = g_pFresh->GetBitmap(bitmapName);
	if (pBitmap == 0)
	{
		return;
	}

	unsigned long textColor = 0xFFFF4949;
	unsigned long diffuse = 0xFF494949;
	char text[32] = "-:-";
	int textStyle = 2;
	int remainMin = GetRemainedSec(pInfo) / 60;

	int hour = remainMin / 60;
	int min = remainMin % 60;

	if (((*(unsigned long*)DOCPTR983(0x812) >> 0xe) & 1) &&
		pInfo->tid == 0x1A100002)
	{
		// 983: o item premium mostra "N dias M horas" pela data de fim (horas restantes pela hora do servidor)
		std::string unusedStr("");
		__int64 hoursLeft = DOCJP()->GetRemainHours983((SYSTEMTIME*)&pInfo->useEndTime);
		unsigned short days = 0;
		unsigned short hrs = 0;

		if (hoursLeft != 0)
		{
			days = (unsigned short)(hoursLeft / 24);
			hrs = (unsigned short)(hoursLeft % 24);
		}

		if (days == 0 && hrs == 0)
		{
			sprintf(text,
				K2L_Compatibility("\xB3\xB2\xC0\xBA\xBD\xC3\xB0\xA3: 1\xBD\xC3\xB0\xA3\xB9\xCC\xB8\xB8"));
		}
		else
		{
			sprintf(text,
				MakeStr(K2L_Compatibility("%d\xC0\xCF %d\xBD\xC3\xB0\xA3"), days, hrs));
		}

		textColor = 0xFF494949;
		diffuse = 0xFFFFFFFF; // 983 008082f0: icone premium sem escurecer
		textStyle = 1;
	}
	else if (hour >= 1 || min >= 1)
	{
		if (hour < 0)
		{
			hour = 0;
		}
		if (min < 0)
		{
			min = 0;
		}

		sprintf(text, "%.2d:%.2d", hour, min);

		textColor = 0xFF494949;
		diffuse = 0xFFFFFFFF;
	}

	pGDI->DrawTexture(pBitmap,
		WRect(pItem->pos.x, pItem->pos.y, (float)pBitmap->Width(),
			(float)pBitmap->Height()),
		diffuse, 0);

	unsigned long oldColor = pGDI->GetTextColor();
	pGDI->SetTextColor(textColor, 0xFFFFFFFF);
	pGDI->SetTextStyle(2);

	pGDI->Print11(pItem->pos +
			WPoint((float)pBitmap->Width(), (float)pBitmap->Height()),
		textStyle, text);

	pGDI->SetTextColor(oldColor, 0xFFFFFFFF);
	pGDI->SetTextStyle(0);
}

int GetTotalDay_S(_SYSTEMTIME time)
{
	int monthDays[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int total = 0;

	if (time.wYear < 2 || time.wMonth < 1 || time.wMonth > 12 || time.wDay > 31)
	{
		return 0;
	}

	for (int year = 1; year < time.wYear; year++)
	{
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		{
			total += 366;
		}
		else
		{
			total += 365;
		}
	}

	if ((time.wYear % 4 == 0 && time.wYear % 100 != 0) || time.wYear % 400 == 0)
	{
		monthDays[1] = 29;
	}
	else
	{
		monthDays[1] = 28;
	}

	for (int month = 0; month < time.wMonth - 1; month++)
	{
		total += monthDays[month];
	}

	if (monthDays[time.wMonth - 1] < time.wDay)
	{
		time.wDay = monthDays[time.wMonth - 1];
	}

	return total + time.wDay;
}

__declspec(noinline) int FrSPCardBuffDlg::GetRemainedSec(const sSCardAvilityPeriodInfo* pInfo)
{
	int remain;
	unsigned long elapsed;
	unsigned long tid = pInfo->tid;

	if ((tid & 0xFC000000) != 0x7C000000 &&
		(!((*(unsigned long*)DOCPTR983(0x812) >> 0xe) & 1) || tid != 0x1A100002))
	{
		remain = MAKELONG(pInfo->useEndTime.wMilliseconds,
			pInfo->useEndTime.wSecond);
		unsigned long start = MAKELONG(pInfo->useStartTime.wMilliseconds,
			pInfo->useStartTime.wSecond);
		elapsed = g_CurrentTime - start;
	}
	else
	{
		// 983: diferenca entre o fim e a hora do servidor, em CTime (62fc60), nao a conta manual do 645
		ATL::CTime serverTime(CSharedDoc::Instance()->GetServerTime(), -1);
		ATL::CTime endTime(pInfo->useEndTime, -1);

		remain = (int)(endTime.GetTime() - serverTime.GetTime());
		elapsed = g_CurrentTime - m_SpecialBuffStartTick;
	}

	return remain - elapsed / 1000;
}

// 983 00625487 (0060b4c0 + caption "carta usada")
void OpenCardUseResult983(FrCmdTarget* pOwner, unsigned long tid)
{
	FrCardUseDlg* pDlg = CreateForm<FrCardUseDlg>(g_pFresh->GetManager(), pOwner, "usecard_result", NULL);
	pDlg->SetCaption(K2L_Compatibility("\xc4\xab\xb5\xe5\xbb\xe7\xbf\xeb\xbf\xcf\xb7\xe1"));
	pDlg->m_tid = tid;
	pDlg->Open(NULL, 0);
}
