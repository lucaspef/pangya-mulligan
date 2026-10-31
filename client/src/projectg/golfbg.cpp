#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "shadow.h"
#include "wind.h"
#include "fx.h"
#include "quadtree.h"
#include "polysoup.h"
#include "scenemanager.h"
#include "clientsetting.h"
#include "golfball.h"
#include "packet.h"
#include "golfbg.h"
#include "autotarget.h"
#include "sky.h"
#include "../shared/localize.h"
#include "mathconsts.h"
#include "taskmanager.h"
#include "jp/posteffect983.h"
#include "jp/shareddocjp.h" // 983: GetHoleValue983 (00633560)


extern bool g_bQuit;

WVector GetTeeShotPos();

IMPLEMENT_ACTOR(CGolfBg, IActor)

CGolfBg::CGolfBg()
{
	m_dispPriority = DISP_PRIORITY_1;

	m_renderWeather = 0;
}

void CGolfBg::OnDestroy()
{
	if (CShadowManager::Instance())
		delete CShadowManager::Instance();
}

// course name ("Data" field of the course IFF entry) of the current game
#define GetCourseName() \
	(Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL)

// Course.iff TypeId of a map index (0x80+ are the "event" variants)
#define GetCourseTypeId(map) \
	((((map) > 0x7f && (map) != 0xfd) ? (map) - 0x80 : (map)) | 0x28000000)

// 983: 0072e030
void CGolfBg::SetGridCenter(WVector& pos)
{
	// 983: nao grava em pos; SendMsg expandido (GetActor + HandleMsg)
	WVector ground = GetPVS().GetGroundPoint(pos, false, NULL, false);

	MsgObject msgGrid(this, M983(0x74), (int)&ground, 0, 0, 0, 0);
	GetActor("Grid") << msgGrid;

	MsgObject msgRule(this, M983(0x74), (int)&ground, 0, 0, 0, 0);
	GetActor("GolfRule") << msgRule;
}

// 983: GetHoleIndex expandido no chamador (indice 1-based em m_holeOrder; 0x13 = nao achou)
static inline unsigned char HoleIndexInline983(unsigned char hole)
{
	unsigned char i = 0;
	do
	{
		if (Doc()->m_holeOrder[i] == hole)
			return i + 1;
		i++;
	} while (i < 0x12);
	return 0x13;
}

static __forceinline void RefreshHoleWnd983()
{
	CTask* task = CTaskManager::Instance() ? CTaskManager::Instance()->GetCurrentTask() : NULL;
	CPostEffect983* fx = task ? task->GetPostEffect983() : NULL;
	if (fx && fx->IsActive())
		fx->LoadShaders();
}

void CGolfBg::DoReLoad()
{
	SendMsg(this, "Ambient", M983(0x1f4), 0, 0, 0, 0);
	SendMsg(this, "Player", M983(0x140), 0, 0, 0, 0);

	if (CFx::Instance())
		CFx::Instance()->SetActive(false);

	int hole = GOLFDOC()->m_currentHole;

	if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
	{
		GOLFDOC()->m_pAutoTarget->Reset();

		unsigned char mapHole = DOCJP()->GetHoleValue983(
			HoleIndexInline983((unsigned char)hole));
		if (!GOLFDOC()->m_pPolySoup->Load(
				MakeStr("%s_%02d.gbin", GetCourseName(), mapHole, 0), false))
		{
			g_bQuit = true;
			return;
		}

		GOLFDOC()->m_waveInfo.Load(MakeStr("%s_wave.txt", GetCourseName()));
		GOLFDOC()->m_riverInfo.Load(MakeStr("%s_river.txt", GetCourseName()));

		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(GetCourseTypeId(Doc()->m_pinIndex[17]));
		if (pCourse)
		{
			char* name = pCourse->Data;
			if (name)
			{
				if (GOLFDOC()->m_pSky)
				{
					GOLFDOC()->m_pSky->Clear();
					delete GOLFDOC()->m_pSky;
					GOLFDOC()->m_pSky = NULL;
				}

				GOLFDOC()->m_pSky = new CSky;
				GOLFDOC()->m_pSky->LoadSky(name, 0.0f);
				GOLFDOC()->m_pSky->RegisterCloud(MakeStr("%s_cloud.dds", name));
			}
		}

		GOLFDOC()->m_pPolySoup->LoadMapCheckData(hole);
		CSceneManager::Instance()->SetWeather(Doc()->m_golfGame.weather, true);
		RefreshHoleWnd983();
	}
	else
	{
		bool special = false;
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1 &&
			Doc()->m_itemManager.HasGrandPrixSpecialHole983(
				Doc()->m_roomInfo.gpTicket983))
			special = true;

		unsigned char mapHole = DOCJP()->GetHoleValue983(
			HoleIndexInline983((unsigned char)hole));
		GetCurMap(); // 983: resultado descartado
		if (!GOLFDOC()->m_pPolySoup->Load(
				MakeStr("%s_%02d.gbin", GetCourseName(), mapHole, 0), false))
		{
			g_bQuit = true;
			return;
		}

		if (special)
		{
			GOLFDOC()->m_waveInfo.Load(
				MakeStr("%s_wave.txt", GetCourseName()));
			GOLFDOC()->m_riverInfo.Load(
				MakeStr("%s_river.txt", GetCourseName()));
			RefreshHoleWnd983();
		}

		if (GOLFDOC()->m_pSky)
		{
			GOLFDOC()->m_pSky->Clear();
			delete GOLFDOC()->m_pSky;
			GOLFDOC()->m_pSky = NULL;
		}

		GOLFDOC()->m_pSky = new CSky;

		if (GOLFDOC()->m_tutorialMode < 15)
		{
			GOLFDOC()->m_pSky->LoadSky("blue", 0.0f);
			GOLFDOC()->m_pSky->RegisterCloud("blue");
		}
		else
		{
			GOLFDOC()->m_pSky->LoadSky(MakeStr("%s", GetSkyBoxName()), 0.0f);
			GOLFDOC()->m_pSky->RegisterCloud(
				MakeStr("%s_cloud.dds", GetSkyBoxName()));
		}

		GOLFDOC()->m_pPolySoup->LoadMapCheckData(hole);

		if (special)
			CSceneManager::Instance()->SetWeather(
				Doc()->m_golfGame.weather, true);
	}

	DoInitAfterLoad();

	if (CFx::Instance())
		CFx::Instance()->SetActive(true);

	GetActor("GroundItemMan") << MsgObject(this, M983(0x27a), 0, 0, 0, 0, 0);

	// 983 0072f61d: [00e73e60] e o CTaskManager (NULL no nosso exe); GetCurrent = tarefa atual, vtable +0x10 = Init
	if (AfxGetTask())
		AfxGetTask()->Init(NULL);

	if (OnlinePlay())
	{
		WSendPacket send((enumClientPacket)0x11);
		send.Send(TO_GAME);
	}
	else
	{
		SendMsg(this, "GolfRule", M983(0x81), 0, 0, 0, 0);
	}
}

void CAutoTarget::SetList(const std::vector<WVector>& list, const WVector& pos)
{
	m_pointList.reserve(list.size() + 1);
	m_pointList.assign(list.begin(), list.end());
	m_pointList.push_back(pos);

	for (unsigned int i = 0; i < m_pointList.size(); i++)
		m_pointList[i].y = 0;
}

void CGolfBg::DoInitAfterLoad()
{
	GOLFDOC()->m_pPolySoup->MakeDataStructure();

	SetLowestPoint();

	WPolySoup::sPoint* pGreen = NULL;
	if (!(Doc()->m_golfGame.pCourse &&
			Doc()->m_golfGame.pCourse->DiffFlag == 1))
		pGreen = GOLFDOC()->m_pPolySoup->FindPoint("\xb1\xd7\xb8\xae\xb5\xe5"
												   "2");
	if (pGreen == NULL)
		pGreen = GOLFDOC()->m_pPolySoup->FindPoint("\xb1\xd7\xb8\xae\xb5\xe5");

	SetGridCenter(pGreen->pos);
	SetStartPoint(GOLFDOC()->GetHoleData().tee);
	SetHolePoint(GOLFDOC()->GetHoleData().pin);

	SendMsg(this, "GolfBg", M983(0x142), Doc()->m_golfGame.weather, 0, 0, 0);

	if (GOLFDOC()->m_pSky)
	{
		GOLFDOC()->m_pSky->GenerateStars(-1);
		GOLFDOC()->m_pSky->GenerateLensFlare();
		GOLFDOC()->m_pSky->GenerateMoons();
	}

	CShadowManager::Instance()->UpdateLight("sun", g_lightset);

	if (!OnlinePlay())
	{
		float radius = GolfBall().GetRadius();
		WVector tee = GOLFDOC()->GetHoleData().tee;
		GolfBall().Reset(WVector(tee.x, tee.y + radius, tee.z));
	}

	CSceneManager::Instance()->Load();

	GOLFDOC()->m_pAutoTarget->Reset();

	std::vector<CNodeContainer>::const_iterator it;
	for (it = GOLFDOC()->m_pPolySoup->m_node.begin();
		it != GOLFDOC()->m_pPolySoup->m_node.end(); it++)
	{
		if (it->GetType() == CNodeContainer::TYPE_AUTO)
			GOLFDOC()->m_pAutoTarget->SetList(it->GetList(),
				WVector(GOLFDOC()->GetHoleData().pin));
	}

	GOLFDOC()->m_pPolySoup->EndLoad(true);
}

void CGolfBg::OnPreLoadInit()
{
	// 983 0072dfa0: so cria com a instancia ([00e74234], gravada pelo ctor 0096ae90) ainda NULL
	if (!CShadowManager::Instance())
		new CShadowManager(g_resrcmng, COption::Instance()->vIsShadowEnabled());
}

void CGolfBg::OnLoad()
{
	if (CFx::Instance())
		CFx::Instance()->SetActive(false);

	GOLFDOC()->m_pAutoTarget = new CAutoTarget;

	// 983: GetHoleIndex expandido no ramo 0x12; no ramo 0x14 chama a funcao (0046f4f0)
	if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
		SetCurMap(
			Doc()->m_pinIndex[Between<unsigned char>(1,
								  HoleIndexInline983(GOLFDOC()->m_currentHole),
								  18) -
				1]);
	else if (Doc()->m_roomInfo.realGameType == 0x14)
		SetCurMap(
			Doc()->m_pinIndex[Between<unsigned char>(1,
								  GetHoleIndex(GOLFDOC()->m_currentHole), 18) -
				1]);

	CGolfDoc* pGolfDoc = GOLFDOC();
	unsigned char mapHole = DOCJP()->GetHoleValue983(
		HoleIndexInline983(pGolfDoc->m_currentHole));
	if (!pGolfDoc->m_pPolySoup->Load(
			MakeStr("%s_%02d.gbin", GetCourseName(), mapHole, 0), false))
	{
		g_bQuit = true;
		return;
	}

	GOLFDOC()->m_waveInfo.Load(MakeStr("%s_wave.txt", GetCourseName()));
	GOLFDOC()->m_riverInfo.Load(MakeStr("%s_river.txt", GetCourseName()));

	GOLFDOC()->m_pSky = new CSky;

	if (GOLFDOC()->m_tutorialMode < 15)
	{
		GOLFDOC()->m_pSky->LoadSky("blue", 0.0f);
		GOLFDOC()->m_pSky->RegisterCloud("blue");
	}
	else if (Doc()->m_roomInfo.gameType == GAME_TYPE_USEMAX ||
		Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
	{
		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(GetCourseTypeId(Doc()->m_pinIndex[17]));
		if (pCourse)
		{
			char* name = pCourse->Data;
			if (name)
			{
				GOLFDOC()->m_pSky->LoadSky(name, 0.0f);
				GOLFDOC()->m_pSky->RegisterCloud(MakeStr("%s_cloud.dds", name));
			}
		}
	}
	else
	{
		GOLFDOC()->m_pSky->LoadSky(MakeStr("%s", GetSkyBoxName()), 0.0f);
		GOLFDOC()->m_pSky->RegisterCloud(
			MakeStr("%s_cloud.dds", GetSkyBoxName()));
	}

	DoInitAfterLoad();

	if (CFx::Instance())
		CFx::Instance()->SetActive(true);
}

void CGolfBg::OnInit()
{
	CSceneManager::Instance()->UpdateFog(true);
}

const char* __fastcall GetCurMapName();     // 005afae0
unsigned char __fastcall GetHoleIndex(unsigned char hole); // 0046f4f0

void CGolfBg::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x13d):
	{
		int weather = msg.param1;
		if (weather > 1)
			GOLFDOC()->m_pSky->ClearSky();

		GOLFDOC()->m_pSky->GenerateClouds(50, Wind().GetPower983());
		GOLFDOC()->m_pSky->ChangeWeatherTex(weather);
		GOLFDOC()->m_pSky->ShowClouds(weather > 0);
	}
	break;

	case M983(0x142):
		SetRenderWeather((unsigned char)msg.param1);
		// 983 0072f6a0: SetWeather only when the scene manager exists
		if (CSceneManager::Instance())
			CSceneManager::Instance()->SetWeather(GetRenderWeather(),
				msg.param2 != 0);
		Doc()->m_golfGame.weather = (unsigned char)msg.param1;
		break;

	case M983(0x13e):
		GOLFDOC()->m_pSky->LoadSky(MakeStr("%s", GetSkyBoxName()), 0.0f);
		GOLFDOC()->m_pSky->RegisterCloud(
			MakeStr("%s_cloud.dds", GetSkyBoxName()));
		GOLFDOC()->m_pSky->GenerateMoons();
		GOLFDOC()->m_pSky->GenerateStars(-1);
		GOLFDOC()->m_pSky->GenerateLensFlare();
		break;

	case M983(0x13f):
		GOLFDOC()->m_pSky->SetColor(msg.param1);
		break;

	case M983(0x140):
		DoReLoad();
		break;

	case M983(0x118):
		GOLFDOC()->m_pPolySoup->SetRenderMode(msg.param1);
		break;

	case M983(0x141):
		if (GOLFDOC()->m_pPolySoup)
			GOLFDOC()->m_pPolySoup->UpdateBgFxWind();
		break;

	case M983(0x1e5):
	{
		// builds the guide path of the auto-target: start (tee) -> list ->
		// a turn around the last point -> list (backwards) -> start
		const std::vector<WVector>& list =
			GOLFDOC()->m_pAutoTarget->GetPointList();
		std::vector<WVector>* pPath = (std::vector<WVector>*)msg.param1;

		unsigned int capacity = list.size() * 2 + list.size() * 2 + 7;
		if (pPath->capacity() < capacity)
			pPath->reserve(capacity);
		else
			pPath->clear();

		WVector start;
		start = GetTeeShotPos();
		start.y = GetPVS().GetGroundPoint(start, false, NULL, false).y + 100.0f;

		WVector first;
		first = list.front();
		first.y = GetPVS().GetGroundPoint(first, true, NULL, true).y + 100.0f;

		WVector dir, norm;
		norm = dir = start - first;
		WVector back = start + norm.Normalize() * 250.0f;

		pPath->push_back((back + start) * 0.5f);
		pPath->push_back(start);
		pPath->push_back((first + start) * 0.5f);
		pPath->push_back(first);

		int i;
		for (i = 1; i < (int)list.size() - 1; i++)
		{
			WVector pos = (list[i - 1] + list[i]) * 0.5f;
			pos.y = GetPVS().GetGroundPoint(pos, true, NULL, true).y + 100.0f;
			pPath->push_back(pos);

			pos = list[i];
			pos.y =
				GetPVS().GetGroundPoint(list[i], true, NULL, true).y + 100.0f;
			pPath->push_back(pos);
		}

		float sign = (rand() % 2) ? 1.0f : -1.0f;

		WVector side = list.back() - list[i - 1];
		WVector turn = list.back() +
			WCrossProduct(WVector(0.0f, sign, 0.0f), side).Normalize() * 30.0f;
		turn.y = GetPVS().GetGroundPoint(turn, false, NULL, false).y + 100.0f;

		// 983 (0072f6a0): "last" = list[i - 1] (last point pushed by the forward loop)
		WVector last = list[i - 1];
		last.y =
			GetPVS().GetGroundPoint(list[i - 1], true, NULL, true).y + 100.0f;

		WVector out = (turn - last).Normalize() * 250.0f;
		WVector farPos = turn + out;
		WVector cross = turn + WCrossProduct(WVector(0.0f, -sign, 0.0f), out);

		pPath->push_back((last + turn) * 0.5f);
		pPath->push_back(turn);
		pPath->push_back((turn + farPos) * 0.5f);
		pPath->push_back(farPos);
		pPath->push_back((cross + farPos) * 0.5f);
		pPath->push_back(cross);
		pPath->push_back((last + cross) * 0.5f);
		pPath->push_back(last);

		for (i = (int)list.size() - 3; i >= 0; i--)
		{
			WVector pos = (list[i + 1] + list[i]) * 0.5f;
			pos.y = GetPVS().GetGroundPoint(pos, true, NULL, true).y + 100.0f;
			pPath->push_back(pos);

			pos = list[i];
			pos.y =
				GetPVS().GetGroundPoint(list[i], true, NULL, true).y + 100.0f;
			pPath->push_back(pos);
		}

		out = WVector(dir).Normalize() * 250.0f;
		farPos = out + start;
		sign = (rand() % 2) ? 1.0f : -1.0f;
		cross = start + WCrossProduct(WVector(0.0f, sign, 0.0f), out);

		pPath->push_back((cross + first) * 0.5f);
		pPath->push_back(cross);
		pPath->push_back((cross + farPos) * 0.5f);
		pPath->push_back(farPos);
		pPath->push_back((farPos + start) * 0.5f);
	}
	break;

	case M983(0x1e7):
		if (!GOLFDOC()->m_pPolySoup->Load(
				MakeStr("%s_rank.gbin", GetCurMapName()), false))
		{
			g_bQuit = true;
			return;
		}

		GOLFDOC()->m_pPolySoup->EndLoad(false);
		GOLFDOC()->m_pPolySoup->MakeDataStructure();
		SetLowestPoint();
		CShadowManager::Instance()->UpdateLight("sun", g_lightset);
		GOLFDOC()->m_pSky->LoadSky(GetSkyBoxName(), 0.0f);
		GOLFDOC()->m_pSky->ClearClouds();
		GOLFDOC()->m_pSky->GenerateLensFlare();
		GOLFDOC()->m_pSky->GenerateMoons();
		GOLFDOC()->m_pSky->GenerateStars(-1);
		CSceneManager::Instance()->Load();
		CSceneManager::Instance()->UpdateFog(true);
		SendMsg(this, "Effect", M983(0x17b), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
		break;

	case 0x2712:
	{
		char name[256];
		strcpy(name, (const char*)msg.param1);
	}
	break;

	case M983(0x1e8):
	{
		// 983 (007308c3): numero do mapa do buraco = GetHoleValue983(GetHoleIndex(hole)); nome = GetCurMapName()
		unsigned char mapHole =
			DOCJP()->GetHoleValue983(GetHoleIndex(GOLFDOC()->m_currentHole));
		if (!GOLFDOC()->m_pPolySoup->Load(
				MakeStr("%s_%02d.gbin", GetCurMapName(), mapHole, 0), false))
		{
			g_bQuit = true;
			return;
		}

		DoInitAfterLoad();
		if (AfxGetTask())
			AfxGetTask()->Init(NULL);
		SendMsg(this, "GolfRule", M983(0x81), 0, 0, 0, 0);
	}
	return;

	// 983 so: mapa de premiacao (g_award.gbin, ceu "pink")
	case 0x33d:
		if (!GOLFDOC()->m_pPolySoup->Load("g_award.gbin", false))
		{
			g_bQuit = true;
			return;
		}

		GOLFDOC()->m_pPolySoup->EndLoad(false);
		GOLFDOC()->m_pPolySoup->MakeDataStructure();
		SetLowestPoint();
		CShadowManager::Instance()->UpdateLight("sun", g_lightset);
		GOLFDOC()->m_pSky->LoadSky("pink", 0.0f);
		GOLFDOC()->m_pSky->ClearClouds();
		CSceneManager::Instance()->Load();
		CSceneManager::Instance()->UpdateFog(true);
		SendMsg(this, "Effect", M983(0x17b), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "NPC", 0x1fb, 0, 0, 0, 0);
		break;
	}
}


void CGolfBg::SetStartPoint(const WVector& pos)
{
	GOLFDOC()->GetHoleData().tee =
		GetPVS().GetGroundPoint(pos, false, &GolfBall().m_meshIndex, false);
}

void CGolfBg::SetHolePoint(const WVector& pos)
{
	GOLFDOC()->GetHoleData().pin =
		GetPVS().GetGroundPoint(pos, false, NULL, false);
}

void CGolfBg::SetSkyColor(unsigned long color)
{
	if (GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->SetColor(color);
}

void CGolfBg::SetLowestPoint()
{
	SendMsg(this, "Camera", M983(0xef), 0, 0, 0, 0);
}

const char* CGolfBg::GetSkyBoxName()
{
	// 983 (0072e350): so "blue" ou o Data do curso; sem o ramo "Halloween" do 645
	if (GetCourseName() == NULL)
		return "blue";

	return GetCourseName();
}

void CGolfBg::SetRenderWeather(unsigned char weather)
{
	m_renderWeather = weather;

	if (IsLocalContent((localContentType_t)0x96 /*983: era 0x9a; medido 0072e370*/) &&
		Doc()->m_golfGame.gameType < 15) // 983 0072e370 (645: 11)
	{
		if (m_renderWeather <= 0)
			m_renderWeather = 1;

		if (GOLFDOC()->m_pPolySoup)
			GOLFDOC()->m_pPolySoup->ChangePointType(2, 3);
	}
}
