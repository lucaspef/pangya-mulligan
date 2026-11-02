// JP 983: unidade 0064b1a0–0064f1a0 (ShotTrajectoryForMassGame). Classe nova do 983. A simulação (`BuildTrajectory`) é a de
// `ShotPreview::Predict` (shotpreview.cpp da frente A) com os dados vindos do registro da tacada em vez da barra de força; o
// desenho da linha é o de `ShotPreview::Display`. Deslocamentos e constantes medidos no exe 983.
#include "minatl.h"
#include "projectg.h"
#include "shottrajectoryformassgame.h"
#include "subgolfrule.h"
#include "golfdoc.h"
#include "shareddoc.h"
#include "quadtree.h"
#include "wind.h"
#include "club.h"
#include "jp/golfdocjp.h"
#include "fx.h"
#include "fxsequence.h"
#include "woverlay.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"

extern WMatrix g_camera;

// campos do CGolfDoc/CSharedDoc/CQuadTree/CFxSequence do 983 pelos membros (offsets VC10 = 983 conferidos abaixo; A740)
#define GOLFDOC983_GAMEMODE() (GOLFDOC()->m_gameMode)
// pino do buraco da vez: registros de 0x38 bytes em CGolfDoc+0xd8 (+0x0c = pin), índice a partir de 1
#define GOLFDOC983_PLAYERPOS() (&GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].pin)
// quadro da simulação do terreno (CQuadTree+0x21b8 tempo do quadro, +0x21bc quadro)
#define PVS983_FRAMETIME() (GetPVS().m_frameTime)
#define PVS983_FRAME() (GetPVS().m_frame)
// 0064b220 no 983 (cópia fora de linha; aqui embutida): zera o tempo do quadro e põe o quadro
inline void Pvs983SetFrame(unsigned long frame)
{
	CQuadTree& pvs = GetPVS();
	pvs.m_frameTime = 0.0f;
	pvs.m_frame = frame;
}
// posição do efeito (CFxSequence+0x110c)
#define FXSEQ983_POS(seq) ((seq)->m_pos)
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8 && offsetof(CGolfDoc, m_gameMode) == 0x158);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c && sizeof(sHoleData) == 0x38 && offsetof(sHoleData, pin) == 0x0c);
C_ASSERT_VC10(offsetof(CQuadTree, m_frameTime) == 0x21b8 && offsetof(CQuadTree, m_frame) == 0x21bc);
C_ASSERT_VC10(offsetof(CFxSequence, m_pos) == 0x110c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame) + offsetof(sGolfGame, gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel) + offsetof(sChannelInfo, Type) == 0x10a99);
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused47d0) == 0x10704); // A744: ex-*((const bool*)Doc() + 0x10704), "tem compasso"

IMPLEMENT_ACTOR(ShotTrajectoryForMassGame, IActor)

void ShotTrajectoryForMassGame::OnLoad()
{
}

void ShotTrajectoryForMassGame::OnReload()
{
	OnLoad();
}

void ShotTrajectoryForMassGame::sShotFx::Close()
{
	if (pSeq)
	{
		CFx::Instance()->CloseSequence(pSeq, false);
		if (pSeq)
		{
			delete pSeq;
			pSeq = NULL;
		}
		pSeq = NULL;
	}
	bActive = false;
	pSeq = NULL;
	state = 0;
}

void ShotTrajectoryForMassGame::sShotFx::Update(WVector from, WVector to)
{
	switch (state)
	{
	case 0:
	{
		state = 1;
		CFxSequence* seq = CFx::Instance()->OpenSequence("Tomahawk_A.seq", true);
		if (seq == NULL)
			return;
		FXSEQ983_POS(seq) = from;
		pSeq = CFx::Instance()->OpenSequence("Tomahawk_B.seq", false);
		if (pSeq == NULL)
			return;
		break;
	}
	case 1:
	{
		float dy = to.y - from.y;
		if (dy < 0.0f)
		{
			state = 2;
			if (pSeq)
			{
				CFx::Instance()->CloseSequence(pSeq, false);
				if (pSeq)
				{
					delete pSeq;
					pSeq = NULL;
				}
				pSeq = NULL;
			}
			pSeq = CFx::Instance()->OpenSequence("Tomahawk_C.seq", false);
			if (pSeq == NULL)
				return;
		}
		break;
	}
	case 3:
	{
		if (pSeq)
		{
			CFx::Instance()->CloseSequence(pSeq, false);
			if (pSeq)
			{
				delete pSeq;
				pSeq = NULL;
			}
			pSeq = NULL;
		}
		CFxSequence* seq = CFx::Instance()->OpenSequence("small_tomahawk_d.seq", true);
		if (seq == NULL)
			return;
		FXSEQ983_POS(seq) = to;
		state = 4;
		break;
	}
	}
	if (pSeq)
		FXSEQ983_POS(pSeq) = to;
}

// some fora dos modos 0x008…0x200 e quando o rastro está desligado
bool ShotTrajectoryForMassGame::IsHidden()
{
	if ((GOLFDOC983_GAMEMODE() & 0x3f8) == 0)
		return true;
	return m_bShow == 0;
}

void ShotTrajectoryForMassGame::Display()
{
	if (IsHidden())
		return;
	float nearDist = RangeScale983(Doc()->m_golfGame.gameType, Doc()->m_curChannel.Type, GolfBall().GetRadius());
	for (int i = 0; i < MAX_TRACK; ++i)
	{
		if (m_pos[i].empty() || !m_track[i].bActive)
			continue;
		int cur = m_track[i].cur;
		if (GOLFDOC983_GAMEMODE() & 0x200)
			continue;
		WVector me = *GOLFDOC983_PLAYERPOS();
		WVector tip = m_pos[i][cur];
		if (m_pPlayerPos)
		{
			float dx = me.x - tip.x;
			float dz = me.z - tip.z;
			if (nearDist < sqrtf(dz * dz + 0.0f + dx * dx))
			{
				WVector toCam = g_camera.pivot - tip;
				float dist = sqrtf(toCam.z * toCam.z + toCam.y * toCam.y + toCam.x * toCam.x) / 300.0f;
				if (dist < 1.0f)
				{
					float t = 1.0f;
					float size = (1.0f - dist) * 8.0f;
					// igual ao 983: com cur == 0 os dois pontos ficam sem valor
					WVector prev, at;
					if (cur > 0)
					{
						prev = m_pos[i][cur - 1];
						at = tip;
					}
					if (0.0f < m_track[i].frac)
						t = (0.02f - m_track[i].frac) * 50.0f;
					at = prev * t + at * (1.0f - t);
					WVector scr = g_view->Projection(at);
					if (0.0f < scr.z && scr.z < 1.0f)
					{
						WRect rc;
						rc.x = scr.x - 0.5f * size;
						rc.y = scr.y - 0.5f * size;
						rc.w = size;
						rc.h = size;
						m_pPlayerPos->Render(g_view, WRect(0, 0, 1.0f, 1.0f), rc, 0, m_track[i].color | 0x80000000, 0.0f, 0);
					}
				}
			}
		}
		WTVertex v[4];
		WTVertex* vl[5];
		vl[0] = &v[0];
		v[0].tu = 1.0f;
		v[0].tv = 0.0f;
		vl[1] = &v[1];
		v[1].tv = 0.0f;
		vl[2] = &v[2];
		v[2].tu = 0.0f;
		vl[3] = &v[3];
		v[3].tu = 0.0f;
		v[1].tu = 1.0f;
		v[2].tv = 1.0f;
		vl[4] = NULL;
		v[3].tv = 1.0f;
		for (int j = 1; j <= cur; ++j)
		{
			int a = (int)(m_track[i].alpha[j] * 255.0f);
			if (a == 0)
				continue;
			const WVector* p = &m_pos[i][j];
			WVector to = *p;
			unsigned long color = m_track[i].color | (a << 24);
			v[0].diffuse = color;
			v[1].diffuse = color;
			v[2].diffuse = color;
			v[3].diffuse = color;
			if (cur == j)
			{
				float t;
				if (0.0f < m_track[i].frac)
					t = (0.02f - m_track[i].frac) * 50.0f;
				else
					t = 1.0f;
				to = p[-1] * t + to * (1.0f - t);
				if ((m_track[i].special & 0x70) && m_bShowEffect)
					m_fx[i].Update(p[-1], to);
			}
			if ((m_track[i].special & 0x70) == 0 || !m_bShowEffect)
			{
				WVector dir(to.x - p[-1].x, to.y - p[-1].y, to.z - p[-1].z);
				WVector side;
				side = WCrossProduct(WVector(0.0f, 1.0f, 0.0f), dir);
				side.Normalize();
				side *= m_lineThick * GolfBall().GetRadius();
				WVector c0, c1, c2, c3;
				c0 = p[-1] - side;
				c1 = to - side;
				c2 = side + to;
				c3 = p[-1] + side;
				v[0].SetPosition(c0);
				v[1].SetPosition(c1);
				v[2].SetPosition(c2);
				v[3].SetPosition(c3);
				g_view->DrawPolygonFan(vl, (m_hLine & 0x7ff) | 0x400000, 0x200, false);
			}
		}
	}
}

// leitura por índice de uma tabela filha (0049e3b0 no 983; o lua_tinker da frente A só lê por nome)
static int LuaGetByIndex983(lua_tinker::table tbl, int index)
{
	lua_tinker::table_obj* obj = tbl.m_obj;
	if (obj->validate())
	{
		lua_pushnumber(obj->m_L, (double)index);
		lua_gettable(obj->m_L, obj->m_index);
	}
	else
	{
		lua_pushnil(obj->m_L);
	}
	return lua_tinker::pop<int>(obj->m_L);
}

// uma cor = três leituras da mesma tabela filha (r, g, b nos índices 1 a 3)
#define LUA983_COLOR(tbl, name) \
	((LuaGetByIndex983((tbl).GetValueByName<lua_tinker::table>(name), 3) & 0xff) | \
		(((LuaGetByIndex983((tbl).GetValueByName<lua_tinker::table>(name), 2) & 0xff) | \
			 ((LuaGetByIndex983((tbl).GetValueByName<lua_tinker::table>(name), 1) & 0xff) << 8)) \
			<< 8))

void ShotTrajectoryForMassGame::LoadConfig()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(true);
	script->RunFile("shot_trajectory_for_mass_game.lua");
	lua_system::lua_table_helper tbl = script->GetTable("shot_trajectory_mass_game_table");
	m_color[0] = LUA983_COLOR(tbl, "default_color");
	m_color[1] = LUA983_COLOR(tbl, "tomahawk_color");
	m_color[2] = LUA983_COLOR(tbl, "cobra_color");
	m_color[3] = LUA983_COLOR(tbl, "spike_color");
	m_queueCheckTime = tbl.GetValueByName<float>("queue_check_time");
	m_fadeOutSpeed = tbl.GetValueByName<float>("fade_out_speed");
	m_lineThick = tbl.GetValueByName<float>("trajectory_line_thick");
	m_bShowEffect = tbl.GetValueByName<bool>("show_effect");
	m_posReduce = tbl.GetValueByName<int>("ball_pos_count_reduce");
	m_bWaitMax = tbl.GetValueByName<bool>("wait_max_trajectory");
	if (m_posReduce < 1)
		m_posReduce = 1;
	m_speed = 100.0f / (float)m_posReduce * 0.01f;
}

void ShotTrajectoryForMassGame::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}
	if (msg.message == 0x312)
		m_queue.push_back(*(const sMassGameShot*)msg.param1);
}

ShotTrajectoryForMassGame::ShotTrajectoryForMassGame()
	: m_pad983_3b0(0)
{
	m_speed = 1.0f;
	m_elapsed = 0;
	m_pPlayerPos = NULL;
	m_bShow = 1;
	m_bShowEffect = false;
	m_bWaitMax = false;
	m_queueCheckTime = 1.0f;
	m_fadeOutSpeed = 0.4f;
	m_lineThick = 1.0f;
	for (int i = 0; i < MAX_TRACK; ++i)
	{
		m_pos[i].reserve(1500);
		m_track[i].progress = 0;
		m_track[i].cur = 0;
		m_track[i].frac = 0;
		m_track[i].color = m_color[0];
		m_track[i].effectIndex = -1;
		m_track[i].special = 0;
		m_track[i].alpha.reserve(1500);
	}
	m_queueTime = 0;
	m_pBall = new CGolfBall;
	m_hIndicator = g_resrcmng->LoadTexture("{indicator.bmp", 0, 0, NULL);
	m_dispPriority = DISP_PRIORITY_2;
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, this, "ShotTrajectoryInMassGame", 0, NULL);
	m_pPlayerPos = g_resrcmng->GetOverlay("{player_pos.bmp", 0);
	m_color[0] = 0xffffff;
	m_color[1] = 0xffffff;
	m_color[2] = 0xffffff;
	m_color[3] = 0xffffff;
	LoadConfig();
	m_hLine = g_resrcmng->LoadTexture("[swingtale02.jpg", 0, 0, NULL);
}

ShotTrajectoryForMassGame::~ShotTrajectoryForMassGame()
{
	if (m_pBall)
	{
		delete m_pBall;
		m_pBall = NULL;
	}
	if (g_resrcmng && m_hIndicator)
	{
		g_resrcmng->Release(m_hIndicator);
		m_hIndicator = 0;
	}
	if (g_resrcmng && m_hLine)
	{
		g_resrcmng->Release(m_hLine);
		m_hLine = 0;
	}
	if (g_resrcmng && m_pPlayerPos)
	{
		g_resrcmng->Release(m_pPlayerPos);
		m_pPlayerPos = NULL;
	}
}

void ShotTrajectoryForMassGame::OnInit()
{
	for (int i = 0; i < MAX_TRACK; ++i)
		m_pos[i].clear();
	m_pad983_3b0 = 0;
}

// refaz a tacada na bola global (guardada em m_pBall e devolvida no fim) e tira os pontos da linha: um a cada
// `ball_pos_count_reduce` quadros, mais todos os quadros com evento (quique)
bool ShotTrajectoryForMassGame::BuildTrajectory(const sMassGameShot* shot, std::vector<WVector>* positions,
	int* pFirstBoundIndex)
{
	WVector vel;
	m_pBall->Reset(vel);
	*m_pBall = GolfBall();
	CGolfBall& ball = GolfBall();
	ball.Reset(shot->pos);
	CGolfBall::sExpected* pre = ball.PreDataToWrite(0);
	pre->pos = ball.m_pos;
	pre->event = 0;
	pre->state = (CGolfBall::eBallState)shot->state;
	pre->vel = shot->vel;
	ball.m_initSpecial = shot->special;
	ball.m_initCurve = shot->initCurve;
	ball.m_initSpin = shot->initSpin;
	unsigned char groundType = shot->groundType;
	ball.m_groundType = groundType;
	float ratio = shot->ratio;
	unsigned char clubId = shot->clubId;
	float clubPower = shot->clubPower;
	float clubRange = shot->clubRange;
	float power = shot->power;
	ball.m_vel = pre->vel;
	ball.m_state = pre->state;
	ball.m_special = ball.m_initSpecial;
	ball.m_spin = shot->spin;
	ball.m_curve = shot->curve;
	ball.m_tomahawkFrame = -1;
	unsigned long savedFrame = PVS983_FRAME();
	WVector savedWind = Wind().GetWind983();
	unsigned long frame = shot->frame;
	Pvs983SetFrame(frame);
	WVector wind(shot->wind.x, shot->wind.y, shot->wind.z);
	Wind().SetWind(wind);
	((CClubJpShot*)&GolfClub())->SetShotOverride983(shot->clubA, shot->clubB);
	GetPVS().ResetVariable();
	ball.ClearBuffer();
	ball.m_afterBurner.z = 0.0f;
	ball.m_afterBurner.y = 0.0f;
	ball.m_afterBurner.x = 0.0f;
	ball.m_rollFrame = ball.m_state == CGolfBall::BALL_ROLL ? 0 : -1;
	ball.ResetVectorSlideCount();
	ball.m_renderNum = 0;
	ball.m_bounceFrame = -1;
	ball.m_topFrame = -1;
	ball.m_nearObjFrame = -1;
	ball.m_holeFrame = -1;
	ball.m_holeOutFrame = -1;
	ball.m_greenFrame = -1;
	ball.m_cupFrame = -1;
	ball.m_afterBurnerFrame = -1;
	ball.m_stayCount = 0;
	ball.m_standGround = 0;
	ball.m_prevColTriIndex = -1;
	ball.m_prevColObjIndex = -1;
	ball.m_colTriIndex = -1;
	ball.m_colObjIndex = -1;
	ball.m_meshIndex = -1;
	ball.m_cobraFrame = -1;
	ball.m_tomahawkFrame = -1;
	ball.m_spikeFrame = -1;
	ball.m_spikeDiveFrame = -1;
	ball.m_spikeEndFrame = -1;
	ball.m_vectorSlideFrame = -1;
	while (ball.m_state != CGolfBall::BALL_STOP)
	{
		++ball.m_renderNum;
		CGolfBall::sExpected* e = ball.PreDataToWrite(ball.m_renderNum);
		e->sound[0] = 0;
		e->event = 0;
		e->effect = 0;
		GetPVS().PreProcess(0.02f, true);
		GetPVS().BallProcess(0.02f);
		if (GolfClub().GetApproachMode() == 4 || (ball.m_special & 0x60))
		{
			if (ball.m_special & 0x20)
			{
				if (ball.m_cobraFrame < 0)
				{
					float base = GetPVS().GetBasePosY(ball.m_pos, ball.GetRadius() + ball.GetRadius());
					ball.m_pos.y = Max(base, ball.m_pos.y);
				}
				else
				{
					ball.m_spin = ball.m_spin - 0.02f;
				}
			}
			else if (ball.m_special & 0x40)
			{
				if (ball.m_spikeFrame < 0)
				{
					float base = GetPVS().GetBasePosY(ball.m_pos, ball.GetRadius() + ball.GetRadius());
					ball.m_pos.y = Max(base, ball.m_pos.y);
				}
				else
				{
					ball.m_spin = ball.m_spin - 0.02f;
				}
			}
			else
			{
				if (ball.m_bounceFrame < 0 && GetPVS().GetObjColNum() == 0)
					ball.m_spin = ball.m_spin - (0.5f - ball.m_initSpin * 0.1f) * 0.02f;
				else
					ball.m_spin = ball.m_spin - 0.002f;
			}
		}
		bool bClamp = false;
		if (ball.m_renderNum < 3000)
		{
			if (ball.m_renderNum >= 0x5dc)
			{
				float speedSq = ball.m_vel.z * ball.m_vel.z + ball.m_vel.x * ball.m_vel.x + ball.m_vel.y * ball.m_vel.y;
				if (0.1f <= sqrtf(speedSq) && GetPVS().GetObjColNum() < 10 && GetPVS().GetMeshIndex(ball.m_pos, false) != -1)
					bClamp = false;
				else
					bClamp = true;
			}
		}
		else
		{
			bClamp = true;
		}
		if (bClamp)
		{
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			WVector gp = GetPVS().GetGroundPoint(ball.m_pos, false, NULL, false);
			ball.m_pos = gp;
			ball.m_state = CGolfBall::BALL_STOP;
			ball.m_pos.y = ball.GetRadius() + ball.m_pos.y;
		}
		e->pos = ball.m_pos;
		e->vel = ball.m_vel;
		e->curveRot = ball.m_curveRot;
		e->spinRot = ball.m_spinRot;
		e->state = ball.m_state;
		if (ball.m_state == CGolfBall::BALL_STOP)
			ball.m_special &= 0xffffff8f;
		else if (ball.m_state == CGolfBall::BALL_BOUNCE && ball.m_vel.y < 0.0f && ball.m_topFrame < 0)
			ball.m_topFrame = ball.m_renderNum;
		if (ball.m_tomahawkFrame < 0 && (ball.m_initSpecial & 0x10) && (ball.m_special & 0x10) == 0)
			ball.m_tomahawkFrame = ball.m_renderNum;
		if (ball.m_special & 0x20)
		{
			if (ball.m_renderNum == 1)
			{
				ball.m_vel.y = 0.0f;
				WVector& n = ball.m_vel.Normalize();
				vel.x = power * n.x;
				vel.y = power * n.y;
				vel.z = power * n.z;
				ball.m_vel.x = vel.x;
				ball.m_vel.y = vel.y;
				ball.m_vel.z = vel.z;
			}
			const WVector& start = ball.PreData(0)->pos;
			float dx = ball.m_pos.x - start.x;
			float dz = ball.m_pos.z - start.z;
			if (ball.m_cobraFrame < 0)
			{
				float dist = sqrtf(dz * dz + dx * dx + 0.0f);
				float powerRatio = sqrtf(ratio);
				if ((powerRatio * clubRange - 100.0f) * 3.2f < dist)
				{
					ball.m_cobraFrame = ball.m_renderNum;
					ball.m_vel.Normalize();
					switch (clubId)
					{
					case 0:
						ball.m_vel *= 74.0f;
						break;
					case 1:
						ball.m_vel *= 76.0f;
						break;
					case 2:
						ball.m_vel *= 80.0f;
						break;
					}
					ball.m_vel *= powerRatio;
					ball.m_spin = 2.5f;
				}
			}
		}
		if (ball.m_spikeEndFrame < 0 && (ball.m_initSpecial & 0x40))
		{
			if ((ball.m_special & 0x40) == 0)
			{
				if (ball.m_spikeDiveFrame > 0)
					ball.m_spikeEndFrame = ball.m_renderNum;
			}
		}
		if (ball.m_special & 0x40)
		{
			if (ball.m_renderNum == 1)
			{
				ball.m_vel.y = 0.0f;
				WVector& n = ball.m_vel.Normalize();
				wind.x = power * n.x;
				wind.y = power * n.y;
				wind.z = power * n.z;
				ball.m_vel.x = wind.x;
				ball.m_vel.y = wind.y;
				ball.m_vel.z = wind.z;
			}
			const WVector& start = ball.PreData(0)->pos;
			float dx = ball.m_pos.x - start.x;
			float dz = ball.m_pos.z - start.z;
			if (ball.m_spikeFrame < 0 && 0.0f < sqrtf(dz * dz + dx * dx + 0.0f))
			{
				ball.m_spikeFrame = ball.m_renderNum;
				ball.m_vel.Normalize();
				ball.m_vel *= 72.5f;
				float powerRatio = sqrtf(ratio);
				ball.m_vel *= powerRatio + powerRatio;
				ball.m_spin = 3.1f;
			}
			if (ball.m_topFrame >= 0 && ball.m_topFrame + 0x3c < ball.m_renderNum && ball.m_spikeDiveFrame < 0)
			{
				ball.m_spikeDiveFrame = ball.m_renderNum;
				float v;
				switch (clubId)
				{
				case 0:
					v = Min(sqrtf(ratio) * clubPower, 344.0f);
					ball.m_vel.y = -8.0f - (344.0f - v) / 112.0f * 21.5f;
					break;
				case 1:
					v = Min(sqrtf(ratio) * clubPower, 306.0f);
					ball.m_vel.y = -10.3f - (306.0f - v) / 105.0f * 20.5f;
					break;
				case 2:
					v = Min(sqrtf(ratio) * clubPower, 273.0f);
					ball.m_vel.y = -10.8f - (273.0f - v) / 100.0f * 20.2f;
					break;
				}
				ball.m_vel *= sqrtf(ratio) * 7.0f;
				ball.m_spin = ball.m_initSpin;
			}
		}
		if (e->event == 6 && ball.m_vectorSlideFrame == -1)
			ball.m_vectorSlideFrame = ball.m_renderNum;
	}
	ball.m_special = ball.m_initSpecial;
	ball.m_groundType = groundType;
	bool bFirst = true;
	int skipped = 0;
	positions->push_back(ball.PreData(0)->pos);
	for (int i = 0; i < ball.m_renderNum; ++i)
	{
		if (ball.PreData(i)->event != 0)
		{
			positions->push_back(ball.PreData(i)->pos);
			if (bFirst)
			{
				bFirst = false;
				if (pFirstBoundIndex)
					*pFirstBoundIndex = (int)positions->size() - 1;
			}
			skipped = 0;
		}
		else if (skipped == m_posReduce)
		{
			positions->push_back(ball.PreData(i)->pos);
			skipped = 0;
		}
		++skipped;
	}
	positions->push_back(ball.PreData(ball.m_renderNum)->pos);
	if (pFirstBoundIndex && bFirst)
		*pFirstBoundIndex = (int)positions->size() - 1;
	ball.Reset(wind);
	GolfBall() = *m_pBall;
	GetPVS().PreProcess(0.02f, true);
	Pvs983SetFrame(savedFrame);
	Wind().SetWind(savedWind);
	((CClubJpShot*)&GolfClub())->ClearShotOverride983();
	return true;
}

// tira uma tacada da fila para a 1ª linha livre; com `wait_max_trajectory` as doze linhas só partem juntas, cada uma com um
// atraso sorteado de 0 a 0,9 s
void ShotTrajectoryForMassGame::PopShot()
{
	if (m_queue.empty())
		return;
	bool bFound = false;
	for (int i = 0; i < MAX_TRACK; ++i)
	{
		if (!m_pos[i].empty())
			continue;
		const sMassGameShot* shot = &m_queue.front();
		sTrack& track = m_track[i];
		track.frac = 0;
		track.progress = 0;
		track.cur = 0;
		track.effectIndex = -1;
		track.alpha.clear();
		track.special = shot->special;
		track.delay = 0;
		track.bActive = !m_bWaitMax;
		if (shot->special & 0x10)
			m_track[i].color = m_color[1];
		else if (shot->special & 0x20)
			m_track[i].color = m_color[2];
		else if (shot->special & 0x40)
			m_track[i].color = m_color[3];
		else
			m_track[i].color = m_color[0];
		m_fx[i].Close();
		BuildTrajectory(shot, &m_pos[i], &track.effectIndex);
		track.alpha.assign(m_pos[i].size(), 0.7f);
		m_queue.pop_front();
		bFound = true;
		break;
	}
	if (m_bWaitMax && bFound)
	{
		for (int i = 0; i < MAX_TRACK; ++i)
		{
			if (m_pos[i].empty())
				return;
			if (m_track[i].bActive == true)
				return;
		}
		for (int i = 0; i < MAX_TRACK; ++i)
		{
			int r = rand() % 10;
			m_track[i].bActive = OnlinePlay() != 0;
			m_track[i].delay = (float)r * 0.1f;
			m_elapsed = 0;
		}
	}
}

void ShotTrajectoryForMassGame::OnProcess(float delta)
{
	m_queueTime += delta;
	m_elapsed += delta;
	float step = m_speed * delta;
	int tail = (int)(m_speed * 20.0f);
	float fade = m_fadeOutSpeed;
	int lag = (int)(m_speed * 10.0f);
	for (int i = 0; i < MAX_TRACK; ++i)
	{
		if (m_pos[i].empty())
			continue;
		sTrack& track = m_track[i];
		if (!track.bActive)
			continue;
		if (!(track.delay <= m_elapsed))
			continue;
		track.progress += step;
		// igual ao 983: perto do fim da linha o apagamento dobra e o atraso some — e ficam assim para as linhas seguintes
		if ((unsigned int)track.cur > m_pos[i].size() - tail)
		{
			lag = 0;
			fade = fade * 2.0f;
		}
		if (m_bShowEffect && track.effectIndex <= track.cur && track.effectIndex != -1)
		{
			if ((track.special & 0x70) && (GOLFDOC983_GAMEMODE() & 0xfffffdff))
				m_fx[i].state = 3;
			else
				m_fx[i].state = 4;
			track.effectIndex = -1;
		}
		int last = track.cur - lag;
		if (last < 0)
			last = 0;
		for (int j = 0; j <= last; ++j)
		{
			track.alpha[j] -= fade * delta;
			if (track.alpha[j] < 0.0f)
				track.alpha[j] = 0.0f;
		}
		if (track.alpha.back() <= 0.0f)
			m_pos[i].clear();
		int idx = (int)(track.progress * 50.0f);
		int old = track.cur;
		track.cur = Min<unsigned int>(idx, m_pos[i].size() - 1);
		if (track.cur != old)
		{
			track.frac = 0.0f;
		}
		else
		{
			track.frac += step;
			if (track.frac > 1.0f)
				track.frac = 1.0f;
		}
	}
	if (m_queueCheckTime < m_queueTime)
	{
		m_queueTime = 0.0f;
		if (!IsHidden())
			PopShot();
	}
}

// 006ade10: troca o bloco de 0x6c em CClub+0x80 (delete do anterior, novo sClub zerado); curva em +0x34 e giro em +0x44
void CClubJpShot::SetShotOverride983(float a, float b)
{
	CClub& club = *(CClub*)this;
	if (club.m_pShotOverride983)
		operator delete(club.m_pShotOverride983);
	club.m_pShotOverride983 = NULL;
	club.m_pShotOverride983 = new CClub::sClub;
	memset(club.m_pShotOverride983, 0, sizeof(CClub::sClub));
	club.m_pShotOverride983->curve = a;
	club.m_pShotOverride983->spin = b;
}

// 006ac640: apaga o bloco e zera o ponteiro
void CClubJpShot::ClearShotOverride983()
{
	CClub& club = *(CClub*)this;
	if (club.m_pShotOverride983)
		operator delete(club.m_pShotOverride983);
	club.m_pShotOverride983 = NULL;
}

// 006b0fb0: refaz o taco do jogador da vez. 006b0630 (= CClub::SelectBestClub(const WVector&)) recebe o vetor em
// Doc+0xd8 - 0x2c + Doc+0x17c*0x38 (= m_pHoleData[m_currentHole-1].pin); se o documento permite compasso (00b2e8b0) refaz a barra de forca (00636bd0)
void PerformPowerBarSetting();

void CClubJp::Refresh983()
{
	// A745: o 983 le Doc+0xd8/+0x17c (m_pHoleData/m_currentHole) = pino do buraco da vez; o nosso lia m_pPlayer/m_currentPlayer
	((CClub*)this)->SelectBestClub(*GOLFDOC983_PLAYERPOS());
	if (Doc()->m_unused47d0) // 00b2e8b0 (S5::IsUsableCalipers)
		PerformPowerBarSetting();
}
