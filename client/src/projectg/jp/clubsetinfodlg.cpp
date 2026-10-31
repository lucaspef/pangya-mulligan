// JP 983: unidade 00498810–00499800 (FrClubSetInfoDlg, ficha de um taco no meu quarto). Classe do 839/983, sem par no 645;
// ver include/jp/clubsetinfodlg.h. Sessão B91 (F3.7.9, 16º lote). O compilador gera (e casam): os ajustes 00498b40/00498b50 e
// os moldes de iterador de std::map/std::list do meio da unidade. No fim da unidade há quatro acessores sem chamador
// (004997c0–004997f0) de classes da FrClubSetResetWnd e da FrClubSetWorkshopSelector, e no meio o 00498b60
// (FrClubSetResetWnd::IsOpen983, chamado pelo CTask::OnPacketCommon).
#include "minatl.h"
#include <string.h>
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frstatic.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "../../shared/localize.h"
#include "../../shared/itemmanager.h"
#include "shareddoc.h"
#include "clubsetstat.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopselector.h"
#include "clubsetresetwnd.h"
#include "clubsetinfodlg.h"

extern Fresh* g_pFresh; // 00f02340

// 00498810 (GetMessageMap); mapa 00e04a10 (7 entradas)
BEGIN_FRESH_MSGMAP(FrClubSetInfoDlg, FrForm)
ON_FRESH_VI("portrait", FRCMD_INIT, FrClubSetInfoDlg::OnPortraitInit983)
ON_FRESH_VI("name", FRCMD_INIT, FrClubSetInfoDlg::OnNameInit983)
ON_FRESH_VI("level", FRCMD_INIT, FrClubSetInfoDlg::OnLevelInit983)
ON_FRESH_VI("avility", FRCMD_INIT, FrClubSetInfoDlg::OnAvilityInit983)
ON_FRESH_VI("static_avility", FRCMD_INIT, FrClubSetInfoDlg::OnStaticAvilityInit983)
ON_FRESH_VI("icon_clubsetworkshop", FRCMD_INIT, FrClubSetInfoDlg::OnIconInit983)
ON_FRESH_VI("ok", FRCMD_INIT, FrClubSetInfoDlg::OnOkInit983)
END_FRESH_MSGMAP()

// 00498820
FrClubSetInfoDlg::FrClubSetInfoDlg()
	: m_pPortrait(NULL), m_pName(NULL), m_pLevel(NULL), m_pAvility(NULL), m_pStaticAvility(NULL), m_pIcon(NULL),
	m_bPeriod(false), m_clubGuid(0)
{
}

IMPLEMENT_OBJECT(FrClubSetInfoDlg, FrForm)

// 00498890
FrClubSetInfoDlg::~FrClubSetInfoDlg()
{
	m_pPortrait = NULL;
	m_pName = NULL;
	m_pLevel = NULL;
	m_pAvility = NULL;
	m_pStaticAvility = NULL;
	m_pIcon = NULL;
	m_bPeriod = false;
	m_clubGuid = 0;
}

// 004988e0
bool FrClubSetInfoDlg::OnInit()
{
	return FrForm::OnInit();
}

// 004988f0
void FrClubSetInfoDlg::SetPortrait983(IFF_ITEM_COMMON* pItem)
{
	if (m_pPortrait && pItem)
		m_pPortrait->SetBgImg(pItem->Icon);
}

// 00498920: "이름: %s" (nome) e "레벨: 이하" / "레벨: 이상" (nível pedido: até / a partir de)
void FrClubSetInfoDlg::SetName983(IFF_ITEM_COMMON* pItem)
{
	if (m_pName && pItem)
	{
		char name[0x80] = { 0 };
		strncpy(name, pItem->Name, sizeof(name));
		m_pName->AddText(MakeStr(K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n"), name), false, true);
		m_pName->AddText(K2L_Compatibility(pItem->IsUnderLvl ? "\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf\n"
			: "\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3\n"), false, true);
	}
}

// 004989f0
void FrClubSetInfoDlg::SetLevel983(IFF_ITEM_COMMON* pItem)
{
	if (m_pLevel && pItem)
		m_pLevel->SetBgImg(MakeStr("level_%03d", (pItem->Level & 0x7f) + 1));
}

// 00498a30: 파워 / 컨트롤 / 정확도 / 스핀 / 커브 슬롯 (power / control / accuracy / spin / curve)
const char* FrClubSetInfoDlg::GetSlotText983(int slot, int value)
{
	const char* names[5] = {
		K2L_Compatibility("\xc6\xc4\xbf\xf6\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xbd\xba\xc7\xc9\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xbf\xba\xea\xbd\xbd\xb7\xd4"),
	};
	if (value)
		return MakeStr("%s+%d", names[slot], value);
	return "";
}

// 00498ab0: 비거리 (distância)
const char* FrClubSetInfoDlg::GetDistanceText983(int value)
{
	const char* name = K2L_Compatibility("\xba\xf1\xb0\xc5\xb8\xae");
	if (value)
		return MakeStr("%s-%d", name, value);
	return "";
}

// 00498b60
bool FrClubSetResetWnd::IsOpen983()
{
	return s_pInstance != NULL;
}

// 00498c30
void FrClubSetInfoDlg::OnProc(const float deltaTime)
{
	if (FrClubSetResetWnd::s_pInstance && FrClubSetResetWnd::s_pInstance->GetState983() == 4)
		FrForm::OnOK();
}

// 00498c50
void FrClubSetInfoDlg::OnPortraitInit983(int param)
{
	m_pPortrait = KINDOF_CAST983(FrArea, param);
}

// 00498cc0
void FrClubSetInfoDlg::OnNameInit983(int param)
{
	m_pName = KINDOF_CAST983(FrEdit, param);
}

// 00498d30
void FrClubSetInfoDlg::OnLevelInit983(int param)
{
	m_pLevel = KINDOF_CAST983(FrArea, param);
}

// 00498da0
void FrClubSetInfoDlg::OnAvilityInit983(int param)
{
	m_pAvility = KINDOF_CAST983(FrEdit, param);
}

// 00498e10
void FrClubSetInfoDlg::OnStaticAvilityInit983(int param)
{
	m_pStaticAvility = KINDOF_CAST983(FrStatic, param);
}

// 00498e80
void FrClubSetInfoDlg::OnIconInit983(int param)
{
	m_pIcon = KINDOF_CAST983(FrArea, param);
}

// 00498ef0
void FrClubSetInfoDlg::OnOkInit983(int param)
{
	m_pOk = KINDOF_CAST983(FrButton, param);
}

// 00498fc0
void FrClubSetInfoDlg::SetDesc983(IFF_ITEM_COMMON* pItem)
{
	if (pItem)
	{
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->TypeId);
		if (pDesc)
		{
			char desc[0x400] = "\0";
			strcpy(desc, pDesc->Desc);
			if (strlen(desc))
				SetMessage(desc, false);
		}
	}
}

// 00499100: "클럽공방을 이용한 클럽세트입니다." (taco feito na oficina)
void FrClubSetInfoDlg::SetWorkshopIcon983(unsigned long clubGuid)
{
	if (IsClubStepChanged983(clubGuid) || IsClubGradeChanged983(clubGuid))
	{
		if (m_pIcon)
		{
			m_pIcon->SetBgImg(g_pFresh->GetBitmap("clubmade_icon_make"));
			m_pIcon->SetToolTipText(K2L_Compatibility("\xc5\xac\xb7\xb4\xb0\xf8\xb9\xe6\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xd1 "
				"\xc5\xac\xb7\xb4\xbc\xbc\xc6\xae\xc0\xd4\xb4\xcf\xb4\xd9."));
			m_pIcon->SetVisible(true);
		}
	}
}

// 00499210: cartas de atributo presas ao taco (CSharedDoc::m_cardAbilityList[0]): tipo 0 = bônus de slot / distância do
// Card.iff (verde), 1 = 1ª linha da Desc.iff (vermelho), 5 = idem (roxo)
void FrClubSetInfoDlg::SetCardText983(unsigned long clubTid, unsigned long clubGuid)
{
	if (IsLocalContent((localContentType_t)0x52))
	{
		char text[0x200] = { 0 };
		if (m_pAvility && m_pStaticAvility)
		{
			WRect rect;
			if (m_bPeriod == true)
			{
				m_pAvility->AddText(K2L_Compatibility("\n\n"), false, true);
				rect = m_pStaticAvility->GetRect();
				rect.y += 25.0;
				m_pStaticAvility->SetRect(rect);
			}
			else
			{
				rect = m_pStaticAvility->GetRect();
				rect.y -= 3.0;
				m_pStaticAvility->SetRect(rect);
			}

			for (std::list<sSCardAvilityPeriodInfo>::iterator it = Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (it->partsTid != clubTid || it->partsUid != clubGuid)
					continue;
				if (it->cardType == 0)
				{
					IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(it->tid);
					if (!pCard)
						continue;
					for (int i = 0; i < 5; ++i)
					{
						if (pCard->COM[i] > 0)
							m_pAvility->AddText(MakeStr(K2L_Compatibility("\\c0xff60ee0e\\c%s\\c0xff000000\\c"),
								GetSlotText983(i, pCard->COM[i])), false, true);
					}
					if (pCard->Avility == 1)
						m_pAvility->AddText(MakeStr(K2L_Compatibility("\\c0xff60ee0e\\c%s\\c0xff000000\\c"),
							GetDistanceText983(pCard->AvilityValue)), false, true);
				}
				else if (it->cardType == 1)
				{
					IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(it->tid);
					if (!pDesc)
						continue;
					strcpy(text, pDesc->Desc);
					for (int i = 0; i < 0x200; ++i)
					{
						if (text[i] == '\n')
						{
							text[i] = 0;
							break;
						}
					}
					m_pAvility->AddText(MakeStr(K2L_Compatibility("\\c0xffff0000\\c%s\\c0xff000000\\c"), text), false, true);
				}
				else if (it->cardType == 5)
				{
					IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(it->tid);
					if (!pDesc)
						continue;
					strcpy(text, pDesc->Desc);
					for (int i = 0; i < 0x200; ++i)
					{
						if (text[i] == '\n')
						{
							text[i] = 0;
							break;
						}
					}
					m_pAvility->AddText(MakeStr(K2L_Compatibility("\\c0xff57007F\\c%s\\c0xff000000\\c"), text), false, true);
				}
			}
		}
	}
}

// 00499550
void FrClubSetInfoDlg::FindClubPeriod983(unsigned long clubTid, unsigned char* pType, int* pDays, int* pHours, bool* pExpired)
{
	std::map<unsigned int, sItemInfo>& clubs = Doc()->m_clubSetMap;
	for (std::map<unsigned int, sItemInfo>::const_iterator it = clubs.begin(); it != clubs.end(); ++it)
	{
		if (it->second.tid == clubTid)
		{
			*pType = it->second.ItemType;
			*pDays = it->second.HourRemain / 24;
			*pHours = it->second.HourRemain % 24;
			*pExpired = it->second.Expired;
			return;
		}
	}
}

// 004995e0: "사용기간: %d일 %d시간" (tipo 1), "사용기간이 만료되었습니다." (vencido), "열람 가능 기간: %d일 %d시간"
// (0x1a000042 com o conteúdo 0x14), "남은시간: 1시간미만" / "남은시간: %d일 %d시간"
void FrClubSetInfoDlg::SetPeriodText983(IFF_ITEM_COMMON* pItem)
{
	if (!pItem || !m_pName)
		return;

	unsigned char type = 0;
	bool bExpired = false;
	int days = 0, hours = 0;
	FindClubPeriod983(pItem->TypeId, &type, &days, &hours, &bExpired);
	if (type)
		m_bPeriod = true;
	if (m_bPeriod == true)
		m_pName->AddText(K2L_Compatibility("\n"), false, true);

	switch (type)
	{
	case 1:
		m_pName->AddText(MakeStr(K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, hours),
			false, true);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		if (bExpired)
			m_pName->AddText(K2L_Compatibility("\\c0xffff0000\\c\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3\\c0xff000000\\c\xc0\xcc "
				"\\c0xffff0000\\c\xb8\xb8\xb7\xe1\\c0xff000000\\c\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n"), false, true);
		else if (IsLocalContent((localContentType_t)0x14) && pItem->TypeId == 0x1a000042)
			m_pName->AddText(MakeStr(K2L_Compatibility("\xbf\xad\xb6\xf7 \xb0\xa1\xb4\xc9 \xb1\xe2\xb0\xa3: %d\xc0\xcf "
				"%d\xbd\xc3\xb0\xa3\n"), days, hours), false, true);
		else if (days == 0 && hours == 0)
			m_pName->AddText(K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n"), false, true);
		else
			m_pName->AddText(MakeStr(K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n"),
				days, hours), false, true);
		break;
	}
}

// 00499730
void FrClubSetInfoDlg::SetItem983(sItemInfo* pInfo)
{
	if (pInfo && (pInfo->tid & 0xfc000000) == 0x10000000)
	{
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(pInfo->tid);
		if (pItem)
		{
			m_clubGuid = pInfo->guid;
			SetPortrait983(pItem);
			SetName983(pItem);
			SetLevel983(pItem);
			SetPeriodText983(pItem);
			SetWorkshopIcon983(pInfo->guid);
			SetCardText983(pInfo->tid, pInfo->guid);
			SetDesc983(pItem);
		}
	}
}

// 004997c0
FrClubSetResetQuestionDlg* FrClubSetResetQuestionDlg::GetInstance983()
{
	return s_pInstance;
}

// 004997d0
FrClubSetResetQuestionDlg* FrClubSetResetQuestionDlg::GetInstanceB983()
{
	return s_pInstance;
}

// 004997e0
void FrClubSetWorkshopSelector::SetKind983(int kind)
{
	m_kind = kind;
}

// 004997f0
unsigned long FrClubSetWorkshopSelector::GetSelGuid983()
{
	return m_selGuid;
}
