#include "minatl.h"
#include "projectg.h"
#include "commonutil.h"
#include "clientsetting.h"
#include "golfball.h"
#include "quadtree.h"
#include "npc.h"
#include "fx.h"
#include "wbone.h"
#include "wutil.h"
#include <float.h>
#include "slicestr.h"
#include "networksystem.h"
#include "jp/lua983.h"


extern WMatrix g_camera;

const char* GetStrConst(const char* str, char* out);
int Ceil(float value, float epsilon);
int float2int(float value);
WMatrix operator*(const WMatrix& left, float right);
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

// WIP (run 1): header + first group of simple functions only.

// 983 CCrab 0076a020 / CMole 00769db0 read sPlayerData +0x4da = bunker (gauge 0x4c8 + 0x10, unusedWord 0x4d8):
// the creature event only plays while the current player's ball is in the bunker. Member, not raw offset (cl 14.44).
#define InBunker(player) ((player)->bunker)

// Index of the segment whose upper knot is the first one >= t.
static __forceinline int CatmullFindSegment(const float* knots, int count,
	float t)
{
	int i = 0;
	while (i < count - 1 && !(t <= knots[i + 1]))
	{
		++i;
	}
	return i;
}

// WView::InFrustumSafe(const Waabb&) is not declared in wview.h (a new name
// there moves the PCH canaries): reach it through a helper class.
class WViewEx : public WView
{
public:
	bool InFrustumSafe(const Waabb& box);
};
#pragma comment(linker, \
	"/alternatename:?InFrustumSafe@WViewEx@@QAE_NABVWaabb@@@Z=?InFrustumSafe@WView@@QAE_NABVWaabb@@@Z")

IMPLEMENT_ACTOR(CNPC, IActor)

void CNPC::OnDestroy()
{
	Destroy();
}

// 983 0076a680: apaga tambem o script da torcida do Grand Prix (+0x24)
void CNPC::Destroy()
{
	if (m_pCrowdScript983)
	{
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(m_pCrowdScript983);
		m_pCrowdScript983 = NULL;
	}
	std::list<CCreature*>::iterator it;
	for (it = m_creatures.begin(); it != m_creatures.end(); it++)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_creatures.clear();
}

void CNPC::OnInit()
{
	Destroy();
	// 983 00775a90: script da torcida do Grand Prix (tabelas caddie_pet_name / mascot_pet_name)
	m_pCrowdScript983 = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (m_pCrowdScript983)
		m_pCrowdScript983->RunFile("grandprix_crowd.lua");
	for (int i = 0; i < Doc()->m_pGolfDoc->m_pPolySoup->GetAreaNum(); i++)
	{
		WPolySoup::sArea* area = Doc()->m_pGolfDoc->m_pPolySoup->GetArea(i);
		if (area->type == 1)
			SetNPC(area->aabb, area->name);
	}
	for (std::vector<CNodeContainer>::iterator it =
			 Doc()->m_pGolfDoc->m_pPolySoup->m_node.begin();
		it != Doc()->m_pGolfDoc->m_pPolySoup->m_node.end(); ++it)
	{
		if (it->GetType() == (CNodeContainer::eType)0xC)
			SetNPC(it->GetList(), it->GetName().c_str());
	}
}

void CNPC::OnProcess(float delta)
{
	std::list<CCreature*>::iterator it;
	for (it = m_creatures.begin(); it != m_creatures.end(); it++)
	{
		(*it)->Process(delta);
	}
}

void CNPC::HandleMsg(const MsgObject& msg)
{
	std::list<CCreature*>::iterator it;
	switch (msg.message)
	{
	case M983(0x188): // is there a creature of type param1? answered through param2
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			if ((*it)->GetType() == msg.param1)
			{
				*(int*)msg.param2 = 1;
				break;
			}
		}
		break;
	case M983(0x182): // switch off the active event creatures
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			CCreature* creature = *it;
			if ((creature->m_flags & 1) && creature->GetType() != 9)
			{
				if (creature->GetActive())
				{
					creature->m_bActive = false;
				}
			}
		}
		break;
	case M983(0x183): // play the event of the creature whose area holds the ball
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			CCreature* creature = *it;
			if (creature->m_flags & 1)
			{
				Waabb area = creature->GetArea();
				if (area.IsInclude(GolfBall().m_pos))
				{
					creature->PlayEvent();
					break;
				}
			}
		}
		break;
	case M983(0x184): // *param1 = 0 when the ball is inside the area of an event creature
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			CCreature* creature = *it;
			if (creature->m_flags & 1)
			{
				Waabb area = creature->GetArea();
				if (area.IsInclude(GolfBall().m_pos))
				{
					*(int*)msg.param1 = 0;
					break;
				}
			}
		}
		break;
	case M983(0x186):
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			if (!((*it)->m_flags & 1))
			{
				(*it)->Record();
			}
		}
		break;
	case M983(0x185):
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			(*it)->Replay();
		}
		break;
	case M983(0x18a):
		for (it = m_creatures.begin(); it != m_creatures.end(); it++)
		{
			if ((*it)->GetType() == 9)
			{
				(*it)->PlayEvent();
			}
		}
		break;
	case 0x1fb: // 983 00773037: palco da premiacao do Grand Prix (CGolfBg 00730bb9) -> torcida nas areas tipo 1
		for (int i = 0; i < Doc()->m_pGolfDoc->m_pPolySoup->GetAreaNum(); i++)
		{
			WPolySoup::sArea* area = Doc()->m_pGolfDoc->m_pPolySoup->GetArea(i);
			if (area->type == 1)
				SetGPCrowd983(area->aabb, area->name);
		}
		break;
	}
}

// 983 00770f30 (vtable 00d22920): caddie da torcida, virado para a origem do palco, laco do movimento "\xc8\xa6\xc0\xce"
class CGPAward_Caddie983 : public CCreature
{
public:
	CGPAward_Caddie983(const char* name, const Waabb& area)
	{
		m_addTex = 0;
		m_pPet = g_resrcmng->GetPuppet(name, false, true, false);
		if (!m_pPet)
			return;
		CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, m_pPet, this, name, 0, NULL);
		m_angle = 0.0f;
		m_f80 = 0.0f;
		m_bRecorded = false;
		m_alpha = 0xff;
		m_recIndex = 0;
		SetMotion("\xc8\xa6\xc0\xce", -1.0f);
		m_f70 = 0.0f;
		m_bActive = true;
		InitFx();
		SetArea(area, 0);
		m_target = WVector(0.0f, 0.0f, 0.0f);
		WVector d = m_target - m_mat.pivot;
		d.y = 0.0f;
		float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
			m_mat = RotMat(angle, 1) * m_mat;
	}
	virtual void Process(float delta) // 983 0076bfb0
	{
		if (m_pPet && m_bActive)
		{
			float time = delta + m_motionTime;
			m_motionTime = time;
			if (!(time < 0.0f))
			{
				ScriptProcess(m_pMotion, time - delta, time);
				if (m_motionTime > m_pMotion->e)
				{
					m_motionTime -= m_pMotion->e;
					SetMotion("\xc8\xa6\xc0\xce", 0.0f);
				}
			}
		}
	}
};

// 983 00771100 (vtable 00d22948): mascote da torcida, laco do "Default"
class CGPAward_Mascot983 : public CCreature
{
public:
	CGPAward_Mascot983(const char* name) { Init(name, 0); }
	virtual void Process(float delta) // 983 0076c040
	{
		if (m_pPet && m_bActive)
		{
			float time = delta + m_motionTime;
			m_motionTime = time;
			if (!(time < 0.0f))
			{
				ScriptProcess(m_pMotion, time - delta, time);
				if (m_motionTime > m_pMotion->e)
				{
					m_motionTime -= m_pMotion->e;
					SetMotion("Default", 0.0f);
				}
			}
		}
	}
};

// 983 00769a80: nome da tabela do grandprix_crowd.lua pelo indice (1..get_array_size)
static const char* GetGPCrowdPetName983(lua_system::lua_script* script, const char* table, int index)
{
	if (index == 0 || !script)
		return NULL;
	lua_system::lua_table_helper tbl = script->GetTable(table);
	int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
	if (index > count)
		return NULL;
	return ((LuaChild983&)tbl).GetInl983<const char*>(index);
}

// 983 00772630: "*type 36" caddie, "*type 37" mascote (nome = *pet-esimo da tabela), "*type 38" = msg 0x344 ao DummyPlayer
void CNPC::SetGPCrowd983(const Waabb& area, const char* option)
{
	WSliceStr slice((char*)option);
	int type;
	int index = 0;
	slice.Get("*type", "%d", &type);
	slice.Get("*pet", "%d", &index);
	if (type == 0x24)
	{
		const char* p = GetGPCrowdPetName983(m_pCrowdScript983, "caddie_pet_name", index);
		std::string name = p ? p : "";
		if (!name.empty())
		{
			CCreature* creature = new CGPAward_Caddie983(name.c_str(), area);
			creature->SetType(type);
			creature->SetMotion("\xc8\xa6\xc0\xce", -1.0f);
			m_creatures.push_back(creature);
		}
	}
	else if (type == 0x25)
	{
		const char* p = GetGPCrowdPetName983(m_pCrowdScript983, "mascot_pet_name", index);
		std::string name = p ? p : "";
		if (!name.empty())
		{
			CCreature* creature = new CGPAward_Mascot983(name.c_str());
			creature->SetArea(area, 0);
			creature->SetType(type);
			creature->SetMotion("Default", -1.0f);
			m_creatures.push_back(creature);
		}
	}
	else if (type == 0x26)
	{
		IActor* dummy = AfxGetTask()->GetActor("DummyPlayer");
		if (dummy)
			dummy << MsgObject(NULL, 0x344, index, (int)&area, 0, 0, 0);
	}
}

void CNPC::SetNPC(const std::vector<WVector>& points, const char* name)
{
	if (strstr(name, "santa"))
	{
		if (NET()->IsConnected(WNetworkSystem::NET_GAME))
		{
			CSharedDoc* shared = Doc();
			if (shared->m_golfGame.gameType != 0 || shared->m_gameMode != 0)
				return;
			// m_holeRandom is indexed by the 1-based hole index here
			if (shared->m_holeRandom[GetHoleIndex(
										 Doc()->m_pGolfDoc->m_currentHole) -
					1] %
					100 >=
				3)
				return;
		}
		WSliceStr slice(name);
		float radius;
		if (!slice.Get("*r", "%f", &radius))
			radius = 32.0f;
		int i;
		for (i = 0; i < 3; i++)
		{
			CSanta* santa = new CSanta("NPC_Mozzi.pet");
			santa->SetRadius(radius);
			santa->SetArea(points, (float)i * -0.05f, 0);
			santa->SetType(7);
			m_creatures.push_back(santa);
		}
		CSanta* santa = new CSanta("NPC_Santa.pet");
		santa->SetRadius(radius);
		santa->SetArea(points, (float)(i + 1) * -0.05f, 0);
		santa->SetType(7);
		m_creatures.push_back(santa);
		return;
	}
	CCreature* creature;
	WSliceStr slice(name);
	float value;
	if (strstr(name, "flight"))
	{
		creature = new CFighter("NPC_Fighter.pet");
		creature->SetArea(points, 0, 0);
		creature->SetType(8);
	}
	else if (strstr(name, "ufo"))
	{
		creature = new CUfo("sand_house12.pet");
		creature->SetArea(points, 0, 0);
		creature->SetType(0xB);
	}
	else if (strstr(name, "penguinwalk"))
	{
		creature = new ntPenguin("NPC_penguin02.pet");
		creature->SetArea(points, 0, 0);
		creature->SetType(0xC);
	}
	else if (strstr(name, "drg"))
	{
		if (!slice.Get("*d", "%f", &value))
			value = 0;
		creature = new ntNpc_Dragon("NPC_dragon.pet", value);
		creature->SetArea(points, 0, 0);
		creature->SetType(0xF);
	}
	else if (strstr(name, "ship"))
	{
		if (!slice.Get("*d", "%f", &value))
			value = 0;
		creature = new ntNpc_PolaShip("NPC_ship.pet", value);
		creature->SetArea(points, 0, 0);
		creature->SetType(0x11);
	}
	else if (strstr(name, "vessel"))
	{
		char petName[64] = "";
		int index = 2;
		value = 0;
		if (strstr(name, "vessel4"))
			index = 4;
		if (!slice.Get("*t", "%f", &value))
			value = 1.0f;
		sprintf(petName, "gs_ship_0%d.pet", index);
		creature = new CNPC_TRSHIP(petName, value);
		creature->SetArea(points, 0, 0);
		creature->SetType(0x14);
	}
	else if (strstr(name, "walex"))
	{
		char petName[64] = "";
		int index = 1;
		value = 0;
		if (strstr(name, "walex03"))
			index = 3;
		if (!slice.Get("*vel", "%f", &value))
			value = 1.0f;
		sprintf(petName, "npc_alex0%d.pet", index);
		creature = new CNPC_TORTOISE(petName, value);
		creature->SetArea(points, 0, 0);
		creature->SetType(0x17);
	}
	else if (strstr(name, "herald"))
	{
		if (!slice.Get("*vel", "%f", &value))
			value = 1.0f;
		if (strstr(name, "herald01"))
			creature = new CNPC_TORTOISE("NPC_Herald01.pet", value);
		else
			creature = new CNPC_TORTOISE("NPC_Herald03.pet", value);
		creature->SetArea(points, 0, 0);
		creature->SetType(0x17);
	}
	else if (strstr(name, "npc_sbir"))
	{
		// 983 00771720: skeletal bird on the TRSHIP path
		value = 0;
		if (!slice.Get("*d", "%f", &value))
			value = 1.0f;
		CNPC_TRSHIP* ship = new CNPC_TRSHIP("skeletal_bird.pet", value);
		creature = ship;
		creature->SetArea(points, 0, 0);
		creature->SetType(0x14);
		ship->SetMotionName("NPC_Default");
	}
	else if (strstr(name, "zdair"))
	{
		// 983: airship keeps the CCreature type (no SetType)
		value = 0;
		if (!slice.Get("*d", "%f", &value))
			value = 1.0f;
		CNPC_TRSHIP* ship = new CNPC_TRSHIP("mland_airship.pet", value);
		creature = ship;
		creature->SetArea(points, 0, 0);
		ship->SetMotionName("NPC_Default");
	}
	else if (strstr(name, "crn") || strstr(name, "gns") ||
		strstr(name, "pnd") || strstr(name, "toy") || strstr(name, "stn") ||
		strstr(name, "msn") || strstr(name, "sol"))
	{
		char petName[256] = "";
		char prefix[32] = "";
		bool tortoise = false;
		value = 0;
		if (!slice.Get("*v", "%f", &value))
			value = 1.0f;
		if (strstr(name, "#"))
			tortoise = true;
		if (strstr(name, "crn"))
			strcpy(prefix, "NPC_crane");
		else if (strstr(name, "gns"))
			strcpy(prefix, "NPC_ginseng");
		else if (strstr(name, "pnd"))
			strcpy(prefix, "NPC_panda");
		else if (strstr(name, "toy"))
			strcpy(prefix, "NPC_toy");
		else if (strstr(name, "stn"))
			strcpy(prefix, "NPC_stone");
		else if (strstr(name, "msn"))
			strcpy(prefix, "NPC_messenger");
		else if (strstr(name, "sol"))
			strcpy(prefix, "NPC_soldier");
		std::string full(name);
		std::string suffix("");
		std::string::size_type pos = full.find(strstr(name, "*") ? '*' : '#');
		if (pos == 0)
			pos = full.length();
		suffix = full.substr(7, pos - 7);
		sprintf(petName, "%s%s.pet", prefix, suffix.c_str());
		if (tortoise)
		{
			creature = new CNPC_TORTOISE(petName, value);
			creature->SetArea(points, 0, 0);
			creature->SetType(0x17);
		}
		else
		{
			creature = new CNPC_TRSHIP(petName, value);
			creature->SetArea(points, 0, 0);
			creature->SetType(0x14);
		}
	}
	else if (strstr(name, "npc_t"))
	{
		const char* petName = "";
		value = 1.0f;
		slice.Get("*v", "%f", &value);
		if (strstr(name, "npc_t0"))
			petName = "npc_insam_ani_N.pet";
		else if (strstr(name, "npc_t1"))
			petName = "npc_cat_ani02_N.pet";
		ntNpc_WizCityNode* node = new ntNpc_WizCityNode(petName, value);
		value = 1.0f;
		slice.Get("*s", "%f", &value);
		node->SetScale(value);
		creature = node;
		creature->SetArea(points, 0, 0);
		creature->SetType(0x20);
	}
	else
	{
		const char* suffix = strstr(name, "npc_");
		if (!suffix)
			return;
		creature = new CDisposable(MakeStr("%s.pet", suffix + 4));
		creature->SetArea(points, 0, 0);
		creature->SetType(9);
	}
	m_creatures.push_back(creature);
}

#define KEY_SCALE "*s"
#define KEY_VEL "*vel"

void CNPC::SetNPC(const Waabb& area, char* name)
{
	WSliceStr slice(name);
	int type;
	char petName[32];
	CCreature* creature = NULL;
	unsigned long flags = 0;
	int num;
	slice.Get("*type", "%d", &type);
	slice.Get("*pet", "%s", petName);
	switch (type)
	{
	case 0:
		if (slice.Get("*num", "%d", &num))
		{
			for (int i = 0; i < num; i++)
			{
				creature = new CGull(petName);
				creature->SetArea(area, 0);
				creature->SetType(type);
				m_creatures.push_back(creature);
			}
			return;
		}
		else
		{
			creature = new CGull(petName);
		}
		break;
	case 1:
		if (slice.Get("*num", "%d", &num))
		{
			for (int i = 0; i < num; i++)
			{
				creature = new CButterfly(petName);
				creature->SetArea(area, 4);
				creature->SetType(type);
				m_creatures.push_back(creature);
			}
			return;
		}
		else
		{
			creature = new CButterfly(petName);
			flags = 4;
		}
		break;
	case 2:
		if (slice.Get("*num", "%d", &num))
		{
			creature = new CDuck(petName, num + 1);
		}
		else
		{
			creature = new CDuck(petName, 0);
		}
		flags = 1;
		break;
	case 3:
		creature = new CMole(petName);
		break;
	case 4:
		creature = new CDolphin(petName);
		break;
	case 5:
		creature = new CCrab(petName);
		break;
	case 6:
		// the original dereferences a null creature without "*num"
		if (slice.Get("*num", "%d", &num))
			creature = new CSparrow(petName, num);
		break;
	case 7:
	case 10:
		// the original falls into the common tail with a null creature
		break;
	case 8:
		creature = new CFighter(petName);
		break;
	case 9:
		creature = new CDisposable(petName);
		break;
	case 13:
		if (slice.Get("*num", "%d", &num))
		{
			for (int i = 0; i < num; i++)
			{
				creature = new ntPenguin_Sailor(petName);
				creature->SetArea(area, 0);
				creature->SetType(type);
				m_creatures.push_back(creature);
			}
			return;
		}
		else
		{
			creature = new ntPenguin_Sailor(petName);
		}
		break;
	case 14:
		if (slice.Get("*num", "%d", &num))
		{
			creature = new ntPenguin_Duck(petName, num + 1);
		}
		else
		{
			creature = new ntPenguin_Duck(petName, 0);
		}
		flags = 1;
		break;
	case 16:
		if (slice.Get("*num", "%d", &num))
		{
			for (int i = 0; i < num; i++)
			{
				creature = new ntNpc_BlackSparrow("NPC_BTsparrow.pet");
				creature->SetArea(area, 0);
				creature->SetType(type);
				m_creatures.push_back(creature);
			}
			return;
		}
		else
		{
			creature = new ntNpc_BlackSparrow("NPC_BTsparrow.pet");
		}
		break;
	case 18:
	{
		float vel;
		vel = 2.0f;
		slice.Get(KEY_VEL, "%f", &vel);
		num = 1;
		slice.Get("*num", "%d", &num);
		for (int i = 0; i < num; i++)
		{
			creature = new ntNpc_PolaPenguin1(petName, vel);
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	}
	case 19:
		num = 1;
		slice.Get("*num", "%d", &num);
		for (int i = 0; i < num; i++)
		{
			creature = new ntNpc_PolaPenguin2(petName);
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	case 31:
	{
		float scale;
		num = 1;
		slice.Get("*num", "%d", &num);
		scale = 1.0f;
		slice.Get(KEY_SCALE, "%f", &scale);
		for (int i = 0; i < num; i++)
		{
			ntNpc_WizCityArea* walker = new ntNpc_WizCityArea(petName);
			walker->SetScale(scale);
			creature = walker;
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	}
	case 21:
	{
		// the original does not give vel a default here
		float vel;
		num = 1;
		slice.Get("*num", "%d", &num);
		slice.Get(KEY_VEL, "%f", &vel);
		for (int i = 0; i < num; i++)
		{
			creature = new CNPC_BIRD(petName, vel);
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	}
	case 0x1c:
	case 0x1e:
	{
		// 983 00774ea0: ntNpc_DLCreature, vel padrao 2.0
		float vel = 2.0f;
		slice.Get(KEY_VEL, "%f", &vel);
		num = 1;
		slice.Get("*num", "%d", &num);
		for (int i = 0; i < num; i++)
		{
			creature = new ntNpc_DLCreature(petName, vel);
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	}
	case 22:
	case 0x1d: // 983 00774ea0: same parrot path as 0x16
		if (!slice.Get("num", "%d", &num))
			num = 1;
		for (int i = 0; i < num; i++)
		{
			creature = new CNPC_PARROT(petName);
			creature->SetArea(area, 0);
			creature->SetType(type);
			m_creatures.push_back(creature);
		}
		return;
	default:
		return;
	}
	creature->SetArea(area, flags);
	creature->SetType(type);
	m_creatures.push_back(creature);
}

CCreature::~CCreature()
{
	if (g_resrcmng)
	{
		if (m_addTex)
		{
			g_resrcmng->Release(m_addTex);
			m_addTex = 0;
		}
		if (g_resrcmng && m_pPet)
		{
			g_resrcmng->Release(m_pPet, false);
			m_pPet = NULL;
		}
	}
	if (m_pCells)
	{
		delete[] m_pCells;
		m_pCells = NULL;
	}
	m_path.std::vector<WVector>::~vector();
	m_recPath.std::vector<WVector>::~vector();
	m_alphaList.clear();
}

void CCreature::Init(const char* name, unsigned long layer)
{
	m_addTex = 0;
	m_pPet = g_resrcmng->GetPuppet(name, false, true, false);
	if (!m_pPet)
		return;
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, m_pPet, this, name,
		layer, NULL);
	m_angle = 0.0f;
	m_bRecorded = false;
	m_alpha = 0xff;
	m_recIndex = 0;
	m_f80 = 0.7853982f;
	SetMotion("NPC_Default", -1.0f);
	if (m_flags & 1)
	{
		m_bActive = false;
		m_alphaList.clear();
		sAlpha alpha;
		int from;
		int to;
		if (m_pPet->CheckFrameData((int)floor(m_pMotion->s * 30.0f),
				(int)ceil(m_pMotion->e * 30.0f), from, to))
		{
			for (int i = from; i <= to; ++i)
			{
				if (sscanf(m_pPet->m_frame[i].data, "*opa %f", &alpha.alpha))
				{
					alpha.alpha *= 2.55f;
					alpha.time = m_pPet->GetFrameDataTime(i);
					m_alphaList.push_back(alpha);
				}
			}
		}
	}
	else
	{
		m_bActive = true;
		m_f70 = 1.0471976f;
	}

	InitFx();
}

// 983: laco das caixas "*fx" do puppet separado de Init (00766970)
void CCreature::InitFx()
{
	for (w_bound_box* box = m_pPet->m_bbList.Start(); box;
		box = m_pPet->m_bbList.Next())
	{
		if (!box->info->option)
			continue;
		const char* p = strstr(box->info->option, "*fx");
		if (!p)
			continue;
		char cmd[32] = "\0";
		char fx[32] = "\0";
		char bone[32] = "\0";
		sscanf(p, "%s %s %s", cmd, fx, bone);
		if (!box->bone || box->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;
		char* ext = strrchr(fx, '.');
		if (!ext || strlen(ext) < 4)
			continue;
		bool disposable = stricmp(bone, "disposable") == 0;
		++ext;
		if (stricmp(ext, "spr") == 0 || stricmp(ext, "seq") == 0)
		{
			Attacher attacher;
			attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
			WSingleton<CFx>::Instance()->Attach(attacher, fx, disposable, NULL);
		}
	}
}

void CCreature::SetArea(const Waabb& area, unsigned long flags)
{
	m_area = area;
	if (m_flags & 1)
		return;
	m_mat = RotMat(Random(-180.0f, 180.0f) * g_DEGTORAD, 1);
	float* pivot = &m_mat.pivot.x;
	const float* lo = &m_area.min.x;
	const float* hi = &m_area.max.x;
	for (int i = 0; i < 3; i++)
	{
		float r = Random(0, 1.0f);
		pivot[i] = (1.0f - r) * hi[i] + r * lo[i];
	}
	if (flags & 1)
	{
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, false, NULL, true);
	}
	if (flags & 2)
	{
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, false, NULL, false);
	}
	m_target = m_mat.pivot;
	float cellsX = (float)ceil((m_area.max.x - m_area.min.x) / m_cellSize);
	float cellsY;
	if (flags & 4)
		cellsY = (float)ceil((m_area.max.y - m_area.min.y) / m_cellSize);
	else
		cellsY = 1.0f;
	float cellsZ = (float)ceil((m_area.max.z - m_area.min.z) / m_cellSize);
	m_gridX = (int)cellsX;
	m_gridY = (int)cellsY;
	m_gridZ = (int)cellsZ;
	m_cellCount = m_gridY > 1 ? 4 : 2;
	m_cellX = (m_area.max.x - m_area.min.x) / cellsX;
	m_cellY = (m_area.max.y - m_area.min.y) / cellsY;
	m_cellZ = (m_area.max.z - m_area.min.z) / cellsZ;
	m_pCells = new int[m_cellCount];
	int index = ((int)((m_target.y - m_area.min.y) / m_cellY) * m_gridZ +
					(int)((m_target.z - m_area.min.z) / m_cellZ)) *
			m_gridX +
		(int)((m_target.x - m_area.min.x) / m_cellX);
	for (int i = 0; i < m_cellCount; i++)
	{
		m_pCells[i] = index;
	}
}

void CCreature::SetArea(const std::vector<WVector>& points, float radius,
	unsigned long flags)
{
	m_path.clear();
	m_path.reserve(points.size());
	m_path.assign(points.begin(), points.end());
	m_mat.za = (m_path[1] - m_path[0]).Normalize() * -1.0f;
	m_mat.ya = WVector::UNIT_POS_Y;
	m_mat.xa = WCrossProduct(m_mat.ya, m_mat.za);
	m_mat.pivot = points[0];
	m_target = m_path[1];
	m_motionTime = radius;
}

void CCreature::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = m_pMotion->s;
				}
				else
				{
					m_motionTime -= m_pMotion->e;
				}
			}
			if (m_flags & 1)
			{
				std::list<sAlpha>::iterator it = m_alphaList.begin();
				std::list<sAlpha>::iterator end = m_alphaList.end();
				while (it != m_alphaList.end())
				{
					std::list<sAlpha>::iterator cur;
					cur = it;
					++it;
					if (it == end)
					{
						m_alpha = (unsigned char)(*cur).alpha;
					}
					else if (m_motionTime > (*cur).time &&
						m_motionTime < (*it).time)
					{
						float t = (m_motionTime - (*cur).time) /
							((*it).time - (*cur).time);
						m_alpha = (unsigned char)((1.0f - t) * (*cur).alpha +
							t * (*it).alpha);
						break;
					}
				}
			}
			else
			{
				CheckTarget(delta);
			}
			if (m_bPetBillboard983) // 983 0076b05b
				BillboardToCamera983();
		}
	}
}

// 983 00764230: com "*petbillboard" no script, gira o modelo em y para o lado da camera
void CCreature::BillboardToCamera983()
{
	WVector dir((float)(m_mat.xz * -1.0), (float)(m_mat.yz * -1.0), (float)(m_mat.zz * -1.0));
	WVector delta(g_camera.xm - m_mat.xm, g_camera.ym - m_mat.ym, g_camera.zm - m_mat.zm);
	WMatrix rot;
	RotMat(CalcDeltaAngle(delta, dir), 1, rot);
	m_mat = rot * m_mat;
}

void CCreature::Display()
{
	if ((COption::Instance()->vIsNpcEnabled() ||
			GOLFDOC()->m_currentHole == 19) &&
		m_pPet && m_bActive && !(m_motionTime < 0.0f))
	{
		if (m_flags & 1)
			m_pPet->SetAlpha(m_alpha);
		WMatrix* matrix =
			Abs(m_angle) > 0.0f ? &(RotMat(m_angle, 2) * m_mat) : &m_mat;
		m_pPet->SetMatrix(*matrix);
		m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
		m_pPet->Transform(g_view, 1.0f, 0);
		m_pPet->UpdateBSphere();
		if (g_view->InFrustum(m_pPet->GetBoundSphere()))
		{
			m_pPet->UpdateLightSource(&g_lightset, false, NULL);
			WPuppet* puppet = m_pPet;
			WView* view = g_view;
			puppet->Render(view, true, 1.0f,
				((WViewEx*)view)->InFrustumSafe(puppet->m_bound), false, false);
		}
	}
}

void CCreature::DrawDebugLine()
{
	g_view->DrawSphere(m_pPet->GetBoundSphere(), 0xff00ff00, 20, 0);
	if (!(m_flags & 1))
	{
		WVector* pos = &m_mat.pivot;
		WVector* dir = &m_mat.za;
		WVector off(dir->x * 5.0f, dir->y * 5.0f, dir->z * 5.0f);
		WVector end(pos->x - off.x, pos->y - off.y, pos->z - off.z);
		g_view->DrawLine(*pos, 0xffffff00, end, 0xffffff00, 0);
		g_view->DrawLine(*pos, 0xffff0000, m_target, 0xffff0000, 0);
	}
}

void CCreature::SetMotion(const char* name, float time)
{
	if (0)
		((CCreature*)0)->CCreature::DrawDebugLine();
	if (m_pPet)
	{
		w_motion_data* motion = m_pPet->FindMotionData(name);
		if (motion)
		{
			ScriptProcess(motion, motion->s, motion->s, true);
			m_pMotion = motion;
			if (time >= 0.0f)
			{
				float e = motion->e;
				time += motion->s;
				m_motionTime = time > e ? e : time;
			}
			else
			{
				m_motionTime = Random(motion->s, motion->e);
			}
		}
		else
		{
			m_motionTime = 0.0f;
		}
	}
}

void CCreature::ScriptProcess(const w_motion_data* motion, float from, float to,
	bool single)
{
	// 983 0076a720: single (SetMotion) roda so o quadro de 'from'
	if (motion && ((from <= motion->e && from != to) || single))
	{
		float end = to;
		int first;
		int last;
		if (single)
		{
			first = Ceil(from, 0.03f);
			last = first;
		}
		else
		{
			from *= 30.0f;
			to = Min(motion->e, to) * 30.0f;
			first = Ceil(from, 0.03f);
			if (to == (float)floor(to))
				last = (int)to - 1;
			else
				last = (int)(float)floor(to);
			if (last < first)
				goto loop;
		}
		if (m_pPet->CheckFrameData(first, last, (int&)from, (int&)to) &&
			(int&)from <= (int&)to)
		{
			for (int i = (int&)from; i <= (int&)to; ++i)
				ParsePuppetScript(m_pPet->m_frame[i].data);
		}
loop:
		if (end > motion->e)
		{
			from = motion->s * 30.0f;
			to = Min(motion->e, end - motion->e) * 30.0f;
			first = Ceil(from, 0.03f);
			if (to == (float)floor(to))
				last = (int)to - 1;
			else
				last = (int)(float)floor(to);
			if (first <= last &&
				m_pPet->CheckFrameData(first, last, (int&)from, (int&)to) &&
				(int&)from <= (int&)to)
			{
				for (int i = (int&)from; i <= (int&)to; ++i)
					ParsePuppetScript(m_pPet->m_frame[i].data);
			}
		}
	}
}

void CCreature::ParsePuppetScript(const char* script)
{
	static char cmd[1024];

	char name[64];
	char fx[64];
	char bone[64];

	if (!script)
		return;
	do
	{
		const char* p = strchr(script, '*');
		if (!p)
			return;
		int count = 0;
		if (!*p)
			return;
		char c;
		do
		{
			c = p[++count];
			if (c == ' ')
				break;
			if (c == '(')
			{
				count++;
				break;
			}
		} while (c != '\0');
		if (count <= 1)
			return;
		strncpy(cmd, p, count);
		cmd[count] = 0;
		script = p + count;
		if (_strcmpi(cmd, "*fx(") != 0)
		{
			if (_strcmpi(cmd, "*petbillboard") == 0)
				m_bPetBillboard983 = true; // 983 00769d83
			continue;
		}

		name[0] = '\0';
		script = GetStrConst(script, name);
		if (strlen(name) == 0)
			return;
		if (strlen(name) >= 0x40)
			return;
		if (name[0] == '@')
		{
			if (!m_pMotion)
				return;
			if (strcmp(name + 1, m_pMotion->name) != 0)
				return;
			script = GetStrConst(script, fx);
			if (strlen(fx) == 0)
				return;
			if (strlen(fx) >= 0x40)
				return;
		}
		else
		{
			strcpy(fx, name);
		}

		Attacher attacher;
		unsigned int j = 0;
		do
		{
			script = GetStrConst(script, bone);
			if (strlen(bone) == 0)
				break;
			if (strlen(bone) >= 0x40)
				break;
			WBone* pBone = m_pPet->FindBone(bone);
			if (pBone)
			{
				attacher.mat[j] = &pBone->m_matrix;
			}
			else
			{
				w_bound_box* pBox = m_pPet->m_bbList.Find(bone);
				if (pBox)
				{
					if (pBox->bone)
						attacher.mat[j] = &pBox->bone->m_matrix;
					attacher.pos[j] = &pBox->info->spherePivot;
				}
			}
			++j;
		} while (j < 4);
		if (0 < (int)j)
			WSingleton<CFx>::Instance()->Attach(attacher, fx, true, NULL);
	} while (script);
}

void CCreature::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			int tries = 0;
			int nx, ny, nz;
			int index;
			int i;
			do
			{
				tries++;
				int rnd[3];
				for (i = 0; i < 3; i++)
				{
					rnd[i] = rand() % 3 - 1;
				}
				int cur = m_pCells[0];
				int x = cur % m_gridX;
				int y = cur / (m_gridZ * m_gridX);
				int z = cur % (m_gridZ * m_gridX) / m_gridX;
				nx = Between(0, rnd[0] + x, m_gridX - 1);
				ny = Between(0, rnd[1] + y, m_gridY - 1);
				nz = Between(0, rnd[2] + z, m_gridZ - 1);
				index = (m_gridZ * ny + nz) * m_gridX + nx;
				for (i = 0; i < m_cellCount; i++)
				{
					if (m_pCells[i] == index)
					{
						break;
					}
				}
			} while (i < m_cellCount && tries < 5);
			for (i = m_cellCount - 1; i >= 1; i--)
			{
				m_pCells[i] = m_pCells[i - 1];
			}
			m_pCells[0] = index;
			m_target.x = Random(0.0f, m_cellX) + nx * m_cellX + m_area.min.x;
			if (m_gridY > 1)
			{
				m_target.y =
					Random(0.0f, m_cellY) + ny * m_cellY + m_area.min.y;
			}
			m_target.z = Random(0.0f, m_cellZ) + nz * m_cellZ + m_area.min.z;
			if (m_bRecorded)
			{
				m_recPath.push_back(m_target);
			}
		}
		else
		{
			m_target = m_recPath[m_recIndex];
			m_recIndex++;
		}
	}
	if (m_gridY > 1)
	{
		m_mat.pivot += (m_target - m_mat.pivot).Normalize() * m_speed * delta;
	}
	else
	{
		float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
		{
			if (Abs(angle) > m_f80)
			{
				if (Abs(m_angle) < m_f7c)
				{
					m_angle -= angle * delta;
				}
			}
			else if (Abs(m_angle) > Abs(angle * delta))
			{
				m_angle += (angle > 0.0f ? m_f80 : -m_f80) * delta;
			}
			else
			{
				m_angle = 0.0f;
			}
			float limit = delta * m_f70;
			angle = Between(-limit, angle, limit);
			m_mat = RotMat(angle, 1) * m_mat;
		}
		else if (Abs(m_angle) > 0.0f)
		{
			if (Abs(m_angle) > delta * m_f80)
			{
				m_angle += (m_angle > 0.0f ? -m_f80 : m_f80) * delta;
			}
			else
			{
				m_angle = 0.0f;
			}
		}
		m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	}
}

void CCreature::PlayEvent()
{
	m_motionTime = 0.0f;
	m_bActive = true;
	m_alpha = 0xff;
	m_mat = RotMat(Random(-g_PI, g_PI), 1);
	WVector offset = m_radius * m_mat.za;
	m_mat.pivot =
		GetPVS().GetGroundPoint(offset + GolfBall().m_pos, false, NULL, true);
}

void CCreature::Record()
{
	m_recPath.clear(); // 983 0076d510: end = begin (o 645 destruia o vetor)
	m_recPath.reserve(10);
	m_recPath.push_back(m_target);
	m_recTime = m_motionTime;
	m_bRecorded = true;
	m_recMatrix = m_mat;
	m_recAngle = m_angle;
	m_recActive = m_bActive;
}

void CCreature::Replay()
{
	if (m_flags & 1)
	{
		m_bActive = false;
		return;
	}
	m_target = m_recPath[0];
	m_recIndex = 1;
	m_motionTime = m_recTime;
	m_bRecorded = false;
	m_mat = m_recMatrix;
	m_angle = m_recAngle;
	m_bActive = m_recActive;
}

CGull::CGull(const char* name)
{
	m_speed = 10.0f;
	Init(name, 0);
	m_f7c = 0.785398185f;
	m_cellSize = 48.0f;
}

CGull::~CGull()
{
}

CButterfly::CButterfly(const char* name)
{
	m_speed = 7.0f;
	Init(name, 0);
	m_f7c = 0.0f;
	m_cellSize = 16.0f;
}

CButterfly::~CButterfly()
{
}

CDuck::CDuck(const char* name, int count)
	: m_pItems(NULL)
{
	m_speed = 2.0f;
	Init(name, 0);
	m_f70 = 0.523598790f;
	m_f7c = 0.0f;
	m_cellSize = 32.0f;
	m_count = count;
	if (count > 0)
	{
		m_pItems = new WMatrix[count];
	}
}

void CDuck::SetArea(const Waabb& area, unsigned long flags)
{
	CCreature::SetArea(area, flags);
	if (m_count == 0)
	{
		return;
	}
	m_pItems[0] = m_mat;
	for (int i = 1; i < m_count; i++)
	{
		m_pItems[i] = m_pItems[i - 1];
		m_pItems[i].pivot = m_pItems[i - 1].pivot + m_pItems[i - 1].za * 6.4f;
	}
}

void CDuck::Process(float delta)
{
	CheckTarget(delta);
	if (m_count == 0)
	{
		return;
	}
	m_pItems[0] = m_mat;
	for (int i = 1; i < m_count; i++)
	{
		WVector toPrev = m_pItems[i - 1].pivot - m_pItems[i].pivot;
		if (toPrev.Magnitude() >= 3.2f)
		{
			float maxTurn = delta * m_f70 * 0.7f;
			float minTurn = delta * m_f70 * -0.7f;
			float angle;
			float diff =
				CalcDeltaAngle(m_pItems[i - 1].pivot - m_pItems[i].pivot,
					m_pItems[i].za * -1.0f);
			if (diff < minTurn)
			{
				angle = minTurn;
			}
			else if (diff > maxTurn)
			{
				angle = maxTurn;
			}
			else
			{
				angle = diff;
			}
			m_pItems[i] = RotMat(angle, 1) * m_pItems[i];
			m_pItems[i].pivot += m_pItems[i].za * -1.0f * m_speed * delta;
		}
	}
}

void CDuck::Display()
{
	if (m_count == 0)
	{
		CCreature::Display();
		return;
	}
	for (int i = 0; i < m_count; i++)
	{
		m_pPet->SetMatrix(m_pItems[i] * (i == 0 ? 1.0f : 0.5f));
		m_pPet->Transform(g_view, 1.0f, 0);
		m_pPet->UpdateBSphere();
		if (g_view->InFrustum(m_pPet->GetBoundSphere()))
		{
			m_pPet->UpdateLightSource(&g_lightset, false, NULL);
			m_pPet->Render(g_view, false, 1.0f,
				((WViewEx*)g_view)->InFrustumSafe(m_pPet->m_bound), true,
				false);
		}
	}
}

CMole::CMole(const char* name)
{
	unsigned int& flags = m_flags;
	flags |= 1;
	Init(name, 0);
	m_radius = 6.4f;
}

void CMole::PlayEvent()
{
	CGolfDoc* doc = Doc()->m_pGolfDoc;
	if (InBunker(doc->GetPlayer(doc->m_currentPlayer)) < 1)
	{
		return;
	}
	CCreature::PlayEvent();
	if (GetPVS().GetGroundType(m_mat.pivot, true, NULL) == 3)
	{
		return;
	}
	m_mat.za = GolfBall().m_pos - (m_area.max + m_area.min) * 0.5f;
	m_mat.za.y = 0.0f;
	m_mat.za.Normalize();
	m_mat.xa = WCrossProduct(m_mat.ya, m_mat.za);
	m_mat.xa.Normalize();
	m_mat.pivot = GetPVS().GetGroundPoint(
		m_radius * m_mat.za + GolfBall().m_pos, false, NULL, true);
}

CDolphin::CDolphin(const char* name)
{
	unsigned int& flags = m_flags;
	flags |= 1;
	Init(name, 0);
	m_radius = 0.0f;
}

void CDolphin::PlayEvent()
{
	if (GolfBall().m_bOB && GolfBall().m_water == CGolfBall::WATER_SEA)
	{
		CCreature::PlayEvent();
	}
}

CCrab::CCrab(const char* name)
{
	unsigned int& flags = m_flags;
	flags |= 1;
	Init(name, 0);
	m_radius = -3.2f;
}

void CCrab::PlayEvent()
{
	CGolfDoc* doc = Doc()->m_pGolfDoc;
	if (InBunker(doc->GetPlayer(doc->m_currentPlayer)) >= 1)
	{
		CCreature::PlayEvent();
	}
}

CSparrow::CSparrow(const char* name, int count)
	: m_pItems(NULL)
{
	m_pPet = g_resrcmng->GetPuppet(name, false, true, false);
	m_angle = 0.0f;
	m_bRecorded = false;
	m_alpha = 0xff;
	m_bActive = false;
	m_maxCount = count;
	m_pItems = new WMatrix[count];
}

void CSparrow::PlayEvent()
{
	m_bActive = true;
	float speed = GolfBall().m_vel.Magnitude();
	m_count =
		(int)(m_maxCount / GolfBall().PreData(0)->vel.Magnitude() * speed);
	for (int i = 0; i < m_count; i++)
	{
		m_pItems[i] = RotMat(Random(-180.0f, 180.0f) * g_DEGTORAD, 1);
		m_pItems[i].pivot = (m_area.max + m_area.min) * 0.5f;
	}
}

void CSparrow::Display()
{
	if (m_pPet && m_bActive)
	{
		for (int i = 0; i < m_count; i++)
		{
			m_pPet->SetMatrix(m_pItems[i]);
			m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
			m_pPet->Transform(g_view, 1.0f, 0);
			m_pPet->UpdateBSphere();
			if (g_view->InFrustum(m_pPet->GetBoundSphere()))
			{
				m_pPet->UpdateLightSource(&g_lightset, false, NULL);
				m_pPet->Render(g_view, false, 1.0f,
					((WViewEx*)g_view)->InFrustumSafe(m_pPet->m_bound), true,
					false);
			}
		}
	}
}

CSanta::CSanta(const char* name)
{
	Init(name, 0);
	m_nodeIndex = 1;
	m_speed = 120.0f;
	m_f7c = 0.785398185f;
	m_f70 = 2.09439516f;
	m_climbRate = 0;
	m_f80 = 1.57079637f;
	m_state = 2;
}

CSanta::~CSanta()
{
}

void CSanta::Process(float delta)
{
	if (!m_pPet)
	{
		return;
	}
	if (m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				m_motionTime -= m_pMotion->e;
				const char* name;
				if (rand() % 10 == 0 &&
					_strcmpi(m_pMotion->name, "NPC_Default") == 0)
				{
					name = "NPC_Default1";
				}
				else
				{
					name = "NPC_Default";
				}
				SetMotion(name, 0.0f);
			}
			CheckTarget(delta);
		}
	}
}

void CSanta::CheckTarget(float delta)
{
	WVector d = m_target - m_mat.pivot;
	d.y = 0.0f;
	if (d.Magnitude() < (delta < 0.1f ? 0.1f : delta) * m_speed)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			if (m_nodeIndex == m_path.size() - 1)
			{
				m_bActive = false;
				return;
			}
			unsigned char prev = m_nodeIndex;
			m_nodeIndex = (m_nodeIndex + 1) % m_path.size();
			m_target = m_path[m_nodeIndex];
			m_target +=
				(RotMat(Random(-1.5707964f, 1.5707964f), 0).za * m_radius) *
				RotMat(Random(-3.1415927f, 3.1415927f), 1);
			d = m_target - m_mat.pivot;
			d.y = 0.0f;
			WVector seg;
			seg.x = m_path[m_nodeIndex].x - m_path[prev].x;
			seg.y = 0.0f;
			seg.z = m_path[m_nodeIndex].z - m_path[prev].z;
			float dist = d.Magnitude();
			m_speed = dist * 120.0f / seg.Magnitude();
			m_climbRate = (m_target.y - m_mat.pivot.y) / dist * m_speed;
			float turn = CalcDeltaAngle(d, m_mat.za * -1.0f);
			m_f7c = Abs(turn) > 1.5707964f ? 1.5707964f : Abs(turn);
			m_state = 1;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			if (m_recIndex < m_recPath.size())
			{
				m_target = m_recPath[m_recIndex];
				m_recIndex++;
			}
			else
			{
				m_bActive = false;
			}
		}
	}
	float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	switch (m_state)
	{
	case 0:
		if (Abs(m_angle) > delta * m_f80)
		{
			m_angle += (m_angle > 0 ? -m_f80 : m_f80) * delta; // 983 0076e200: sinal do proprio m_angle
		}
		else
		{
			m_angle = 0;
			m_state = 2;
		}
		break;
	case 1:
		if (Abs(m_angle) < m_f7c)
		{
			m_angle += (angle > 0 ? -m_f80 : m_f80) * delta;
		}
		else
		{
			m_state = 0;
		}
		break;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot.y += delta * m_climbRate;
	float dist = (g_camera.pivot - m_mat.pivot).Magnitude();
	if (dist < 640.0f)
	{
		if (!g_audio->IsPlaying("Santa"))
			g_audio->PlaySfx("santaboost", 0, 1, "Santa", 0, 0.5f, 200.0f);
		g_audio->SetSFXVolumeWhilePlaying("Santa",
			(640.0f - dist) * 0.0015625f);
	}
	else
	{
		g_audio->StopSfx("Santa");
	}
}

CFighter::CFighter(const char* name)
{
	Init(name, 0);
	m_nodeIndex = 1;
	m_speed = 320.0f;
	m_f7c = 0.785398185f;
	m_f70 = 2.09439516f;
	m_climbRate = 0;
	m_f80 = 1.57079637f;
	m_state = 2;
}

CFighter::~CFighter()
{
}

void CFighter::CheckTarget(float delta)
{
	WVector d = m_target - m_mat.pivot;
	d.y = 0.0f;
	if (d.Magnitude() < (delta < 0.1f ? 0.1f : delta) * m_speed)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			if (m_nodeIndex == m_path.size() - 1)
			{
				m_bActive = false;
				return;
			}
			m_nodeIndex = (m_nodeIndex + 1) % m_path.size();
			m_target = m_path[m_nodeIndex];
			d = m_target - m_mat.pivot;
			d.y = 0.0f;
			m_speed = d.Magnitude();
			m_climbRate = (m_target.y - m_mat.pivot.y) / m_speed * m_speed;
			float turn = CalcDeltaAngle(d, m_mat.za * -1.0f);
			m_f7c = Abs(turn) > 1.5707964f ? 1.5707964f : Abs(turn);
			m_state = 1;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			if (m_recIndex < m_recPath.size())
			{
				m_target = m_recPath[m_recIndex];
				m_recIndex++;
			}
			else
			{
				m_bActive = false;
			}
		}
	}
	float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	switch (m_state)
	{
	case 0:
		if (Abs(m_angle) > delta * m_f80)
		{
			m_angle += (m_angle > 0 ? -m_f80 : m_f80) * delta; // 983 0076e200: sinal do proprio m_angle
		}
		else
		{
			m_angle = 0;
			m_state = 2;
		}
		break;
	case 1:
		if (Abs(m_angle) < m_f7c)
		{
			m_angle += (angle > 0 ? -m_f80 : m_f80) * delta;
		}
		else
		{
			m_state = 0;
		}
		break;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot.y += delta * m_climbRate;
}

CUfo::CUfo(const char* name)
{
	Init(name, 0);
	m_nodeIndex = 1;
	m_speed = 2.85714293f;
	m_f7c = 0.785398185f;
	m_f70 = 2.09439516f;
	m_climbRate = 0;
	m_f80 = 1.57079637f;
	m_state = 2;
}

CUfo::~CUfo()
{
}

void CUfo::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				m_motionTime -= m_pMotion->e;
				SetMotion("NPC_Default", 0.0f);
			}
			CheckTarget(delta);
		}
	}
}

void CUfo::CheckTarget(float delta)
{
	WVector d = m_target - m_mat.pivot;
	d.y = 0.0f;
	if (d.Magnitude() < (delta < 0.1f ? 0.1f : delta) * m_speed)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			if (m_nodeIndex == m_path.size() - 1)
			{
				m_bActive = false;
				return;
			}
			unsigned char prev = m_nodeIndex;
			m_nodeIndex = (m_nodeIndex + 1) % m_path.size();
			m_target = m_path[m_nodeIndex];
			m_target +=
				(m_radius * RotMat(Random(-1.5707964f, 1.5707964f), 0).za) *
				RotMat(Random(-3.1415927f, 3.1415927f), 1);
			d = m_target - m_mat.pivot;
			d.y = 0.0f;
			WVector e = m_path[m_nodeIndex] - m_path[prev];
			e.y = 0.0f;
			float len = d.Magnitude();
			m_speed = len * 120.0f / e.Magnitude();
			m_climbRate = (m_target.y - m_mat.pivot.y) / len * m_speed;
			float turn = CalcDeltaAngle(d, m_mat.za * -1.0f);
			m_f7c = Abs(turn) > 1.5707964f ? 1.5707964f : Abs(turn);
			m_state = 1;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			if (m_recIndex < m_recPath.size())
			{
				m_target = m_recPath[m_recIndex];
				m_recIndex++;
			}
			else
			{
				m_bActive = false;
			}
		}
	}
	float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	switch (m_state)
	{
	case 0:
		if (Abs(m_angle) > delta * m_f80)
		{
			m_angle += (m_angle > 0 ? -m_f80 : m_f80) * delta; // 983 0076e200: sinal do proprio m_angle
		}
		else
		{
			m_angle = 0;
			m_state = 2;
		}
		break;
	case 1:
		if (Abs(m_angle) < m_f7c)
		{
			m_angle += (angle > 0 ? -m_f80 : m_f80) * delta;
		}
		else
		{
			m_state = 0;
		}
		break;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot.y += delta * m_climbRate;
}

ntPenguin::ntPenguin(const char* name)
{
	Init(name, 0);
	m_nodeIndex = 1;
	m_speed = 2.85714293f;
	m_f7c = 0.785398185f;
	m_f70 = 2.09439516f;
	m_climbRate = 0;
	m_f80 = 1.57079637f;
	m_state = 2;
}

ntPenguin::~ntPenguin()
{
}

void ntPenguin::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				m_motionTime -= m_pMotion->e;
				SetMotion("NPC_Default", 0.0f);
			}
			CheckTarget(delta);
		}
	}
}

void ntPenguin::CheckTarget(float delta)
{
	WVector d = m_target - m_mat.pivot;
	d.y = 0.0f;
	if (d.Magnitude() < (delta < 0.1f ? 0.1f : delta) * m_speed)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			if (m_nodeIndex == m_path.size() - 1)
				m_nodeIndex = 0;
			m_nodeIndex = (m_nodeIndex + 1) % m_path.size();
			m_target = m_path[m_nodeIndex];
			m_target +=
				(RotMat(Random(-1.5707964f, 1.5707964f), 0).za * m_radius) *
				RotMat(Random(-3.1415927f, 3.1415927f), 1);
			d = m_target - m_mat.pivot;
			d.y = 0.0f;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			if (m_recIndex < m_recPath.size())
			{
				m_target = m_recPath[m_recIndex];
				m_recIndex++;
			}
			else
			{
				m_nodeIndex = 0;
			}
		}
	}
	float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot.y += delta * m_climbRate;
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
}

ntPenguin_Sailor::ntPenguin_Sailor(const char* name)
{
	m_speed = 2.0f;
	Init(name, 0);
	m_f7c = 0.785398185f;
	m_cellSize = 48.0f;
	SetMotion("NPC_Act0", 0.0f);
	m_f120 = Random(0.7f, 1.0f);
	m_b124 = true;
}

ntPenguin_Sailor::~ntPenguin_Sailor()
{
}

void ntPenguin_Sailor::Process(float delta)
{
	if (!m_pPet)
	{
		return;
	}
	if (!m_bActive)
	{
		return;
	}
	float time = delta + m_motionTime;
	m_motionTime = time;
	if (time < 0.0f)
	{
		return;
	}
	ScriptProcess(m_pMotion, time - delta, time);
	if (m_motionTime > m_pMotion->e)
	{
		if (m_flags & 1)
		{
			m_bActive = false;
			m_motionTime = m_pMotion->s;
		}
		else
		{
			m_motionTime -= m_pMotion->e - m_pMotion->s;
			switch (rand() % 10)
			{
			case 0:
				SetMotion("NPC_Act1", 0.0f);
				m_b124 = false;
				break;
			case 1:
				SetMotion("NPC_Default", 0.0f);
				m_b124 = false;
				break;
			default:
				SetMotion("NPC_Act0", 0.0f);
				m_b124 = true;
				break;
			}
		}
	}
	if (m_flags & 1)
	{
		std::list<sAlpha>::iterator it = m_alphaList.begin();
		std::list<sAlpha>::iterator end = m_alphaList.end();
		while (it != m_alphaList.end())
		{
			std::list<sAlpha>::iterator cur;
			cur = it;
			++it;
			if (it == end)
			{
				m_alpha = (unsigned char)(*cur).alpha;
			}
			else if (m_motionTime > (*cur).time && m_motionTime < (*it).time)
			{
				float t =
					(m_motionTime - (*cur).time) / ((*it).time - (*cur).time);
				m_alpha = (unsigned char)((1.0f - t) * (*cur).alpha +
					t * (*it).alpha);
				return;
			}
		}
	}
	else if (m_b124)
	{
		CheckTarget(delta);
	}
}

void ntPenguin_Sailor::Display()
{
	if (!COption::Instance()->vIsNpcEnabled() && GOLFDOC()->m_currentHole != 19)
	{
		return;
	}
	if (m_pPet && m_bActive && !(m_motionTime < 0.0f))
	{
		if (m_flags & 1)
		{
			m_pPet->SetAlpha(m_alpha);
		}
		m_pPet->SetMatrix((Abs(m_angle) > 0.0f ? RotMat(m_angle, 2) * m_mat : m_mat) *
				m_f120);
		m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
		m_pPet->Transform(g_view, 1.0f, 0);
		m_pPet->UpdateBSphere();
		if (g_view->InFrustum(m_pPet->GetBoundSphere()))
		{
			m_pPet->UpdateLightSource(&g_lightset, false, NULL);
			WPuppet* puppet = m_pPet;
			WView* view = g_view;
			puppet->Render(view, true, 1.0f,
				((WViewEx*)view)->InFrustumSafe(puppet->m_bound), false, false);
		}
	}
}

void ntPenguin_Sailor::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			int tries = 0;
			int nx, ny, nz;
			int index;
			int i;
			do
			{
				tries++;
				int rnd[3];
				for (i = 0; i < 3; i++)
				{
					rnd[i] = rand() % 3 - 1;
				}
				int cur = m_pCells[0];
				int x = cur % m_gridX;
				int y = cur / (m_gridZ * m_gridX);
				int z = cur % (m_gridZ * m_gridX) / m_gridX;
				nx = Between(0, rnd[0] + x, m_gridX - 1);
				ny = Between(0, rnd[1] + y, m_gridY - 1);
				nz = Between(0, rnd[2] + z, m_gridZ - 1);
				index = (m_gridZ * ny + nz) * m_gridX + nx;
				for (i = 0; i < m_cellCount; i++)
				{
					if (m_pCells[i] == index)
						break;
				}
			} while (i < m_cellCount && tries < 5);
			for (i = m_cellCount - 1; i >= 1; i--)
			{
				m_pCells[i] = m_pCells[i - 1];
			}
			m_pCells[0] = index;
			m_target.x = Random(0.0f, m_cellX) + nx * m_cellX + m_area.min.x;
			if (m_gridY > 1)
				m_target.y =
					Random(0.0f, m_cellY) + ny * m_cellY + m_area.min.y;
			m_target.z = Random(0.0f, m_cellZ) + nz * m_cellZ + m_area.min.z;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			m_target = m_recPath[m_recIndex];
			m_recIndex++;
		}
	}
	if (m_gridY > 1)
	{
		m_mat.pivot += (m_target - m_mat.pivot).Normalize() * m_speed * delta;
	}
	else
	{
		float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
		{
			float limit = delta * m_f70;
			angle = Between(-limit, angle, limit);
			m_mat = RotMat(angle, 1) * m_mat;
		}
		m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	}
}

ntPenguin_Duck::ntPenguin_Duck(const char* name, int count)
	: m_pItems(NULL)
{
	m_speed = 2.0f;
	Init(name, 0);
	m_f70 = 0.523598790f;
	m_f7c = 0.0f;
	m_cellSize = 32.0f;
	m_count = count;
	if (count > 0)
	{
		m_pItems = new WMatrix[count];
		m_pValues = new float[m_count];
		m_pValues[0] = 1.0f;
		for (int i = 1; i < m_count; i++)
		{
			m_pValues[i] = Random(0.6f, 0.8f);
		}
	}
	SetMotion("NPC_Act0", 0.0f);
	m_b120 = false;
}

ntPenguin_Duck::~ntPenguin_Duck()
{
	if (m_pItems)
	{
		delete[] m_pItems;
		m_pItems = NULL;
	}
	if (m_pValues)
	{
		delete[] m_pValues;
		m_pValues = NULL;
	}
}

void ntPenguin_Duck::SetArea(const Waabb& area, unsigned long flags)
{
	CCreature::SetArea(area, flags);
	if (m_count == 0)
	{
		return;
	}
	m_pItems[0] = m_mat;
	for (int i = 1; i < m_count; i++)
	{
		m_pItems[i] = m_pItems[i - 1];
		m_pItems[i].pivot = m_pItems[i - 1].pivot + m_pItems[i - 1].za * 6.4f;
	}
}

void ntPenguin_Duck::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (!(m_motionTime <= m_pMotion->e))
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = m_pMotion->s;
				}
				else
				{
					m_motionTime -= m_pMotion->e - m_pMotion->s;
					SetMotion("NPC_Act0", 0);
					if (rand() % 10 != 0)
					{
						SetMotion("NPC_Act0", 0);
						m_b120 = true;
					}
					else
					{
						SetMotion("NPC_Act1", 0);
						m_b120 = false;
					}
				}
			}
			if (m_b120)
				CheckTarget(delta);
			m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false); // 983 0076b64e
			if (m_count == 0)
				return;
			if (!m_b120)
				return;
			m_pItems[0] = m_mat;
			for (int i = 1; i < m_count; i++)
			{
				WVector toPrev = m_pItems[i - 1].pivot - m_pItems[i].pivot;
				if (toPrev.Magnitude() >= 4.16f)
				{
					float angle = Between(delta * m_f70 * -0.7f,
						CalcDeltaAngle(m_pItems[i - 1].pivot -
								m_pItems[i].pivot,
							m_pItems[i].za * -1.0f),
						delta * m_f70 * 0.7f);
					m_pItems[i] = RotMat(angle, 1) * m_pItems[i];
					m_pItems[i].pivot +=
						m_pItems[i].za * -1.0f * m_speed * delta;
					m_pItems[i].pivot = GetPVS().GetGroundPoint(
						m_pItems[i].pivot, true, NULL, false);
				}
			}
		}
	}
}

void ntPenguin_Duck::Display()
{
	if (!COption::Instance()->vIsNpcEnabled() && GOLFDOC()->m_currentHole != 19)
	{
		return;
	}
	if (m_pPet && m_bActive && !(m_motionTime < 0.0f))
	{
		if (m_count == 0)
		{
			CCreature::Display();
			return;
		}
		for (int i = 0; i < m_count; i++)
		{
			m_pPet->SetMatrix(m_pItems[i] * m_pValues[i]);
			m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
			m_pPet->Transform(g_view, 1.0f, 0);
			m_pPet->UpdateBSphere();
			if (g_view->InFrustum(m_pPet->GetBoundSphere()))
			{
				m_pPet->UpdateLightSource(&g_lightset, false, NULL);
				WPuppet* puppet = m_pPet;
				WView* view = g_view;
				puppet->Render(view, false, 1.0f,
					((WViewEx*)view)->InFrustumSafe(puppet->m_bound), true,
					false);
			}
		}
	}
}

ntNpc_Dragon::ntNpc_Dragon(const char* name, float speed)
{
	Init(name, 0);
	m_dist = 0;
	m_waitEnd = speed;
	m_bWait = 1;
}

ntNpc_Dragon::~ntNpc_Dragon()
{
}

void ntNpc_Dragon::Process(float delta)
{
	if (m_bActive && m_pPet)
	{
		m_motionTime += delta;
		if (m_bWait)
		{
			if (!(m_motionTime > m_waitEnd))
			{
				return;
			}
			m_motionTime = 0.0f;
			m_bWait = 0;
			SetMotion("NPC_Act02", -1.0f);
		}
		ScriptProcess(m_pMotion, m_motionTime - delta, m_motionTime);
		if (m_motionTime > m_pMotion->e)
		{
			int r = rand();
			const char* name;
			if (r % 101 < 5)
			{
				name = "NPC_Act02";
			}
			else if (r % 101 < 10)
			{
				name = "NPC_Act01";
			}
			else
			{
				name = "NPC_Default";
			}
			SetMotion(name, 0.0f);
		}
		m_dist = delta * 100.0f + m_dist;
		CheckTarget(delta);
	}
}

void ntNpc_Dragon::Display()
{
	if (m_bActive && m_bWait != 1)
	{
		CCreature::Display();
	}
}

void ntNpc_Dragon::CheckTarget(float delta)
{
	WVector next = m_spline.Evaluate(m_dist + 0.1f);
	m_mat.pivot = m_spline.Evaluate(m_dist);
	if (WisEqual(next, m_path[m_path.size() - 1], 1e-5f))
	{
		m_dist = 0.0f;
	}
	m_mat = MakeCameraMatrix(next, m_mat.pivot);
}

void ntNpc_Dragon::SetArea(const std::vector<WVector>& points, float speed,
	unsigned long flags)
{
	CCreature::SetArea(points, speed, flags);
	WVector* pPoints = (WVector*)_alloca(points.size() * sizeof(WVector));
	float* pTimes = (float*)_alloca(points.size() * sizeof(float));
	pPoints[0] = points[0];
	pTimes[0] = 0;
	for (unsigned int i = 1; i < points.size(); i++)
	{
		pPoints[i] = points[i];
		pTimes[i] = pTimes[i - 1] + (points[i] - points[i - 1]).Magnitude();
	}
	m_spline.Init(pPoints, pTimes, points.size());
}

ntNpc_BlackSparrow::ntNpc_BlackSparrow(const char* name)
{
	m_speed = 5.5f;
	Init(name, 0);
	m_f7c = 0.785398185f;
	m_cellSize = 48.0f;
	SetMotion("NPC_Default", 0.0f);
	m_b120 = true;
}

ntNpc_BlackSparrow::~ntNpc_BlackSparrow()
{
}

void ntNpc_BlackSparrow::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = m_pMotion->s;
				}
				else
				{
					m_motionTime -= m_pMotion->e - m_pMotion->s;
					if (rand() % 101 < 20)
					{
						SetMotion("NPC_Act01", 0.0f);
						m_b120 = false;
					}
					else
					{
						SetMotion("NPC_Default", 0.0f);
						m_b120 = true;
					}
				}
			}
			if (m_flags & 1)
			{
				std::list<sAlpha>::iterator it = m_alphaList.begin();
				std::list<sAlpha>::iterator end = m_alphaList.end();
				while (it != m_alphaList.end())
				{
					std::list<sAlpha>::iterator cur;
					cur = it;
					++it;
					if (it == end)
					{
						m_alpha = (unsigned char)(*cur).alpha;
					}
					else if (m_motionTime > (*cur).time &&
						m_motionTime < (*it).time)
					{
						float t = (m_motionTime - (*cur).time) /
							((*it).time - (*cur).time);
						m_alpha = (unsigned char)((1.0f - t) * (*cur).alpha +
							t * (*it).alpha);
						return;
					}
				}
			}
			else if (m_b120)
			{
				CheckTarget(delta);
			}
		}
	}
}

void ntNpc_BlackSparrow::Display()
{
	if (!COption::Instance()->vIsNpcEnabled() && GOLFDOC()->m_currentHole != 19)
	{
		return;
	}
	if (m_pPet && m_bActive && !(m_motionTime < 0.0f))
	{
		if (m_flags & 1)
		{
			m_pPet->SetAlpha(m_alpha);
		}
		m_pPet->SetMatrix((Abs(m_angle) > 0.0f ? RotMat(m_angle, 2) * m_mat : m_mat) * 1.0f);
		m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
		m_pPet->Transform(g_view, 1.0f, 0);
		m_pPet->UpdateBSphere();
		if (g_view->InFrustum(m_pPet->GetBoundSphere()))
		{
			m_pPet->UpdateLightSource(&g_lightset, false, NULL);
			WPuppet* puppet = m_pPet;
			WView* view = g_view;
			puppet->Render(view, true, 1.0f,
				((WViewEx*)view)->InFrustumSafe(puppet->m_bound), false, false);
		}
	}
}

void ntNpc_BlackSparrow::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			int tries = 0;
			int nx, ny, nz;
			int index;
			int i;
			do
			{
				tries++;
				int rnd[3];
				for (i = 0; i < 3; i++)
				{
					rnd[i] = rand() % 3 - 1;
				}
				int cur = m_pCells[0];
				int x = cur % m_gridX;
				int y = cur / (m_gridZ * m_gridX);
				int z = cur % (m_gridZ * m_gridX) / m_gridX;
				nx = Between(0, rnd[0] + x, m_gridX - 1);
				ny = Between(0, rnd[1] + y, m_gridY - 1);
				nz = Between(0, rnd[2] + z, m_gridZ - 1);
				index = (m_gridZ * ny + nz) * m_gridX + nx;
				for (i = 0; i < m_cellCount; i++)
				{
					if (m_pCells[i] == index)
						break;
				}
			} while (i < m_cellCount && tries < 5);
			for (i = m_cellCount - 1; i >= 1; i--)
			{
				m_pCells[i] = m_pCells[i - 1];
			}
			m_pCells[0] = index;
			m_target.x = Random(0.0f, m_cellX) + nx * m_cellX + m_area.min.x;
			if (m_gridY > 1)
				m_target.y =
					Random(0.0f, m_cellY) + ny * m_cellY + m_area.min.y;
			m_target.z = Random(0.0f, m_cellZ) + nz * m_cellZ + m_area.min.z;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			m_target = m_recPath[m_recIndex];
			m_recIndex++;
		}
	}
	if (m_gridY > 1)
	{
		m_mat.pivot += (m_target - m_mat.pivot).Normalize() * m_speed * delta;
	}
	else
	{
		float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
		{
			float limit = delta * m_f70;
			angle = Between(-limit, angle, limit);
			m_mat = RotMat(angle, 1) * m_mat;
		}
		m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	}
}

ntNpc_PolaShip::ntNpc_PolaShip(const char* name, float speed)
{
	Init(name, 0);
	m_dist = 0;
	m_waitEnd = speed;
	m_bWait = 1;
}

ntNpc_PolaShip::~ntNpc_PolaShip()
{
}

void ntNpc_PolaShip::Process(float delta)
{
	if (m_bActive && m_pPet)
	{
		m_motionTime += delta;
		if (m_bWait)
		{
			if (!(m_motionTime > m_waitEnd))
			{
				return;
			}
			m_motionTime = 0.0f;
			m_bWait = 0;
			SetMotion("NPC_Act02", -1.0f);
		}
		ScriptProcess(m_pMotion, m_motionTime - delta, m_motionTime);
		w_motion_data* motion = m_pMotion;
		if (m_motionTime > motion->e)
		{
			m_motionTime -= motion->e - motion->s;
		}
		m_dist = delta * 10.0f + m_dist;
		CheckTarget(delta);
	}
}

void ntNpc_PolaShip::Display()
{
	if (m_bActive && m_bWait != 1)
	{
		CCreature::Display();
	}
}

void ntNpc_PolaShip::CheckTarget(float delta)
{
	WVector next = m_spline.Evaluate(m_dist + 0.1f);
	m_mat.pivot = m_spline.Evaluate(m_dist);
	if (WisEqual(next, m_path[m_path.size() - 1], 1e-5f))
	{
		m_dist = 0.0f;
	}
	next.y = m_mat.pivot.y;
	m_mat = MakeCameraMatrix(next, m_mat.pivot);
}

void ntNpc_PolaShip::SetArea(const std::vector<WVector>& points, float speed,
	unsigned long flags)
{
	CCreature::SetArea(points, speed, flags);
	WVector* pPoints = (WVector*)_alloca(points.size() * sizeof(WVector));
	float* pTimes = (float*)_alloca(points.size() * sizeof(float));
	pPoints[0] = points[0];
	pTimes[0] = 0;
	for (unsigned int i = 1; i < points.size(); i++)
	{
		pPoints[i] = points[i];
		pTimes[i] = pTimes[i - 1] + (points[i] - points[i - 1]).Magnitude();
	}
	m_spline.Init(pPoints, pTimes, points.size());
}

ntNpc_PolaPenguin1::ntNpc_PolaPenguin1(const char* name, float speed)
{
	m_speed = speed;
	Init(name, 0);
}

ntNpc_PolaPenguin1::~ntNpc_PolaPenguin1()
{
}

void ntNpc_PolaPenguin1::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			w_motion_data* motion = m_pMotion;
			if (m_motionTime > motion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = motion->s;
					CheckTarget(delta);
					return;
				}
				m_motionTime -= motion->e - motion->s;
			}
			CheckTarget(delta);
		}
	}
}

void ntNpc_PolaPenguin1::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		FindTarget();
	}
	float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
}

void ntNpc_PolaPenguin1::FindTarget()
{
	for (int i = 0; i < 3; i++)
	{
		float t = Random(0.0f, 1.0f);
		m_target.p[i] = m_area.min.p[i] * t + m_area.max.p[i] * (1.0f - t);
	}
	m_target = GetPVS().GetGroundPoint(m_target, false, NULL, false);
}

ntNpc_PolaPenguin2::ntNpc_PolaPenguin2(const char* name)
{
	m_speed = 2.0f;
	Init(name, 0);
	m_b120 = true;
	SetMotion("NPC_Act0", 0.0f);
}

ntNpc_PolaPenguin2::~ntNpc_PolaPenguin2()
{
}

void ntNpc_PolaPenguin2::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			w_motion_data* motion = m_pMotion;
			if (m_motionTime > motion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = motion->s;
					CheckTarget(delta);
					return;
				}
				m_motionTime -= motion->e - motion->s;
				if (m_b120 == 1)
				{
					if (IsSlidable())
					{
						SetMotion("NPC_Default", 0.0f);
						m_b120 = false;
						CheckTarget(delta);
						return;
					}
				}
				SetMotion("NPC_Act0", 0.0f);
				m_b120 = true;
			}
			CheckTarget(delta);
		}
	}
}

void ntNpc_PolaPenguin2::Display()
{
	if (!COption::Instance()->vIsNpcEnabled() && GOLFDOC()->m_currentHole != 19)
	{
		return;
	}
	if (m_pPet && m_bActive && !(m_motionTime < 0.0f))
	{
		if (m_flags & 1)
		{
			m_pPet->SetAlpha(m_alpha);
		}
		WVector move = m_pPet->GetDeltaVec(m_pMotion->name, m_motionTime);
		move = RotVec(move, m_mat);
		m_mat.pivot -= move;
		m_pPet->SetMatrix(m_mat);
		m_pPet->Animate983(0, 0, m_motionTime, 0, 0, 0);
		m_pPet->Transform(g_view, 1.0f, 0);
		m_pPet->UpdateBSphere();
		m_mat.pivot += move;
		if (g_view->InFrustum(m_pPet->GetBoundSphere()))
		{
			m_pPet->UpdateLightSource(&g_lightset, false, NULL);
			WPuppet* puppet = m_pPet;
			WView* view = g_view;
			puppet->Render(view, true, 1.0f,
				((WViewEx*)view)->InFrustumSafe(puppet->m_bound), false, false);
		}
	}
}

void ntNpc_PolaPenguin2::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		FindTarget();
	}
	if (m_b120)
	{
		float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
		{
			float limit = delta * m_f70;
			angle = Between(-limit, angle, limit);
			m_mat = RotMat(angle, 1) * m_mat;
		}
		m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
		return;
	}
	else
	{
		WVector move = m_pPet->GetDeltaVec(m_pMotion->name,
			m_motionTime - delta, m_motionTime);
		move = RotVec(move, m_mat);
		m_mat.pivot += move;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	}
}

void ntNpc_PolaPenguin2::FindTarget()
{
	for (int i = 0; i < 3; i++)
	{
		float t = Random(0.0f, 1.0f);
		m_target.p[i] = (1.0f - t) * m_area.max.p[i] + t * m_area.min.p[i];
	}
	while (m_area.IsInclude(m_target))
	{
		m_target += (m_target - m_mat.pivot).Normalize() * 3.2f;
	}
	m_target = GetPVS().GetGroundPoint(m_target, true, NULL, false);
}

bool ntNpc_PolaPenguin2::IsSlidable()
{
	float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		return false;
	}
	w_motion_data* motion = m_pPet->FindMotionData("NPC_Default");
	if (!motion)
	{
		return false;
	}
	WVector dist = m_target - m_mat.pivot;
	WVector move;
	move = m_pPet->GetDeltaVec("NPC_Default", motion->e);
	if (dist.Magnitude() < move.Magnitude())
	{
		return false;
	}
	return true;
}

CNPC_BIRD::CNPC_BIRD(const char* name, float speed)
{
	m_speed = speed;
	Init(name, 0);
}

CNPC_BIRD::~CNPC_BIRD()
{
}

void CNPC_BIRD::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			w_motion_data* motion = m_pMotion;
			if (m_motionTime > motion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = motion->s;
					CheckTarget(delta);
					return;
				}
				m_motionTime -= motion->e - motion->s;
			}
			CheckTarget(delta);
		}
	}
}

void CNPC_BIRD::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		FindTarget();
	}
	float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
}

void CNPC_BIRD::FindTarget()
{
	for (int i = 0; i < 3; i++)
	{
		float t = Random(0.0f, 1.0f);
		m_target.p[i] = m_area.min.p[i] * t + m_area.max.p[i] * (1.0f - t);
	}
	m_target = GetPVS().GetGroundPoint(m_target, false, NULL, false);
}

ntNpc_DLCreature::ntNpc_DLCreature(const char* name, float speed)
{
	m_speed = speed;
	Init(name, 0);
}

ntNpc_DLCreature::~ntNpc_DLCreature()
{
}

void ntNpc_DLCreature::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f) && m_pMotion) // 983: confere a motion (o CNPC_BIRD nao)
		{
			ScriptProcess(m_pMotion, time - delta, time);
			w_motion_data* motion = m_pMotion;
			if (m_motionTime > motion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = motion->s;
				}
				else
					m_motionTime -= motion->e - motion->s;
			}
			CheckTarget(delta);
		}
	}
}

void ntNpc_DLCreature::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		FindTarget();
	}
	float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
}

void ntNpc_DLCreature::FindTarget()
{
	for (int i = 0; i < 3; i++)
	{
		float t = Random(0.0f, 1.0f);
		m_target.p[i] = m_area.min.p[i] * t + m_area.max.p[i] * (1.0f - t);
	}
	m_target = GetPVS().GetGroundPoint(m_target, false, NULL, false);
}

CNPC_PARROT::CNPC_PARROT(const char* name)
{
	m_speed = 10.0f;
	Init(name, 0);
	m_f7c = 0.785398185f;
	m_cellSize = 48.0f;
}

CNPC_PARROT::~CNPC_PARROT()
{
}

CNPC_TORTOISE::CNPC_TORTOISE(const char* name, float speed)
{
	Init(name, 0);
	m_nodeIndex = 1;
	m_f7c = 0.785398185f;
	m_speed = speed + speed;
	m_f70 = 2.09439516f;
	m_climbRate = 0;
	m_f80 = 1.57079637f;
	m_state = 2;
}

CNPC_TORTOISE::~CNPC_TORTOISE()
{
}

void CNPC_TORTOISE::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				m_motionTime -= m_pMotion->e;
				SetMotion("NPC_Default", 0.0f);
			}
			CheckTarget(delta);
		}
	}
}

void CNPC_TORTOISE::CheckTarget(float delta)
{
	WVector d = m_target - m_mat.pivot;
	d.y = 0.0f;
	if (d.Magnitude() < (delta < 0.1f ? 0.1f : delta) * m_speed)
	{
		if (m_recIndex >= m_recPath.size() || m_bRecorded)
		{
			if (m_nodeIndex == m_path.size() - 1)
				m_nodeIndex = 0;
			m_nodeIndex = (m_nodeIndex + 1) % m_path.size();
			m_target = m_path[m_nodeIndex];
			m_target +=
				(RotMat(Random(-1.5707964f, 1.5707964f), 0).za * m_radius) *
				RotMat(Random(-3.1415927f, 3.1415927f), 1);
			d = m_target - m_mat.pivot;
			d.y = 0.0f;
			if (m_bRecorded)
				m_recPath.push_back(m_target);
		}
		else
		{
			if (m_recIndex < m_recPath.size())
			{
				m_target = m_recPath[m_recIndex];
				m_recIndex++;
			}
			else
			{
				m_nodeIndex = 0;
			}
		}
	}
	float angle = CalcDeltaAngle(d, m_mat.za * -1.0f);
	if (Abs(angle) > g_DEGTORAD)
	{
		float limit = delta * m_f70;
		angle = Between(-limit, angle, limit);
		m_mat = RotMat(angle, 1) * m_mat;
	}
	m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
	m_mat.pivot.y += delta * m_climbRate;
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
}

CNPC_TRSHIP::CNPC_TRSHIP(const char* name, float speed)
{
	Init(name, 0);
	m_dist = 0;
	float clamped = speed < -1.0f ? -1.0f : (speed > 1.0f ? 1.0f : speed);
	m_bWait = true;
	if (clamped == speed)
	{
		m_f124 = 1.0f;
	}
	else
	{
		m_f124 = speed;
	}
	m_motionName.assign("NPC_default", 11);
}

CNPC_TRSHIP::~CNPC_TRSHIP()
{
}

void CNPC_TRSHIP::Process(float delta)
{
	if (m_bActive && m_pPet)
	{
		m_motionTime += delta;
		if (m_bWait)
		{
			if (!(m_motionTime > 0.0f))
			{
				return;
			}
			m_motionTime = 0.0f;
			m_bWait = false;
			SetMotion(m_motionName.c_str(), -1.0f);
		}
		ScriptProcess(m_pMotion, m_motionTime - delta, m_motionTime);
		w_motion_data* motion = m_pMotion;
		if (m_motionTime > motion->e)
		{
			m_motionTime -= motion->e - motion->s;
		}
		m_dist = delta * 10.0f + m_dist;
		CheckTarget(delta);
	}
}

void CNPC_TRSHIP::Display()
{
	if (m_bActive && m_bWait != true)
	{
		CCreature::Display();
	}
}

void CNPC_TRSHIP::CheckTarget(float delta)
{
	WVector next = m_spline.Evaluate(m_dist + 0.1f);
	m_mat.pivot = m_spline.Evaluate(m_dist);
	if (WisEqual(next, m_path[m_path.size() - 1], 1e-5f))
	{
		m_dist = 0.0f;
	}
	next.y = m_mat.pivot.y;
	m_mat = MakeCameraMatrix(next, m_mat.pivot);
}

void CNPC_TRSHIP::SetArea(const std::vector<WVector>& points, float speed,
	unsigned long flags)
{
	CCreature::SetArea(points, speed, flags);
	WVector* pPoints = (WVector*)_alloca(points.size() * sizeof(WVector));
	float* pTimes = (float*)_alloca(points.size() * sizeof(float));
	pPoints[0] = points[0];
	pTimes[0] = 0;
	for (unsigned int i = 1; i < points.size(); i++)
	{
		pPoints[i] = points[i];
		float length = (points[i] - points[i - 1]).Magnitude();
		pTimes[i] = pTimes[i - 1] + (length != 0.0f ? length : 1.0f) / m_f124; // 983 00773560 +0x124
	}
	m_spline.Init(pPoints, pTimes, points.size());
}

ntNpc_AreaWalker::ntNpc_AreaWalker(const char* name)
{
	m_walking = true;
	m_speed = 2.0f;
	Init(name, 0);
}

ntNpc_AreaWalker::~ntNpc_AreaWalker()
{
}

void ntNpc_AreaWalker::Process(float delta)
{
	if (m_pPet && m_bActive)
	{
		float time = delta + m_motionTime;
		m_motionTime = time;
		if (!(time < 0.0f))
		{
			ScriptProcess(m_pMotion, time - delta, time);
			if (m_motionTime > m_pMotion->e)
			{
				if (m_flags & 1)
				{
					m_bActive = false;
					m_motionTime = m_pMotion->s;
					CheckTarget(delta);
				}
				SelectNextMotion();
			}
			CheckTarget(delta);
		}
	}
}

void ntNpc_AreaWalker::CheckTarget(float delta)
{
	if ((m_mat.pivot - m_target).Magnitude() < 1.0f)
	{
		FindTarget();
	}
	if (m_walking)
	{
		float angle = CalcDeltaAngle(m_target - m_mat.pivot, m_mat.za * -1.0f);
		if (Abs(angle) > g_DEGTORAD)
		{
			float limit = delta * m_f70;
			angle = Between(-limit, angle, limit);
			m_mat = RotMat(angle, 1) * m_mat;
		}
		m_mat.pivot += m_mat.za * -1.0f * m_speed * delta;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	}
	else
	{
		WVector move = m_pPet->GetDeltaVec(m_pMotion->name,
			m_motionTime - delta, m_motionTime);
		move = RotVec(move, m_mat);
		m_mat.pivot += move;
		m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	}
}

void ntNpc_AreaWalker::FindTarget()
{
	for (int i = 0; i < 3; i++)
	{
		float t = Random(0.0f, 1.0f);
		m_target.p[i] = (1.0f - t) * m_area.max.p[i] + t * m_area.min.p[i];
	}
	while (m_area.IsInclude(m_target))
	{
		m_target += (m_target - m_mat.pivot).Normalize() * 3.2f;
	}
	m_target = GetPVS().GetGroundPoint(m_target, true, NULL, false);
}

void ntNpc_AreaWalker::AddWalkingMotion(const char* name)
{
	std::vector<std::string>::iterator it =
		std::find(m_walkingMotions.begin(), m_walkingMotions.end(), name);
	if (it == m_walkingMotions.end())
	{
		m_walkingMotions.push_back(name);
	}
}

void ntNpc_AreaWalker::SelectNextMotion()
{
	std::map<std::string, std::list<MotionMap> >::iterator map;
	map = m_motions.find(m_pMotion->name);
	if (map == m_motions.end())
		return;
	int roll = rand() % 100;
	const std::list<MotionMap>& motions = (*map).second;
	std::list<MotionMap>::const_iterator it;
	for (it = motions.begin(); it != motions.end(); ++it)
	{
		if ((*it).index > roll)
		{
			const char* name = (*it).name.c_str();
			if (name)
			{
				SetMotion(name, 0);
				m_walking =
					std::find(m_walkingMotions.begin(), m_walkingMotions.end(),
						m_pMotion->name) != m_walkingMotions.end();
			}
			return;
		}
	}
}

ntNpc_WizCityArea::ntNpc_WizCityArea(const char* name)
	: ntNpc_AreaWalker(name)
{
	ConstructNextMotion();
	AddWalkingMotion("NPC_Default");
	int choice = rand() % 2;
	if (choice < 1)
	{
		SetMotion("NPC_Default", -1.0f);
	}
	else
	{
		m_walking = false;
		SetMotion("NPC_Ato", -1.0f);
	}
}

void ntNpc_WizCityArea::Display()
{
	WMatrix matrix = m_mat;
	m_mat *= m_scale;
	CCreature::Display();
	m_mat = matrix;
}

void ntNpc_WizCityArea::ConstructNextMotion()
{
	MotionMap map;
	std::list<MotionMap> motions;
	map.name = "NPC_Ato";
	map.index = 100;
	motions.push_back(map);
	m_motions.insert(std::make_pair("NPC_Default", motions));
	map.name = "NPC_Default";
	map.index = 100;
	motions.clear();
	motions.push_back(map);
	m_motions.insert(std::make_pair("NPC_Ato", motions));
}

ntNpc_NodeWalker::ntNpc_NodeWalker(const char* name, float speed)
{
	m_speed = speed;
	m_length = 0.0f;
	m_f124 = 0.0f;
	Init(name, 0);
}

ntNpc_NodeWalker::~ntNpc_NodeWalker()
{
}

void ntNpc_NodeWalker::Process(float delta)
{
	m_f124 = delta * m_speed + m_f124;
	CCreature::Process(delta);
}

void ntNpc_NodeWalker::SetArea(const std::vector<WVector>& points, float speed,
	unsigned long flags)
{
	CCreature::SetArea(points, speed, flags);
	m_path.push_back(points[0]);
	m_path.push_back(points[1]);
	MakeSpline(m_path, m_length, m_spline);
}

void ntNpc_NodeWalker::CheckTarget(float delta)
{
	WVector next = m_spline.Evaluate(m_f124 + 0.1f);
	m_mat.pivot = m_spline.Evaluate(m_f124);
	if (m_f124 > m_length)
	{
		m_f124 = 0.0f;
	}
	next = GetPVS().GetGroundPoint(next, true, NULL, false);
	m_mat.pivot = GetPVS().GetGroundPoint(m_mat.pivot, true, NULL, false);
	if (next.y < m_mat.pivot.y)
	{
		next.y = m_mat.pivot.y;
	}
	m_mat = MakeCameraMatrix(next, m_mat.pivot);
}

void ntNpc_NodeWalker::MakeSpline(const std::vector<WVector>& points,
	float& length, ntCatmullRom& spline)
{
	int count = points.size();
	WVector* pPoints = (WVector*)_alloca(count * sizeof(WVector));
	float* pTimes = (float*)_alloca(count * sizeof(float));
	pPoints[0] = points[0];
	pTimes[0] = 0;
	for (int i = 1; i < count; i++)
	{
		pPoints[i] = points[i];
		pTimes[i] = pTimes[i - 1] + (points[i] - points[i - 1]).Magnitude();
	}
	length = pTimes[count - 2];
	spline.Init(pPoints, pTimes, count);
}

ntNpc_WizCityNode::ntNpc_WizCityNode(const char* name, float speed)
	: ntNpc_NodeWalker(name, speed)
{
	m_scale = 1.0f;
}

void ntNpc_WizCityNode::Display()
{
	WMatrix matrix = m_mat;
	m_mat *= m_scale;
	CCreature::Display();
	m_mat = matrix;
}

CDisposable::CDisposable(const char* name)
{
	unsigned int& flags = m_flags;
	flags |= 1;
	Init(name, 0);
}

void CDisposable::PlayEvent()
{
	CCreature::PlayEvent();
	m_mat.za = m_path[0] - m_path[1];
	m_mat.za.y = 0.0f;
	m_mat.za.Normalize();
	m_mat.ya = WVector::UNIT_POS_Y;
	m_mat.xa = WCrossProduct(m_mat.ya, m_mat.za);
	m_mat.pivot = GetPVS().GetGroundPoint(m_path[0], true, NULL, true);
}

COnGameNet::COnGameNet(const char* name, unsigned char type)
	: m_hTexture(0)
{
	Init(name, 0);
	m_speed = 0.5f;
	m_playerIndex = type;
	m_hTexture = g_resrcmng->LoadTexture("camera.jpg", 0, 0,
		MakeStr("camera_%d.jpg", type));
	w_texlist* tex = g_resrcmng->FindTexture("camera.jpg");
	if (tex)
	{
		m_pPet->ChangeTextureByHandle(m_hTexture, tex->texhandle, NULL);
	}
}

COnGameNet::~COnGameNet()
{
	if (g_resrcmng && m_hTexture)
	{
		g_resrcmng->Release(m_hTexture);
		m_hTexture = 0;
	}
}

void COnGameNet::Process(float delta)
{
	if (Doc()->m_pGolfDoc->m_currentPlayer != m_playerIndex)
	{
		m_bActive = false;
		return;
	}
	m_bActive = true;
	CCreature::Process(delta);
}

void COnGameNet::CheckTarget(float delta)
{
	if (GolfBall().m_playFrame >= 0 && Doc()->m_pGolfDoc->m_gameMode != 0x1000)
	{
		return;
	}
	AfxGetTask()->GetActor("Player")
		<< MsgObject(0, M983(0x124), m_playerIndex, (int)&m_playerMatrix, 0, 0, 0);
	WVector dir = m_playerMatrix.za;
	dir.y = 0.0f;
	dir.Normalize();
	if (GolfBall().m_groundType == 2)
		m_target = m_playerMatrix.pivot - dir * (float)(2.56 + FLT_EPSILON);
	else
		m_target = m_playerMatrix.pivot - dir * (float)(3.84 + FLT_EPSILON);
	m_target.y = GolfBall().m_pos.y + (float)(3.84 + FLT_EPSILON);
	dir = m_target - m_mat.pivot;
	float dist = dir.Magnitude();
	float t = dist * 0.15625f;
	if (t > 1.0f)
	{
		t = 1.0f;
	}
	m_mat.za = dir * t + (GolfBall().m_pos - m_mat.pivot) * (1.0f - t);
	m_mat.za.y = 0.0f;
	m_mat.za.Normalize();
	m_mat.za *= -1.0f;
	m_mat.ya = WVector::UNIT_POS_Y;
	m_mat.xa = WCrossProduct(m_mat.ya, m_mat.za);
	WVector pivot = m_mat.pivot;
	m_mat.pivot = WVector::ZERO;
	m_mat = RotMat(-0.5235988f, 1) * m_mat;
	m_mat.pivot = pivot;
	if (dist < 32.0f + FLT_EPSILON)
	{
		if (dist > 0.5)
			m_mat.pivot += dir * (dist * m_speed) * delta;
	}
	else
	{
		m_mat.pivot = m_target;
	}
}

ntCatmullRom::ntCatmullRom()
	: m_pPoints(NULL),
	  m_pKnots(NULL),
	  m_pLengths(NULL),
	  m_totalLength(0.0f),
	  m_count(0)
{
}

ntCatmullRom::~ntCatmullRom()
{
	Clean();
}

bool ntCatmullRom::Init(const WVector* points, const float* knots, int count)
{
	if (m_count != 0)
	{
		return false;
	}
	if (count <= 3 || points == NULL || knots == NULL)
	{
		return false;
	}

	m_pPoints = new WVector[count];
	m_pKnots = new float[count];
	m_count = count;
	for (unsigned int i = 0; i < (unsigned int)count; i++)
	{
		m_pPoints[i] = points[i];
		m_pKnots[i] = knots[i];
	}

	unsigned int segments = count - 1;
	m_pLengths = new float[segments];
	m_totalLength = 0.0f;
	for (unsigned int i = 0; i < segments; i++)
	{
		m_pLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
		m_totalLength += m_pLengths[i];
	}
	return true;
}

void ntCatmullRom::Clean()
{
	if (m_pPoints)
	{
		delete[] m_pPoints;
		m_pPoints = NULL;
	}
	if (m_pKnots)
	{
		delete[] m_pKnots;
		m_pKnots = NULL;
	}
	if (m_pLengths)
	{
		delete[] m_pLengths;
		m_pLengths = NULL;
	}
	m_totalLength = 0.0f;
	m_count = 0;
}

WVector ntCatmullRom::Evaluate(float t)
{
	if (m_count < 2)
	{
		return WVector::ZERO;
	}
	if (t <= m_pKnots[0])
	{
		return m_pPoints[0];
	}
	if (!(t < m_pKnots[m_count - 1]))
	{
		return m_pPoints[m_count - 1];
	}

	int i = CatmullFindSegment(m_pKnots, m_count, t);
	float u = (t - m_pKnots[i]) / (m_pKnots[i + 1] - m_pKnots[i]);
	float h = u * 0.5f;
	WVector r;
	if (i == 0)
	{
		const WVector* p = m_pPoints;
		r.x = (((p[0].x - (p[1].x + p[1].x)) + p[2].x) * u +
				  ((p[1].x * 4.0f - p[0].x * 3.0f) - p[2].x)) *
				h +
			p[0].x;
		r.y = (((p[0].y - (p[1].y + p[1].y)) + p[2].y) * u +
				  ((p[1].y * 4.0f - p[0].y * 3.0f) - p[2].y)) *
				h +
			p[0].y;
		r.z = (((p[0].z - (p[1].z + p[1].z)) + p[2].z) * u +
				  ((p[1].z * 4.0f - p[0].z * 3.0f) - p[2].z)) *
				h +
			p[0].z;
	}
	else if (i < m_count - 2)
	{
		const WVector* p = m_pPoints + i;
		r.x = (((((p[0].x * 3.0f - p[-1].x) - p[1].x * 3.0f) + p[2].x) * u +
				   ((((p[-1].x + p[-1].x) - p[0].x * 5.0f) + p[1].x * 4.0f) -
					   p[2].x)) *
					  u +
				  (p[1].x - p[-1].x)) *
				h +
			p[0].x;
		r.y = (((((p[0].y * 3.0f - p[-1].y) - p[1].y * 3.0f) + p[2].y) * u +
				   ((((p[-1].y + p[-1].y) - p[0].y * 5.0f) + p[1].y * 4.0f) -
					   p[2].y)) *
					  u +
				  (p[1].y - p[-1].y)) *
				h +
			p[0].y;
		r.z = (((((p[0].z * 3.0f - p[-1].z) - p[1].z * 3.0f) + p[2].z) * u +
				   ((((p[-1].z + p[-1].z) - p[0].z * 5.0f) + p[1].z * 4.0f) -
					   p[2].z)) *
					  u +
				  (p[1].z - p[-1].z)) *
				h +
			p[0].z;
	}
	else
	{
		const WVector* p = m_pPoints + (m_count - 2);
		r.x = (((p[-1].x - (p[0].x + p[0].x)) + p[1].x) * u +
				  (p[1].x - p[-1].x)) *
				h +
			p[0].x;
		r.y = (((p[-1].y - (p[0].y + p[0].y)) + p[1].y) * u +
				  (p[1].y - p[-1].y)) *
				h +
			p[0].y;
		r.z = (((p[-1].z - (p[0].z + p[0].z)) + p[1].z) * u +
				  (p[1].z - p[-1].z)) *
				h +
			p[0].z;
	}
	return r;
}

// B1491: Velocity/Acceleration como o 983 00765630/00765b30: menos de 2 pontos = ZERO e t preso aos nos
// (t <= 1o no vira 0.0, fldz; acima do ultimo = ultimo). Sem isso o segmento saia count-1 e lia m_pKnots[count].
static __forceinline float CatmullClampKnot(const float* knots, int count, float t)
{
	if (!(knots[0] < t))
		return 0.0f;
	if (knots[count - 1] < t)
		return knots[count - 1];
	return t;
}

WVector ntCatmullRom::Velocity(float t)
{
	if (m_count < 2)
	{
		return WVector::ZERO;
	}
	t = CatmullClampKnot(m_pKnots, m_count, t);
	int i = CatmullFindSegment(m_pKnots, m_count, t);
	float u = (t - m_pKnots[i]) / (m_pKnots[i + 1] - m_pKnots[i]);
	WVector r;
	if (i == 0)
	{
		const WVector* p = m_pPoints;
		r.x = ((p[1].x * 4.0f - p[0].x * 3.0f) - p[2].x) * 0.5f +
			((p[0].x - (p[1].x + p[1].x)) + p[2].x) * u;
		r.y = ((p[1].y * 4.0f - p[0].y * 3.0f) - p[2].y) * 0.5f +
			((p[0].y - (p[1].y + p[1].y)) + p[2].y) * u;
		r.z = ((p[1].z * 4.0f - p[0].z * 3.0f) - p[2].z) * 0.5f +
			((p[0].z - (p[1].z + p[1].z)) + p[2].z) * u;
	}
	else if (i < m_count - 2)
	{
		const WVector* p = m_pPoints + i;
		float k = u * 1.5f;
		r.x = (p[1].x - p[-1].x) * 0.5f +
			((((p[0].x * 3.0f - p[-1].x) - p[1].x * 3.0f) + p[2].x) * k +
				((((p[-1].x + p[-1].x) - p[0].x * 5.0f) + p[1].x * 4.0f) -
					p[2].x)) *
				u;
		r.y = (p[1].y - p[-1].y) * 0.5f +
			((((p[0].y * 3.0f - p[-1].y) - p[1].y * 3.0f) + p[2].y) * k +
				((((p[-1].y + p[-1].y) - p[0].y * 5.0f) + p[1].y * 4.0f) -
					p[2].y)) *
				u;
		r.z = (p[1].z - p[-1].z) * 0.5f +
			((((p[0].z * 3.0f - p[-1].z) - p[1].z * 3.0f) + p[2].z) * k +
				((((p[-1].z + p[-1].z) - p[0].z * 5.0f) + p[1].z * 4.0f) -
					p[2].z)) *
				u;
	}
	else
	{
		const WVector* p = m_pPoints + (m_count - 2);
		r.x = (p[1].x - p[-1].x) * 0.5f +
			((p[-1].x - (p[0].x + p[0].x)) + p[1].x) * u;
		r.y = (p[1].y - p[-1].y) * 0.5f +
			((p[-1].y - (p[0].y + p[0].y)) + p[1].y) * u;
		r.z = (p[1].z - p[-1].z) * 0.5f +
			((p[-1].z - (p[0].z + p[0].z)) + p[1].z) * u;
	}
	return r;
}

WVector ntCatmullRom::Acceleration(float t)
{
	if (m_count < 2)
	{
		return WVector::ZERO;
	}
	t = CatmullClampKnot(m_pKnots, m_count, t);
	int i = CatmullFindSegment(m_pKnots, m_count, t);
	WVector r;
	if (i == 0)
	{
		const WVector* p = m_pPoints;
		r.x = (p[0].x - (p[1].x + p[1].x)) + p[2].x;
		r.y = (p[0].y - (p[1].y + p[1].y)) + p[2].y;
		r.z = (p[0].z - (p[1].z + p[1].z)) + p[2].z;
	}
	else if (i < m_count - 2)
	{
		const WVector* p = m_pPoints + i;
		float k = ((t - m_pKnots[i]) / (m_pKnots[i + 1] - m_pKnots[i])) * 3.0f;
		r.x = (((p[0].x * 3.0f - p[-1].x) - p[1].x * 3.0f) + p[2].x) * k +
			((((p[-1].x + p[-1].x) - p[0].x * 5.0f) + p[1].x * 4.0f) - p[2].x);
		r.y = (((p[0].y * 3.0f - p[-1].y) - p[1].y * 3.0f) + p[2].y) * k +
			((((p[-1].y + p[-1].y) - p[0].y * 5.0f) + p[1].y * 4.0f) - p[2].y);
		r.z = (((p[0].z * 3.0f - p[-1].z) - p[1].z * 3.0f) + p[2].z) * k +
			((((p[-1].z + p[-1].z) - p[0].z * 5.0f) + p[1].z * 4.0f) - p[2].z);
	}
	else
	{
		const WVector* p = m_pPoints + (m_count - 2);
		r.x = (p[-1].x - (p[0].x + p[0].x)) + p[1].x;
		r.y = (p[-1].y - (p[0].y + p[0].y)) + p[1].y;
		r.z = (p[-1].z - (p[0].z + p[0].z)) + p[1].z;
	}
	return r;
}

float ntCatmullRom::FindParameterByDistance(float from, float distance)
{
	const float* last = &m_pKnots[m_count - 1];
	if (distance > ArcLength(from, *last))
	{
		return *last;
	}

	float t =
		((m_pKnots[m_count - 1] - m_pKnots[0]) * distance) / m_totalLength +
		from;
	for (int iter = 0; iter < 32; iter++)
	{
		float err = ArcLength(from, t) - distance;
		float mag = err;
		if (*(unsigned int*)&err & 0x80000000)
		{
			mag = -err;
		}
		if (mag < 0.001f)
		{
			return t;
		}
		const WVector& v = Velocity(t);
		t -= err / sqrtf(v.y * v.y + v.x * v.x + v.z * v.z);
	}
	return g_HUGE;
}

float ntCatmullRom::ArcLength(float from, float to)
{
	if (to <= from)
	{
		return 0.0f;
	}
	if (from < m_pKnots[0])
	{
		from = m_pKnots[0];
	}
	if (m_pKnots[m_count - 1] < to)
	{
		to = m_pKnots[m_count - 1];
	}

	int i0 = CatmullFindSegment(m_pKnots, m_count, from);
	float u0 = (from - m_pKnots[i0]) / (m_pKnots[i0 + 1] - m_pKnots[i0]);
	int i1 = CatmullFindSegment(m_pKnots, m_count, to);
	float u1 = (to - m_pKnots[i1]) / (m_pKnots[i1 + 1] - m_pKnots[i1]);

	if (i0 == i1)
	{
		return SegmentArcLength(i0, u0, u1);
	}

	float length = SegmentArcLength(i0, u0, 1.0f);
	for (int i = i0 + 1; i < i1; i++)
	{
		length += m_pLengths[i];
	}
	return SegmentArcLength(i1, 0.0f, u1) + length;
}

float ntCatmullRom::SegmentArcLength(int segment, float from, float to)
{
	// 5-point Gauss-Legendre quadrature
	static const float s_gaussPoint[5] = { 0.0f, 0.538469310105683f,
		-0.538469310105683f, 0.906179845938664f, -0.906179845938664f };
	static const float s_gaussWeight[5] = { 0.568888888888889f,
		0.478628670499366f, 0.478628670499366f, 0.236926885056189f,
		0.236926885056189f };
	if (to <= from)
	{
		return 0.0f;
	}
	if (from < 0.0f)
	{
		from = 0.0f;
	}
	if (to > 1.0f)
	{
		to = 1.0f;
	}

	float ax, ay, az;
	float bx, by, bz;
	float cx, cy, cz;
	if (segment == 0)
	{
		const WVector* p = m_pPoints;
		ax = (p[0].x - (p[1].x + p[1].x)) + p[2].x;
		ay = (p[0].y - (p[1].y + p[1].y)) + p[2].y;
		az = (p[0].z - (p[1].z + p[1].z)) + p[2].z;
		bx = (p[1].x * 4.0f - p[0].x * 3.0f) - p[2].x;
		by = (p[1].y * 4.0f - p[0].y * 3.0f) - p[2].y;
		bz = (p[1].z * 4.0f - p[0].z * 3.0f) - p[2].z;
	}
	else if (segment < m_count - 2)
	{
		const WVector* p = m_pPoints + segment;
		ax = ((p[0].x * 3.0f - p[-1].x) - p[1].x * 3.0f) + p[2].x;
		ay = ((p[0].y * 3.0f - p[-1].y) - p[1].y * 3.0f) + p[2].y;
		az = ((p[0].z * 3.0f - p[-1].z) - p[1].z * 3.0f) + p[2].z;
		bx = (((p[-1].x + p[-1].x) - p[0].x * 5.0f) + p[1].x * 4.0f) - p[2].x;
		by = (((p[-1].y + p[-1].y) - p[0].y * 5.0f) + p[1].y * 4.0f) - p[2].y;
		bz = (((p[-1].z + p[-1].z) - p[0].z * 5.0f) + p[1].z * 4.0f) - p[2].z;
		cx = p[1].x - p[-1].x;
		cy = p[1].y - p[-1].y;
		cz = p[1].z - p[-1].z;
	}
	else
	{
		segment = m_count - 2;
		const WVector* p = m_pPoints + segment;
		ax = (p[-1].x - (p[0].x + p[0].x)) + p[1].x;
		ay = (p[-1].y - (p[0].y + p[0].y)) + p[1].y;
		az = (p[-1].z - (p[0].z + p[0].z)) + p[1].z;
		bx = p[1].x - p[-1].x;
		by = p[1].y - p[-1].y;
		bz = p[1].z - p[-1].z;
	}

	float sum = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		float u = ((to - from) * s_gaussPoint[i] + from + to) * 0.5f;
		float vx, vy, vz;
		if (segment == 0 || segment >= m_count - 2)
		{
			vx = bx * 0.5f + ax * u;
			vy = by * 0.5f + ay * u;
			vz = bz * 0.5f + az * u;
		}
		else
		{
			float k = u * 1.5f;
			vx = cx * 0.5f + (ax * k + bx) * u;
			vy = cy * 0.5f + (ay * k + by) * u;
			vz = cz * 0.5f + (az * k + bz) * u;
		}
		sum += sqrtf(vy * vy + vx * vx + vz * vz) * s_gaussWeight[i];
	}
	return (to - from) * sum * 0.5f;
}
