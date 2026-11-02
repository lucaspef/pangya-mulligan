#include "minatl.h"
#include "s5/utilities.h"
#define REBANG_CLUB_CPP
unsigned char GetDifficulty()
{
	if (Doc()->m_golfGame.pCourse == NULL)
		return 1;
	return Doc()->m_golfGame.pCourse->Difficulty;
}


#include "club.h"
#include "msg983.h"
#include "vc10_layout.h"

inline sClubSet::sClubSet()
{
	pInfo = NULL;
	memset(pHead, 0, sizeof(pHead));
}

inline sFindClubSet::sFindClubSet(unsigned long tid)
{
	m_tid = tid;
}

inline bool sFindClubSet::operator()(sClubSet* pSet)
{
	return pSet->pInfo->c.TypeId == m_tid;
}

inline CClub::CClub()
	: m_pClub(NULL), m_clubList(16, 8), m_flags(0)
{
	Init();
}

inline bool CClub::Is(char* name, sClub* pClub)
{
	if (pClub)
		return strstr(pClub->name, name) ? true : false;
	return strstr(m_pClub->name, name) ? true : false;
}

#if _MSC_VER <= 1600 // sem uso; no STL novo _Nodeptr e privado
struct RbClubListTypes : std::list<sClubSet*>
{
	typedef _Nodeptr Nodeptr;
};
#endif

#include "golfdoc.h"
#include "golfball.h"
#include "quadtree.h"
#include "autotarget.h"
#include "polysoup.h"
#include "inputmanager.h"
#include "chatmsg.h"
#include "quitwindow.h"
#include "itemwindow.h"
#include "cardsystem.h"
#include "statushelper.h"
#include "golftask.h"
#include "hackingmanager.h"
#include "soundmanager.h"
#include "actor.h"
#include "taskmanager.h"
#include "slicestr.h"
#include "wresrcmng.h"
#include "wpuppet.h"
#include "packet.h"
#include "../shared/localize.h"
#include "mathconsts.h"


// HACK: these CGolfDoc members are still hidden in padding in golfdoc.h, so
// they are reached through their offsets.
// 983: m_pAutoTarget = +0x10; m_bar.gauge = +0xe8 e m_bar.gaugeStep = +0xec (o 645 tinha a barra em +0xd8; Process 006ae8d0)
C_ASSERT_VC10(offsetof(CGolfDoc, m_pAutoTarget) == 0x10);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar) + offsetof(sBar, gauge) == 0xe8 && offsetof(CGolfDoc, m_bar) + offsetof(sBar, gaugeStep) == 0xec);

// HACK: CSharedDoc members not declared yet.
#define DOC_FIELD_5B9C(doc) ((doc)->m_playingMode) // 645: +0x5b9c; 983: +0x1122c (s88)

// HACK: local content id missing from localize.h
const localContentType_t S4_RANGE_BUFF = (localContentType_t)0x70;

extern WMatrix g_camera;
extern Fresh* g_pFresh;

class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	void SetHelpMsg(int id, const char* msg);
};

namespace S5
{
	bool IsUsableCalipers();
}

void PerformPowerBarSetting();
bool IsMyTurn(bool check);
sPlayerData* PLAYER(unsigned char index);

char* clubName[] = { "1W", "2W", "3W", "4W", "5W", "2I", "3I", "4I", "5I", "6I",
	"7I", "8I", "9I", "SW", "PW", "1PT", "2PT", NULL };

float bar_speed[17] = { 460.0f, 460.0f, 460.0f, 460.0f, 460.0f, 440.0f, 440.0f,
	410.0f, 410.0f, 390.0f, 390.0f, 390.0f, 390.0f, 360.0f, 360.0f, 200.0f,
	200.0f };

float ground_accuracy[15] = { 8.0f, 8.0f, 0.0f, 1.0f, 5.0f, 5.0f, 5.0f, 5.0f,
	5.0f, 0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f };

inline bool IsUsedItembyMe(unsigned long item)
{
	if (0)
		((CClub*)0)->CClub::ClearClubSet();
	if (IsMyTurn(false))
	{
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		return GOLFDOC()->GetPlayer(cur)->item == item ? true : false;
	}
	return false;
}

CClub& GolfClub()
{
	static CClub s_club;
	return s_club;
}

CClub::~CClub()
{
	for (sClub* p = m_clubList.Start(); p; p = m_clubList.Next())
	{
		if (p)
			delete p;
	}
	m_clubList.Reset();
}

void CClub::ClearClubSet()
{
	for (std::list<sClubSet*>::iterator it = m_clubSetList.begin();
		it != m_clubSetList.end(); it++)
	{
		sClubSet* pSet = *it;
		for (int i = 0; i < 4; i++)
		{
			if (g_resrcmng)
			{
				if (pSet->pHead[i])
				{
					g_resrcmng->Release(pSet->pHead[i], false);
					pSet->pHead[i] = NULL;
				}
				if (g_resrcmng && pSet->pHeadSub[i])
				{
					g_resrcmng->Release(pSet->pHeadSub[i], false);
					pSet->pHeadSub[i] = NULL;
				}
			}
		}
		delete pSet;
	}
	m_clubSetList.clear();
	m_bClubSetInit = false;
}

void CClub::Init()
{
	AddClub("1W", 230.0f, 10.0f, 236.0f, 1.61f, 0.55f, 0);
	AddClub("2W", 210.0f, 13.0f, 204.0f, 1.41f, 0.5f, 0);
	AddClub("3W", 190.0f, 16.0f, 176.0f, 1.26f, 0.45f, 0);
	AddClub("2I", 180.0f, 20.0f, 161.0f, 1.07f, 0.45f, 1);
	AddClub("3I", 170.0f, 24.0f, 149.0f, 0.95f, 0.45f, 1);
	AddClub("4I", 160.0f, 28.0f, 139.0f, 0.83f, 0.45f, 1);
	AddClub("5I", 150.0f, 32.0f, 131.0f, 0.73f, 0.45f, 1);
	AddClub("6I", 140.0f, 36.0f, 124.0f, 0.67f, 0.41f, 1);
	AddClub("7I", 130.0f, 40.0f, 118.0f, 0.61f, 0.36f, 1);
	AddClub("8I", 120.0f, 44.0f, 114.0f, 0.57f, 0.3f, 1);
	AddClub("9I", 110.0f, 48.0f, 110.0f, 0.53f, 0.25f, 1);
	AddClub("PW", 100.0f, 52.0f, 107.0f, 0.49f, 0.18f, 2);
	AddClub("SW", 80.0f, 56.0f, 93.0f, 0.42f, 0.17f, 2);
	AddClub("1PT", 20.0f, 0.0f, 30.0f, 0.0f, 0.0f, 3);
	AddClub("2PT", 10.0f, 0.0f, 21.0f, 0.0f, 0.0f, 3);
	m_reserved = 0;
	m_powerShot = 0;
	m_approachMode = 4;
	m_barPower = 0;
	m_barRemain = 0;
	m_bar = 2;
	m_bBarLock = false;
	m_prevIndex = 0;
	m_addPower = 0;
	m_bClubSetInit = false;
	m_pClub = m_clubList.Find("1W");
	m_curIndex = m_pClub->index;
}

void CClub::AddCrypticValue()
{
	if (HackingManager::Instance())
	{
		HackingManager::Instance()->AddCrypticIntValue(&m_powerShot);
		for (sClub* p = m_clubList.Start(); p; p = m_clubList.Next())
		{
			HackingManager::Instance()->AddCrypticFloatValue(&p->range);
			HackingManager::Instance()->AddCrypticFloatValue(&p->lie);
			HackingManager::Instance()->AddCrypticFloatValue(&p->power);
			HackingManager::Instance()->AddCrypticFloatValue(&p->curve);
			HackingManager::Instance()->AddCrypticFloatValue(&p->spin);
			HackingManager::Instance()->AddCrypticFloatValue(&p->barSpeed);
		}
	}
}

void CClub::Process(float deltaTime)
{
	if (GOLFDOC()->m_tutorialMode >= 1 &&
		(!IsLocalContent(S4_REPLAY_SYSTEM) || DOC_FIELD_5B9C(Doc()) != 2))
	{
		if (IsMyTurn(false))
		{
			ClubProcess(deltaTime);
			BarProcess(deltaTime);
		}
		if (m_prevIndex != m_curIndex)
		{
			m_flags |= 1;
			if (m_pClub->type != GetType(m_prevIndex))
				m_flags |= 2;
			else
				m_flags &= ~2;
			GOLFDOC()->m_bar.gauge =
				m_pClub->type == 3 ? BAR_START : BAR_MIDDLE;
			GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.gauge;
			if (IsMyTurn(false) && (GOLFDOC()->m_gameMode & 0x200) &&
				m_curIndex < m_prevIndex && m_approachMode == 4)
			{
				WMatrix mat;
				g_camera = *GOLFDOC()->MakeTopViewCameraMatrix(mat, true);
			}
			m_prevIndex = m_curIndex;
		}
		else
		{
			m_flags &= ~3;
		}
	}
}

void CClub::ClubProcess(float deltaTime)
{
	if (g_pFresh->GetManager()->GetWheelFocus() != NULL)
		return;
	if (CQuitWindow::Instance()->IsActive())
		return;
	if (!CItemWindow::IsClosed())
		return;
	if (CChatMsg::Instance()->IsOpened())
		return;
	if (g_input->Get("LALT", true))
		return;
	if (g_input->Get("RALT", true))
		return;
	int state;
	MsgObject msg(NULL, M983(0x199), (int)&state, 0, 0, 0, 0);
	AfxGetTask()->GetActor("Player") << msg;
	if (state == 3)
		return;
	if (GOLFDOC()->m_tutorialMode == 10)
		return;
	WVector delta = g_input->GetMouseDelta();
	if (m_curIndex < m_clubList.GetCount() - 1 &&
		(delta.z < 0.0f ||
			((g_input->GetDown("PAGE_DOWN", true) ||
				 g_input->GetDown("PAD_PLUS", true)) &&
				!g_input->Get("LSHIFT", true) &&
				!g_input->Get("RSHIFT", true)) ||
			(g_input->GetDown("\xc1\xc2", true) &&
				(g_input->Get("LSHIFT", true) ||
					g_input->Get("RSHIFT", true)) &&
				!g_input->Get("LCONTROL", true) &&
				!g_input->Get("RCONTROL", true))))
	{
		if (m_powerShot == 0)
		{
			m_curIndex++;
			m_pClub = SelectClub(m_curIndex);
		}
		else
		{
			m_pClub = SelectClub(m_curIndex + 1);
			if (m_pClub == NULL || m_pClub->type == 3)
			{
				m_pClub = SelectClub(m_curIndex);
				g_audio->PlaySfx("play_unplayable");
				CHelpMsg::Instance()->SetHelpMsg(0x12, NULL);
			}
			else
			{
				m_curIndex++;
				if (m_pClub->type == 2 || m_pClub->type == 1)
					AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x32b, 0, 0, 0, 0, 0);
			}
		}
		if (S5::IsUsableCalipers() || IsUsedItembyMe(0x18000009))
			PerformPowerBarSetting();
		g_audio->PlaySfx("\xc5\xac\xb7\xb4\xb1\xb3\xc3\xbc");
	}
	if (delta.z <= 0.0f &&
		(((!g_input->GetDown("PAGE_UP", true) &&
			  !g_input->GetDown("PAD_MINUS", true)) ||
			 g_input->Get("LSHIFT", true)) ||
			g_input->Get("RSHIFT", true)))
	{
		if (!g_input->GetDown("\xbf\xec", true))
			return;
		if (!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true))
			return;
		if (g_input->Get("LCONTROL", true))
			return;
		if (g_input->Get("RCONTROL", true))
			return;
	}
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe) == true)
		m_limitIndex = 0;
	if (m_curIndex > m_limitIndex)
	{
		m_curIndex--;
		m_pClub = SelectClub(m_curIndex);
		g_audio->PlaySfx("\xc5\xac\xb7\xb4\xb1\xb3\xc3\xbc");
		if (S5::IsUsableCalipers() || IsUsedItembyMe(0x18000009))
			PerformPowerBarSetting();
		if (m_powerShot != 0 && (m_pClub->type == 2 || m_pClub->type == 1))
			AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x32b, 0, 0, 0, 0, 0);
		return;
	}
	if (CHelpMsg::Instance() && m_limitIndex)
	{
		g_audio->PlaySfx("play_unplayable");
		CHelpMsg::Instance()->SetHelpMsg(0x10, NULL);
	}
}

void CClub::BarProcess(float deltaTime)
{
	switch (m_bar)
	{
	case 0:
		if (m_flags & 1)
			m_barTime = 0;
		m_barTime += deltaTime;
		if (m_barTime > 0.7f || CChatMsg::Instance()->IsActive())
			m_bar = 1;
		break;
	case 1:
		if (!(m_flags & 1) || (CChatMsg::Instance()->IsActive() && m_bBarLock))
		{
			m_barRemain -= deltaTime * 1000.0;
			if (0.0f <= m_barRemain)
				return;
			m_barRemain = 0;
			m_bar = 2;
			if (OnlinePlay())
			{
				WSendPacket packet(0x16);
				packet.Encode1(m_curIndex);
				packet.Send(TO_GAME);
			}
			return;
		}
		goto setBar3;
	case 2:
		if (!(m_flags & 1))
			return;
		if (CChatMsg::Instance()->IsActive())
			return;
		if (m_bBarLock)
			return;
		if (GolfBall().m_groundType == 2)
		{
			if (OnlinePlay())
			{
				WSendPacket packet(0x16);
				packet.Encode1(m_curIndex);
				packet.Send(TO_GAME);
			}
			return;
		}
setBar3:
		m_bar = 3;
		m_barPower = 0;
		m_barRemain = 456.0f;
		break;
	case 3:
		m_barPower += deltaTime * 765.0;
		if (m_barPower > 255.0)
		{
			m_barTime = 0;
			m_bar = 0;
			m_barPower = 255.0f;
			return;
		}
		break;
	}
}

void CClub::AddClub(char* name, float range, float lie, float power,
	float curve, float spin, unsigned char type)
{
	sClub* pClub = m_clubList.Find(name);
	if (pClub)
	{
		lie = lie * g_DEGTORAD;
		pClub->range = range;
		pClub->lie = lie;
		pClub->type = type;
		pClub->power = power;
		pClub->curve = curve;
		pClub->spin = spin;
		return;
	}
	pClub = new sClub;
	pClub->pointFactor = 1.0f;
	lie = lie * g_DEGTORAD;
	pClub->range = range;
	pClub->lie = lie;
	pClub->type = type;
	pClub->power = power;
	pClub->curve = curve;
	pClub->spin = spin;
	pClub->index = (unsigned char)m_clubList.GetCount();
	strcpy(pClub->name, name);
	m_clubList.AddItem(pClub, pClub->name, false);
	for (unsigned char i = 0; i < 17; i++)
	{
		if (strcmp(pClub->name, clubName[i]) == 0)
		{
			pClub->id = i;
			break;
		}
	}
	pClub->barSpeed = bar_speed[pClub->id];
	if (HackingManager::Instance())
	{
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->range);
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->lie);
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->power);
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->curve);
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->spin);
		HackingManager::Instance()->AddCrypticFloatValue(&pClub->barSpeed);
	}
}

CClub::sClub* CClub::SelectClub(unsigned char index)
{
	sClub* pClub = m_clubList.Start();
	for (unsigned char i = 0; i < index; i++)
		pClub = m_clubList.Next();
	return pClub;
}

void CClub::SelectClubByDist(float dist)
{
	m_curIndex = m_clubList.GetCount() - 1;
	m_pClub = m_clubList.End();
	while (m_pClub && m_curIndex != m_limitIndex &&
		((m_powerShot != 0 && m_pClub->type == 3) || !(GetRange() > dist)))
	{
		m_pClub = m_clubList.Prev();
		m_curIndex--;
	}
	if (m_pClub == NULL)
	{
		m_curIndex = 0;
		m_pClub = m_clubList.Find("1W");
	}
}

void CClub::SelectBestClub()
{
	WVector pin = GOLFDOC()->GetHoleData().pin;
	SelectBestClub(pin);
	if (S5::IsUsableCalipers())
		PerformPowerBarSetting();
}

void CClub::SelectBestClub(const WVector& pin)
{
	if (0) // HACK: keeps the original emission dependency on GetWideAccuracy.
		((CClub*)0)->GetWideAccuracy();
	const WVector& pos = GolfBall().m_pos;
	float dx = pos.x - pin.x;
	float dz = pos.z - pin.z;
	m_limitIndex = 0;
	dx *= dx;
	float dist = sqrtf(dz * dz + dx) * 0.3125f;
	if (dist < 10.0f)
		m_approachMode = 0;
	else if (dist < 15.0f)
		m_approachMode = 1;
	else if (dist < 28.0f)
		m_approachMode = 2;
	else if (dist < 58.0f)
		m_approachMode = 3;
	else
		m_approachMode = 4;
	SetAccuracy();
	if (GolfBall().m_groundType == 2)
	{
		switch (m_approachMode)
		{
		case 0:
			m_bOffGreen = 0;
			m_pClub = m_clubList.Find("2PT");
			break;
		case 1:
			m_bOffGreen = 0;
			m_pClub = m_clubList.Find("1PT");
			break;
		case 2:
			m_bOffGreen = 1;
			m_pClub = m_clubList.Find("2PT");
			break;
		case 3:
		case 4:
			m_bOffGreen = 1;
			m_pClub = m_clubList.Find("1PT");
			break;
		}
		m_curIndex = m_pClub->index;
		m_limitIndex = 13;
		GOLFDOC()->m_bar.gauge = 140.0f;
		GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.gauge;
	}
	else
	{
		m_bOffGreen = 1;
		float best = -10000.0f;
		int meshIndex = GetPVS().GetMeshIndex(GolfBall().m_pos, false);
		CAutoTarget* pAuto = GOLFDOC()->m_pAutoTarget;
		WVector pt = pAuto->GetPoint("1W");
		WVector dir;
		dir.x = pt.x - GolfBall().m_pos.x;
		dir.y = 0;
		dir.z = pt.z - GolfBall().m_pos.z;
		dir.Normalize();
		bool bHazard = false;
		if (Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].stat.Level < 6 &&
			GetDifficulty() < 3)
		{
			float r = GetRange("1W") * 3.2f;
			for (float f = 0.8f; f < 1.01f; f += 0.1f)
			{
				float len = r * f;
				if (GetPVS().GetGroundType(GolfBall().m_pos + dir * len, true,
						NULL) == 10)
				{
					bHazard = true;
					break;
				}
				if (GOLFDOC()->m_pPolySoup->IsOutOfBound(
						GolfBall().m_pos + dir * len))
				{
					bHazard = true;
					break;
				}
			}
		}
		for (sClub* pClub = m_clubList.Start(); pClub;
			pClub = m_clubList.Next())
		{
			if (pClub->index < 13)
			{
				if (GetPVS().GetGroundSlope(meshIndex) + 0.06981317f <=
					pClub->lie)
				{
					float range = GetRange(pClub->name);
					float score = range - dist;
					if (score <= 0.0f)
						score = -score;
					score = pClub->barSpeed * 0.5f - score;
					if (dist < range)
						score += 10.0f;
					if (bHazard)
					{
						float r = range * 3.2f;
						for (float f = 0.8f; f < 1.01f; f += 0.1f)
						{
							float len = r * f;
							if (GOLFDOC()->m_pPolySoup->IsOutOfBound(
									GolfBall().m_pos + dir * len))
							{
								score -= 1000.0f;
							}
							else
							{
								switch (GetPVS().GetGroundType(
									GolfBall().m_pos + dir * len, true, NULL))
								{
								case 3:
									score -= 200.0f;
									break;
								case 4:
								case 6:
								case 7:
								case 11:
									score -= 100.0f;
									break;
								case 10:
									score -= 1000.0f;
									break;
								}
							}
						}
					}
					score *= pClub->pointFactor;
					if (score > best)
					{
						m_pClub = pClub;
						m_curIndex = pClub->index;
						best = score;
					}
				}
				else
				{
					m_limitIndex++;
				}
			}
		}
		if (m_approachMode < 4)
		{
			m_limitIndex = Max<unsigned char>(m_limitIndex, 7);
			m_clubList.Start();
			sClub* p = NULL;
			for (int i = 0; i < 11; i++)
				p = m_clubList.Next();
			m_pClub = p;
			m_curIndex = p->index;
		}
		if (GolfBall().m_groundType == 3)
		{
			m_clubList.Start();
			sClub* p = NULL;
			for (int i = 0; i < 12; i++)
				p = m_clubList.Next();
			m_pClub = p;
			m_curIndex = p->index;
		}
	}
	if (GolfBall().m_groundType == 0)
	{
		if (Doc()->m_golfGame.gameType == 1)
		{
			unsigned char cur = GOLFDOC()->m_currentPlayer;
			sPlayerData* pPlayer = GOLFDOC()->GetPlayer(cur);
			unsigned char team = pPlayer->team;
			sTeamData* pTeam = GOLFDOC()->GetTeam(team);
			if (pTeam->stroke[GOLFDOC()->m_currentHole - 1] <
					(unsigned char)2 &&
				m_curIndex > 2)
			{
				m_pClub = m_clubList.Find("3W");
				m_curIndex = m_pClub->index;
			}
		}
		else
		{
			unsigned char cur = GOLFDOC()->m_currentPlayer;
			sPlayerData* pPlayer = GOLFDOC()->GetPlayer(cur);
			// 983 006b0630: fora do time tambem exige !m_b195_983 (30battle) para trocar ao 3W
			if (pPlayer->stroke[GOLFDOC()->m_currentHole - 1] <
					(unsigned char)2 &&
				!GOLFDOC()->m_b195_983 &&
				m_curIndex > 2)
			{
				m_pClub = m_clubList.Find("3W");
				m_curIndex = m_pClub->index;
			}
		}
	}
	if (GOLFDOC()->m_tutorialMode == 10)
	{
		m_pClub = m_clubList.Find("1W");
		m_prevIndex = m_curIndex = m_pClub->index;
		return;
	}
	m_prevIndex = m_curIndex;
}

void CClub::SetAccuracy()
{
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(cur);
	m_accuracy = (float)(int)pPlayer->accuracy +
		ground_accuracy[GolfBall().m_groundType];
	float wide = m_accuracy + 10.0f;
	if (35.0f < wide)
		wide = 35.0f;
	m_wideAccuracy = wide;
	// 983 006adc00: evento 0xe (flags +0x4e4) em vez do S3_SAFETEE_ITEM/item 0x18000028 (+3) do 645;
	// soma ground_accuracy[1] (8.0, DAT_00e15d94)
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe) == true)
	{
		unsigned char cur2 = GOLFDOC()->m_currentPlayer;
		pPlayer = GOLFDOC()->GetPlayer(cur2);
		m_accuracy = (float)(int)pPlayer->accuracy + ground_accuracy[1];
	}
}

void CClub::SetApproachMode()
{
	WVector pin = GOLFDOC()->GetHoleData().pin;
	CGolfBall& ball = GolfBall();
	m_limitIndex = 7;
	float dist = sqrtf((ball.m_pos.z - pin.z) * (ball.m_pos.z - pin.z) +
					 (ball.m_pos.x - pin.x) * (ball.m_pos.x - pin.x)) *
		0.3125f;
	if (dist < 10.0f)
	{
		m_bOffGreen = 0;
		m_approachMode = 0;
	}
	else if (dist < 15.0f)
	{
		m_bOffGreen = 0;
		m_approachMode = 1;
	}
	else
	{
		if (dist < 28.0f)
		{
			m_approachMode = 2;
		}
		else if (dist < 58.0f)
		{
			m_approachMode = 3;
		}
		else
		{
			m_approachMode = 4;
			m_limitIndex = 0;
		}
		m_bOffGreen = 1;
	}
	if (GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2)
		m_bOffGreen = 1;
}

float CClub::GetNarrowAccuracy()
{
	if (m_pClub->type == 3)
		return 0.0f;
	float f = (float)ceil(
		(double)((GolfClub().m_powerShot != 0 ? 0.6f : 1.0f) * m_accuracy));
	if (f < 2.0f)
		f = 2.0f;
	return f;
}

float CClub::GetWideAccuracy()
{
	if (m_pClub->type == 3)
		return 0.0f;
	float f = (float)ceil(
		(double)((GolfClub().m_powerShot != 0 ? 0.6f : 1.0f) * m_accuracy));
	if (f < 2.0f)
		f = 2.0f;
	f = f + 10.0f;
	if (f > 35.0f)
		f = 35.0f;
	return f;
}

// 983 (006aff10): sem o ramo do 1W "lucky" (GetAirNightLucky*) e sem RANGE_BUFF; no putter fora do green soma 10 se o
// evento 0x40 estiver ativo; no fim: cartoes + (mascote + (GetAuxPartProperty + GetRangeBonus983) + alcance)
float __fastcall MascotProperty983(unsigned char player, unsigned char type);
float CClub::GetRange(const char* name)
{
	sClub* pClub = name ? m_clubList.Find(name) : m_pClub;
	float range = pClub->range;
	switch (pClub->type)
	{
	case 0:
	{
		float sum = GetPowerShotFactor() + pClub->range;
		float power = (float)(int)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->power;
		range = (power + power) + sum;
		break;
	}
	case 1:
		range = GetPowerShotFactor() + pClub->range;
		break;
	case 2:
		switch (m_approachMode)
		{
		case 0:
		case 1:
		case 2:
			range = (m_powerShot != 0 ? 30.0f : 0.0f) + 30.0f;
			break;
		case 3:
			range = (m_powerShot != 0 ? 20.0f : 0.0f) + 60.0f;
			break;
		case 4:
			range = GetPowerShotFactor() + pClub->range;
			break;
		}
		break;
	case 3:
		if (m_bOffGreen)
			range = Is("1PT", pClub) ? 40.0f : 30.0f;
		if (GOLFDOC()->IsCurrentPlayerEvent983(0x40) == true)
			range += 10.0f;
		break;
	}
	if (m_pClub->type != 3)
	{
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		float aux = Doc()->GetAuxPartProperty(cur, 0);
		float bonus = Doc()->GetRangeBonus983(cur);
		aux = bonus + aux;
		float mascot = MascotProperty983(cur, 0);
		CCardManager::Instance()->SetPlayerIndex(GOLFDOC()->m_currentPlayer);
		bool bPower = false;
		if (GolfClub().m_powerShot != 0)
			bPower = true;
		float card = CCardManager::Instance()->GetTotalRange(bPower);
		return card + ((mascot + aux) + range);
	}
	return range;
}

// JP 983 (006add30): alcance do 1W de um jogador sem tacada forte: taco + 2 x forca + pecas auxiliares +
// bonus novo (006362a0, sempre do jogador da vez) + cartoes. O indice do CCardManager volta ao jogador da vez.
float CClub::GetRangeByEquip983(int player, sUserEquip* pEquip) // 006ad580
{
	if (pEquip->guidChar == 0)
		return 0.0f;

	sCharacterInfo* pChar;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		pChar = &Doc()->m_userInfo[player].charInfo;
	else
		pChar = Doc()->FindChar983(pEquip->guidChar);
	if (pChar == NULL)
		return 0.0f;

	if (!IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (Doc()->FindClubSet983(pEquip->guidClubSet) == NULL)
			return 0.0f;
	}

	// o 983 procura o caddie e o mascote e nao usa o resultado
	if (!IS_KINDOF(CGolfTask, AfxGetTask()))
		Doc()->FindCaddie983(pEquip->guidCaddie);
	if (!IS_KINDOF(CGolfTask, AfxGetTask()))
		Doc()->GetMyMascotInfo(pEquip->guidMascot);
	IS_KINDOF(CGolfTask, AfxGetTask());

	CStatusHelper status;
	int power = status.GetStatus983(0);
	if (power < 0)
		power = 0;
	else if (power > 100)
		power = 100;
	power -= 15;

	float range = m_clubList.Find("1W")->range;
	range = (float)power + (float)power + range;

	float aux;
	float mascot;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		aux = Doc()->GetAuxPartRange983(player);
		aux = Doc()->GetRangeBonus983(player) + aux;
		mascot = Doc()->GetMascotRange983(player);
	}
	else
	{
		aux = Doc()->GetAuxPartRange983(0);
		aux = Doc()->GetRangeBonus983(0) + aux;
		mascot = Doc()->GetMascotRange983(0);
	}
	aux = mascot + aux;
	range = aux + range;

	CCardManager::Instance()->SetPlayerIndex(player);
	float card = CCardManager::Instance()->GetTotalRange(false);
	return card + range;
}

float CClub::GetDriverRange983(unsigned char player)
{
	float range = m_clubList.Find("1W")->range;
	float power = (float)(int)GOLFDOC()->GetPlayer(player)->power;
	float aux = Doc()->GetAuxPartProperty(player, 0);
	aux += Doc()->GetRangeBonus983(GOLFDOC()->m_currentPlayer);
	CCardManager::Instance()->SetPlayerIndex(player);
	float card = (float)CCardManager::Instance()->GetTotalRange(false);
	CCardManager::Instance()->SetPlayerIndex(GOLFDOC()->m_currentPlayer);
	range = power + power + range;
	range += aux;
	range += card;
	return range;
}

float CClub::GetLie(const char* name)
{
	sClub* pClub = name ? m_clubList.Find(name) : m_pClub;
	if (m_approachMode == 4)
		return pClub->lie;
	float initSpin = GolfBall().m_initSpin;
	return initSpin * 0.06981316953897476f + pClub->lie;
}

// 983 (006b0180): sem o ramo RANGE_BUFF e sem GetAirNightLucky*; soma GetAuxPartProperty + GetRangeBonus983 + mascote
float __fastcall MascotProperty983(unsigned char player, unsigned char type);
float CClub::GetPower()
{
	float result = (float)m_pClub->power;
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	float aux = Doc()->GetAuxPartProperty(cur, 0);
	float rangeBonus = Doc()->GetRangeBonus983(cur);
	float mascot = MascotProperty983(cur, 0);
	float add = 0.0f;
	if (GolfClub().m_powerShot != 0)
		add = GetPowerShotFactor() + m_addPower;
	CCardManager::Instance()->SetPlayerIndex(GOLFDOC()->m_currentPlayer);
	bool bPower = false;
	if (GolfClub().m_powerShot != 0)
		bPower = true;
	float cardRange = CCardManager::Instance()->GetTotalRange(bPower);
	aux = cardRange + (mascot + (rangeBonus + aux));
	switch (m_pClub->type)
	{
	case 0:
	{
		sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		float base = (float)(aux * 0.5 + (float)(int)pPlayer->power);
		return (float)(((base + base + add) * 1.5) / (float)m_pClub->range + 1.0) *
			(float)m_pClub->power;
	}
	case 1:
		return (float)((float)m_pClub->power * aux * 1.2999999523162842) / (float)m_pClub->range +
			(float)(add / (float)m_pClub->range + 1.0) * (float)m_pClub->power;
	case 2:
	{
		switch (m_approachMode)
		{
		case 0:
		case 1:
		case 2:
		{
			float f = 28.0f;
			if (m_powerShot == 0)
				f = 0.0f;
			result = (float)((f + 52.0) * ((GetLie() * 0.5) / 0.9773843735456467 + 0.5));
			break;
		}
		case 3:
		{
			float f = 18.0f;
			if (m_powerShot == 0)
				f = 0.0f;
			result = (float)((f + 80.0) * ((GetLie() * 0.5) / 0.9773843735456467 + 0.5));
			break;
		}
		case 4:
			result = (float)(add / (float)m_pClub->range + 1.0) * (float)m_pClub->power;
			break;
		}
		return ((float)m_pClub->power * aux) / (float)m_pClub->range + result;
	}
	case 3:
		if (m_bOffGreen)
		{
			if (strstr(m_pClub->name, "1PT") == NULL)
				result = 36.0f;
			else
				result = 42.0f;
		}
		if (GOLFDOC()->IsCurrentPlayerEvent983(0x40) == true)
		{
			if (m_bOffGreen)
			{
				if (strstr(m_pClub->name, "1PT") == NULL)
					return 42.0f;
				return 50.0f;
			}
			if (strstr(m_pClub->name, "1PT") != NULL)
				return 36.0f;
			result = 30.0f;
		}
		return result;
	}
	return result;
}

void CClub::ClearPointFactor()
{
	for (sClub* p = m_clubList.Start(); p; p = m_clubList.Next())
		p->pointFactor = 1.0f;
}

void CClub::SetPointFactor(float factor)
{
	m_pClub->pointFactor = factor;
}

unsigned char CClub::GetType(unsigned char index)
{
	for (sClub* p = m_clubList.Start(); p; p = m_clubList.Next())
	{
		if (p->index == index)
			return p->type;
	}
	return 0;
}

void CClub::SetClub(const char* name)
{
	m_pClub = m_clubList.Find(name);
	m_curIndex = m_pClub->index;
}

void CClub::SetClub(unsigned char index)
{
	m_pClub = SelectClub(index);
	m_curIndex = m_pClub->index;
}

sClubSet* CClub::AddClubSet(unsigned long tid)
{
	sClubSet* pSet = FindClubSet(tid);
	if (pSet == NULL)
	{
		IFF_STRUCT::sClubSet* pInfo = ItemManager()->FindClubSet(tid);
		if (pInfo)
		{
			pSet = new sClubSet;
			pSet->pInfo = pInfo;
			for (int i = 0; i < 4; i++)
			{
				IFF_STRUCT::sClub* pIffClub =
					ItemManager()->FindClub(pInfo->ClubIds[i]);
				pSet->pHead[i] = g_resrcmng->GetPuppet(
					MakeStr("%s.pet", pIffClub->Data), false, true, false);
				pSet->pHead[i]->CalcBound(NULL, true);
				pSet->pHeadSub[i] = g_resrcmng->GetPuppet(
					MakeStr("%s.pet", pIffClub->Data), false, true, false);
				pSet->pHeadSub[i]->CalcBound(NULL, true);
				w_bound_box* pCamera = pSet->pHead[i]->FindBoundBox("camera");
				w_bound_box* pTarget = pSet->pHead[i]->FindBoundBox("target");
				sClubSet::sFrame& f = pSet->frame[i];
				sClubSet::sGuide& g = pSet->guide[i];
				g.diffuse = 0xffffff;
				g.ambient = 0x808080;
				g.ambient2 = 0xffffff;
				g.type = 2;
				if (pCamera == NULL || pTarget == NULL)
				{
					f.pivot.z = 0.0f;
					f.pivot.y = 0.0f;
					f.pivot.x = 0.0f;
					f.za.y = 0.0f;
					f.za.x = 0.0f;
					f.ya.z = 0.0f;
					f.ya.x = 0.0f;
					f.xa.z = 0.0f;
					f.xa.y = 0.0f;
					f.za.z = 1.0f;
					f.ya.y = 1.0f;
					f.xa.x = 1.0f;
					WVector gd;
					gd.x = f.ya.x * 10.0f + f.pivot.x;
					gd.y = f.ya.y * 10.0f + f.pivot.y;
					gd.z = f.ya.z * 10.0f + f.pivot.z;
					pSet->dist983[i] = 10.0f;
					g.nearOne = gd;
				}
				else
				{
					float angle = 0.0f;
					WSliceStr option(pCamera->info->option);
					option.Get("*angle", "%f", &angle);
					f.pivot = pCamera->info->spherePivot;
					WVector d;
					d.z = pTarget->info->spherePivot.z -
						pCamera->info->spherePivot.z;
					d.y = pTarget->info->spherePivot.y -
						pCamera->info->spherePivot.y;
					d.x = pTarget->info->spherePivot.x -
						pCamera->info->spherePivot.x;
					f.za = d.Normalize();
					float c = (float)cos((double)(angle * g_DEGTORAD));
					float sn = (float)sin((double)(angle * g_DEGTORAD));
					WVector t;
					t.z = f.za.y * 0.0f - c * f.za.x;
					t.y = sn * f.za.x - f.za.z * 0.0f;
					t.x = c * f.za.z - sn * f.za.y;
					f.xa = t.Normalize();
					WVector u;
					u.x = f.za.y * f.xa.z - f.xa.y * f.za.z;
					u.y = f.za.z * f.xa.x - f.xa.z * f.za.x;
					u.z = f.xa.y * f.za.x - f.za.y * f.xa.x;
					f.ya = u;
					pSet->dist983[i] = (float)sqrt((double)(d.x * d.x + d.y * d.y + d.z * d.z));
					WVector gd;
					gd.x = (f.ya.x * 10.0f + f.pivot.x) -
						pTarget->info->spherePivot.x;
					gd.y = (f.ya.y * 10.0f + f.pivot.y) -
						pTarget->info->spherePivot.y;
					gd.z = (f.ya.z * 10.0f + f.pivot.z) -
						pTarget->info->spherePivot.z;
					g.nearOne = gd;
				}
				g.nearOne.Negate();
				g.nearOne.Normalize();
			}
			m_clubSetList.push_back(pSet);
		}
	}
	return pSet;
}


sClubSet* CClub::FindClubSet(unsigned long tid)
{
	std::list<sClubSet*>::iterator it;
	it = std::find_if(m_clubSetList.begin(), m_clubSetList.end(),
		sFindClubSet(tid));
	if (it != m_clubSetList.end())
		return *it;
	return NULL;
}

float CClub::GetPowerShotFactor()
{
	float table[4] = { 0.0f, 10.0f, 20.0f, 15.0f };
	return table[m_powerShot];
}

float CClub::GetAirNightLuckyRange(char* name)
{
	float result = 0.0f;
	if (stricmp(name, "1W") == 0)
		result = 216.0f;
	else if (stricmp(name, "2W") == 0)
		result = 206.0f;
	else if (stricmp(name, "3W") == 0)
		result = 196.0f;
	return result;
}

float CClub::GetAirNightLuckyPower(char* name)
{
	float result = 0.0f;
	if (stricmp(name, "1W") == 0)
		result = 216.0f;
	else if (stricmp(name, "2W") == 0)
		result = 194.0f;
	else if (stricmp(name, "3W") == 0)
		result = 176.0f;
	return result;
}

const char* CClub::GetClubString(eClub club)
{
	switch (club)
	{
	case CLUB_WOOD_1:
		return "WOOD_1";
	case CLUB_WOOD_2:
		return "WOOD_2";
	case CLUB_WOOD_3:
		return "WOOD_3";
	case CLUB_IRON_2:
		return "IRON_2";
	case CLUB_IRON_3:
		return "IRON_3";
	case CLUB_IRON_4:
		return "IRON_4";
	case CLUB_IRON_5:
		return "IRON_5";
	case CLUB_IRON_6:
		return "IRON_6";
	case CLUB_IRON_7:
		return "IRON_7";
	case CLUB_IRON_8:
		return "IRON_8";
	case CLUB_IRON_9:
		return "IRON_9";
	case CLUB_WEDGE_P:
		return "WEDGE_P";
	case CLUB_WEDGE_S:
		return "WEDGE_S";
	case CLUB_PUTTER_1:
		return "PUTTER_1";
	case CLUB_PUTTER_2:
		return "PUTTER_2";
	default:
		return "NOT DEFINED";
	}
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?MakeTopViewCameraMatrix@CGolfDoc@@QAEPAVWMatrix@@AAV2@_N@Z=?MakeTopViewCameraMatrix@CGolfDoc@@QAE?AVWMatrix@@_N@Z") // retorno por valor = ponteiro oculto 1o
#pragma comment(linker, "/alternatename:?SetHelpMsg@CHelpMsg@@QAEXHPBD@Z=?SetHelpMsg@CHelpMsg@@QAEXHPAD@Z") // const char x char
