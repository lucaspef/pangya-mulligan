#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "capturedbg.h"
#include "scenemanager.h"
#include "fx.h"
#include "fxsequence.h"
#include "mathconsts.h"
#include "guideline.h"

IMPLEMENT_ACTOR(CGuideLine, IActor)


CGuideLine::CGuideLine()
	: m_texHandle(0)
{
	AddSkipList(0x85806);
}

void CGuideLine::OnPreLoadInit()
{
	m_pVtx[0] = &m_vtx[0];
	m_pVtx[1] = &m_vtx[1];
	m_pVtx[2] = &m_vtx[3];
	m_pVtx[3] = &m_vtx[2];
	m_pVtx[4] = NULL;

	for (int i = 0; i < MAX_POINT; i++)
		m_point[i].pSeq = NULL;

	CSceneManager::Instance()->RegisterElement(OBJ_FUNC, NULL, this,
		"\xb0\xa1\xc0\xcc\xb5\xe5\xb6\xf3\xc0\xce", 0, NULL);

	m_texHandle = g_resrcmng->LoadTexture("~Track.jpg", 0, 0, 0);
}

void CGuideLine::OnLoad()
{
}

void CGuideLine::OnInit()
{
	m_bHidden = m_bHideReq = false;
	m_bActive = false;
	m_lifeTime = 0.0f;
	m_pointNum = 0;
	m_bDelay = false;
	m_color983 = -1;
}

void CGuideLine::OnDestroy()
{
	for (int i = 0; i < MAX_POINT; i++)
	{
		if (m_point[i].pSeq)
		{
			CFx::Instance()->CloseSequence(m_point[i].pSeq, true);
			delete m_point[i].pSeq;
			m_point[i].pSeq = NULL;
		}
	}

	if (g_resrcmng && m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
}

void CGuideLine::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode < 15 || GolfBall().m_groundType != 2 ||
		!m_bActive || CCapturedBg::Instance()->IsCaptured())
		return;

	if (m_bHidden && !m_bHideReq)
		CalcSegment();

	if (m_lifeTime > 0.0f)
	{
		m_lifeTime -= delta;
		if (m_lifeTime < 0.0f)
			SetActive(false);
	}

	m_bHidden = m_bHideReq;
	m_bHideReq = false;

	if (m_alpha > 0.0f && m_alpha < 255.0f)
	{
		m_alpha = Min(m_alpha + delta * 255.0f, 255.0f);
		for (int i = 0; i < 4; i++)
			m_vtx[i].diffuse = ((int)m_alpha << 24) | 0xffffff;
	}
}

void CGuideLine::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x1a0):
		SetActive(false);
		break;

	case M983(0x1a2):
		m_color983 = msg.param2 ? msg.param2 : -1;
		SetActive(msg.param1 != 0);
		break;

	case M983(0x1a1):
		if (m_bActive)
		{
			if (!m_bHidden)
			{
				for (int i = 0; i < MAX_POINT; i++)
				{
					if (!m_point[i].pSeq)
						break;

					CFx::Instance()->CloseSequence(m_point[i].pSeq, true);
					delete m_point[i].pSeq;
					m_point[i].pSeq = NULL;
				}
			}
			m_bHideReq = true;
		}
		break;
	}
}

void CGuideLine::SetActive(bool bActive)
{
	if (m_bActive)
	{
		for (int i = 0; i < MAX_POINT; i++)
		{
			if (!m_point[i].pSeq)
				break;

			CFx::Instance()->CloseSequence(m_point[i].pSeq, true);
			delete m_point[i].pSeq;
			m_point[i].pSeq = NULL;
		}
	}

	m_bActive = bActive;
	if (bActive)
	{
		m_bHidden = false;
		m_bHideReq = false;
		m_bDelay = true;
		CalcSegment();
		m_lifeTime = m_pointNum * 0.05f + (m_color983 == 0xffffffff ? 5.0f : 3.75f);
	}
}

void CGuideLine::Display()
{
	if (Skipped() || GOLFDOC()->m_tutorialMode < 15 ||
		GolfBall().m_groundType != 2 || m_bHidden || !m_bActive ||
		CCapturedBg::Instance()->IsCaptured())
		return;

	// 983 0075b3f0: cada vertice recebe a cor do 0x1a2 (m_color983); o 645 so usava o diffuse do fade (OnProcess)
	for (int i = 0; i < m_pointNum - 1; i++)
	{
		m_vtx[0].SetPosition(m_point[i].left);
		m_vtx[0].tu = 0.0f;
		m_vtx[0].tv = (0.0f + i) * 0.1f;
		m_vtx[0].diffuse = m_color983;
		m_vtx[1].SetPosition(m_point[i].right);
		m_vtx[1].tu = 1.0f;
		m_vtx[1].tv = (0.0f + i) * 0.1f;
		m_vtx[1].diffuse = m_color983;
		m_vtx[2].SetPosition(m_point[i + 1].left);
		m_vtx[2].tu = 0.0f;
		m_vtx[2].tv = (i + 1.0f) * 0.1f;
		m_vtx[2].diffuse = m_color983;
		m_vtx[3].SetPosition(m_point[i + 1].right);
		m_vtx[3].tu = 1.0f;
		m_vtx[3].tv = (i + 1.0f) * 0.1f;
		m_vtx[3].diffuse = m_color983;

		g_view->DrawPolygonFan(m_pVtx, (m_texHandle & 0x7ff) | 0x20b00000,
			0x200, false);
	}
}

void CGuideLine::CalcSegment()
{
	if (GolfBall().m_groundType != 2)
		return;

	GolfBall().Reset(GolfBall().m_pos);

	unsigned char cur = GOLFDOC()->m_currentPlayer;
	sPlayerData* player = GOLFDOC()->GetPlayer(cur);
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector dir = pin - player->pos;

	float dist = sqrtf(dir.x * dir.x + dir.z * dir.z);
	float power =
		sqrtf(dist / (GolfClub().GetRange() * 3.2f)) * GolfClub().GetPower();

	float angle;
	SendMsg(this, "Player", M983(0x110), (int)&angle, 0, 0, 0);

	GolfBall().m_vel = RotMat(angle, 1).za * power;
	GolfBall().m_state = CGolfBall::BALL_ROLL;
	GolfBall().m_rollFrame = 0;

	m_alpha = 1.0f;

	m_pointNum = 0;
	SetPoint(GOLFDOC()->m_cameraData.mat.za * 1.6f);

	while (m_pointNum < MAX_POINT && GolfBall().m_state == CGolfBall::BALL_ROLL)
	{
		CGolfBall::sExpected* pre = GolfBall().NewPreData();
		GetPVS().BallProcess(0.02f);
		pre->pos = GolfBall().m_pos;

		WVector step;
		step = GolfBall().m_pos - m_point[m_pointNum - 1].pos;
		step.y = 0.0f;
		step.Normalize();
		step *= 1.6f;

		WVector toNext;
		toNext = m_point[m_pointNum - 1].pos + step - GolfBall().m_pos;
		WVector toPrev;
		toPrev = m_point[m_pointNum - 1].pos - GolfBall().m_pos;

		while (m_pointNum < MAX_POINT &&
			toNext.x * toPrev.x + toNext.z * toPrev.z > 0.0f)
		{
			SetPoint(step);

			toNext = m_point[m_pointNum - 1].pos + step - GolfBall().m_pos;
			toPrev = m_point[m_pointNum - 1].pos - GolfBall().m_pos;
		}
	}

	GolfBall().Reset(m_point[0].pos);
	GolfBall().m_groundType = 2;

	m_bDelay = false;
}

void CGuideLine::SetPoint(const WVector& dir)
{
	if (m_pointNum == 0)
	{
		m_point[m_pointNum].pos = GolfBall().m_pos;
	}
	else
	{
		m_point[m_pointNum].pos =
			GetPVS().GetGroundPoint(m_point[m_pointNum - 1].pos + dir, false,
				NULL, false) +
			WVector(0.0f, GolfBall().GetRadius(), 0.0f);
	}

	WVector side = WCrossProduct(dir, WVector::UNIT_POS_Y).Normalize();
	side *= GolfBall().GetRadius();

	m_point[m_pointNum].left = m_point[m_pointNum].pos + side;
	m_point[m_pointNum].right = m_point[m_pointNum].pos - side;
	m_point[m_pointNum].left.y -= GolfBall().GetRadius();
	m_point[m_pointNum].right.y -= GolfBall().GetRadius();

	// 983 0075b700: fecha a seq anterior do ponto antes de abrir outra
	if (m_point[m_pointNum].pSeq)
	{
		CFx::Instance()->CloseSequence(m_point[m_pointNum].pSeq, true);
		delete m_point[m_pointNum].pSeq;
		m_point[m_pointNum].pSeq = NULL;
	}

	m_point[m_pointNum].pSeq =
		CFx::Instance()->OpenSequence("TrackItem.seq", false);
	m_point[m_pointNum].pSeq->Pos() = m_point[m_pointNum].pos;
	if (m_bDelay)
		m_point[m_pointNum].pSeq->SetDelay(m_pointNum * 50.0f);

	m_pointNum++;
}
