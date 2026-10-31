#include "minatl.h"
#include "s5/demonlanddlg.h"
#include "s5/utilities.h"
#include "mathconsts.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frgaugebar.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "projectg.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "sea.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "netresourcemanager.h"

extern Fresh* g_pFresh;

unsigned char GetNextHole();

IMPLEMENT_OBJECT(FrDemonNextHole, FrForm)

BEGIN_FRESH_MSGMAP(FrDemonNextHole, FrForm)
ON_FRESH_VI("nextholepattern", FRCMD_INIT, FrDemonNextHole::OnAreaPattern_Init)
ON_FRESH_VI("nextholepattern", FRCMD_OWNERDRAW,
	FrDemonNextHole::OnAreaPattern_OwnerDraw)
ON_FRESH_VI("nexthole", FRCMD_INIT, FrDemonNextHole::OnAreaNextHole_Init)
ON_FRESH_VI("nexthole", FRCMD_OWNERDRAW,
	FrDemonNextHole::OnAreaNextHole_OwnerDraw)
END_FRESH_MSGMAP()

FrDemonNextHole::FrDemonNextHole()
{
	m_pAreaPattern = NULL;
	m_pAreaNextHole = NULL;
	m_pPatternBmp = NULL;
	m_state = 1;
}

FrDemonNextHole::~FrDemonNextHole()
{
}

bool FrDemonNextHole::OnInit()
{
	m_pPatternBmp = g_pFresh->GetBitmap("demon_nexthole_pattern");
	if (!m_pPatternBmp)
		return false;

	unsigned int map;
	if (GetCurMap() <= 0x7f || GetCurMap() == 0xfd)
		map = GetCurMap();
	else
		map = GetCurMap() - 0x80;

	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(map | 0x28000000);
	if (!course)
		return false;

	m_pOverlay[0] =
		g_resrcmng->GetOverlay(MakeStr("%s_1.png", course->c.Name), 0);
	m_pOverlay[1] =
		g_resrcmng->GetOverlay(MakeStr("%s_2.png", course->c.Name), 0);
	if (m_pOverlay[0] || m_pOverlay[1])
	{
		m_texWidth = m_pOverlay[0]->GetWidth();
		m_texHeight = m_pOverlay[0]->GetHeight();
	}
	return FrForm::OnInit();
}

void FrDemonNextHole::SetState(unsigned char state)
{
	unsigned long newState = state;
	if (m_state == newState)
	{
		if (IsEnabled() && m_state == 1)
		{
			Enable(false);
			SetVisible(false);
		}
	}
	else
	{
		m_state = newState;
		if (newState == 1)
		{
			Enable(false);
			SetVisible(false);
		}
		else
		{
			Enable(true);
			SetVisible(true);
			SetNextHole(GetNextHole());
		}
	}
}

void FrDemonNextHole::SetNextHole(int hole)
{
	if (m_pOverlay[0] && m_pOverlay[1])
	{
		if (hole <= 10 && hole != 19 &&
			Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] !=
				GOLFDOC()->m_currentHole)
		{
			m_overlayIndex = 0;
		}
		else
		{
			m_overlayIndex = 1;
			hole -= 10;
		}

		if (hole == 19 ||
			Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] ==
				GOLFDOC()->m_currentHole)
		{
			S5::UTIL::CalculateUVPos((WRect&)m_uvRect, m_texWidth, m_texHeight,
				140.0f, 93.0f, 8);
		}
		else
		{
			S5::UTIL::CalculateUVPos((WRect&)m_uvRect, m_texWidth, m_texHeight,
				140.0f, 93.0f, hole - 1);
		}
	}
}

void FrDemonNextHole::OnAreaPattern_Init(int param)
{
	m_pAreaPattern = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDemonNextHole::OnAreaPattern_OwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI && m_pPatternBmp)
	{
		pGDI->DrawTexture(m_pPatternBmp,
			WRect(m_rect.x + 120.0f, m_rect.y + 110.0f, 63.0f, 35.0f),
			0xffffffff, 0);
	}
}

void FrDemonNextHole::OnAreaNextHole_Init(int param)
{
	m_pAreaNextHole = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDemonNextHole::OnAreaNextHole_OwnerDraw(int param)
{
	if (m_state != 1)
	{
		WRect rect(m_pAreaNextHole->GetRect().TopLeft(), WSize(140.0f, 93.0f));
		m_pOverlay[m_overlayIndex]->Render(g_view, m_uvRect, rect, 0,
			0xffffffff, 0.0f, 0);
	}
}

IMPLEMENT_OBJECT(FrDemonAttackGauge, FrForm)

BEGIN_FRESH_MSGMAP(FrDemonAttackGauge, FrForm)
ON_FRESH_VI("attackgauge", FRCMD_INIT,
	FrDemonAttackGauge::OnImageBarAttackPoint_Init)
ON_FRESH_VI("gaugemarker", FRCMD_INIT,
	FrDemonAttackGauge::OnAreaAttackMarker_Init)
ON_FRESH_VI("gaugemarker", FRCMD_OWNERDRAW,
	FrDemonAttackGauge::OnAreaAttackMarker_OwnerDraw)
ON_FRESH_VI("blingbar", FRCMD_INIT, FrDemonAttackGauge::OnAreaBlingBar_Init)
ON_FRESH_VI("blingbar", FRCMD_OWNERDRAW,
	FrDemonAttackGauge::OnAreaBlingBar_OwnerDraw)
END_FRESH_MSGMAP()

FrDemonAttackGauge::FrDemonAttackGauge()
{
	m_pImageBar = NULL;
	m_pAreaAttackMarker = NULL;
	m_pAttackPointBmp = NULL;
}

FrDemonAttackGauge::~FrDemonAttackGauge()
{
}

bool FrDemonAttackGauge::OnInit()
{
	m_pAttackPointBmp = g_pFresh->GetBitmap("demon_attack_point");
	m_pGaugeLightBmp = g_pFresh->GetBitmap("demon_gauge_light");
	return FrForm::OnInit();
}

void FrDemonAttackGauge::SetState(unsigned char state)
{
	unsigned long newState = state;
	if (m_state != newState)
	{
		m_state = newState;
		if (newState == 1)
		{
			Enable(false);
			SetVisible(false);
		}
		else
		{
			Enable(true);
			SetVisible(true);
		}
	}
}

void FrDemonAttackGauge::OnImageBarAttackPoint_Init(int param)
{
	m_pImageBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pImageBar)
	{
		m_pImageBar->SetRange(0, 1000, 0);
		m_pImageBar->SetBarSpeed(250.0f);
		m_pImageBar->SetDestPos(1000, false);
	}
}

void FrDemonAttackGauge::OnAreaAttackMarker_Init(int param)
{
	m_pAreaAttackMarker = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDemonAttackGauge::OnAreaAttackMarker_OwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI && m_pAttackPointBmp)
	{
		pGDI->DrawTexture(m_pAttackPointBmp,
			WRect(m_rect.x + 50.0f, m_rect.y + 50.0f, 24.0f, 15.0f), 0xffffffff,
			0);
	}
}

void FrDemonAttackGauge::OnAreaBlingBar_Init(int param)
{
	m_pAreaBlingBar = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDemonAttackGauge::OnAreaBlingBar_OwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI && m_pGaugeLightBmp)
	{
		FrGaugeBarImage* bar = m_pImageBar;
		unsigned int width = m_pGaugeLightBmp->Width();
		float x = (float)bar->GetPixelPerHeight() +
			(bar->GetRect().x - (float)(width >> 1));
		unsigned int height = m_pGaugeLightBmp->Height() >> 1;
		float y = (bar->GetRect().y - (float)height) + 15.0f;
		pGDI->DrawTexture(m_pGaugeLightBmp, WRect(x, y, 33.0f, 68.0f),
			0xffffffff, 0);
	}
}

IMPLEMENT_OBJECT(FrDemonProcessRank, FrForm)

BEGIN_FRESH_MSGMAP(FrDemonProcessRank, FrForm)
ON_FRESH_VI("leaderpet", FRCMD_INIT, FrDemonProcessRank::OnAreaLeaderPet_Init)
ON_FRESH_VI("leaderpet", FRCMD_OWNERDRAW,
	FrDemonProcessRank::OnAreaLeaderPet_OwnerDraw)
ON_FRESH_VI("attackerlist", FRCMD_INIT,
	FrDemonProcessRank::OnListBoxUserInfoList_Init)
ON_FRESH_VI("attackerlist", FRCMD_OWNERDRAW,
	FrDemonProcessRank::OnListBoxUserInfoList_OwnerDraw)
END_FRESH_MSGMAP()

FrDemonProcessRank::FrDemonProcessRank()
{
	m_pUserInfoList = NULL;
}

FrDemonProcessRank::~FrDemonProcessRank()
{
}

bool FrDemonProcessRank::OnInit()
{
	return FrForm::OnInit();
}

void FrDemonProcessRank::OnAreaLeaderPet_Init(int param)
{
	m_pAreaLeaderPet = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrDemonProcessRank::OnAreaLeaderPet_OwnerDraw(int param)
{
}

void FrDemonProcessRank::OnListBoxUserInfoList_Init(int param)
{
	m_pUserInfoList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

const Bitmap* GetTitleBitmap(const sRivalData* pRival)
{
	const Bitmap* pIcon;
	if (pRival->titleTypeId)
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pRival->titleTypeId);
		if (pSkin)
			pIcon = g_pFresh->GetManager()->GetBitmap("TITLES", pSkin->c.Icon);
		else
			pIcon = g_pFresh->GetManager()->GetBitmap("LEVELS",
				MakeStr("level_%03d", pRival->level + 1));
	}
	else
		pIcon = g_pFresh->GetManager()->GetBitmap("LEVELS",
			MakeStr("level_%03d", pRival->level + 1));
	return pIcon;
}

void FrDemonProcessRank::OnListBoxUserInfoList_OwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;
	if (!param)
		return;

	FrListItem* pItem = (FrListItem*)param;
	sRivalData* pRival = (sRivalData*)pItem->pData;

	const Bitmap* pTitle = GetTitleBitmap(pRival);
	// HACK: the original reads an uninitialised WRect here (bug in the game).
	WRect rc;
	float y = rc.y + 8.0f;
	int half = (int)(rc.h * 0.5f);
	WRect dst(rc.x, y - half, rc.w, (float)half);
	pGDI->DrawTexture(pTitle, dst, 0xffffffff, 0);

	if (pRival->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1, "-");
	else
		pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1, "%d",
			pRival->rank);

	if (pRival->guildUID)
	{
		const Bitmap* pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983);
		if (pEmblem)
		{
			pGDI->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 33.0f, pItem->pos.y + 3.0f - 6.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xffffffff, 0);
		}
	}

	g_pFresh->GetManager()->PrintText(
		WPoint(pItem->pos.x + 60.0f, pItem->pos.y + 3.0f), 0, pRival->nickname,
		90.0f, 0xffffffff);

	pGDI->Print(WPoint(pItem->pos.x + 179.0f, pItem->pos.y + 3.0f), 2, "%d",
		pRival->totalScore);
	pGDI->Print(WPoint(pItem->pos.x + 222.0f, pItem->pos.y + 3.0f), 2, "%I64d",
		pRival->totalPang);

	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
	{
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"End");
	}
	else if (pRival->state == 3)
	{
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"Out");
	}
	else
	{
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"%d\xc8\xa6", GetHoleIndex(pRival->hole));
	}
}

IMPLEMENT_OBJECT(FrDemonFinalScore, FrForm)

BEGIN_FRESH_MSGMAP(FrDemonFinalScore, FrForm)
ON_FRESH_VI("attackgauge", FRCMD_INIT,
	FrDemonFinalScore::OnImageBarAttackPoint_Init)
ON_FRESH_VI("giftlist", FRCMD_INIT, FrDemonFinalScore::OnInit_Gift_List)
ON_FRESH_VI("giftlist", FRCMD_OWNERDRAW,
	FrDemonFinalScore::OnOwnerDraw_Gift_List)
END_FRESH_MSGMAP()

FrDemonFinalScore::FrDemonFinalScore()
	: m_pImageBar(NULL)
{
}

FrDemonFinalScore::~FrDemonFinalScore()
{
}

bool FrDemonFinalScore::OnInit()
{
	if (!m_boxOpener.OnInit())
		return false;
	return FrForm::OnInit();
}

void FrDemonFinalScore::OnProc(const float dt)
{
	m_boxOpener.OnProc(dt);
}

void FrDemonFinalScore::OnImageBarAttackPoint_Init(int param)
{
	m_pImageBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pImageBar)
	{
		m_pImageBar->SetRange(0, 1000, 0);
		m_pImageBar->SetBarSpeed(250.0f);
		m_pImageBar->SetDestPos(1000, false);
	}
}

void FrDemonFinalScore::OnInit_Gift_List(int param)
{
	m_boxOpener.m_pGiftList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrDemonFinalScore::OnOwnerDraw_Gift_List(int param)
{
	if (param)
		m_boxOpener.DrawGiftList((FrListItem*)param);
}

IMPLEMENT_OBJECT(FrDemonLandTicket, FrForm)

BEGIN_FRESH_MSGMAP(FrDemonLandTicket, FrForm)
ON_FRESH_VI("ticketmsg", FRCMD_INIT, FrDemonLandTicket::OnInit_TicketMsg_Static)
END_FRESH_MSGMAP()

FrDemonLandTicket::FrDemonLandTicket()
{
	m_pTicketMsg = NULL;
	m_ticketNumber = 0;
}

FrDemonLandTicket::~FrDemonLandTicket()
{
}

void FrDemonLandTicket::SetTicketNumber(int number)
{
	m_ticketNumber = number;
	if (m_pTicketMsg)
	{
		m_pTicketMsg->SetCaption(MakeStr(
			"\xb4\xeb\xb1\xe2\xb9\xf8\xc8\xa3 : %d \xc0\xd4\xb4\xcf\xb4\xd9. "
			"\xc0\xe1\xbd\xc3\xb8\xb8 \xb1\xe2\xb4\xd9\xb7\xc1 "
			"\xc1\xd6\xbc\xbc\xbf\xe4.",
			number));
	}
}

void FrDemonLandTicket::OnInit_TicketMsg_Static(int param)
{
	m_pTicketMsg = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pTicketMsg)
	{
		m_pTicketMsg->SetCaption("\xb8\xb6\xbf\xd5\xc0\xc7 \xb6\xa5 "
								 "\xc0\xd4\xc0\xe5 \xbf\xe4\xc3\xbb "
								 "\xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9.");
	}
}

CDemonLandForm::CDemonLandForm()
{
	InitializeForm();
}

CDemonLandForm::~CDemonLandForm()
{
}

bool CDemonLandForm::CreateDemonLandForm(eDemonDlg dlg)
{
	switch (dlg)
	{
	case DEMONDLG_NEXTHOLE:
		m_pNextHole = CreateForm<FrDemonNextHole>(g_pFresh->GetManager(), NULL,
			"demon_nexthole", NULL);
		m_pNextHole->EnableDrag(false);
		m_pNextHole->Open(NULL, 0xc);
		m_pNextHole->SetState(1);
		return true;
	case DEMONDLG_ATTACKGAUGE:
		m_pAttackGauge = CreateForm<FrDemonAttackGauge>(g_pFresh->GetManager(),
			NULL, "demon_attackgauge", NULL);
		m_pAttackGauge->EnableDrag(false);
		m_pAttackGauge->Open(NULL, 0xc);
		break;
	case DEMONDLG_PROCESSRANK:
		m_pProcessRank = CreateForm<FrDemonProcessRank>(g_pFresh->GetManager(),
			NULL, "demon_processrank", NULL);
		m_pProcessRank->EnableDrag(false);
		m_pProcessRank->SetFixed(true);
		m_pProcessRank->Open(NULL, 0xc);
		break;
	case DEMONDLG_FINALSCORE:
		break;
	default:
		return false;
	}
	return true;
}

void CDemonLandForm::InitializeForm()
{
	m_pNextHole = NULL;
	m_pAttackGauge = NULL;
	m_pProcessRank = NULL;
	m_pFinalScore = NULL;
}

void CDemonLandForm::AdjustWindowPos()
{
	if (m_pNextHole && m_pAttackGauge)
	{
		WRect rect;
		AdjustNextHolePos(rect);
		AdjustAttackGaugePos(rect);
	}
}

// HACK: GetNextHole is an inline of shareddoc.inl in the original; it is
// emitted as an out-of-line COMDAT there.
unsigned char GetNextHole()
{
	for (unsigned char i = 0; i < 18; i++)
	{
		if (Doc()->m_holeOrder[i] == GOLFDOC()->m_currentHole)
			return Doc()->m_holeOrder[i + 1];
	}
	return 19;
}

FrDemonNextHole* CDemonLandForm::GetDemonNextHole()
{
	return m_pNextHole;
}

FrDemonAttackGauge* CDemonLandForm::GetDemonAttackGauge()
{
	return m_pAttackGauge;
}

FrDemonProcessRank* CDemonLandForm::GetDemonProcessRank()
{
	return m_pProcessRank;
}

FrDemonFinalScore* CDemonLandForm::GetDemonFinalScore()
{
	return m_pFinalScore;
}

void CDemonLandForm::ReleaseForm()
{
}

void CDemonLandForm::AdjustNextHolePos(WRect& rect)
{
	const WRect& hole = m_pNextHole->GetRect();
	rect.x = 0.0f;
	rect.y = hole.h;
	rect.w = hole.w;
	rect.h = hole.h;
	rect.x = g_view->GetWidth() * 0.5f - rect.w;
	m_pNextHole->SetRect(rect);
}

void CDemonLandForm::AdjustAttackGaugePos(WRect& rect)
{
	const WRect& gauge = m_pAttackGauge->GetRect();
	rect.x = 0.0f;
	rect.y = gauge.y;
	rect.w = gauge.w;
	rect.h = gauge.h;
	rect.x = g_view->GetWidth() * 0.5f + gauge.w;
	m_pAttackGauge->SetRect(rect);
}
