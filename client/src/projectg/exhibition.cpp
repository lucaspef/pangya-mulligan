#include "minatl.h"
#include "vc10_layout.h"
#include "exhibition.h"
#include "petframe.h"

// CFxBox's first virtual after the destructor (vtable slot 1); not declared in the shared
// fxbox.h because a new virtual there changes the CFxBox vtable for every unit
class CFxBoxVt
{
public:
	virtual ~CFxBoxVt();
	virtual void ProcessAttacher(float elapsed);
};

// CFxSpray / CFxSequence attach fields (offsets from the original)
struct SprayAttach
{
	char pad[8];
	const Attacher* pAttacher;
	float attachX;
	float attachY;
	WMatrix* attachMat[4];
	WVector* attachPos[4];
};

struct SeqAttach
{
	char pad[0x14];
	const Attacher* pAttacher;
	float attachX;
	float attachY;
};
#include "texturepart.h"
#include "fxbox.h"
#include "fxsequence.h"
#include "fxspray.h"
#include "fx.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "wresrcmng.h"
#include "wbone.h"
#include "rtti.h"
#include "taskmanager.h"
#include "mascot.h"
#include "mathconsts.h"
#include "../shared/localize.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "frdesktop.h"
#include "fxinterpreter.h"
#include "jp/motionadvisor.h"
#include "jp/gdskinitem.h" // 983: FindTexturePart (CGDSkinItem "TanLines")
#include "gdskininstance983.h" // 983 00542f00
#include <algorithm> // 983: SetModel(tid) (caddie pela textura do corpo)
#include <iterator>
#include <ctype.h>


extern Fresh* g_pFresh;
extern WMatrix g_camera;

class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

class FrBongdariShopDlg
{
public:
	void CreateBonusBall(WVector pos);
};

// Not in shareddoc.h (PCH): declared on a helper class and aliased.
class CSharedDocVis
{
public:
	std::list<std::string>* GetVisGroup(const char* name);
};
#pragma comment(linker, \
	"/alternatename:?GetVisGroup@CSharedDocVis@@QAEPAV?$list@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@PBD@Z=?GetVisGroup@CSharedDoc@@QAEPAV?$list@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@PBD@Z")

const char* GetStrConst(const char* str, char* out);
const char* GetCharsTillSpace(const char* str, char* out);
int Ceil(float value, float epsilon);

// HACK: forces the float constants that the original unit emitted as objects.
inline void rb_UseConsts()
{
	(void)g_PI;
	(void)g_DEGTORAD;
	(void)g_HUGE;
}

// 983 00d33d78 (GetNextCharMotion 0087394c): 15 personagens; a do 645 tinha 10 e os tipos 10..14 liam o
// s_caddieMotionTable logo depois. Linha 8 sem o 10 (= 983).
static const int s_charMotionTable[15][12] = {
	{ 0, 1, 3, 5, 6, -1 },
	{ 0, 1, 3, 5, 6, -1 },
	{ 0, 1, 3, 5, 6, -1 },
	{ 0, 1, 3, 5, 6, -1 },
	{ 0, 3, 1, 5, 6, -1 },
	{ 0, 3, 5, 6, -1 },
	{ 0, 6, 3, -1 },
	{ 0, 4, 4, 0 },
	{ 0, 3, 5, 7, -1 },
	{ 0, 3, 5, 7, 10, -1 },
	{ 0, 3, 5, 7, -1 },
};

static const int s_caddieMotionTable[28][9] = {
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
	{ 0, 3, 4, 6, 7, -1 },
};

static char* s_charMotions[12] = {
	"\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc",
	"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"1\xb5\xee\xb8\xf0\xbc\xc7",
	"2\xb5\xee\xb8\xf0\xbc\xc7",
	"3\xb5\xee\xb8\xf0\xbc\xc7",
	"\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbf"
	"OB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8",
	"\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0",
	"\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
};

static char* s_caddieMotions[10] = {
	"\xb1\xe2\xba\xbb",
	"\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2",
	"\xb3\xf4\xc0\xba\xb0\xf8\xc7\xc7\xc7\xcf\xb1\xe2",
	"\xba\xa1\xc4\xbf",
	"\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2",
	"\xbd\xc7\xb0\xdd",
	"\xc2\xf9\xbd\xba",
	"\xc8\xa6\xc0\xce",
	"\xc8\xa6\xc0\xce\xbf\xf8",
};

// 983: as expressoes do bate-papo por personagem ficam no CreateCharacterOption (00e73e84; include/jp/createcharacteroption.h,
// B124.1); a tabela abaixo e a do 645
#include "jp/createcharacteroption.h"

static char* s_chatFacials[10][4] = {
	{ "m_skin_01-f03.png", "m_skin_01-f04.png", "m_skin_01-f12.png",
     "m_skin_01-reset.png"															  },
	{ "F_Skin-f05.png",    "F_Skin-f01.png",    "F_Skin-f06.png",
     "F_Skin-reset.png"																 },
	{ "A_Face-f02.png",    "A_Face-f03.png",    "A_Face-f05.png",
     "A_Face-reset.png"																 },
	{ "C_Face-f03.png",    "C_Face-f02.png",    "C_Face-f04.png",
     "C_Face-reset.png"																 },
	{ "D_Face-f07.png",    "D_Face-f04.png",    "D_Face-f05.png",
     "D_Face-reset.png"																 },
	{ "E_def_f03.png",     "E_def_f05.png",     "E_def_f06.png",     "E_def_reset.png"  },
	{ "G_Face-blink1.png", "G_Face-f03.png",    "G_Face-f06.png",
     "G_Face-reset.PNG"																 },
	{ "H_def_f4.PNG",      "H_def_f6.PNG",      "H_def_f7.PNG",      "H_Face-reset.PNG" },
	{ "I_def_f13.PNG",     "I_def_f23.PNG",     "I_def_f21.PNG",     "I_def_reset.PNG"  },
	{ "J_Def_Blink17.PNG", "J_Def_Blink19.PNG", "J_Def_Blink22.PNG",
     "J_def-reset.PNG"																  },
};

inline void WPuppet::LockBoneSetPos(unsigned char pos)
{
	if (0)
		((CExhibition*)0)->CExhibition::AttachAngelWingFx();
	ms_lockedBoneSetPos = pos;
}

static char s_scriptName[0x40];
static char s_scriptCmd[0x400];

// 983 00877e30: + uid (+0x644) e o FxInterpreter (+0x548); +0x642/+0x643/+0x648 zerados; +0x576 nao e mais gravado
CExhibition::CExhibition(float x, float y, float z, unsigned long uid, bool bFlag)
	: m_idleTimer(0.0f),
	  m_motionLength(0.0f),
	  m_bLeftDown(false),
	  m_bRightDown(false),
	  m_distance(20.0f),
	  m_pitchLimit(0.0f),
	  m_motionTime(0.0f),
	  m_pMotion(NULL),
	  m_pPetFrame(NULL),
	  m_pPuppet(NULL),
	  m_motionCursor(-1),
	  m_charIndex(-1),
	  m_caddieIndex(-1),
	  m_bChatFacial(false)
{
	if (0) // HACK: keeps the inline naked ProcessBongdariShop emitted (nakedify)
		((CExhibition*)0)->ProcessBongdariShop(*(float*)0);
	m_center.x = x;
	m_center.y = y;
	m_bAutoRotate = bFlag;
	m_scale = z;
	m_bEnableZoom = true;
	m_bEnableRotate = true;
	m_bMotionToggle983 = false;
	m_bRandomMotion983 = false;
	m_uid = uid;
	m_bPetBillboard983 = false;
	Init();
	m_fxBox.Init(g_resrcmng);
	m_fxBox.SetCamera(&g_camera);
	m_fxBox.m_frameTime = 33.333332f;
	m_pFxInterpreter = new FxInterpreter(&m_fxBox, 0);
}

// 983 00877180 (as derivadas chamam direto): + apaga o FxInterpreter
CExhibition::~CExhibition()
{
	ReleasePet();
	m_fxBox.ClearAll(false);
	for (std::list<sTexturePart*>::iterator it = m_texParts.begin();
		it != m_texParts.end(); ++it)
	{
		sTexturePart* pPart = *it;
		if (pPart)
			delete pPart;
	}
	m_texParts.clear();
	if (m_pFxInterpreter)
	{
		delete m_pFxInterpreter;
		m_pFxInterpreter = NULL;
	}
}

void CExhibition::ReleasePet()
{
	if (m_pPuppet)
		m_pPuppet->LockBoneSetPos(0xff);
	if (m_type == EXHIBIT_CHARACTER)
	{
		if (m_pPetFrame)
		{
			delete m_pPetFrame;
			m_pPetFrame = NULL;
		}
		m_pPuppet = NULL;
	}
	else
	{
		if (g_resrcmng && m_pPuppet)
		{
			g_resrcmng->Release(m_pPuppet, false);
			m_pPuppet = NULL;
		}
	}
	if (g_resrcmng && m_msgTexture)
	{
		g_resrcmng->Release(m_msgTexture);
		m_msgTexture = 0;
	}
}

void CExhibition::Init()
{
	m_pitchSpeed = m_yawSpeed = 0.0f;
	m_defaultPitch = m_defaultYaw = 0.0f;
	m_pitch = m_yaw = 0.0f;
	m_defaultDistance = m_distance;
	m_lastYaw = 0.0f;
	m_dirTimer = 0.0f;
	m_facialTimer = 0.0f;
	m_targetDistance = m_distance;
	m_unused54 = 0;
	m_minDistance = 15.0f;
	m_maxDistance = 25.0f;
	m_tid = 0;
	m_pUids = NULL;
	m_flags.Set(0x16); // 983 00874930: + 0x10 (UpdateCamera centra o boneco entre os pes)
	m_bMotionChanged = false;
	UpdateCamera(0.0f);
	m_rotateState = 2;
	m_modelMat.Reset();
	m_msgTexture = 0;
	m_light.diffuse = 0x969696;
	m_light.type = 2;
}

void CExhibition::SetDistance(float distance, bool bZoom, bool bFixed)
{
	m_targetDistance = Between(m_minDistance, distance, m_maxDistance);
	if (bZoom)
	{
		m_targetDistance = distance;
		m_distance = distance;
	}
	UpdateCamera(0.0f);
	// 983 00874a20: so g_view && bFixed (sem o teste da CRealMyRoomTask do 645)
	if (g_view && bFixed)
	{
		g_view->SetCamera(m_camera);
		g_view->UpdateCamera();
	}
}

float CExhibition::GetDistance()
{
	return m_targetDistance;
}

void CExhibition::Zoom(float delta)
{
	SetDistance(delta + m_targetDistance, false, false);
	m_rotateState = 1;
	m_autoRotateTimer = 0.0f;
}

void CExhibition::UpdateCamera(float deltaTime)
{
	float diff = m_distance - m_targetDistance;
	float step = deltaTime * 15.0f;
	if (diff > step)
		m_distance -= step;
	else if (diff < deltaTime * -15.0f)
		m_distance += step;
	else
		m_distance = m_targetDistance;
	m_camera = RotMat(m_pitch, 0) * RotMat(m_yaw, 1);
	// 983 00873b60: distancia escalada pela altura da tela (SCREEN_YS / 600) e centro entre os dois pes
	float scale = (float)(g_view->SCREEN_YS / 600.0);
	m_camera.pivot = m_camera.za * (-m_distance * scale);
	if (m_flags.GetFlag(0x10) && m_pPetFrame)
	{
		WBone* footL = m_pPetFrame->GetBonePet()->FindBone("Bip01 L Foot");
		WBone* footR = m_pPetFrame->GetBonePet()->FindBone("Bip01 R Foot");
		if (footL && footR)
		{
			float x = footR->m_matrix.xm + footL->m_matrix.xm;
			float z = footR->m_matrix.zm + footL->m_matrix.zm;
			m_camera.pivot.x += x * 0.5f;
			m_camera.pivot.z += z * 0.5f;
		}
	}
}

const char* CExhibition::GetMotionName()
{
	if (m_pMotion)
		return m_pMotion->name;
	return NULL;
}

void CExhibition::Process(float deltaTime, bool bPaused)
{
	if (!m_pPuppet)
		return;
	if (!m_pPuppet->m_rootbone)
		return;
	if (m_alpha < 255.0f)
	{
		m_alpha = 255.0f;
		SetAlpha(0xff);
	}
	int state = g_input->GetButton(RIGHT_BUTTON);
	if (state == 1)
	{
		if (InValidArea())
			m_bRightDown = true;
	}
	else if (state == 2)
		m_bRightDown = false;
	state = g_input->GetButton(LEFT_BUTTON);
	if (state == 1)
	{
		if (InValidArea())
			m_bLeftDown = true;
	}
	else if (state == 2)
		m_bLeftDown = false;
	m_bMotionChanged = false;
	if (!m_flags.GetFlag(8))
	{
rotate_check:
		if (!m_bLeftDown || !m_bEnableRotate)
		{
zoom_check:
			if (!m_bRightDown || !m_bEnableZoom)
			{
				if (m_bAutoRotate)
				{
					float old = m_autoRotateTimer;
					m_autoRotateTimer = deltaTime + old;
					if (10.0f < deltaTime + old && m_rotateState == 1)
					{
						// 983: uma volta so (2 pi) quando passou de meia volta acima do padrao (o 645 usava GetBaseAngle)
						float dp = (float)((m_defaultPitch + (double)g_PI < m_pitch ? g_PI * 2.0 : 0.0) - m_pitch +
							m_defaultPitch);
						float dy = (float)((m_defaultYaw + (double)g_PI < m_yaw ? g_PI * 2.0 : 0.0) - m_yaw +
							m_defaultYaw);
						if (WisZero(dp) && WisZero(dy))
						{
							m_yawSpeed = 0.0f;
							m_pitchSpeed = 0.0f;
							m_rotateState = 2;
							m_targetDistance = m_defaultDistance;
						}
						else
						{
							float sum = Abs<float>(dy) + Abs<float>(dp);
							m_rotateState = 0;
							m_pitchSpeed = (float)(dp / sum * (double)g_PI);
							m_yawSpeed = (float)(dy / sum * (double)g_PI);
							m_targetDistance = m_defaultDistance;
						}
					}
				}
			}
			else if (bPaused && m_flags.GetFlag(1))
			{
				WVector delta = CInputManager::Instance()->GetMouseDelta();
				Zoom((float)(delta.y * (double)0.1f));
				CMouseCursor::Instance()->SetCursor(6);
			}
		}
		else
		{
			// 983: velocidade proporcional ao tempo do quadro (0,7 por unidade de mouse por segundo; o 645 usava 0,01 fixo)
			// e limite de cima/baixo com o angulo em [0, 2 pi): abaixo de limite + 18 graus ou acima de 300 graus
			double speed = deltaTime * 0.7;
			float dp = (float)(m_pitch - CInputManager::Instance()->GetMouseDelta().y * speed - m_defaultPitch);
			if (m_flags.GetFlag(4) && (dp < m_pitchLimit + 18.0 * g_DEGTORAD || 300.0 * g_DEGTORAD < dp))
				m_pitch = (float)(m_pitch - CInputManager::Instance()->GetMouseDelta().y * speed);
			if (m_flags.GetFlag(2))
				m_yaw = (float)(m_yaw - CInputManager::Instance()->GetMouseDelta().x * speed);
			if (m_flags.GetFlag(6))
			{
				m_rotateState = 1;
				m_autoRotateTimer = 0.0f;
			}
		}
	}
	else
	{
		if (!m_bLeftDown)
			goto zoom_check;
		if (!m_bRightDown || g_input->GetButton(MIDDLE_BUTTON) != 2)
			goto rotate_check;
		m_bMotionChanged = NextMotion();
	}
	if (m_rotateState == 0)
	{
		m_pitch = deltaTime * m_pitchSpeed + m_pitch;
		m_yaw = deltaTime * m_yawSpeed + m_yaw;
		if (((m_pitchSpeed < 0.0f && m_pitch < m_defaultPitch) ||
				(0.0f < m_pitchSpeed && m_defaultPitch < m_pitch)) ||
			((m_yawSpeed < 0.0f && m_yaw < m_defaultYaw) ||
				(0.0f < m_yawSpeed && m_defaultYaw < m_yaw)))
		{
			m_pitch = m_defaultPitch;
			m_yaw = m_defaultYaw;
			m_yawSpeed = 0.0f;
			m_pitchSpeed = 0.0f;
			m_rotateState = 2;
		}
		goto rotate;
	}
	else if (m_rotateState == 1)
	{
		m_yawSpeed = 0.0f;
		m_pitchSpeed = 0.0f;
	}
	else if (m_rotateState == 2)
	{
rotate:
		if (m_type == EXHIBIT_BALL || m_type == EXHIBIT_CLUB ||
			m_type == EXHIBIT_MATCH || m_type == EXHIBIT_ITEM)
		{
			WMatrix rot;
			RotMat((float)(deltaTime * (-60.0 * g_DEGTORAD)), 1, rot);
			m_modelMat *= rot;
		}
	}
	UpdateCamera(deltaTime);
	m_pitch = GetBaseAngle(m_pitch, m_defaultPitch);
	m_yaw = GetBaseAngle(m_yaw, m_defaultYaw);
	float t = deltaTime + m_dirTimer;
	m_dirTimer = t;
	if (1.0f < t)
	{
		m_dirTimer = t - 1.0f;
		if (!m_bDirChanged)
		{
			if (g_DEGTORAD < Abs<float>(GetBaseAngle(m_lastYaw, m_yaw) - m_yaw))
			{
				m_lastYaw = m_yaw;
				m_bDirChanged = true;
			}
		}
	}
	if (m_bChatFacial)
	{
		float t2 = deltaTime + m_facialTimer;
		m_facialTimer = t2;
		if (2.0f < t2)
		{
			m_facialTimer = t2 - 2.0f;
			SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(m_charIndex, 3));
			m_bChatFacial = false;
		}
	}
	float idle = m_idleTimer;
	m_idleTimer = idle - deltaTime;
	if (0.0f < idle - deltaTime)
		return;
	if (m_pMotion)
	{
		// 983: o script do quadro passa pelo ScriptProcess fora de linha; com m_bMotionToggle983 o tempo do movimento para
		ScriptProcess(m_pMotion, m_motionTime, deltaTime + m_motionTime, false);
		if (!m_bMotionToggle983)
			m_motionTime += deltaTime;
		if (m_pMotion->e < m_motionTime)
		{
			SetMotion(m_pMotion->nextmotion, true, m_motionTime - m_pMotion->e);
			goto done;
		}
	}
	Animate();
done:
	float elapsed = deltaTime * 1000.0f;
	CFxBox* pBox = &m_fxBox;
	((CFxBoxVt*)pBox)->ProcessAttacher(elapsed);
	m_fxBox.Process(elapsed);
}

int CExhibition::ProcessBongdariShop(float deltaTime)
{
	if (!m_pPuppet)
		return 0;
	if (m_alpha < 255.0f)
	{
		m_alpha = 255.0f;
		SetAlpha(0xff);
	}
	int state = g_input->GetButton(LEFT_BUTTON);
	if (state == 1)
	{
		if (InValidArea())
			m_bLeftDown = true;
	}
	else if (state == 2)
		m_bLeftDown = false;
	if (m_bAutoRotate)
	{
		float old = m_autoRotateTimer;
		m_autoRotateTimer = deltaTime + old;
		if (10.0f < deltaTime + old && m_rotateState == 1)
		{
			float dp = m_defaultPitch - GetBaseAngle(m_pitch, m_defaultPitch);
			float dy = m_defaultYaw - GetBaseAngle(m_yaw, m_defaultYaw);
			if (!WisZero(dp) || !WisZero(dy))
			{
				float a = Abs<float>(dy);
				float b = Abs<float>(dp);
				m_rotateState = 0;
				// 983 008779d0: divide (o 645 multiplicava pelo inverso)
				m_pitchSpeed = dp / (b + a) * g_PI;
				m_yawSpeed = g_PI * (dy / (b + a));
			}
			else
			{
				m_yawSpeed = 0.0f;
				m_pitchSpeed = 0.0f;
				m_rotateState = 2;
			}
			m_targetDistance = m_defaultDistance;
		}
	}
	if (m_rotateState == 0)
	{
		m_pitch = deltaTime * m_pitchSpeed + m_pitch;
		m_yaw = deltaTime * m_yawSpeed + m_yaw;
		if (((m_pitchSpeed < 0.0f && m_pitch < m_defaultPitch) ||
				(0.0f < m_pitchSpeed && m_defaultPitch < m_pitch)) ||
			((m_yawSpeed < 0.0f && m_yaw < m_defaultYaw) ||
				(0.0f < m_yawSpeed && m_defaultYaw < m_yaw)))
		{
			m_pitch = m_defaultPitch;
			m_yaw = m_defaultYaw;
			m_yawSpeed = 0.0f;
			m_pitchSpeed = 0.0f;
			m_rotateState = 2;
		}
	}
	else if (m_rotateState == 1)
	{
		m_yawSpeed = 0.0f;
		m_pitchSpeed = 0.0f;
	}
	UpdateCamera(deltaTime);
	m_pitch = GetBaseAngle(m_pitch, m_defaultPitch);
	m_yaw = GetBaseAngle(m_yaw, m_defaultYaw);
	float t = deltaTime + m_dirTimer;
	m_dirTimer = t;
	if (1.0f < t)
	{
		m_dirTimer = t - 1.0f;
		if (!m_bDirChanged)
		{
			float da = Abs<float>(GetBaseAngle(m_lastYaw, m_yaw) - m_yaw);
			if (g_DEGTORAD < da)
			{
				m_lastYaw = m_yaw;
				m_bDirChanged = true;
			}
		}
	}
	float idle = m_idleTimer;
	m_idleTimer = idle - deltaTime;
	if (0.0f < idle - deltaTime)
		return 0;
	int result = 0;
	if (m_pMotion)
	{
		// 983 00877d1c: ScriptProcess fora de linha (como o Process)
		ScriptProcess(m_pMotion, m_motionTime, deltaTime + m_motionTime, false);
		float t3 = deltaTime + m_motionTime;
		m_motionTime = t3;
		if (m_pMotion->e < t3)
		{
			const char* next = m_pMotion->nextmotion;
			if (strcmp(m_pMotion->name, next) != 0)
				result = (strcmp(m_pMotion->name,
							  "\xbb\xf5\xb7\xce\xbd\xc3\xc0\xdb") == 0) +
					1;
			SetMotion(next, true, t3 - m_pMotion->e);
			goto done;
		}
	}
	Animate();
done:
	float elapsed = deltaTime * 1000.0f;
	CFxBox* pBox = &m_fxBox;
	((CFxBoxVt*)pBox)->ProcessAttacher(elapsed);
	m_fxBox.Process(elapsed);
	return result;
}

void CExhibition::SetDirection(float direction)
{
	m_yaw = direction;
}

void CExhibition::SetDefaultDirection(float direction)
{
	m_rotateState = 1;
	m_autoRotateTimer = 10.0f;
	m_defaultYaw = direction;
}

bool CExhibition::IsDirectionChanged()
{
	if (!m_bLeftDown && m_bDirChanged)
	{
		m_bDirChanged = false;
		return true;
	}
	return false;
}

// 983 00873880: um passo de 2*pi so, para a faixa [base, base + 2*pi] (645: lacos de +-pi)
float CExhibition::GetBaseAngle(float angle, float base)
{
	const double twoPi = (double)(g_PI * 2.0f);
	if (angle > base + twoPi)
		angle = (float)(angle - twoPi);
	if (angle < base)
		angle = (float)(angle + twoPi);
	return angle;
}

// 983 00874850: com "*petbillboard" no script, gira o modelo em y para o lado da camera antes de animar
void CExhibition::BillboardToCamera983()
{
	WVector dir((float)(m_modelMat.xz * -1.0), (float)(m_modelMat.yz * -1.0), (float)(m_modelMat.zz * -1.0));
	WVector delta(m_camera.xm - m_modelMat.xm, m_camera.ym - m_modelMat.ym, m_camera.zm - m_modelMat.zm);
	WMatrix rot;
	RotMat(CalcDeltaAngle(delta, dir), 1, rot);
	m_modelMat = rot * m_modelMat;
}

// 983 00874eb0: a luz e a mesma para todos os tipos (angulos fixos em relacao a camera); a matriz vai para o boneco
// (WPuppet::SetMatrix) e a animacao sai do WPuppet::Animate983 / CPetFrame::Animate983 (era GetBones + ApplyBones);
// sem o LockBoneSetPos do fim
void CExhibition::Animate()
{
	WMatrix lightRot;
	RotMat(WVector(-0.7853982f, -0.5235988f, 0.0f), lightRot);
	m_light.nearOne = RotVec(WVector(0.0f, 0.0f, 1.0f), lightRot * m_camera);
	m_light.nearOne.Normalize();
	if (m_bPetBillboard983)
		BillboardToCamera983();
	const WMatrix* pMat;
	WMatrix modelMat;
	switch (m_type)
	{
	case 0:
		m_pPetFrame->m_pBonePet->SetMatrix(m_modelMat);
		m_pPetFrame->Animate983(NULL, NULL, m_motionTime);
		goto done;
	case 1:
	case 6:
	case 12:
		pMat = &m_modelMat;
		break;
	case 2:
	case 4:
		m_pPuppet->SetMatrix(m_modelMat);
		goto done;
	case 3:
		modelMat = m_modelMat * RotMat(-15.0f * g_DEGTORAD, 2);
		pMat = &modelMat;
		break;
	case 5:
		modelMat = m_modelMat * RotMat(-15.0f * g_DEGTORAD, 2);
		pMat = &modelMat;
		break;
	case 7:
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		Waabb box;
		box = m_pPuppet->m_bound;
		pMat = &m_modelMat;
		m_modelMat.ym = (box.max.y - box.min.y) * -0.5f;
		break;
	}
	case 8:
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		pMat = &m_modelMat;
		break;
	case 9:
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		modelMat = m_modelMat * RotMat(-15.0f * g_DEGTORAD, 2);
		pMat = &modelMat;
		break;
	case 10:
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		WMatrix scale;
		scale.Reset();
		scale *= 0.7f;
		modelMat = m_modelMat * RotMat(90.0f * g_DEGTORAD, 0) *
			RotMat(-15.0f * g_DEGTORAD, 2) * scale;
		pMat = &modelMat;
		break;
	}
	case 11:
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		WMatrix scale;
		scale.Reset();
		scale *= 0.85f;
		modelMat = m_modelMat * RotMat(90.0f * g_DEGTORAD, 0) *
			RotMat(-15.0f * g_DEGTORAD, 2) * scale;
		pMat = &modelMat;
		break;
	}
	case 13:
		if (!IsLocalContent(S4_REAL_MYROOM))
			goto done;
		pMat = &m_modelMat;
		m_modelMat.xx = m_modelMat.yy = 2.0f;
		break;
	default:
		goto done;
	}
	m_pPuppet->SetMatrix(*pMat);
	m_pPuppet->Animate983(NULL, NULL, m_motionTime, 0, NULL, NULL);
done:
	if (m_pPuppet)
		m_pPuppet->Transform(NULL, 1.0f, 0);
}

void CExhibition::ScriptProcess(const w_motion_data* pMotion, float from,
	float to, bool bForce)
{
	if ((from <= pMotion->e && from != to) || bForce)
	{
		int first, last;
		if (bForce)
		{
			first = last = Ceil(from * 30.0f, 0.03f);
		}
		else
		{
			float end = pMotion->e;
			if (to < end)
				end = to;
			end *= 30.0f;
			first = Ceil(from * 30.0f, 0.03f);
			float diff = (float)floor(end) - end;
			if (Wabs(diff) >= g_EPSILON)
				last = (int)floor(end);
			else
				last = (int)end - 1;
			int start = (int)floor(pMotion->s * 30.0f + 0.5f);
			if (first == start)
				first++;
			if (first > last)
				return;
		}
		WPuppet* pPuppet;
		if (m_type == EXHIBIT_CHARACTER)
			pPuppet = m_pPetFrame->GetCurAniPet();
		else
			pPuppet = m_pPuppet;
		int from2, to2;
		if (pPuppet && pPuppet->CheckFrameData(first, last, from2, to2) &&
			from2 <= to2)
		{
			for (int i = from2; i <= to2; ++i)
				ParsePuppetScript(pPuppet->m_frame[i].data);
		}
	}
}

void CExhibition::ParsePuppetScript(const char* script)
{
	char cond[0x400];
	char motion[0x100];
	char tex2[0x40];
	char seg[0x28];
	char name1[0x40];
	char name2[0x40];
	char fxName[0x40];
	char part[0x40];
	char name3[0x40];
	if (!script)
		return;
	do
	{
		const char* p = strchr(script, '*');
		if (!p)
			return;
		size_t n = 0;
		if (*p == '\0')
			return;
		char c;
		do
		{
			c = p[n + 1];
			++n;
			if (c == ' ')
				break;
			if (c == '(')
			{
				++n;
				break;
			}
		} while (c != '\0');
		if ((int)n <= 1)
			return;
		strncpy(s_scriptCmd, p, n);
		p += n;
		s_scriptCmd[n] = 0;
		const char* rest = p;
		// 983: "*fx(" e "*fxstop(" passam pelo FxInterpreter do objeto (efeito ja ligado no grupo = so liga/desliga; senao cria)
		int fxMode = 0;
		if (strcmp(s_scriptCmd, "*fx(") == 0)
			fxMode = 1;
		else if (strcmp(s_scriptCmd, "*fxstop(") == 0)
			fxMode = 2;
		if (fxMode != 0)
		{
			fxName[0] = '\0';
			p = GetStrConst(p, fxName);
			if (strlen(fxName) == 0)
				return;
			if (strlen(fxName) >= 0x40)
				return;
			if (fxName[0] == '#')
			{
				// a chave "#..." nao serve aqui: o nome vem depois
				memset(fxName, 0, sizeof(fxName));
				p = GetStrConst(p, fxName);
			}
			if (fxName[0] == '@')
			{
				if (!m_pMotion)
					return;
				if (strcmp(fxName + 1, m_pMotion->name) != 0)
					return;
				p = GetStrConst(p, name2);
				if (strlen(name2) == 0)
					return;
				if (strlen(name2) >= 0x40)
					return;
			}
			else
			{
				strcpy(name2, fxName);
				fxName[0] = '\0'; // sem condicao
			}
			rest = GetStrConst(p, part);
			int len = (int)strlen(part);
			if (len == 0 || len >= 0x40)
				return;
			bool bFound = false;
			std::vector<unsigned long> groups; // grupo do efeito = tid da peca do "&tid" que estiver vestida (0 = sem)
			if (part[0] == '&')
			{
				do
				{
					unsigned long id = atoi(part + 1);
					if (!bFound && m_pPetFrame->m_partList.FindPart(id))
					{
						bFound = true;
						groups.push_back(id);
					}
					rest = GetStrConst(rest, part);
					len = (int)strlen(part);
				} while (len != 0 && len < 0x40 && part[0] == '&');
				if (!bFound)
					goto next; // 983: segue para o proximo comando (o 645 parava)
			}
			strcpy(name3, part);
			if (strlen(name3) == 0)
				return;
			if (strlen(name3) >= 0x40)
				return;
			Attacher attacher;
			attacher.x = m_center.x;
			attacher.y = m_center.y;
			unsigned int i = 0;
			do
			{
				if (i != 0)
				{
					rest = GetStrConst(rest, name3);
					if (strlen(name3) == 0)
						break;
					if (strlen(name3) >= 0x40)
						break;
				}
				WBone* pBone = m_pPuppet->FindBone(name3);
				if (!pBone)
				{
					w_bound_box* bb = m_pPuppet->m_bbList.Find(name3);
					if (bb)
					{
						if (bb->bone)
							attacher.mat[i] = &bb->bone->m_matrix;
						attacher.pos[i] = &bb->info->spherePivot;
					}
				}
				else
					attacher.mat[i] = &pBone->m_matrix;
				++i;
			} while (i < 4);
			if ((int)i > 0)
			{
				char* ext = strrchr(name2, '.');
				if (ext && strlen(ext) >= 4)
				{
					std::vector<unsigned long>::iterator it;
					if (stricmp(ext + 1, "spr") == 0)
					{
						if (groups.size() != 0)
						{
							// 983: apaga e avanca o iterador mesmo assim (com 1 grupo passaria do fim); aqui o erase devolve o seguinte
							for (it = groups.begin(); it != groups.end();)
							{
								CFxSpray* spray = (CFxSpray*)m_pFxInterpreter->FindSpray(name2, fxName, &attacher, *it, 0, 0);
								if (spray)
								{
									spray->m_bActive = fxMode == 1;
									it = groups.erase(it);
								}
								else
									++it;
							}
						}
						else
						{
							CFxSpray* spray = (CFxSpray*)m_pFxInterpreter->FindSpray(name2, fxName, &attacher, 0, 0, 0);
							if (spray)
								spray->m_bActive = fxMode == 1;
							else
								groups.push_back(0);
						}
						if (groups.size() != 0)
						{
							for (unsigned int n = 0; n < groups.size(); n++)
								m_pFxInterpreter->Attach(name2, fxName, &attacher, FxInterpreter::eET_SPRAY, true, groups[n], 0,
									false);
						}
					}
					else
					{
						// 983: a sequencia achada fica com +0x1108 = (i == 7), ou seja sempre desligada (i <= 4)
						if (groups.size() != 0)
						{
							for (it = groups.begin(); it != groups.end();)
							{
								CFxSequence* seq =
									(CFxSequence*)m_pFxInterpreter->FindSequence(name2, fxName, &attacher, *it, 0, 0);
								if (seq)
								{
									if (*((bool*)seq + VC10_FXSEQ_ACTIVE) != (i == 7))
										*((bool*)seq + VC10_FXSEQ_ACTIVE) = i == 7;
									it = groups.erase(it);
								}
								else
									++it;
							}
						}
						else
						{
							CFxSequence* seq =
								(CFxSequence*)m_pFxInterpreter->FindSequence(name2, fxName, &attacher, 0, 0, 0);
							if (seq)
							{
								if (*((bool*)seq + VC10_FXSEQ_ACTIVE) != (i == 7))
									*((bool*)seq + VC10_FXSEQ_ACTIVE) = i == 7;
							}
							else
								groups.push_back(0);
						}
						if (groups.size() != 0)
						{
							for (unsigned int n = 0; n < groups.size(); n++)
								m_pFxInterpreter->Attach(name2, fxName, &attacher, FxInterpreter::eET_SEQUENCE, true, groups[n], 0,
									false);
						}
					}
				}
			}
		}
		else if (strcmp(s_scriptCmd, "*hidebone(") == 0)
		{
			char bone[0x40] = { 0 };
			int i = 0;
			do
			{
				p = GetStrConst(p, bone);
				rest = p;
				if (strlen(bone) == 0)
					break;
				if (strlen(bone) >= 0x40)
					break;
				WBone* pBone;
				if (m_type == EXHIBIT_CHARACTER)
					pBone = m_pPetFrame->GetBonePet()->FindBone(bone);
				else
					pBone = m_pPuppet->FindBone(bone);
				if (pBone)
					pBone->SetInvisible(false, true);
				if (!p)
					return;
				if (*p == ')')
					break;
			} while (++i < 0x40);
		}
		else if (strcmp(s_scriptCmd, "*snd(") == 0)
		{
			rest = GetStrConst(p, s_scriptName);
			if (strlen(s_scriptName) == 0)
				return;
			if (strlen(s_scriptName) >= 0x40)
				return;
			g_audio->PlaySfx(s_scriptName);
		}
		else if (strcmp(s_scriptCmd, "*ptex(") == 0)
		{
			name1[0] = '\0';
			p = GetStrConst(p, name1);
			if (strlen(name1) == 0)
				return;
			if (strlen(name1) >= 0x40)
				return;
			if (name1[0] == '@')
			{
				if (!m_pMotion)
					return;
				if (stricmp(name1 + 1, m_pMotion->name) != 0)
					return;
				p = GetStrConst(p, s_scriptName);
				if (strlen(s_scriptName) == 0)
					return;
				if (strlen(s_scriptName) >= 0x40)
					return;
			}
			else
			{
				strcpy(s_scriptName, name1);
			}
			rest = GetStrConst(p, tex2);
			if (strlen(tex2) == 0)
				return;
			if (strlen(tex2) >= 0x40)
				return;
			if (m_bChatFacial)
				return;
			SetPendingTexPart(s_scriptName, tex2);
		}
		else if (strcmp(s_scriptCmd, "*playif(") == 0)
		{
			int off = 0;
			motion[0] = '\0';
			cond[0] = '\0';
			p = GetStrConst(p, cond);
			rest = GetStrConst(p, motion);
			if (cond[0] == '\0')
				return;
			if (motion[0] == '\0')
				return;
			for (;;)
			{
				char* segStart = cond + off;
				char* semi = strchr(segStart, ';');
				int segLen;
				if (!semi)
				{
					segLen = (int)strlen(segStart);
					if (segLen == 0)
						goto next;
				}
				else
					segLen = semi - cond - off;
				strncpy(seg, segStart, segLen);
				seg[segLen] = '\0';
				strcat(seg, ".mpet");
				// 983: na pagina inicial ("TOPPAGE") o boneco 0x4000005 nao troca de movimento
				if (m_pPetFrame && m_pPetFrame->HasMPet(seg) &&
					(!g_pFresh->IsCurrentLayout("TOPPAGE") || m_pPetFrame->m_tid != 0x4000005))
					break;
				if (!semi)
					goto next;
				off += segLen + 1;
			}
			SetMotion(motion, false, 0.0f);
		}
		else if (stricmp(s_scriptCmd, "*showvis(") == 0)
		{
			ParseScript_ShowVis(true, p);
		}
		else if (stricmp(s_scriptCmd, "*hidevis(") == 0)
		{
			ParseScript_ShowVis(false, p);
		}
		else if (strcmp(s_scriptCmd, "*give") == 0)
		{
			FrWnd* pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"bongdari_shop");
			FrBongdariShopDlg* pShop = NULL;
			if (pWnd)
				pShop = (FrBongdariShopDlg*)pWnd;
			else
				__rtti_obj = NULL;
			if (pShop)
			{
				w_bound_box* bb = m_pPuppet->m_bbList.Find("out");
				if (bb)
					pShop->CreateBonusBall(bb->info->spherePivot);
			}
		}
		else if (strcmp(s_scriptCmd, "*petbillboard") == 0)
		{
			m_bPetBillboard983 = true; // 983 00876407
		}
next:
		script = rest;
	} while (script);
}

void CExhibition::ParseScript_ShowVis(bool bShow, const char* script)
{
	char name[256] = { 0 };
	for (int i = 0; i < 0x40; ++i)
	{
		script = GetStrConst(script, name);
		if (strlen(name) == 0)
			return;
		if (strlen(name) >= 0x100)
			return;
		std::list<std::string>* pGroup =
			((CSharedDocVis*)Doc())->GetVisGroup(name);
		if (pGroup)
		{
			std::list<std::string>::iterator it = pGroup->begin();
			if (it != pGroup->end())
			{
				do
				{
					WBone* bone =
						m_pPetFrame->GetBonePet()->FindBone((*it).c_str());
					if (bone)
						bone->SetInvisible(false, !bShow);
					++it;
				} while (it != pGroup->end());
			}
		}
		if (!script)
			return;
		if (*script == ')')
			return;
	}
}

bool CExhibition::InValidArea()
{
	WVector mouse = CInputManager::Instance()->GetMousePoint();
	if (m_areaW > 0.0f && m_areaH > 0.0f)
	{
		float dx = Abs<float>(mouse.x - m_center.x);
		if (dx < m_areaW * 0.5f)
		{
			float dy = Abs<float>(mouse.y - m_center.y);
			if (dy < m_areaH * 0.5f)
				return true;
		}
	}
	else if (m_areaRadius > 0.0f)
	{
		float dy = mouse.y - m_center.y;
		float dx = mouse.x - m_center.x;
		if (dy * dy + dx * dx < m_areaRadius * m_areaRadius)
			return true;
	}
	return false;
}

// 983 00873e30: campo de visao fixo (pi/2), escala pela altura de 600, sem as asas do 645; liga os efeitos no fim
void CExhibition::SetCenter(float x, float y)
{
	m_center.x = x;
	m_center.y = y;
	m_fxBox.SetScreenCenter(x, y);
	float oldFov = g_view->GetFOV();
	g_view->SetFOV(1.5707964f);
	float scale = (float)(m_areaH * m_scale / 600.0);
	float t = (float)tan(g_view->GetFOV() * 0.5);
	m_modelMat.pivot = WVector(0.0f, -((t * m_distance + t * m_distance) * scale), 0.0f);
	g_view->SetFOV(oldFov);
	AttachFx();
}

const WVector2D& CExhibition::GetCenter()
{
	return m_center;
}

void CExhibition::SetArea(float radius)
{
	m_areaW = 0.0f;
	m_areaH = 0.0f;
	m_areaRadius = radius;
}

void CExhibition::SetArea(float w, float h)
{
	m_areaRadius = 0.0f;
	m_areaW = w;
	m_areaH = h;
}

// 983 00876b90: + troca do nome pelo CMotionAdvisor (efeito de conjunto; B110.1 b) e a condicao do FxInterpreter
bool CExhibition::SetMotion(const char* name, bool bContinue, float time)
{
	if (!bContinue)
	{
		m_fxBox.ClearAll(false);
		AttachFx();
	}
	if (m_type == EXHIBIT_CHARACTER)
	{
		if (!m_pPetFrame)
			return false;
		const char* motion = name;
		if (CMotionAdvisor::Instance())
		{
			const char* advised = CMotionAdvisor::Instance()->GetMotionName(m_uid, m_charIndex, name);
			if (advised)
				motion = advised;
		}
		m_pMotion = m_pPetFrame->FindMotionData(motion, true, NULL);
		m_pPetFrame->SetInvisible(false);
	}
	else
	{
		if (!m_pPuppet)
			return false;
		if (!m_pPuppet->m_rootbone)
			return false;
		m_pMotion = m_pPuppet->FindMotionData(name);
		m_pPuppet->m_rootbone->SetInvisible(true, false);
	}
	m_pFxInterpreter->SetCond(name, 0);
	if (m_pMotion)
	{
		ScriptProcess(m_pMotion, m_pMotion->s, m_pMotion->s, true);
		m_motionTime = Min(time + m_pMotion->s, m_pMotion->e);
		UpdateCamera(0.0f);
		Animate();
		return true;
	}
	return false;
}

// 983 00873920: o teste "sem movimento" fica fora de linha
__declspec(noinline) static bool IsNoMotion(int motion)
{
	return motion == -1;
}

const char* CExhibition::GetNextCharMotion(int index)
{
	m_motionCursor++;
	m_motionCursor %= 12;
	int motion = s_charMotionTable[index][m_motionCursor];
	if (IsNoMotion(motion))
	{
		m_motionCursor = 0;
		motion = s_charMotionTable[index][0];
	}
	return s_charMotions[motion];
}

const char* CExhibition::GetNextCaddieMotion(int index)
{
	if (index >= 28)
		index = 27;
	m_motionCursor = (m_motionCursor + 1) % 9;
	if (s_caddieMotionTable[index][m_motionCursor] == -1)
	{
		m_motionCursor = 0;
		return s_caddieMotions[s_caddieMotionTable[index][0]];
	}
	return s_caddieMotions[s_caddieMotionTable[index][m_motionCursor]];
}

w_motion_data* CExhibition::GetMotionData()
{
	return m_pMotion;
}

bool CExhibition::NextMotion()
{
	for (;;)
	{
		bool bResult;
		switch (m_type)
		{
		case EXHIBIT_CHARACTER:
			bResult = SetMotion(GetNextCharMotion(m_charIndex), false, 0.0f);
			break;
		case EXHIBIT_CADDIE:
			bResult =
				SetMotion(GetNextCaddieMotion(m_caddieIndex), false, 0.0f);
			break;
		case EXHIBIT_MASCOT:
			bResult = SetMotion("Default", false, 0.0f);
			break;
		case EXHIBIT_CLUB:
			bResult = SetMotion("default", false, 0.0f);
			break;
		case EXHIBIT_ITEM:
		case EXHIBIT_CARD:
			// 983 00876ce0: item/carta ignoram o resultado e sempre retornam true
			SetMotion("default", false, 0.0f);
			return true;
		case EXHIBIT_BALL:
		case EXHIBIT_MATCH:
		case EXHIBIT_FURNITURE:
		case EXHIBIT_FURNITURE_BASE:
		case EXHIBIT_FURNITURE_WALL:
		case EXHIBIT_FURNITURE_FLOOR:
		case EXHIBIT_FURNITURE_ROOM:
		case EXHIBIT_FURNITURE_TEXTURE:
			return true;
		default:
			continue;
		}
		if (bResult)
			return bResult;
	}
}

// 983 00876d90 (corpo comum das derivadas): o modo vai ao 4o argumento do Build (6 no 983); sem o ParseBoundingBoxScript
// do 645 no fim (os efeitos entram pelo SetMotion -> AttachFx); falhou -> m_pPetFrame = NULL
bool CExhibition::SetModelTnL(bool bHardwareTnL, CPartTidList& tids, unsigned long* uids,
	const char* motion, float delay, int option)
{
	m_motionCursor = -1;
	m_tid = tids.m_charTid;
	m_pUids = uids;
	ReleasePet();
	m_type = EXHIBIT_CHARACTER;
	m_light.ambient = 0x969696;
	m_pPetFrame = new CPetFrame;
	// 983: a falha vem primeiro; Random com a semente padrao (-1; era 0); o motivo vai direto no SetMotion
	if (!m_pPetFrame->Build(&tids, uids, true, bHardwareTnL, option, 0))
	{
		if (m_pPetFrame)
		{
			delete m_pPetFrame;
			m_pPetFrame = NULL;
		}
		return false;
	}
	m_pPuppet = m_pPetFrame->GetBonePet();
	m_partTids = tids;
	// 983: WMatrix::IDENTITY (00e42574) direto para o boneco (era ApplyBones(NULL, identidade))
	m_pPuppet->SetMatrix(WMatrix::IDENTITY);
	m_pPuppet->Transform(NULL, 1.0f, 0);
	m_pPuppet->UpdateBound(false, false);
	SetScreenPos(m_pPuppet);
	m_charIndex = m_pPetFrame->m_tid & 0x3ffffff;
	SetMotion(motion ? motion : GetNextCharMotion(m_charIndex), false, 0.0f);
	m_idleTimer = Random(0.0f, delay);
	if (m_pMotion)
		m_motionLength = (m_pMotion->e - m_pMotion->s) + m_idleTimer;
	else
		m_motionLength = 0.0f;
	m_alpha = 0.0f;
	m_pPetFrame->SetAlpha(0);
	if (tids.m_charTid == 0x4000005 || tids.m_charTid == 0x4000006)
		m_pitchLimit = -0.2617994f;
	else
		m_pitchLimit = 0.0f;
	m_pPetFrame->Refresh(&tids);
	return true;
}

// HACK: SetModel(tid) stays C, but the original lays each block shared by several cases
// (setmotion, setdefault, getpuppet, return false) after its FIRST predecessor while VC7.1
// always puts it after the last one (label position, duplicated code, case order and
// conditional gotos all tried). The later predecessors' jumps (cases 7, 6, 0x1f, 9) are
// therefore _emit with the original displacements, and case 0xb takes its FindMatch key
// from edx through the selector below (every C form allocated eax). The C is in the #else.

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
bool CExhibition::SetModel(unsigned long tid)
{
	m_motionCursor = -1;
	m_tid = tid;
	m_pUids = NULL;
	ReleasePet();
	const char* model;
	int n;
	switch (m_tid >> 26)
	{
	case 8:
	{
		// 983: o caddie dono da roupa soma os dois campos de 5 bits do tid; o boneco sai da textura do corpo da roupa
		// (roupas com modelo proprio: nomes fixos, maid* pelo caddie 0x1c00001f e pela serie; "..._ex_..." -> "<prefixo>_ex.pet")
		m_type = EXHIBIT_CADDIE;
		m_light.ambient = 0x969696;
		IFF_STRUCT::sCadItem* pCadItem = ItemManager()->FindCadItem(m_tid);
		if (!pCadItem)
			return false;
		IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(
			((pCadItem->c.TypeId >> 0x15 & 0x1f) + (pCadItem->c.TypeId >> 0x10 & 0x1f)) | 0x1c000000);
		if (!pCaddie)
			return false;
		std::string model(pCadItem->BodyTex);
		std::string body;
		std::transform(model.begin(), model.end(), std::back_inserter(body), tolower);
		unsigned int pos = body.find("_ex_", 0, 4);
		bool bOwnerMaid = ((pCadItem->c.TypeId >> 0x15 & 0x1f) + (pCadItem->c.TypeId >> 0x10 & 0x1f) | 0x1c000000) ==
			0x1c00001f;
		bool bFirst = (pCadItem->c.TypeId & 0x1fff) < 3;
		if (body == "caddie_quma_ex_01.jpg")
			model.assign("quma_ex_02.pet", 0xe);
		else if (body == "caddie_dol_ex_01.jpg")
			model.assign("dol_ex_02.pet", 0xd);
		else if (pos != std::string::npos)
		{
			unsigned int i = 0;
			for (; i < body.size(); i++)
			{
				if (body[i] >= 'a' && body[i] <= 'z')
					break;
			}
			model.assign(body.begin() + i, body.begin() + pos + 3);
			model.append(".pet", 4);
		}
		else if (body == "pipin3_bd_01-01.jpg")
			model.assign("pipin_ex_02.pet", 0xf);
		else if (body == "[2#caddie_cadie_004.jpg")
			model.assign("cadie_ex_02.pet", 0xf);
		else if (body == "[2#ho_body_001_02.jpg")
			model.assign("ho_mesh_ex02.pet", 0x10);
		else if (body == "[2#ho_body_002_04.jpg")
			model.assign("ho_mesh_ex03.pet", 0x10);
		else if (body == "caddie_maid_body_001.jpg")
		{
			if (bOwnerMaid)
				model.assign(bFirst ? "maid_1.pet" : "maid_13.pet");
			else
				model.assign(bFirst ? "maid.pet" : "maid_12.pet");
		}
		else if (body == "caddie_maid_body_002.jpg")
		{
			if (bOwnerMaid)
				model.assign(bFirst ? "maid2_1.pet" : "maid2_3.pet");
			else
				model.assign(bFirst ? "maid2.pet" : "maid2_2.pet");
		}
		else if (body == "caddie_maid_body_003.jpg")
		{
			if (bOwnerMaid)
				model.assign(bFirst ? "maid3_1.pet" : "maid3_3.pet");
			else
				model.assign(bFirst ? "maid3.pet" : "maid3_2.pet");
		}
		else
			model.assign(MakeStr("%s_ex.pet", pCaddie->Data));
		m_pPuppet = g_resrcmng->GetPuppet(model.c_str(), false, true, false);
		if (!m_pPuppet)
			return false;
		m_caddieIndex = pCaddie->c.TypeId & 0x3ffffff;
		n = strlen(pCadItem->FaceTex);
		if (n != 0)
			SetPendingTexPart("face", pCadItem->FaceTex);
		n = strlen(pCadItem->BodyTex);
		if (n != 0)
			SetPendingTexPart("body", pCadItem->BodyTex);
		SetMotion(GetNextCaddieMotion(m_caddieIndex), false, 0.0f);
		break;
	}
	case 7:
	{
		m_type = EXHIBIT_CADDIE;
		m_light.ambient = 0x969696;
		IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(m_tid);
		if (!pCaddie)
			return false;
		m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pCaddie->Data),
			false, true, false);
		if (!m_pPuppet)
			return false;
		m_caddieIndex = pCaddie->c.TypeId & 0x3ffffff;
		AttachFx(); // 983: era o ParseBoundingBoxScript do 645
		SetMotion(GetNextCaddieMotion(m_caddieIndex), false, 0.0f);
		break;
	}
	case 5:
	{
		m_type = EXHIBIT_BALL;
		m_light.ambient = 0x646464;
		IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(m_tid);
		if (!pBall)
			return false;
		m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pBall->Data), false,
			true, false);
		if (!m_pPuppet)
			return false;
		switch (m_tid)
		{
		case 0x14000005:
		case 0x1400001a:
		case 0x14000080:
			AttachBallFx983(); // 983 008785f3 -> 00875310 (nao o AttachFx 00873a30)
		}
		break;
	}
	case 4:
	{
		m_type = EXHIBIT_CLUB;
		m_light.ambient = 0x646464;
		IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(m_tid);
		if (!pClub)
			return false;
		m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s_ui.pet", pClub->c.Icon),
			false, true, false);
		if (!m_pPuppet)
			return false;
setdefault:
		AttachFx(); // 983: era o ParseBoundingBoxScript do 645
		SetMotion("default", true, 0.0f);
		break;
	}
	case 0xb:
	{
		// 983: com os dois bits 0x3000000 ligados o tid inteiro e a chave e o modelo e o 1o
		unsigned long matchId = tid;
		unsigned long icon = 0;
		if ((tid & 0x3000000) != 0x3000000)
		{
			icon = tid & 0xff;
			matchId = tid & 0xffffff00;
		}
		m_type = EXHIBIT_MATCH;
		m_light.ambient = 0x646464;
		IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(matchId);
		if (!pMatch)
			return false;
		m_pPuppet = g_resrcmng->GetPuppet(
			MakeStr("%s.pet", pMatch->Icon[icon]), false, true, false);
		if (!m_pPuppet)
			return false;
		break;
	}
	case 0x10:
	{
		m_type = EXHIBIT_MASCOT;
		m_light.ambient = 0x969696;
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(m_tid);
		if (pMascot)
		{
			m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pMascot->Data),
				false, true, false);
			jp983_rastro("D319 Exhibit mascote tid=%08x data=%s puppet=%p", m_tid, (const char*)pMascot->Data, m_pPuppet);
			if (m_pPuppet)
			{
				AttachFx(); // 983: era o ParseBoundingBoxScript do 645
				bool motionOk = SetMotion("Default", false, 0.0f);
				jp983_rastro("D340 Exhibit mascote motion=%d pMotion=%p root=%p fx=%p", (int)motionOk, m_pMotion, m_pPuppet->m_rootbone, m_pFxInterpreter);
				typedef std::map<unsigned int, sMascotInfo> MascotMap;
				MascotMap& mascots = Doc()->m_mascotMap;
				n = 0;
				MascotMap::const_iterator it = mascots.begin();
				while (it != mascots.end())
				{
					if ((*it).second.tid == m_tid)
					{
						pMascot = ItemManager()->FindMascot(m_tid);
						if (pMascot && pMascot->ShowMsg)
							m_msgTexture = CMascot::ChangeMascotMsgInTex(
								(*it).second.szMsg, pMascot, m_pPuppet,
								MakeStr("ui%02d", n));
						break;
					}
					++n;
					++it;
				}
				break;
			}
		}
		return false;
	}
	case 6:
	{
		m_type = EXHIBIT_ITEM;
		m_light.ambient = 0x646464;
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(m_tid);
		if (!pItem)
			return false;
		model = pItem->Data;
getpuppet:
		m_pPuppet =
			g_resrcmng->GetPuppet(MakeStr("%s.pet", model), false, true, false);
		if (!m_pPuppet)
			return false;
		goto setdefault;
	}
	case 0x1f:
	{
		m_type = EXHIBIT_CARD;
		m_light.ambient = 0xc8c8c8;
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_tid);
		if (!pCard)
			return false;
		unsigned long kind = m_tid >> 0x16 & 0xf;
		if (kind == 3 || kind == 4)
		{
			m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s.pet", pCard->Image),
				false, true, false);
		}
		if (!m_pPuppet)
			return false;
		goto setdefault;
	}
	case 9:
	{
		unsigned long kind = m_tid >> 0x15 & 0x1f;
		if (kind != 6 && kind != 5 && kind != 4 && kind != 8)
			break;
		m_type = EXHIBIT_ITEM;
		m_light.ambient = 0x646464;
		IFF_STRUCT::sSetItem* pSet = ItemManager()->FindSetItem(m_tid);
		if (!pSet)
			return false;
		model = pSet->c.Icon;
		goto getpuppet;
	}
	case 0x12:
	{
		if (!IsLocalContent(S4_REAL_MYROOM))
			break;
		m_light.ambient = 0x989898;
		IFF_STRUCT::sFurniture* pFurn = ItemManager()->FindFurniture(m_tid);
		if (!pFurn)
			return false;
		unsigned long kind = pFurn->c.TypeId >> 0xb & 0x7ff;
		if (kind == 8)
			m_type = EXHIBIT_FURNITURE_ROOM;
		else if (kind == 9)
			m_type = EXHIBIT_FURNITURE_FLOOR;
		else if (kind == 0xe)
			m_type = EXHIBIT_FURNITURE_TEXTURE;
		else if (pFurn->IsMove == 5)
			m_type = EXHIBIT_FURNITURE_BASE;
		else if (pFurn->IsMove == 6)
			m_type = EXHIBIT_FURNITURE_WALL;
		else
			m_type = EXHIBIT_FURNITURE;
		if (m_type == EXHIBIT_FURNITURE_WALL ||
			m_type == EXHIBIT_FURNITURE_FLOOR ||
			m_type == EXHIBIT_FURNITURE_ROOM)
		{
			m_pPuppet = g_resrcmng->GetPuppet(MakeStr("%s_ui.pet", pFurn->Data),
				false, true, false);
		}
		else
		{
			if (m_type == EXHIBIT_FURNITURE_TEXTURE)
			{
				pFurn = ItemManager()->FindFurniture(m_tid);
				if (!pFurn)
					return false;
				m_pPuppet = g_resrcmng->GetPuppet(
					MakeStr("%s.pet", pFurn->Data), false, true, false);
				if (!m_pPuppet)
					return false;
				for (int i = 0; i < 3; ++i)
				{
					if (strlen(pFurn->OrgTex[i]) != 0 &&
						strlen(pFurn->Tex[i]) != 0)
						SetPendingTexPart(MakeStr("%s.jpg", pFurn->OrgTex[i]),
							MakeStr("%s.jpg", pFurn->Tex[i]));
				}
			}
			else
			{
				m_pPuppet = g_resrcmng->GetPuppet(
					MakeStr("%s.pet", pFurn->Data), false, true, false);
			}
		}
		if (!m_pPuppet)
			return false;
		AttachFx(); // 983: era o ParseBoundingBoxScript do 645
		break;
	}
	}
	// 983: WMatrix::IDENTITY (00e42574) direto para o boneco (era ApplyBones(NULL, identidade)); a duracao soma 0.0 em double
	m_pPuppet->SetMatrix(WMatrix::IDENTITY);
	m_pPuppet->Transform(NULL, 1.0f, 0);
	m_pPuppet->UpdateBound(false, false);
	SetScreenPos(m_pPuppet);
	m_idleTimer = 0.0f;
	if (m_pMotion)
		m_motionLength = (float)(m_pMotion->e - m_pMotion->s + 0.0);
	else
		m_motionLength = 0.0f;
	m_alpha = 0.0f;
	m_pPuppet->SetAlpha(0);
	return true;
}

bool CExhibition::SetModel(const char* name, eExhibitType type)
{
	m_motionCursor = -1;
	m_tid = 0;
	ReleasePet();
	switch (type)
	{
	case EXHIBIT_CARD:
		m_type = EXHIBIT_CARD;
		m_light.ambient = 0xffffff;
		AttachFx(); // 983: era o ParseBoundingBoxScript do 645
		m_pPuppet =
			g_resrcmng->GetPuppet(MakeStr("%s.pet", name), false, true, false);
		break;
	case EXHIBIT_BALL:
		m_type = EXHIBIT_BALL;
		m_light.ambient = 0xc8c8c8;
		m_pPuppet =
			g_resrcmng->GetPuppet(MakeStr("%s.pet", name), false, true, false);
		break;
	case EXHIBIT_CADDIE:
		m_type = EXHIBIT_CADDIE;
		m_light.ambient = 0x969696;
		m_pPuppet =
			g_resrcmng->GetPuppet(MakeStr("%s.pet", name), false, true, false);
		AttachFx(); // 983: era o ParseBoundingBoxScript do 645
		SetMotion("\xb1\xe2\xba\xbb", false, 0.0f);
		break;
	}
	// 983 00876f90: WMatrix::IDENTITY direto para o boneco; a duracao soma 0.0 em double (como o SetModel(tid))
	m_pPuppet->SetMatrix(WMatrix::IDENTITY);
	m_pPuppet->Transform(NULL, 1.0f, 0);
	m_pPuppet->UpdateBound(false, false);
	SetScreenPos(m_pPuppet);
	m_idleTimer = 0.0f;
	if (m_pMotion)
		m_motionLength = (float)(m_pMotion->e - m_pMotion->s + 0.0);
	else
		m_motionLength = 0.0f;
	m_alpha = 0.0f;
	m_pPuppet->SetAlpha(0);
	return true;
}

// 983 00873a30: os efeitos do boneco vem do FxInterpreter (caixas *fx( do CPetFrame); liga os pendentes "daecheonsa" e
// "yojeong nalgae" (asas, cp949). O 645 lia o efeito da bola (sBall::Fx) aqui.
void CExhibition::AttachFx()
{
	if (!m_pPuppet)
		return;
	m_pFxInterpreter->DetachFxAll(-1);
	m_pFxInterpreter->ParsePetFrameScript_fx(m_pPetFrame, &m_center.x, m_type == EXHIBIT_CHARACTER, 0);
	m_pFxInterpreter->ShowNamed("\xb4\xeb\xc3\xb5\xbb\xe7", true, 0);
	m_pFxInterpreter->ShowNamed("\xbf\xe4\xc1\xa4\xb3\xaf\xb0\xb3", true, 0);
}

// 983 00875310: o efeito da bola (Fx[0] no osso FxBone[0] ou na caixa "center") que o 645 lia no AttachFx
void CExhibition::AttachBallFx983()
{
	IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(m_tid);
	if (!pBall)
		return;
	char* fx = pBall->Fx[0];
	int len = (int)strlen(fx);
	if (len < 5)
		return;
	Attacher attacher;
	char* boneName = pBall->FxBone[0];
	if (!boneName[0])
	{
		boneName = "center";
	}
	else
	{
		WBone* bone = m_pPuppet->FindBone(boneName);
		if (bone)
		{
			attacher.mat[0] = &bone->m_matrix;
			goto attach;
		}
		if (!boneName[0])
			boneName = "center";
	}
	{
		w_bound_box* bb = m_pPuppet->m_bbList.Find(boneName);
		if (bb)
		{
			if (bb->bone)
				attacher.mat[0] = &bb->bone->m_matrix;
			attacher.pos[0] = &bb->info->spherePivot;
		}
	}
attach:
	attacher.x = m_center.x;
	attacher.y = m_center.y;
	if (stricmp(fx + len - 3, "spr") == 0)
	{
		CFxSpray* spray = m_fxBox.LoadSpray(fx, NULL);
		if (spray)
		{
			((SprayAttach*)spray)->pAttacher = &attacher;
			((SprayAttach*)spray)->attachX = attacher.x;
			((SprayAttach*)spray)->attachY = attacher.y;
			memcpy(((SprayAttach*)spray)->attachMat, attacher.mat, sizeof(attacher.mat));
			memcpy(((SprayAttach*)spray)->attachPos, attacher.pos, sizeof(attacher.pos));
			m_fxBox.AddSpray(spray);
			return;
		}
	}
	else
	{
		CFxSequence* seq = m_fxBox.LoadSequence(fx, true, NULL);
		if (seq)
		{
			((SeqAttach*)seq)->pAttacher = &attacher;
			((SeqAttach*)seq)->attachX = attacher.x;
			((SeqAttach*)seq)->attachY = attacher.y;
			m_fxBox.AddSequence(seq);
		}
	}
}

// 983 008754a0: FOV fixo de 90° (o SetFOV do 983 já corrige a razão da tela) e distâncias medidas em 600 de altura
void CExhibition::SetScreenPos(WPuppet* pPuppet)
{
	if (!pPuppet || !g_view)
		return;
	WMatrix cam;
	cam.Reset();
	cam.zm = -m_distance * g_view->SCREEN_YS / 600.0;
	float oldFov = g_view->GetFOV();
	int type = m_type;
	if (type == EXHIBIT_CHARACTER || type == EXHIBIT_CADDIE)
	{
		WBone* head = pPuppet->FindBone("Bip01 Head");
		WBone* foot = pPuppet->FindBone("Bip01 Footsteps");
		WVector headPos;
		WVector footPos;
		if (!head || !foot)
		{
			w_bound_box* bb = pPuppet->m_bbList.Find("center");
			if (!bb)
				return;
			headPos = WVector(0.0f,
				(bb->info->aabb.max.y - bb->info->aabb.min.y) * 0.75f, 0.0f);
			footPos = WVector::ZERO;
		}
		else
		{
			headPos = head->m_matrix.pivot;
			footPos = foot->m_matrix.pivot;
		}
		g_view->SetFOV(1.5707964f);
		g_view->SetCamera(cam);
		g_view->UpdateCamera();
		WVector projHead = g_view->Projection(headPos);
		WVector projFoot = g_view->Projection(footPos);
		float dy = Abs<float>(projHead.y - projFoot.y);
		float ratio2 = (m_areaH * 0.75f) / dy;
		float scale;
		if (m_areaH * 0.75f <= dy)
			scale = 1.0f - (1.0f - ratio2) * 0.5f;
		else
			scale = (ratio2 - 1.0f) * 0.5f + 1.0f;
		if (0.5f <= scale)
		{
			if (1.2f < scale)
				scale = 1.2f;
		}
		else
			scale = 0.5f;
		m_modelMat.Reset();
		m_modelMat *= scale;
	}
	else
	{
		g_view->SetFOV(1.5707964f);
		m_modelMat.Reset();
	}
	m_targetDistance = m_distance;
	m_defaultDistance = m_distance;
	float k = m_areaH * m_scale / 600.0;
	float t = tanf(g_view->GetFOV() * 0.5);
	float h = m_distance * t * 2.0f;
	m_modelMat.pivot = WVector(0.0f, -h * k, 0.0f);
	g_view->SetFOV(oldFov);
	g_view->UpdateCamera();
	type = m_type;
	if (type == EXHIBIT_CHARACTER || type == EXHIBIT_CADDIE)
	{
		m_defaultPitch = 6.25319f;
	}
	else
	{
		if (type != EXHIBIT_FURNITURE && type != EXHIBIT_FURNITURE_BASE &&
			type != EXHIBIT_FURNITURE_WALL)
			goto done;
		m_defaultPitch = 5.85319f;
	}
	m_defaultYaw = 0.33f;
done:
	m_pitch = m_defaultPitch;
	m_yaw = m_defaultYaw;
	m_lastYaw = m_defaultYaw;
}

void CExhibition::SetAlpha(unsigned char alpha)
{
	if (m_type == EXHIBIT_CHARACTER)
	{
		if (m_pPetFrame)
			m_pPetFrame->SetAlpha(alpha);
	}
	else if (m_pPuppet)
		m_pPuppet->SetAlpha(alpha);
}

// 983: sem ParseBoundingBoxScript (os *fx( das caixas sao do FxInterpreter: AttachFx)

// 983 00875960: o nome da textura passa antes pelo CGDSkinItem ("TanLines", marcas de bronzeado da roupa; 00544200 com as
// pecas guardadas) e a busca/insercao usa o nome trocado
sTexturePart* CExhibition::FindTexturePart(char* primary, char* secondary)
{
	sTexturePart* pPart;
	CGDSkinItem* pSkinItem = CGDSkinItemPetJp::Instance983();
	std::string texture;
	pSkinItem->Replace(primary, secondary, &texture, (const sSkinParts983*)NULL);
	std::list<sTexturePart*>::iterator it;
	it = std::find_if(m_texParts.begin(), m_texParts.end(),
		sFindTexPart(texture.c_str()));
	if (it != m_texParts.end())
	{
		pPart = *it;
	}
	else
	{
		pPart = new sTexturePart;
		strcpy(pPart->name, primary);
		strcpy(pPart->texture, texture.c_str());
		m_texParts.push_back(pPart);
	}
	return pPart;
}

void CExhibition::SetPendingTexPart(char* primary, const char* secondary)
{
	sTexturePart* pPart = FindTexturePart(primary, (char*)secondary);
	if (!pPart)
		return;
	if (m_type == EXHIBIT_CHARACTER)
	{
		if (!m_pPetFrame)
			return;
		if (pPart->pBitmap == NULL)
		{
			int texture = m_pPetFrame->m_skinTexture;
			m_pPetFrame->GetBonePet()->ChangeTexturePart(texture,
				pPart->texture, &pPart->pBitmap, &pPart->rect);
		}
		else
		{
			int texture = m_pPetFrame->m_skinTexture;
			m_pPetFrame->GetBonePet()->ChangeTexturePart(texture,
				pPart->pBitmap, pPart->rect);
		}
		m_pPetFrame->SetMakeup();
	}
	else if (m_type == EXHIBIT_FURNITURE_TEXTURE)
	{
		if (!m_pPuppet)
			return;
		int partHandle = g_resrcmng->LoadTexture(primary, 0, 0, NULL);
		if (!partHandle)
			return;
		int texHandle = g_resrcmng->LoadTexture(secondary, 0, 0, NULL);
		if (!texHandle)
			return;
		m_pPuppet->ChangeTextureByHandle(texHandle, partHandle, m_pPuppet);
	}
	else
	{
		if (!m_pPuppet)
			return;
		if (pPart->pBitmap == NULL)
			m_pPuppet->ChangeTexturePart(pPart->name, pPart->texture,
				&pPart->pBitmap, &pPart->rect);
		else
			m_pPuppet->ChangeTexturePart(pPart->name, pPart->pBitmap,
				pPart->rect);
	}
}

void CExhibition::SetChatFacial(int facial, unsigned long type)
{
	// 983 00875c30: 15 personagens e a tabela no objeto global de 00e73e84 (era s_chatFacials[10][4])
	unsigned long index = type & 0x3ffffff;
	if ((unsigned int)facial <= 3 && index < 15)
	{
		m_bChatFacial = true;
		SetPendingTexPart("face", CreateCharacterOption::Instance()->GetChatFacial983(index, facial));
	}
}

// 983 00873aa0: as asas sao o pendente "angeliceffect" do FxInterpreter (o 645 carregava High-Rank.seq nos dedos)
void CExhibition::AttachAngelWingFx()
{
	m_pFxInterpreter->ShowNamed("angeliceffect", true, -1);
}

bool CExhibition::ChangeBoneTexture(const char* bone, const char* texPart,
	const char* texName)
{
	if (0) // HACK: emission order (original emits it after the CInputManager::Instance users)
		CInputManager::Instance();
	w_texlist* pTex = g_resrcmng->FindTexture(texPart);
	WBone* pBone;
	if (pTex && m_pPuppet && (pBone = m_pPuppet->m_rootbone))
	{
		int handle = g_resrcmng->LoadTexture(texName, 0x80000000, 0, NULL);
		return pBone->ChangeTexture(bone, pTex->texhandle, handle);
	}
	return false;
}

bool CExhibition::ChangeBoneTexture(const char* bone, const char* texName)
{
	if (0) // HACK: emission order (original emits it after the CInputManager::Instance users)
		CInputManager::Instance();
	WBone* pBone;
	if (m_pPuppet && (pBone = m_pPuppet->m_rootbone))
	{
		int handle = g_resrcmng->LoadTexture(texName, 0x80000000, 0, NULL);
		return pBone->ChangeTexture(bone, handle);
	}
	return false;
}

void CExhibition::DisplayClip(const WRect& clip, float fov, float nearClip,
	float farClip)
{
	bool bFog = g_view->SetFogEnable(false);
	float oldNear = g_view->clip_near;
	float oldFar = g_view->clip_far;
	float oldFov = g_view->GetFOV(); // 983 008743c0: GetFOV/SetFOV de verdade, sem a razão da tela
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	m_pPuppet->UpdateBSphere();
	WSphere* pSphere = &m_pPuppet->m_boundSphere;
	float dx = m_camera.pivot.x - pSphere->pos.x;
	float dy = m_camera.pivot.y - pSphere->pos.y;
	float dz = m_camera.pivot.z - pSphere->pos.z;
	float dist = (float)sqrt(dx * dx + dy * dy + dz * dz);
	float radius =
		(float)sqrt(m_modelMat.zx * m_modelMat.zx +
			m_modelMat.yx * m_modelMat.yx + m_modelMat.xx * m_modelMat.xx) *
		pSphere->radius;
	float nearV = dist - radius;
	if (nearV <= 0.5)
		nearV = 0.5f;
	float farV = Min((float)(nearV + 100.0), radius + dist); // 983 008743c0: perto + 100 (o 645 usa 100 fixo)
	if (nearClip > 0.0f)
		nearV = nearClip;
	if (farClip > nearClip)
		farV = Max(farClip, nearV);
	g_view->SetProjectionMode(WView::PERSPECTIVE); // 983 008743c0: perspectiva durante o desenho
	g_view->SetClip(nearV, farV, false);
	g_view->SetCamera(m_camera);
	g_view->SetClippingArea(clip);
	g_view->SetFOV(fov * g_DEGTORAD);
	g_view->SetScreenCenter(m_center.x, m_center.y);
	g_view->UpdateCamera();
	m_pPuppet->UpdateLightSource(&m_light, false, NULL);
	m_pPuppet->Render(g_view, true, 1.0f, false, false, false);
	g_view->Flush(1, 1);
	g_view->SetProjectionMode(oldMode);
	g_view->SetCamera(g_camera);
	g_view->SetClip(oldNear, oldFar, false);
	g_view->SetFOV(oldFov);
	WRect rc;
	rc.w = g_view->SCREEN_XS;
	rc.h = g_view->SCREEN_YS;
	rc.x = 0.0f;
	rc.y = 0.0f;
	g_view->SetClippingArea(rc);
	g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5f, g_view->SCREEN_YS * 0.5f);
	g_view->UpdateCamera();
	g_view->SetFogEnable(bFog);
}


// 983 00873f10 (sem par no 645/839 por nome; par 839 0086e430): desenha o modelo com a camera dada (a FrBongdariShopDlg passa
// a dela em +0xa8c, ou a da CCameraWork +0x120), sem neblina, em perspectiva; em telas com 720 linhas ou mais recua a camera
// ((YS - 720) / 13,6 + 9 unidades para tras). Repoe clip, FOV, area e modo; nao repoe a camera (o 983 tambem nao).
void CExhibition::DisplayCamera983(bool, float nearClip, float farClip, float fov, const WRect& clip, WMatrix camera)
{
	if (g_view->SCREEN_YS >= 720.0f)
	{
		float bx = (float)(camera.za.x * -1.0);
		float by = (float)(camera.za.y * -1.0);
		float bz = (float)(camera.za.z * -1.0);
		float k = (float)(g_view->SCREEN_YS - 720.0);
		k = (float)(k / (double)13.6f);
		k = (float)(k + 9.0);
		float ox = bx * k;
		float oy = by * k;
		float oz = bz * k;
		camera.pivot.x += ox;
		camera.pivot.y += oy;
		camera.pivot.z += oz;
	}
	bool bFog = g_view->SetFogEnable(false);
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float oldNear = g_view->clip_near;
	float oldFar = g_view->clip_far;
	float oldFov = g_view->GetFOV();
	m_pPuppet->UpdateBSphere();
	WSphere* pSphere = &m_pPuppet->m_boundSphere;
	float dx = camera.pivot.x - pSphere->pos.x;
	float dy = camera.pivot.y - pSphere->pos.y;
	float dz = camera.pivot.z - pSphere->pos.z;
	float dist = (float)sqrt(dx * dx + dy * dy + dz * dz);
	float radius =
		(float)sqrt(m_modelMat.zx * m_modelMat.zx +
			m_modelMat.yx * m_modelMat.yx + m_modelMat.xx * m_modelMat.xx) *
		pSphere->radius;
	float nearV = dist - radius;
	if (nearV <= 0.5)
		nearV = 0.5f;
	float farV = (float)(nearV + 100.0); // 983: perto + 100 (o DisplayClip do 645 usa 100 fixo)
	if (radius + dist < farV)
		farV = radius + dist;
	if (nearClip > 0.0f)
		nearV = nearClip;
	if (farClip > nearClip)
		farV = Max(farClip, nearV);
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetClip(nearV, farV, false);
	g_view->SetCamera(camera);
	g_view->SetFOV((float)(fov * (double)0.017453292f)); // 983 00873f10: fmul double 0.01745329238 (grau em float)
	g_view->SetClippingArea(clip);
	g_view->SetScreenCenter(m_center.x, m_center.y);
	g_view->UpdateCamera();
	if (m_pPuppet)
	{
		m_pPuppet->UpdateLightSource(&m_light, false, NULL);
		m_pPuppet->Render(g_view, true, 1.0f, false, false, false);
	}
	m_fxBox.Display(g_view);
	g_view->Flush(1, 1);
	g_view->SetProjectionMode(oldMode);
	g_view->SetClip(oldNear, oldFar, false);
	WRect rc;
	rc.x = 0.0f;
	rc.y = 0.0f;
	rc.w = g_view->SCREEN_XS;
	rc.h = g_view->SCREEN_YS;
	g_view->SetClippingArea(rc);
	g_view->SetFOV(oldFov);
	g_view->SetScreenCenter((float)(0.5 * g_view->SCREEN_XS), (float)(g_view->SCREEN_YS * 0.5));
	g_view->UpdateCamera();
	g_view->SetFogEnable(bFog);
}

// 983 00874af0 (corpo comum das derivadas): sempre pela esfera do boneco (o 645 fixava 0.5-10 para bola/taco/item), longe =
// min(dist + raio, perto + 100), perspectiva e campo de visao pi/2 durante o desenho; o modo vai ao ultimo argumento do Render
void CExhibition::DisplayTnL(bool bHardwareTnL, float nearClip, float farClip)
{
	bool bFog = g_view->SetFogEnable(false);
	g_view->SetCamera(m_camera);
	float oldNear = g_view->clip_near;
	float oldFar = g_view->clip_far;
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float oldFov = g_view->GetFOV();
	WSphere sphere;
	if (m_pPuppet)
	{
		m_pPuppet->UpdateBSphere();
		sphere = m_pPuppet->m_boundSphere;
	}
	else
	{
		sphere.pos = WVector(0.0f, 0.0f, 0.0f);
		sphere.radius = 0.0f;
	}
	float dx = m_camera.pivot.x - sphere.pos.x;
	float dy = m_camera.pivot.y - sphere.pos.y;
	float dz = m_camera.pivot.z - sphere.pos.z;
	float dist = (float)sqrt(dx * dx + dy * dy + dz * dz);
	float radius =
		(float)sqrt(m_modelMat.zx * m_modelMat.zx +
			m_modelMat.yx * m_modelMat.yx + m_modelMat.xx * m_modelMat.xx) *
		sphere.radius;
	float nearV = dist - radius;
	if (nearV <= 0.5)
		nearV = 0.5f;
	float farV = Min((float)(nearV + 100.0), dist + radius);
	if (nearClip > 0.0f)
		nearV = nearClip;
	if (farClip > nearClip)
		farV = Max(farClip, nearV);
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetFOV(1.5707964f);
	g_view->SetClip(nearV, farV, false);
	g_view->SetScreenCenter(m_center.x, m_center.y);
	g_view->UpdateCamera();
	if (m_type == EXHIBIT_CHARACTER)
	{
		if (m_pPetFrame)
		{
			m_pPetFrame->UpdateLightSource(&m_light, false, NULL);
			m_pPetFrame->Render(g_view, true, 1.0f, false, false, !bHardwareTnL);
		}
	}
	else if (m_pPuppet)
	{
		m_pPuppet->UpdateLightSource(&m_light, false, NULL);
		m_pPuppet->Render(g_view, true, 1.0f, false, false, false);
	}
	m_fxBox.Display(g_view);
	g_view->Flush(1, 1);
	g_view->SetProjectionMode(oldMode);
	g_view->SetClip(oldNear, oldFar, false);
	g_view->SetFOV(oldFov);
	g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5f, g_view->SCREEN_YS * 0.5f);
	g_view->UpdateCamera();
	g_view->SetFogEnable(bFog);
}

// layout do 983 (construtor 00877e30, destrutor 00877180, Init 00874930; out/F3.5/s115/RELATORIO.md)
C_ASSERT_VC10(sizeof(CExhibition) == 0x64c);
C_ASSERT(offsetof(CExhibition, m_pPetFrame) == 0xa8);
C_ASSERT(offsetof(CExhibition, m_pPuppet) == 0xac);
C_ASSERT(offsetof(CExhibition, m_light) == 0xe4);
C_ASSERT(offsetof(CExhibition, m_fxBox) == 0x108);
C_ASSERT_VC10(offsetof(CExhibition, m_pFxInterpreter) == 0x548);
C_ASSERT_VC10(offsetof(CExhibition, m_texParts) == 0x54c);
C_ASSERT_VC10(offsetof(CExhibition, m_type) == 0x558);
C_ASSERT_VC10(offsetof(CExhibition, m_tid) == 0x55c);
C_ASSERT_VC10(offsetof(CExhibition, m_flags) == 0x564);
C_ASSERT_VC10(offsetof(CExhibition, m_bMotionChanged) == 0x568);
C_ASSERT_VC10(offsetof(CExhibition, m_bChatFacial) == 0x575);
C_ASSERT_VC10(offsetof(CExhibition, m_partTids) == 0x578);
C_ASSERT_VC10(offsetof(CExhibition, m_bEnableZoom) == 0x640);
C_ASSERT_VC10(offsetof(CExhibition, m_bRandomMotion983) == 0x643);
C_ASSERT_VC10(offsetof(CExhibition, m_uid) == 0x644);
C_ASSERT_VC10(offsetof(CExhibition, m_bPetBillboard983) == 0x648);
C_ASSERT_VC10(sizeof(CExhibitionByHardwareTnL) == 0x64c);

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:??0CExhibition983@@QAE@MMMK_N@Z=??0CExhibition@@QAE@MMMK_N@Z") // mesma VA 00877e30
#pragma comment(linker, "/alternatename:?SetArea@CExhibition983@@QAEXMM@Z=?SetArea@CExhibition@@QAEXMM@Z") // mesma VA 00873900
#pragma comment(linker, "/alternatename:?SetDistance@CExhibition983@@QAEXM_N0@Z=?SetDistance@CExhibition@@QAEXM_N0@Z") // mesma VA 00874a20
// A289: sem estes dois o lobbyavataractor (bonecos, caddie e mascote do lobby) chamava stubs vazios -> nunca animava/girava nem pegava clique
#pragma comment(linker, "/alternatename:?Process@CExhibition983@@QAEXM_N@Z=?Process@CExhibition@@QAEXM_N@Z") // mesma VA 00877340
#pragma comment(linker, "/alternatename:?InValidArea@CExhibition983@@QAE_NXZ=?InValidArea@CExhibition@@IAE_NXZ") // mesma VA 00873d50
#pragma comment(linker, "/alternatename:??1CExhibitionByHardwareTnL983@@UAE@XZ=??1CExhibitionByHardwareTnL@@UAE@XZ") // mesma classe (vtable 00d05e9c, 0x64c)
#pragma comment(linker, "/alternatename:?Display@CExhibitionByHardwareTnL983@@UAEXMM@Z=?Display@CExhibitionByHardwareTnL@@UAEXMM@Z") // idem
#pragma comment(linker, "/alternatename:?SetModel@CExhibitionByHardwareTnL983@@UAE_NAAVCPartTidList@@PAKPBDMH@Z=?SetModel@CExhibitionByHardwareTnL@@UAE_NAAVCPartTidList@@PAKPBDMH@Z") // idem
