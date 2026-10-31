#include "minatl.h"
#include "projectg.h"
#include "camera.h"
#include "quadtree.h"
#include "quitwindow.h"
#include "itemwindow.h"
#include "mousecursor.h"
#include "capturedbg.h"
#include "chatmsg.h"
#include "messengerdlg.h"
#include "user_info.h"
#include "networksystem.h"
#include "frwndmanager.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "fresh.h"

extern WMatrix g_camera;
extern Fresh* g_pFresh;
extern WView* g_view;
bool IsMyTurn(bool check);
int WisZero(const float& v, float e);
float GetBaseAngle(float angle);
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	void SetHelpMsg(int index, char* msg);
};

// 983 00714350: inline, devolve a referencia (doc+0xd8 holes[cur-1].pin)
static __forceinline const WVector& GetPinPos()
{
	return GOLFDOC()->GetHoleData().pin;
}

static WVector CrossProduct(const WVector& a, const WVector& b)
{
	return WVector(a.z * b.y - a.y * b.z, a.x * b.z - a.z * b.x,
		a.y * b.x - a.x * b.y);
}

// HACK: references the remaining wmath.h constants so the .rdata constant
// table has the same layout as the original
static float rb_UnusedConstants()
{
	return g_2_PI + g_HUGE;
}

void CCamera::OnInit()
{
	m_rotAcc = 0.0f;
	m_moveAcc = 0.0f;
	m_horizonAcc = 0.0f;
	m_rate = 0.0f;
	m_curve = 0.0f;
	m_bFixed = false;
	m_timer = 0.0f;
	m_bFreeLook = false;
	m_idleTime = 0.0f;
	m_bSendAngle = false;
	m_bEnable = false;
	m_rotTotal = m_rotRemain = 0.0f;
	m_spot.time = 0.0f;
	m_spot.bFollow = false;
	m_angleStart = 0.0f;
	m_angleEnd = 0.0f;
}

float CCamera::GetChangeRate()
{
	return m_curve * m_curve;
}

float CCamera::GetHeightRate(float height)
{
	return (height - m_groundHeight) / (192.0f + (height - m_groundHeight));
}

#include "club.h"

void CCamera::LookAt(WVector& eye, WVector& at)
{
	g_camera = MakeCameraMatrix(eye, at);
}

void CCamera::GetRotateAcc(float dt)
{
	static int s_last;
	if (s_last == m_rotDir && m_rotDir != 0)
		m_rotAcc = Between(0.0f, dt + m_rotAcc, 1.0f);
	else
		m_rotAcc = 0.0f;
	s_last = m_rotDir;
}

void CCamera::GetMoveAcc(float dt)
{
	static int s_last;
	if (s_last == m_moveDir && m_moveDir != 0)
		m_moveAcc = Between(0.0f, dt + m_moveAcc, 1.0f);
	else
		m_moveAcc = 0.0f;
	s_last = m_moveDir;
}

void CCamera::GetHorizonAcc(float dt)
{
	static int s_last;
	if (s_last == m_horizonDir && m_horizonDir != 0)
		m_horizonAcc = Between(0.0f, dt + m_horizonAcc, 1.0f);
	else
		m_horizonAcc = 0.0f;
	s_last = m_horizonDir;
}

void CCamera::YawFixedCamera(float amount)
{
	// 983 00713c50: acc * 5 * amount (nessa ordem)
	m_yaw -= m_rotAcc * 5.0f * amount;
	if (m_yaw > 10.0f)
	{
		m_yaw = 10.0f;
		return;
	}
	else if (m_yaw < -10.0f)
	{
		m_yaw = -10.0f;
		return;
	}
	m_target -= g_camera.xa * (m_rotAcc * 5.0f * amount);
	g_camera = MakeCameraMatrix(m_eye, m_target);
}

void CCamera::PitchFixedCamera(float amount)
{
	// 983 00713d70: acc * 5 * amount (nessa ordem)
	m_pitch += m_horizonAcc * 5.0f * amount;
	if (m_pitch > 5.0f)
	{
		m_pitch = 5.0f;
		return;
	}
	else if (m_pitch < -5.0f)
	{
		m_pitch = -5.0f;
		return;
	}
	// 983: x = d.x + x, mas y = y + d.y e z = z + d.z (não o operator+=)
	WVector d = g_camera.ya * (m_horizonAcc * 5.0f * amount);
	WVector& t = m_target;
	t.x = d.x + t.x;
	t.y = t.y + d.y;
	t.z = t.z + d.z;
	g_camera = MakeCameraMatrix(m_eye, m_target);
}

CCamera::CCamera()
{
	AddSkipList(0x84c82);
}

void CCamera::SetSpotCamera(bool bSpot)
{
	m_spot.bFollow = !bSpot;
	int index;
	WVector& za = GOLFDOC()->m_cameraData.mat.za;
	float range = GolfClub().GetRange(NULL) * 3.2f;
	WVector offset = range * za;
	m_spot.ground =
		GetPVS().GetGroundPoint(GolfBall().m_pos + offset, true, &index, true);
	if (index == -1)
	{
		if (bSpot)
			SendMsg(this, "GolfRule", M983(0x78), 8, 0, 0, 0);
		return;
	}

	WMatrix& cam = GOLFDOC()->m_cameraData.mat;
	m_spot.ground +=
		WVector(cam.ya.x * 30.0f - cam.za.x * 60.0f + cam.xa.x * 0.0f,
			cam.ya.y * 30.0f - cam.za.y * 60.0f + cam.xa.y * 0.0f,
			cam.ya.z * 30.0f - cam.za.z * 60.0f + cam.xa.z * 0.0f);
	m_spot.pitch = -15.0f * g_DEGTORAD;

	if (bSpot)
	{
		m_eye = m_spot.ground;
		m_target = m_spot.ground + GOLFDOC()->m_cameraData.mat.za;
		m_target.y += tanf(m_spot.pitch);
		g_camera = MakeCameraMatrix(m_eye, m_target);
	}
	else
	{
		m_spot.pos = g_camera.pivot;
		m_spot.angle =
			atanf(g_camera.za.y / sqrtf(1.0f - g_camera.za.y * g_camera.za.y));
		m_spot.dir = (m_spot.ground - m_spot.pos) * 0.8333333f;
		m_spot.time = 0.0f;
	}
}

extern bool g_rastroNoPlay983; // B1504 temporario (quadtree.cpp)

void CCamera::ProcessMousePick(float dt)
{
	WVector mouse = g_input->GetMousePoint();
	WVector point;
	WVector start;
	{
		WVector ray;
		g_view->GetScreen2World(mouse.x, mouse.y, start, ray);
		point = GetPVS().GetRayIntersection(start, ray);
	}

	// 983 00714350: Shift + clique (o 645 usava Ctrl)
	if (!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true))
	{
		WVector ballScreen = g_view->Projection(GolfBall().m_pos);
		WVector2D delta(mouse.x - ballScreen.x, mouse.y - ballScreen.y);
		if (delta.Magnitude() > 10.0f)
		{
			// 983: direção no plano (y = 0); dist = quadrado do tamanho antes de normalizar
			WVector diff = point - GolfBall().m_pos;
			WVector dir(diff.x, 0.0f, diff.z);
			float dist = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
			dir.Normalize();
			GolfClub().SelectClubByDist(sqrtf(dist) * 0.3125f);

			// 983: só o y do produto vetorial dir × ya
			const WVector& up = g_camera.ya;
			float crossY = dir.x * up.z - dir.z * up.x;
			if (!(Wabs(crossY) < g_EPSILON))
			{
				float angle = acosf(Between(-1.0f,
					dir.x * up.x + dir.y * up.y + dir.z * up.z,
					1.0f));
				if (!WisZero(angle, g_EPSILON))
				{
					if (crossY > 0.0f)
					{
						m_rotDir = -1;
						angle = angle * -1.0f;
					}
					else
						m_rotDir = 1;
					angle = Between(-g_PI, angle, g_PI);
					m_rotTotal = m_rotRemain = angle;
					angle = angle + GOLFDOC()->m_cameraData.angle;
					SendMsg(this, "Spot", M983(0xf6), (int)&angle, (int)&point, 0, 0);
					return;
				}
			}
			SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
		}
	}
	else
	{
		point.y = GolfBall().m_radius + point.y;
		if (GOLFDOC()->m_bDropRequest)
		{
			WVector ballToPin;
			ballToPin = GolfBall().m_pos - GetPinPos();
			ballToPin.y = 0.0f;
			WVector pointToPin;
			pointToPin = point - GetPinPos();
			pointToPin.y = 0.0f;
			unsigned char ground = GetPVS().GetGroundType(point, true, NULL);
			g_rastroNoPlay983 = true; // B1591 temporario: objeto que recusou o ponto (linha "B1504 NoPlay")
			bool bOverlap = GetPVS().OverlappedWithObject(point, 0.0f);
			g_rastroNoPlay983 = false;
			if (!bOverlap &&
				(ground == 0 ||
					(ground == 1 &&
						!GOLFDOC()->m_pPolySoup->IsOutOfBound(point) &&
						pointToPin.Magnitude() >
							ballToPin.Magnitude() - 25.6f)))
			{
				GOLFDOC()->m_bDropRequest = 0;
				if (OnlinePlay() && GOLFDOC()->m_tutorialMode == 15)
				{
					WSendPacket packet((enumClientPacket)0x19);
					packet.EncodeBuffer(&point, 12);
					packet.Send((eSendTo)0);
					return;
				}
				SendMsg(this, "GolfRule", M983(0x8d), (int)&point, 0, 0, 0);
			}
			else
			{
				g_audio->PlaySfx("play_unplayable");
				if (CHelpMsg::Instance())
					CHelpMsg::Instance()->SetHelpMsg(20, NULL);
			}
		}
		else if (!WNetworkSystem::Instance()->IsConnected(
					 WNetworkSystem::NET_GAME))
			SendMsg(this, "GolfRule", M983(0x8e), (int)&point, 0, 0, 0);
	}
}

float CCamera::GetCameraAngle()
{
	return Min(0.0f, GOLFDOC()->m_cameraData.pitch - atanf(m_curve * 0.65f));
}

IObject* CCameraMakeInstance()
{
	/*DEAD*/
	// HACK: dead call, only so that this function is compiled (and emitted)
	// after the camera functions that wait for GolfBall(), as in the original
	if (0)
		GolfBall();
	return new CCamera;
}

struct __impCCamera
{
	__impCCamera()
	{
		ObjectFactory().AddObjectFunctor(CCameraMakeInstance, "CCamera");
		ActorFactory().AddActorClass("CCamera");
	}
};
const WRTTI CCamera::m_RTTI("CCamera", &IActor::m_RTTI);
static __impCCamera __implCCamera;

void CCamera::ProcessToSpot(float dt)
{
	if (GOLFDOC()->m_tutorialMode < 1)
		return;
	if (CQuitWindow::Instance()->IsActive())
		return;
	if (!CItemWindow::IsClosed())
		return;
	if (!IsMyTurn(false))
		return;
	if (!WisZero(m_rotRemain, g_EPSILON))
		return;

	if (g_input->GetDown("NUM_1", true) || g_input->GetDown("PAD_1", true))
	{
		if (GOLFDOC()->m_gameMode != 0x40000)
		{
			if (GetPVS().GetMeshIndex(GolfBall().m_pos +
						GolfClub().GetRange(NULL) * 3.2f *
							GOLFDOC()->m_cameraData.mat.za,
					false) == -1)
				return;
		}

		switch (GOLFDOC()->m_gameMode)
		{
		case 8:
		case 0x100:
			SendMsg(this, "GolfRule", M983(0x78), 0x40000, 0, 0, 0);
			SetSpotCamera(false);
			g_audio->PlaySfx("camera_move_to_spot");
			break;
		case 0x200:
			if (GOLFDOC()->m_bDropRequest)
				break;
		case 0x8000:
			SendMsg(this, "GolfRule", M983(0x78), 0x40000, 0, 0, 0);
			SetSpotCamera(true);
			g_audio->PlaySfx("camera_move_to_spot");
			break;
		case 0x40000:
			SendMsg(this, "GolfRule", M983(0x78), 8, 0, 0, 0);
			break;
		}
	}

	if (GOLFDOC()->m_gameMode == 0x40000)
	{
		if (GolfClub().Updated())
			SetSpotCamera(true);

		if (m_spot.bFollow)
		{
			m_spot.time += dt;
			if (m_spot.time > 1.2f)
			{
				m_spot.time = 1.2f;
				m_spot.bFollow = false;
			}
			m_spot.rate =
				(m_spot.pitch - m_spot.angle) * m_spot.time + m_spot.angle;
			m_eye = m_spot.pos + m_spot.time * m_spot.dir;
			m_eye.y = Max(GetPVS().GetGroundPoint(m_eye, false, NULL, true).y +
					GOLFDOC()->m_cameraData.clearance,
				m_eye.y);
			m_target = m_eye + GOLFDOC()->m_cameraData.mat.za;
			m_target.y += tanf(m_spot.rate);
			g_camera = MakeCameraMatrix(m_eye, m_target);
		}
	}
}

void CCamera::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0xf1):
		m_curve = 0.0f;
		m_rate = 0.5f;
		break;
	case M983(0xee):
		m_eye = GOLFDOC()->m_cameraData.mat.pivot;
		m_target =
			GOLFDOC()->m_cameraData.mat.pivot + GOLFDOC()->m_cameraData.mat.za;
		m_target.y += tanf(GOLFDOC()->m_cameraData.pitch);
		m_curve = 0.0f;
		m_rate = m_baseRate =
			GetHeightRate(GOLFDOC()->m_cameraData.mat.pivot.y);
		m_homeRate = GetHeightRate(GOLFDOC()->m_cameraData.mat.pivot.y);
		break;
	case M983(0xef):
		m_groundHeight = GetPVS().m_groundHeight;
		m_maxHeight = m_groundHeight + 192.0f;
		break;
	case M983(0xf0):
		m_bFixed = false;
		break;
	case M983(0xf2):
		m_angleEnd = *(float*)msg.param1;
		break;
	case M983(0x19a):
		m_bEnable = true;
		break;
	case 0x2718:
		m_idleTime = 0.0f;
		GOLFDOC()->m_cameraData.angle = *(float*)msg.param1;
		if (!IsMyTurn(false))
		{
			m_angleEnd = GOLFDOC()->m_cameraData.angle;
			m_angleStart = GOLFDOC()->m_cameraData.angle;
		}
		break;
	case M983(0x19c):
		*(int*)msg.param1 = m_bEnable;
		break;
	case M983(0x19b):
		m_bEnable = false;
		break;
	}
}

void CCamera::Translate(float dt)
{
	if (GOLFDOC()->m_gameMode == 0x200)
	{
		float h = Between(0.1f,
			(g_camera.pivot.y -
				GetPVS().GetGroundPoint(g_camera.pivot, true, NULL, true).y) *
				0.01f,
			1.0f);
		g_camera.pivot +=
			GOLFDOC()->m_cameraData.mat.za * (h * m_horizonAcc * dt * 400.0f);
		g_camera.pivot = GetPVS().TestCameraCollision(g_camera.pivot);
	}
	else
	{
		WVector pos =
			GOLFDOC()->m_cameraData.mat.za * (dt * m_horizonAcc * 400.0f) +
			g_camera.pivot;
		WVector newPos = GetPVS().TestCameraCollision(pos);
		float minY = m_groundHeight;
		minY = Max(GetPVS().GetGroundPoint(newPos, true, NULL, true).y, minY);
		newPos.y = Between(minY + GOLFDOC()->m_cameraData.clearance, newPos.y,
			m_maxHeight);
		WVector d;
		d = newPos - g_camera.pivot;
		m_eye += d;
		m_target = m_eye + GOLFDOC()->m_cameraData.mat.za;
		// 983 007151c0: h / (192 + h), mesma conta de GetHeightRate (nao h / 192)
		m_rate = GetHeightRate(m_eye.y);
		m_curve = m_rate - m_baseRate;
		m_target.y += tanf(GetCameraAngle());
		g_camera = MakeCameraMatrix(m_eye, m_target);
		m_bFixed = true;
		m_baseRate = GetHeightRate(minY + GOLFDOC()->m_cameraData.clearance);

		if (GOLFDOC()->m_gameMode == 0x100 &&
			((g_camera.pivot - GOLFDOC()->m_cameraData.mat.pivot - d) *
				GOLFDOC()->m_cameraData.mat.za) *
					((g_camera.pivot - GOLFDOC()->m_cameraData.mat.pivot) *
						GOLFDOC()->m_cameraData.mat.za) <
				0.0f &&
			Wabs(g_camera.pivot.y - GOLFDOC()->m_cameraData.mat.pivot.y) < 3.2f)
		{
			m_eye = GOLFDOC()->m_cameraData.mat.pivot;
			m_target = GOLFDOC()->m_cameraData.mat.pivot +
				GOLFDOC()->m_cameraData.mat.za;
			m_curve = 0.0f;
			m_rate = m_baseRate;
			m_target.y += tanf(GOLFDOC()->m_cameraData.pitch);
			g_camera.pivot = GOLFDOC()->m_cameraData.mat.pivot;
			if (IsMyTurn(true))
			{
				GOLFDOC()->m_uiAlpha = 0xff;
				CChatMsg::Instance()->ReplaceBackupUiAlpha();
			}
			GOLFDOC()->m_gameMode = 8;
			m_timer = 0.25f;
			return;
		}

		if (GOLFDOC()->m_gameMode == 8)
		{
			GOLFDOC()->m_uiAlpha = 0x80;
			CChatMsg::Instance()->ReplaceBackupUiAlpha();
			GOLFDOC()->m_gameMode = 0x100;
		}
	}
}

void CCamera::UpDown(float dt)
{
	if (GOLFDOC()->m_gameMode == 0x200)
	{
		float lo =
			GetPVS().GetGroundPoint(g_camera.pivot, true, NULL, true).y + 16.0f;
		// 983 007157e0: h/300 (nao h*0.00333) e o limite inferior antes do superior
		float s;
		if (300.0 > g_camera.pivot.y - lo)
		{
			s = (g_camera.pivot.y - lo) / 300.0;
			s = s > (double)0.2f ? s : 0.2f;
			g_camera.pivot.y = s * 400.0 * dt + g_camera.pivot.y;
		}
		else
			g_camera.pivot.y = dt * 400.0 + g_camera.pivot.y;
		float hi = m_groundHeight + 2560.0f;
		if (g_camera.pivot.y < lo)
			g_camera.pivot.y = lo;
		else if (hi < g_camera.pivot.y)
			g_camera.pivot.y = hi;
		return;
	}

	if (GOLFDOC()->m_gameMode == 8 && dt > 0.0f)
	{
		GOLFDOC()->m_gameMode = 0x100;
		GOLFDOC()->m_uiAlpha = 0x80;
		CChatMsg::Instance()->ReplaceBackupUiAlpha();
	}

	// 983 007157e0: rate = curve + baseRate sempre (sem os ramos m_bFixed/
	// groundType/GetChangeRate do 645) e so o y do pivot muda (sem - za*k)
	m_curve = Between(0.0f, dt * m_moveAcc + m_curve, 1.0f - m_baseRate);
	m_rate = m_curve + m_baseRate;
	g_camera.pivot.y =
		(GOLFDOC()->m_cameraData.mat.pivot.y - m_groundHeight + 192.0f) *
			m_rate +
		m_groundHeight;
	g_camera.pivot = GetPVS().TestCameraCollision(g_camera.pivot);
	m_eye = g_camera.pivot;
	m_target = g_camera.pivot + GOLFDOC()->m_cameraData.mat.za;
	m_target.y += tanf(GetCameraAngle());
	g_camera = MakeCameraMatrix(m_eye, m_target);

	if (!m_bFixed && m_rate == m_homeRate)
	{
		GOLFDOC()->m_gameMode = 8;
		if (IsMyTurn(true))
		{
			GOLFDOC()->m_uiAlpha = 0xff;
			CChatMsg::Instance()->ReplaceBackupUiAlpha();
		}
	}
}

void CCamera::RotateEnvironment(float angle)
{
	GOLFDOC()->m_cameraData.angle =
		GetBaseAngle(angle + GOLFDOC()->m_cameraData.angle);
	// 983 00715b40: na camera de revisao do ShotReview nao soma o angleOffset
	bool review;
	SendMsg(this, "ShotReview", 0x2fd, (int)"IS_REVIEW_CAMERA_MODE",
		(int)&review, 0, 0);
	float rot = review ? GOLFDOC()->m_cameraData.angle
		: GOLFDOC()->m_cameraData.angle +
			(IsMyTurn(true) ? 0.0f : GOLFDOC()->m_cameraData.angleOffset);
	GOLFDOC()->m_cameraData.mat = RotMat(rot, 1);
	CGolfDoc* doc = GOLFDOC();
	float back = doc->m_cameraData.distance * -1.0f;
	doc->m_cameraData.mat.pivot = GetPVS().TestCameraCollision(
		WVector(back * doc->m_cameraData.mat.za.x, doc->m_cameraData.height,
			back * doc->m_cameraData.mat.za.z) +
		doc->m_cameraData.target);
	m_homeRate = GetHeightRate(GOLFDOC()->m_cameraData.mat.pivot.y);
	if (IsMyTurn(false))
	{
		m_bSendAngle = true;
		SendMsg(this, "Player", M983(0x1a1), (int)&angle, 0, 0, 0);
		SendMsg(this, "Spot", M983(0x1a1), (int)&angle, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a1), (int)&angle, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x1a1), (int)&angle, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x1a1), (int)&angle, 0, 0, 0);
		SendMsg(this, "GuideLine", M983(0x1a1), 0, 0, 0, 0);
	}
}

void CCamera::RotateForGallery(float angle)
{
	WVector u1;
	if (0)
		u1.Magnitude();
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		WMatrix mat = g_camera * RotMat(angle, 1);
		mat = g_camera;
		mat.pivot.Reset();
		mat *= RotMat(angle, 1); // 983 0066f570 WMatrix::operator*=
		RotateEnvironment(angle);

		if (GOLFDOC()->m_gameMode & 8)
			mat.pivot = GOLFDOC()->m_cameraData.mat.pivot;
		else
		{
			WMatrix rot = RotMat(angle, 1);
			WVector rel = g_camera.pivot - GOLFDOC()->m_cameraData.target;
			mat.pivot = GOLFDOC()->m_cameraData.target + rel * rot;
			mat.pivot = GetPVS().TestCameraCollision(mat.pivot);
			if (GOLFDOC()->m_gameMode & 0x200)
				mat.pivot.y = g_camera.pivot.y;
		}
		g_camera = mat;
		m_eye = g_camera.pivot;
		m_target = g_camera.za + g_camera.pivot;
	}
}

void CCamera::Rotate(float dt)
{
	dt *= m_rotAcc;
	if (m_rotRemain < 0.0f || 0.0f < m_rotRemain)
	{
		if (Wabs(m_rotRemain) < Wabs(dt) ||
			WisEqual(m_rotRemain, dt, g_EPSILON))
		{
			dt = m_rotRemain;
			m_rotTotal = m_rotRemain = 0.0f;
		}
		else
			m_rotRemain -= dt;
	}

	WMatrix mat = g_camera * RotMat(dt, 1);
	mat = g_camera;
	mat.pivot.Reset();
	mat = mat * RotMat(dt, 1);
	RotateEnvironment(dt);

	if (GOLFDOC()->m_gameMode & 8)
		mat.pivot = GOLFDOC()->m_cameraData.mat.pivot;
	else
	{
		WVector rel = g_camera.pivot - GOLFDOC()->m_cameraData.target;
		mat.pivot = GOLFDOC()->m_cameraData.target + rel * RotMat(dt, 1);
		mat.pivot = GetPVS().TestCameraCollision(mat.pivot);
		if (GOLFDOC()->m_gameMode & 0x200)
			mat.pivot.y = g_camera.pivot.y;
	}
	g_camera = mat;
	m_eye = g_camera.pivot;
	m_target = g_camera.za + g_camera.pivot;

	if ((GOLFDOC()->m_gameMode & 0x40000) && !m_spot.bFollow)
		SetSpotCamera(true);
}

void CCamera::FixedCamera(float dt)
{
	if (CQuitWindow::Instance()->IsActive() || !CItemWindow::IsClosed())
		return;
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		return;
	if (GOLFDOC()->m_bDropRequest)
		return;

	for (int i = 3; i < 10; ++i)
	{
		if (g_input->GetDown(MakeStr("PAD_%d", i), true) ||
			g_input->GetDown(MakeStr("NUM_%d", i), true))
		{
			WPolySoup::sCamera* cam = GOLFDOC()->m_pPolySoup->FindCamera(
				MakeStr("\xc4\xab\xb8\xde\xb6\xf3%d", i));
			if (!cam)
				return;
			m_yaw = 0.0f;
			m_pitch = 0.0f;
			m_eye = cam->mat.pivot;
			m_target = cam->mat.za * 10.0f + m_eye;
			g_camera = cam->mat;
			g_view->SetFOV(cam->fov * g_DEGTORAD);
			GOLFDOC()->m_gameMode = 0x8000;
			return;
		}
	}

	if (GOLFDOC()->m_gameMode != 0x8000)
		return;

	m_rotDir = 0;
	if ((CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMLEFT &&
			g_input->GetButton(LEFT_BUTTON) > 0))
	{
		m_rotDir = 1;
		YawFixedCamera(dt);
	}
	if (CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMRIGHT &&
		g_input->GetButton(LEFT_BUTTON) > 0)
	{
		m_rotDir = -1;
		YawFixedCamera(dt * -1.0f);
	}
	if (g_input->Get("\xc1\xc2", true) && !g_input->Get("LCONTROL", true) &&
		!g_input->Get("RCONTROL", true) && !g_input->Get("LSHIFT", true) &&
		!g_input->Get("RSHIFT", true))
		RotateEnvironment(dt * 1.0471976f);
	if (g_input->Get("\xbf\xec", true) && !g_input->Get("LCONTROL", true) &&
		!g_input->Get("RCONTROL", true) && !g_input->Get("LSHIFT", true) &&
		!g_input->Get("RSHIFT", true))
		RotateEnvironment(dt * -1.0471976f);
	GetRotateAcc(dt);

	m_horizonDir = 0;
	if (CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_TOP ||
		(CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMUP &&
			g_input->GetButton(LEFT_BUTTON) > 0))
	{
		m_horizonDir = 1;
		PitchFixedCamera(dt);
	}
	if (CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_BOTTOM ||
		(CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMDOWN &&
			g_input->GetButton(LEFT_BUTTON) > 0))
	{
		m_horizonDir = -1;
		PitchFixedCamera(dt * -1.0f);
	}
	GetHorizonAcc(dt);
}

void CCamera::ProcessInput(float dt)
{
	m_rotDir = 0;
	m_moveDir = 0;
	m_horizonDir = 0;

	if ((GOLFDOC()->m_gameMode & 0x40000) && m_spot.bFollow)
		return;
	if (CQuitWindow::Instance()->IsActive() || !CItemWindow::IsClosed())
		return;

	static unsigned long s_lastTick;
	static unsigned long s_curTick;
	static bool s_bAccel;
	static unsigned long s_step;
	static bool s_bFwdHeld;
	static bool s_bBackHeld;
	static bool s_bFirst = true;

	if ((CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMLEFT &&
			g_input->GetButton(LEFT_BUTTON) > 0) ||
		(g_input->Get("\xc1\xc2", true) && !g_input->Get("LCONTROL", true) &&
			!g_input->Get("RCONTROL", true) && !g_input->Get("LSHIFT", true) &&
			!g_input->Get("RSHIFT", true)))
	{
		m_rotDir = 1;
		Rotate((float)(dt * 1.0471975430846214));
	}

	if ((CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMRIGHT &&
			g_input->GetButton(LEFT_BUTTON) > 0) ||
		(g_input->Get("\xbf\xec", true) && !g_input->Get("LCONTROL", true) &&
			!g_input->Get("RCONTROL", true) && !g_input->Get("LSHIFT", true) &&
			!g_input->Get("RSHIFT", true)))
	{
		m_rotDir = -1;
		Rotate((float)(dt * -1.0471975430846214));
	}

	if (!(GOLFDOC()->m_gameMode & 0x40000))
	{
		if ((CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_TOP &&
				g_pFresh->IsDesktopFocused()) ||
			(CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMUP &&
				g_input->GetButton(LEFT_BUTTON) > 0) ||
			(g_input->Get("\xbb\xf3", true) &&
				!g_input->Get("LCONTROL", true) &&
				!g_input->Get("RCONTROL", true) &&
				!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true)))
		{
			if (GOLFDOC()->m_gameMode == 0x200)
			{
				m_horizonDir = 1;
				Translate(dt);
			}
			else if (m_rate < 1.0f)
			{
				m_moveDir = 1;
				UpDown(dt);
			}
		}

		if ((CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_BOTTOM &&
				g_pFresh->IsDesktopFocused()) ||
			(CMouseCursor::Instance()->GetArea() ==
					CMouseCursor::AREA_CAMDOWN &&
				g_input->GetButton(LEFT_BUTTON) > 0) ||
			(g_input->Get("\xc7\xcf", true) &&
				!g_input->Get("LCONTROL", true) &&
				!g_input->Get("RCONTROL", true) &&
				!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true)))
		{
			if (GOLFDOC()->m_gameMode == 0x200)
			{
				m_horizonDir = -1;
				Translate(dt * -1.0f);
			}
			else if (m_rate > 0.0f)
			{
				m_moveDir = -1;
				UpDown(dt * -1.0f);
			}
		}

		float accel = 0.0f;
		DWORD now = 0;
		if ((CMouseCursor::Instance()->GetArea() ==
					CMouseCursor::AREA_CAMFORWARD &&
				g_input->GetButton(LEFT_BUTTON) > 0) ||
			(g_input->Get("\xbb\xf3", true) &&
				(g_input->Get("LSHIFT", true) ||
					g_input->Get("RSHIFT", true))) ||
			g_input->Get("HOME", true))
		{
			if (GOLFDOC()->m_gameMode == 0x200)
			{
				now = timeGetTime();
				s_curTick = now;
				if (now - s_lastTick > 800 && !s_bAccel)
				{
					s_lastTick = now;
					if (!s_bFirst)
						s_bAccel = true;
					s_bFirst = false;
				}
				if (now - s_lastTick > 50 && s_bAccel == true)
				{
					s_lastTick = now;
					if (s_step < 25)
						s_step++;
					accel = (float)(s_step >> 1) * dt;
				}
				if (m_rate > 0.0f)
				{
					m_moveDir = -1;
					UpDown((accel + dt) * -1.0f);
				}
				s_bFwdHeld = true;
				s_bBackHeld = false;
			}
			else if (IsMyTurn(false))
			{
				m_horizonDir = 1;
				Translate(dt);
			}
		}
		else if (!s_bBackHeld)
		{
			s_curTick = 0;
			s_lastTick = 0;
			s_bAccel = false;
			accel = 0.0f;
			s_step = 0;
			s_bFwdHeld = false;
			s_bFirst = true;
		}

		if ((CMouseCursor::Instance()->GetArea() ==
					CMouseCursor::AREA_CAMBACKWARD &&
				g_input->GetButton(LEFT_BUTTON) > 0) ||
			(g_input->Get("\xc7\xcf", true) &&
				(g_input->Get("LSHIFT", true) ||
					g_input->Get("RSHIFT", true))) ||
			g_input->Get("END", true))
		{
			if (GOLFDOC()->m_gameMode == 0x200)
			{
				now = timeGetTime();
				s_curTick = now;
				if (now - s_lastTick > 800 && !s_bAccel)
				{
					s_lastTick = now;
					if (!s_bFirst)
						s_bAccel = true;
					s_bFirst = false;
				}
				if (now - s_lastTick > 50 && s_bAccel == true)
				{
					s_lastTick = now;
					if (s_step < 25)
						s_step++;
					accel = (float)(s_step >> 1) * dt;
				}
				if (m_rate < 1.0f)
				{
					m_moveDir = 1;
					UpDown(accel + dt);
				}
				s_bBackHeld = true;
				s_bFwdHeld = false;
			}
			else if (IsMyTurn(false))
			{
				m_horizonDir = -1;
				Translate(dt * -1.0f);
			}
		}
		else if (!s_bFwdHeld)
		{
			s_curTick = 0;
			s_lastTick = 0;
			s_bAccel = false;
			s_step = 0;
			s_bBackHeld = false;
			s_bFirst = true;
		}

		bool bGuideBook = false;
		SendMsg(this, "GuideBook", M983(0x12c), (int)&bGuideBook, 0, 0, 0);
		bool bScore = false;
		SendMsg(this, "Score", M983(0x139), (int)&bScore, 0, 0, 0);
		// 983 00716890: câmera de revisão do ShotReview (ou mouse sobre o botão dela) bloqueia o clique
		bool bReview;
		SendMsg(this, "ShotReview", 0x2fd, (int)"IS_REVIEW_CAMERA_MODE",
			(int)&bReview, 0, 0);
		if (!bReview)
			SendMsg(this, "ShotReview", 0x2fd, (int)"IS_MOUSE_OVER_REVIEW_BUTTON",
				(int)&bReview, 0, 0);
		bool bInView = CMouseCursor::Instance()->IsInclude(0, 0,
			g_view->GetWidth(), g_view->GetHeight());

		// 983: Ctrl + clique (tipos de jogo 0 e 1) marca o ponto para o TutorGuide
		if ((Doc()->m_golfGame.gameType == 0 || Doc()->m_golfGame.gameType == 1) &&
			(GOLFDOC()->m_gameMode & 0x200) &&
			(g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true)) &&
			!m_moveDir && !m_horizonDir && CMouseCursor::Instance()->GetArea() == 0 &&
			g_input->GetButton(LEFT_BUTTON) == 1 &&
			GOLFDOC()->m_tutorialMode >= 1 &&
			!CUserInfo::Instance()->IsVisible() && !bGuideBook && !bScore &&
			!g_pFresh->GetManager()->HasValidWindow() &&
			MESSENGER()->IsGameFocus() == true)
		{
			ProcessTutorGuidePick(dt);
			return;
		}

		if ((GOLFDOC()->m_gameMode & 0x200) && IsMyTurn(false) && !m_moveDir &&
			!m_horizonDir && CMouseCursor::Instance()->GetArea() == 0 &&
			g_input->GetButton(LEFT_BUTTON) == 1 &&
			GOLFDOC()->m_tutorialMode >= 1 &&
			!CUserInfo::Instance()->IsVisible() && !bGuideBook && !bScore &&
			!g_pFresh->GetManager()->HasValidWindow() &&
			MESSENGER()->IsGameFocus() == true && !bReview && bInView)
			ProcessMousePick(dt);
	}
}

// 983 00714a10: ponto do clique no chão; fora do campo (OB) não marca
void CCamera::ProcessTutorGuidePick(float dt)
{
	WVector mouse = g_input->GetMousePoint();
	WVector point;
	{
		WVector start;
		WVector ray;
		g_view->GetScreen2World(mouse.x, mouse.y, start, ray);
		point = GetPVS().GetRayIntersection(start, ray);
	}
	if (GOLFDOC()->m_pPolySoup->IsOutOfBound(point) &&
		GetPVS().IsOBArea(point) == true)
		return;
	SendMsg(this, "TutorGuide", 0x30f, (int)&point, 0, 0, 0);
}

void CCamera::ProcessForcedMove(float dt)
{
	float dir = m_rotDir == 1 ? 1.0f : -1.0f;
	Rotate((float)(dt * 1.0471975430846214 * 4.0 * dir)); // 983 00717240: 60 graus double * 4
	float ratio;
	if (Wabs(m_rotTotal) < g_EPSILON)
		ratio = 1.0f;
	else
		ratio = 1.0f - m_rotRemain / m_rotTotal;
	SendMsg(this, "Spot", M983(0xf7), (int)&ratio, 0, 0, 0);
}

void CCamera::OnProcess(float delta)
{
	if (!m_bEnable || CCapturedBg::Instance()->IsCaptured() ||
		(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].info.dwIdentity & 1))
		return;

	if (g_input->GetButton(MIDDLE_BUTTON) == 1 && CItemWindow::IsClosed())
	{
		m_bFreeLook = !m_bFreeLook;
		CMouseCursor::Instance()->SetCamMode(m_bFreeLook);
		if (m_bFreeLook)
			SendMsg(this, "Screen", M983(0x1b1), 0, 0, 0, 0);
	}

	if (CMouseCursor::Instance()->GetArea() == CMouseCursor::AREA_CAMRESET &&
		CMouseCursor::Instance()->IsInButtonDownArea(LEFT_BUTTON) &&
		g_input->GetButton(LEFT_BUTTON) == 2)
		SendMsg(this, "GolfRule", M983(0x78), 8, 0, 0, 0);

	if (OnlinePlay() && Doc()->m_replayState != 2)
	{
		if (IsMyTurn(false))
		{
			m_idleTime += delta;
			if (m_idleTime > 2.0f)
			{
				m_idleTime -= 2.0f;
				if (m_bSendAngle)
				{
					m_bSendAngle = false;
					WSendPacket packet((enumClientPacket)0x13);
					packet.EncodeBuffer(&GOLFDOC()->m_cameraData.angle, 4);
					packet.Send((eSendTo)0);
				}
			}
			goto process;
		}
	}
	else if (Doc()->m_gameMode != 5 || IsMyTurn(false))
		goto process;

	{
		float step = Between((float)(delta * -0.5235987715423107), // 983 007172d0: 30 graus double
			GetBaseAngle(m_angleEnd - m_angleStart), (float)(delta * 0.5235987715423107));
		if (!WisZero(step, g_EPSILON))
		{
			SendMsg(this, "Player", M983(0x1a1), (int)&step, 0, 0, 0);
			SendMsg(this, "Spot", M983(0x1a1), (int)&step, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a1), (int)&step, 0, 0, 0);
			SendMsg(this, "Grid", M983(0x1a1), (int)&step, 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x1a1), (int)&step, 0, 0, 0);
			SendMsg(this, "GuideLine", M983(0x1a1), 0, 0, 0, 0);
			m_angleStart = GetBaseAngle(step + m_angleStart);
			RotateForGallery(step);
		}
	}

process:
	ProcessToSpot(delta);
	FixedCamera(delta);

	if (GOLFDOC()->m_gameMode & 0x8000)
		return;

	if (m_timer > 0.0f)
	{
		m_timer -= delta;
		return;
	}

	if (WisZero(m_rotRemain, g_EPSILON))
		ProcessInput(delta);
	else
		ProcessForcedMove(delta);
	GetRotateAcc(delta);
	GetMoveAcc(delta);
	GetHorizonAcc(delta);
}
