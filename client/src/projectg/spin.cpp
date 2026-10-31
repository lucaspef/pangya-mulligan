#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "clientsetting.h"
#include "mousecursor.h"
#include "quitwindow.h"
#include "itemwindow.h"
#include "spin.h"
#include "commonutil.h"
#include "jp/uiframesetmanager.h"
#include "../shared/localize.h"

extern const CUiFrameSetManager::Size g_sclOne983; // 00e424f8 (1, 1)


IMPLEMENT_ACTOR(CSpin, IActor)

CSpin::CSpin()
{
	m_dispPriority = DISP_PRIORITY_4;
	AddSkipList(0x400);
}

void CSpin::OnPreLoadInit()
{
	// 983 (0079a820): centro da bola pelas medidas "d_ball_center_x/y" (escala 1x1); sem elas fica o valor anterior
	CUiFrameSetManager::NamedDim* dimX = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* dimY = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
	if (dimX != NULL && dimY != NULL)
	{
		m_centerX = dimX->Calc(g_sclOne983);
		m_centerY = dimY->Calc(g_sclOne983);
	}
}

void CSpin::Reset()
{
	m_hitX = 0.0f;
	m_hitY = 0.0f;
	m_autoX = 0.0f;
	m_autoY = 0.0f;
	m_bAutoPoint = false;
	m_bClicked = false;
	m_bLoop = false;
	m_direction = 2;
	m_nextDirection = 2;
	m_loopType = 0;
	m_bUnused = false;
	m_color = 0xffffffff;
	m_bActive = false;
	m_bLock = false;
}

void CSpin::OnInit()
{
	Reset();
}

void CSpin::ProcessAutoPoint(float delta)
{
	if (!m_bAutoPoint)
		return;

	// 983 (0079a7d0): opera direto em m_autoX/m_autoY
	float x = m_autoX;
	float y = m_autoY;
	ProcessLoopHitPoint(delta, x, y, delta);

	m_autoX = x;
	m_autoY = y;
}

void CSpin::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode < 12)
		return;
	if (CQuitWindow::Instance()->IsActive())
		return;
	if (!CItemWindow::IsClosed())
		return;
	if (GolfClub().GetType() == 3)
		return;
	if (!m_bActive)
		return;

	if (PLAYER(GOLFDOC()->m_currentPlayer)->item == 0x18000000 ||
		PLAYER(GOLFDOC()->m_currentPlayer)->item == 0x18000011 ||
		GOLFDOC()->m_tutorialMode == 12)
		m_rangeY = 30.0f;
	else
		m_rangeY = (float)(int)PLAYER(GOLFDOC()->m_currentPlayer)->spin;

	if (PLAYER(GOLFDOC()->m_currentPlayer)->item == 0x18000001 ||
		GOLFDOC()->m_tutorialMode == 12)
		m_rangeX = 30.0f;
	else
		m_rangeX = (float)(int)PLAYER(GOLFDOC()->m_currentPlayer)->curve;

	if (m_bLoop == true)
		ProcessAutoPoint(delta);

	if (m_bLock)
		return;

	float x = 0.0f;
	float y = 0.0f;

	if (CMouseCursor::Instance()->GetArea() == 3 &&
		CMouseCursor::Instance()->IsInButtonDownArea(LEFT_BUTTON) &&
		g_input->GetButton(LEFT_BUTTON))
	{
		// 983 (0079ab1a): o centro da bola vem das medidas "d_ball_center_x/y" do CUiFrameSetManager (sem elas nao ha clique)
		CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
		CUiFrameSetManager::NamedDim* cx = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
		CUiFrameSetManager::NamedDim* cy = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
		if (cx != NULL && cy != NULL)
		{
		x = UnscaleByViewHeight983(g_view, g_input->GetMousePoint().x - cx->Calc(scl));
		y = UnscaleByViewHeight983(g_view, g_input->GetMousePoint().y - cy->Calc(scl));
		if (IsInclude(x, y))
		{
			if (Abs(x) < 1.0f && Abs(y) < 1.0f)
			{
				m_hitX = 0.0f;
				m_hitY = 0.0f;
			}
			else
			{
				m_hitX = x;
				m_hitY = y;
			}
			m_color = 0xffffffff;
		}
		else if (WisZero(x))
		{
			m_hitX = 0.0f;
			m_hitY = (y > 0.0f ? 1.0f : -1.0f) * m_rangeY;
			m_color = 0xa0ffffff;
		}
		else
		{
			float slope = y / x;
			float hx = m_rangeX * m_rangeY /
				sqrtf(
					m_rangeX * m_rangeX * slope * slope + m_rangeY * m_rangeY);

			// 983 0079a950: os 4 limites de 20 graus sao calculados em float (-0.34906584, 1.2217305, 1.9198622)
			if (slope > (float)tan(-20.0f * g_DEGTORAD) &&
				slope < (float)tan(20.0f * g_DEGTORAD))
			{
				m_hitX = (x > 0.0f ? 1.0f : -1.0f) * m_rangeX;
				m_hitY = 0.0f;
				m_color = 0xa0ffffff;
			}
			else if (slope > (float)tan(g_PI / 2.0f - 20.0f * g_DEGTORAD) ||
				slope < (float)tan(g_PI / 2.0f + 20.0f * g_DEGTORAD))
			{
				m_hitX = 0.0f;
				m_hitY = y > 0.0f ? m_rangeY : m_rangeY * -1.0f;
				m_color = 0xa0ffffff;
			}
			else
			{
				m_hitX = x > 0.0f ? hx : hx * -1.0f;
				m_color = 0xa0ffffff;
				m_hitY = m_hitX * slope;
			}
		}
		}
	}

	if (GOLFDOC()->m_tutorialMode == 12 && m_bClicked && IsPointInAutoPoint())
	{
		m_bClicked = false;
		IActor* pActor = GetActor("Tutorial");
		if (pActor)
			pActor << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
	}

	if ((g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true)) &&
		!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true))
	{
		float step = delta * g_PI;
		x = m_hitX;
		y = m_hitY;

		if (g_input->Get("\xbb\xf3", true))
		{
			float limit =
				sqrtf(1.0f - x * x / (m_rangeX * m_rangeX)) * m_rangeY * -1.0f;
			if (y == limit)
			{
				float angle = acosf(Between(-1.0f, x / m_rangeX, 1.0f));
				if (x > 0.0f)
					m_angle = Max(angle * -1.0f - step, -g_PI / 2.0f);
				else
					m_angle = Min(step - angle, -g_PI / 2.0f);
				x = cosf(m_angle) * m_rangeX;
				y = sinf(m_angle) * m_rangeY;
			}
			else
			{
				y -= delta * 50.0f;
				if (y < limit)
				{
					y = limit;
					m_color = 0xa0ffffff;
				}
				else
					m_color = 0xffffffff;
			}
		}

		if (g_input->Get("\xc7\xcf", true))
		{
			float limit =
				sqrtf(1.0f - x * x / (m_rangeX * m_rangeX)) * m_rangeY;
			if (y == limit)
			{
				float angle = acosf(Between(-1.0f, x / m_rangeX, 1.0f));
				if (x < 0.0f)
					m_angle = Max(angle - step, g_PI / 2.0f);
				else
					m_angle = Min(angle + step, g_PI / 2.0f);
				x = cosf(m_angle) * m_rangeX;
				y = sinf(m_angle) * m_rangeY;
			}
			else
			{
				y += delta * 50.0f;
				if (y > limit)
				{
					y = limit;
					m_color = 0xa0ffffff;
				}
				else
					m_color = 0xffffffff;
			}
		}

		if (g_input->Get("\xc1\xc2", true))
		{
			float limit =
				sqrtf(1.0f - y * y / (m_rangeY * m_rangeY)) * m_rangeX * -1.0f;
			if (x == limit)
			{
				float angle = acosf(Between(-1.0f, x / m_rangeX, 1.0f));
				if (y > 0.0f)
					m_angle = Min(angle + step, g_PI);
				else
					m_angle = Max(angle * -1.0f - step, -g_PI);
				x = cosf(m_angle) * m_rangeX;
				y = sinf(m_angle) * m_rangeY;
			}
			else
			{
				x -= delta * 50.0f;
				if (x < limit)
				{
					x = limit;
					m_color = 0xa0ffffff;
				}
				else
					m_color = 0xffffffff;
			}
		}

		if (g_input->Get("\xbf\xec", true))
		{
			float limit =
				sqrtf(1.0f - y * y / (m_rangeY * m_rangeY)) * m_rangeX;
			if (x == limit)
			{
				float angle = acosf(Between(-1.0f, x / m_rangeX, 1.0f));
				if (y < 0.0f)
					m_angle = Min(step - angle, 0.0f);
				else
					m_angle = Max(angle - step, 0.0f);
				x = cosf(m_angle) * m_rangeX;
				y = sinf(m_angle) * m_rangeY;
			}
			else
			{
				x += delta * 50.0f;
				if (x > limit)
				{
					x = limit;
					m_color = 0xa0ffffff;
				}
				else
					m_color = 0xffffffff;
			}
		}

		m_hitX = x;
		m_hitY = y;
	}
}

void CSpin::GetNextDirection()
{
	switch (m_loopType)
	{
	case 0:
		m_direction = 0;
		break;

	case 1:
		if (m_nextDirection == 5)
			m_nextDirection = 2;
		else
			m_nextDirection++;
		break;

	case 2:
		if (m_direction == 3)
			m_nextDirection = 5;
		else if (m_direction == 5)
			m_nextDirection = 3;
		break;
	}
}

void CSpin::ProcessLoopHitPoint(float delta, float& x, float& y, float& z)
{
	float limit;

	switch (m_direction)
	{
	case 1:
		if (y < 0.0f)
		{
			y += delta * 50.0f;
			if (y >= 0.0f)
			{
				GetNextDirection();
				m_direction = m_nextDirection;
				y = 0.0f;
			}
		}
		else if (x < 0.0f)
		{
			x += delta * 50.0f;
			if (x >= 0.0f)
			{
				GetNextDirection();
				m_direction = m_nextDirection;
				x = 0.0f;
			}
		}
		else if (y > 0.0f)
		{
			y -= delta * 50.0f;
			if (y <= 0.0f)
			{
				GetNextDirection();
				m_direction = m_nextDirection;
				y = 0.0f;
			}
		}
		else
		{
			x -= delta * 50.0f;
			if (x <= 0.0f)
			{
				GetNextDirection();
				m_direction = m_nextDirection;
				x = 0.0f;
			}
		}
		break;

	case 2:
		limit = sqrtf(1.0f - x * x / 400.0) * -20.0f;
		if (y == limit)
			m_direction = m_nextDirection != 0;
		else
		{
			y -= delta * 50.0f;
			if (y < limit)
				y = limit;
		}
		break;

	case 3:
		limit = sqrtf(1.0f - y * y / 400.0) * -20.0f;
		if (x == limit)
			m_direction = m_nextDirection != 0;
		else
		{
			x -= delta * 50.0f;
			if (x < limit)
				x = limit;
		}
		break;

	case 4:
		limit = sqrtf(1.0f - x * x / 400.0) * 20.0f;
		if (y == limit)
			m_direction = m_nextDirection != 0;
		else
		{
			y += delta * 50.0f;
			if (y > limit)
				y = limit;
		}
		break;

	case 5:
		limit = sqrtf(1.0f - y * y / 400.0) * 20.0f;
		if (x == limit)
			m_direction = m_nextDirection != 0;
		else
		{
			x += delta * 50.0f;
			if (x > limit)
				x = limit;
		}
		break;
	}
}

bool CSpin::IsPointInAutoPoint()
{
	if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
		return false;

	// 983 (0079a650): auto point junto do centro (0,0) nao conta
	if (fabs(m_autoX) < 0.6f && fabs(m_autoY) < 0.6f)
		return false;

	if (fabs(m_autoX - m_hitX) < 2.0f && fabs(m_autoY - m_hitY) < 2.0f)
		return true;

	return false;
}

void CSpin::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x161):
		if (GolfClub().GetType() == 3)
		{
			*(float*)msg.param1 = 0.0f;
			*(float*)msg.param2 = 0.0f;
		}
		else
		{
			*(float*)msg.param1 = m_hitX / 30.0;
			*(float*)msg.param2 = m_hitY / 30.0;
		}
		break;

	case M983(0x163):
		m_bActive = true;
		break;

	case M983(0x164):
		m_bActive = false;
		break;

	case M983(0x165):
		switch (msg.param1)
		{
		case 0:
			m_hitX = Between(-1.0f, GolfBall().m_initCurve, 1.0f) * 30.0f;
			m_hitY = Between(-1.0f, GolfBall().m_initSpin, 1.0f) * 30.0f;
			break;

		case 1:
			m_hitX = (float)msg.param2;
			m_hitY = (float)msg.param3;
			break;

		case 2:
			// 983 (0079b529): grava direto o ponto automatico
			m_autoX = (float)msg.param2;
			m_autoY = (float)msg.param3;
			return;
		}

		{
			// 983 (0079b59d): sem ConvertToWorld
			bool bInclude = IsInclude(m_hitX, m_hitY);
			m_color = bInclude ? 0xffffffff : 0xa0ffffff;
			m_bActive = true;
		}
		break;

	case M983(0x167):
		*(unsigned long*)msg.param3 = m_bActive ? m_color : 0;
	case M983(0x166):
		// 983 (0079b5e5): posicao na tela = hit escalado pela altura + medidas "d_ball_center_x/y" do CUiFrameSetManager
		{
			CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
			CUiFrameSetManager::NamedDim* cx = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
			CUiFrameSetManager::NamedDim* cy = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
			if (cx != NULL && cy != NULL)
			{
				*(float*)msg.param1 = ScaleByViewHeight983(g_view, m_hitX) + cx->Calc(scl);
				*(float*)msg.param2 = ScaleByViewHeight983(g_view, m_hitY) + cy->Calc(scl);
			}
		}
		break;

	case M983(0x168):
		{
			CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
			CUiFrameSetManager::NamedDim* cx = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
			CUiFrameSetManager::NamedDim* cy = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
			if (cx != NULL && cy != NULL)
			{
				*(float*)msg.param1 = ScaleByViewHeight983(g_view, m_autoX) + cx->Calc(scl);
				*(float*)msg.param2 = ScaleByViewHeight983(g_view, m_autoY) + cy->Calc(scl);
			}
			*(unsigned long*)msg.param3 = m_color;
		}
		break;

	case M983(0x162):
		*(float*)msg.param1 = m_hitX / m_rangeX;
		*(float*)msg.param2 = m_hitY / m_rangeY;
		break;

	case M983(0x169):
		if (msg.param1 < 2)
			m_bAutoPoint = msg.param1 == 1;
		else if (msg.param1 == 3)
			m_bLock = true;
		else if (msg.param1 == 4)
			m_bLock = false;
		else
			m_bClicked = true;
		break;

	case M983(0x16a):
		m_bLoop = !m_bLoop;
		break;

	case M983(0x16b):
		m_direction = msg.param1;
		m_nextDirection = msg.param2;
		m_loopType = msg.param3;
		break;

	case 10010:
		Reset();
		break;

	}
}

bool CSpin::IsInclude(float x, float y)
{
	return x * x / (m_rangeX * m_rangeX) + y * y / (m_rangeY * m_rangeY) < 1.0f
		? true
		: false;
}
