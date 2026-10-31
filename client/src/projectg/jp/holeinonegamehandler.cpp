// JP 983: unidade ~00579590–0057e8f0 (_gateway::_golfgame::CHoleInOneGameHandler + CTeeShotGroundBoneTransformer*). Fonte
// reconstruída do 983 (classe nova, sem par no 645). Sessão B23: registro, construtor, tabela de mensagens, Execute, três virtuais e
// 17 das 24 mensagens. Sessão B24: transformadores, OnProcess e as mensagens 0x01, 0x02, 0x08, 0x0d, 0x10, 0x18.
#include "minatl.h"
#include "projectg.h"
#include "holeinonegamehandler.h"
#include "golfdocjp.h"
#include "golfdoc.h"
#include "shareddoc.h"
#include "shareddocjp.h"
#include "vc10_layout.h"
#include "docofs983.h"
#include "initdelegate.h"
#include "packet.h"
#include "taskmanager.h"
#include "golfball.h"
#include "quadtree.h"
#include "club.h"
#include "soundmanager.h"
#include "chatmsg.h"
#include "messagemanager.h"
#include "icontentsdoc.h"
#include "golfruleholeinone.h"
#include "polysoup.h"
#include <math.h>
#include <algorithm>

using namespace _gateway;
using namespace _gateway::_golfgame;

// JP 983: campos lidos por deslocamento do 983 (= VC10). A739: ancorados nos membros (no VS 2022 o CSharedDoc/CGolfDoc andam);
// A743: todos pelo membro (C_ASSERT_VC10 abaixo).

inline CItemTableJpHio& ItemTable983() { return *(CItemTableJpHio*)(char*)&::Doc()->m_itemManager; }
#define ITEMTABLE983() (&ItemTable983())
#define ROOMPLAYER983(uid) (*(char**)&::Doc()->m_rivalList + ::Doc()->GetIndex(uid) * 0x1b8) // 1o campo do vector = _Myfirst (VC10 e cl novo)
C_ASSERT_VC10(sizeof(sRivalData) == 0x1b8 && offsetof(sRivalData, state) == 0x36 && offsetof(sRivalData, holeStroke) == 0x80 && offsetof(sRivalData, holePang) == 0x98);
C_ASSERT_VC10(offsetof(sRivalData, totalBonusPang) == 0x130 && offsetof(sRivalData, totalScore) == 0x14c && offsetof(sRivalData, rank) == 0x152); // A744: ex-pRoomPlayer + 0xNNN
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8 && offsetof(CSharedDoc, m_approachStartTime) == 0x1057c && offsetof(CSharedDoc, m_myInfo.info.dwGuid) == 0x81a); // A743: ex-DOC983_*
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameTimeLimit) == 0x106a0 && offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) == 0x20c && offsetof(CSharedDoc, m_rivalList) == 0x1042c && sizeof(sRivalData) == 0x1b8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeRandom) == 0x10534 && offsetof(CSharedDoc, m_holeStatistics) + 0x4a == 0x10012);
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8 && offsetof(CGolfDoc, m_gameMode) == 0x158);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c && offsetof(CGolfDoc, m_currentPlayer) == 0x189);


// JP 983: buraco do CGolfDoc (0x38 por buraco em +0xd8; aqui só as duas posições) e ficha do jogador na sala (0x1b8 em
// CSharedDoc+0x1042c), lidos por deslocamento. Contêiner 0xaa do CContentsDoc = CHoleInOneGameDoc (B1484: m_8c pelo membro; cru +0x8c anda no cl 14.44).
#define GOLFDOC983_CURHOLE() (GOLFDOC()->m_pHoleData + GOLFDOC()->m_currentHole - 1) // A744: ex-sHole983 cru (+0x00 tee, +0x0c pin)
#define GOLFDOC983_PLAYER() (GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer))
C_ASSERT_VC10(sizeof(sHoleData) == 0x38 && offsetof(sHoleData, pin) == 0x0c);
C_ASSERT_VC10(offsetof(sPlayerData, totalStroke) == 0x4f1 && offsetof(sPlayerData, item) == 0x4bc); // A744: ex-GOLFDOC983_PLAYER_U8/U32
extern const WVector g_vZero983_f041c4; // 00f041c4 (vetor nulo de alguma unidade do engine)

// 005796b0 = MTRand1p1(semente); 00579700 = MTRand1p1::rand(); 00579770 = rand(n) — cópias desta unidade.

float CTeeShotGroundBoneTransformerNormal::RandAngle(MTRand1p1& rand)
{
	float angle = (float)(rand.rand() * 45.0);
	return (float)((angle - 20.0) / 0.28575f);
}

float CTeeShotGroundBoneTransformerNormal::Height(float dist, float angle)
{
	return (float)((angle - -20.0) / 0.28575f * 0.75 + (dist - 25.0) * 3.2f);
}

static bool __fastcall LessAngle983(const CTeeShotGroundBoneTransformerExpert::sItem& a,
	const CTeeShotGroundBoneTransformerExpert::sItem& b) // 00579810
{
	return a.m_angle < b.m_angle;
}

float CTeeShotGroundBoneTransformerExpert::RandAngle()
{
	float angle = (float)(m_rand.rand() * 45.0);
	return (float)((angle - 20.0) / 0.28575f);
}

float CTeeShotGroundBoneTransformerExpert::Height(float dist, float angle)
{
	return (float)((angle - -20.0) / 0.28575f * 0.75 + (dist - 20.0) * 3.2f);
}

// 983: 0057a120 / 0057a150 (classe da WPolySoup; o global 00e7ea88 e o que o 009385c0 le no modo 0xb)
WPolySoup::cTeeShotGroundBoneTransformer::cTeeShotGroundBoneTransformer(unsigned long seed) : m_seed(seed)
{
}

void WPolySoup::cTeeShotGroundBoneTransformer::SetCurrent()
{
	ms_pTeeShotTransformer = this;
}

CTeeShotGroundBoneTransformerNormal::CTeeShotGroundBoneTransformerNormal(unsigned long seed)
	: cTeeShotGroundBoneTransformerJp(seed)
{
}

void CTeeShotGroundBoneTransformerExpert::AssignKinds()
{
	m_items[0].m_kind = 2;
	m_items[0].m_value = m_rand.randInt(2);
	for (unsigned int i = 1; i < m_items.size(); ++i)
	{
		sItem& item = m_items[i];
		if ((i & 1) == 0)
		{
			item.m_kind = 2;
			sItem& prev = m_items[i - 1];
			if (prev.m_kind == 1 && prev.m_value == 2)
			{
				item.m_value = (m_rand.randInt(1) != 0) + 1;
				continue;
			}
		}
		else if ((i & 3) == 1)
		{
			item.m_kind = 1;
			sItem& prev = m_items[i - 1];
			if (prev.m_kind == 2 && prev.m_value == 0)
			{
				item.m_value = m_rand.randInt(1) != 0;
				continue;
			}
		}
		else
		{
			item.m_kind = 0;
		}
		item.m_value = m_rand.randInt(2);
	}
}

IObject* CHoleInOneGameHandlerMakeInstance() // 0057e7a0
{
	return new CHoleInOneGameHandler;
}

struct __sCHoleInOneGameHandler
{
	__sCHoleInOneGameHandler() // 0057e810
	{
		ObjectFactory().AddObjectFunctor(CHoleInOneGameHandlerMakeInstance, "CHoleInOneGameHandler");
	}
};

const WRTTI CHoleInOneGameHandler::m_RTTI("CHoleInOneGameHandler", &IObject::m_RTTI);
static __sCHoleInOneGameHandler __implCHoleInOneGameHandler;

inline bool IsHoleInOneMode983(int mode)
{
	return mode >= 0x0d && mode <= 0x0e;
}

static void ReleaseHoleInOneGameHandler() // 005798a0 (vazia)
{
}

void CHoleInOneGameHandler::OnMsg04(const MsgObject& msg)
{
}

void CHoleInOneGameHandler::OnSend12c()
{
	WSendPacket packet((enumClientPacket)0x12c);
	packet.Send(TO_GAME);
}

void CHoleInOneGameHandler::OnToggleMission()
{
	m_bMission = m_bMission == false;
}

static bool AddReleaseHoleInOneGameHandler() // 0057a2c0
{
	static bool s_bAdded; // 00e75c6d
	if (!s_bAdded)
	{
		CInitDelegate::AddReleaseFunction(ReleaseHoleInOneGameHandler);
		s_bAdded = true;
	}
	return s_bAdded;
}

void CHoleInOneGameHandler::On05()
{
	CHoleInOneGameDoc* pDoc = Doc983();
	pDoc->Fn577220_983();
	pDoc->Fn576690_983();
	m_endTime = 0;
}

void CHoleInOneGameHandler::OnLeave()
{
	m_time = 0.0f;
	if (m_pTransformer)
	{
		delete m_pTransformer;
		m_pTransformer = NULL;
	}
	WPolySoup::ms_pTeeShotTransformer = NULL;
}

void CHoleInOneGameHandler::On15()
{
	m_35 = false;
	Doc983()->Fn570750_983();
}

void CTeeShotGroundBoneTransformerExpert::Apply()
{
	for (unsigned int i = 0; i < m_items.size(); ++i)
	{
		sItem& item = m_items[i];
		sTeeGroundModel983* pModel = item.m_pModel;
		float angle = RandAngle();
		float t = (float)(angle * 0.28575f);
		float height = Height(pModel->m_b0, t);
		if (!(272.0 < height))
			height = 272.0f;
		float factor = (float)((item.m_kind + (m_rand.rand() + item.m_value) / 3.0) / 3.0);
		height = (float)((height - 272.0) * factor + 272.0);

		WVector rot(0.0f, 0.0f, 0.0f);
		CHoleInOneGameDoc* pContainer;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pContainer);
		if (pContainer->m_8c)
		{
			rot.x = (float)(((float)(m_rand.rand() * 6.0) - 3.0) * 0.017453292f);
			rot.y = (float)(((float)(m_rand.rand() * 6.0) - 3.0) * 0.017453292f);
			rot.z = (float)(((float)(m_rand.rand() * 6.0) - 3.0) * 0.017453292f);
		}
		else if (::Doc()->m_roomInfo.realGameType != 0x0c && ::Doc()->m_roomInfo.realGameType != 0x0e)
		{
			rot.x = (float)(((float)m_rand.rand(6.0) - 3.0) * 0.017453292f);
			rot.y = (float)(((float)m_rand.rand(6.0) - 3.0) * 0.017453292f);
			rot.z = (float)(((float)m_rand.rand(6.0) - 3.0) * 0.017453292f);
		}
		pModel->m_pAnim->Fn934b50_983(m_pCenter, height, angle, rot);
	}
}

void CHoleInOneGameHandler::SendTimeOver()
{
	if (OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0x137);
		packet.Send(TO_GAME);
	}
	else
	{
		AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x93, 2, 0, 0, 0, 0);
	}
}

void CHoleInOneGameHandler::OnSetHole()
{
	// sHioSpot983 e WVector2D têm o mesmo layout (x, z em 8 bytes); pedido C76.1
	GOLFDOC()->SetHoleTees983(GOLFDOC()->m_currentHole, (std::vector<WVector2D>*)&Doc983()->m_spots);
}

void CHoleInOneGameHandler::OnReadScreenValue()
{
	float value = 0.0f;
	IActor* pScreen = AfxGetTask()->GetActor("Screen");
	pScreen << MsgObject(NULL, 0x246, 0, 0, 0, 0, 0);
	pScreen << MsgObject(NULL, 0xbe, (unsigned long)&value, 0, 0, 0, 0);
	Doc983()->m_44 = value;
}

void CHoleInOneGameHandler::OnGetScreenValue(const MsgObject& msg)
{
	*MSGJP(msg).Param2<float*>() = Doc983()->m_44;
}

void CHoleInOneGameHandler::OnGetPair(const MsgObject& msg)
{
	CHoleInOneGameDoc* pDoc = Doc983();
	float* p3 = MSGJP(msg).Param3<float*>();
	float* p2 = MSGJP(msg).Param2<float*>();
	*p2 = pDoc->m_3c;
	*p3 = pDoc->m_40;
}

void CHoleInOneGameHandler::OnGet7c(const MsgObject& msg)
{
	*MSGJP(msg).Param2<int*>() = Doc983()->m_7c;
}

void CHoleInOneGameHandler::OnSetTime(const MsgObject& msg)
{
	float time = *MSGJP(msg).Param2<float*>();
	m_time = time;
	if (time <= 0.0f)
		SendTimeOver();
}

void CHoleInOneGameHandler::OnWait()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x93, 0x200000, 0, 0, 0, 0));
	AfxGetTask()->GetActor("Letter") << MsgObject(NULL, 0x1fe, (unsigned long)"Wait", 0, 0, 0, 0);
}

void CHoleInOneGameHandler::On0e(const MsgObject& msg)
{
	CHoleInOneGameDoc* pDoc = Doc983();
	if (MSGJP(msg).Param2<int>())
	{
		pDoc->Fn5706d0_983();
		pDoc->Fn570770_983();
	}
	else
	{
		pDoc->Fn5706e0_983();
		pDoc->Fn570770_983();
	}
}

void CHoleInOneGameHandler::OnItem(const MsgObject& msg)
{
	CHoleInOneGameDoc* pDoc = Doc983();
	unsigned long typeId = MSGJP(msg).Param2<unsigned long>();
	if (ITEMTABLE983()->Find983(typeId)->param14 != 0)
	{
		pDoc->Fn577ed0_983();
		pDoc->Fn578580_983();
		pDoc->Fn576820_983(typeId);
		AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x16f, 1, typeId, 0, 0, 0);
	}
}

// 0057b490: troca `times` × n vezes a posição i % n com uma sorteada
void __fastcall Shuffle983(MTRand1p1& rand, unsigned int n, sTeeGroundModel983** list, int times)
{
	for (unsigned int i = 0; i < n * times; ++i)
	{
		unsigned int j = rand.randInt(n - 1);
		sTeeGroundModel983* p = list[i % n];
		list[i % n] = list[j];
		list[j] = p;
	}
}

int __fastcall ItemValuePlus983(int base, unsigned long typeId) // 0057b2b0 (só a mensagem 0x10 chama)
{
	return ITEMTABLE983()->Find983(typeId)->param14 + base;
}

void CHoleInOneGameHandler::OnSet20(const MsgObject& msg)
{
	m_20 = MSGJP(msg).Param2<int>();
}

void CHoleInOneGameHandler::OnGet20(const MsgObject& msg)
{
	*MSGJP(msg).Param2<int*>() = m_20;
}

void CHoleInOneGameHandler::OnGetMission(const MsgObject& msg)
{
	*MSGJP(msg).Param2<bool*>() = m_bMission;
}

void CHoleInOneGameHandler::OnAddTime(const MsgObject& msg)
{
	m_endTime += MSGJP(msg).Param2<int>() * 1000;
}

void CHoleInOneGameHandler::OnGetRemainTime(const MsgObject& msg)
{
	*MSGJP(msg).Param2<int*>() = m_endTime - ::Doc()->m_approachStartTime + g_CurrentTime;
}

void CHoleInOneGameHandler::SetHolePos(WVector& pos, bool bHoleIn)
{
	sHoleData* pHole = GOLFDOC983_CURHOLE();
	pHole->pin = GetPVS().GetGroundPoint(pHole->pin, false, NULL, false);
	if (GOLFDOC983_PLAYER()->totalStroke)
	{
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
		pHole->tee.x = pos.x;
		pHole->tee.y = pos.y;
		pHole->tee.z = pos.z;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xc6, bHoleIn != false, m_bMission != false, 0, 0, 0));
	}
	else
	{
		pos = GetPVS().GetGroundPoint(pos, false, &GolfBall().m_meshIndex, false);
		pHole->tee.x = pos.x;
		pHole->tee.y = pos.y;
		pHole->tee.z = pos.z;
	}
}

void CHoleInOneGameHandler::UpdateTime(float delta)
{
	if (m_time > 0.0f)
	{
		m_time -= delta;
		if (m_time <= 0.0f)
		{
			m_time = 0.0f;
			SendTimeOver();
		}
	}
}

void CHoleInOneGameHandler::On01(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	if (IsHoleInOneMode983(::Doc()->m_roomInfo.realGameType))
	{
		CHoleInOneGameDoc* pDoc = Doc983();
		unsigned int spot;
		if (m_bMission)
		{
			if (GolfBall().m_holeIn == 1)
				spot = (pDoc->m_spot + 1) % 12;
			else
				spot = pDoc->m_spot;
			if (spot > 12)
				spot = 12;
			pDoc->m_spot = spot;
		}
		else
		{
			spot = pDoc->m_spot;
			if (spot > 12)
				spot = 12;
		}
		WVector pos(pDoc->m_spots[spot].m_x, 0.0f, pDoc->m_spots[spot].m_z);
		m_30 = spot;
		bool bHoleIn = GolfBall().m_holeIn == 1;
		SetHolePos(pos, bHoleIn);
		if (bHoleIn && m_bMission == true)
		{
			sHoleData* pHole = GOLFDOC983_CURHOLE();
			WVector d = pHole->pin - pHole->tee;
			GolfClub().SelectClubByDist((float)(d.Magnitude() * 0.3125));
		}
	}
	else
	{
		unsigned long uid = pPacket->Decode4();
		float xz[2];
		pPacket->DecodeBuffer(xz, sizeof(xz));
		WVector pos(xz[0], 0.0f, xz[1]);
		if (::Doc()->m_myInfo.info.dwGuid == uid)
			SetHolePos(pos, GolfBall().m_holeIn == 1);
		AfxGetTask()->GetActor("Spot") << MsgObject(NULL, 0x128, uid, (unsigned long)xz, 0, 0, 0);
	}
}

void CHoleInOneGameHandler::On02(const MsgObject& msg)
{
	int spot = MSGJP(msg).Param2<int>();
	CHoleInOneGameDoc* pDoc = Doc983();
	if (spot < (int)pDoc->m_spots.size())
	{
		WVector pos(pDoc->m_spots[spot].m_x, 0.0f, pDoc->m_spots[spot].m_z);
		SetHolePos(pos, false);
	}
}

void CHoleInOneGameHandler::On08()
{
	float a = 0.0f;
	CHoleInOneGameDoc* pDoc = Doc983();
	float b = 0.0f;
	AfxGetTask()->GetActor("Spin") << MsgObject(NULL, 0x1cc, (unsigned long)&a, (unsigned long)&b, 0, 0, 0);
	unsigned long club = GOLFDOC983_PLAYER()->item;
	if (club >= 0x18000000 && (club <= 0x18000001 || club == 0x18000011))
	{
		pDoc->m_3c = 0.0f;
		pDoc->m_40 = 0.0f;
		return;
	}
	pDoc->m_3c = a;
	pDoc->m_40 = b;
}

void CHoleInOneGameHandler::OnProcess(float delta)
{
	if (!m_bEntered)
		return;
	UpdateTime(delta);
	if ((unsigned long)(m_endTime - ::Doc()->m_approachStartTime + g_CurrentTime) > ::Doc()->m_golfGame.gameTimeLimit)
	{
		m_bEntered = false;
		WSendPacket packet((enumClientPacket)0x12c);
		packet.Send(TO_GAME);
	}
	if (*(unsigned char*)&GOLFDOC()->m_gameMode & 0x80)
	{
		if (GolfBall().m_state == 0)
		{
			if (!m_35)
			{
				m_35 = true;
				if (GolfBall().m_holeIn == 1)
				{
					g_audio->PlaySfx("grand_skycraper_sfx");
					AfxGetTask()->GetActor("Effect") << MsgObject(NULL, 0x1ea, 10, 0, 0, 0, 0);
					AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x235, 0, 0, 0, 0, 0);
					// "홀인원" (cp949)
					AfxGetTask()->GetActor("Letter") << MsgObject(NULL, 0x1fe, (unsigned long)"\xc8\xa6\xc0\xce\xbf\xf8", 0, 0, 0, 0);
				}
			}
			g_pMainFrame983->SetGameSpeed(1.0f);
		}
	}
}

// 0057cb60 / 0057cc10: avisos no chat em vermelho (0xffff7878); ninguém chama no 983 (sem referência achada).
void __stdcall ChatSystemMsg983(int id)
{
	std::string text = _systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)id);
	CChatMsg::Instance()->AddChatMsg(text, 0xffff7878, true, false);
}

void __stdcall ChatText983(const char* psz)
{
	std::string text = psz;
	CChatMsg::Instance()->AddChatMsg(text, 0xffff7878, true, false);
}

bool CHoleInOneGameHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec<CHoleInOneGameHandler, const MsgObject&>(this, msg);
}

void CHoleInOneGameHandler::On10(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	CHoleInOneGameDoc* pDoc = Doc983();
	std::vector<unsigned long> items;
	int n = pPacket->Decode4();
	for (int i = 0; i < n; ++i)
	{
		unsigned long typeId = pPacket->Decode4();
		items.push_back(typeId);
	}
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x16f, 0, (unsigned long)&items, 0, 0, 0);
	int total = 0;
	for (std::vector<unsigned long>::iterator it = items.begin(), end = items.end(); it != end; ++it)
		total = ItemValuePlus983(total, *it);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x247, total, 0, 0, 0, 0);
	pDoc->SetItemTotal983(total);
}

bool CTeeShotGroundBoneTransformerNormal::Transform983(sTeeGroundModel983* pCenter, const TeeGroundModels983& models)
{
	MTRand1p1 rand(m_seed);
	TeeGroundModels983 list(models);
#if _MSC_VER == 1600
	Shuffle983(rand, list.size(), list._Myfirst, 3);
#else // STL novo sem _Myfirst (D734 P9)
	Shuffle983(rand, list.size(), list.data(), 3);
#endif
	float step = (float)(90.0 / (float)(unsigned int)models.size());
	for (unsigned int i = 0; i < list.size(); ++i)
	{
		sTeeGroundModel983* pModel = list[i];
		pModel->m_b0 = (float)((float)i * step + 210.0);
		float angle = RandAngle(rand);
		float t = (float)(angle * 0.28575f);
		float height = Height(pModel->m_b0, t);
		pModel->m_pAnim->Fn934b50_983(pCenter, height, angle, g_vZero983_f041c4);
	}
	return true;
}

static void RegisterHoleInOneGameHandlerActor() // 0057d5e0
{
	ActorRegistry983().Add(CHoleInOneGameHandler::m_RTTI.GetName(),
		ObjectFactory().CreateObject(CHoleInOneGameHandler::m_RTTI.GetName()));
}

CTeeShotGroundBoneTransformerExpert::CTeeShotGroundBoneTransformerExpert(int seed)
	: cTeeShotGroundBoneTransformerJp(seed), m_pCenter(NULL)
{
}

void CTeeShotGroundBoneTransformerExpert::SortByAngle()
{
	sItem& first = m_items[0];
	float dx = first.m_pModel->m_x - m_pCenter->m_x;
	float dz = first.m_pModel->m_z - m_pCenter->m_z;
	float base = (float)atan2(dz, dx);
	first.m_angle = base;
	for (unsigned int i = 1; i < m_items.size(); ++i)
	{
		sItem& item = m_items[i];
		float dx = item.m_pModel->m_x - m_pCenter->m_x;
		float dz = item.m_pModel->m_z - m_pCenter->m_z;
		float angle = (float)atan2(dz, dx);
		if (angle < base)
			angle = (float)(angle + 6.2831855f);
		item.m_angle = angle;
	}
	std::sort(m_items.begin(), m_items.end(), LessAngle983);
}

static bool AddInitHoleInOneGameHandler() // 0057d8e0
{
	static bool s_bAdded; // 00e75c6c
	if (!s_bAdded)
	{
		CInitDelegate::AddInitFunction(RegisterHoleInOneGameHandlerActor);
		s_bAdded = true;
	}
	return s_bAdded;
}

bool CHoleInOneGameHandler::OnEnter()
{
	WPolySoup::ms_pTeeShotTransformer = NULL;
	if (::Doc()->m_golfGame.gameType != 0x0b)
		return false;

	if (IsHoleInOneMode983(::Doc()->m_roomInfo.realGameType))
	{
		m_pTransformer = new CTeeShotGroundBoneTransformerExpert(rand());
		m_bMission = ::Doc()->m_roomInfo.realGameType == 0x0d;
	}
	else
	{
		m_pTransformer = new CTeeShotGroundBoneTransformerNormal(::Doc()->m_holeRandom[0]);
	}
	WPolySoup::ms_pTeeShotTransformer = m_pTransformer;

	if (m_functionMap.size() == 0)
		Initialize();
	m_pDocument->Reset();
	m_bEntered = true;
	m_35 = false;
	return true;
}

bool CTeeShotGroundBoneTransformerExpert::Transform983(sTeeGroundModel983* pCenter, const TeeGroundModels983& models)
{
	m_rand.seed(m_seed);
	m_pCenter = pCenter;
	m_items.resize(models.size());
	float dist = GOLFCLUBJP()->Fn6add30_983(0);
	for (unsigned int i = 0; i < models.size(); ++i)
	{
		m_items[i].m_pModel = models[i];
		m_items[i].m_pModel->m_b0 = dist;
	}
	SortByAngle();
	AssignKinds();
	Apply();
	m_items.clear();
	return true;
}

void CHoleInOneGameHandler::On0d(const MsgObject& msg)
{
	CHoleInOneGameDoc* pDoc = Doc983();
	sHioRecord983* pRecord = MSGJP(msg).Param2<sHioRecord983*>();
	pDoc->SetRecord983(pRecord);
	sRivalData* pRival = (sRivalData*)ROOMPLAYER983(pRecord->m_uid);
	pRival->totalScore = pRecord->m_0c;
	if (::Doc()->m_myInfo.info.dwGuid == pRecord->m_uid)
		pRival->holeStroke[0] = (unsigned char)pDoc->m_28;
}

C_ASSERT_VC10(offsetof(CSharedDoc, m_hioJackpotMode983) == 0x1044c);
C_ASSERT(offsetof(sRivalData, hioScore983) == 0x138 && offsetof(sRivalData, hioPrizePang983) == 0x140 && offsetof(sRivalData, hioTrophyTid983) == 0x148);

void CHoleInOneGameHandler::On18(const MsgObject& msg)
{
	CHoleInOneGameDoc* pDoc = Doc983();
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	::Doc()->m_hioJackpotMode983 = pPacket->Decode4();
	int n = pPacket->Decode4();
	pDoc->ResizePlayers983(n);
	for (int i = 0; i < n; ++i)
	{
		unsigned long uid = pPacket->Decode4();
		int v152 = pPacket->Decode4();
		int v14c = pPacket->Decode4();
		int holeIn = pPacket->Decode4();
		int v80 = pPacket->Decode4();
		int point = pPacket->Decode4();
		int mine = pPacket->Decode4();
		__int64 v98 = pPacket->Decode8();
		__int64 v130 = pPacket->Decode8();
		__int64 prizePang = pPacket->Decode8();
		int trophyTid = pPacket->Decode4();
		int pairs = pPacket->Decode4();
		std::vector<sHioPair983>& list = pDoc->m_players.at(i);
		for (int j = 0; j < pairs; ++j)
		{
			sHioPair983 pair;
			pair.m_a = pPacket->Decode4();
			pair.m_b = pPacket->Decode4();
			list.push_back(pair);
		}
		if (MyGuid(false) == uid)
			*(unsigned long*)((char*)::Doc()->m_holeStatistics + 0x4a) = mine;
		sRivalData* pRival = (sRivalData*)ROOMPLAYER983(uid);
		pRival->rank = (unsigned char)v152;
		pRival->totalScore = v14c;
		((int*)&pRival->hioScore983)[0] = holeIn;
		pRival->holeStroke[0] = (unsigned char)v80;
		((int*)&pRival->hioScore983)[1] = point;
		pRival->holePang[0] = v98;
		pRival->totalBonusPang = v130;
		pRival->hioPrizePang983 = prizePang;
		pRival->hioTrophyTid983 = trophyTid;
		pRival->state = 2;
		pDoc->SetPlayerPairs983(uid, &list);
	}
}

CHoleInOneGameHandler::CHoleInOneGameHandler()
	: m_time(0.0f), m_pTransformer(NULL), m_30(0), m_bMission(false), m_endTime(0)
{
}

bool CHoleInOneGameHandler::Initialize()
{
#define HIO_ARG(id, f) AddFunctionType(id, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CHoleInOneGameHandler::*)())&CHoleInOneGameHandler::f)
#define HIO_VOID(id, f) AddFunctionType(id, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CHoleInOneGameHandler::*)())&CHoleInOneGameHandler::f)
	HIO_ARG(0x01, On01);
	HIO_ARG(0x02, On02);
	HIO_VOID(0x03, OnSetHole);
	HIO_ARG(0x04, OnMsg04);
	HIO_VOID(0x05, OnReadScreenValue);
	HIO_ARG(0x06, OnGetScreenValue);
	HIO_ARG(0x07, OnGetPair);
	HIO_VOID(0x08, On08);
	HIO_ARG(0x09, OnGet7c);
	HIO_ARG(0x0a, OnSetTime);
	HIO_VOID(0x0b, OnSend12c);
	HIO_VOID(0x0c, OnWait);
	HIO_ARG(0x0e, On0e);
	HIO_ARG(0x0d, On0d);
	HIO_ARG(0x0f, OnItem);
	HIO_ARG(0x10, On10);
	HIO_ARG(0x11, OnSet20);
	HIO_ARG(0x12, OnGet20);
	HIO_VOID(0x13, OnToggleMission);
	HIO_ARG(0x14, OnGetMission);
	HIO_VOID(0x15, On15);
	HIO_ARG(0x18, On18);
	HIO_ARG(0x16, OnAddTime);
	HIO_ARG(0x17, OnGetRemainTime);
#undef HIO_ARG
#undef HIO_VOID
	return true;
}

static bool s_bInitAdded = AddInitHoleInOneGameHandler();
static bool s_bReleaseAdded = AddReleaseHoleInOneGameHandler();

// JP 983 00934b50: membro do no do chao do tee (WPolySoup::sNode; o tipo emprestado CTeeGroundAnim983 e o mesmo objeto). Pega a
// matriz do mundo deste no relativa a do no central (pCenter->m_pAnim), troca a posicao por (k*px, angle/(|xa|*|rel.xa|), k*pz)
// com k = height / (|xa| * hypot(px, pz)), aplica a rotacao `rot` e a matriz do no central e grava com SetLocal + Update(IDENTITY).
void CTeeGroundAnim983::Fn934b50_983(sTeeGroundModel983* pCenter, float height, float angle, const WVector& rot)
{
	WPolySoup::sNode* self = (WPolySoup::sNode*)this;
	WPolySoup::sNode* center = (WPolySoup::sNode*)pCenter->m_pAnim;
	const WMatrix& cw = center->m_world;
	WMatrix rel = self->m_world * ~cw;
	float lenC = sqrtf(cw.xx * cw.xx + cw.yx * cw.yx + cw.zx * cw.zx);
	float k = height / (sqrtf(rel.xm * rel.xm + rel.zm * rel.zm) * lenC);
	float lenR = sqrtf(rel.xx * rel.xx + rel.yx * rel.yx + rel.zx * rel.zx);
	float f = angle / lenR / lenC;
	float px = k * rel.xm;
	float pz = k * rel.zm;
	rel.xm = px;
	rel.ym = f;
	rel.zm = pz;
	WMatrix rotm;
	RotMat(rot, rotm);
	WMatrix res = (rotm * rel) * cw;
	self->SetLocal(res);
	self->Update(WMatrix::IDENTITY);
}
