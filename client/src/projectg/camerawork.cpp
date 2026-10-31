#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "camerawork.h"
#include "soundmanager.h"
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "scenemanager.h"
#include "wbone.h"
#include "commonutil.h"
#include "taskmanager.h"
#include "wpuppet.h"
#include "inputmanager.h"
#include "autotarget.h"
#include "golfcadie.h"
#include "wresrcmng.h"
#include <math.h>



// HACK: defer the empty constructor until the late frustum helper has emitted.
inline CCameraWork::sInput::sInput()
{
	if (0)
		((WView*)0)->InFrustumSafe(*(const WSphere*)0);
}

// HACK: scenemanager.h is part of the PCH; GetPuppet is declared on a helper class (see trajectory.cpp)
class CSceneManagerEx : public CSceneManager
{
public:
	WPuppet* GetPuppet(const char* name);
};

#pragma comment(linker, \
	"/alternatename:?GetPuppet@CSceneManagerEx@@QAEPAVWPuppet@@PBD@Z=?GetPuppet@CSceneManager@@QAEPAVWPuppet@@PBD@Z")

#define SCENEMGR() ((CSceneManagerEx*)CSceneManager::Instance())

class CQuadTreeEx : public CQuadTree
{
public:
	float RayIntersectAll(const WVector& start, const WVector& ray, bool bFlag);
};

// HACK: golfdoc.h is part of the PCH; GetDefaultCamera is declared on a helper class (as CSceneManagerEx above)
class CGolfDocEx : public CGolfDoc
{
public:
	WMatrix GetDefaultCamera();
};

#pragma comment(linker, \
	"/alternatename:?GetDefaultCamera@CGolfDocEx@@QAE?AVWMatrix@@XZ=?GetDefaultCamera@CGolfDoc@@QAE?AVWMatrix@@XZ")

#define GOLFDOCEX() ((CGolfDocEx*)GOLFDOC())

WVector GetTeeShotPos();
int WisZero(const float& f, float epsilon);

extern WMatrix g_camera;
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

int __fastcall Ceil(float value, float threshold);
// 983 006cedc0: funcao livre __fastcall (ecx = retorno, edx = eye), nao usa o this
WMatrix __fastcall GetCameraMatrix(const WVector& eye, const WVector& at, float roll);
int __fastcall float2int(float f);

template <>
CCameraWork* WSingleton<CCameraWork>::m_pInstance = NULL;

extern WResourceManager* g_resrcmng;

sCameraSelectionData puttTable[13] = {
	{ 1,  false, false },
    { 1,  false, false },
    { 1,  false, false },
	{ 1,  false, false },
    { 1,  false, false },
    { 0,  false, false },
	{ 0,  false, false },
    { 1,  false, false },
    { 1,  false, false },
	{ -1, false, false },
    { -1, false, false },
    { -1, false, false },
	{ -1, false, false }
};
sCameraSelectionData driveTable[29] = {
	{ 1,  true,  false },
    { 1,  true,  false },
    { -1, true,  false },
	{ 1,  true,  false },
    { 1,  true,  false },
    { 0,  true,  true  },
	{ 0,  false, false },
    { 0,  false, false },
    { 1,  true,  false },
	{ 1,  true,  false },
    { 1,  true,  false },
    { 1,  true,  false },
	{ 1,  true,  false },
    { -1, true,  false },
    { -1, true,  false },
	{ 0,  false, false },
    { 0,  false, false },
    { 0,  false, false },
	{ 1,  true,  false },
    { -1, true,  false },
    { 0,  false, false },
	{ -1, true,  false },
    { -1, true,  false },
    { 1,  true,  false },
	{ 0,  false, false },
    { 0,  false, false },
    { 0,  false, false },
	{ 0,  false, false },
    { 0,  false, true  }
};
static char* s_scriptKey = "*";
static char s_scriptCmd[0x400];

CCameraWork::CCameraWork()
{
	m_pfnCamera = NULL;
	m_sumDriveWeight = 0;
	m_sumPuttWeight = 0;
	m_b1df = false;
	m_numDriveCam = 29;
	m_numPuttCam = 13;
	int i;
	for (i = 0; i < m_numDriveCam; ++i)
	{
		m_sumDriveWeight += driveTable[i].weight > 0 ? driveTable[i].weight : 0;
		// 983 006d7c40: candidatas = segundo flag (+5, 00e16135); com o use (+4) entravam a 14 (ponto de colisao) e
		// outras que pedem dado da tacada -> CheckDrive...SafeCamera sorteava a 14 sem colisao e caia (dono4 00:41)
		if (driveTable[i].flag)
			m_camCandidates.push_back(0 + i);
	}
	for (i = 0; i < m_numPuttCam; ++i)
	{
		m_sumPuttWeight += puttTable[i].weight > 0 ? puttTable[i].weight : 0;
		if (puttTable[i].flag) // 983: 00e160cd (+5)
			m_camCandidates.push_back(100 + i);
	}
	m_holeCam[0].id = 0;
	m_holeCam[0].pPuppet = NULL;
	m_holeCam[1].id = 0;
	m_holeCam[1].pPuppet = NULL;
	m_holeCam[2].id = 0;
	m_holeCam[2].pPuppet = NULL;
	m_holeCam[3].id = 0;
	m_holeCam[3].pPuppet = NULL;
	m_pRankPuppet = NULL; // 983 006d7f65
	m_pPuppet983_240 = NULL;
	m_pPuppet = NULL;
	m_pMotion = NULL;
	m_f2a0 = 0;
	m_pLoadPos = (WVector*)&GolfBall();
	m_b2b0 = false;
	m_b1dc = false;
	m_bCameraChecking = true;
	m_i2ac = 1;
	m_b29c = false;
	m_loadCamMode983 = -1; // 983 006d7fd3
}

// JP 983 (006d7a90): m_vecA.clear() (o ~vector explicito do 645 libera duas vezes no VS2010) e solta tambem o puppet novo
CCameraWork::~CCameraWork()
{
	m_vecA.clear();
	for (int i = 0; i < 4; ++i)
	{
		if (g_resrcmng && m_holeCam[i].pPuppet)
		{
			g_resrcmng->Release(m_holeCam[i].pPuppet, false);
			m_holeCam[i].pPuppet = NULL;
		}
	}
	if (g_resrcmng && m_pRankPuppet)
	{
		g_resrcmng->Release(m_pRankPuppet, false);
		m_pRankPuppet = NULL;
	}
	if (g_resrcmng && m_pPuppet983_240)
	{
		g_resrcmng->Release(m_pPuppet983_240, false);
		m_pPuppet983_240 = NULL;
	}
}

void CCameraWork::Load()
{
	char name[64];
	CGolfDoc* doc = GOLFDOC();
	for (int i = 0; i < doc->m_playerNum; i++)
	{
		if (doc->GetPlayer(i)->state != 3)
		{
			unsigned long tid = GOLFDOC()->GetPartTidList(i).m_charTid;
			if (FindHoleInCam(tid) == NULL)
			{
				doc = GOLFDOC();
				for (int j = 0; j < doc->m_playerNum; j++)
				{
					if (doc->GetPlayer(j)->state != 3 && m_holeCam[j].id == 0)
					{
						IFF_STRUCT::sChar* ch =
							CSharedDoc::Instance()->m_itemManager.FindChar(tid);
						if (ch != NULL)
						{
							sprintf(name, "%s_cam.apet", ch->Data);
							m_holeCam[j].pPuppet =
								g_resrcmng->GetPuppet(name, false, true, false);
						}
						else
						{
							m_holeCam[j].pPuppet = NULL;
						}
						m_holeCam[j].id = tid;
						if (m_holeCam[j].pPuppet == NULL)
						{
							if ((tid & 0x3ffffff) == 4)
								m_holeCam[j].pPuppet = g_resrcmng->GetPuppet("m_def_cam.apet", false, true, false);
							else
								m_holeCam[j].pPuppet = g_resrcmng->GetPuppet("f_def_cam.apet", false, true, false);
						}
						break;
					}
					doc = GOLFDOC();
				}
			}
		}
		doc = GOLFDOC();
	}
	m_b2b1 = false;
}

void CCameraWork::ReplayCamera(bool bNext)
{
	if (!bNext)
	{
		m_vecB.clear();
	}
	int index;
	if (GolfClub().GetType() == 3)
	{
		index = m_i90 - 100;
		do
		{
			index = (index + 1) % m_numPuttCam;
		} while (puttTable[index].weight <= 0);
		index += 100;
	}
	else
	{
		index = m_i90;
		do
		{
			index = (index + 1) % m_numDriveCam;
		} while (driveTable[index].weight <= 0);
	}
	m_b34 = false;
	m_f28 = 0.0f;
	m_f30 = 0.0f;
	m_f2c = 0.0f;
	m_i90 = index;
	m_mode = -1;
	m_b1a0 = false;
	m_b1dc = false;
	m_f268 = 0.95993108f;
	m_i294 = 0;
	SetCamera(index, true);
	CheckCameraType(true);
	InitCtrlPoints(m_i8c);
}

void CCameraWork::SelectCamera(int type)
{
	m_b34 = false;
	m_f28 = 0.0f;
	m_f30 = 0.0f;
	m_f2c = 0.0f;
	m_mode = -1;
	m_b1df = false;
	m_pMotion = NULL;
	m_matA = g_camera;
	m_matB = g_camera;
	m_matSelect983 = g_camera; // 983 006db710..006db88a
	m_f268 = 0.95993108f;
	m_b1dc = (type == -1);
	m_i298 = rand();
	m_i294 = -1;
	m_vecB.clear();
	m_pLoadPos = (WVector*)&GolfBall();
	if (type == -1)
	{
		if (GolfClub().GetType() == 3)
		{
			int r = m_i298 % m_sumPuttWeight;
			type = 100;
			for (int i = 0; i < m_numPuttCam; i++)
			{
				r -= (puttTable[i].weight > 0) ? puttTable[i].weight : 0;
				if (r < 0)
				{
					type = i + 100;
					break;
				}
			}
		}
		else
		{
			int r = m_i298 % m_sumDriveWeight;
			type = 0;
			for (int i = 0; i < m_numDriveCam; i++)
			{
				r -= (driveTable[i].weight > 0) ? driveTable[i].weight : 0;
				if (r < 0)
				{
					type = i;
					break;
				}
			}
		}
	}
	m_i90 = type;
	m_pPuppet = NULL;
	SetCamera(type, true);
	if (type < 200)
	{
		CheckCameraType(false);
		InitCtrlPoints(m_i8c);
	}
}

void CCameraWork::SetCaddieCamera(const WMatrix& mat, const WVector& pos)
{
	// 983 006dba40: contas em WVector; a camera nasce no meio do caminho bola -> caddie, 10 acima
	m_f268 = 0.95993108f;
	m_b1dc = false;
	m_b1df = false;
	m_matA = mat;
	SetCamera(0xca, true);
	m_matA.GetRotMatrix(&m_matC);
	WVector dir;
	dir = WVector::UNIT_POS_Z * m_matC;
	WVector d;
	d = pos - GolfBall().m_pos;
	m_f1a8 = CalcDeltaAngle(d, dir);
	WVector half = d * 0.5f;
	m_srcCamera = half + GolfBall().m_pos + WVector(0.0f, 10.0f, 0.0f);
	m_posFrom = m_srcCamera;
	m_posTo = mat.pivot;
	m_matB = m_matA;
}

// 983 (A s159): o build do exe (REBANG_SEMANTIC_ONLY) chama esta funcao pelas versoes em C e nao a tinha (stub); os HACK
// abaixo so servem ao build byte a byte do 645.
WPuppet* CCameraWork::FindHoleInCam(unsigned long id)
{
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (m_holeCam[i].id == id)
		{
			return m_holeCam[i].pPuppet;
		}
	}
	return 0;
}

// JP 983 (006d77d0). Diferencas para o 645: guarda o modo (m_loadCamMode983; -1 se desconhecido) e zera m_b2b1; modos 0/2 poem
// m_f268 = 55 graus; modo 1 sem o tipo de jogo 3 e com "grandprix_cam.apet" (tipo 4, realGameType 0x14/0x15); modo 3 marca
// m_b2b1 quando a tacada e a "E_killerwhale01"; modos novos 4 (preview do Mland, puppet guardado em m_pPuppet983_240, 1o
// movimento) e 5 ("memorial_cam.apet", sem guardar); o resto passou a LoadCameraMotion983 (006d6be0).
float CCameraWork::LoadCamera(eCamMode mode, char* name, const WVector& pos)
{
	m_b1df = true;
	m_b1dc = false;
	m_loadPos = pos;
	m_f2a0 = 0;
	m_b2b1 = false;
	m_pLoadPos = &m_loadPos;
	m_loadCamMode983 = mode;
	if (mode == 0 || mode == 2)
	{
		m_f268 = 0.95993108f;
		m_pPuppet = FindHoleInCam(GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer).m_charTid);
	}
	else if (mode == 1)
	{
		if (m_pRankPuppet == NULL)
		{
			switch (CSharedDoc::Instance()->m_golfGame.gameType)
			{
			case 0:
			case 7:
				m_pRankPuppet = g_resrcmng->GetPuppet("rank_stroke.apet", false, true, false);
				break;
			case 1:
				m_pRankPuppet = g_resrcmng->GetPuppet("rank_match.apet", false, true, false);
				break;
			case 4:
				if ((unsigned)(CSharedDoc::Instance()->m_roomInfo.realGameType - 0x14) <= 1)
					m_pRankPuppet = g_resrcmng->GetPuppet("grandprix_cam.apet", false, true, false);
				break;
			}
		}
		m_pPuppet = m_pRankPuppet;
	}
	else if (mode == 3)
	{
		m_pPuppet = FindHoleInCam(GOLFDOC()->GetPartTidList(GOLFDOC()->m_currentPlayer).m_charTid);
		char* swing = NULL;
		rand();
		CTask* task = (CTaskManager::Instance() == NULL)
			? NULL
			: CTaskManager::Instance()->GetCurrentTask();
		IActor* player = task->GetActor("Player");
		if (player != NULL)
		{
			player << MsgObject(NULL, 0x2715, (int)&swing, 0, 0, 0, 0);
		}
		name = MakeStr("%s_%02d", swing, 1);
		if (strstr(swing, "E_killerwhale01"))
			m_b2b1 = true;
	}
	else if (mode == 4)
	{
		if (m_pPuppet983_240 == NULL)
			m_pPuppet983_240 = g_resrcmng->GetPuppet("mland_gamepreview.apet", false, true, false);
		m_pPuppet = m_pPuppet983_240;
		name = m_pPuppet->GetFirstMotionData()->name;
	}
	else if (mode == 5)
	{
		m_pPuppet = g_resrcmng->GetPuppet("memorial_cam.apet", false, true, false);
	}
	else
	{
		m_pPuppet = NULL;
		m_loadCamMode983 = -1;
	}
	return LoadCameraMotion983(mode, name);
}

// JP 983 (006d6be0): fim do LoadCamera do 645 (movimento, InitVars, duracao). Modos 0 e 3: a soma para em 10 (o 645 parava
// antes de passar de 10 e so nos modos 0/203); modo 5: para depois de passar de 300.
float CCameraWork::LoadCameraMotion983(int mode, char* name)
{
	if (m_pPuppet != NULL)
		m_pMotion = m_pPuppet->FindMotionData(name);
	else
		m_pMotion = NULL;
	InitVars();
	if (m_pMotion != NULL)
	{
		m_f190 = m_pMotion->s;
		float total = m_pMotion->e - m_pMotion->s;
		for (w_motion_data* next = m_pPuppet->FindMotionData(m_pMotion->nextmotion); next != NULL;
			next = m_pPuppet->FindMotionData(next->nextmotion))
		{
			float len = next->e - next->s;
			float absLen = len;
			if (*(unsigned int*)&absLen & 0x80000000)
				absLen = -absLen;
			if (absLen < g_EPSILON)
				break;
			total += len;
			if (mode == 0 || mode == 3)
			{
				if (total > 10.0f)
				{
					total = 10.0f;
					break;
				}
			}
			if (mode == 5 && total > 300.0f)
				break;
		}
		m_f264 = m_pMotion->s + total;
		return total;
	}
	m_f264 = -1.0f;
	return -1.0f;
}

// JP 983 (006cde90): devolve referencia; o HACK do bloco de constantes do 645 saiu (o 983 nao tem)
const WVector& CCameraWork::GetSourceCamera()
{
	return m_srcCamera;
}

// JP 983 (006cdea0)
const WVector& CCameraWork::GetDestCamera()
{
	return m_destCamera;
}

void CCameraWork::SetDestCamera(const WVector& dest)
{
	m_destCamera = dest;
}

void CCameraWork::SetInstalledCamera(const WVector& pos)
{
	SetCamera(0x15, false);
	m_srcCamera = pos;
	m_b29c = true;
}

void CCameraWork::UninstallCamera()
{
	m_b29c = false;
}

void CCameraWork::SetCamera(int type, bool bForce)
{
	if (m_b29c)
	{
		return;
	}
	CGolfBall& ball = GolfBall();
	int render = ball.m_renderNum;
	int top = ball.m_topFrame > 0 ? ball.m_topFrame : 0;
	int bounce = ball.m_bounceFrame > 0 ? ball.m_bounceFrame : 0;
	m_i8c = type;
	m_ibc = -1;
	int count = (int)m_vecB.size();
	if (m_i294 < 0 || m_i294 >= count)
	{
		// recording: remember the camera that was picked
		m_i294 = -1;
		m_vecB.push_back(m_i8c);
	}
	else
	{
		// replaying: take the recorded camera instead
		m_i8c = m_vecB[m_i294++];
	}
	if (!m_b1dc)
	{
		m_b34 = false;
		m_f28 = 0.0f;
		m_f30 = 0.0f;
		m_f2c = 0.0f;
	}
	if (bForce)
	{
		m_srcCamera = m_matA.pivot;
		WVector dest(m_matA.za.x + m_matA.pivot.x, m_matA.za.y + m_matA.pivot.y,
			m_matA.za.z + m_matA.pivot.z);
		m_destCamera = dest;
		m_matB = GetCameraMatrix(m_srcCamera, m_destCamera, m_f2a0);
		InitVars();
	}
	if (m_i8c < 100 && bForce)
	{
		m_fa8 = 75.0f;
	}
	else
	{
		m_fa8 = -1.0f;
	}
	m_f2a0 = 0.0f;
	m_ib4 = -2;
	if (m_i8c < 100 && bForce && GolfBall().m_renderNum < 50)
	{
		m_pfnCamera = NULL;
		return;
	}
	switch (m_i8c)
	{
	default:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FollowCamera;
		break;
	case 1:
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::LookBackCamera;
		break;
	case 2:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::Rail_or_SideCamera;
		break;
	case 3:
	{
		WVector up(0.0f, m_i298 % 2 ? 1.0f : -1.0f, 0.0f);
		WVector side;
		side = WCrossProduct(up, ball.PreData(top)->vel);
		side = side.Normalize() * 10.0f;
		float height = (float)(m_i298 % 11) - 5.0f;
		const CGolfBall::sExpected* p = ball.PreData(top);
		WVector pos(side.x + p->pos.x, height + p->pos.y + side.y,
			side.z + p->pos.z);
		m_v180 = pos;
		m_flags = m_i298 % 2 + 1;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::PanCamera;
		break;
	}
	case 4:
	case 11:
	{
		WVector dir;
		dir = ball.PreData(render)->pos - ball.PreData(bounce)->pos;
		if (!WisZero(dir.x, g_EPSILON) || !WisZero(dir.z, g_EPSILON))
		{
			dir.y = 0.0f;
			dir = RotVec(dir.Normalize(),
				RotMat(Random(-2.0943952f, 2.0943952f, m_i298), 1));
		}
		else
		{
			dir = ball.PreData(render)->pos - ball.PreData(0)->pos;
			dir.y = 0.0f;
			dir.Normalize();
		}
		WVector pos;
		if (ball.m_bOB)
		{
			WVector off = dir * 50.0f;
			const CGolfBall::sExpected* p = ball.PreData(render);
			pos.x = off.x + p->pos.x;
			pos.y = off.y + p->pos.y + 25.0f;
			pos.z = off.z + p->pos.z;
		}
		else
		{
			WVector off = dir * 20.0f;
			const CGolfBall::sExpected* p = ball.PreData(render);
			pos.x = off.x + p->pos.x;
			pos.y = off.y + p->pos.y + 10.0f;
			pos.z = off.z + p->pos.z;
		}
		m_v180 = pos;
		if (m_i8c == 4)
		{
			*(CameraFn*)&m_pfnCamera = &CCameraWork::DropCamera;
		}
		else
		{
			*(CameraFn*)&m_pfnCamera = &CCameraWork::Drop_and_ZoomCamera;
		}
		break;
	}
	case 5:
		if (GolfClub().GetType() == 3)
		{
			m_fa8 = 25.0f;
		}
		if (m_ib8 > 10)
		{
			m_destCamera = ball.PreData(m_ib8 - 10)->pos;
			WVector dir;
			dir = ball.PreData(m_ib8 - 10)->pos - ball.PreData(m_ib8 - 9)->pos;
			dir.y = 0.0f;
			dir.Normalize();
			dir.y = 0.5f;
			dir.Normalize();
			WVector off = dir * 40.0f;
			const CGolfBall::sExpected* p = ball.PreData(m_ib8);
			WVector pos(off.x + p->pos.x, off.y + p->pos.y, off.z + p->pos.z);
			m_srcCamera = pos;
		}
		m_f268 = 0.95993108f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::Chase_and_FollowCamera;
		break;
	case 7:
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::CircleCamera;
		break;
	case 8:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::Chase_and_LookBackCamera;
		break;
	case 9:
	case 12:
	case 18:
	case 27:
	{
		// (four identical copies in the original, one per case)
		m_fa8 = m_i8c == 27 ? 25.0f : -1.0f;
		WVector side;
		side = WCrossProduct(WVector::UNIT_POS_Y, ball.PreData(top)->vel);
		side = side.Normalize() * 40.0f;
		side.y += 8.0f;
		WVector vel;
		vel = ball.PreData(top)->vel;
		const CGolfBall::sExpected* first = ball.PreData(0);
		WVector pos(side.x + first->pos.x, side.y + first->pos.y,
			side.z + first->pos.z);
		float along = (float)(m_i298 % 11) - 5.0f;
		m_srcCamera = pos + vel.Normalize() * along;
		m_destCamera = ball.PreData(0)->pos;
		m_destCamera.y += 2.0f;
		if (m_i8c == 9)
		{
			*(CameraFn*)&m_pfnCamera = &CCameraWork::ZoomInCamera;
		}
		else if (m_i8c == 12)
		{
			*(CameraFn*)&m_pfnCamera = &CCameraWork::BezierCamera;
		}
		else if (m_i8c == 18)
		{
			m_f1a8 = 0.2617994f;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::FollowWithSwingCamera;
		}
		else
		{
			*(CameraFn*)&m_pfnCamera = &CCameraWork::SpikeACamera;
		}
		break;
	}
	case 10:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::CaddyViewCamera;
		break;
	case 13:
	{
		m_fa8 = -1.0f;
		WVector up(0.0f, m_i298 % 2 ? 1.0f : -1.0f, 0.0f);
		WVector side;
		side = WCrossProduct(up, ball.PreData(0)->vel);
		side.y = 0.0f;
		side = side.Normalize() * 30.0f;
		const CGolfBall::sExpected* first = ball.PreData(0);
		WVector half = (ball.PreData(render)->pos - first->pos) * 0.5f;
		first = ball.PreData(0);
		WVector pos(half.x + first->pos.x + side.x,
			half.y + first->pos.y + side.y, half.z + first->pos.z + side.z);
		m_srcCamera = pos;
		m_srcCamera.y = ball.PreData(top)->pos.y + 2.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FromMidPointCamera;
		break;
	}
	case 14:
	{
		int col = m_ib0;
		m_fa8 = -1.0f;
		WVector out =
			(ball.PreData(col + 1)->pos - ball.PreData(col)->pos).Normalize();
		WVector in =
			(ball.PreData(col - 1)->pos - ball.PreData(col)->pos).Normalize();
		WVector dir;
		dir = in + out;
		dir.y = 0.0f;
		dir = dir.Normalize() * 160.0f;
		const CGolfBall::sExpected* p = ball.PreData(m_ib0);
		WVector pos(dir.x + p->pos.x, dir.y + p->pos.y, dir.z + p->pos.z);
		m_srcCamera = pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FromColPointCamera;
		m_srcCamera.y += 96.0f;
		break;
	}
	case 19:
		m_fa8 = -1.0f;
		m_f190 = 0.0f;
		m_f1a8 = 0.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::SidePanWithZoomInCamera;
		break;
	case 20:
	{
		WPuppet* puppet = SCENEMGR()->GetPuppet(
			MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer));
		WBone* nub = puppet->FindBone("Bip01 HeadNub");
		WBone* head = puppet->FindBone("Bip01 Head");
		m_destCamera =
			0.7f * head->GetMatrix().pivot + 0.3f * nub->GetMatrix().pivot;
		const WVector& back =
			RotMat(GOLFDOC()->m_cameraData.angle + 0.34906584f, 1).za;
		WVector pos(m_destCamera.x - back.x * 3.0f,
			m_destCamera.y - back.y * 3.0f, m_destCamera.z - back.z * 3.0f);
		m_srcCamera = pos;
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::PangyaACamera;
		m_srcCamera.y += 0.5f;
		break;
	}
	case 21:
	{
		m_f1a8 = 1.5707964f;
		const CGolfBall::sExpected* first = ball.PreData(0);
		WVector dest(first->pos.x, first->pos.y + 3.0f, first->pos.z);
		m_destCamera = dest;
		const WVector& back =
			RotMat(GOLFDOC()->m_cameraData.angle + m_f1a8, 1).za;
		first = ball.PreData(0);
		WVector pos(first->pos.x - back.x * 15.0f,
			first->pos.y - back.y * 15.0f + 3.0f,
			first->pos.z - back.z * 15.0f);
		m_srcCamera = pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FixedAtSideCamera;
		break;
	}
	case 22:
	{
		const CGolfBall::sExpected* first = GolfBall().PreData(0);
		WVector dest(first->pos.x, first->pos.y + 3.0f, first->pos.z);
		m_destCamera = dest;
		const WVector& front = RotMat(Random(-1.5707964f, 1.5707964f, m_i298) +
				GOLFDOC()->m_cameraData.angle,
			1)
								   .za;
		first = ball.PreData(0);
		WVector pos(front.x * 30.0f + first->pos.x,
			front.y * 30.0f + first->pos.y + 10.0f,
			front.z * 30.0f + first->pos.z);
		m_srcCamera = pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FromRandomPointCamera;
		break;
	}
	case 23:
		m_fa8 = 0.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::BankFromHeadCamera;
		break;
	case 24:
		if (m_i2a4 == 0 && m_pPuppet == NULL)
		{
			m_fa8 = 25.0f;
			WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
			WVector off(10.0f, 0.0f, -20.0f);
			m_srcCamera = ball.PreData(0)->pos + RotVec(off, rot);
			WVector at(0.0f, 2.0f, -2.0f);
			m_destCamera = ball.PreData(0)->pos + RotVec(at, rot);
			*(CameraFn*)&m_pfnCamera = &CCameraWork::CobraACamera;
		}
		else
		{
			m_fa8 = -1.0f;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::CobraACamera;
		}
		break;
	case 25:
		if (m_i2a4 == 0 && m_pPuppet == NULL)
		{
			m_fa8 = 25.0f;
			m_ib8 = GolfBall().m_playFrame;
			if (m_ib8 < GolfBall().m_cobraFrame)
			{
				m_srcCamera = GOLFDOC()->m_cameraData.mat.pivot;
				m_destCamera = m_srcCamera + GOLFDOC()->m_cameraData.mat.za;
				*(CameraFn*)&m_pfnCamera = &CCameraWork::CobraBCamera;
			}
			else
			{
				m_f2a0 = 0.2617994f -
					(float)(m_ib8 - GolfBall().m_cobraFrame) * 0.0017453292f;
				int index = (GolfBall().m_cobraFrame + m_ib8 * 4) / 5;
				m_srcCamera = GolfBall().PreData(index)->pos;
				m_srcCamera.y += 1.0f;
				m_destCamera = GolfBall().m_pos;
				*(CameraFn*)&m_pfnCamera = &CCameraWork::CobraBCamera;
			}
		}
		else
		{
			m_fa8 = -1.0f;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::CobraBCamera;
		}
		break;
	case 26:
		if (m_i2a4 == 0 && m_pPuppet == NULL)
		{
			m_fa8 = 25.0f;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::TomahawkACamera;
		}
		else
		{
			m_fa8 = -1.0f;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::TomahawkACamera;
		}
		break;
	case 28:
	{
		int cur = m_ib8;
		float speed = ball.PreData(cur)->vel.Magnitude();
		float dist = Between<float>(100.0f, speed + speed, 150.0f);
		if (cur > 10)
		{
			m_destCamera = ball.PreData(cur - 10)->pos;
			WVector dir;
			dir = ball.PreData(m_ib8 - 10)->pos - ball.PreData(m_ib8 - 9)->pos;
			dir.y = 0.0f;
			dir.Normalize();
			dir.y = 2.0f;
			dir.Normalize();
			WVector off(dist * dir.x, dir.y * dist, dir.z * dist);
			const CGolfBall::sExpected* p = ball.PreData(m_ib8);
			WVector pos(off.x + p->pos.x, off.y + p->pos.y, off.z + p->pos.z);
			m_srcCamera = pos;
		}
		m_f268 = 0.95993108f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::TopViewCamera;
		break;
	}
	case 100:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::JustLookingCamera;
		break;
	case 101:
	{
		WVector side;
		side = WCrossProduct(WVector::UNIT_POS_Y, ball.PreData(0)->vel);
		side.Normalize();
		const CGolfBall::sExpected* first = ball.PreData(0);
		WVector pos(side.x * 16.0f + first->pos.x,
			side.y * 16.0f + first->pos.y + 32.0f,
			side.z * 16.0f + first->pos.z);
		m_srcCamera = pos;
		m_destCamera = ball.PreData(0)->pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::SideCamera;
		m_destCamera.y += 2.5f;
		break;
	}
	case 102:
	case 103:
	case 104:
	{
		// (the original shares only the tail of these three cases)
		WVector pos;
		if (m_i8c == 102)
		{
			WVector dir;
			dir = ball.PreData(0)->vel;
			const WVector& n = dir.Normalize();
			const CGolfBall::sExpected* first = ball.PreData(0);
			pos.x = first->pos.x - n.x * 5.0f;
			pos.y = first->pos.y + 25.0f - n.y * 5.0f;
			pos.z = first->pos.z - n.z * 5.0f;
		}
		else if (m_i8c == 103)
		{
			WVector dir;
			dir = (ball.PreData(0)->pos - GetHoleCupPos()).Normalize();
			WVector up(0.0f, m_i298 % 2 ? 1.0f : -1.0f, 0.0f);
			const WVector& side = WCrossProduct(up, dir).Normalize();
			WVector off((dir.x + side.x) * 3.0f, (dir.y + side.y) * 3.0f,
				(dir.z + side.z) * 3.0f);
			const WVector& pin = GetHoleCupPos();
			pos.x = pin.x - off.x;
			pos.y = pin.y + 25.0f - off.y;
			pos.z = pin.z - off.z;
		}
		else
		{
			WVector dir;
			dir = (ball.PreData(0)->pos - ball.PreData(ball.m_renderNum)->pos)
					  .Normalize();
			WVector up(0.0f, m_i298 % 2 ? 1.0f : -1.0f, 0.0f);
			WVector off = WCrossProduct(up, dir).Normalize() * 3.0f;
			const CGolfBall::sExpected* mid =
				ball.PreData(ball.m_renderNum / 2);
			pos.x = off.x + mid->pos.x;
			pos.y = off.y + mid->pos.y;
			pos.z = off.z + mid->pos.z;
		}
		m_srcCamera = pos;
		m_destCamera = ball.PreData(0)->pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::JustLookingCamera;
		break;
	}
	case 105:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::BallSideCamera;
		break;
	case 106:
	{
		const CGolfBall::sExpected* first = ball.PreData(0);
		WVector dest(first->pos.x, first->pos.y + 3.0f, first->pos.z);
		m_destCamera = dest;
		const WVector& back =
			RotMat(GOLFDOC()->m_cameraData.angle + 1.7453293f, 1).za;
		WVector pos(m_destCamera.x - back.x * 16.0f,
			m_destCamera.y - back.y * 16.0f, m_destCamera.z - back.z * 16.0f);
		m_srcCamera = pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FollowBallCamera;
		m_b26c = false;
		break;
	}
	case 107:
	{
		WVector dir;
		dir = GetHoleCupPos() - ball.PreData(0)->pos;
		dir.y = 0.0f;
		WVector off = dir.Normalize() * 20.0f;
		m_srcCamera = ball.PreData(0)->pos + off * RotMat(-2.6179938f, 1);
		m_srcCamera.y += 20.0f;
		m_destCamera = (ball.PreData(0)->pos + GetHoleCupPos()) * 0.5f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FixedCamera;
		break;
	}
	case 108:
	{
		WVector dir;
		dir = GetHoleCupPos() - ball.PreData(0)->pos;
		dir.y = 0.0f;
		WVector off = dir.Normalize() * 20.0f;
		m_srcCamera = GetHoleCupPos() + off * RotMat(-1.0471976f, 1);
		m_srcCamera.y += 20.0f;
		m_destCamera = (ball.PreData(0)->pos + GetHoleCupPos()) * 0.5f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FixedCamera;
		break;
	}
	case 109:
	{
		WVector dir;
		dir = GetHoleCupPos() - ball.PreData(0)->pos;
		dir.y = 0.0f;
		WVector off = dir.Normalize() * 10.0f;
		m_srcCamera = GetHoleCupPos() + off * RotMat(-1.5707964f, 1);
		m_srcCamera.y += 20.0f;
		m_destCamera = ball.m_pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::JustLookingCamera;
		break;
	}
	case 110:
	{
		WVector dir;
		dir = ball.m_vel;
		WVector off = dir.Normalize() * 10.0f;
		const WVector& pin = GetHoleCupPos();
		WVector pos(pin.x - off.x, pin.y - off.y, pin.z - off.z);
		m_srcCamera = pos;
		m_srcCamera.y += 10.0f;
		m_destCamera = ball.m_pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::JustLookingCamera;
		break;
	}
	case 111:
	{
		WVector dir;
		dir = GetHoleCupPos() - ball.m_pos;
		dir.y = 0.0f;
		WVector off = dir.Normalize() * 15.0f;
		m_srcCamera = GetHoleCupPos() + off * RotMat(-0.34906584f, 1);
		m_srcCamera.y += 20.0f;
		m_destCamera = ball.m_pos;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::PuttSecondACamera;
		break;
	}
	case 112:
	{
		WVector dir;
		dir = GetHoleCupPos() - ball.PreData(0)->pos;
		dir.y = 0.0f;
		dir.Normalize();
		WVector off = dir * 60.0f;
		m_srcCamera = GetHoleCupPos() + off * RotMat(-0.05235988f, 1);
		m_destCamera = ball.m_pos;
		m_f268 = 0.17453292f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::PuttSecondBCamera;
		break;
	}
	case 150:
	{
		const WVector& pin = GetHoleCupPos();
		WVector pos(pin.x, pin.y - 2.0f, pin.z);
		m_srcCamera = pos;
		WVector dest(m_srcCamera.x, m_srcCamera.y + 1.0f, m_srcCamera.z + 0.2f);
		m_destCamera = dest;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FromHoleCupCamera;
		break;
	}
	case 200:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::FirstPersonCamera;
		break;
	case 201:
		*(CameraFn*)&m_pfnCamera = &CCameraWork::StillFrameCamera;
		break;
	case 202:
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::CaddieChanceCamera;
		break;
	case 203:
	{
		// 983 006da513: so o LoadCamera(3); o desvio do 645 (jogo de massa/tipo 10 -> camera 204) saiu
		float length = LoadCamera((eCamMode)3, NULL, GetTeeShotPos());
		m_b26c = false;
		m_fa8 = -1.0f;
		if (length <= 0.0f)
		{
			m_b1df = false;
			*(CameraFn*)&m_pfnCamera = &CCameraWork::IntroACamera;
			m_f1a8 = 2.6179938f;
			m_f25c = 80.0f;
			m_f260 = 15.0f;
			break;
		}
		m_pfnCamera = NULL;
		break;
	}
	case kTeeBackCamera983:
	{
		// 983 case 204 (006da40c): 25 behind the ball along the hole angle, 5 up, then ToDefaultCamera (983 0xcd)
		const WVector& front = RotMat(GOLFDOC()->m_cameraData.angle, 1).za;
		WVector back(front.x * 25.0f, front.y * 25.0f, front.z * 25.0f);
		const WVector& b = GolfBall().m_pos;
		WVector pos(b.x - back.x, b.y - back.y, b.z - back.z);
		m_srcCamera = pos;
		m_srcCamera.y += 5.0f;
		m_destCamera = m_srcCamera + RotMat(GOLFDOC()->m_cameraData.angle, 1).za;
		m_matB = GetCameraMatrix(m_srcCamera, m_destCamera, 0.0f);
		SetCamera(204, false);
		break;
	}
	case 204:
		m_quatFrom = m_matB;
		m_quatTo = GOLFDOCEX()->GetDefaultCamera();
		m_posFrom = m_matB.pivot;
		m_posTo = GOLFDOC()->m_cameraData.mat.pivot;
		m_f190 = 0.0f;
		m_f264 = 1.0f;
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::ToDefaultCamera;
		break;
	case 205:
		m_matB = GOLFDOCEX()->GetDefaultCamera();
		m_srcCamera = m_matB.pivot;
		m_destCamera = m_srcCamera + m_matB.za;
		m_f268 = 0.017453292f;
		m_f264 = 1.0f;
		m_f190 = 0.0f;
		m_fa8 = -1.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::ZoomOutCamera;
		break;
	case 206:
		m_fa8 = -1.0f;
		if (m_i2a4 == 1)
		{
			WVector off = (GOLFDOC()->m_cameraData.mat.ya +
							  GOLFDOC()->m_cameraData.mat.za) *
				32.0f;
			m_srcCamera = GolfBall().m_pos + off * RotMat(-1.9198622f, 1);
			m_destCamera = GolfBall().m_pos;
		}
		else if (m_i2a4 == 2)
		{
			WVector off = (GOLFDOC()->m_cameraData.mat.ya +
							  GOLFDOC()->m_cameraData.mat.za) *
				32.0f;
			m_srcCamera = GolfBall().m_pos + off * RotMat(2.0943952f, 1);
			const WVector& b = GolfBall().m_pos;
			WVector dest(b.x, b.y + 2.0f, b.z);
			m_destCamera = dest;
		}
		m_pfnCamera = NULL;
		break;
	case 207:
		m_fa8 = -1.0f;
		if (m_i2a4 == 1)
		{
			WVector from(-5.92f, -4.61f, 37.5f);
			m_posFrom = from * GOLFDOC()->m_cameraData.mat;
			WVector fromAt(m_posFrom.x + 0.21f, m_posFrom.y + 0.3f,
				m_posFrom.z - 0.93f);
			m_quatFrom = MakeCameraMatrix(m_posFrom, fromAt);
			WVector to(0.99f, -5.4f, 37.78f);
			m_posTo = to * GOLFDOC()->m_cameraData.mat;
			WVector toAt(m_posTo.x - 0.3f, m_posTo.y + 0.41f,
				m_posTo.z - 0.86f);
			m_quatTo = MakeCameraMatrix(m_posTo, toAt);
		}
		else if (m_i2a4 == 2)
		{
			WVector from(-10.28f, -0.51f, 17.76f);
			m_posFrom = from * GOLFDOC()->m_cameraData.mat;
			WVector fromAt(m_posFrom.x + 0.93f, m_posFrom.y + 0.09f,
				m_posFrom.z + 0.35f);
			m_quatFrom = MakeCameraMatrix(m_posFrom, fromAt);
			WVector to(-6.45f, 0.62f, 13.05f);
			m_posTo = to * GOLFDOC()->m_cameraData.mat;
			WVector toAt(m_posTo.x + 0.1f, m_posTo.y - 0.07f,
				m_posTo.z + 0.99f);
			m_quatTo = MakeCameraMatrix(m_posTo, toAt);
		}
		m_f264 = 0.5f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::SwingReplayBCamera;
		break;
	case 208:
		m_fa8 = -1.0f;
		m_destCamera = GetHoleCupPos();
		m_f1a8 = 0.0f;
		m_f25c = 64.0f;
		m_destCamera.y += 3.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::ExtraTimeCamera;
		m_i2ac = 0;
		break;
	case 209:
		// 983 case 210 (006dab78): = 209, turning at 2 deg/s (006cecf0); ScoreBoard of GP modes 0x14/0x15 (0067b4a0)
		m_fa8 = -1.0f;
		m_destCamera = GetHoleCupPos();
		m_f1a8 = 0.0f;
		m_f25c = 64.0f;
		m_destCamera.y += 3.0f;
		*(CameraFn*)&m_pfnCamera = &CCameraWork::ExtraTimeSlowCamera983;
		m_i2ac = 0;
		break;
	}
	m_v48 = m_srcCamera;
	m_v54 = m_srcCamera;
}

void CCameraWork::CheckCameraType(bool bForce)
{
	if (!m_bCameraChecking)
	{
		return;
	}
	if (GolfClub().GetType() == 3)
	{
		if (GolfBall().m_groundType == 2)
		{
			WVector toHole = GetHoleCupPos() - GolfBall().m_pos;
			float angle = CalcDeltaAngle(toHole, GolfBall().m_vel);
			if (angle <= 0.0f)
			{
				angle = -angle;
			}
			if (angle <= 0.7853981573134661)
			{
				return;
			}
			SetCamera(100, true);
			return;
		}
		unsigned int count = m_camCandidates.size();
		SetCamera(m_camCandidates[rand() % count], true);
		return;
	}
	if (GOLFDOC()->m_bar.grade == 1) // 983 006daee6: CGolfDoc+0xf0 (grade da barra), o 645 lia m_gameMode
	{
		if (GolfBall().m_renderNum != 0)
		{
			switch (m_i298 % 3)
			{
			case 0:
				SetCamera(100, true);
				return;
			case 1:
				SetCamera(0x15, true);
				return;
			case 2:
				break;
			default:
				return;
			}
		}
		SetCamera(0x16, true);
		return;
	}
	unsigned int special = GolfBall().m_initSpecial & 0x70;
	if (special != 0)
	{
		const CGolfBall::sExpected* pre =
			GolfBall().PreData(GolfBall().m_renderNum);
		WVector d = GetHoleCupPos() - pre->pos;
		int chance = 1;
		if (bForce)
		{
			chance = 5;
		}
		if (WVector(d.x, 0.0f, d.z).Magnitude() < 16.0f)
		{
			int r = rand();
			switch (special)
			{
			case 0x40:
				if (r % 100 < chance * 5 &&
					GolfBall().m_spikeEndFrame == GolfBall().m_bounceFrame)
				{
					SetCamera(0x1b, true);
				}
				break;
			case 0x20:
				if (r % 100 < chance * 10)
				{
					if (r % 10 < 5)
					{
						SetCamera(0x18, true);
					}
					else
					{
						SetCamera(0x19, true);
					}
				}
				break;
			case 0x10:
				if (r % 100 < chance * 5 &&
					GolfBall().m_tomahawkFrame == GolfBall().m_bounceFrame)
				{
					SetCamera(0x1a, true);
				}
				break;
			}
		}
	}
	if (m_fa0 < 0.0f)
	{
		return;
	}
	if (m_i8c == 10)
	{
		if (!m_b1de)
		{
			SetCamera(0, true);
			return;
		}
		IActor* caddie = AfxGetTask()->GetActor("Caddie");
		if (caddie != NULL)
		{
			CGolfCaddie::sCaddie* cad = NULL;
			caddie << MsgObject(NULL, M983(0x1e0), (int)&cad, 0, 0, 0, 0);
			// 983: m_bCaddieView (+0x45); the 645 literal +0x3d read a byte of m_startDelay
			if (cad == NULL || !cad->m_bCaddieView)
			{
				SetCamera(0, true);
			}
		}
	}
	m_ib0 = 0;
	while (m_ib0 < (int)m_fa4)
	{
		const CGolfBall::sExpected* pre = GolfBall().PreData(m_ib0);
		if (pre != NULL && pre->event == 2)
		{
			break;
		}
		m_ib0++;
	}
	int landing = m_ib0;
	if (landing < (int)m_fa0)
	{
		m_flags |= 0x20000000;
		if ((float)landing - m_fa8 < 100.0)
		{
			SetCamera(0xe, true);
			return;
		}
		if (m_i8c != 4 && m_i8c != 0xb)
		{
			return;
		}
	}
	else
	{
		if (!(m_fa0 - m_fa8 < 100.0) && !(m_f9c - m_fa8 < 50.0))
		{
			if (!(m_fa0 - m_fa8 < 150.0))
			{
				return;
			}
			switch (m_i298 % 2)
			{
			case 0:
				SetCamera(4, true);
				return;
			case 1:
				break;
			default:
				return;
			}
		}
	}
	SetCamera(0xd, true);
}


void CCameraWork::InitCtrlPoints(int type)
{
	CGolfBall& ball = GolfBall();
	int render = ball.m_renderNum;
	int top = ball.m_topFrame > 0 ? ball.m_topFrame : 0;
	int bounce = ball.m_bounceFrame > 0 ? ball.m_bounceFrame : 0;
	int roll = ball.m_rollFrame > 0 ? ball.m_rollFrame : 0;
	int cur = (int)m_fa8; // 983 006cf124: _ftol2 trunca (sem +0.5)
	int curClamped;
	if (cur < 0)
	{
		curClamped = 0;
	}
	else
	{
		curClamped = cur;
		if (cur > render)
		{
			curClamped = render;
		}
	}
	if (top == 0)
	{
		top = render / 2;
	}
	if (roll < top)
	{
		top = ball.m_renderNum / 3;
		roll = (ball.m_renderNum * 2) / 3;
	}
	m_f18c = 5.0f;

	WVector flat;
	if (GolfBall().IsHoleIn())
	{
		const CGolfBall::sExpected* last = GolfBall().PreData(ball.m_renderNum);
		const CGolfBall::sExpected* e =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		flat.x = last->pos.x - e->pos.x;
		flat.y = last->pos.y - e->pos.y;
		flat.z = last->pos.z - e->pos.z;
	}
	else
	{
		const WVector& hole = GOLFDOC()->GetHoleData().pin;
		const CGolfBall::sExpected* last = GolfBall().PreData(ball.m_renderNum);
		flat.x = hole.x - last->pos.x;
		flat.y = hole.y - last->pos.y;
		flat.z = hole.z - last->pos.z;
	}
	flat.y = 0.0f;
	flat.Normalize();

	switch (type)
	{
	case 0:
	{
		m_ctrl[0] = *(const WVector*)&m_matA.pivot;
		const CGolfBall::sExpected* e =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		WVector vel(e->vel.x * 0.5f, e->vel.y * 0.5f, e->vel.z * 0.5f);
		const CGolfBall::sExpected* p = GolfBall().PreData(top);
		WVector pt;
		if (p == NULL)
		{
			pt.x = vel.x;
			pt.z = vel.z;
			pt.y = vel.y + 1.0f;
		}
		else
		{
			pt.x = vel.x + p->pos.x;
			pt.z = vel.z + p->pos.z;
			pt.y = vel.y + p->pos.y;
		}
		m_ctrl[1] = pt;
		WVector up(0.0f, (m_i298 % 2) != 0 ? 1.0f : -1.0f, 0.0f);
		WVector side = WCrossProduct(up, vel);
		side.Normalize();
		vel = WVector(side.x * 30.0f, side.y * 30.0f, side.z * 30.0f);
		p = GolfBall().PreData(roll);
		WVector base;
		if (p == NULL)
		{
			base = m_ctrl[1];
		}
		else
		{
			base = p->pos;
		}
		m_ctrl[2] =
			WVector(base.x + vel.x, vel.y + base.y + 25.0f, vel.z + base.z);
		e = ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		p = GolfBall().PreData(ball.m_renderNum);
		WVector d(e->pos.x - p->pos.x, e->pos.y - p->pos.y,
			e->pos.z - p->pos.z);
		if (flat.y * 0.0f + flat.z * d.z + flat.x * d.x <= 0.0f)
		{
			WVector n = flat.Normalize();
			flat = WVector(n.x * -15.0f, n.y * -15.0f, n.z * -15.0f);
		}
		else
		{
			WVector n = flat.Normalize();
			flat = WVector(n.x * 15.0f, n.y * 15.0f, n.z * 15.0f);
		}
		flat.y = flat.y + 5.0f;
		p = GolfBall().PreData(ball.m_renderNum);
		WVector off(flat.x * 5.0f, flat.y * 5.0f, flat.z * 5.0f);
		if (p == NULL)
		{
			m_ctrl[3] = WVector(off.x + m_ctrl[2].x, off.y + m_ctrl[2].y,
				off.z + m_ctrl[2].z);
		}
		else
		{
			m_ctrl[3] =
				WVector(off.x + p->pos.x, off.y + p->pos.y, off.z + p->pos.z);
		}
		break;
	}
	case 1:
	{
		const CGolfBall::sExpected* p = GolfBall().PreData(curClamped);
		WVector v(p->vel.x * 0.8f, p->vel.y * 0.8f, p->vel.z * 0.8f);
		p = GolfBall().PreData(curClamped);
		m_ctrl[0] = WVector(v.x + p->pos.x, v.y + p->pos.y, v.z + p->pos.z);
		p = GolfBall().PreData(top);
		WVector v2(p->vel.x + p->vel.x, p->vel.y + p->vel.y,
			p->vel.z + p->vel.z);
		p = GolfBall().PreData(top);
		m_ctrl[1] =
			WVector(p->pos.x + v2.x, p->pos.y + 50.0f + v2.y, p->pos.z + v2.z);
		p = GolfBall().PreData(roll);
		WVector v3(p->vel.x, p->vel.y, p->vel.z);
		p = GolfBall().PreData(roll);
		m_ctrl[2] =
			WVector(p->pos.x + v3.x, p->pos.y + 10.0f + v3.y, p->pos.z + v3.z);
		p = GolfBall().PreData(ball.m_renderNum);
		m_ctrl[3] = WVector(p->pos.x, p->pos.y + 10.0f, p->pos.z);
		break;
	}
	case 2:
	{
		const CGolfBall::sExpected* e =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		WVector vel(e->vel.x, e->vel.y, e->vel.z);
		WVector up(0.0f, (m_i298 % 2) != 0 ? 1.0f : -1.0f, 0.0f);
		WVector side = WCrossProduct(up, vel);
		side.Normalize();
		WVector s(side.x * 25.0f, side.y * 25.0f, side.z * 25.0f);
		const CGolfBall::sExpected* p = GolfBall().PreData(curClamped);
		m_ctrl[0] = WVector(s.x + p->pos.x, s.y + p->pos.y, s.z + p->pos.z);
		p = GolfBall().PreData(top);
		WVector v1(p->vel.x * 1.2f, p->vel.y * 1.2f, p->vel.z * 1.2f);
		p = GolfBall().PreData(top);
		m_ctrl[1] = WVector(p->pos.x + v1.x + s.x,
			p->pos.y + 50.0f + v1.y + s.y, p->pos.z + v1.z + s.z);
		p = GolfBall().PreData(roll);
		WVector v2(p->vel.x * 1.5f, p->vel.y * 1.5f, p->vel.z * 1.5f);
		p = GolfBall().PreData(roll);
		m_ctrl[2] = WVector(v2.x + p->pos.x + s.x, v2.y + p->pos.y + s.y,
			v2.z + p->pos.z + s.z);
		if (ball.m_bOB == 0)
		{
			p = GolfBall().PreData(ball.m_renderNum);
			m_ctrl[3] =
				WVector(p->pos.x + s.x, p->pos.y + 10.0f + s.y, p->pos.z + s.z);
		}
		else
		{
			WVector v4(s.x * 5.0f, s.y * 5.0f, s.z * 5.0f);
			p = GolfBall().PreData(ball.m_renderNum);
			m_ctrl[3] = WVector(p->pos.x + v4.x, p->pos.y + 30.0f + v4.y,
				p->pos.z + v4.z);
		}
		break;
	}
	case 4:
	case 0xb:
	{
		const CGolfBall::sExpected* p = GolfBall().PreData(bounce);
		m_ctrl[0] = p->pos;
		int n = (int)((float)(ball.m_renderNum - bounce) * -0.5f); // 983 006cfb5e: _ftol2 trunca (sem +0.5)
		p = GolfBall().PreData(bounce - n);
		m_ctrl[1] = p->pos;
		n = (int)((float)(ball.m_renderNum - bounce) * -0.75f); // 983 006cfba0
		p = GolfBall().PreData(bounce - n);
		m_ctrl[2] = p->pos;
		n = (int)m_fa4; // 983 006cfbd2
		p = GolfBall().PreData(n);
		m_ctrl[3] = WVector(p->pos.x, p->pos.y + 3.0f, p->pos.z);
		break;
	}
	case 8:
	{
		m_ctrl[0] = *(const WVector*)&m_matA.pivot;
		const CGolfBall::sExpected* e =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		WVector vel(e->vel.x, e->vel.y, e->vel.z);
		WVector up(0.0f, (m_i298 % 2) != 0 ? 1.0f : -1.0f, 0.0f);
		WVector side = WCrossProduct(up, vel);
		side.Normalize();
		WVector s(side.x * 30.0f, side.y * 30.0f, side.z * 30.0f);
		WVector v1(vel.x * 2.5f, vel.y * 2.5f, vel.z * 2.5f);
		const CGolfBall::sExpected* p = GolfBall().PreData(top);
		m_ctrl[1] = WVector(p->pos.x + v1.x + s.x,
			p->pos.y + 25.0f + v1.y + s.y, p->pos.z + v1.z + s.z);
		p = GolfBall().PreData(top);
		WVector v2(p->vel.x * 1.5f, p->vel.y * 1.5f, p->vel.z * 1.5f);
		p = GolfBall().PreData(roll);
		m_ctrl[2] = WVector(v2.x + p->pos.x + s.x, v2.y + p->pos.y + s.y,
			v2.z + p->pos.z + s.z);
		WVector end;
		if (ball.m_bOB == 0)
		{
			int last = ball.m_renderNum;
			const CGolfBall::sExpected* e0 =
				last < 0 ? NULL : &ball.m_expected[0];
			p = GolfBall().PreData(last);
			WVector d(e0->pos.x - p->pos.x, e0->pos.y - p->pos.y,
				e0->pos.z - p->pos.z);
			float dot = d.z * flat.z;
			float dotx = d.x * flat.x;
			WVector f(flat.x * 40.0f, flat.y * 40.0f, flat.z * 40.0f);
			if (flat.y * 0.0f + dotx + dot <= 0.0f)
			{
				p = GolfBall().PreData(last);
				m_ctrl[3] = WVector(p->pos.x + f.x, p->pos.y + 10.0f + f.y,
					p->pos.z + f.z);
				break;
			}
			p = GolfBall().PreData(last);
			end = WVector(p->pos.x - f.x, (p->pos.y + 10.0f) - f.y,
				p->pos.z - f.z);
		}
		else
		{
			WVector f(flat.x * 40.0f, flat.y * 40.0f, flat.z * 40.0f);
			p = GolfBall().PreData(ball.m_renderNum);
			end =
				WVector(p->pos.x + f.x, p->pos.y + 10.0f + f.y, p->pos.z + f.z);
		}
		m_ctrl[3] = end;
		break;
	}
	case 0xc:
	{
		m_ctrl[0] = m_srcCamera;
		float sign = (m_i298 % 2) != 0 ? 1.0f : -1.0f;
		float ox = sign * 10.0f;
		float oz = sign * 10.0f;
		const CGolfBall::sExpected* p = GolfBall().PreData(top);
		m_ctrl[1] = WVector(ox + p->pos.x, p->pos.y + 100.0f, oz + p->pos.z);
		WVector up(0.0f, (m_i298 % 2) != 0 ? 1.0f : -1.0f, 0.0f);
		WVector side = WCrossProduct(up, flat);
		side.Normalize();
		WVector s(side.x * 15.0f, side.y * 15.0f, side.z * 15.0f);
		sign = (m_i298 % 2) != 0 ? 1.0f : -1.0f;
		WVector o(sign * 10.0f, s.y, sign * 10.0f);
		p = GolfBall().PreData(roll);
		m_ctrl[2] = WVector(o.x + p->pos.x + s.x, p->pos.y + 50.0f + s.y,
			o.z + p->pos.z + s.z);
		flat.x = s.x * 0.3f + flat.x * -15.0f;
		flat.z = s.z * 0.3f + flat.z * -15.0f;
		flat.y = s.y * 0.3f + flat.y * -15.0f + 3.0f;
		if (ball.m_bOB == 0)
		{
			WVector f(flat.x * 3.0f, flat.y * 3.0f, flat.z * 3.0f);
			p = GolfBall().PreData(ball.m_renderNum);
			m_ctrl[3] = WVector(f.x + p->pos.x, f.y + p->pos.y, f.z + p->pos.z);
			break;
		}
		WVector f(flat.x * 5.0f, flat.y * 5.0f, flat.z * 5.0f);
		p = GolfBall().PreData(ball.m_renderNum);
		m_ctrl[3] = WVector(f.x + p->pos.x, f.y + p->pos.y, f.z + p->pos.z);
		break;
	}
	case 0x12:
	{
		m_ctrl[0] = m_srcCamera;
		const CGolfBall::sExpected* e =
			ball.m_renderNum < 0 ? NULL : &ball.m_expected[0];
		WVector side = WCrossProduct(WVector::UNIT_POS_Y, e->vel);
		side.Normalize();
		WVector s(side.x * 100.0f, side.y * 100.0f, side.z * 100.0f);
		const CGolfBall::sExpected* p = GolfBall().PreData(top);
		m_ctrl[1] =
			WVector(p->pos.x - s.x, p->pos.y + 100.0f - s.y, p->pos.z - s.z);
		p = GolfBall().PreData(roll);
		m_ctrl[2] =
			WVector(s.x + p->pos.x, p->pos.y + 50.0f + s.y, s.z + p->pos.z);
		flat = s;
		WVector n = flat.Normalize();
		flat.x = n.x * 30.0f;
		flat.y = n.y * 30.0f;
		flat.z = n.z * 30.0f;
		flat.y = flat.y + 15.0f;
		p = GolfBall().PreData(ball.m_renderNum);
		m_ctrl[3] =
			WVector(flat.x + p->pos.x, flat.y + p->pos.y, flat.z + p->pos.z);
		break;
	}
	}

	const CGolfBall::sExpected* last = GolfBall().PreData(ball.m_renderNum);
	float dx = last->pos.x - m_ctrl[3].x;
	float dy = last->pos.y - m_ctrl[3].y;
	float dz = last->pos.z - m_ctrl[3].z;
	float f = (float)sqrt((double)(dx * dx + dy * dy + dz * dz)) * 0.1f;
	m_f18c = f;
	if (f < 1.0f)
	{
		m_f18c = 1.0f;
		return;
	}
	if (f <= 5.0f)
	{
		m_f18c = f;
		return;
	}
	m_f18c = 5.0f;
}


void CCameraWork::InitVars()
{
	m_f9c = (float)GolfBall().m_topFrame;
	m_fa0 = (float)GolfBall().m_bounceFrame;
	m_fa4 = (float)GolfBall().m_renderNum;
	m_ib4 = -2;
	m_ib8 = 0;
	m_f190 = 0;
	m_b1a2 = false;
	m_i1a4 = 0;
	m_f194 = 0;
	m_f19c = 0;
	m_f264 = -1.0f;
	m_b26c = true;
	if (m_loadCamMode983 == 5) // 983 006d0643: camera do memorial
		m_b26c = false;
	m_f18c = 5.0f;
	m_i2ac = 2;
	if (m_b26c && m_i298 % 6 == 0) // 983 006d06b7
	{
		if (GetPVS().GetObjColNum() == 0)
		{
			if (GolfClub().GetType() != 3)
			{
				int index = GolfBall().m_renderNum;
				const CGolfBall::sExpected* pre = GolfBall().PreData(index);
				if (GetPVS().GetGroundType(pre->pos, true, NULL) == 2)
				{
					m_b26d = true;
					goto done;
				}
			}
		}
	}
	m_b26d = false;
done:
	m_b1a0 = false;
	m_b1a1 = false;
	m_flags = 1;
	m_ballStart = GolfBall().m_pos;
	g_audio->StopSfx("\xb0\xf8_\xb9\xd9\xb6\xf7");
}

void CCameraWork::InputProcess(float dt)
{
	// offsets do 983 usados no Process 006dbde0 / InputProcess 006d6d00 (A455)
	C_ASSERT_VC10(offsetof(CCameraWork, m_srcCamera) == 0x3c && offsetof(CCameraWork, m_destCamera) == 0x60);
	C_ASSERT_VC10(offsetof(CCameraWork, m_matB) == 0xf0 && offsetof(CCameraWork, m_matSelect983) == 0x120);
	C_ASSERT_VC10(offsetof(CCameraWork, m_f190) == 0x1c0 && offsetof(CCameraWork, m_b1dc) == 0x20c);
	C_ASSERT_VC10(offsetof(CCameraWork, m_ballStart) == 0x24c && offsetof(CCameraWork, m_f264) == 0x298);
	C_ASSERT_VC10(offsetof(CCameraWork, m_f268) == 0x29c && offsetof(CCameraWork, m_b26c) == 0x2a0 && offsetof(CCameraWork, m_b26d) == 0x2a1);
	C_ASSERT_VC10(offsetof(CCameraWork, m_f2a0) == 0x2d4 && offsetof(CCameraWork, m_i2ac) == 0x2e0 && offsetof(CCameraWork, m_loadCamMode983) == 0x2e8);
	C_ASSERT(offsetof(CCameraWork, m_matSelect983) == VC10_CAMWORK_MATSELECT && offsetof(CCameraWork, m_i2ac) == VC10_CAMWORK_I2AC); // D873
	if (m_i2ac == 0)
	{
		g_camera = GetCameraMatrix(m_srcCamera, m_destCamera, m_f2a0);
		return;
	}
	if (m_loadCamMode983 == 5) // 983 006d6da4: camera do memorial so vai para +0x120 (a Bongdari le dali); sem FOV nem g_camera
	{
		m_matSelect983 = GetCameraMatrix(m_srcCamera, m_destCamera, m_f2a0);
		return;
	}
	if (g_input->GetDown("\xbb\xf3", true) ||
		g_input->GetDown("\xc7\xcf", true) ||
		g_input->GetDown("\xc1\xc2", true) ||
		g_input->GetDown("\xbf\xec", true))
	{
		m_b34 = true;
	}
	if (g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true))
	{
		if (g_input->Get("\xbb\xf3", true))
		{
			m_f28 = (float)(m_f28 - dt * 0.52359879016876221); // 983 006d6e7d: fmul qword 00d1a510
		}
		if (g_input->Get("\xc7\xcf", true))
		{
			m_f28 = (float)(dt * 0.52359879016876221 + m_f28); // 983 006d6ea1
		}
	}
	float rate;
	if (!g_input->Get("\xbb\xf3", true) && !g_input->Get("\xc7\xcf", true))
	{
		float a = m_f28;
		if (*(int*)&a < 0)
		{
			a = -a;
		}
		if (a >= g_EPSILON && m_i2ac == 2)
		{
			if (0.0f < m_f28)
			{
				m_f28 = (float)(m_f28 - dt * 1.0471975803375244); // 983 006d6f36
				if (m_f28 < 0.0f)
				{
					m_f28 = 0.0f;
				}
			}
			else
			{
				m_f28 = (float)(dt * 1.0471975803375244 + m_f28); // 983 006d6f14: fmul qword 00d1a508
				if (0.0f < m_f28)
				{
					m_f28 = 0.0f;
				}
			}
		}
	}
	rate = (m_i2ac == 2) ? 0.5235988f : 0.2617994f;
	if (!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true))
	{
		if (g_input->Get("\xbb\xf3", true) && (m_i2ac == 1 || !m_b2b0))
		{
			m_f30 = rate * dt + m_f30;
		}
		if (g_input->Get("\xc7\xcf", true))
		{
			m_f30 = m_f30 - rate * dt;
		}
	}
	if (!g_input->Get("\xbb\xf3", true) && !g_input->Get("\xc7\xcf", true))
	{
		float a = m_f30;
		if (*(int*)&a < 0)
		{
			a = -a;
		}
		if (a >= g_EPSILON && m_i2ac == 2)
		{
			// 983 006d705c: fadd st0,st0; fmul dt
			if (0.0f < m_f30)
			{
				m_f30 = m_f30 - (rate + rate) * dt;
				if (m_f30 < 0.0f)
				{
					m_f30 = 0.0f;
				}
			}
			else
			{
				m_f30 = (rate + rate) * dt + m_f30;
				if (0.0f < m_f30)
				{
					m_f30 = 0.0f;
				}
			}
		}
	}
	rate = (m_i2ac == 2) ? 1.5707964f : 0.5235988f;
	if (g_input->Get("\xc1\xc2", true))
	{
		m_f2c = m_f2c - rate * dt;
	}
	if (g_input->Get("\xbf\xec", true))
	{
		m_f2c = rate * dt + m_f2c;
	}
	if (!g_input->Get("\xc1\xc2", true) && !g_input->Get("\xbf\xec", true))
	{
		float a = m_f2c;
		if (*(int*)&a < 0)
		{
			a = -a;
		}
		if (g_EPSILON <= a && m_i2ac == 2)
		{
			if (0.0f < m_f2c)
			{
				m_f2c = m_f2c - (rate + rate) * dt;
				if (m_f2c < 0.0f)
				{
					m_f2c = 0.0f;
				}
			}
			else
			{
				m_f2c = (rate + rate) * dt + m_f2c;
				if (0.0f < m_f2c)
				{
					m_f2c = 0.0f;
				}
			}
		}
	}
	if (m_i2ac != 2)
	{
		float v = m_f2c;
		if (-1.0471976f <= v)
		{
			if (1.0471976f < v)
			{
				v = 1.0471976f;
			}
		}
		else
		{
			v = -1.0471976f;
		}
		m_f2c = v;
	}
	{
		float v = m_f30;
		if (-0.5235988f <= v)
		{
			if (0.5235988f < v)
			{
				v = 0.5235988f;
			}
		}
		else
		{
			v = -0.5235988f;
		}
		m_f30 = v;
	}
	if (!m_b34)
	{
		m_f28 = 0.0f;
		m_f30 = 0.0f;
		m_f2c = 0.0f;
	}
	if (m_b1dc)
	{
		float fov = (float)(m_f28 + 0.95993108116090298); // 983: fadd qword 00d1a428
		if (0.17453292f <= fov)
		{
			if (g_PI_DIV_2 < fov)
			{
				fov = 1.5707964f;
			}
		}
		else
		{
			fov = 0.17453292f;
		}
		g_view->SetFOV(fov);
		float ax = m_f2c;
		if (*(int*)&ax < 0)
		{
			ax = -ax;
		}
		if (ax < g_EPSILON)
		{
			float ay = m_f30;
			if (*(int*)&ay < 0)
			{
				ay = -ay;
			}
			if (ay < g_EPSILON)
			{
				g_camera = m_matB;
				return;
			}
		}
		float roll = m_f2a0;
		float c = (float)cos((double)m_f30);
		WVector dir;
		dir.x = (float)sin((double)m_f2c) * c;
		dir.y = (float)sin((double)m_f30);
		dir.z = (float)cos((double)m_f2c) * c;
		WVector at = RotVec(dir, m_matA);
		g_camera = GetCameraMatrix(*(const WVector*)&m_matB.xm, at, roll);
		return;
	}
	float fov = m_f268 + m_f28;
	if (0.17453292f <= fov)
	{
		if (g_PI_DIV_2 < fov)
		{
			fov = 1.5707964f;
		}
	}
	else
	{
		fov = 0.17453292f;
	}
	g_view->SetFOV(fov);
	m_matB = GetCameraMatrix(m_srcCamera, m_destCamera, m_f2a0);
	float ax = m_f2c;
	if (*(int*)&ax < 0)
	{
		ax = -ax;
	}
	float ay = m_f30;
	if (*(int*)&ay < 0)
	{
		ay = -ay;
	}
	WMatrix result;
	if (ax >= g_EPSILON || ay >= g_EPSILON)
	{
		WVector eye;
		WVector at;
		float roll;
		if (m_i2ac == 2)
		{
			const WVector& ball = *m_pLoadPos;
			WVector d;
			d.x = m_matB.xm - ball.x;
			d.y = m_matB.ym - ball.y;
			d.z = m_matB.zm - ball.z;
			float s = m_f2c;
			if (s <= 0.0f)
			{
				s = -s;
			}
			// 983 006d73ed: 1 - |ang| / 120 graus (double); o nosso multiplicava por 0.4777f
			float len = (float)((1.0 - s / 2.0943950861692429) *
				(float)sqrt((double)(d.x * d.x + d.y * d.y + d.z * d.z)));
			if (len <= 10.0f)
			{
				len = 10.0f;
			}
			d.Normalize();
			d.x = d.x * len;
			d.y = d.y * len;
			d.z = d.z * len;
			WMatrix ry = RotMat(m_f2c, 1);
			WMatrix rx = RotMat(*(const WVector*)&m_matB, m_f30);
			WVector p = RotVec(RotVec(d, rx), ry);
			eye.x = ball.x + p.x;
			eye.y = ball.y + p.y;
			eye.z = ball.z + p.z;
			WMatrix ry2 = RotMat(m_f2c, 1);
			WMatrix rx2 = RotMat(*(const WVector*)&m_matB, m_f30);
			WVector q = RotVec(RotVec(*(const WVector*)&m_matB.za, rx2), ry2);
			at.x = eye.x + q.x;
			at.y = eye.y + q.y;
			at.z = eye.z + q.z;
			roll = m_f2a0;
		}
		else
		{
			roll = m_f2a0;
			float c = (float)cos((double)m_f30);
			WVector dir;
			dir.x = (float)sin((double)m_f2c) * c;
			dir.y = (float)sin((double)m_f30);
			dir.z = (float)cos((double)m_f2c) * c;
			at = RotVec(dir, m_matB);
			eye = *(const WVector*)&m_matB.xm;
		}
		result = GetCameraMatrix(eye, at, roll);
		g_camera = result;
	}
	else
	{
		g_camera = m_matB;
	}
	if (0 < GolfBall().m_playFrame)
	{
		ProcessBallSound();
	}
}


void CCameraWork::Process(float dt, float t)
{
	// offsets do 983 usados no Process 006dbde0 (A456: inicio, ramo do puppet, modo 3 e def_cam conferidos)
	C_ASSERT_VC10(offsetof(CCameraWork, m_pfnCamera) == 0x38 && offsetof(CCameraWork, m_i8c) == 0x8c);
	C_ASSERT_VC10(offsetof(CCameraWork, m_mode) == 0x94 && offsetof(CCameraWork, m_flags) == 0x98);
	C_ASSERT_VC10(offsetof(CCameraWork, m_fa8) == 0xa8 && offsetof(CCameraWork, m_fac) == 0xac);
	C_ASSERT_VC10(offsetof(CCameraWork, m_ib4) == 0xb4 && offsetof(CCameraWork, m_ib8) == 0xb8);
	C_ASSERT_VC10(offsetof(CCameraWork, m_matA) == 0xc0 && offsetof(CCameraWork, m_v180) == 0x1b0);
	C_ASSERT_VC10(offsetof(CCameraWork, m_f198) == 0x1c8 && offsetof(CCameraWork, m_b1df) == 0x20f);
	C_ASSERT_VC10(offsetof(CCameraWork, m_loadPos) == 0x210 && offsetof(CCameraWork, m_pRankPuppet) == 0x23c);
	C_ASSERT_VC10(offsetof(CCameraWork, m_pPuppet983_240) == 0x240 && offsetof(CCameraWork, m_pPuppet) == 0x244);
	C_ASSERT_VC10(offsetof(CCameraWork, m_pMotion) == 0x248 && offsetof(CCameraWork, m_pLoadPos) == 0x2dc);
	C_ASSERT_VC10(offsetof(CCameraWork, m_b2b0) == 0x2e4 && offsetof(CCameraWork, m_b2b1) == 0x2e5);
	m_v54 = m_v48;
	m_v48 = m_srcCamera;
	float prevTime = m_f190;
	m_ib8 = GolfBall().m_playFrame; // 983 006dbe6f: +0x68 (quadro atual); o 645 lia m_renderNum (total) e a camera nunca saia do jogador
	bool bMotionCam = false;
	m_fac = (float)m_ib8;
	WVector up(0.0f, 1.0f, 0.0f);
	m_f190 = dt + m_f190;
	if (!m_b1df)
	{
		m_pMotion = NULL;
		if (m_i8c < 0xca && CheckState())
		{
			m_ballStart = GolfBall().m_pos;
			InputProcess(dt);
			return;
		}
		if (m_mode >= 0)
		{
			SetCamera(m_mode, false);
			m_mode = -1;
			InitCtrlPoints(m_i8c);
		}
		if (m_fa8 <= m_fac)
		{
			if (m_i8c == 3 || m_i8c == 4 || m_i8c == 0xb)
			{
				int cur = (int)m_fa8; // 983 006dc7a0: _ftol2 trunca (sem +0.5)
				if (m_ib4 < cur)
				{
					m_srcCamera = m_v180;
				}
			}
			if (m_pfnCamera != NULL)
			{
				(this->*(*(CameraFn*)&m_pfnCamera))(dt, t);
				CheckDriveCameraBallVisibilityAndSetBallVisibilitySafeCamera();
			}
		}
		else
		{
			m_ib4 = m_ib8;
		}
		if ((int)m_flags < 0)
		{
			if (0.8f <= m_f190 - m_f198)
			{
				m_flags &= 0x7fffffff;
			}
			else
			{
				EarthQuake();
			}
		}
	}
	else
	{
		// 983 006dbde0: FOV pelas chaves do osso da camera; os puppets do Mland e do ranking (e o modo 5) tem funcoes proprias;
		// "0xcb" do 645 virou o modo 3 do LoadCamera (killerwhale) e termina em 0xcd; 5 apets "*_def_cam" viram a camera.
		static const WMatrix s_matPitch(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.0f, -1.0f, 0.0f);
		static bool s_bFlag = false;
		if (m_pMotion == NULL)
		{
			return;
		}
		if (m_f190 <= m_pMotion->e)
		{
			ScriptProcess(m_pMotion, prevTime, m_f190);
		}
		else
		{
			ScriptProcess(m_pMotion, prevTime, m_pMotion->e);
			float over = m_f190 - m_pMotion->e;
			w_motion_data* next =
				m_pPuppet->FindMotionData(m_pMotion->nextmotion);
			m_pMotion = next;
			if (next != NULL)
			{
				float len = next->e - next->s;
				float alen = len;
				if (*(int*)&alen < 0)
				{
					alen = -alen;
				}
				if (g_EPSILON <= alen)
				{
					m_f190 = over + next->s;
				}
				else
				{
					m_f190 = next->s;
					m_f264 = next->s - 1.0f;
				}
				ScriptProcess(next, next->s, m_f190);
			}
		}
		if (m_pMotion != NULL)
		{
			WBone* camBone = m_pPuppet->FindBone(m_pMotion->applybone);
			if (camBone == NULL)
			{
				camBone = m_pPuppet->GetRootBone();
			}
			float fov = camBone->GetKeyFOV983(m_f190);
			m_f268 = (fov == -1.0f) ? 0.959931f : fov;
			if (m_loadCamMode983 != 5)
			{
				g_view->SetFOV(m_f268);
			}
			if (m_pPuppet == m_pPuppet983_240)
			{
				MlandPreviewCamera983();
			}
			else if (m_pPuppet == m_pRankPuppet)
			{
				RankCamera983();
			}
			else if (m_loadCamMode983 == 5)
			{
				RankCamera983();
			}
			else
			{
			m_pPuppet->Animate983(NULL, NULL, m_f190, 0, NULL, NULL);
			m_pPuppet->Transform(NULL, 1.0f, 0);
			WBone* bone = m_pPuppet->FindBone(m_pMotion->applybone);
			WVector fwd(0.0f, 0.0f, 1.0f);
			if (bone != NULL || (bone = m_pPuppet->GetRootBone()) != NULL)
			{
				float angle = 0.0f;
				WVector off;
				CTask* task = (CTaskManager::Instance() == NULL)
					? NULL
					: CTaskManager::Instance()->GetCurrentTask();
				IActor* player = task->GetActor("Player");
				if (player != NULL)
				{
					player << MsgObject(NULL, M983(0x110), (int)&angle, 0, 0, 0, 0);
				}
				task = (CTaskManager::Instance() == NULL)
					? NULL
					: CTaskManager::Instance()->GetCurrentTask();
				player = task->GetActor("Player");
				if (player != NULL)
				{
					player << MsgObject(NULL, M983(0x128), (int)&off, 0, 0, 0, 0);
				}
				WMatrix mat;
				mat = bone->m_matrix;
				if ((!WisZero(off.x, g_EPSILON) || !WisZero(off.y, g_EPSILON) ||
						!WisZero(off.z, g_EPSILON)) &&
					!m_b2b1)
				{
					WMatrix rm;
					RotMat(angle, 1, rm);
					mat *= rm;
				}
				WMatrix rot;
				mat.GetRotMatrix(&rot);
				rot = s_matPitch * rot;
				WVector pos(mat.pivot.x, mat.pivot.y, mat.pivot.z);
				if (m_loadCamMode983 == 3)
				{
					WMatrix r(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
						1.0f);
					r.Rotate(angle + g_PI, 1);
					rot = rot * r;
					pos = pos * r;
					if (s_bFlag)
					{
						player = (CTaskManager::Instance() == NULL)
							? NULL
							: CTaskManager::Instance()
								  ->GetCurrentTask()
								  ->GetActor("Player");
						if (player != NULL)
						{
							player << MsgObject(NULL, M983(0x127), (int)&m_loadPos, 1,
								0, 0, 0);
						}
					}
					if (!s_bFlag)
					{
						s_bFlag = true;
					}
				}
				else
				{
					const char* petName = m_pPuppet->GetPuppetName();
					if ((_stricmp(petName, "i_def_cam.apet") == 0 ||
							_stricmp(petName, "k_def_cam.apet") == 0 ||
							_stricmp(petName, "FF_Hana_Def_cam.apet") == 0 ||
							_stricmp(petName, "mm_Def_Cam.apet") == 0 ||
							_stricmp(petName, "cc_Def_Cam.apet") == 0) &&
						strstr(m_pMotion->name,
							"\xbf\xec\xb5\xe5\xc6\xc4\xbf\xf6\xbc\xa6\x32") != NULL)
					{
						static const WMatrix s_matFlip(-1.0f, 0.0f, 0.0f, 0.0f,
							1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
						rot = rot * s_matFlip;
						pos = pos * s_matFlip;
					}
				}
				WVector d = fwd * rot;
				// 983: no modo 0 do LoadCamera a posicao do jogador (0x128) nao e descontada
				pos.x = pos.x + m_loadPos.x;
				if (m_loadCamMode983 == 0)
				{
					pos.y = pos.y + m_loadPos.y;
					pos.z = pos.z + m_loadPos.z;
				}
				else
				{
					pos.x = pos.x - off.x;
					pos.y = pos.y + m_loadPos.y - off.y;
					pos.z = pos.z + m_loadPos.z - off.z;
				}
				m_srcCamera = pos;
				WVector dest(d.x + m_srcCamera.x, d.y + m_srcCamera.y,
					d.z + m_srcCamera.z);
				bMotionCam = true;
				m_destCamera = dest;
				up.x = rot.xy;
				up.y = rot.yy;
				up.z = rot.zy;
			}
			}
		}
		if (m_loadCamMode983 == 3 && 0.0f < m_f264 && m_f264 < m_f190)
		{
			s_bFlag = false;
			m_b1df = false;
			// 983 006dc684 pushes 0xcd = its ToDefaultCamera; ours is 0xcc (0xcd = ZoomOutCamera, the zoom of the hole opening)
			SetCamera(0xcc, false);
		}
	}
	if (m_b26c)
	{
		CheckGround(m_srcCamera, m_destCamera);
	}
	CGolfBall& b = GolfBall();
	float dx = m_srcCamera.x - b.m_pos.x;
	float dy = m_srcCamera.y - b.m_pos.y;
	float dz = m_srcCamera.z - b.m_pos.z;
	if ((m_b26d ||
			192.0f < (float)sqrt((double)(dz * dz + dy * dy + dx * dx))) &&
		m_ib8 > 0)
	{
		const CGolfBall::sExpected* e = GolfBall().PreData(m_ib8);
		if ((int)e->state == 3 && m_i8c != 0x19)
		{
			float f = m_f268 - dt * 0.17453292f;
			if (f <= 0.34906584f)
			{
				f = 0.34906584f;
			}
			m_f268 = f;
			m_destCamera.y = (float)(m_destCamera.y - (1.0 - f / 0.95993108116090298) * 5.0); // 983 006dc9b1: fdiv 55 graus (double)
		}
	}
	m_ballStart = GolfBall().m_pos;
	InputProcess(dt);
	if (bMotionCam)
	{
		g_camera.xy = up.x;
		g_camera.yy = up.y;
		g_camera.zy = up.z;
		g_camera.xx = g_camera.zz * up.y - up.z * g_camera.yz;
		g_camera.yx = up.z * g_camera.xz - g_camera.zz * up.x;
		g_camera.zx = g_camera.yz * up.x - up.y * g_camera.xz;
	}
}

// 983 006ce360 (nome nosso): camera do puppet de ranking e do modo 5 (memorial_cam.apet): a camera fica no osso do movimento
// (ou na raiz), virada pela mesma matriz de inclinacao do Process, a partir de m_loadPos.
void CCameraWork::RankCamera983()
{
	m_pPuppet->Animate983(NULL, NULL, m_f190, 0, NULL, NULL);
	m_pPuppet->Transform(NULL, 1.0f, 0);
	WBone* bone = m_pPuppet->FindBone(m_pMotion->applybone);
	if (bone != NULL || (bone = m_pPuppet->GetRootBone()) != NULL)
	{
		static const WMatrix s_matPitch(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.0f, -1.0f, 0.0f);
		WMatrix mat;
		mat = bone->m_matrix;
		WMatrix rot;
		mat.GetRotMatrix(&rot);
		rot = s_matPitch * rot;
		WVector d = WVector(0.0f, 0.0f, 1.0f) * rot;
		m_srcCamera = mat.pivot + m_loadPos;
		m_destCamera = m_srcCamera + d;
	}
}

// 983 006d0880 (nome nosso): previa do buraco no Mland (mland_gamepreview.apet, LoadCamera modo 4). O puppet e posto no pino,
// virado para o tee; no ultimo segundo do movimento a camera vai por Slerp ate a camera padrao do buraco (MakeDefaultCamera).
void CCameraWork::MlandPreviewCamera983()
{
	// 983 006d0880: angulo pelo tee - pino (dois GetHoleData), transicao
	// pela camera padrao com WVector e WQuat como o 983
	m_pPuppet->Animate983(NULL, NULL, m_f190, 0, NULL, NULL);
	m_pPuppet->Transform(NULL, 1.0f, 0);
	WVector dir = GOLFDOC()->GetHoleData().tee - GOLFDOC()->GetHoleData().pin;
	float angle = (float)(atan2(dir.z, dir.x) + 1.5707963705062866);
	WMatrix mat;
	RotMat(angle, 1, mat);
	mat.pivot = GOLFDOC()->GetHoleData().pin;
	WBone* root = m_pPuppet->GetRootBone();
	WMatrix pitch;
	RotMat(-1.5707964f, 0, pitch);
	mat = pitch * root->m_matrix * mat;
	WVector pos = mat.pivot;
	if (m_pMotion->e - m_f190 < 1.0)
	{
		GOLFDOC()->MakeDefaultCamera(false);
		const WMatrix& cam = GOLFDOC()->GetCameraData().mat;
		WVector from = mat.pivot;
		float rate = (0.0f < m_pMotion->e - m_f190) ? m_pMotion->e - m_f190 : 0.0f;
		rate = sin(rate * 1.5707964f);
		WQuat q0;
		q0 = cam;
		WQuat q1;
		q1 = mat;
		WQuat q;
		q = WQuaternionSlerp(q0, q1, rate);
		mat = q;
		pos = from * rate + cam.pivot * (1.0f - rate);
	}
	m_srcCamera = pos;
	m_destCamera = m_srcCamera + mat.za;
	if (m_b26c)
	{
		m_b26c = false;
	}
}

bool CCameraWork::CheckGround(WVector& pos, WVector& dest)
{
	float h = GetPVS().GetBasePosY(pos, GOLFDOC()->m_cameraData.clearance);
	float y = pos.y;
	if (h <= y)
	{
		h = y;
	}
	if (m_b1df)
	{
		dest.y = (h - pos.y) + dest.y;
	}
	if (pos.y < h)
	{
		pos.y = h;
		return true;
	}
	return false;
}

void CCameraWork::ScriptProcess(w_motion_data* motion, float from, float to)
{
	WPuppet* puppet = m_pPuppet;
	if (puppet == NULL)
		return;
	if (!(from <= motion->e))
		return;
	if (from == to)
		return;

	from *= 30.0;
	float e = motion->e;
	to = (e <= to) ? e : to;
	to *= 30.0;
	int first = Ceil(from, 0.03f);
	from = (float)floor((double)to);
	int last = (from == to) ? (int)to - 1 : (int)from;
	if (first > last)
		return;
	int* pFrom = (int*)&from;
	int* pTo = (int*)&to;
	if (!puppet->CheckFrameData(first, last, *pFrom, *pTo))
		return;
	for (int i = *pFrom; i <= *pTo; i++)
		ParsePuppetScript(m_pPuppet->m_frame[i].data);
}

void CCameraWork::ParsePuppetScript(char* script)
{
	if (script == NULL)
		return;

	int len;
	char* cmd;
	for (cmd = strchr(script, '*'); cmd != NULL && (len = 0, *cmd != '\0');
		cmd = strchr(cmd + len, '*'))
	{
		char c;
		do
		{
			c = cmd[len + 1];
			++len;
			if (c == ' ')
				break;
			if (c == '(')
			{
				++len;
				break;
			}
		} while (c != '\0');
		if (len <= 1)
			return;

		strncpy(s_scriptCmd, cmd, len);
		s_scriptCmd[len] = '\0';
		_strcmpi(s_scriptCmd, s_scriptKey);
	}
}

bool CCameraWork::CheckState()
{
	// 983 006d0ba0: sai cedo com true; pino por referencia; distancia
	// horizontal por WVector(d.x, 0, d.z).Magnitude() contra raio * 3.0
	if (m_fac >= m_fa4)
	{
		return true;
	}
	if (GolfBall().IsHoleIn())
	{
		const WVector& pin = GOLFDOC()->GetHoleData().pin;
		WVector d = GolfBall().m_pos - pin;
		if (!m_b1a1)
		{
			CGolfBall& b = GolfBall();
			if (WVector(d.x, 0.0f, d.z).Magnitude() < b.m_radius * 3.0 &&
				d.y < 0.0f)
			{
				m_b1a1 = true;
			}
			if (!m_b1a1)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void CCameraWork::ProcessBallSound()
{
	if (!(m_f190 - m_f19c < 0.1f) || m_i1a4 == 1)
	{
		m_f19c = m_f190;
		WVector d;
		d = GolfBall().m_pos - m_srcCamera;
		float dist = d.Magnitude();
		if (m_i1a4 == 0)
		{
			CGolfBall& b = GolfBall();
			if (80.0f < b.m_vel.Magnitude() &&
				dist < 100.0f)
			{
				m_i1a4 = 1;
				g_audio->PlaySfx("\xb0\xf8_\xb9\xd9\xb6\xf7", &d, 5,
					"\xb0\xf8_\xb9\xd9\xb6\xf7", NULL, 0.0f, 100.0f);
				return;
			}
		}
		else if (m_i1a4 & 1)
		{
			CGolfBall& b = GolfBall();
			if (80.0f < b.m_vel.Magnitude())
			{
				if (dist < 100.0f)
				{
					if (!g_audio->IsPlaying("\xb0\xf8_\xb9\xd9\xb6\xf7"))
					{
						g_audio->PlaySfx("\xb0\xf8_\xb9\xd9\xb6\xf7", &d, 5,
							"\xb0\xf8_\xb9\xd9\xb6\xf7", NULL, 0.0f, 100.0f);
					}
					g_audio->SetSFXVolumeWhilePlaying(
						"\xb0\xf8_\xb9\xd9\xb6\xf7", (float)((100.0 - dist) / 100.0));
					return;
				}
				if (m_i1a4 == 1)
				{
					g_audio->StopSfx("\xb0\xf8_\xb9\xd9\xb6\xf7");
					m_i1a4 = 3;
					return;
				}
			}
			else
			{
				g_audio->StopSfx("\xb0\xf8_\xb9\xd9\xb6\xf7");
				m_i1a4 = 2;
			}
		}
	}
}

void CCameraWork::StopBallSound()
{
	m_i1a4 = 2;
	g_audio->StopSfx("\xb0\xf8_\xb9\xd9\xb6\xf7");
}

void CCameraWork::InterpolateBezier(WVector* out, float t, WVector* ctrl,
	int count)
{
	// 983 006ce6f0: soma de vetores (WVector * float), da direita p/ a esquerda
	float t2 = t * t;
	float u = 1.0f - t;
	float u2 = u * u;
	if (count == 3)
		*out = ctrl[0] * u2 + ctrl[1] * (2 * u * t) + ctrl[2] * t2;
	else if (count == 4)
	{
		float t3 = t2 * t;
		float u3 = u2 * u;
		*out = ctrl[0] * u3 + ctrl[1] * (3 * u2 * t) + ctrl[2] * (3 * u * t2) +
			ctrl[3] * t3;
	}
}

void CCameraWork::CheckDriveCameraBallVisibilityAndSetBallVisibilitySafeCamera()
{
	if (m_i8c < 100 && driveTable[m_i8c].use)
	{
		// 983 006db290: caixa do modelo raiz = sModel::aabb (+0x98); a visao do 645 lia +0x60 (a matriz)
		const Waabb& box = GOLFDOC()->m_pPolySoup->FindModel(NULL)->aabb;
		float limit = (box.max - box.min).Magnitude() * 2;
		WVector toBall = GolfBall().m_pos - m_srcCamera;
		if (toBall.Magnitude() <= limit)
		{
			if (((CQuadTreeEx&)GetPVS())
					.RayIntersectAll(m_srcCamera, toBall, true) == 1.0f)
			{
				m_ibc = -1;
				return;
			}
		}
		if (m_ibc < 0)
		{
			m_ibc = m_ib8;
			return;
		}
		if (m_ib8 - m_ibc > 50)
		{
			UninstallCamera();
			unsigned int count = m_camCandidates.size();
			unsigned int pick = rand() % count;
			int* p = &m_camCandidates[0] + pick;
			if (*p == m_i8c)
			{
				pick = (pick + 1) % m_camCandidates.size();
				p = &m_camCandidates[0] + pick;
				if (*p == m_i8c)
				{
					return;
				}
			}
			SetCamera(*p, false);
		}
	}
}

void CCameraWork::EarthQuake()
{
	if (!(m_f190 - m_f194 < 0.1f))
	{
		m_f194 = m_f190;
		int range = 6 - (int)((m_f190 - m_f198) * 6.0f);
		if (range == 0)
			range = 1;
		float half = range / 20.0;
		WVector v;
		v.x = (rand() % range) / 10.0 - half;
		v.y = (rand() % range) / 10.0 - half;
		v.z = (rand() % range) / 10.0 - half;
		WVector d = v * 0.1f;
		m_srcCamera.x += d.x;
		m_srcCamera.y += d.y;
		m_srcCamera.z += d.z;
	}
}

void CCameraWork::FollowCamera(float dt, float t)
{
	InterpolateBezier(&m_srcCamera, (m_fac + t - m_fa8) / (m_fa4 - m_fa8),
		m_ctrl, 4);
	if (m_fac < m_fa0)
	{
		m_destCamera = GolfBall().m_pos;
		return;
	}
	if (m_fac < m_fa4)
	{
		// JP 983 (006d1070): k guardado, altura h = m_f18c * k, destino = bola + (0, t, 0)
		float k = (m_fac + t - m_fa0) / (m_fa4 - m_fa0);
		float h = m_f18c * k;
		m_destCamera = GolfBall().m_pos + WVector(0, h, 0);
	}
}

void CCameraWork::LookBackCamera(float dt, float t)
{
	InterpolateBezier(&m_srcCamera, (t + m_fac - m_fa8) / (m_fa4 - m_fa8),
		m_ctrl, 4);
	if (m_flags & 0x20000000)
	{
		if (m_fac < (float)m_ib0 - 40.0f)
		{
			m_destCamera = GolfBall().m_pos;
			return;
		}
		m_mode = 14;
		return;
	}
	if (m_fac < m_fa0 * 0.65f)
	{
		WVector d = m_srcCamera - m_destCamera;
		if (10.0f < (float)sqrt((double)(d.x * d.x + d.y * d.y + d.z * d.z)))
		{
			m_destCamera = GolfBall().m_pos;
			return;
		}
	}
	m_mode = 2;
}

void CCameraWork::Rail_or_SideCamera(float dt, float t)
{
	// 983 006d13b0
	float r = ((t + m_fac) - m_fa8) / (m_fa4 - m_fa8);
	InterpolateBezier(&m_srcCamera, r, m_ctrl, 4);
	float a = r * g_PI;
	m_f268 = (float)(0.959931081160903 - // 983: constante double
		((float)sin((double)a) * 25.0f) * g_DEGTORAD);
	if (m_fac < m_fa0)
	{
		m_destCamera = GolfBall().m_pos;
	}
	else if (m_fac < m_fa4)
	{
		a = ((t + m_fac) - m_fa0) / (m_fa4 - m_fa0);
		m_destCamera = GolfBall().m_pos + WVector(0.0f, a * m_f18c, 0.0f);
	}
}

void CCameraWork::PanCamera(float dt, float t)
{
	// 983 006d1570
	unsigned int isFar = m_flags & 0x20000000;
	if (isFar != 0 ? m_fac <= (float)m_ib0 - 40.0 : m_fac <= m_fa0 * 0.7f) // 983 006d15bb: fcompp, igual tambem entra
	{
		int back;
		if ((m_flags & 1) != 0)
		{
			m_destCamera = GolfBall().m_pos;
			back = 0;
		}
		else
		{
			back = -10;
			WVector a = GolfBall().PreData(m_ib8 - 9)->pos * t;
			m_destCamera = GolfBall().PreData(m_ib8 - 10)->pos * (1.0f - t) + a;
		}
		t = ((m_fac + t) - m_fa8) / (m_fa0 * 0.7f - m_fa8);
		t = t * g_PI * 0.5f;
		t = (float)sin((double)t);
		m_f268 = (float)(0.959931081160903 - t * 25.0f * g_DEGTORAD);
		if (GolfClub().GetType() != 0)
		{
			return;
		}
		if (m_b1a2)
		{
			return;
		}
		if (!(m_f9c - m_fac < 15.0f - (float)back))
		{
			return;
		}
		WVector d;
		WVector p = GolfBall().PreData((int)m_fac)->pos;
		const WVector& q = GolfBall().PreData((int)m_fac + 1)->pos;
		d.x = q.x - p.x;
		d.z = q.z - p.z;
		p.x = m_srcCamera.x - p.x;
		p.z = m_srcCamera.z - p.z;
		// y de WCrossProduct((d.x, 0, d.z), (p.x, 0, p.z))
		t = d.z * p.x - p.z * d.x;
		if (t > 0.0f)
		{
			g_audio->PlaySfx("pass_LR");
			m_b1a2 = true;
			return;
		}
		g_audio->PlaySfx("pass_RL");
		m_b1a2 = true;
		return;
	}
	m_f268 = 0.95993108f;
	m_mode = isFar != 0 ? 14 : 0;
}

void CCameraWork::DropCamera(float dt, float t)
{
	if (m_fac < m_fa0)
	{
		m_destCamera = GolfBall().m_pos;
		return;
	}
	if (m_fac < m_fa4)
	{
		float r = ((t + m_fac) - m_fa0) / (m_fa4 - m_fa0);
		InterpolateBezier(&m_destCamera, r, m_ctrl, 4);
	}
}

void CCameraWork::Chase_and_FollowCamera(float dt, float t)
{
	// 983 (006d1a10): quadro com dt == 0 nao mexe na camera
	if (dt == 0.0f)
		return;
	m_destCamera = m_destCamera * 0.9f + GolfBall().m_pos * 0.1f;
	float speed = GolfBall().m_vel.Magnitude();

	int i = m_ib8;
	WVector anchor;
	WVector dir;
	for (;;)
	{
		int lo = m_ib8 - 37;
		if (lo <= 2)
		{
			lo = 2;
		}
		if (i >= lo)
		{
			WVector v1 = GolfBall().PreData(i - 1)->pos - GolfBall().PreData(i - 2)->pos;
			WVector v2 = GolfBall().PreData(i)->pos - GolfBall().PreData(i - 1)->pos;
			v1.y = 0.0f;
			v2.y = 0.0f;
			v1.Normalize();
			v2.Normalize();
			float c45 = (float)cos(0.7853981852531433);
			float dot = v2 * v1;
			if (dot < c45)
			{
				float limit = (float)cos(1.3962633609771729);
				anchor = GolfBall().PreData(i - 1)->pos;
				anchor.y = GolfBall().m_pos.y;
				dir = v2 - v1;
				if (dot < limit)
				{
					WMatrix rotA = RotMat(-0.08726646f, 1);
					WMatrix rotB = RotMat(0.08726646f, 1);
					WMatrix& rot = (v1.z * v2.x - v2.z * v1.x <= 0.0f) ? rotB : rotA;
					dir = RotVec(dir, rot);
				}
				break;
			}
			--i;
			continue;
		}
		anchor = GolfBall().m_pos;
		dir = GolfBall().PreData(m_ib8 - 1)->pos - GolfBall().PreData(m_ib8)->pos;
		break;
	}
	WVector n(dir.x, 0.0f, dir.z);
	n.Normalize();
	n.y = 0.5f;
	n.Normalize();
	float k = speed * 0.4f;
	float d = 40.0f;
	if (k >= 40.0f)
	{
		d = k > 80.0f ? 80.0f : k;
	}
	WVector target = n * d + anchor;

	WVector c = m_srcCamera - m_destCamera;
	float len = c.Magnitude();
	if (len < (target - m_destCamera).Magnitude())
	{
		n = WVector(c.x, 0.0f, c.z);
		n.Normalize();
		n.y = 0.5f;
		n.Normalize();
		target = n * len + m_destCamera;
	}
	k = speed * 0.001f;
	float s = g_EPSILON;
	if (k >= g_EPSILON)
	{
		s = k > 0.1f ? 0.1f : k;
	}
	m_srcCamera = m_srcCamera * (1.0f - s) + target * s;
	InterpolateBezier(&m_srcCamera, 0.5f, &m_srcCamera, 3);
}


void CCameraWork::CircleCamera(float dt, float t)
{
	if (m_ib8 < GolfBall().m_spikeDiveFrame)
	{
		if (m_f2a0 > 0.0f)
		{
			m_f2a0 = m_f2a0 - dt * 1.0471975430846214;
			if (0.0f > m_f2a0)
			{
				m_f2a0 = 0;
			}
		}
		WVector dir = GolfBall().m_pos - m_srcCamera;
		dir.y = 0.0f;
		m_f25c = dir.Magnitude();
		dir.Normalize();
		WVector side = WCrossProduct(WVector::UNIT_POS_Y, dir);
		float k = m_f25c * 3.1415926292538643; // 983: PI e PI/3 em double, nao g_PI
		m_srcCamera += side * k * dt;
		return;
	}
	m_srcCamera += GolfBall().m_vel * 0.75f * dt;
	m_destCamera += GolfBall().m_vel * 0.75f * dt;
	m_destCamera.y = m_destCamera.y - dt;
}


void CCameraWork::Chase_and_LookBackCamera(float dt, float t)
{
	// 983 006d2730
	float r = ((t + m_fac) - m_fa8) / (m_fa4 - m_fa8);
	InterpolateBezier(&m_srcCamera, r, m_ctrl, 4);
	float a = r * g_PI;
	m_f268 = (float)(0.959931081160903 - // 983: constante double
		((float)sin((double)a) * 25.0f) * g_DEGTORAD);
	if (m_flags & 0x20000000)
	{
		if (m_fac < (float)m_ib0 - 50.0)
		{
			m_destCamera = GolfBall().m_pos;
		}
		else
		{
			m_mode = 14;
		}
	}
	else
	{
		if (m_fac < m_fa0)
		{
			m_destCamera = GolfBall().m_pos;
		}
		else if (m_fac < m_fa4)
		{
			float k = ((t + m_fac) - m_fa0) / (m_fa4 - m_fa0);
			m_destCamera = GolfBall().m_pos + WVector(0.0f, k * m_f18c, 0.0f);
		}
	}
}

void CCameraWork::ZoomInCamera(float dt, float t)
{
	if (m_fac < m_f9c)
	{
		float r = (t + m_fac) / m_f9c;
		m_f268 = (float)(0.959931081160903 - // 983 006d2930: constantes double
			r * 35.00000021595408 * 0.01745329238474369);
		m_destCamera = GolfBall().m_pos;
		r = 1.0f - r;
		m_destCamera.y = r + r + m_destCamera.y;
		return;
	}
	m_f268 = 0.95993108f;
	m_mode = (m_flags & 0x20000000) ? 14 : 11;
}

void CCameraWork::CaddyViewCamera(float dt, float t)
{
	WPuppet* puppet = SCENEMGR()->GetPuppet("\xc4\xb3\xb5\xf0");
	if (puppet == NULL)
	{
		return;
	}
	// 983 006d2a40: em WVector; 0.0/5.0/10.0/35.0 em double
	WVector d = puppet->m_mat.pivot - GolfBall().m_pos;
	WVector dir;
	dir.x = d.x;
	dir.z = d.z;
	float y = d.y + 10.0;
	dir.y = (0.0 > y) ? y : 0.0f;
	dir.Normalize();
	WBone* head = puppet->FindBoneByTail("Head");
	WVector pos;
	if (head != NULL)
	{
		pos = head->GetMatrix().pivot + WVector(0.0f, 5.0, 0.0f);
	}
	else
	{
		pos = puppet->m_mat.pivot + WVector(0.0f, 10.0, 0.0f);
	}
	m_srcCamera = pos;
	m_srcCamera += WCrossProduct(WVector::UNIT_POS_Y, dir) * 10.0 + dir * 35.0;
	m_destCamera = GolfBall().m_pos;
}


void CCameraWork::Drop_and_ZoomCamera(float dt, float t)
{
	if (m_fac < m_fa0)
	{
		m_f268 = (float)(0.26179938577115536 - // 983 006d2ce0: 15/-40 graus double
			(((t + m_fac) - m_fa8) / (m_fa0 - m_fa8)) * -0.6981316953897476);
		m_destCamera = GolfBall().m_pos;
		return;
	}
	if (m_fac < m_fa4)
	{
		InterpolateBezier(&m_destCamera,
			((t + m_fac) - m_fa0) / (m_fa4 - m_fa0), m_ctrl, 4);
	}
}

void CCameraWork::BezierCamera(float dt, float t)
{
	// 983 006d2e20
	float r = ((t + m_fac) - m_fa8) / (m_fa4 - m_fa8);
	InterpolateBezier(&m_srcCamera, r, m_ctrl, 4);
	if (m_fac < m_fa0)
	{
		m_destCamera = GolfBall().m_pos;
		m_destCamera.y += (1.0f - r) + (1.0f - r);
		return;
	}
	if (m_fac < m_fa4)
	{
		r = ((t + m_fac) - m_fa0) / (m_fa4 - m_fa0);
		m_destCamera = GolfBall().m_pos + WVector(0.0f, m_f18c * r, 0.0f);
	}
}

void CCameraWork::FollowWithSwingCamera(float dt, float t)
{
	InterpolateBezier(&m_srcCamera, ((m_fac + t) - m_fa8) / (m_fa4 - m_fa8),
		m_ctrl, 4);
	if (m_fac < m_fa0)
	{
		m_f1a8 = m_f1a8 - dt * 0.034906585f;
	}
	WMatrix rot = RotMat(m_f1a8, 0);
	WVector d = GolfBall().m_pos - m_srcCamera;
	m_destCamera = d * rot + m_srcCamera;
}

void CCameraWork::SidePanWithZoomInCamera(float dt, float t)
{
	// 983 006d3160: dir = vel com y = 0, side = cross().Normalize(); -75 graus e PI/3 em double
	WVector dir = GolfBall().m_vel;
	dir.y = 0.0f;
	dir.Normalize();
	WVector side = WCrossProduct(dir, WVector::UNIT_POS_Y).Normalize();
	if (m_f190 < 2.0f)
	{
		WVector off = side * 3.0f;
		WVector back = dir * ((1.0f - m_f190 * 3.0f) * 2.0f);
		m_srcCamera = back + GolfBall().m_pos + off;
		if (m_f190 < 0.5f)
		{
			m_destCamera = m_srcCamera - side;
			return;
		}
		if (m_f1a8 > -1.3089969288557768)
		{
			m_f1a8 = m_f1a8 - dt * 1.0471975430846214;
		}
		WMatrix rot;
		RotMat(m_f1a8, 1, rot);
		m_destCamera = m_srcCamera - side * rot;
		return;
	}
	m_mode = 4;
}

void CCameraWork::FromMidPointCamera(float dt, float t)
{
	// 983 006d3490
	if (m_flags & 0x20000000)
	{
		if (m_fac < (float)m_ib0 - 40.0)
		{
			m_destCamera = GolfBall().m_pos;
		}
		else
		{
			m_mode = 14;
		}
	}
	else
	{
		if (m_fac < m_fa0)
		{
			m_destCamera = GolfBall().m_pos;
		}
		else if (m_fa0 > 250.0)
		{
			m_mode = 4;
		}
		else
		{
			float r = ((t + m_fac) - m_fa0) / ((m_fa4 - m_fa0) + 1.0);
			m_destCamera = GolfBall().m_pos + WVector(0.0f, r * 5.0, 0.0f);
		}
	}
}


void CCameraWork::FromColPointCamera(float dt, float t)
{
	// 983 006d3650
	if (m_fac < m_fa0)
	{
		m_destCamera = GolfBall().m_pos;
	}
	else
	{
		float r = ((t + m_fac) - m_fa0) / (m_fa4 - m_fa0);
		m_destCamera = GolfBall().m_pos + WVector(0.0f, r * 5.0, 0.0f);
	}
}

void CCameraWork::PangyaACamera(float dt, float t)
{
	if (m_f190 > 2.0f)
		SetCamera(0x15, true);
}

void CCameraWork::FixedAtSideCamera(float dt, float t)
{
	m_destCamera = GolfBall().m_pos;
}

void CCameraWork::FromRandomPointCamera(float dt, float t)
{
	// 983 006d3810
	float r = ((t + m_fac) - m_fa8) / (m_fa4 - m_fa8);
	WVector a = GolfBall().m_pos * r;
	CGolfBall& b = GolfBall();
	const CGolfBall::sExpected* start =
		(b.m_renderNum < 0) ? NULL : &b.m_expected[0];
	m_destCamera = start->pos * (1.0f - r) + a + WVector(0.0f, 3.0f, 0.0f);
}

void CCameraWork::BankFromHeadCamera(float dt, float t)
{
	// 983 (006d3980): r limitado a 0.999, angulo e contas em double
	float r = ((t + m_fac) - m_fa8) / (m_f9c - m_fa8);
	r = min(r, 0.999f);
	m_f2a0 = (r * 60.0 - 30.0) * 0.017453292f;
	WVector dir = GolfBall().m_vel;
	dir.Normalize();
	WVector side = WCrossProduct(WVector::UNIT_POS_Y, dir);
	side.Normalize();
	m_destCamera = GolfBall().m_pos + side * (float)(r * 0.5 - 1.0);
	m_srcCamera = m_destCamera + dir * (float)((1 - r) * 50.0);
	if (m_fac > m_f9c)
	{
		m_mode = 0;
	}
}

void CCameraWork::CobraACamera(float dt, float t)
{
	if (m_ib8 < GolfBall().m_cobraFrame)
	{
		float r = (t + m_fac) / (float)GolfBall().m_cobraFrame;
		CGolfBall& b0 = GolfBall();
		const WVector* start = b0.m_renderNum < 0 ? 0 : &b0.m_expected[0].pos;
		const CGolfBall::sExpected* end =
			GolfBall().PreData(GolfBall().m_cobraFrame);
		WVector side = WCrossProduct(WVector::UNIT_POS_Y, end->pos - *start);
		side.Normalize();
		side.x = side.x * 5.0f;
		side.y = side.y * 5.0f;
		side.z = side.z * 5.0f;
		CGolfBall& b1 = GolfBall();
		start = b1.m_renderNum < 0 ? 0 : &b1.m_expected[0].pos;
		end = GolfBall().PreData(GolfBall().m_cobraFrame);
		WVector d = (end->pos - *start) * (r * 0.3f + 0.4f);
		CGolfBall& b2 = GolfBall();
		start = b2.m_renderNum < 0 ? 0 : &b2.m_expected[0].pos;
		m_srcCamera = *start + d + side;
		WVector up(0.0f, 1.0f - r, 0.0f);
		m_destCamera = up + GolfBall().m_pos;
	}
	else
	{
		// 983 006d407a: mode 4 only once the ball passed the cobra frame (the 645 set it every frame)
		m_mode = 4;
	}
}

void CCameraWork::CobraBCamera(float dt, float t)
{
	// 983 006d40a0: each branch ends the frame -- before the top the target rises 10/s
	// (only after cobra+50; until then just the eye rises 6/s), after it falls 20/s above the ball, else 5/s
	if (m_ib8 < GolfBall().m_cobraFrame)
	{
		float r = (m_fac + t - m_fa8) / ((float)GolfBall().m_cobraFrame - m_fa8);
		m_f2a0 = 0.2617994f;
		WVector next = GolfBall().PreData((GolfBall().m_cobraFrame + m_ib8 * 4 + 4) / 5)->pos * t;
		m_srcCamera = GolfBall().PreData((GolfBall().m_cobraFrame + m_ib8 * 4) / 5)->pos * (1.0f - t) + next;
		m_srcCamera.y += (r + r) - 1.0f;
		m_destCamera = GolfBall().m_pos;
		return;
	}
	if (m_ib8 < GolfBall().m_bounceFrame)
	{
		m_destCamera.x = GolfBall().m_pos.x;
		m_destCamera.z = GolfBall().m_pos.z;
		if (m_ib8 < GolfBall().m_topFrame)
		{
			m_f2a0 = m_f2a0 - dt * (5.0 * 0.017453292f);
			if (m_ib8 < GolfBall().m_cobraFrame + 50)
			{
				m_srcCamera.y = dt * 6.0 + m_srcCamera.y;
				return;
			}
			m_destCamera.y = dt * 10.0 + m_destCamera.y;
		}
		else
			m_destCamera.y = m_destCamera.y - (m_destCamera.y > GolfBall().m_pos.y ? dt * 20.0 : dt * 5.0);
	}
}

void CCameraWork::TomahawkACamera(float dt, float t)
{
	if (m_f190 < 2.0f)
	{
		GolfBall();
		WVector dir = GolfBall().m_pos - GolfBall().m_vel;
		WMatrix cam = MakeCameraMatrix(GolfBall().m_pos, dir);
		float z = (float)(m_f190 - 3.0);
		m_srcCamera = WVector(-0.7f, -0.3f, z) * cam;
		int i = m_ib8 - 25;
		i = i > 0 ? i : 0;
		const CGolfBall::sExpected* p = GolfBall().PreData(i);
		m_destCamera = p->pos;
		return;
	}
	if (m_fac > m_f9c)
	{
		m_mode = 4;
		return;
	}
	m_mode = 0x13;
}

void CCameraWork::SpikeACamera(float dt, float t)
{
	// 983 006d4840: constantes em double (55 e 35 graus, PI/3, 2PI/3, 0.93, graus->rad) como no 983
	if (m_fac < m_f9c - 75.0)
	{
		float r = (t + m_fac) / m_f9c;
		m_f268 = (float)(0.959931081160903 - r * 0.6108652334660292);
		m_destCamera = GolfBall().m_pos;
		m_destCamera.y += (1.0f - r) * 2.0f;
		return;
	}
	if (m_fac < m_f9c)
	{
		m_f268 = 0.95993108f;
		float r = (t + m_fac) / m_f9c;
		m_f2a0 = (float)((r * 60.0 - 30.0) * 0.01745329238474369);
		WVector v = GolfBall().m_vel;
		v.Normalize();
		WVector side = WCrossProduct(WVector::UNIT_POS_Y, v);
		side.Normalize();
		side = side * (float)(r * 0.5 - 1.0);
		m_destCamera = side + GolfBall().m_pos;
		float d = (float)((1.0f - r * 0.8999999761581421) * 50.0);
		m_srcCamera = m_destCamera + v * d;
		m_posFrom = GolfBall().m_pos - m_destCamera;
		return;
	}
	if (m_ib8 < GolfBall().m_spikeDiveFrame)
	{
		if (m_f2a0 > 0.0)
		{
			m_f2a0 = (float)(m_f2a0 - dt * 1.0471975430846214);
			if (m_f2a0 < 0.0f)
			{
				m_f2a0 = 0.0f;
			}
		}
		WVector dir = GolfBall().m_pos - m_srcCamera;
		dir.y = 0.0f;
		m_f25c = dir.Magnitude();
		dir.Normalize();
		WVector side = WCrossProduct(WVector::UNIT_POS_Y, dir);
		float k = (float)((m_fac - m_f9c) * 0.019999999552965164);
		if (k < 1.0f)
		{
			k = (float)((k * 1.2000000476837158 + 1.0) * (m_f25c * 2.094395086169243));
			m_srcCamera += side * k * dt;
			m_srcCamera.y += dt;
			m_destCamera.y = (float)(0.75 * dt + m_destCamera.y);
			m_destCamera += dt * m_posFrom;
			return;
		}
	}
	else
	{
		m_srcCamera += GolfBall().m_vel * 0.9300000071525574 * dt;
		m_destCamera += GolfBall().m_vel * 0.9300000071525574 * dt;
		if (GolfBall().m_afterBurnerFrame == -1 ||
			m_ib8 < GolfBall().m_afterBurnerFrame)
		{
			m_destCamera.y = (float)(m_destCamera.y - dt * 0.5);
			return;
		}
	}
	return;
}

void CCameraWork::TopViewCamera(float dt, float t)
{
	// 983 (006d4ef0): quadro com dt == 0 nao mexe na camera
	if (dt == 0.0f)
		return;
	m_destCamera = m_destCamera * 0.9f + GolfBall().m_pos * 0.1f;
	WVector d = m_matA.pivot -
		GolfBall().PreData(GolfBall().m_renderNum)->pos;
	float speed = GolfBall().m_vel.Magnitude();
	speed = speed + speed;
	float len = speed < 100.0 ? 100.0f : (speed > 150.0f ? 150.0f : speed);
	d.y = 0.0f;
	d.Normalize();
	d.y = 2.0f;
	d.Normalize();
	d = d * len;
	m_srcCamera = m_srcCamera * 0.9f + (d + GolfBall().m_pos) * 0.1f;
	InterpolateBezier(&m_srcCamera, 0.5f, &m_srcCamera, 3);
}

void CCameraWork::JustLookingCamera(float dt, float t)
{
	if (m_i8c == 100)
	{
		WVector a = m_matB.za;
		WVector n = (GolfBall().m_pos - m_srcCamera).Normalize();
		WVector lim = (m_ballStart - m_matB.pivot).Normalize();
		if (n.y > a.y || lim.y > n.y)
		{
			n.y = (n.y - a.y) * dt * 0.1f + a.y;
			m_destCamera = m_srcCamera + n;
		}
		else
		{
			float k = sqrtf((1.0f - a.y * a.y) / (1.0f - n.y * n.y));
			n.x = n.x * k;
			n.z = n.z * k;
			n.y = a.y;
			m_destCamera = m_srcCamera + n;
		}
	}
	else
	{
		m_destCamera = GolfBall().m_pos;
	}
	if (m_fa4 > 180.0f && m_fac > m_fa4 * 0.5f)
	{
		// 983 006d5522: the first vector is PreData(0) (shot start, inlined), not the last
		// point; the camera only switches while start and ball are on the same side of the cup
		WVector toStart = GolfBall().PreData(0)->pos - GetHoleCupPos();
		WVector toBall = GolfBall().PreData(GolfBall().m_renderNum)->pos - GetHoleCupPos();
		if (0.0f < toBall * toStart)
		{
			WVector d = GolfBall().PreData(GolfBall().m_renderNum)->pos - GetHoleCupPos();
			if (d.Magnitude() < 16.0f)
			{
				SetCamera(0x6f + m_i298 % 2, true);
			}
		}
	}
}

void CCameraWork::SideCamera(float dt, float t)
{
	// 983 006d56f0: usa dt (o do 645 usava t); y da velocidade zerado
	WVector vel = GolfBall().m_vel;
	m_srcCamera += WVector(vel.x, 0.0f, vel.z) * dt * 0.8f;
	m_destCamera = GolfBall().m_pos;
	m_destCamera.y = m_destCamera.y + 2.5f;
}

void CCameraWork::BallSideCamera(float dt, float t)
{
	WVector vel = GolfBall().m_vel;
	WVector side;
	side = WCrossProduct(WVector::UNIT_POS_Y, vel);
	vel = vel.Normalize() * GolfBall().m_radius * 2.0f;
	side = side.Normalize() * GolfBall().m_radius * 1.2f;
	float k = (t + m_fac) / m_fa4;
	t = 1.0f <= k ? 1.0f : k;
	m_destCamera = side + GolfBall().m_pos - vel;
	m_srcCamera = (m_destCamera - m_matB.pivot) * t + m_matB.pivot - vel;
}

void CCameraWork::FollowBallCamera(float dt, float t)
{
	// 983 006d5ba0: em WVector (Magnitude/Normalize); 3.0, 3.2f e 1.0 em double
	const WVector& pin = GOLFDOC()->GetHoleData().pin;
	WVector vel = GolfBall().m_pos - pin;
	vel.y = 0.0f;
	float dist = vel.Magnitude();
	vel = GolfBall().m_vel;
	vel.y = 0.0f;
	if (vel.Magnitude() > 1.0f)
	{
		if (dist > GolfBall().m_radius * 3.0)
		{
			m_destCamera = GetPVS().GetGroundPoint(GolfBall().m_pos, false, 0, false);
			m_destCamera.y += GolfBall().m_radius;
			m_srcCamera = m_destCamera - vel.Normalize() * 3.200000047683716;
			m_srcCamera.y = m_srcCamera.y + 1.0;
		}
	}
}

void CCameraWork::FixedCamera(float dt, float t)
{
	float radius = GolfBall().m_radius;
	WSphere sphere(GolfBall().m_pos, radius);
	if (!g_view->InFrustumSafe(sphere))
		SetCamera(0x6d + m_i298 % 2, true);
}

void CCameraWork::PuttSecondACamera(float dt, float t)
{
	m_srcCamera.y = m_srcCamera.y - dt * 9.0f;
	m_destCamera = GolfBall().m_pos;
}

void CCameraWork::PuttSecondBCamera(float dt, float t)
{
	// 983 006d5f10: distancia em float; o pino e lido de novo depois (o 983 nao reaproveita)
	const WVector& cup = GOLFDOC()->GetHoleData().pin;
	float dist = WVector(g_camera.xm - cup.x, g_camera.ym - cup.y, g_camera.zm - cup.z).Magnitude();
	if (16.0f > dist)
	{
		m_pfnCamera = 0;
		return;
	}
	const WVector& pin = GOLFDOC()->GetHoleData().pin;
	CGolfBall& ball = GolfBall();
	const WVector* e = ball.m_renderNum < 0 ? 0 : &ball.m_expected[0].pos;
	WVector d(e->x - pin.x, 0.0f, e->z - pin.z);
	d.Normalize();
	d *= (float)(dt * 20.0);
	m_srcCamera += d;
	m_destCamera = GolfBall().m_pos;
	m_f268 += dt * (10.0 * g_DEGTORAD);
}

void CCameraWork::FromHoleCupCamera(float dt, float t)
{
	const WVector& pin = GOLFDOC()->GetHoleData().pin;
	WVector src(pin.x, pin.y - 2.0f, pin.z);
	m_srcCamera = src;
	WVector dest(m_srcCamera.x, m_srcCamera.y + 1.0f, m_srcCamera.z + 0.2f);
	m_destCamera = dest;
}

void CCameraWork::IntroACamera(float dt, float t)
{
	// 983 006d61c0 (o 983 poe 0xcd = o nosso 0xcc, numeracao deslocada de 1)
	m_f1a8 = m_f1a8 - dt * (30.0 * g_DEGTORAD);
	m_f25c = m_f25c - dt * 11.0f;
	m_f260 = m_f260 - dt * 2.0f;
	WMatrix rot = RotMat(m_f1a8 + GOLFDOC()->m_cameraData.angle, 1);
	WVector dir(rot.xz, rot.yz, rot.zz);
	WVector off = dir * m_f25c;
	m_srcCamera = GolfBall().m_pos - off;
	m_srcCamera.y = m_f260 + m_srcCamera.y;
	m_destCamera = dir + m_srcCamera;
	if (m_f1a8 < 0.0f)
	{
		m_mode = 0xcc;
	}
}


void CCameraWork::IntroBCamera(float dt, float t)
{
	// 983 006d6350 (o 983 poe 0xce = o nosso 0xcd, numeracao deslocada de 1)
	float r = m_f190 / 5.0;
	m_srcCamera = (m_posTo - m_posFrom) * r + m_posFrom;
	m_srcCamera.y += m_f260;
	m_destCamera = GolfBall().m_pos;
	r = (r + r) - 1.0;
	if (!(r > 0.0f))
	{
		r = -r;
	}
	m_destCamera.y += m_f260 - (m_f260 - 3.0) * r;
	if (m_f190 > 5.0f)
	{
		m_mode = 0xcd;
	}
}

void CCameraWork::ZoomOutCamera(float dt, float t)
{
	// 983 006cdf80: 55 graus em double, o mesmo literal na soma e no teste
	m_f268 += dt * 0.959931081160903;
	if (m_f268 > 0.959931081160903)
		m_f268 = 0.95993108f;
}

void CCameraWork::IntroCCamera(float dt, float t)
{
	m_f1a8 = m_f1a8 - dt * (30.0 * 0.017453292f); // 30 graus/s, dobrado em double como o 983
	m_f25c = dt + dt + m_f25c;
	WPuppet* puppet =
		SCENEMGR()->GetPuppet(MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer));
	WBone* head = puppet->FindBone("Bip01 Head");
	// 983 006d64e0: k = m_f190 / 5.0; fim no 0xcd do 983 = ToDefaultCamera, que no nosso switch e o 204
	float k = m_f190 / 5.0;
	WVector ball = (GolfBall().m_pos + WVector(0.0f, 5.0f, 0.0f)) * k;
	m_destCamera = head->GetMatrix().pivot * (1.0f - k) + ball;
	WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle + m_f1a8, 1);
	m_srcCamera = m_destCamera - rot.za * m_f25c;
	if (!(k <= 1.0f))
	{
		m_mode = 204;
	}
}


void CCameraWork::IntroDCamera(float dt, float t)
{
	// 983 006d6700: d = dir * dt num WVector e return logo depois; o fim grava 0xcd (= ToDefaultCamera do 983; no nosso switch e o 204)
	if (m_f190 < 3.0f)
	{
		const WVector& dir = *(const WVector*)&GOLFDOC()->m_cameraData.mat;
		WVector d = dir * dt;
		m_srcCamera.x += d.x;
		m_srcCamera.y += d.y;
		m_srcCamera.z += d.z;
		m_destCamera += d;
		return;
	}
	if (m_f190 > 4.0f)
	{
		WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
		WVector off(rot.xz * 25.0f, rot.yz * 25.0f, rot.zz * 25.0f);
		const WVector& ball = GolfBall().m_pos;
		WVector src(ball.x - off.x, ball.y - off.y, ball.z - off.z);
		m_srcCamera = src;
		m_srcCamera.y = m_srcCamera.y + 5.0f;
		rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
		WVector dest(m_srcCamera.x + rot.xz, m_srcCamera.y + rot.yz,
			m_srcCamera.z + rot.zz);
		m_destCamera = dest;
		m_matB = GetCameraMatrix(m_srcCamera, m_destCamera, 0.0f);
		m_mode = 204;
	}
}

void CCameraWork::ToDefaultCamera(float dt, float t)
{
	// 983 006d6930: interpolacao em WVector; o eixo za do slerp aponta o alvo
	if (!(m_f190 > m_f264))
	{
		m_srcCamera = (m_posTo - m_posFrom) * m_f190 + m_posFrom;
		WMatrix m;
		m = WQuaternionSlerp(m_quatFrom, m_quatTo, m_f190);
		m_destCamera = m_srcCamera + m.za;
	}
}

void CCameraWork::FirstPersonCamera(float dt, float t)
{
}

void CCameraWork::StillFrameCamera(float dt, float t)
{
}

void CCameraWork::CaddieChanceCamera(float dt, float t)
{
	// 983 006ce9f0: k = 1 - f190 / 3.0 (double), preso em [0, 1] duas vezes no lugar;
	// a rotacao vem antes do UNIT_POS_Z * m_matC
	float k = 1.0f - m_f190 / 3.0;
	k = k < 0.0f ? 0.0f : (k > 1.0f ? 1.0f : k);
	WMatrix rot = RotMat(m_f1a8 * k, 1);
	WVector r = WVector::UNIT_POS_Z * m_matC * rot;
	k = k < 0.0f ? 0.0f : (k > 1.0f ? 1.0f : k);
	m_srcCamera = m_posTo + (m_posFrom - m_posTo) * k;
	m_destCamera = m_srcCamera + r;
}


void CCameraWork::SwingReplayACamera(float dt, float t)
{
}

void CCameraWork::SwingReplayBCamera(float dt, float t)
{
	// 983 006d6a40
	if (!(m_f190 > m_f264))
	{
		float r = m_f190 / m_f264;
		m_srcCamera = m_posFrom + (m_posTo - m_posFrom) * r;
		WMatrix m;
		m = WQuaternionSlerp(m_quatFrom, m_quatTo, r);
		m_destCamera = m_srcCamera + m.za;
	}
}

void CCameraWork::ExtraTimeCamera(float dt, float t)
{
	m_f1a8 += dt * 0.0872665f;
	WMatrix rot;
	rot.Reset();
	rot.Rotate(m_f1a8, 1);
	m_srcCamera = WVector(rot.xz * m_f25c + m_destCamera.x,
		rot.yz * m_f25c + m_destCamera.y, rot.zz * m_f25c + m_destCamera.z);
	m_srcCamera.y = m_f25c * 0.2f + m_srcCamera.y;
}

// 983 006cecf0: ExtraTimeCamera at 2 deg/s instead of 5 (SetCamera 983 210 = our 209)
void CCameraWork::ExtraTimeSlowCamera983(float dt, float t)
{
	m_f1a8 += dt * 0.0349066f;
	WMatrix rot;
	rot.Reset();
	rot.Rotate(m_f1a8, 1);
	m_srcCamera = WVector(rot.xz * m_f25c + m_destCamera.x,
		rot.yz * m_f25c + m_destCamera.y, rot.zz * m_f25c + m_destCamera.z);
	m_srcCamera.y = m_f25c * 0.2f + m_srcCamera.y;
}


void CCameraWork::Quake()
{
	m_flags |= 0x80000000;
	m_f198 = m_f190;
	m_f194 = -1.0f;
}

WMatrix __fastcall GetCameraMatrix(const WVector& eye, const WVector& at,
	float roll)
{
	float a = roll;
	if (*(int*)&a < 0)
	{
		a = -a;
	}
	if (a < g_EPSILON)
	{
		return MakeCameraMatrix(eye, at);
	}
	return RotMat(roll, 2) * MakeCameraMatrix(eye, at);
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?RayIntersectAll@CQuadTreeEx@@QAEMABVWVector@@0_N@Z=?RayIntersectAll@CQuadTree@@QAEMABVWVector@@0_N@Z") // visao derivada
