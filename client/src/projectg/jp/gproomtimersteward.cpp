// JP 983: unidade gproomtimersteward (0055ce30–0055ee70). Classes novas do 983, sem par no 645: peças do relógio (CGrandPrixHour,
// Minute, Second), o quadro de horário da FrGPTypeRoomDlg, o aviso de abertura (CGPRoomTimerSteward), o relógio do Grand Prix
// (CGrandPrixClock) e os auxiliares das linhas da lista do Grand Prix. Sessão B67. Ver include/jp/gproomtimersteward.h.
#include "minatl.h"
#include "projectg.h"
#include "gproomtimersteward.h"
#include "grandprixdocument.h"
#include "icontentsdoc.h"
#include "contentsdoc.h"
#include "gamestringmanager.h"
#include "taskmanager.h"
#include "noticeboard.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "bitmap.h"
#include "titlefontex.h"
#include "../../shared/itemmanager.h"
#include <stdio.h>
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)

extern Fresh* g_pFresh;

// S5 (sharedutilities.cpp da frente A; sem nome no fonte do 645) — empréstimos com nome meu
namespace S5
{
void SysTimeToSec983(const SYSTEMTIME* pTime, __int64* pOut); // 00b30830
void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); // 00b307f0
__int64 TodayTime983(SYSTEMTIME* pTime, const SYSTEMTIME* pToday); // 00b30870: põe ano/mês/dia de pToday em pTime e converte

// 00b307b0: FILETIME -> segundos desde 1970 (0x019db1ded53e8000 = 1601 -> 1970 em 100 ns); a volta (00b307f0) está no
// achievementdlg.cpp
static void __fastcall FileTimeToSec983(const FILETIME* pFt, __int64* pOut)
{
	__int64 t = ((__int64)pFt->dwHighDateTime << 32) | pFt->dwLowDateTime;
	*pOut = (t - 116444736000000000LL) / 10000000;
}

// 00b30830: hora local -> segundos (SystemTimeToFileTime, LocalFileTimeToFileTime)
void __fastcall SysTimeToSec983(const SYSTEMTIME* pTime, __int64* pOut)
{
	FILETIME local, utc;
	SystemTimeToFileTime(pTime, &local);
	LocalFileTimeToFileTime(&local, &utc);
	FileTimeToSec983(&utc, pOut);
}

__int64 __fastcall TodayTime983(SYSTEMTIME* pTime, const SYSTEMTIME* pToday)
{
	__int64 sec = 0;
	pTime->wYear = pToday->wYear;
	pTime->wMonth = pToday->wMonth;
	pTime->wDay = pToday->wDay;
	SysTimeToSec983(pTime, &sec);
	return sec;
}
}


void __fastcall GPFormat2_983(unsigned short value, char* out); // 0055cd50 (grandprixopendlg.cpp)

static const char* s_gateway = "Gateway"; // 00e0bef8

static char s_boardTime[4]; // 00e751d4
static char s_clockTime[4]; // 00e751dc

// ------------------------------------------------------------------------------------------------------------------------
// peças do relógio

// 0055ce30: m_value (hora de 24) → m_hour, m_bPM e m_value de 12
void CGrandPrixHour::OnChange()
{
	m_bPM = m_value >= 12;
	m_hour = m_value;
	SetHour12(m_hour);
}

bool CGrandPrixMinute::Inc()
{
	++m_value;
	if (m_value >= 60)
	{
		m_value = 0;
		return true;
	}
	return false;
}

bool CGrandPrixSecond::Inc()
{
	++m_value;
	if (m_value >= 60)
	{
		m_value = 0;
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------------------------------
// CGrandPrixClock (1ª parte)

bool CGrandPrixClock::LoadBitmaps983()
{
	if (m_pAm == NULL)
		m_pAm = g_pFresh->GetBitmap("gp_time_am");
	if (m_pPm == NULL)
		m_pPm = g_pFresh->GetBitmap("gp_time_pm");
	if (m_pColon == NULL)
		m_pColon = g_pFresh->GetBitmap("gp_timecount_w");
	return true;
}

void CGrandPrixClock::GetTime983(SYSTEMTIME* pOut)
{
	*pOut = m_time;
	pOut->wHour = m_hour.Get();
	if (m_hour.m_bPM == 1)
		pOut->wHour += 12;
	pOut->wMinute = m_minute.Get();
	pOut->wSecond = m_second.Get();
}

// ------------------------------------------------------------------------------------------------------------------------
// sGPTimeBoard983 (1ª parte)

sGPTimeBoard983::sGPTimeBoard983()
	: m_pFont(NULL), m_bLimited(false), m_pInfo(NULL)
{
	m_pLimited[0] = NULL;
	m_pLimited[1] = NULL;
	m_pLimited[2] = NULL;
	m_pLimitedText = NULL;
	for (int i = 0; i < 9; ++i)
		m_pClosed[i] = NULL;
	m_pClosedText = NULL;
}

void sGPTimeBoard983::DrawTime983(float x, float y)
{
	GPFormat2_983(m_start.Get(), s_boardTime);
	m_pFont->Print(x, y, s_boardTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
	GPFormat2_983(m_startMin.Get(), s_boardTime);
	m_pFont->Print(x + 46.0, y, s_boardTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
	GPFormat2_983(m_end.Get(), s_boardTime);
	m_pFont->Print(x + 114.0, y, s_boardTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
	GPFormat2_983(m_endMin.Get(), s_boardTime);
	m_pFont->Print(x + 160.0, y, s_boardTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
}

// ------------------------------------------------------------------------------------------------------------------------
// CGrandPrixClock (2ª parte)

CGrandPrixClock::CGrandPrixClock()
	: m_elapsed(0.0f), m_bRunning(0), m_bShow(0), m_x(0.0f), m_y(0.0f), m_pFont(NULL), m_pAm(NULL), m_pPm(NULL),
	  m_pColon(NULL)
{
}

CGrandPrixClock::~CGrandPrixClock()
{
}

void CGrandPrixClock::Draw983(FrGraphicInterface* pGdi)
{
	if (m_bShow == 0)
		return;
	if (m_pFont)
	{
		GPFormat2_983(m_hour.Get(), s_clockTime);
		m_pFont->Print(m_x, m_y, s_clockTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
		GPFormat2_983(m_minute.Get(), s_clockTime);
		m_pFont->Print(m_x + 82.0, m_y, s_clockTime, (eTITLEFONTALINE)0, 0.0f, 1.0f);
	}
	const Bitmap* pAmPm = m_hour.m_bPM == 0 ? m_pAm : m_pPm;
	pGdi->DrawTexture(pAmPm, WRect(m_x + 59.0, m_y - 1.0, (float)pAmPm->Width(), (float)pAmPm->Height()), 0xffffffff, 0);
	if (m_second.Get() % 2 == 0)
	{
		pGdi->DrawTexture(m_pColon, WRect(m_x + 64.0, m_y + 14.0, (float)m_pColon->Width(), (float)m_pColon->Height()),
			0xffffffff, 0);
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// sGPTimeBoard983 (2ª parte)

void sGPTimeBoard983::DrawLimited983(float x, float y)
{
	if (!m_bLimited)
		return;
	FrGraphicInterface* pGdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pLeft = m_pLimited[0];
	pGdi->DrawTexture(pLeft, WRect(x, y, (float)pLeft->Width(), (float)pLeft->Height()), 0xffffffff, 0);
	float mid = (float)pLeft->Width() + x;
	const Bitmap* pMid = m_pLimited[1];
	pGdi->DrawTexture(pMid, WRect(0.0f, 0.0f, (float)pMid->Width(), (float)pMid->Height()),
		WRect(mid, y, 218.0f, (float)pMid->Height()), 0xffffffff, 0);
	pGdi->DrawTexture(m_pLimited[2], WRect(mid + 218.0, y, (float)m_pLimited[2]->Width(), (float)m_pLimited[2]->Height()),
		0xffffffff, 0);
	pGdi->DrawTexture(m_pLimitedText, WRect(x + 57.0, y + 12.0, (float)m_pLimitedText->Width(),
		(float)m_pLimitedText->Height()), 0xffffffff, 0);
}

// fundo de "encerrado" em 9 pedaços (o meio com 208 × 127) e o texto
void sGPTimeBoard983::DrawClosed983(float x, float y)
{
	if (!m_bClosed)
		return;
	FrGraphicInterface* pGdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* p = m_pClosed[0];
	pGdi->DrawTexture(p, WRect(x, y, (float)p->Width(), (float)p->Height()), 0xffffffff, 0);
	float x1 = (float)p->Width() + x;
	pGdi->DrawTexture(m_pClosed[1], WRect(0.0f, 0.0f, (float)m_pClosed[1]->Width(), (float)m_pClosed[1]->Height()),
		WRect(x1, y, 208.0f, (float)m_pClosed[1]->Height()), 0xffffffff, 0);
	p = m_pClosed[2];
	pGdi->DrawTexture(p, WRect(x1 + 208.0, y, (float)p->Width(), (float)p->Height()), 0xffffffff, 0);
	float y1 = (float)p->Height() + y;
	p = m_pClosed[3];
	pGdi->DrawTexture(p, WRect(0.0f, 0.0f, (float)p->Width(), (float)p->Height()),
		WRect(x, y1, (float)p->Width(), 127.0f), 0xffffffff, 0);
	x1 = (float)p->Width() + x;
	pGdi->DrawTexture(m_pClosed[4], WRect(0.0f, 0.0f, (float)m_pClosed[4]->Width(), (float)m_pClosed[4]->Height()),
		WRect(x1, y1, 208.0f, 127.0f), 0xffffffff, 0);
	pGdi->DrawTexture(m_pClosed[5], WRect(0.0f, 0.0f, (float)m_pClosed[5]->Width(), (float)m_pClosed[5]->Height()),
		WRect(x1 + 208.0, y1, (float)m_pClosed[5]->Width(), 127.0f), 0xffffffff, 0);
	float y2 = (float)m_pClosed[0]->Height() + y + 127.0;
	p = m_pClosed[6];
	pGdi->DrawTexture(p, WRect(x, y2, (float)p->Width(), (float)p->Height()), 0xffffffff, 0);
	x1 = (float)p->Width() + x;
	pGdi->DrawTexture(m_pClosed[7], WRect(0.0f, 0.0f, (float)m_pClosed[7]->Width(), (float)m_pClosed[7]->Height()),
		WRect(x1, y2, 208.0f, (float)m_pClosed[7]->Height()), 0xffffffff, 0);
	pGdi->DrawTexture(m_pClosed[8], WRect(x1 + 208.0, y2, (float)m_pClosed[8]->Width(), (float)m_pClosed[8]->Height()),
		0xffffffff, 0);
	pGdi->DrawTexture(m_pClosedText, WRect(x + 10.0, y + 47.0, (float)m_pClosedText->Width(),
		(float)m_pClosedText->Height()), 0xffffffff, 0);
}

// ------------------------------------------------------------------------------------------------------------------------
// CGrandPrixClock (3ª parte)

// a cada segundo; o minuto vira a hora; o aviso 0x753e / 3 ao "Gateway" a cada minuto
void CGrandPrixClock::Update983(float dt)
{
	if (m_bRunning == 0)
		return;
	m_elapsed += dt;
	if (m_elapsed >= 1.0)
	{
		m_elapsed -= 1.0;
		CGrandPrixTimeBase* pSecond = &m_second;
		if (pSecond->Inc())
		{
			CGrandPrixTimeBase* pMinute = &m_minute;
			if (pMinute->Inc())
			{
				CGrandPrixTimeBase* pHour = &m_hour;
				pHour->Inc();
			}
			MsgObject msg(NULL, 0x753e, 3, 0, 0, 0, 0);
			AfxGetTask()->GetActor(s_gateway) << msg;
		}
	}
}

void CGrandPrixClock::Sync983()
{
	__int64 sec;
	S5::SysTimeToSec983(&Doc()->m_serverTime, &sec);
	S5::SecToSysTime983(&m_time, sec);
	m_hour.Set(m_time.wHour);
	m_minute.Set(m_time.wMinute);
	m_second.Set(m_time.wSecond);
	m_bRunning = 1;
}

__int64 CGrandPrixClock::GetNow983()
{
	SYSTEMTIME t = m_time;
	t.wHour = m_hour.m_hour;
	t.wMinute = m_minute.Get();
	t.wSecond = m_second.Get();
	return S5::TodayTime983(&t, &Doc()->m_serverTime);
}

// ------------------------------------------------------------------------------------------------------------------------
// sGPTimeBoard983 (3ª parte)

bool sGPTimeBoard983::Init983()
{
	_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
	if (OnlinePlay() == 1)
	{
		sGPRoomItem983* pItem = pDoc->FindRoomItem983(*(unsigned long*)DOCPTR983(0x109c1));
		if (pItem == NULL)
			return false;
		m_pInfo = pItem->pInfo;
	}
	m_pLimited[0] = g_pFresh->GetBitmap("gp_limitedbox1");
	m_pLimited[1] = g_pFresh->GetBitmap("gp_limitedbox2");
	m_pLimited[2] = g_pFresh->GetBitmap("gp_limitedbox3");
	m_pLimitedText = g_pFresh->GetBitmap("gp_txt_item_limited");
	m_pClosed[0] = g_pFresh->GetBitmap("gp_closed_background1");
	m_pClosed[1] = g_pFresh->GetBitmap("gp_closed_background2");
	m_pClosed[2] = g_pFresh->GetBitmap("gp_closed_background3");
	m_pClosed[3] = g_pFresh->GetBitmap("gp_closed_background4");
	m_pClosed[4] = g_pFresh->GetBitmap("gp_closed_background5");
	m_pClosed[5] = g_pFresh->GetBitmap("gp_closed_background6");
	m_pClosed[6] = g_pFresh->GetBitmap("gp_closed_background7");
	m_pClosed[7] = g_pFresh->GetBitmap("gp_closed_background8");
	m_pClosed[8] = g_pFresh->GetBitmap("gp_closed_background9");
	m_pClosedText = g_pFresh->GetBitmap("txt_gp_closed");
	sGPInfo983* pInfo = m_pInfo;
	m_start.Set(pInfo->start[4]);
	m_startMin.Set(pInfo->start[5]);
	m_end.Set(pInfo->end[4]);
	m_endMin.Set(pInfo->end[5]);
	if (GPHasConditionEquip983(m_pInfo->ticket) == true)
		m_bLimited = GPHasTicket983(m_pInfo->ticket);
	m_bClosed = false;
	if (*((unsigned char*)DOCPTR983(0x10488)))
		m_bClosed = true;
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------
// CGPRoomTimerSteward

// 0055d180
CGPRoomTimerSteward::CGPRoomTimerSteward() : m_typeId(0)
{
}

void CGPRoomTimerSteward::OnTimer()
{
	CItemManager* pItemManager = &Doc()->m_itemManager;
	IFF983::sGrandPrixData* pData = pItemManager->FindGrandPrixData983(m_typeId);
	if (pData == NULL)
		return;
	const sGPInfo983* pInfo = (const sGPInfo983*)pData;
	CNoticeBoard::Instance()->AddNotice(MakeStr(GetGameString(0x753e), pInfo->start[4], pInfo->start[5], pInfo->name), 1,
		false);
	MsgObject msg(NULL, 0x753e, 10, m_typeId, 0, 0, 0);
	AfxGetTask()->GetActor(s_gateway) << msg;
}

// 0055d020
sGPTimeBoard983::~sGPTimeBoard983()
{
}

// 0055d150
void sGPTimeBoard983::SetFont983(WTitleFontEx* pFont)
{
	m_pFont = pFont;
}

// 0055d160
int sGPTimeBoard983::IsStartPM983()
{
	return m_start.m_bPM;
}

// 0055d170
int sGPTimeBoard983::IsEndPM983()
{
	return m_end.m_bPM;
}

// ------------------------------------------------------------------------------------------------------------------------
// auxiliares das linhas do Grand Prix

void GPItemCheckScore983(sGPRoomItem983* pItem, float value)
{
	if ((float)pItem->pInfo->m_98 > value)
		pItem->flags |= 4;
	int max = pItem->pInfo->m_9c;
	if (value > (float)max && max != 0)
		pItem->flags |= 8;
}

unsigned short GPHour12_983(unsigned short hour)
{
	if (hour > 0 && hour <= 12)
		return hour;
	else if (hour > 12 && hour < 24)
		return hour - 12;
	else if (hour == 0)
		return 12;
	return 0x63;
}

inline unsigned char GetMyLevel983()
{
	return *((unsigned char*)DOCPTR983(0x91d));
}

void GPItemCheckLevel983(sGPRoomItem983* pItem)
{
	unsigned char level = GetMyLevel983();
	if (level < pItem->pInfo->minLevel)
	{
		pItem->flags |= 1;
		return;
	}
	unsigned char maxLevel = pItem->pInfo->maxLevel;
	if (maxLevel != 0 && level > maxLevel)
		pItem->flags |= 2;
}

void GPItemCheckSpecialHole983(sGPRoomItem983* pItem)
{
	CItemManager* pItemManager = &Doc()->m_itemManager;
	if (pItemManager->FindGrandPrixSpecialHole983(pItem->pInfo->ticket))
		pItem->opts |= 1;
	else
		pItem->opts &= ~1;
}

void GPItemCheckRankReward983(sGPRoomItem983* pItem)
{
	CItemManager* pItemManager = &Doc()->m_itemManager;
	if (pItemManager->FindGrandPrixRankReward983(pItem->pInfo->ticket))
		pItem->opts |= 2;
	else
		pItem->opts &= ~2;
}

// 0 fora de hora (1 se for o typeid pedido), 2 aberto, 3 em jogo, 4 encerrado; sem fim depois do começo, a duração em minutos é
// a diferença dos minutos de começo e de fim
void GPItemUpdateState983(sGPRoomItem983* pItem, __int64* pNow, int typeId)
{
	SYSTEMTIME* pToday = &Doc()->m_serverTime;
	pItem->openTime = S5::TodayTime983((SYSTEMTIME*)pItem->pInfo->open, pToday);
	__int64 start = S5::TodayTime983((SYSTEMTIME*)pItem->pInfo->start, pToday);
	__int64 end = S5::TodayTime983((SYSTEMTIME*)pItem->pInfo->end, pToday);
	if (start < end)
	{
		if (*pNow >= end)
		{
			pItem->state = 4;
			return;
		}
		if (*pNow >= start)
		{
			pItem->state = 3;
			return;
		}
		if (*pNow >= pItem->openTime)
		{
			pItem->state = 2;
			return;
		}
		pItem->state = pItem->pInfo->typeId == typeId;
	}
	else
	{
		int minutes = pItem->pInfo->start[5] - pItem->pInfo->end[5];
		if (minutes < 0)
		{
			pItem->state = 4;
			return;
		}
		if (*pNow >= start + minutes * 60)
		{
			pItem->state = 4;
			return;
		}
		if (*pNow >= start)
		{
			pItem->state = 3;
			return;
		}
		if (*pNow >= pItem->openTime)
		{
			pItem->state = 2;
			return;
		}
		pItem->state = pItem->pInfo->typeId == typeId;
	}
}

bool GPItemCheckCondition983(sGPRoomItem983* pItem)
{
	sItemInfo* pMy = Doc()->FindMyItemByTypeId983(pItem->pInfo->m_54);
	if (pMy && pMy->Common[0] >= pItem->pInfo->m_58)
		return true;
	pItem->flags |= 0x80;
	return false;
}

bool GPHasConditionEquip983(unsigned long key)
{
	CItemManager* pItemManager = &Doc()->m_itemManager;
	return pItemManager->FindGrandPrixConditionEquip983(key) ? true : false;
}

void GPItemBuildTitle983(sGPRoomItem983* pItem)
{
	pItem->title.clear();
	const char* rule = GetGameString(0x7547);
	CItemManager* pItemManager = &Doc()->m_itemManager;
	const char* desc = pItemManager->FindArtifactDesc983(pItem->pInfo->artifact);
	if (desc != NULL)
	{
		pItem->title = rule;
		if (pItem->pInfo->m_86)
			pItem->title += GetGameString(0x18769);
		pItem->title += desc;
	}
	else if (pItem->pInfo->m_86)
	{
		pItem->title = rule;
		pItem->title += GetGameString(0x18769);
	}
	switch (pItem->pInfo->m_87)
	{
	case 1:
		if (pItem->title.size() == 0)
			pItem->title = rule;
		else
			pItem->title += ", ";
		pItem->title += GetGameString(0x7548);
		break;
	case 2:
		if (pItem->title.size() == 0)
			pItem->title = rule;
		else
			pItem->title += ", ";
		pItem->title += GetGameString(0x7549);
		break;
	}
}

// a peça vestida da categoria: chave = guid do equipamento atual, valor com `tid`
template <class Map>
bool IsEquipped983(const Map& map, unsigned long guid, unsigned long tid)
{
	typename Map::const_iterator it = map.find(guid);
	if (it == map.end())
		return false;
	return it->second.tid == tid;
}

// O 983 percorre as listas de peça, bola, item e peça auxiliar sem usar o resultado (só personagem, taco, caddie e mascote
// decidem): os laços ficam no código, sem efeito
bool GPHasTicket983(unsigned long key)
{
	std::vector<IFF983::sGrandPrixConditionEquip>* pList = Doc()->m_itemManager.FindGrandPrixConditionEquip983(key);
	if (pList)
	{
	unsigned int count = pList->size();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long tid = *(unsigned long*)(pList->at(i).raw);
		bool bEquip = true;
		switch (tid >> 26)
		{
		case 1:
			bEquip = IsEquipped983(Doc()->m_charMap, Doc()->m_myInfo.userEquip.guidChar, tid);
			break;
		case 4:
			bEquip = IsEquipped983(Doc()->m_clubSetMap, Doc()->m_myInfo.userEquip.guidClubSet, tid);
			break;
		case 7:
			bEquip = IsEquipped983(Doc()->m_caddieMap, Doc()->m_myInfo.userEquip.guidCaddie, tid);
			break;
		case 0x10:
			bEquip = IsEquipped983(Doc()->m_mascotMap, Doc()->m_myInfo.userEquip.guidMascot, tid);
			break;
		case 5:
			for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end(); ++it)
			{
				if (it->tid == tid)
					break;
			}
			break;
		case 2:
			{
				std::map<unsigned int, sCharacterInfo>::iterator it = Doc()->m_charMap.find(Doc()->m_myInfo.userEquip.guidChar);
				if (it != Doc()->m_charMap.end() && ((tid >> 18) & 0xff) == (it->second.tid & 0x3ffffff))
				{
					for (int j = 0; j < 24; ++j)
					{
						if (it->second.tidParts[j] == tid)
							break;
					}
				}
			}
			break;
		case 0x1c:
			{
				std::map<unsigned int, sCharacterInfo>::iterator it = Doc()->m_charMap.find(Doc()->m_myInfo.userEquip.guidChar);
				if (it != Doc()->m_charMap.end())
				{
					for (int j = 0; j < 5; ++j)
					{
						if (it->second.tidAuxParts[j] == tid)
							break;
					}
				}
			}
			break;
		case 6:
			for (int j = 0; j < 10; ++j)
			{
				if (Doc()->m_myInfo.userEquip.tidItemSlot[j] == tid)
					break;
			}
			break;
		}
		if (!bEquip)
			return false;
	}
	}
	return true;
}

void GPItemCheckEquip983(sGPRoomItem983* pItem)
{
	CItemManager* pItemManager = &Doc()->m_itemManager;
	std::vector<IFF983::sGrandPrixConditionEquip>* pList = pItemManager->FindGrandPrixConditionEquip983(pItem->pInfo->ticket);
	if (pList && !pList->empty() && !GPHasTicket983(pItem->pInfo->ticket))
		pItem->flags |= 0x10;
}

std::string GPRuleText983(const sGPInfo983* pInfo)
{
	std::string text;
	CItemManager* pItemManager = &Doc()->m_itemManager;
	const char* desc = pItemManager->FindArtifactDesc983(pInfo->artifact);
	if (desc)
	{
		text = GetGameString(0x7547);
		if (pInfo->m_86)
			text += GetGameString(0x18769);
		text += desc;
	}
	if (pInfo->m_87 == 1)
	{
		if (text.size() == 0)
			text = GetGameString(0x7547);
		else
			text += ", ";
		text += GetGameString(0x7548);
	}
	else if (pInfo->m_87 == 2)
	{
		if (text.size() == 0)
			text = GetGameString(0x7547);
		else
			text += ", ";
		text += GetGameString(0x7549);
	}
	return text;
}
