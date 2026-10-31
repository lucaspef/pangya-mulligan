#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "quadtree.h"
#include "shareddoc.h"
#include "scenemanager.h"
#include "wpuppet.h"
#include "commonutil.h"
#include "trajectory.h"

extern WMatrix g_camera;

// HACK: 0.45f is defined in grid.cpp as a function-local static aliased to
// __real@3ee66666; using the literal here would emit a competing COMDAT that
// all the other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_3ee66666 = 0.45f; // C64.4: o estático de função do rebang não existe mais

// HACK: scenemanager.h is part of the PCH and every declaration added there
// shifts the register allocation of unrelated units. The three CSceneManager
// members used here are declared on a helper class instead and their symbols
// are aliased to the real ones.
class CSceneManagerEx : public CSceneManager
{
public:
	eObjType GetType(const char* name);
	void SetType(const char* name, eObjType type);
	WPuppet* GetPuppet(const char* name);
};

#pragma comment(linker, \
	"/alternatename:?GetType@CSceneManagerEx@@QAE?AW4eObjType@@PBD@Z=?GetType@CSceneManager@@QAE?AW4eObjType@@PBD@Z")
#pragma comment(linker, \
	"/alternatename:?SetType@CSceneManagerEx@@QAEXPBDW4eObjType@@@Z=?SetType@CSceneManager@@QAEXPBDW4eObjType@@@Z")
#pragma comment(linker, \
	"/alternatename:?GetPuppet@CSceneManagerEx@@QAEPAVWPuppet@@PBD@Z=?GetPuppet@CSceneManager@@QAEPAVWPuppet@@PBD@Z")

#define SCENEMGR() ((CSceneManagerEx*)CSceneManager::Instance())

unsigned char GetCurMap();
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

IMPLEMENT_ACTOR(CTrajectory, IActor)

CTrajectory::CTrajectory()
{
	m_pComet = NULL;
	m_pDoughnut = NULL;
	m_pPuttLine = NULL;
	m_pBallTrack = NULL;
	m_pSkidMark = NULL;
	m_pWindTrack = NULL;
	for (int k = 0; k < 3; k++)
		m_pActive[k] = NULL;
	m_pCurPutt = NULL;
	for (int i = 0; i < 32; i++)
		m_pPutt[i] = NULL;
	m_pCobra = NULL;
	m_pExplode = NULL;

	AddSkipList(0x8c342);
	m_dispPriority = DISP_PRIORITY_1;
}

void CTrajectory::OnDestroy()
{
	if (m_pComet)
	{
		delete m_pComet;
		m_pComet = NULL;
	}
	if (m_pDoughnut)
	{
		delete m_pDoughnut;
		m_pDoughnut = NULL;
	}
	if (m_pBallTrack)
	{
		delete m_pBallTrack;
		m_pBallTrack = NULL;
	}
	if (m_pSkidMark)
	{
		delete m_pSkidMark;
		m_pSkidMark = NULL;
	}
	if (m_pWindTrack)
	{
		delete m_pWindTrack;
		m_pWindTrack = NULL;
	}
	if (m_pPuttLine)
	{
		delete m_pPuttLine;
		m_pPuttLine = NULL;
	}

	for (int i = 0; i < 32; i++)
	{
		if (m_pPutt[i])
		{
			delete m_pPutt[i];
			m_pPutt[i] = NULL;
		}
	}

	if (m_pCobra)
	{
		delete m_pCobra;
		m_pCobra = NULL;
	}
	if (m_pExplode)
	{
		delete m_pExplode;
		m_pExplode = NULL;
	}
}

void CTrajectory::OnPreLoadInit()
{
	// 983 0079c080: push 5 (OBJ_CUSTOM), nao OBJ_EFFECT (6) do 645
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, this,
		"\xb1\xcb\xc0\xfb", 0, NULL);
}

void CTrajectory::OnLoad()
{
	m_pComet = new CComet;
	m_pDoughnut = new CDoughnut;
	m_pBallTrack = new CBallTrack;
	m_pSkidMark = new CSkidMark;
	m_pWindTrack = new CBallWindTrack;
	m_pPuttLine = new CPuttLine;
	m_pCobra = new ntSkidMark_Cobra;
	m_pExplode = new ntExplodeMark;
}

void CTrajectory::OnProcess(float delta)
{
	if (GolfBall().m_playFrame >= 0)
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_pActive[i])
				m_pActive[i]->Process(delta);
		}
	}
}

void CTrajectory::Display()
{
	for (int i = 0; i < 32; i++)
	{
		if (m_pPutt[i])
			m_pPutt[i]->Display();
	}

	if (!Skipped() && GolfBall().m_playFrame >= 0)
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_pActive[i])
				m_pActive[i]->Display();
		}

		m_pExplode->Display();
	}
}

// 983 0079b9b0 (funcao propria, chamada em cada caso do HandleMsg): sem rastro quando o tipo do jogador atual e 0xcc
// global __fastcall como no 983 (?ShowTrace@@YI_NK@Z: tipo em ecx; o chamador le Doc+jogador*0x31c6+0x3b42);
// static deixava o cl passar o tipo em eax (convencao propria) e inchava os 8 chamadores
__declspec(noinline) bool __fastcall ShowTrace(unsigned long type)
{
	return (type & 0x3ffffff) != 0xcc;
}

C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, userEquip.tidBall) == 0x3b42); // B1450: ex-cru Doc+jogador*0x31c6+0x3b42
#define CUR_TRACE_TYPE() \
	(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall)

void CTrajectory::HandleMsg(const MsgObject& msg)
{
	CBallTrace** pSlot = NULL;
	if (msg.message != M983(0x18c))
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_pActive[i] == NULL)
			{
				pSlot = &m_pActive[i];
				break;
			}
		}
		if (pSlot == NULL)
			return;
	}

	// 983: os rastros poem o "\xb1\xcb\xc0\xfb" como OBJ_CUSTOM (5), nao OBJ_EFFECT (6) do 645
	switch (msg.message)
	{
	case M983(0x158):
	{
		CBallTrace* pTrace = m_pComet;
		if (pTrace && ShowTrace(CUR_TRACE_TYPE()))
		{
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_CUSTOM)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_CUSTOM);
		}
		break;
	}

	case M983(0x15c):
	{
		CBallTrace* pTrace = m_pDoughnut;
		if (pTrace && ShowTrace(CUR_TRACE_TYPE()))
		{
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_CUSTOM)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_CUSTOM);
		}
		break;
	}

	case M983(0x159):
	{
		CBallTrace* pTrace = m_pBallTrack;
		if (pTrace && ShowTrace(CUR_TRACE_TYPE()))
		{
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_CUSTOM)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_CUSTOM);
		}
		break;
	}

	case M983(0x15a):
		if (m_pSkidMark && ShowTrace(CUR_TRACE_TYPE()))
		{
			m_pSkidMark->SetFrame(GolfBall().m_afterBurnerFrame,
				GolfBall().m_afterBurnerFrame + 75);

			CBallTrace* pTrace = m_pSkidMark;
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_FUNC)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_FUNC);
		}
		break;

	case M983(0x15b):
	{
		CBallTrace* pTrace = m_pWindTrack;
		if (pTrace && ShowTrace(CUR_TRACE_TYPE()))
		{
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_CUSTOM)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_CUSTOM);
		}
		break;
	}

	case M983(0x15d):
		if (m_pPuttLine && ShowTrace(CUR_TRACE_TYPE()))
		{
			m_pCurPutt = new CPuttLine;
			for (int i = 0; i < 32; i++)
			{
				if (m_pPutt[i] == NULL)
				{
					m_pPutt[i] = m_pCurPutt;
					break;
				}
			}

			CBallTrace* pTrace = m_pCurPutt;
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_FUNC)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_FUNC);
		}
		break;

	case M983(0x15e):
		if (m_pCobra && ShowTrace(CUR_TRACE_TYPE()))
		{
			m_pCobra->SetFrame(0, GolfBall().m_cobraFrame);

			CBallTrace* pTrace = m_pCobra;
			*pSlot = pTrace;
			pTrace->Init();

			if (SCENEMGR()->GetType("\xb1\xcb\xc0\xfb") != OBJ_FUNC)
				SCENEMGR()->SetType("\xb1\xcb\xc0\xfb", OBJ_FUNC);
		}
		break;

	case M983(0x15f):
		if (ShowTrace(CUR_TRACE_TYPE()))
		{
			m_pExplode->Init(GolfBall().m_pos);
			return;
		}
		break;

	case M983(0x18c):
	if (ShowTrace(CUR_TRACE_TYPE()))
	{
		WPuppet* pPuppet = SCENEMGR()->GetPuppet("\xb0\xf8");
		if (pPuppet && pPuppet->m_rootbone)
			pPuppet->SetAlpha(255);

		for (int k = 0; k < 3; k++)
			m_pActive[k] = NULL;
		if (m_pExplode)
			m_pExplode->Uninit();

		CPuttLine* pPutt = m_pCurPutt;
		if (pPutt)
		{
			if (!pPutt->IsKeep())
			{
				for (int i = 0; i < 32; i++)
				{
					if (m_pPutt[i] == m_pCurPutt && m_pPutt[i])
					{
						delete m_pPutt[i];
						m_pPutt[i] = NULL;
					}
				}
			}
			else
				pPutt->RemoveUnkeptAlpha();
		}

		int i;
		for (i = 0; i < 32; i++)
		{
			if (m_pPutt[i] == NULL)
				break;
		}

		if (i == 32)
		{
			int oldest = 0;
			int oldestAge = 0;
			for (i = 0; i < 32; i++)
			{
				if (m_pPutt[i] && m_pPutt[i]->HowOldAreYou() > oldestAge)
				{
					oldestAge = m_pPutt[i]->HowOldAreYou();
					oldest = i;
				}
			}

			if (m_pPutt[oldest])
			{
				delete m_pPutt[oldest];
				m_pPutt[oldest] = NULL;
			}
		}
		break;
	}

	case M983(0x160):
	if (ShowTrace(CUR_TRACE_TYPE()))
	{
		WPuppet* pPuppet = SCENEMGR()->GetPuppet("\xb0\xf8");
		if (pPuppet && pPuppet->m_rootbone)
			pPuppet->SetAlpha(255);

		m_pCurPutt = NULL;
		for (int i = 0; i < 32; i++)
		{
			if (m_pPutt[i])
			{
				delete m_pPutt[i];
				m_pPutt[i] = NULL;
			}
		}
		break;
	}
	}
}

CBallTrace::CBallTrace()
	: m_unused4(0), m_texHandle(0), m_bActive(false)
{
	m_pVtx[0] = &m_vtx[0];
	m_pVtx[1] = &m_vtx[1];
	m_pVtx[2] = &m_vtx[3];
	m_pVtx[3] = &m_vtx[2];
	m_pVtx[4] = NULL;
}

CBallTrack::CBallTrack()
	: m_pPoint(NULL), m_bRainbow(false), m_pPointEx(NULL), m_rainbowTex(0)
{
	m_texHandle = g_resrcmng->LoadTexture("[swingtale01.jpg", 0, 0, 0);
}

CBallTrack::~CBallTrack()
{
	if (m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (g_resrcmng && m_rainbowTex)
	{
		g_resrcmng->Release(m_rainbowTex);
		m_rainbowTex = 0;
	}
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (m_pPointEx)
	{
		delete[] m_pPointEx;
		m_pPointEx = NULL;
	}
}

void CBallTrack::Init()
{
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (m_pPointEx)
	{
		delete[] m_pPointEx;
		m_pPointEx = NULL;
	}

	if ((Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall &
			0x3ffffff) == 0x20)
	{
		m_bRainbow = true;
		m_pPointEx = new sPointEx[GolfBall().m_renderNum + 1];
		if (m_rainbowTex == 0)
			m_rainbowTex = g_resrcmng->LoadTexture("[rainbow_001.jpg", 0, 0, 0);
	}
	else
		m_bRainbow = false;

	m_pPoint = new sPoint[GolfBall().m_renderNum + 1];
	m_bActive = true;

	float fade = (GolfBall().m_renderNum - GolfBall().m_bounceFrame) * 0.02f;
	if (fade < 0.5f)
		fade = 0.5f;
	else if (fade > 2.0f)
		fade = 2.0f;
	m_fadeTime = fade;
	m_maxAlpha = 255.0f;
	m_alpha = 255.0f;
	m_fadeStart = GolfBall().m_bounceFrame * 0.02f + m_fadeTime - 1.0f;
	m_lastFrame = 0;
	m_pointNum = 1;

	WVector side =
		WCrossProduct(WVector::UNIT_POS_Y, GolfBall().PreData(0)->vel);
	side.Normalize();
	side *= GolfBall().m_radius * 1.2f;

	m_pPoint[0].left = side + GolfBall().PreData(0)->pos;
	m_pPoint[0].right = GolfBall().PreData(0)->pos - side;

	if (m_bRainbow)
	{
		m_pPointEx[0].pos = GolfBall().PreData(0)->pos;
		m_pPointEx[0].dir = side;
		m_pPointEx[0].age = 0.0f;
		m_pPointEx[0].bActive = 1;
		for (int i = 1; i < GolfBall().m_renderNum + 1; i++)
		{
			m_pPointEx[i].bActive = 0;
			m_pPointEx[i].age = 0.0f;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
	}
}

void CBallTrack::Process(float elapsed)
{
	if (m_bActive)
	{
		int frame = GolfBall().m_playFrame;
		if (m_lastFrame != frame)
		{
			for (int i = m_lastFrame + 1; i <= frame; i++)
			{
				if (i - m_lastFrame < 5)
				{
					if (!GolfBall().PreData(i)->event &&
						(GolfBall().PreData(i)->pos -
							GolfBall().PreData(m_lastFrame)->pos)
								.Magnitude() < 3.2f)
						continue;
				}

				if (GolfBall().m_state != 0)
				{
					WVector side;
					side = WCrossProduct(WVector::UNIT_POS_Y,
						GolfBall().PreData(i)->vel);
					side.Normalize();
					side *= GolfBall().m_radius * 1.2f;

					m_pPoint[m_pointNum].left =
						side + GolfBall().PreData(i)->pos;
					m_pPoint[m_pointNum].right =
						GolfBall().PreData(i)->pos - side;
					m_pointNum++;
					m_lastFrame = i;

					if (m_bRainbow)
					{
						m_pPointEx[m_pointNum].pos = GolfBall().PreData(i)->pos;
						m_pPointEx[m_pointNum].dir = side;
						m_pPointEx[m_pointNum].bActive = 1;
					}
				}
			}
		}

		if (m_bRainbow)
			UpdateRainbowEffect(elapsed);

		if (m_lastFrame < GolfBall().m_renderNum && GolfBall().m_state != 0)
		{
			WVector side;
			side = WCrossProduct(WVector::UNIT_POS_Y,
				GolfBall().PreData(frame)->vel);
			side.Normalize();
			side *= GolfBall().m_radius * 1.2f;

			m_pPoint[m_pointNum].left = side + GolfBall().m_pos;
			m_pPoint[m_pointNum].right = GolfBall().m_pos - side;

			if (m_bRainbow)
			{
				m_pPointEx[m_pointNum].pos = GolfBall().m_pos;
				m_pPointEx[m_pointNum].dir = side;
				m_pPointEx[m_pointNum].bActive = 1;
			}
		}

		float speed = 0.6f;
		if (m_lastFrame > GolfBall().m_renderNum - 30)
			speed = 1.2f;
		for (int i = 0; i <= m_pointNum; i++)
		{
			m_pPoint[i].alpha -= speed * elapsed;
			if (m_pPoint[i].alpha < 0.0f)
				m_pPoint[i].alpha = 0.0f;
		}

		float fadeStart = m_fadeStart;
		m_fadeStart = fadeStart - elapsed;
		if (m_fadeStart < m_fadeTime)
			m_alpha -= (1.0f / m_fadeTime) * m_maxAlpha * elapsed;
		else
			m_alpha = m_maxAlpha;
	}
}

void CBallTrack::Display()
{
	if (m_bActive)
	{
		unsigned int tex = m_texHandle;
		int extra = m_lastFrame < GolfBall().m_renderNum;
		if (m_bRainbow)
			tex = m_rainbowTex;

		for (int i = 1; i < m_pointNum + extra; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				int idx = (j & 2) ? i : i - 1;
				m_vtx[j].diffuse =
					((int)(m_pPoint[idx].alpha * 255.0f) << 24) | 0xffffff;
				const WVector& pos =
					(j & 1) ? m_pPoint[idx].right : m_pPoint[idx].left;
				m_vtx[j].SetPosition(pos);
			}

			g_view->DrawPolygonFan(m_pVtx, (tex & 0x7ff) | 0xc00000, 0x200, 0);
		}
	}
}

void CBallTrack::UpdateRainbowEffect(float elapsed)
{
	for (int i = 0; i < m_pointNum; i++)
	{
		if (m_pPointEx[i].bActive == 1)
			m_pPointEx[i].age += elapsed * 5.0f;

		if (m_pPointEx[i].age > 1.0f)
		{
			m_pPoint[i].left =
				m_pPointEx[i].dir * m_pPointEx[i].age + m_pPointEx[i].pos;
			m_pPoint[i].right =
				m_pPointEx[i].pos - m_pPointEx[i].dir * m_pPointEx[i].age;
		}
	}
}

CSkidMark::CSkidMark()
	: m_pPoint(NULL), m_startFrame(-1), m_endFrame(-1)
{
	m_pointMax = 0;
	m_pointNum = 0;
	m_texHandle = g_resrcmng->LoadTexture("~skidmark01.jpg", 0, 0, 0);
}

CSkidMark::~CSkidMark()
{
	if (m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	m_pointMax = 0;
	m_pointNum = 0;
}

void CSkidMark::SetFrame(int startFrame, int endFrame)
{
	m_startFrame = Max(startFrame, GolfBall().m_playFrame);
	m_endFrame = Min(endFrame, GolfBall().m_renderNum);
}

unsigned char CSkidMark::GetPointAlpha(int index, int frame)
{
	int alpha;
	if (index > 0 && frame > 0)
	{
		WVector a = GolfBall().PreData(frame)->pos;
		WVector b = GolfBall().PreData(frame - 1)->pos;
		if ((a - b).Magnitude() < 0.01f)
		{
			alpha = (int)(m_pPoint[index - 1].alpha * 1.1);
			if (alpha >= 255)
				alpha = 255;
			return alpha;
		}
	}

	return (int)(((float)(m_pointMax - index) / (float)m_pointMax) * 192.0f);
}

void CSkidMark::Init()
{
	if (m_startFrame < GolfBall().m_playFrame)
		m_startFrame = GolfBall().m_playFrame;
	if (m_endFrame < 0 || m_endFrame > GolfBall().m_renderNum)
		m_endFrame = GolfBall().m_renderNum;

	m_pointMax = m_endFrame - m_startFrame + 1;
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (m_pointMax > 0)
		m_pPoint = new sPoint[m_pointMax];

	m_pointNum = 0;
	m_lastFrame = m_startFrame - 1;

	for (int i = 0; i < 4; i++)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
	}
	m_bActive = true;
}

void CSkidMark::Process(float elapsed)
{
	if (m_bActive)
	{
		int frame = GolfBall().m_playFrame;
		if (frame >= m_startFrame)
		{
			WVector side = WCrossProduct(WVector::UNIT_POS_Y,
				GolfBall().PreData(frame)->vel);
			side.Normalize();
			float scale = GolfBall().m_radius * 1.2f;
			side *= scale;

			for (int i = m_lastFrame + 1; i <= frame; i++)
			{
				if (m_pointNum < m_pointMax)
				{
					WVector pos = GolfBall().PreData(i)->pos;
					float gy = GetPVS().GetGroundPoint(pos, true, NULL, true).y;
					WVector left(side.x + pos.x, side.y + gy, side.z + pos.z);
					WVector right(pos.x - side.x, gy - side.y, pos.z - side.z);
					m_pPoint[m_pointNum].left = left;
					m_pPoint[m_pointNum].right = right;
					m_pPoint[m_pointNum].alpha = GetPointAlpha(m_pointNum, i);
					m_pointNum++;
				}
			}

			m_lastFrame = frame;
			if (frame < m_endFrame && m_pointNum < m_pointMax)
			{
				WVector pos = GolfBall().m_pos;
				float gy = GetPVS().GetGroundPoint(pos, true, NULL, true).y;
				m_pPoint[m_pointNum].left =
					WVector(pos.x + side.x, side.y + gy, pos.z + side.z);
				WVector right(pos.x - side.x, gy - side.y, pos.z - side.z);
				m_pPoint[m_pointNum].right = right;
				m_pPoint[m_pointNum].alpha =
					GetPointAlpha(m_pointNum, m_lastFrame);
			}
		}
	}
}

void CSkidMark::Display()
{
	if (m_bActive)
	{
		int num = (m_lastFrame < m_endFrame) ? m_pointNum + 1 : m_pointNum;
		int count = (m_pointMax <= num) ? m_pointMax : num;

		for (int i = 1; i < count; i++)
		{
			unsigned int a = m_pPoint[i].alpha;
			unsigned long color = (((a | 0xffffff00) << 8 | a) << 8) | a;
			for (int j = 0; j < 4; j++)
			{
				m_vtx[j].diffuse = color;
				int idx = (j & 2) ? i : i - 1;
				const WVector& pos =
					(j & 1) ? m_pPoint[idx].right : m_pPoint[idx].left;
				m_vtx[j].SetPosition(pos);
			}

			g_view->DrawPolygonFan(m_pVtx, (m_texHandle & 0x7ff) | 0x21b00000,
				0x200, 0);
		}
	}
}

CBallWindTrack::CBallWindTrack()
	: m_pLine(NULL)
{
	m_prevCamera.Reset();
	m_dir.Reset();
	m_pLine = new sLineEffect[256];
}

CBallWindTrack::~CBallWindTrack()
{
	if (m_pLine)
	{
		delete[] m_pLine;
		m_pLine = NULL;
	}
}

void CBallWindTrack::Init()
{
	m_track.Init();
	m_bActive = true;
	for (int i = 0; i < 256; i++)
		m_pLine[i].bActive = false;
}

void CBallWindTrack::Process(float elapsed)
{
	m_track.Process(elapsed);
	if (m_bActive)
	{
		if (GolfBall().m_bounceFrame > 0)
		{
			if (GolfBall().m_playFrame >
				(GolfBall().m_topFrame + GolfBall().m_bounceFrame) / 2)
			{
				m_bActive = false;
				return;
			}
		}

		m_dir = g_camera.pivot - m_prevCamera;
		float len = m_dir.Magnitude();
		if (len < 1.0f)
		{
			m_prevCamera = g_camera.pivot;
			return;
		}

		m_dir.Normalize();
		for (int i = 0; i < 256; i++)
		{
			if (m_pLine[i].bActive)
			{
				if ((m_pLine[i].from - g_camera.pivot) * g_camera.za < 0.0f ||
					(m_pLine[i].from - GolfBall().m_pos).Magnitude() > 160.0f)
					m_pLine[i].bActive = false;
				else
				{
					float dist = // 983 0079f264: fdiv pelo double 200.0 (cff708)
						(float)((m_pLine[i].from - g_camera.pivot).Magnitude() / 200.0);
					if (dist > 1.0f)
						dist = 1.0f;
					m_pLine[i].alpha = (int)(128.0f * dist);
					m_pLine[i].from -= m_dir * len * 0.5f;
					m_pLine[i].to = m_pLine[i].from - m_dir * m_pLine[i].length;
				}
			}
			else
			{
				// 983 0079f37c: o 1o Random vai para x (o nosso sorteava z primeiro)
				float rx = Random(-1.0f, 1.0f);
				float ry = Random(-1.0f, 1.0f);
				float rz = Random(-1.0f, 1.0f);
				WVector rnd(rx * 75.0f, ry * 75.0f, rz * 75.0f);
				m_pLine[i].from = m_dir * 100.0f + g_camera.pivot + rnd;
				m_pLine[i].length = Random(-1.0f, 1.0f) * 30.0f + 100.0f;
				m_pLine[i].bActive = true;
				m_pLine[i].alpha = 0;
			}
		}

		WVector vel = GolfBall().m_vel;
		vel.Normalize();
		m_alpha = (vel.y > 0.0f) ? 1.0f : vel.y + 0.5f;
		m_prevCamera = g_camera.pivot;
	}
}


void CBallWindTrack::Display()
{
	m_track.Display();
	if (m_bActive && m_alpha > 0.0f && m_dir.Magnitude() != 0.0f)
	{
		for (int i = 0; i < 256; i++)
		{
			if (m_pLine[i].bActive)
			{
				g_view->DrawLine(m_pLine[i].from,
					((int)((float)m_pLine[i].alpha * m_alpha) << 24) | 0xffffff,
					m_pLine[i].to, 0, 0xc00000);
			}
		}
	}
}


CComet::CComet()
{
	m_cometTex = g_resrcmng->LoadTexture("~ball_straight.jpg", 0, 0, 0);
	m_interval = 0.1f;
	m_count = 20;
	for (int i = 0; i < 4; i++)
		m_vtx[i].diffuse = 0xffffffff;
}

CComet::~CComet()
{
	if (m_cometTex)
	{
		g_resrcmng->Release(m_cometTex);
		m_cometTex = 0;
	}
}

void CComet::Init()
{
	m_length = 1.0f;
	m_bTail = 0;
	m_head = 0;
	m_alpha = 255.0f;
	m_time = 0.0f;
	m_bActive = true;

	WVector side = WCrossProduct(GolfBall().m_vel, g_camera.za);
	side = side.Normalize() * 2.0f * GolfBall().m_radius;

	m_line[0][0] = side + GolfBall().m_pos;
	m_line[0][1] = GolfBall().m_pos - side;
}

void CComet::Generate()
{
	WVector side = WCrossProduct(GolfBall().m_vel, g_camera.za);
	side = side.Normalize() * 2.0f * GolfBall().m_radius;

	m_head = (m_head + 1) % m_count;
	m_line[m_head][0] = side + GolfBall().m_pos;
	m_line[m_head][1] = GolfBall().m_pos - side;
	m_bTail = 0;
	if ((int)m_length < m_count)
		m_length += 1.0f;
}

void CComet::Process(float elapsed)
{
	if (m_bActive)
	{
		if (GolfBall().m_vel.y < 0.0f)
		{
			m_length = (m_length - 0.5f > 0.0f) ? m_length - 0.5f : 0.0f;
			float step = Min(elapsed, elapsed * m_alpha * 0.005f);
			m_time += step;
			m_alpha = Max(0.0f, m_alpha - elapsed * 150.0f);
			if (Wabs(m_alpha) < g_EPSILON)
			{
				m_bActive = false;
				m_alpha = 0.0f;
			}
		}
		else
			m_time += elapsed;

		if (m_time > m_interval)
		{
			m_time -= m_interval;
			Generate();
			return;
		}

		WVector side = WCrossProduct(GolfBall().m_vel, g_camera.za);
		side.Normalize();
		side *= GolfBall().m_radius * 2.0f;

		m_tailLeft = side + GolfBall().m_pos;
		m_tailRight = GolfBall().m_pos - side;
		m_bTail = 1;
	}
}


void CComet::Display()
{
	if (m_bActive)
	{
		unsigned long color = ((int)m_alpha << 24) | 0xffffff;
		for (int i = 0; i < (int)m_length - 1; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				int idx = m_head - i;
				int back = j & 2;
				int cnt = m_count;
				idx = back ? idx + cnt - 1 : idx + cnt;
				m_vtx[j].SetPosition(m_line[idx % cnt][j & 1]);
				if (back)
					m_vtx[j].tu = (float)(m_bTail + i + 1) /
						(float)((int)m_length + m_bTail);
				else
					m_vtx[j].tu = (float)(m_bTail + i) /
						(float)((int)m_length + m_bTail);
				m_vtx[j].tv = (j & 1) ? 0.0f : 1.0f;
				m_vtx[j].diffuse = color;
			}

			g_view->DrawPolygonFan(m_pVtx,
				(m_cometTex & 0x7ff) | 0xc00000, 0x200, 0);
		}

		if (m_bTail && m_length >= 1.0f)
		{
			m_vtx[0].SetPosition(m_tailLeft);
			m_vtx[0].diffuse = color;
			m_vtx[0].tu = 0.0f;
			m_vtx[0].tv = 1.0f;
			m_vtx[1].SetPosition(m_tailRight);
			m_vtx[1].diffuse = color;
			m_vtx[1].tu = 0.0f;
			m_vtx[1].tv = 0.0f;
			m_vtx[2].SetPosition(m_line[m_head][0]);
			m_vtx[2].diffuse = color;
			m_vtx[2].tu = 1.0f / (m_length + 1.0f);
			m_vtx[2].tv = 1.0f;
			m_vtx[3].SetPosition(m_line[m_head][1]);
			m_vtx[3].diffuse = color;
			m_vtx[3].tu = 1.0f / (m_length + 1.0f);
			m_vtx[3].tv = 0.0f;

			g_view->DrawPolygonFan(m_pVtx, (m_cometTex & 0x7ff) | 0xc00000,
				0x200, 0);
		}
	}
}


CDoughnut::CDoughnut()
{
	m_count = 10;
	m_pParticle = new sParticle[m_count];
	m_texHandle = g_resrcmng->LoadTexture("~ball_circle01.jpg", 0, 0, 0);
	m_interval = 0.1f;
}

CDoughnut::~CDoughnut()
{
	if (m_pParticle)
	{
		delete[] m_pParticle;
		m_pParticle = NULL;
	}
	if (m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
}

void CDoughnut::Init()
{
	for (int i = 0; i < m_count; i++)
		m_pParticle[i].bActive = false;
	m_time = 0.0f;
	m_head = 0;
	m_bActive = true;
}

void CDoughnut::Process(float elapsed)
{
	if (m_bActive)
	{
		if (GolfBall().m_bounceFrame > 0)
		{
			m_bActive = false;
			return;
		}

		for (int i = 0; i < m_count; i++)
		{
			if (m_pParticle[i].bActive)
			{
				m_pParticle[i].alpha -= elapsed * 255.0f;
				m_pParticle[i].scale -= elapsed * 0.8f;
				if (m_pParticle[i].alpha < 0.0f)
					m_pParticle[i].bActive = false;
			}
		}

		m_time += elapsed;
		if (m_time > m_interval)
		{
			m_time -= m_interval;
			m_pParticle[m_head].bActive = true;
			m_pParticle[m_head].alpha = 255.0f;
			m_pParticle[m_head].scale = 1.0f;
			WVector target = GolfBall().m_pos + GolfBall().m_vel;
			m_pParticle[m_head].matrix =
				MakeCameraMatrix(GolfBall().m_pos, target);
			m_head = (m_head + 1) % m_count;
		}
	}
}

void CDoughnut::Display()
{
	if (m_bActive)
	{
		for (int i = 0; i < m_count; i++)
		{
			if (m_pParticle[i].bActive)
			{
				for (int j = 0; j < 4; j++)
				{
					WVector pos;
					pos = m_pParticle[i].matrix.pivot;
					WVector offset =
						(((j & 2) ? 1.0f : -1.0f) * m_pParticle[i].matrix.xa +
							((j & 1) ? 1.0f : -1.0f) *
								m_pParticle[i].matrix.ya) *
						(m_pParticle[i].scale * 0.5f);
					pos += offset;
					m_vtx[j].SetPosition(pos);
					m_vtx[j].diffuse =
						(int)(m_pParticle[i].alpha) * 0x1000000 + 0xffffff;
					m_vtx[j].tu = (j & 2) ? 1.0f : 0.0f;
					m_vtx[j].tv = (j & 1) ? 0.0f : 1.0f;
				}

				g_view->DrawPolygonFan(m_pVtx, (m_texHandle & 0x7ff) | 0xc00000,
					0x200, 0);
			}
		}
	}
}

CPuttLine::CPuttLine()
	: m_pPoint(NULL)
{
}

CPuttLine::~CPuttLine()
{
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
}

void CPuttLine::Init()
{
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}

	m_pPoint = new sPoint[GolfBall().m_renderNum + 1];
	m_pointNum = GolfBall().m_renderNum;
	m_startTime = timeGetTime();
	m_bActive = true;

	for (int i = 0; i <= GolfBall().m_renderNum; i++)
	{
		WVector side =
			WCrossProduct(WVector::UNIT_POS_Y, GolfBall().PreData(i)->vel);
		side.Normalize();
		side *= GolfBall().m_radius * rb_real_3ee66666;

		m_pPoint[i].left = side + GolfBall().PreData(i)->pos;
		m_pPoint[i].right = GolfBall().PreData(i)->pos - side;
		m_pPoint[i].left.y -= GolfBall().m_radius;
		m_pPoint[i].right.y -= GolfBall().m_radius;
		m_pPoint[i].alpha = 0;
	}
}

void CPuttLine::Process(float elapsed)
{
	if (Doc()->m_pGolfDoc->m_gameMode == 8)
		m_bActive = false;

	if (m_bActive && m_pPoint)
	{
		if (GolfBall().m_state != 0)
		{
			int start = Max(0, GolfBall().m_playFrame - 150);
			int end = Max(0, GolfBall().m_playFrame - 25);
			for (int i = 0; i <= GolfBall().m_playFrame; i++)
			{
				if (GolfBall().PreData(i)->state == CGolfBall::BALL_HOLE)
					m_pPoint[i].alpha = 0;
				else
				{
					if (GetCurMap() == 10)
					{
						if (GolfBall().PreData(i)->state !=
							CGolfBall::BALL_ROLL)
							m_pPoint[i].alpha = 0;
						else
						{
							unsigned char ground = GetPVS().GetGroundType(
								GolfBall().PreData(i)->pos, true, NULL);
							if (ground != 3 && ground != 4 && ground != 11)
								goto fade;
							m_pPoint[i].alpha = 0x100;
						}
					}
					else
					{
fade:
						if (i <= start)
							m_pPoint[i].alpha = 0;
						else if (i >= end)
							m_pPoint[i].alpha = 0x40;
						else if (end == start)
							m_pPoint[i].alpha = 0;
						else
							m_pPoint[i].alpha =
								(i - start) * 0x40 / (end - start);
					}
				}
			}
		}
	}
}

void CPuttLine::Display()
{
	if (m_bActive && m_pPoint)
	{
		for (int i = 1; i <= m_pointNum; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				int idx = (j & 2) ? i : i - 1;
				const WVector& pos =
					(j & 1) ? m_pPoint[idx].right : m_pPoint[idx].left;
				m_vtx[j].SetPosition(pos);
				if (m_pPoint[idx].alpha == 0x100)
					m_vtx[j].diffuse = 0x80e6c882;
				else
					m_vtx[j].diffuse = (m_pPoint[idx].alpha << 24) | 0xffffff;
			}

			g_view->DrawPolygonFan(m_pVtx, 0x20300000, 0x200, 0);
		}
	}
}

bool CPuttLine::IsKeep()
{
	int i;
	for (i = 0; i <= m_pointNum; i++)
	{
		if (m_pPoint[i].alpha == 0x100)
			break;
	}
	if (i != m_pointNum)
		return true;
	return false;
}

int CPuttLine::HowOldAreYou()
{
	return timeGetTime() - m_startTime;
}

void CPuttLine::RemoveUnkeptAlpha()
{
	for (int i = 0; i <= m_pointNum; i++)
	{
		if (m_pPoint[i].alpha != 0x100)
			m_pPoint[i].alpha = 0;
	}
}

ntSkidMark_Cobra::ntSkidMark_Cobra()
	: m_pPoint(NULL), m_startFrame(-1), m_endFrame(-1)
{
	m_pointMax = 0;
	m_pointNum = 0;
	m_texHandle = g_resrcmng->LoadTexture("[_cobra_ef.jpg", 0, 0, 0);
}

ntSkidMark_Cobra::~ntSkidMark_Cobra()
{
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
	m_pointMax = 0;
	m_pointNum = 0;
}

void ntSkidMark_Cobra::Init()
{
	if (m_startFrame < GolfBall().m_playFrame)
		m_startFrame = GolfBall().m_playFrame;
	if (m_endFrame < 0 || m_endFrame > GolfBall().m_cobraFrame)
		m_endFrame = GolfBall().m_renderNum;

	m_pointMax = m_endFrame - m_startFrame + 1;
	if (m_pPoint)
	{
		delete[] m_pPoint;
		m_pPoint = NULL;
	}
	if (m_pointMax > 0)
		m_pPoint = new sPoint[m_pointMax];

	m_pointNum = 0;
	m_lastFrame = m_startFrame - 1;

	for (int i = 0; i < 4; i++)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
	}
	m_bActive = true;
}

void ntSkidMark_Cobra::Process(float elapsed)
{
	if (m_bActive)
	{
		int frame = (GolfBall().m_playFrame < GolfBall().m_cobraFrame)
			? GolfBall().m_playFrame
			: GolfBall().m_cobraFrame;
		if (frame >= m_startFrame)
		{
			WVector side = WCrossProduct(WVector(0.0f, 1.0f, 0.0f),
				GolfBall().PreData(frame)->vel);
			side.Normalize();
			float scale = GolfBall().m_radius * 10.0f;
			side *= scale;

			for (int i = m_lastFrame + 1; i <= frame; i++)
			{
				if (m_pointNum < m_pointMax)
				{
					WVector pos = GolfBall().PreData(i)->pos;
					float gy = GetPVS().GetGroundPoint(pos, true, NULL, true).y;
					WVector left(side.x + pos.x, side.y + gy, side.z + pos.z);
					WVector right(pos.x - side.x, gy - side.y, pos.z - side.z);
					m_pPoint[m_pointNum].left = left;
					m_pPoint[m_pointNum].right = right;
					m_pointNum++;
				}
			}

			m_lastFrame = frame;
			if (frame < m_endFrame && m_pointNum < m_pointMax)
			{
				WVector pos = GolfBall().m_pos;
				float gy = GetPVS().GetGroundPoint(pos, true, NULL, true).y;
				m_pPoint[m_pointNum].left =
					WVector(pos.x + side.x, side.y + gy, pos.z + side.z);
				m_pPoint[m_pointNum].right =
					WVector(pos.x - side.x, gy - side.y, pos.z - side.z);
			}
		}
	}
}

void ntSkidMark_Cobra::Display()
{
	if (m_bActive)
	{
		int num = (m_lastFrame < m_endFrame) ? m_pointNum + 1 : m_pointNum;
		int count = (m_pointMax <= num) ? m_pointMax : num;

		for (int i = 1; i < count; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_vtx[j].diffuse = 0xffffffff;
				int idx = (j & 2) ? i : i - 1;
				const WVector& pos =
					(j & 1) ? m_pPoint[idx].right : m_pPoint[idx].left;
				m_vtx[j].SetPosition(pos);
			}

			g_view->DrawPolygonFan(m_pVtx, (m_texHandle & 0x7ff) | 0x20200000,
				0, 0);
		}
	}
}

void ntSkidMark_Cobra::SetFrame(int startFrame, int endFrame)
{
	m_startFrame = Max(startFrame, GolfBall().m_playFrame);
	m_endFrame = Min(endFrame, GolfBall().m_renderNum);
}

ntExplodeMark::ntExplodeMark()
{
	m_bActive = false;
	m_texHandle = g_resrcmng->LoadTexture("[_boom.jpg", 0, 0, 0);
}

ntExplodeMark::~ntExplodeMark()
{
	if (m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
}

void ntExplodeMark::Init(const WVector& pos)
{
	m_bActive = true;
	m_pos = pos;
}

void ntExplodeMark::Uninit()
{
	m_bActive = false;
}

void ntExplodeMark::Process()
{
}

void ntExplodeMark::Display()
{
	if (m_bActive)
	{
		WVector axisZ(0.0f, 0.0f, 15.0f);
		WVector axisX(15.0f, 0.0f, 0.0f);
		GetPVS().DrawDecal(m_texHandle,
			GetPVS().GetGroundPoint(m_pos, false, NULL, false), axisX, axisZ,
			1.0f, 1.0f, W_POLY_BLEND_NORMAL, false);
	}
}
