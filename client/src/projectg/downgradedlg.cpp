#include "minatl.h"
class FrDowngradeDlg;
#include "helpdlg.h"
#include "downgradedlg.h"
#include "projectg.h"
#include "jp/clubsetstat.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrDowngradeDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrDowngradeDlg, FrForm)

ON_FRESH_VI("title", FRCMD_INIT, FrDowngradeDlg::OnTitleInit)
ON_FRESH_VI("help", FRCMD_INIT, FrDowngradeDlg::OnHelpBtnInit)
ON_FRESH_VV("help", FRCMD_LBUTTONUP, FrDowngradeDlg::OnHelpBtnUp)
ON_FRESH_VI("power", FRCMD_INIT, FrDowngradeDlg::OnPowerInit)
ON_FRESH_VV("power", FRCMD_LBUTTONUP, FrDowngradeDlg::OnPowerBtnUp)
ON_FRESH_VI("control", FRCMD_INIT, FrDowngradeDlg::OnControlInit)
ON_FRESH_VV("control", FRCMD_LBUTTONUP, FrDowngradeDlg::OnControlBtnUp)
ON_FRESH_VI("impact", FRCMD_INIT, FrDowngradeDlg::OnImpactInit)
ON_FRESH_VV("impact", FRCMD_LBUTTONUP, FrDowngradeDlg::OnImpactBtnUp)
ON_FRESH_VI("spin", FRCMD_INIT, FrDowngradeDlg::OnSpinInit)
ON_FRESH_VV("spin", FRCMD_LBUTTONUP, FrDowngradeDlg::OnSpinBtnUp)
ON_FRESH_VI("curve", FRCMD_INIT, FrDowngradeDlg::OnCurveInit)
ON_FRESH_VV("curve", FRCMD_LBUTTONUP, FrDowngradeDlg::OnCurveBtnUp)
ON_FRESH_VI("epower1", FRCMD_INIT, FrDowngradeDlg::OnPowerEdit1Init)
ON_FRESH_VI("econtrol1", FRCMD_INIT, FrDowngradeDlg::OnControlEdit1Init)
ON_FRESH_VI("eimpact1", FRCMD_INIT, FrDowngradeDlg::OnImpactEdit1Init)
ON_FRESH_VI("espin1", FRCMD_INIT, FrDowngradeDlg::OnSpinEdit1Init)
ON_FRESH_VI("ecurve1", FRCMD_INIT, FrDowngradeDlg::OnCurveEdit1Init)
ON_FRESH_VI("epower2", FRCMD_INIT, FrDowngradeDlg::OnPowerEdit2Init)
ON_FRESH_VI("econtrol2", FRCMD_INIT, FrDowngradeDlg::OnControlEdit2Init)
ON_FRESH_VI("eimpact2", FRCMD_INIT, FrDowngradeDlg::OnImpactEdit2Init)
ON_FRESH_VI("espin2", FRCMD_INIT, FrDowngradeDlg::OnSpinEdit2Init)
ON_FRESH_VI("ecurve2", FRCMD_INIT, FrDowngradeDlg::OnCurveEdit2Init)
ON_FRESH_VI("powerbar", FRCMD_INIT, FrDowngradeDlg::OnPowerBarInit)
ON_FRESH_VI("controlbar", FRCMD_INIT, FrDowngradeDlg::OnControlBarInit)
ON_FRESH_VI("impactbar", FRCMD_INIT, FrDowngradeDlg::OnImpactBarInit)
ON_FRESH_VI("spinbar", FRCMD_INIT, FrDowngradeDlg::OnSpinBarInit)
ON_FRESH_VI("curvebar", FRCMD_INIT, FrDowngradeDlg::OnCurveBarInit)
ON_FRESH_VI("help1", FRCMD_INIT, FrDowngradeDlg::OnHelpEdit1Init)
ON_FRESH_VI("help2", FRCMD_INIT, FrDowngradeDlg::OnHelpEdit2Init)

END_FRESH_MSGMAP()

FrDowngradeDlg::FrDowngradeDlg()
{
	memset(m_pBar, 0, sizeof(m_pBar));
	memset(m_pStatBtn, 0, sizeof(m_pStatBtn));
	memset(m_pLevelEdit, 0, sizeof(m_pLevelEdit));
	memset(m_pEnchantEdit, 0, sizeof(m_pEnchantEdit));
	m_clubId = 0;
	m_pHelpDlg = NULL;
}

void FrDowngradeDlg::SetInfo(unsigned long charTypeId, char* upgrade,
	unsigned long* parts, unsigned long* auxParts, int mode)
{
	m_charTypeId = charTypeId;
	m_pUpgrade = upgrade;
	m_pParts = parts;
	m_pAuxParts = auxParts;
	m_mode = mode;
	m_type = 0;
	if (!mode)
	{
		for (int i = 0; i < 5; ++i)
			if (m_pStatBtn[i])
				m_pStatBtn[i]->SetButtonImg("level_up", FrButton::NORMAL);
		if (m_pTitle)
			m_pTitle->SetBgImg("cha_up");
		if (m_pHelpEdit[0])
			m_pHelpEdit[0]->AddText(
				K2L_Compatibility("\264\311\267\302\300\273 \305\260\277\357 \274\366 \300\326\264\302 \300\307\273\363\300\273 \260\256\303\337\276\356 \300\324\276\372\264\331\270\351 \300\317\301\244\267\256\300\307 \306\316\300\273 \301\366\272\322\307\317\277\251 \277\251\267\257\272\320\300\307 \304\263\270\257\305\315\300\307 \264\311\267\302\300\273 \276\367\261\327\267\271\300\314\265\345 \307\322 \274\366 \300\326\275\300\264\317\264\331."),
				0, true);
		if (m_pHelpEdit[1])
			m_pHelpEdit[1]->AddText(
				K2L_Compatibility("\264\331\270\245 \300\345\272\361\277\315 \301\266\307\325\275\303 \306\304\277\366 \264\311\267\302\300\314 20 \300\314\273\363\300\317 \260\346\277\354 \304\301\306\256\267\321\260\372 \301\244\310\256\265\265 \264\311\267\302\300\314 \301\331\276\356\265\345\264\302 \306\320\263\316\306\274\260\241 \300\326\300\270\264\317 \306\304\277\366 \264\311\267\302 \276\367\261\327\267\271\300\314\265\345\264\302 \275\305\301\337\310\367 \260\341\301\244\307\317\275\303\261\342 \271\331\266\370\264\317\264\331."),
				0, true);
	}
	else
	{
		if (m_pTitle)
			m_pTitle->SetBgImg("cha_dn");
		if (m_pHelpEdit[0])
			m_pHelpEdit[0]->AddText(
				K2L_Compatibility("\300\337\270\370\265\310 \276\367\261\327\267\271\300\314\265\345\267\316 \304\263\270\257\305\315 \264\311\267\302\300\307 \271\353\267\261\275\272\260\241 \271\253\263\312\301\256 \273\347\277\353\307\317\261\342 \276\356\267\301\277\357 \266\247, \304\263\270\257\305\315\300\307 \264\311\267\302\304\241\270\246 \264\331\277\356\261\327\267\271\300\314\265\345 \307\322 \274\366 \300\326\275\300\264\317\264\331."),
				0, true);
		if (m_pHelpEdit[1])
			m_pHelpEdit[1]->AddText(
				K2L_Compatibility("-\264\311\267\302 \264\331\277\356\261\327\267\271\300\314\265\345\270\246 \300\314\277\353\307\330 \304\263\270\257\305\315\300\307 \264\311\267\302\300\273 \300\347\301\266\300\375\307\321 \310\304 \264\331\275\303 \304\263\270\257\305\315\300\307 \264\311\267\302\300\273 \276\367\261\327\267\271\300\314\265\345 \307\322 \266\247\264\302 \306\362\273\363\275\303\277\315 \260\260\300\272 \306\316\300\314 \274\322\270\360\265\313\264\317\264\331."),
				0, true);
	}
	SetLevelBar();
}

void FrDowngradeDlg::SetInfo(unsigned long clubId, int mode)
{
	m_clubId = clubId;
	m_mode = mode;
	m_type = 1;
	if (!mode)
	{
		for (int i = 0; i < 5; ++i)
			if (m_pStatBtn[i])
				m_pStatBtn[i]->SetButtonImg("level_up", FrButton::NORMAL);
		if (m_pTitle)
			m_pTitle->SetBgImg("club_up");
		if (m_pHelpEdit[0])
			m_pHelpEdit[0]->AddText(
				K2L_Compatibility("\264\311\267\302\300\273 \305\260\277\357 \274\366 \300\326\264\302 \305\254\267\264\300\273 \274\322\300\257\307\317\260\355 \300\326\264\331\270\351 \300\317\301\244\267\256\300\307 \306\316\300\273 \301\366\272\322\307\317\277\251 \277\251\267\257\272\320\300\307 \305\254\267\264\300\307 \264\311\267\302\300\273 \276\367\261\327\267\271\300\314\265\345 \307\322 \274\366 \300\326\275\300\264\317\264\331."),
				0, true);
		if (m_pHelpEdit[1])
			m_pHelpEdit[1]->AddText(
				K2L_Compatibility("\264\331\270\245 \300\345\272\361\277\315 \301\266\307\325\275\303 \306\304\277\366 \264\311\267\302\300\314 20 \300\314\273\363\300\317 \260\346\277\354 \304\301\306\256\267\321\260\372 \301\244\310\256\265\265 \264\311\267\302\300\314 \301\331\276\356\265\345\264\302 \306\320\263\316\306\274\260\241 \300\326\300\270\264\317 \306\304\277\366 \264\311\267\302 \276\367\261\327\267\271\300\314\265\345\264\302 \275\305\301\337\310\367 \260\341\301\244\307\317\275\303\261\342 \271\331\266\370\264\317\264\331."),
				0, true);
	}
	else
	{
		if (m_pTitle)
			m_pTitle->SetBgImg("club_dn");
		if (m_pHelpEdit[0])
			m_pHelpEdit[0]->AddText(
				K2L_Compatibility("\300\337\270\370\265\310 \276\367\261\327\267\271\300\314\265\345\267\316 \305\254\267\264 \264\311\267\302\300\307 \271\353\267\261\275\272\260\241 \271\253\263\312\301\256 \273\347\277\353\307\317\261\342 \276\356\267\301\277\357 \266\247, \305\254\267\264\300\307 \264\311\267\302\304\241\270\246 \264\331\277\356\261\327\267\271\300\314\265\345 \307\322 \274\366 \300\326\275\300\264\317\264\331."),
				0, true);
		if (m_pHelpEdit[1])
			m_pHelpEdit[1]->AddText(
				K2L_Compatibility("-\264\311\267\302 \264\331\277\356\261\327\267\271\300\314\265\345\270\246 \300\314\277\353\307\330 \305\254\267\264\300\307 \264\311\267\302\300\273 \300\347\301\266\300\375\307\321 \310\304 \264\331\275\303 \305\254\267\264\300\307 \264\311\267\302\300\273 \276\367\261\327\267\271\300\314\265\345 \307\322 \266\247\264\302 \306\362\273\363\275\303\277\315 \260\260\300\272 \306\316\300\314 \274\322\270\360\265\313\264\317\264\331."),
				0, true);
	}
	SetLevelBar();
}

void FrDowngradeDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDowngradeDlg::OnHelpBtnInit(int param)
{
	m_pHelpBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnHelpBtnUp()
{
	if (!m_pHelpDlg)
	{
		m_pHelpDlg =
			CreateForm<FrHelpDlg>(g_pFresh->GetManager(), this, "help", NULL);
		m_pHelpDlg->Open((FRESH_PFN_RESULT)&FrDowngradeDlg::OnHelpDlgResult,
			true);
	}
}

bool FrDowngradeDlg::OnHelpDlgResult(int result, FrForm* form)
{
	m_pHelpDlg = NULL;
	return true;
}

void FrDowngradeDlg::OnHelpEdit1Init(int param)
{
	m_pHelpEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnHelpEdit2Init(int param)
{
	m_pHelpEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnPowerBarInit(int param)
{
	m_pBar[0] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pBar[0]->SetRange(0, 50, false);
}

void FrDowngradeDlg::OnControlBarInit(int param)
{
	m_pBar[1] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pBar[1]->SetRange(0, 30, false);
}

void FrDowngradeDlg::OnImpactBarInit(int param)
{
	m_pBar[2] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pBar[2]->SetRange(0, 30, false);
}

void FrDowngradeDlg::OnSpinBarInit(int param)
{
	m_pBar[3] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pBar[3]->SetRange(0, 30, false);
}

void FrDowngradeDlg::OnCurveBarInit(int param)
{
	m_pBar[4] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pBar[4]->SetRange(0, 30, false);
}

void FrDowngradeDlg::OnPowerInit(int param)
{
	m_pStatBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnControlInit(int param)
{
	m_pStatBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnImpactInit(int param)
{
	m_pStatBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnSpinInit(int param)
{
	m_pStatBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnCurveInit(int param)
{
	m_pStatBtn[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDowngradeDlg::OnPowerBtnUp()
{
	OpenDowngrade(0);
}

void FrDowngradeDlg::OnControlBtnUp()
{
	OpenDowngrade(1);
}

void FrDowngradeDlg::OnImpactBtnUp()
{
	OpenDowngrade(2);
}

void FrDowngradeDlg::OnSpinBtnUp()
{
	OpenDowngrade(3);
}

void FrDowngradeDlg::OnCurveBtnUp()
{
	OpenDowngrade(4);
}

void FrDowngradeDlg::OpenDowngrade(unsigned char stat)
{
	unsigned char type = m_mode ? 2 : 0;
	if (m_type == 1)
		++type;
	// 983 (00aa5eb0): pela tarefa ao ator principal (vtable+0x30); o 983 nao tem o ator "RealMyRoom" (A821: seta sem efeito)
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xd4), type, stat, 0, 0, 0));
}

void FrDowngradeDlg::OnPowerEdit1Init(int param)
{
	m_pLevelEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnControlEdit1Init(int param)
{
	m_pLevelEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnImpactEdit1Init(int param)
{
	m_pLevelEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnSpinEdit1Init(int param)
{
	m_pLevelEdit[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnCurveEdit1Init(int param)
{
	m_pLevelEdit[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnPowerEdit2Init(int param)
{
	m_pEnchantEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnControlEdit2Init(int param)
{
	m_pEnchantEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnImpactEdit2Init(int param)
{
	m_pEnchantEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnSpinEdit2Init(int param)
{
	m_pEnchantEdit[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::OnCurveEdit2Init(int param)
{
	m_pEnchantEdit[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDowngradeDlg::SetLevelNumber(int index)
{
	if (!m_pLevelEdit[index])
		return;
	unsigned char value, limit;
	if (m_type == 0)
	{
		unsigned char item =
			ItemManager()->GetItemLevel((eLvlType)index, m_pParts, m_pAuxParts);
		value = item +
			ItemManager()->GetCharLevel((eLvlType)index, m_charTypeId,
				m_pUpgrade);
		limit = item +
			ItemManager()->GetCharCapacity((eLvlType)index, m_charTypeId,
				m_pParts, m_pAuxParts, Doc()->m_myInfo.stat.Level);
	}
	else
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(m_clubId);
		IFF_STRUCT::sClubSet* club = ItemManager()->FindClubSet(it->second.tid);
		if (!club)
			return;
		value = club->Attr[index] + it->second.Common[index];
		limit = GetClubSlotMax983(index, club, &it->second, false);
	}
	m_pLevelEdit[index]->SetLine(1, MakeStr("%d", Min(value, limit)),
		0xffff0000, false, 0);
}

void FrDowngradeDlg::SetEnchantNumber(int index)
{
	if (!m_pEnchantEdit[index])
		return;
	if (m_type == 0)
	{
		unsigned short limit = ItemManager()->GetCharCapacity((eLvlType)index,
			0, m_pParts, m_pAuxParts, Doc()->m_myInfo.stat.Level);
		int value = m_pUpgrade ? m_pUpgrade[index] : 0;
		m_pEnchantEdit[index]->SetLine(1, MakeStr("%d/%d", value, limit),
			0xff000000, false, 0);
	}
	else
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(m_clubId);
		IFF_STRUCT::sClubSet* club = ItemManager()->FindClubSet(it->second.tid);
		if (!club)
			return;
		unsigned short limit = GetClubSlotMax983(index, club, &it->second, false);
		m_pEnchantEdit[index]->SetLine(1,
			MakeStr("%d/%d", it->second.Common[index],
				limit - club->Attr[index]),
			0xff000000, false, 0);
	}
}

void FrDowngradeDlg::SetLevelBar()
{
	int limit;
	if (m_type == 0)
	{
		for (unsigned char i = 0; i < 5; ++i)
		{
			int item =
				ItemManager()->GetItemLevel((eLvlType)i, m_pParts, m_pAuxParts);
			limit = item +
				ItemManager()->GetCharCapacity((eLvlType)i, m_charTypeId,
					m_pParts, m_pAuxParts, Doc()->m_myInfo.stat.Level);
			int value = item +
				ItemManager()->GetCharLevel((eLvlType)i, m_charTypeId,
					m_pUpgrade);
			if (m_pBar[i])
			{
				m_pBar[i]->SetDestPos(value, false);
				m_pBar[i]->SetExpand(limit - value);
			}
			if (m_pStatBtn[i])
			{
				if (!m_mode)
					m_pStatBtn[i]->Enable(value < limit);
				else
					m_pStatBtn[i]->Enable(m_pUpgrade[i] > 0);
			}
			SetLevelNumber(i);
			SetEnchantNumber(i);
		}
	}
	else
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(m_clubId);
		if (it == Doc()->m_clubSetMap.end())
			return;
		IFF_STRUCT::sClubSet* club = ItemManager()->FindClubSet(it->second.tid);
		if (!club)
			return;
		for (unsigned char i = 0; i < 5; ++i)
		{
			// 983 00aa62bf: teto = slots do taco + slots ganhos na oficina (0049c490), sem cartas
			limit = GetClubSlotMax983(i, club, &it->second, false);
			int value = club->Attr[i] + it->second.Common[i];
			if (m_pBar[i])
			{
				m_pBar[i]->SetDestPos(value, false);
				m_pBar[i]->SetExpand(limit - value);
			}
			if (m_pStatBtn[i])
			{
				if (!m_mode)
					m_pStatBtn[i]->Enable(value < limit);
				else
					m_pStatBtn[i]->Enable(it->second.Common[i] > 0);
			}
			SetLevelNumber(i);
			SetEnchantNumber(i);
		}
	}
}
