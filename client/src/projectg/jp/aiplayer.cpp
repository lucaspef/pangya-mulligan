// JP 983: CAIPlayer (sessões B36 a B38). A unidade vai de ~0046d8e0 a 004756f9 (~31 kB): o corte da F3.7.1 (00473f14)
// pegava só o fim (moldes de vetor, construtor, criador, HandleMsg). Escrita inteira: a classe, o cálculo do caminho da bola
// (CalcPath983 00474bb0 e o que ele chama) e o seletor de tacada por tipo que ficou sem chamador (Shot983 00471b60 e os
// cinco tipos). Fonte reconstruída do 983 (classe sem par no 645; par no KR 839).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "aiplayer.h"
#include "shareddoc.h"
#include "contentsdoc.h"
#include "randomhouse.h"
#include "golfruleholeinone.h"
#include "golfdoc.h"
#include "golfball.h"
#include "quadtree.h"
#include "polysoup.h"
#include "nodecontainer.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"


// campos do CSharedDoc e do CGolfDoc que o 983 le por deslocamento; A746: membros (no VS 2022 o Doc anda: m_pGolfDoc 0x11024 -> 0x10cc4)
#define HOLEORDER983(i) (::Doc()->m_holeOrder[i]) // +0x1051e
#define HOLERANDOM983(i) (::Doc()->m_holeRandom[i]) // +0x10534
#define GOLFDOC983() (::Doc()->m_pGolfDoc) // +0x11024
#define CURHOLE983() (GOLFDOC983()->m_currentHole) // CGolfDoc +0x17c
#define HOLEDATA983(hole) (GOLFDOC983()->m_pHoleData[(hole) - 1]) // CGolfDoc +0xd8
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e && offsetof(CSharedDoc, m_holeRandom) == 0x10534 && offsetof(CSharedDoc, m_pGolfDoc) == 0x11024);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c && offsetof(CGolfDoc, m_pHoleData) == 0xd8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.nUserLimit) == 0x10937 && offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594 && offsetof(CSharedDoc, m_golfGame.holes) == 0x10699);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel) + offsetof(sChannelInfo, Type) == 0x10a99);
C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && sizeof(sRivalData) == 0x1b8 && offsetof(sRivalData, bGrandPrixAI983) == 0x1b4);

// 00643a90 (CSharedDoc, 15 bytes): acrescenta o registro ao m_rivalList. Nome meu.
class CSharedDocAIJp
{
public:
	void AddRival983(sRivalData* pRival);
};

// registro de 00a17cb0 (adversário pronto do Grand Prix; o mesmo de dummyplayeractor.cpp). Aqui é copiado inteiro: 0x25c.
struct sAwardNpc983
{
	char m_00[8];
	char name[0x28]; // +0x08
	unsigned long charIndex; // +0x30
	char m_34[0x228];
};
class CItemManagerAwardJp
{
public:
	sAwardNpc983* FindNpc983(unsigned long id); // 00a17cb0
};
#define ITEMMGRAWARDJP() ((CItemManagerAwardJp*)&Doc()->m_itemManager)

// leitura por nome fora de linha (00470e60 int, 00470f20 float: os dois moldes são desta unidade) — ver holeinonemissiondlg.cpp
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
};
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)

struct sAIPlayerLayout983
{
	C_ASSERT_VC10(sizeof(sAIPlayerInfo983) == 0x30);
	C_ASSERT(offsetof(sAIPlayerInfo983, m_bDone) == 0x0a);
	C_ASSERT(offsetof(sAIPlayerInfo983, m_path) == 0x0c);
	C_ASSERT_VC10(offsetof(sAIPlayerInfo983, m_times) == 0x20);
	C_ASSERT(sizeof(sAIDocInfo983) == 0x28);
	C_ASSERT(offsetof(sAIDocInfo983, m_pang) == 0x10);
	C_ASSERT(sizeof(sAwardNpc983) == 0x25c);
	C_ASSERT_VC10(offsetof(sAIPlayerInfo983, m_phase) == 0x1c);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_curHole) == 0x31);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_greenFrame) == 0x3c);
	C_ASSERT(offsetof(sRivalData, state) == 0x36 && offsetof(sRivalData, holePang) == 0x98);
	C_ASSERT(offsetof(sRivalData, totalScore) == 0x14c && offsetof(sRivalData, ballPos) == 0x158);
	C_ASSERT(offsetof(sRivalData, tidChar983) == 0x1ac && offsetof(sRivalData, grandPrixAIId983) == 0x1b6);
	C_ASSERT(sizeof(sHoleData) == 0x38 && offsetof(sHoleData, par) == 0x18);
	C_ASSERT_VC10(sizeof(CAIPlayer) == 0x5c);
	C_ASSERT(offsetof(CAIPlayer, m_count) == 0x18);
	C_ASSERT(offsetof(CAIPlayer, m_players) == 0x20);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_hole) == 0x30);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_bTurn) == 0x38);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_holeInDist) == 0x48);
	C_ASSERT_VC10(offsetof(CAIPlayer, m_pAIDoc) == 0x58);
};

IMPLEMENT_ACTOR(CAIPlayer, IActor)

// 0046da20
void CAIPlayer::Release983()
{
	if (m_pAIDoc)
		m_pAIDoc->Release();
}

// 0046da40: há documento e ele não está ocupado
bool CAIPlayer::CanBuild983()
{
	if (!m_pAIDoc)
		return false;
	return !m_pAIDoc->IsBusy983();
}

// 0046da60
int CAIPlayer::GetInfo983(int index, int hole, sAIDocInfo983* pOut)
{
	return m_pAIDoc->GetInfo983(index, hole, pOut);
}

// 0046da70
bool CAIPlayer::Fn46da70_983(int a, int b)
{
	if (!m_pAIDoc)
		return false;
	return m_pAIDoc->Fn90fe00_983(a, b);
}

// 0046da90
void CAIPlayer::Reset983()
{
	m_done = 0;
	m_time = 0;
}

// 0046daa0
void CAIPlayer::IncDone983()
{
	m_done++;
}

// 0046ebe0
void CAIPlayer::OnPreLoadInit()
{
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa3, m_pAIDoc);
	m_count = m_pAIDoc->GetCount983();
}

// 0046ec10
void CAIPlayer::OnDestroy()
{
	Release983();
}

// 0046ec20
int CAIPlayer::GetInfo08_983(int index, int hole)
{
	sAIDocInfo983 info;
	GetInfo983(index, hole, &info);
	return info.m_08;
}

// 0046ec60
bool CAIPlayer::IsValid983(unsigned int index)
{
	return index <= m_players.size();
}

// 0046ec90
void CAIPlayer::SetDone983(int index, bool bDone)
{
	if (!m_players.empty())
		m_players[index].m_bDone = bDone;
}

// 0046ecd0
bool CAIPlayer::IsAllDone983()
{
	return m_done == m_players.size();
}

// 0046ef50: sorteio de 0 a n - 1 (n = 0 vale 100)
int CAIPlayer::Rand983(unsigned int n)
{
	if (n < 1)
		n = 100;
	return CRandomHouse::Instance()->GetRandom() % n;
}

// 0046f4f0 `GetHoleIndex` é `inline` de shareddoc.inl (frente A): a cópia fora de linha desta unidade sai dela.

// 0046f7e0
void CAIPlayer::OnInit()
{
	m_hole = GetHoleIndex(CURHOLE983()) - 1;
	m_curHole = CURHOLE983();
	Reset983();
}

// 0046fa00: tipo do caminho pelo resultado do buraco (-3 = albatroz … 0 = par)
int CAIPlayer::ShotKind983(int score)
{
	switch (score)
	{
	case -3:
	case -2:
		if (score == -2)
		{
			int r = Rand983(3);
			if (r)
				return r;
			return 2;
		}
		return 1;
	case -1:
	case 0:
		if (score == 0)
		{
			int r = Rand983(4);
			if (r)
				return r;
			break;
		}
		return 2;
	}
	return 3;
}

// 0046fa60: jogadores de verdade no m_rivalList (os de IA ficam no fim)
int CAIPlayer::HumanCount983()
{
	return Doc()->m_rivalList.size() - m_count;
}

// 0046fa90: registro do adversário pronto da IA `index` (não mexe em *pOut se não houver)
void CAIPlayer::GetNpc983(int index, sAwardNpc983* pOut)
{
	int id = m_pAIDoc->GetNpcId983(index);
	if (id)
	{
		sAwardNpc983* pNpc = ITEMMGRAWARDJP()->FindNpc983(id);
		if (pNpc)
			*pOut = *pNpc;
	}
}

// 0046fad0
unsigned long CAIPlayer::GetCharType983(int index)
{
	sAwardNpc983 npc;
	GetNpc983(index, &npc);
	if (npc.charIndex == 0)
		return 0x4000000;
	return npc.charIndex | 0x4000000;
}

// 004701e0: índice do jogador de IA -> número no m_rivalList
int CAIPlayer::HumanIndex983(int index)
{
	return HumanCount983() + index;
}

// 004701f0: número do jogador na sala -> índice do jogador de IA
int CAIPlayer::ToIndex983(int player)
{
	return player - HumanCount983();
}

// 00470210
void CAIPlayer::Start983()
{
	int hole = m_hole;
	for (unsigned int i = 0; i < Doc()->m_rivalList.size(); ++i)
	{
		if (Doc()->m_rivalList[i].bGrandPrixAI983)
		{
			sAIDocInfo983 info;
			GetInfo983(ToIndex983(i), hole, &info);
			Doc()->m_rivalList[i].state = 0;
			Doc()->m_rivalList[i].holePang[hole] = info.m_pang;
			Doc()->m_rivalList[i].totalBonusPang += info.m_bonusPang;
			Doc()->m_rivalList[i].holeScore[hole] = info.m_08;
		}
	}
}

// 00470380
void CAIPlayer::Fn470380_983(int index)
{
	unsigned char hole = m_hole;
	int player = HumanIndex983(index);
	int i = 0;
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it, ++i)
	{
		if (i == player && it->bGrandPrixAI983)
		{
			it->totalScore = 0;
			it->totalPang = 0;
			for (int k = 0; k < hole + 1; ++k)
			{
				it->totalScore += it->holeScore[k];
				it->totalPang += it->holePang[k];
			}
			it->state = 1;
			return;
		}
	}
}

// 00470420: quadros do voo da bola mandados ao "Spot". No 983 os dois ramos dão m_greenFrame (o m_outFrame não é lido aqui).
int CAIPlayer::FlyFrame983(const WVector* pPos)
{
	if (IsOnGreen983(pPos))
		m_frame = m_greenFrame;
	else
		m_frame = m_greenFrame;
	return m_frame;
}

// 00471170
void CAIPlayer::Fn471170_983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("grandprix_ai_player.lua");
	lua_system::lua_table_helper tbl = script->GetTable("ai_player_pos_config");
	m_greenFrame = LUA983_GET(tbl, int, "on_green_ball_fly_speed_frame");
	m_outFrame = LUA983_GET(tbl, int, "outside_green_ball_fly_speed_frame");
	m_holeInDist = LUA983_GET(tbl, float, "HoleIn_Valid_Distance");
	m_greenFailRate = LUA983_GET(tbl, int, "on_green_fail_rate");
	m_holeInFailRate = LUA983_GET(tbl, int, "in_HoleIn_Valid_dist_fail_rate");
	m_cupPassRate = LUA983_GET(tbl, int, "HoleCupPassRate");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
}

// 004713b0: a IA `index` chegou ao buraco — última animação do "Spot" (0x122) e a bola vai para o pino
void CAIPlayer::Arrive983(int index)
{
	if (m_players.empty())
		return;
	m_players[index].m_state = 2;
	unsigned char hole = m_curHole;
	int frame = FlyFrame983(&Doc()->m_rivalList[HumanIndex983(index)].ballPos);
	GetActor("Spot") << MsgObject(NULL, 0x122, HumanIndex983(index), hole, frame, 0, 0);
	const WVector* pPin = &HOLEDATA983(m_curHole).pin;
	Doc()->m_rivalList[HumanIndex983(index)].ballPos = *pPin;
}

// 004714b0: fim de uma tacada da IA `index` — passa para a seguinte (0x122 ao "Spot") ou chega ao buraco
void CAIPlayer::NextStep983(int index)
{
	if (m_players.empty())
		return;
	int player = HumanIndex983(index);
	m_players[index].m_step++;
	sAIPlayerInfo983& info = m_players[index];
	if (info.m_step == (unsigned short)info.m_path.size())
	{
		Arrive983(index);
		return;
	}
	m_players[index].m_state = 0;
	WVector pos;
	sAIPlayerInfo983& next = m_players[index];
	unsigned int step = next.m_step;
	if (step < next.m_path.size())
		pos = next.m_path[step];
	unsigned char hole = m_curHole;
	int frame = FlyFrame983(&Doc()->m_rivalList[player].ballPos);
	GetActor("Spot") << MsgObject(NULL, 0x122, player, hole, frame, 0, 0);
	Doc()->m_rivalList[player].ballPos = pos;
}

// 004715f0: o jogador de IA `index` terminou a vez
void CAIPlayer::Finish983(int index)
{
	if (IsValid983(index))
	{
		SetDone983(index, true);
		IncDone983();
		Fn470380_983(index);
		if (IsAllDone983())
			m_bTurn = false;
	}
}

// 00471980
void CAIPlayer::OnLoad()
{
	Fn471170_983();
	m_pAIDoc->Fn911180_983();
}

// 004719a0: relógio das tacadas. O 983 lê `t` sem valor quando o m_times está vazio (copiado como está).
void CAIPlayer::OnProcess(float delta)
{
	if (!m_bTurn || m_players.empty())
		return;
	delta /= g_pMainFrame983->m_gameSpeed;
	sAITime983 t;
	for (unsigned int i = 0; i < m_players.size(); ++i)
	{
		sAIPlayerInfo983& info = m_players[i];
		if (info.m_state == 1)
		{
			if (!info.m_times.empty())
				t = info.m_times[info.m_step];
			switch (info.m_phase)
			{
			case 0:
				if (0.0f != m_time && 0.0f != t.y)
					info.m_phase = 1;
				break;
			case 1:
				if (m_time > t.y)
					info.m_phase = 2;
				break;
			case 2:
				if (t.z + t.y < m_time)
				{
					info.m_phase = 0;
					NextStep983(i);
				}
				break;
			}
		}
	}
	m_time += delta;
}

// 00471ae0: nome do adversário pronto ou "AIPlayerNN"
void CAIPlayer::GetName983(int index, std::string* pName)
{
	sAwardNpc983 npc;
	GetNpc983(index, &npc);
	if (strlen(npc.name) == 0)
		*pName = MakeStr("AIPlayer%02d", index);
	else
		*pName = npc.name;
}

// 00471e50
void CAIPlayer::Fn471e50_983()
{
	for (unsigned int i = 0; i < (unsigned int)m_count; ++i)
	{
		sRivalData rival;
		rival.hole = HOLEORDER983(0);
		rival.rank = 1;
		rival.state = 0;
		rival.totalStroke = 0;
		rival.totalScore = 0;
		rival.totalPang = 0;
		rival.totalBonusPang = 0;
		rival.uid = 0;
		rival.oid = 0;
		rival.ballPos = HOLEDATA983(CURHOLE983()).tee;
		rival.observerState = 1;
		rival.observerRank = 1;
		rival.titleTypeId = 0;
		rival.team = 0;
		rival.flag = 0;
		memset(rival.holeStroke, 0, sizeof(rival.holeStroke));
		memset(rival.holeScore, 0, sizeof(rival.holeScore));
		memset(rival.holePang, 0, sizeof(rival.holePang));
		rival.order = (unsigned char)(Doc()->m_rivalList.size() + 1);
		rival.quitOrder = ::Doc()->m_roomInfo.nUserLimit;
		rival.finishOrder = ::Doc()->m_roomInfo.nUserLimit;
		rival.level = 0;
		std::string name;
		GetName983(i, &name);
		strcpy(rival.nickname, name.c_str());
		strcpy(rival.guildName, "no guild");
		rival.guildUID = 0;
		rival.guildEmblemVer983 = 1;
		strcpy(rival.guildMark, "no emblem");
		rival.guildPoint = 0;
		rival.capability = 0;
		rival.mascotTypeId = 0;
		rival.pangMastery = 0;
		rival.pangNitro = 0;
		rival.tidChar983 = GetCharType983(i);
		rival.rankTieBreak983 = 0.0f;
		rival.bGrandPrixAI983 = 1;
		rival.grandPrixAIId983 = (unsigned short)m_pAIDoc->GetNpcId983(i);
		((CSharedDocAIJp*)Doc())->AddRival983(&rival);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Cálculo do caminho (sessão B38). O mapa traz linhas de pontos "*nlNN" (NN = tipo sorteado pelo ShotKind983): uma é
// sorteada, cada ponto dela é espalhado em volta, e o fim é ajustado ao número de tacadas (pontos a mais em volta do pino).

extern const WVector g_vZero983_f041c4; // 00f041c4

// 0046e750: tacadas a menos pelo par do buraco
unsigned char ParExtra983(unsigned char par)
{
	switch (par)
	{
	case 4:
		return 1;
	case 5:
		return 2;
	}
	return 0;
}

// 0046f540: medida na escala do campo (mapa, tipo do canal, valor).
float __fastcall Scale983(int map, unsigned long channelType, float value)
{
	if (map == 0xb)
		value = ::Doc()->m_courseScale983[0] * value;
	else
		value = (float)(value * 3.0);
	if (IsLocalContent((localContentType_t)0x3b) && (channelType & 0x800) && map != 0xf && map != 0x10)
		value += value;
	if ((unsigned int)(::Doc()->m_roomInfo.realGameType - 0x14) <= 1)
		value = ::Doc()->m_courseScale983[2] * value;
	return value;
}

// 00470040 (embutida aqui; o CGolfTask::OnPacket chama a cópia fora de linha em 006a0d46 — corpo na pktviews983.cpp da C): a medida na escala do campo corrente
inline float CourseScale983(float value)
{
	return Scale983(::Doc()->m_golfGame.gameType, ::Doc()->m_curChannel.Type, value);
}

// 0046ecf0: último ponto da linha
bool CAIPlayer::LastNode983(CNodeContainer* pNode, WVector* pOut)
{
	const std::vector<WVector>& list = pNode->GetList();
	if (list.empty())
		return false;
	*pOut = list[list.size() - 1];
	return true;
}

// 00472660
std::string LineName983(int kind)
{
	return std::string(MakeStr("*nl%02d", kind));
}

// 0046ed40
float CAIPlayer::Dist983(WVector a, WVector b)
{
	WVector d(b.x - a.x, 0.0f, b.z - a.z);
	return d.Magnitude();
}

// 0046ed90
void CAIPlayer::Dir983(const WVector* pFrom, const WVector* pTo, WVector* pOut)
{
	WVector d;
	d.x = pTo->x - pFrom->x;
	d.y = pTo->y - pFrom->y;
	d.z = pTo->z - pFrom->z;
	pOut->x = d.x;
	pOut->z = d.z;
	pOut->y = 0.0f;
	pOut->Normalize();
}

// 0046ede0 (sem chamador vivo)
float CAIPlayer::Reach983(const WVector* pFrom, const WVector* pTo, const WVector* pTarget)
{
	float dist = Dist983(*pFrom, *pTo);
	WVector dir = g_vZero983_f041c4;
	WVector zero = g_vZero983_f041c4;
	Dir983(pFrom, pTo, &dir);
	Dir983(pFrom, pTarget, &dir);
	float raw = CalcDeltaAngle(dir, zero);
	float angle = (float)(raw * 57.295779513082323);
	if (angle <= 0.0f)
		angle = -angle;
	if (0.0f == raw)
		return dist;
	if (0.0f > raw)
		return (float)(dist / (float)cos(raw));
	angle = (float)(180.0 - (angle + 90.0));
	if (angle <= 0.0f)
		angle = -angle;
	return (float)(dist / (float)sin((float)(angle * 0.017453292519943295)));
}

// 0046f840: dentro do campo e fora do buraco/obstáculo de tipo 10
bool CAIPlayer::IsFair983(const WVector* pPos)
{
	if (!GOLFDOC()->m_pPolySoup->IsOutOfBound(*pPos))
	{
		CGolfBall::eWater attr = (CGolfBall::eWater)0;
		if (GetPVS().GetGroundType(*pPos, false, &attr) != 10 || (attr != 0 && attr != 1))
			return true;
	}
	return false;
}

// 0046f8b0: gira no plano xz de um múltiplo de 18 graus. No 983 o sinal é sempre -1 (Rand983(1) dá 0).
void CAIPlayer::Turn983(const WVector* pDir, WVector* pOut)
{
	int step = Rand983(100);
	float sign = Rand983(1) ? 1.0f : -1.0f;
	float deg = (float)(step * 18) * sign;
	float rad = (float)(deg * 0.017453292519943295);
	float c = (float)cos(rad);
	float s = (float)sin(rad);
	pOut->x = c * pDir->x - s * pDir->z;
	pOut->z = c * pDir->z + pDir->x * s;
	pOut->y = 0.0f;
	pOut->Normalize();
}

// 0046f950: ponto a `dist` do centro, na direção centro ← origem girada
void CAIPlayer::Around983(const WVector* pCenter, const WVector* pFrom, float dist, WVector* pOut)
{
	WVector dir = g_vZero983_f041c4;
	Dir983(pFrom, pCenter, &dir);
	WVector turned = g_vZero983_f041c4;
	Turn983(&dir, &turned);
	pOut->x = pCenter->x + dist * turned.x;
	pOut->y = pCenter->y + turned.y * dist;
	pOut->z = pCenter->z + dist * turned.z;
}

// 0046fb10 (sem chamador vivo)
bool CAIPlayer::GreenFail983(int strokes, float dist)
{
	int r = Rand983(100);
	if (strokes > 1 && r > m_greenFailRate)
	{
		if (dist > 96.0f)
			return false;
	}
	return true;
}

// 0046fb50 (sem chamador vivo)
bool CAIPlayer::HoleInFail983()
{
	return Rand983(100) <= m_holeInFailRate;
}

// 0046fb70 (sem chamador vivo)
bool CAIPlayer::CupPass983(float dist)
{
	if (Rand983(100) <= m_cupPassRate && dist >= 9.6f)
		return true;
	return false;
}

// 0046fba0
const WVector* CAIPlayer::Pin983()
{
	return &GOLFDOC()->GetHoleData().pin;
}

// 0046fbd0: o chão é green (tipo 2)
bool CAIPlayer::IsOnGreen983(const WVector* pPos)
{
	if (!GOLFDOC()->m_pPolySoup->IsOutOfBound(*pPos))
	{
		CGolfBall::eWater attr = (CGolfBall::eWater)0;
		switch (GetPVS().GetGroundType(*pPos, false, &attr))
		{
		case 2:
			return true;
		case 10:
			if (attr != 0 && attr != 1)
				return false;
			break;
		}
	}
	return false;
}

// 0046fc50 (sem chamador vivo)
bool CAIPlayer::IsOut983(const WVector* pPos)
{
	if (GOLFDOC()->m_pPolySoup->IsOutOfBound(*pPos))
		return true;
	CGolfBall::eWater attr = (CGolfBall::eWater)0;
	return GetPVS().GetGroundType(*pPos, false, &attr) == 10;
}

// 0046fcb0 (sem chamador vivo)
int CAIPlayer::RandRange983(int lo, int hi)
{
	int r = Rand983(hi);
	if (r < lo)
		return lo;
	if (r > hi)
		r = hi;
	return r;
}

// 00470150: o ponto da linha deslocado de `dist` numa direção sorteada
void CAIPlayer::Spread983(const WVector* pPin, const WVector* pNode, float dist, WVector* pOut)
{
	*pOut = *pNode;
	WVector dir;
	dir.x = pPin->x - pNode->x;
	dir.z = pPin->z - pNode->z;
	dir.y = 0.0f;
	dir.Normalize();
	WVector turned;
	Turn983(&dir, &turned);
	pOut->x += dist * turned.x;
	pOut->y += turned.y * dist;
	pOut->z += dist * turned.z;
}

// 00470440 (sem chamador vivo): o alvo fica a 90 graus ou mais da origem, vistos do pino
bool CAIPlayer::IsBehind983(const WVector* pTarget, const WVector* pFrom)
{
	WVector a = g_vZero983_f041c4;
	WVector b = g_vZero983_f041c4;
	Dir983(Pin983(), pFrom, &a);
	Dir983(Pin983(), pTarget, &b);
	float angle = (float)(CalcDeltaAngle(b, a) * 57.295779513082323);
	if (angle <= 0.0f)
		angle = -angle;
	return angle >= 90.0f;
}

// 004704f0 (sem chamador vivo)
float CAIPlayer::BallSize983()
{
	return (float)(2.0 * (float)(CourseScale983(GolfBall().GetRadius()) * 2.0));
}

// 004705a0 (sem chamador vivo): 0 entra, 1 passa do buraco, 2 e 3 tacada ao green, 4 tacada comum
int CAIPlayer::ShotResult983(const WVector* pPos, unsigned short strokes, float dist)
{
	if (!IsOnGreen983(pPos) && 32.0f <= dist)
	{
		if (GreenFail983(strokes, dist))
			return HoleInFail983() ? 2 : 3;
		return 4;
	}
	return CupPass983(dist) ? 0 : 1;
}

// 00470620 (sem chamador vivo)
void CAIPlayer::ShotLong983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut)
{
	if (IsBehind983(pTarget, pFrom))
	{
		*pOut = *pTarget;
		return;
	}
	float reach = Reach983(pFrom, Pin983(), pTarget);
	float dist = Dist983(*pFrom, *pTarget);
	if (dist < reach)
	{
		float size = CourseScale983(GolfBall().GetRadius()) * 2.0f;
		float f = (float)(unsigned long)Rand983(n);
		if (f < 1.0f)
			f = 1.0f;
		dist = f * size + reach;
	}
	WVector dir = g_vZero983_f041c4;
	Dir983(pFrom, pTarget, &dir);
	pOut->x = pFrom->x + dist * dir.x;
	pOut->y = pFrom->y + dir.y * dist;
	pOut->z = pFrom->z + dist * dir.z;
}

// 00470800 (sem chamador vivo)
void CAIPlayer::ShotShort983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut)
{
	if (!IsBehind983(pTarget, pFrom))
	{
		*pOut = *pTarget;
		return;
	}
	float reach = Reach983(pFrom, Pin983(), pTarget);
	float dist = Dist983(*pFrom, *pTarget);
	if (reach < dist)
	{
		float size = CourseScale983(GolfBall().GetRadius()) * 2.0f;
		Rand983(n);
		dist = reach - (float)(unsigned long)Rand983(n) * size;
	}
	WVector dir = g_vZero983_f041c4;
	Dir983(pFrom, pTarget, &dir);
	pOut->x = pFrom->x + dist * dir.x;
	pOut->y = pFrom->y + dir.y * dist;
	pOut->z = pFrom->z + dist * dir.z;
}

// 004709e0 (sem chamador vivo)
void CAIPlayer::ShotNearPin983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut)
{
	float dist = Dist983(*Pin983(), *pTarget);
	float limit = (float)(m_holeInDist * (double)3.2f);
	float f;
	if (dist <= limit)
	{
		f = (float)(unsigned long)Rand983((unsigned long)(limit - dist)) + dist;
	}
	else
	{
		float size = CourseScale983(GolfBall().GetRadius()) * 2.0f;
		f = (float)(unsigned long)Rand983(n);
		if (f >= 1.0f)
			f = m_holeInDist - f * size;
		else
			f = m_holeInDist - size * 1.0f;
	}
	WVector dir = g_vZero983_f041c4;
	Dir983(Pin983(), pTarget, &dir);
	WVector off;
	off.x = f * dir.x;
	off.y = dir.y * f;
	off.z = f * dir.z;
	const WVector* pPin = Pin983();
	pOut->x = pPin->x + off.x;
	pOut->y = pPin->y + off.y;
	pOut->z = pPin->z + off.z;
}

// 00470bf0 (sem chamador vivo): anda do pino para fora até sair do green e segue até 32 adiante ou até o fim do campo
void CAIPlayer::ShotOut983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut)
{
	WVector dir;
	Dir983(Pin983(), pTarget, &dir);
	WVector pos = *Pin983();
	WVector edge = g_vZero983_f041c4;
	unsigned int i;
	for (i = 0; i < 100; ++i)
	{
		pos.x += dir.x * 6.4f;
		pos.y += dir.y * 6.4f;
		pos.z += dir.z * 6.4f;
		if (!IsOnGreen983(&pos))
			edge = pos;
	}
	WVector prev = g_vZero983_f041c4;
	for (i = 0; i < 100; ++i)
	{
		edge.x += dir.x * 6.4f;
		edge.y += dir.y * 6.4f;
		edge.z += dir.z * 6.4f;
		if (IsOut983(&edge))
		{
			*pOut = prev;
			return;
		}
		if (32.0f < Dist983(edge, pos))
		{
			*pOut = edge;
			return;
		}
		prev = edge;
	}
}

// 00470da0 (sem chamador vivo): último ponto ainda no green, do pino na direção dada
void CAIPlayer::GreenEdge983(const WVector* pDir, WVector* pOut)
{
	WVector prev = g_vZero983_f041c4;
	WVector pos = *Pin983();
	for (unsigned int i = 0; i < 100; ++i)
	{
		pos.x += pDir->x * 3.2f;
		pos.y += pDir->y * 3.2f;
		pos.z += pDir->z * 3.2f;
		if (!IsOnGreen983(&pos))
		{
			*pOut = prev;
			return;
		}
		prev = pos;
	}
}

// 00471260: espalha um ponto da linha (até raio da bola + 32, mais 16)
void CAIPlayer::Jitter983(const WVector* pNode, WVector* pOut)
{
	unsigned long range = (unsigned long)(float)(GolfBall().GetRadius() + 32.0);
	int dist = (int)((float)(unsigned long)Rand983(range) + 16.0);
	WVector pin = *Pin983();
	WVector pos = g_vZero983_f041c4;
	Spread983(&pin, pNode, (float)dist, &pos);
	*pOut = pos;
}

// 00471630 (sem chamador vivo)
void CAIPlayer::ShotGreenEdge983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut)
{
	float dist = Dist983(*Pin983(), *pTarget);
	float limit = (float)(m_holeInDist * (double)3.2f);
	WVector dir = g_vZero983_f041c4;
	Dir983(Pin983(), pTarget, &dir);
	WVector edge = g_vZero983_f041c4;
	GreenEdge983(&dir, &edge);
	float edgeDist = Dist983(*Pin983(), edge);
	if (edgeDist < (float)(m_holeInDist * (double)3.2f))
		limit = edgeDist;
	float f = dist;
	if (dist < limit)
	{
		float size = CourseScale983(GolfBall().GetRadius()) * 2.0f;
		float g = (float)(unsigned long)Rand983(n);
		if (g < 1.0f)
			g = 1.0f;
		f = g * size + limit;
	}
	WVector off;
	off.x = f * dir.x;
	off.y = dir.y * f;
	off.z = f * dir.z;
	const WVector* pPin = Pin983();
	pOut->x = pPin->x + off.x;
	pOut->y = pPin->y + off.y;
	pOut->z = pPin->z + off.z;
}

// 00471b60 (sem chamador vivo): uma tacada de `from` para `to` conforme o tipo
bool CAIPlayer::Shot983(WVector from, WVector to, int n, int kind, WVector* pOut)
{
	WVector target = g_vZero983_f041c4;
	float dist = Dist983(from, to);
	float size = BallSize983();
	float range = size * (float)n;
	if ((float)(0.5 * dist) < range)
		range = (float)(0.5 * dist);
	if (kind == 0 || kind == 1 || dist < 96.0f)
		range = size;
	float f = (float)RandRange983((int)size, (int)range);
	Around983(Pin983(), &to, f, &target);
	switch (kind)
	{
	case 0:
		ShotLong983(&target, &to, n, pOut);
		return true;
	case 1:
		ShotShort983(&target, &to, n, pOut);
		return true;
	case 2:
		ShotNearPin983(&target, &to, n, pOut);
		return true;
	case 3:
		ShotGreenEdge983(&target, &to, n, pOut);
		return true;
	case 4:
		ShotOut983(&target, &to, n, pOut);
		break;
	}
	return true;
}

// 00473500: instantes das tacadas dentro dos `frame` da vez
void sAIPlayerInfo983::MakeTimes983(int strokes, float frame)
{
	std::vector<sAITime983> unused;
	sAITime983 t;
	int i = 0;
	if (strokes > 0)
	{
		t.x = frame / strokes;
		double third = t.x / 3.0;
		do
		{
			t.y = (float)(i++ * t.x + third);
			float rest = t.x * i - t.y;
			int limit = (int)rest;
			int r = rand() % limit;
			t.z = (float)r + t.y;
			m_times.push_back(t);
		} while (i < strokes);
	}
}

// 004736a0: copia a linha espalhando cada ponto (o ponto fica como está se o espalhado cair fora)
bool CAIPlayer::JitterNodes983(CNodeContainer* pSrc, CNodeContainer* pDst)
{
	if (pSrc->GetList().empty())
		return false;
	for (unsigned int i = 0; i < pSrc->GetList().size(); ++i)
	{
		WVector pos;
		Jitter983(&pSrc->GetList()[i], &pos);
		if (!IsFair983(&pos))
			pos = pSrc->GetList()[i];
		pDst->Add(pos);
	}
	return !pDst->GetList().empty();
}

// 00473780: par 4 ou 5 no modo de m_b195_983 — os dois últimos pontos da linha e `count - 2` tacadas em volta do pino
bool CAIPlayer::Approach983(int count, CNodeContainer* pSrc, CNodeContainer* pDst)
{
	int size = (int)pSrc->GetList().size();
	if (count < 2)
	{
		pDst->Add(pSrc->GetList()[size - 1]);
		return true;
	}
	for (int j = 0; j < 2; ++j)
		pDst->Add(pSrc->GetList()[size - 2 + j]);
	count -= 2;
	if (count)
	{
		WVector from = pSrc->GetList()[size - 2];
		for (int i = 0; i < count; ++i)
		{
			WVector pos = g_vZero983_f041c4;
			int k = count - (unsigned short)(i + 1);
			double base = k * 16.0;
			float dist = (float)((float)(unsigned long)Rand983((unsigned long)(float)(base - (k - 1) * 16.0)) + base);
			Around983(Pin983(), &from, dist, &pos);
			IsFair983(&pos);
			pDst->Add(pos);
		}
	}
	return true;
}

// 004739c0
bool CAIPlayer::CopyNodes983(CNodeContainer* pSrc, CNodeContainer* pDst, int count)
{
	if (pSrc->GetList().empty())
		return false;
	for (int i = 0; i < count; ++i)
		pDst->Add(pSrc->GetList()[i]);
	return !pDst->GetList().empty();
}

// 00473a40: a linha menos o último ponto e `extra` tacadas em volta do pino
bool CAIPlayer::Extend983(int extra, int hole, CNodeContainer* pSrc, CNodeContainer* pDst)
{
	int count = (int)pSrc->GetList().size() - 1;
	for (int j = 0; j < count; ++j)
		pDst->Add(pSrc->GetList()[j]);
	WVector from = g_vZero983_f041c4;
	if (!LastNode983(pDst, &from))
		return false;
	for (int i = 0; i < extra; ++i)
	{
		WVector pos = g_vZero983_f041c4;
		int k = extra - (unsigned short)(i + 1);
		double base = k * 16.0;
		float dist = (float)((float)(unsigned long)Rand983((unsigned long)(float)(base - (k - 1) * 16.0)) + base);
		Around983(Pin983(), &from, dist, &pos);
		IsFair983(&pos);
		pDst->Add(pos);
	}
	return true;
}

// 004740f0: ajusta a linha ao número de tacadas
bool CAIPlayer::MakePath983(int hole, int strokes, CNodeContainer* pSrc, CNodeContainer* pDst)
{
	int size = (int)pSrc->GetList().size();
	if (GOLFDOC()->m_b195_983 == true && GOLFDOC()->GetHoleData().par != 3)
		return Approach983(strokes - ParExtra983(GOLFDOC()->GetHoleData().par), pSrc, pDst);
	if (size - 2 < strokes - 1)
	{
		CNodeContainer node("");
		if (!Extend983(strokes - 1 - (size - 2), hole, pSrc, &node))
			return false;
		if (!CopyNodes983(&node, pDst, (int)node.GetList().size()))
			return false;
	}
	else if (size - 2 > strokes - 1)
	{
		if (!CopyNodes983(pSrc, pDst, strokes))
			return false;
	}
	else
	{
		if (!CopyNodes983(pSrc, pDst, size - 1))
			return false;
	}
	return true;
}

// 004747c0: sorteia uma das linhas "*nlNN" do tipo pedido
bool CAIPlayer::PickLine983(int hole, int kind, int strokes, std::vector<CNodeContainer>* pNodes, CNodeContainer* pOut)
{
	std::vector<CNodeContainer> found;
	std::string prefix = LineName983(kind);
	CNodeContainer node("");
	for (unsigned int i = 0; i < pNodes->size(); ++i)
	{
		if ((*pNodes)[i].GetName().find(prefix.c_str()) != std::string::npos)
			found.push_back((*pNodes)[i]);
	}
	if (found.empty())
		return false;
	unsigned short pick = (unsigned short)Rand983(found.size());
	*pOut = found[pick];
	return true;
}

// 00474bb0: caminho de uma IA no buraco — as linhas vêm do documento (0090fe00)
bool CAIPlayer::CalcPath983(int hole, int kind, int strokes, std::vector<WVector>* pPath)
{
	std::vector<CNodeContainer> nodes;
	if (!Fn46da70_983(hole, (int)&nodes))
		return false;
	CNodeContainer line("");
	if (!PickLine983(hole, kind, strokes, &nodes, &line))
		return false;
	CNodeContainer spread("");
	if (!JitterNodes983(&line, &spread))
		return false;
	CNodeContainer path("");
	if (!MakePath983(hole, strokes, &spread, &path))
		return false;
	for (unsigned int i = 0; i < path.GetList().size(); ++i)
		pPath->push_back(path.GetList()[i]);
	return pPath->begin() != pPath->end();
}

// 004752f0: monta os jogadores de IA do buraco `hole` (0…): sorteio com a semente do buraco, caminho e tempos de cada um
void CAIPlayer::Build983(int hole)
{
	if (!m_players.empty())
	{
		m_players.clear();
		m_players.reserve(m_count);
	}
	CRandomHouse::Instance()->SetRandomSeed(HOLERANDOM983(hole));
	for (unsigned int i = 0; i < (unsigned int)m_count; ++i)
	{
		sAIPlayerInfo983 info;
		info.m_index = i;
		info.m_state = 1;
		int score = GetInfo08_983(i, hole);
		int kind = ShotKind983(score);
		int strokes = HOLEDATA983(CURHOLE983()).par + score;
		std::vector<WVector> path;
		if (CalcPath983(hole, kind, strokes, &path))
		{
			if (info.m_path.size())
				info.m_path.clear();
			info.m_path = path;
			info.MakeTimes983(strokes, 60.0f);
		}
		m_players.push_back(info);
	}
}

CAIPlayer::CAIPlayer()
	: m_count(0), m_time(0), m_bTurn(false), m_greenFrame(0), m_outFrame(0), m_frame(0), m_holeInDist(0), m_greenFailRate(0), m_holeInFailRate(0), m_cupPassRate(0), m_pAIDoc(NULL)
{
}

CAIPlayer::~CAIPlayer()
{
}

// 00475560: mensagens 0x346–0x350
void CAIPlayer::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x346: // buraco novo: monta os jogadores de IA
	{
		unsigned int hole = GetHoleIndex((unsigned char)msg.param1) - 1;
		if (hole < ::Doc()->m_golfGame.holes && CanBuild983())
			Build983(hole);
		break;
	}
	case 0x347:
		m_bTurn = true;
	case 0x34c:
		Start983();
		break;
	case 0x348:
		if (!m_players.empty())
			m_players[ToIndex983(msg.param1)].m_state = 1;
		break;
	case 0x349:
		*(int*)msg.param1 = m_count;
		break;
	case 0x34a:
		GetInfo983(msg.param1, m_hole, (sAIDocInfo983*)msg.param2);
		break;
	case 0x34b:
		if (m_pAIDoc)
			m_pAIDoc->Fn90d900_983(3, msg.param1);
		break;
	case 0x34d: // o jogador de IA `param1` acabou a tacada: avisa o "Spot" e fecha a vez dele
	{
		int player = msg.param1;
		GetActor("Spot") << MsgObject(NULL, 0x123, player, ::Doc()->m_holeOrder[1 + m_hole], 0, 0, 0);
		Finish983(ToIndex983(player));
		break;
	}
	case 0x34e:
		*(int*)msg.param1 = m_done;
		break;
	case 0x34f: // byte +0x1b4 do registro `param1` do m_rivalList (CSharedDoc+0x1042c, 0x1b8 por registro)
		*(bool*)msg.param2 = ::Doc()->m_rivalList[msg.param1].bGrandPrixAI983 != 0;
		break;
	case 0x350:
		Fn471e50_983();
		break;
	}
}
