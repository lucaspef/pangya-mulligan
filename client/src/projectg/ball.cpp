#include "minatl.h"
#include "projectg.h"
#include "fx.h"

// HACK: CFx::AddFxKeeper is not declared in fx.h. Adding it there perturbs
// projectg.cpp (SaveScreenShot), so it is declared in a local helper class.
class CFxEx : public CFx
{
public:
	void AddFxKeeper(sFxKeeper& keeper);
};

#pragma comment(linker, \
	"/alternatename:?AddFxKeeper@CFxEx@@QAEXAAUsFxKeeper@@@Z=?AddFxKeeper@CFx@@QAEXAAUsFxKeeper@@@Z")
#include "golfball.h"
#include "quadtree.h"
#include "wresrcmng.h"
#include "wpuppet.h"
#include "wbone.h"
#include "mathconsts.h"
#include "../shared/localize.h"
#include "ball.h"

extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;

unsigned long ModifyColor(unsigned long color, float factor);
WMatrix RotMat(const WVector& axis, float angle);

inline WMatrix operator*(const WMatrix& m, float scalar)
{
	WMatrix result;
	result.xa = m.xa * scalar;
	result.ya = m.ya * scalar;
	result.za = m.za * scalar;
	result.pivot = m.pivot;
	return result;
}

#define CUR_BALL_TID \
	Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall

float BALL_AMBIENT_RATIO = 0.8f;

IMPLEMENT_ACTOR(CBall, IActor)

CBall::CBall()
	: m_pPuppet(NULL), m_shadowTex(0), m_bVisible(true), m_lastFrame(-1)
{
	m_dispPriority = DISP_PRIORITY_1;
}

void CBall::OnPreLoadInit()
{
	m_axis = WVector::UNIT_POS_X;

	for (int i = 0; i < 4; i++)
	{
		m_shadowVtx[i].diffuse = 0x80ffffff;
		m_shadowVtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_shadowVtx[i].tv = (i & 1) ? 0.0f : 1.0f;
	}

	m_pShadowVtx[0] = &m_shadowVtx[0];
	m_pShadowVtx[1] = &m_shadowVtx[1];
	m_pShadowVtx[2] = &m_shadowVtx[3];
	m_pShadowVtx[3] = &m_shadowVtx[2];
	m_pShadowVtx[4] = NULL;
	m_light.type = 2;
	m_shadowTex = g_resrcmng->LoadTexture("[ball_shadow.jpg", 0, 0, NULL);

	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, NULL,
		"\xb0\xf8", 8, NULL);
	CSceneManager::Instance()->RegisterElement(OBJ_FUNC, NULL, this,
		"\xb0\xf8\xb1\xd7\xb8\xb2\xc0\xda", 0, NULL);
}

void CBall::OnDestroy()
{
	std::list<sBall*>::iterator it;
	for (it = m_ballList.begin(); it != m_ballList.end(); it++)
	{
		if (*it)
		{
			if (g_resrcmng && (*it)->puppet)
			{
				g_resrcmng->Release((*it)->puppet, false);
				(*it)->puppet = NULL;
			}
			if (*it)
			{
				delete *it;
				*it = NULL;
			}
		}
	}
	m_ballList.clear();

	if (g_resrcmng && m_shadowTex)
	{
		g_resrcmng->Release(m_shadowTex);
		m_shadowTex = 0;
	}
}

void CBall::OnLoad()
{
	if (IsLocalCountry(30))
	{
		sFxKeeper keeper;
		keeper.keeper = this;
		keeper.func = &CFxKeeper::EmptyFx;
		((CFxEx*)CFx::Instance())->AddFxKeeper(keeper);
	}

	if (IsLocalContent((localContentType_t)0x40 /*983: era 0x3f; interpolado*/))
		AddBall(0x14000000);

	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state != 3)
			AddBall(Doc()->m_userInfo[i].userEquip.tidBall);
	}

	std::list<sBall*>::iterator it = m_ballList.begin();
	if (it != m_ballList.end())
		m_pPuppet = (*it)->puppet;
}

void CBall::OnReload()
{
	std::list<sBall*>::iterator it;
	for (it = m_ballList.begin(); it != m_ballList.end(); it++)
	{
		if (*it)
		{
			if (g_resrcmng && (*it)->puppet)
			{
				g_resrcmng->Release((*it)->puppet, false);
				(*it)->puppet = NULL;
			}
			if (*it)
			{
				delete *it;
				*it = NULL;
			}
		}
	}
	m_ballList.clear();

	CSceneManager::Instance()->SetPuppet("\xb0\xf8", NULL);

	OnLoad();
}

void CBall::OnInit()
{
	m_matrix.Reset();
	m_sink = 0.0f;
	m_sinkFrame = -1;
	m_light.nearOne = g_lightset.nearOne;
}

CBall::sBall* CBall::AddBall(unsigned long tid)
{
	sBall* ball = FindBall(tid);
	if (ball)
		return ball;

	IFF_STRUCT::sBall* iff = ItemManager()->FindBall(tid);
	if (iff)
	{
		ball = new sBall;
		ball->iff = iff;

		char name[40];
		sprintf(name, "%s.pet", iff->Data);
		ball->puppet = g_resrcmng->GetPuppet(name, false, true, false);

		m_ballList.push_back(ball);
	}

	return ball;
}

CBall::sBall* CBall::FindBall(unsigned long tid)
{
	std::list<sBall*>::iterator it =
		std::find_if(m_ballList.begin(), m_ballList.end(), sFindBall(tid));

	if (it != m_ballList.end())
		return *it;

	return NULL;
}

float CBall::GetDownDelta()
{
	if (m_sinkFrame == GolfBall().m_renderNum)
		return 1.0f;

	return Max(0.0f, (float)(GolfBall().m_playFrame - m_sinkFrame)) /
		(float)(GolfBall().m_renderNum - m_sinkFrame);
}

void CBall::OnProcess(float delta)
{
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
	{
		if (Doc()->m_replayState == 11 || Doc()->m_replayState == 12)
		{
			if (Doc()->m_replayPlayType == 2)
			{
				if (Doc()->m_replayState == 11)
				{
					if (Doc()->m_replayShotIndex > -1)
					{
						Doc()->m_replayShotIndex -= 2;
						if (Doc()->m_replayShotIndex == 0)
							Doc()->m_replayState = 1;
						else if (Doc()->m_replayShotIndex < 0)
							Doc()->m_replayState = 14;
					}
				}
				else if (Doc()->m_replayState == 12)
				{
					Doc()->m_replayState = 3;
				}

				AfxGetTask()->GetActor("GolfRule")
					<< MsgObject(NULL, M983(0xa4), 0, 0, 0, 0, 0);
			}
			else if (Doc()->m_replayPlayType == 1)
			{
				AfxGetTask()->GetActor("GolfRule")
					<< MsgObject(NULL, M983(0xa4), 0, 0, 0, 0, 0);
			}
		}
	}

	if (!m_bVisible)
		return;

	CGolfBall& ball = GolfBall();

	if (ball.m_state == CGolfBall::BALL_STOP)
	{
		m_matrix.pivot = ball.m_pos;
		if (m_sink > 0.0f)
			m_matrix.pivot.y -= ball.m_radius * m_sink * 2.0f;

		if (GOLFDOC()->m_gameMode & 0x80)
		{
			DetachFx(IFF_STRUCT::sBall::FX_FLY);
			DetachFx(IFF_STRUCT::sBall::FX_ROLL);
		}
		return;
	}

	m_light.diffuse = g_lightset.diffuse;

	if (WVectorLen(GolfBall().m_vel) < 10.0f)
	{
		DetachFx(IFF_STRUCT::sBall::FX_FLY);
		DetachFx(IFF_STRUCT::sBall::FX_ROLL);
	}

	m_axis = WCrossProduct(WVector::UNIT_POS_Y, ball.m_vel);
	if (WisEqual(m_axis, WVector::ZERO, g_EPSILON))
		m_axis = WVector::UNIT_POS_Y;

	m_matrix = RotMat(m_axis, ball.m_spinRot);
	m_matrix.pivot = ball.m_pos;

	if (ball.m_playFrame > 0 && ball.m_afterBurnerFrame > 0 &&
		ball.m_playFrame >= ball.m_afterBurnerFrame &&
		ball.m_playFrame < ball.m_afterBurnerFrame + 50)
	{
		WVector shake((rand() % 11 - 5) * 0.005f, 0.0f,
			(rand() % 11 - 5) * 0.005f);
		m_matrix.pivot += shake;
	}

	if (m_sink > 0.0f)
	{
		float radius = ball.m_radius;
		m_matrix.pivot.y -= (radius * GetDownDelta()) * m_sink * 2.0f;
	}

	if (m_lastFrame < GolfBall().m_bounceFrame &&
		GolfBall().m_bounceFrame <= GolfBall().m_playFrame)
	{
		AttachFx(IFF_STRUCT::sBall::FX_BOUNCE);
	}
	m_lastFrame = GolfBall().m_playFrame;

	sBall* cur = FindBall(CUR_BALL_TID);
	if (!cur)
		return;

	bool bFx = false;
	unsigned int i;
	for (i = 0; i < 4; i++)
	{
		if (cur->spray[i])
		{
			bFx = true;
			break;
		}
	}
	if (!bFx)
	{
		for (i = 0; i < 4; i++)
		{
			if (cur->seq[i])
			{
				bFx = true;
				break;
			}
		}
	}
	if (bFx)
		TransformBall();
}

void CBall::TransformBall()
{
	float scale;

	if (GolfBall().m_playFrame < 0)
	{
		scale = 1.0f;
	}
	else
	{
		WVector p[2];
		WVector up = g_camera.ya * GolfBall().m_radius;
		p[0] = g_view->Projection(GolfBall().m_pos - up);
		WVector down = GolfBall().m_radius * g_camera.ya;
		p[1] = g_view->Projection(down + GolfBall().m_pos);

		float d = p[0].y - p[1].y;
		float h = (d > 0.0f) ? d : -d;
		scale = 1.5f / ((h < 0.1f) ? 0.1f : ((h > 1.5f) ? 1.5f : h));
	}

	if (m_pPuppet)
	{
		m_pPuppet->SetMatrix(m_matrix * scale);
		m_pPuppet->Transform(NULL, 1.0f, 0);
		m_pPuppet->UpdateBSphere();
	}
}

void CBall::OnDisplay()
{
	if (!m_bVisible)
		return;

	TransformBall();

	if (m_pPuppet && !m_pPuppet->GetAlpha(NULL))
		return;

	if (GolfBall().m_state == CGolfBall::BALL_STOP && !m_bUpdateLight)
		return;

	m_bUpdateLight = false;
	if (m_pPuppet)
		m_pPuppet->UpdateLightSource(&m_light, false, false);
}

void CBall::Display()
{
	if (Skipped() || !m_bVisible)
		return;

	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector d = pin - GolfBall().m_pos;
	CGolfBall& ball = GolfBall();
	if (sqrtf(d.x * d.x + d.z * d.z) < ball.m_radius * 4.0f)
		return;

	float size = GolfBall().m_radius * 1.2f;
	int index;
	WVector ground =
		GetPVS().GetGroundPoint(GolfBall().m_pos, true, &index, false);
	if (index == -1)
		return;

	WVector normal = GetPVS().GetMeshNormal(index);
	WVector a = WCrossProduct(normal, WVector::UNIT_POS_Z);
	WVector b = WCrossProduct(normal, a);
	a.Normalize();
	b.Normalize();

	WVector pos = ground + WVector(0.0f, 0.001f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		WVector v = pos;
		v += a * (((i & 2) ? 1.0f : -1.0f) * size);
		v += b * (((i & 1) ? 1.0f : -1.0f) * size);
		m_shadowVtx[i].SetPosition(v);
	}

	g_view->DrawPolygonFan(m_pShadowVtx, (m_shadowTex & 0x7ff) | 0x26300000,
		0x200, false);
}

void CBall::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x163):
		m_bVisible = true;
		CSceneManager::Instance()->SetVisible("\xb0\xf8", true);
		break;

	case M983(0x164):
		m_bVisible = false;
		CSceneManager::Instance()->SetVisible("\xb0\xf8", false);
		break;

	case M983(0x1eb):
		m_bUpdateLight = true;
		m_light.diffuse = ModifyColor(g_lightset.diffuse,
			GetPVS().GetShadowIntensity(GolfBall().m_pos));
		m_light.ambient = ModifyColor(g_lightset.ambient, BALL_AMBIENT_RATIO);
		break;

	case M983(0x1ed):
		switch (GetPVS().GetGroundType(GolfBall().m_pos, true, NULL))
		{
		case 0:
		case 1:
		case 2:
		case 6:
		case 9:
			m_sink = 0.0f;
			break;
		case 3:
		case 4:
		case 5:
			m_sink = 1.0f - GetPVS().GetMinPowerOnGround(GolfBall().m_pos);
			break;
		}
		break;

	case M983(0x1ec):
		switch (GetPVS().GetGroundType(
			GolfBall().PreData(GolfBall().m_renderNum)->pos, true, NULL))
		{
		case 0:
		case 1:
		case 2:
		case 6:
		case 9:
			m_sink = 0.0f;
			m_sinkFrame = -1;
			break;
		case 3:
		case 4:
		case 5:
			m_sink = 1.0f -
				GetPVS().GetMinPowerOnGround(
					GolfBall().PreData(GolfBall().m_renderNum)->pos);
			m_sinkFrame =
				Max(GolfBall().m_renderNum - 100, GolfBall().m_rollFrame);
			break;
		}
		break;

	case M983(0x4c):
	{
		unsigned long tid = msg.param1;

		DetachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		DetachFx(IFF_STRUCT::sBall::FX_B4_SHOT);

		sBall* ball = AddBall(tid);
		if (!ball)
			break;

		CUR_BALL_TID = tid;

		m_pPuppet = ball->puppet;
		if (m_pPuppet)
			m_pPuppet->UpdateLightSource(&m_light, false, false);
		CSceneManager::Instance()->SetPuppet("\xb0\xf8", m_pPuppet);

		AttachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		AttachFx(IFF_STRUCT::sBall::FX_B4_SHOT);
		break;
	}

	case M983(0x4d):
	{
		unsigned long tid = msg.param1;
		int player = msg.param2;

		DetachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		DetachFx(IFF_STRUCT::sBall::FX_B4_SHOT);

		sBall* ball = AddBall(tid);
		if (!ball)
			break;

		Doc()->m_userInfo[player].userEquip.tidBall = tid;

		m_pPuppet = ball->puppet;
		if (m_pPuppet)
			m_pPuppet->UpdateLightSource(&m_light, false, false);
		CSceneManager::Instance()->SetPuppet("\xb0\xf8", m_pPuppet);

		AttachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		AttachFx(IFF_STRUCT::sBall::FX_B4_SHOT);
		break;
	}

	case 0x4e: // 983 (00712ac0): sem par no 645; refaz todos os efeitos da bola atual
	{
		for (int i = 0; i < 7; i++)
			DetachFx(i);
		if (!AddBall(CUR_BALL_TID))
			break;
		for (int i = 0; i < 7; i++)
			AttachFx(i);
		break;
	}

	case M983(0x1a0):
	{
		DetachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		DetachFx(IFF_STRUCT::sBall::FX_B4_SHOT);

		m_lastFrame = -1;

		sBall* ball = FindBall(CUR_BALL_TID);
		if (!ball)
			break;

		m_pPuppet = ball->puppet;
		if (m_pPuppet)
			m_pPuppet->UpdateLightSource(&m_light, false, false);
		CSceneManager::Instance()->SetPuppet("\xb0\xf8", m_pPuppet);

		AttachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		AttachFx(IFF_STRUCT::sBall::FX_B4_SHOT);
		break;
	}

	case M983(0x1ee):
	{
		sBall* ball = FindBall(CUR_BALL_TID);
		if (!ball)
			break;

		AttachFx(IFF_STRUCT::sBall::FX_ALWAYS);
		AttachFx(IFF_STRUCT::sBall::FX_B4_SHOT);

		ball->spray[2] = ball->spray[3] = NULL;
		ball->seq[2] = ball->seq[3] = NULL;
		break;
	}

	case M983(0x1ef):
		AttachFx(IFF_STRUCT::sBall::FX_SHOT);
		AttachFx(IFF_STRUCT::sBall::FX_FLY);
		break;

	case 0x268: // 983 (00712f35): SetTurnChangeEvent 00681ff0 no tee do 1o buraco com item de aniversario (tabela 00e15168)
	{
		Attacher attacher;
		sBall* ball = FindBall(CUR_BALL_TID);
		if (!ball || !ball->puppet)
			break;
		w_bound_box* box = ball->puppet->m_bbList.Find("center");
		if (box && box->bone)
			attacher.mat[0] = &box->bone->m_matrix;
		CFx::Instance()->Attach(attacher, "birth_balloon.seq", true, NULL);
		g_audio->PlaySfx("birth_bomb");
		break;
	}
	}
}

void CBall::AttachFx(int type)
{
	if (type < 0 || type >= IFF_STRUCT::sBall::FX_MAX_NUM)
		return;

	sBall* ball = FindBall(CUR_BALL_TID);
	if (!ball || !ball->iff)
		return;

	if (!ball->iff->Fx[type][0])
		return;

	int len = strlen(ball->iff->Fx[type]);
	if (len < 5)
		return;

	Attacher attacher;
	WBone* bone;
	if (ball->iff->FxBone[type][0] && m_pPuppet &&
		(bone = m_pPuppet->FindBone(ball->iff->FxBone[type])) != NULL)
	{
		attacher.mat[0] = &bone->m_matrix;
	}
	else if (m_pPuppet)
	{
		w_bound_box* box;
		if (ball->iff->FxBone[type][0])
			box = m_pPuppet->m_bbList.Find(ball->iff->FxBone[type]);
		else
			box = m_pPuppet->m_bbList.Find("center");
		if (box)
		{
			if (box->bone)
				attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
		}
	}

	int slot;
	switch (type)
	{
	case IFF_STRUCT::sBall::FX_ALWAYS:
		slot = 0;
		break;
	case IFF_STRUCT::sBall::FX_B4_SHOT:
		slot = 1;
		break;
	case IFF_STRUCT::sBall::FX_FLY:
		slot = 2;
		break;
	case IFF_STRUCT::sBall::FX_ROLL:
		slot = 3;
		break;
	default:
		slot = -1;
		break;
	}

	void* fx =
		CFx::Instance()->Attach(attacher, ball->iff->Fx[type], true, NULL);
	if (slot == -1)
		return;

	if (!strcmpi(ball->iff->Fx[type], "BFX_B3.seq"))
		g_audio->PlaySfx("Ball_B");

	if (!strcmpi(&ball->iff->Fx[type][len - 3], "spr"))
		ball->spray[slot] = (CFxSpray*)fx;
	else
		ball->seq[slot] = (CFxSequence*)fx;
}

void CBall::DetachFx(int type)
{
	if (type != IFF_STRUCT::sBall::FX_ALWAYS &&
		type != IFF_STRUCT::sBall::FX_B4_SHOT &&
		type != IFF_STRUCT::sBall::FX_FLY && type != IFF_STRUCT::sBall::FX_ROLL)
		return;

	sBall* ball = FindBall(CUR_BALL_TID);
	if (!ball)
		return;

	int slot;
	switch (type)
	{
	case IFF_STRUCT::sBall::FX_ALWAYS:
		slot = 0;
		break;
	case IFF_STRUCT::sBall::FX_B4_SHOT:
		slot = 1;
		break;
	case IFF_STRUCT::sBall::FX_FLY:
		slot = 2;
		break;
	case IFF_STRUCT::sBall::FX_ROLL:
		slot = 3;
		break;
	default:
		return;
	}

	if (ball->spray[slot])
	{
		CFx::Instance()->CloseSpray(ball->spray[slot]);
		ball->spray[slot] = NULL;
	}

	if (ball->seq[slot])
	{
		if (slot == 0)
			CFx::Instance()->CloseSequence(ball->seq[slot], true);
		else
			CFx::Instance()->CloseSequence(ball->seq[slot], false);
		ball->seq[slot] = NULL;
	}
}

void CBall::EmptyFx(void* fx)
{
	if (!fx)
		return;

	for (std::list<sBall*>::iterator it = m_ballList.begin();
		it != m_ballList.end(); ++it)
	{
		sBall* ball = *it;
		if (ball)
		{
			for (int i = 0; i < 4; i++)
			{
				if (ball->spray[i] == fx)
					ball->spray[i] = NULL;
				if (ball->seq[i] == fx)
					ball->seq[i] = NULL;
			}
		}
	}
}
