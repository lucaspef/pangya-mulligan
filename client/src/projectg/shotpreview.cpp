#include "minatl.h"
#include "shotpreview.h"
#include <functional>
#include <algorithm>
#include <math.h>
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "3dfont.h"
#include "wind.h"
#include "statushelper.h"
#include "cardsystem.h"
#include "fx.h"
#include "fxsequence.h"
#include "rtti.h"
#include "../shared/localize.h"
#ifdef JP983_DEV_PREVIEW
#include "gmtoolkit.h"
#include "inputmanager.h"
#endif

// HACK: the linker keeps the copy of the "predict_*" names of golfplayer.obj (earlier in link order), which is a local static of a
// naked function there, so this unit must not emit its own copy: name that one.
#define RB_STR_PREDICT_NORMAL "predict_normal"
#define RB_STR_PREDICT_TOMAHWAK "predict_tomahwak"
#define RB_STR_PREDICT_COBRA "predict_cobra"
#define RB_STR_PREDICT_SPIKE "predict_spike"

// HACK: forces the constant table the original unit emitted
inline void rb_UseConsts()
{
	(void)g_PI;
	(void)g_DEGTORAD;
	(void)g_HUGE;
	(void)g_EPSILON;
	(void)BAR_END;
	(void)BAR_START;
}

const WVector& GetHoleCupPos(); // 983: devolve referencia
WMatrix RotMat(const WVector& axis, float angle);
WMatrix operator*(const WMatrix& left, float right);

int ShotPreview::PangCoin::_PetReferenceCounter = 0;

// HACK: the original runs BAR_LEN's initializer before the actor's but allocates its bss
// after it; a separate ".bss" section keeps the initializer order and the image layout.
#pragma bss_seg(".bss")
static float BAR_LEN = BAR_END - BAR_START;
#pragma bss_seg()

IMPLEMENT_ACTOR(ShotPreview, IActor)

// 983: m_bar (+0xe0) .calipers (+0x20) = +0x100 (UpdateSensor::Update 00844590); o 645 lia +0xf0 cru
// HACK: a macro so that no GetPowerBarPos COMDAT is emitted (the dead copies of
// the naked functions would reference it)
#define GetPowerBarPos() (GOLFDOC()->m_bar.calipers)

namespace
{
}

ShotPreview::PangCoin::PangCoin(const WVector& pos, float scale, bool bDown)
{
	m_bVisible = true;
	m_scale = scale;
	m_angle = (float)(rand() % 0x13a) * 0.01f;
	m_pPuppet = g_resrcmng->GetPuppet("coin.pet", false, true, false);
	++_PetReferenceCounter;
	m_name = MakeStr("\xc6\xce\xc4\xda\xc0\xce_%d", _PetReferenceCounter);
	m_matrix.Reset();
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, m_pPuppet, NULL,
		m_name.c_str(), 8, NULL);
	m_matrix.pivot = pos;
	m_pos = pos;
	m_pPuppet->CalcBound(NULL, true);
	m_radius = scale * m_pPuppet->GetBoundSphere().radius;
	if (bDown)
		m_matrix.ym -= m_radius;
	else
		m_pos.y += m_radius;
}

ShotPreview::PangCoin::~PangCoin()
{
	--_PetReferenceCounter;
	if (m_pPuppet)
	{
		CSceneManager::Instance()->DeleteElement(m_name.c_str());
		if (g_resrcmng && m_pPuppet)
		{
			g_resrcmng->Release(m_pPuppet, false);
			m_pPuppet = NULL;
		}
	}
}

void ShotPreview::PangCoin::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
	CSceneManager::Instance()->SetVisible(m_name.c_str(), bVisible);
}

void ShotPreview::PangCoin::Restore()
{
	if (m_pPuppet == NULL)
	{
		m_pPuppet = g_resrcmng->GetPuppet("coin.pet", false, true, false);
		CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, m_pPuppet, NULL,
			m_name.c_str(), 8, NULL);
	}
}

void ShotPreview::PangCoin::Update(float dt)
{
	m_angle = dt * g_PI + m_angle;
	if (m_pPuppet && m_bVisible && (GOLFDOC()->m_gameMode & 0x80))
	{
		const CGolfBall& ball = GolfBall();
		float dx = ball.m_pos.x - m_pos.x;
		float dy = ball.m_pos.y - m_pos.y;
		float dz = ball.m_pos.z - m_pos.z;
		float dist = sqrtf(dx * dx + dy * dy + dz * dz);
		if (dist <= m_radius)
		{
			CFxSequence* pSeq =
				CFx::Instance()->OpenSequence("coin_pang.seq", true);
			pSeq->m_pos = m_pos;
			CSceneManager::Instance()->DeleteElement(m_name.c_str());
			if (g_resrcmng && m_pPuppet)
			{
				g_resrcmng->Release(m_pPuppet, false);
				m_pPuppet = NULL;
			}
			g_audio->PlaySfx(MakeStr("pang_coin_%d", rand() % 3 + 1));
		}
	}
}

void ShotPreview::PangCoin::Render()
{
	if (0)
		GOLFDOC();
	if (m_pPuppet && m_bVisible)
	{
		WVector pos = m_matrix.pivot;
		m_matrix = RotMat(WVector(0.0f, 1.0f, 0.0f), m_angle);
		m_matrix.pivot = pos;
		m_pPuppet->UpdateLightSource(&g_lightset, false, false);
		m_pPuppet->SetMatrix(m_matrix * m_scale);
		m_pPuppet->UpdateBSphere();
		m_pPuppet->Transform(NULL, 1.0f, 0);
	}
}

ShotPreview::UpdateSensor::UpdateSensor()
{
	memset(this, 0, sizeof(UpdateSensor));
}

bool ShotPreview::UpdateSensor::operator==(const UpdateSensor& other) const
{
	int r = memcmp(this, &other, sizeof(UpdateSensor));
	return r == 0;
}

// 983 00842ac0: spin/curve (8 primeiros bytes) diferem e o resto (0x1c bytes) e igual
bool ShotPreview::UpdateSensor::OnlySpinCurveChanged(const UpdateSensor& other) const
{
	if (memcmp(this, &other, 8) == 0)
		return false;
	return memcmp((const char*)this + 8, (const char*)&other + 8, 0x1c) == 0;
}

#define FIELD983(type, base, off) (*(type*)((char*)(base) + (off)))
static int s_lastSpin983; // 983 DAT_00e7c71c
static int s_lastCurve983; // 983 DAT_00e7c720

#ifdef JP983_DEV_PREVIEW
static bool s_devPreview983; // Ctrl+R (so GM): previsao completa do 645 por cima do Play Assist

bool DevPreviewCtrlR983()
{
	// 983 00a764b0 (CGMToolkit::IsAdministrator): bit 2 do dwIdentity do login
	return CGMToolkit::Instance()->IsAdministrator() &&
		(g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true));
}

bool DevPreviewOn983()
{
	return s_devPreview983 && CGMToolkit::Instance()->IsAdministrator();
}

// B1403 (PEDIDOS 19:32 b): Ctrl+setas so mexem o ponto de impacto (m_initSpin: lie + 4 graus por unidade em CClub::GetLie e o fator
// do quique); o back/top spin forte vem da seta cima/baixo DURANTE a barra (SetSpecialShot 0x11/0x12 -> m_initSpecial 1/2 ->
// CQuadTree::MakeAfterBurner, -40*m_initSpin), que o 983 nao preve (lista so tem normal/tomahawk/cobra/spike). A previsao GM
// marca tambem onde a bola para com esse comando (baixo = 2 com impacto embaixo, cima = 1 com impacto em cima).
static int s_devSpinCmd983;
static std::vector<WVector> s_devSpinPos983;
static std::vector<WVector> s_devSpinBound983;
#define DEV_SPIN_PREDICT983(spin) \
	do { \
		s_devSpinPos983.clear(); \
		s_devSpinBound983.clear(); \
		if (DevPreviewOn983() && GolfClub().GetType() != 3 && Wabs(spin) >= 1.0f / 30.0f) \
		{ \
			s_devSpinCmd983 = (spin) > 0.0f ? 2 : 1; \
			Predict(s_devSpinPos983, s_devSpinBound983, NULL); \
			s_devSpinCmd983 = 0; \
		} \
	} while (0)
#endif

// 983 00842ea0: spin/curve (x30, truncados) andaram 5 ou mais desde o ultimo previsto
bool ShotPreview::UpdateSensor::MovedSignificantly() const
{
	float dx = (float)abs(s_lastSpin983 - (int)(m_spin * 30.0));
	if (!(5.0f <= dx))
	{
		float dy = (float)abs(s_lastCurve983 - (int)(m_curve * 30.0));
		if (dy < 5.0f)
			return false;
	}
	return true;
}

C_ASSERT(sizeof(ShotPreview::UpdateSensor) == 0x24);

void ShotPreview::UpdateSensor::Update()
{
	if (0)
		((ShotPreview*)0)->IsSkipState();
	AfxGetTask()->GetActor("Player")
		<< MsgObject(NULL, M983(0x110), (int)&m_angle, 0, 0, 0, 0);
	AfxGetTask()->GetActor("Spin")
		<< MsgObject(NULL, M983(0x161), (int)&m_curve, (int)&m_spin, 0, 0, 0);
	((unsigned char*)this)[9] = GolfClub().GetIndex();
	m_wind = Wind().GetGlobalWind();
	m_powerShot = GolfClub().PowerShotType();
	unsigned char player = GOLFDOC()->m_currentPlayer;
	m_item = GOLFDOC()->GetPlayer(player)->item;
	m_barPos = GetPowerBarPos();
}

// 983 008430e0: carrega assist_top/trajectory_tex/assist_side/assist_sphere e sorteia a direcao inicial (rotacao em Y)
ShotPreview::AssistGuide::AssistGuide()
{
	m_texTrajectory = g_resrcmng->LoadTexture("trajectory_tex.png", 0, 0, NULL);
	m_texTop = g_resrcmng->LoadTexture("assist_top.png", 0, 0, NULL);
	m_scale = 1.0f;
	m_ten = 10;
	m_f24 = 20.0f;
	m_bActive = false;
	m_f28 = 100.0f;
	m_f2c = 90.0f;
	m_f30 = 95.0f;
	m_f34 = 0.0f;
	m_f38 = 0.0f;
	m_f3c = 25.0f;
	m_f40 = 0.5f;
	m_size = 0.0f;
	m_pos = WVector(0.0f, 0.0f, 0.0f);
	m_pSide = g_resrcmng->GetPuppet("assist_side.pet", false, true, false);
	if (m_pSide)
		m_pSide->CalcBound((Waabb*)NULL, true);
	m_pSphere = g_resrcmng->GetPuppet("assist_sphere.pet", false, true, false);
	if (m_pSphere)
		m_pSphere->CalcBound((Waabb*)NULL, true);
	m_pDraw = NULL;
	RandomizeDir();
}

void ShotPreview::AssistGuide::RandomizeDir()
{
	WMatrix rot = RotMat(Random(0.0f, 6.2831855f), 1);
	const float* m = (const float*)&rot;
	m_dir.x = m[0] + m[3] + m[6];
	m_dir.z = m[2] + m[5] + m[8];
	m_dir.y = 0.0f;
	m_dir.Normalize();
}

// 983 008432c0: so age com o flag +0x20 ligado; newDir sorteia nova direcao (Y), scale reduz por Random(0,1)
void ShotPreview::AssistGuide::Reset(bool scale, bool newDir)
{
	if (!m_bActive)
		return;
	m_bActive = false;
	if (newDir)
		RandomizeDir();
	if (scale)
	{
		float r = Random(0.0f, 1.0f);
		m_dir.x *= r;
		m_dir.y *= r;
		m_dir.z *= r;
	}
}

// 983 00842d40: avanca as duas fases da animacao do guia (+0x34 a 0.05/s, +0x38 a +0x40/s), voltando a 0 ao passar de 1
void ShotPreview::AssistGuide::Update(float dt)
{
	m_f34 += 0.05f * dt;
	if (m_f34 > 1.0f)
		m_f34 = 0.0f;
	m_f38 += m_f40 * dt;
	if (m_f38 > 1.0f)
		m_f38 = 0.0f;
}

// 983 00842da0: raio-base do marcador pelo alcance do taco, ajustado por tipo de taco
float ShotPreview::AssistGuide::GetClubScale()
{
	float v = (float)sqrt(GolfClub().GetRange() / 230.0f) * 0.12f;
	switch (GolfClub().GetType())
	{
	case 0:
		return v * 0.82f;
	case 1:
		return v * 0.45f;
	case 2:
		return v * 0.4f;
	}
	return v;
}

// 983 008433c0
WVector ShotPreview::AssistGuide::GetVector()
{
	float k = GetClubScale();
	return WVector(m_scale * k * m_dir.x, m_dir.y * k * m_scale,
		m_scale * m_dir.z * k);
}

// 983 00843440: quad girando em torno de m_pos com a textura assist_top
void ShotPreview::AssistGuide::Draw()
{
	WTVertex v[4];
	WTVertex* vl[5];
	for (int i = 0; i < 4; ++i)
	{
		bool hi = (i & 2) != 0;
		bool lo = (i & 1) != 0;
		WMatrix rot = RotMat(m_f34 * 6.2831853071795862f, 1);
		const float* m = (const float*)&rot;
		float x = m_size * (hi ? 1.0f : -1.0f);
		float y = m_size * 0.2f;
		float z = m_size * (lo ? -1.0f : 1.0f);
		v[i].x = m_pos.x + (y * m[3] + x * m[0] + z * m[6] + m[9]);
		v[i].y = m_pos.y + (m[4] * y + m[1] * x + m[7] * z + m[10]);
		v[i].z = m_pos.z + (z * m[8] + m[5] * y + m[2] * x + m[11]);
		v[i].tu = hi ? 1.0f : 0.0f;
		v[i].tv = lo ? 0.0f : 1.0f;
		v[i].diffuse = 0xffffffff;
	}
	vl[0] = &v[0];
	vl[1] = &v[1];
	vl[2] = &v[3];
	vl[3] = &v[2];
	vl[4] = NULL;
	g_view->DrawPolygonFan(vl, m_texTop & 0x7ff | 0x400000, 0x200, false);
	if (m_pDraw)
	{
		WMatrix mat = m_pDraw->GetMatrix();
		mat.Rotate(0.005f, 1, true);
		m_pDraw->SetMatrix(mat);
		m_pDraw->Render(g_view, true, 1.0f, false, true, false);
	}
}

// 983 00843f80: faixa da trajetoria (trajectory_tex) em quads por segmento; a cauda (90%..95% dos pontos, +0x2c/+0x30) some em alfa 200 -> 0
void ShotPreview::AssistGuide::DrawTrail983(const std::vector<WVector>& pts)
{
	WTVertex v[4];
	WTVertex* vl[5] = { &v[0], &v[1], &v[2], &v[3], NULL };
	for (int i = 0; i < 4; ++i)
	{
		v[i].diffuse = 0xc8ffffff;
		v[i].tu = 0.0f;
		v[i].tv = 0.0f;
	}
	int n = (int)pts.size();
	int start = (int)(n * 0.01 * m_f2c);
	int end = (int)(n * 0.01 * m_f30);
	int perTex = n / m_ten;
	float fade = 200.0f / (float)(end - start);
	float width = m_f3c * GolfBall().GetRadius() * 0.1f;
	int idx = -start;
	for (int k = 0; k < n - 1; ++k, ++idx)
	{
		const WVector& p0 = pts[k];
		const WVector& p1 = pts[k + 1];
		WVector side = WCrossProduct(p1 - p0, WVector(0.0f, 1.0f, 0.0f));
		side.Normalize();
		side = side * width;
		v[0].SetPosition(p0 - side);
		v[1].SetPosition(p1 - side);
		v[2].SetPosition(p1 + side);
		v[3].SetPosition(p0 + side);
		if (perTex != 0)
		{
			float a = (1.0f / (float)perTex) * (float)(k % perTex);
			float u0 = a - m_f38;
			float u1 = (1.0f / (float)perTex + a) - m_f38;
			v[0].tu = u0;
			v[0].tv = 1.0f;
			v[1].tu = u1;
			v[1].tv = 1.0f;
			v[2].tu = u1;
			v[2].tv = 0.0f;
			v[3].tu = u0;
			v[3].tv = 0.0f;
		}
		if (idx >= 0)
		{
			float a0 = 200.0f - fade * (float)idx;
			float a1 = 200.0f - fade * (float)(idx + 1);
			if (!(a0 >= 0.0f))
				a0 = 0.0f;
			if (!(a1 >= 0.0f))
				a1 = 0.0f;
			v[0].diffuse = v[3].diffuse = ((unsigned long)(int)(a0 + 0.5f) << 24) | 0xffffff;
			v[1].diffuse = v[2].diffuse = ((unsigned long)(int)(a1 + 0.5f) << 24) | 0xffffff;
		}
		g_view->DrawPolygonFan(vl, (m_texTrajectory & 0x7ff) | 0xc00000, 0x200, false);
	}
}

// 983 00842f90 (modo 0x200): linha azul que some do mesmo trecho 90%..95% (alfa 255 -> 0)
void ShotPreview::AssistGuide::DrawFadeLine983(const std::vector<WVector>& pts)
{
	int n = (int)pts.size();
	unsigned long color = 0xff0000ff;
	for (int i = 0; i < n - 1; ++i)
	{
		int start = (int)(n * 0.01 * m_f2c);
		if (i - start >= 0)
		{
			int end = (int)(n * 0.01 * m_f30);
			float a = 255.0f - (255.0f / (float)(end - start)) * (float)(i - start);
			if (!(a >= 0.0f))
				a = 0.0f;
			color = ((unsigned long)(int)a << 24) | 0xff;
		}
		g_view->DrawLine(pts[i], color, pts[i + 1], color, 0, 2.0f);
	}
}

ShotPreview::ShotPreview()
	: m_predictType(0), m_bEnable(1)
{
	// 983 00844d40: +0x54 nasce 0 (param_1[0x15] = 0); so o item Play Assist liga bits via TOGGLE_* (o 645 nascia com 10)
	m_optionFlag = 0;
	m_elapsed = 0.0f;
	m_pBall = new CGolfBall;
	m_pSensor = new UpdateSensor;
	m_pSensorCur = new UpdateSensor;
	m_bSensorChanged = false;
	m_bSensorMoved = true; // 983 00844d40: byte +0x234 nasce 1
	m_guideSide = 0;
	m_indicatorTex = g_resrcmng->LoadTexture("{indicator.bmp", 0, 0, NULL);
	m_dispPriority = DISP_PRIORITY_2;
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, this,
		"ShotPreview", 0, NULL);
	m_pFont = new C3dFont;
	m_pFont->LoadTexture("[font_bar.jpg", 0x10, 0x10, "1234567890ym%-.:", 6.0f,
		-25.0f);
}

std::string ShotPreview::TwAddOption(const tweaker_param& param)
{
	unsigned int flag = GetOptionFlag();
	SetOptionFlag(param[0].to_int() | flag);
	return "TwAddOption Processed";
}

std::string ShotPreview::TwSubOption(const tweaker_param& param)
{
	unsigned int flag = GetOptionFlag();
	SetOptionFlag(~param[0].to_int() & flag);
	return "TwSubOption Processed";
}

ShotPreview::~ShotPreview()
{
	std::for_each(m_coins.begin(), m_coins.end(), Delete_Object());
	m_coins.clear();
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
	if (m_pBall)
	{
		delete m_pBall;
		m_pBall = NULL;
	}
	if (m_pSensor)
	{
		delete m_pSensor;
		m_pSensor = NULL;
	}
	if (m_pSensorCur)
	{
		delete m_pSensorCur;
		m_pSensorCur = NULL;
	}
	if (g_resrcmng && m_indicatorTex)
	{
		g_resrcmng->Release(m_indicatorTex);
		m_indicatorTex = 0;
	}
}

void ShotPreview::OnReload()
{
	OnLoad();
}

namespace
{
	WVector GetProjectionVector(const WVector& pos)
	{
		WVector v = pos;
		WTVertex vtx;
		vtx.x = v.x;
		vtx.y = v.y;
		vtx.z = v.z;
		g_view->Projection2(&vtx, v);
		v.x = vtx.x;
		v.y = vtx.y;
		v.z = vtx.z;
		return v;
	}
}

unsigned short ShotPreview::GetBallInitSpecialValueFromPredictType(
	PredictType::Enum type)
{
	if (type == PredictType::TOMAHAWK)
		return 0x10;
	if (type == PredictType::COBRA)
		return 0x20;
	if (type == PredictType::SPIKE)
		return 0x40;
	return 0;
}

bool ShotPreview::IsSkipState()
{
	// 983 00844a40: sem bits de Play Assist (+0x54) nao ha previsao; so no turno de quem joga (00530c00, ecx = this = verdadeiro)
#ifdef JP983_DEV_PREVIEW
	bool dev = DevPreviewOn983();
#else
	bool dev = false;
#endif
	if (m_optionFlag == 0 && !dev)
		return true;
	if (!IsMyTurn(true))
		return true;
	unsigned long mode = GOLFDOC()->m_gameMode;
	if ((mode & 0x3f8) == 0)
		return true;
	if ((m_optionFlag & 0x42) != 0 && (mode & 0xc0) != 0 && !dev)
		return true;
	// putter: 983/645 sem previsao; a do GM (Ctrl+R) tambem preve o putt (rolagem no green, item 16 do dono)
	if (GolfClub().GetType() == 3 && !dev)
		return true;
	return m_bEnable == 0;
}

float __fastcall Scale983(int map, unsigned long channelType, float value); // 0046f540 (jp/aiplayer.cpp, frente B)

namespace
{
	unsigned long GetGuideLineColor(const WVector& pos)
	{
		// 983 (00844450): distância no plano (y zerado nos dois) contra o raio do copo do 0046f540
		WVector p = pos;
		p.y = 0.0f;
		WVector pin = GOLFDOC()->GetHoleData().pin;
		pin.y = 0.0f;
		WVector d = pin - p;
		return d.Magnitude() < Scale983(Doc()->m_golfGame.gameType,
				  Doc()->m_curChannel.Type, GolfBall().m_radius)
			? 0xa80000ff
			: 0xa8ffff00;
	}
}

// 983 008446c0: posiciona o marcador "bound" (assist_side/assist_sphere) no ponto de quique; tamanho = dist limitada a [f24,f28]*0.1*3.2
void ShotPreview::AssistGuide::UpdateBound983(const WVector& pos, float dist)
{
	m_pos = pos;
	float lo = (float)(m_f24 * 0.1 * (double)3.2f);
	float hi = (float)(0.1 * m_f28 * (double)3.2f);
	m_size = dist < lo ? lo : (dist > hi ? hi : dist);
	float cup = Scale983(Doc()->m_golfGame.gameType,
		Doc()->m_curChannel.Type, GolfBall().m_radius);
	WVector toPin = GOLFDOC()->GetHoleData().pin - m_pos;
	if (cup + cup < toPin.Magnitude())
	{
		WVector ground = GetPVS().GetGroundPoint(m_pos, true, NULL, true);
		float d = fabsf((m_pos - ground).Magnitude() - GolfBall().m_radius);
		m_pDraw = 0.03125f <= d ? m_pSphere : m_pSide;
	}
	else
		m_pDraw = m_pSide;
	if (!m_pDraw)
		return;
	WMatrix mat(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	mat.pivot = m_pos;
	if (w_bound_box* box = m_pDraw->FindBoundBox("bound"))
	{
		const Waabb& bb = box->info->aabb;
		mat *= m_size / ((bb.max.z - bb.min.z) * 0.5f);
	}
	m_pDraw->SetMatrix(mat);
	m_pDraw->UpdateLightSource(&g_lightset, false, NULL);
	m_pDraw->SetAlpha(0x96);
}

void ShotPreview::DrawIndicator(const WVector& pos, unsigned long color,
	float scale)
{
	float size = scale * 20.0f;
	WVector ground = GetPVS().GetGroundPoint(pos, false, NULL, false);
	WTVertex vtx;
	WTVertex v[4];
	WTVertex* vl[5];
	memset(vl, 0, sizeof(vl));
	g_view->Projection2(&vtx, ground);
	if (vtx.z > 0.0f && vtx.z < 1.0f)
	{
		float half = size * 0.5f;
		v[0].x = vtx.x - half;
		v[0].y = vtx.y;
		v[0].z = vtx.z;
		v[1].x = vtx.x - half;
		v[1].y = vtx.y - size;
		v[1].z = vtx.z;
		v[2].x = vtx.x + half;
		v[2].y = vtx.y - size;
		v[2].z = vtx.z;
		v[3].x = vtx.x + half;
		v[3].y = vtx.y;
		v[3].z = vtx.z;
		v[0].rhw = 1.0f;
		v[0].diffuse = color;
		v[0].tu = 0.0f;
		v[0].tv = 1.0f;
		v[1].rhw = 1.0f;
		v[1].diffuse = color;
		v[1].tu = 0.0f;
		v[1].tv = 0.0f;
		v[2].rhw = 1.0f;
		v[2].diffuse = color;
		v[2].tu = 1.0f;
		v[2].tv = 0.0f;
		v[3].rhw = 1.0f;
		v[3].diffuse = color;
		v[3].tu = 1.0f;
		v[3].tv = 1.0f;
		vl[0] = &v[0];
		vl[1] = &v[1];
		vl[2] = &v[2];
		vl[3] = &v[3];
		if (g_view->GetWidth() - 0.5f >= v[0].x && v[2].x >= 0.5f &&
			vtx.y >= 0.5f && g_view->GetHeight() - 0.5f >= v[1].y)
		{
			unsigned int flags = m_indicatorTex & 0x7ff | 0x6400000;
			if ((GOLFDOC()->m_gameMode & 0x200) == 0)
			{
				m_pFont->SetProjectionStyle(
					C3dFont::ProjectionStyle::Perspective);
			}
			else
			{
				flags = m_indicatorTex & 0x7ff | 0x6700000;
				m_pFont->SetProjectionStyle(C3dFont::ProjectionStyle::Screen);
			}
			g_view->DrawPolygonFan(vl, flags, 0, true);
#ifdef JP983_DEV_PREVIEW
			if ((m_optionFlag & 8) || DevPreviewOn983())
#else
			if (m_optionFlag & 8)
#endif
			{
				WVector p = pos;
				WVector cup = GetHoleCupPos();
				WVector diff(p.x - cup.x, 0.0f, p.z - cup.z);
				m_pFont->SetPos(ground);
				if (color == 0xa80000ff)
				{
					m_pFont->SetColor(-0x7f0001);
					m_pFont->Printf(g_view, -7, 0x400000, "0y");
					return;
				}
				m_pFont->SetColor(-1);
				m_pFont->Printf(g_view, -7, 0x400000,
					MakeStr("%.1fy",
						(double)(diff.Magnitude() * 0.3125f)));
			}
		}
	}
}

void ShotPreview::OnInit()
{
	// 983 008450b0: limpa os dois vetores, zera lado/flag do sensor e reinicia o guia
	m_predictPos.clear();
	m_boundPos.clear();
	m_guideSide = 0;
	m_bSensorChanged = false;
	m_assist.Reset(true, true);
	// 983 008450b0: escala = 1 - 005e48e0(2) * 0.01, presa em [0,1]
	CStatusHelper status;
	float scale = 1.0f - (float)status.GetStatus983(2) * 0.01f;
	m_assist.SetScale(scale < 0.0f ? 0.0f : (scale > 1.0f ? 1.0f : scale));
}

void ShotPreview::OnLoad()
{
	m_buttons.clear();
	int pButtons[4] = { 0 };
	SendMsg(this, "GolfRule", M983(0x24a), (int)RB_STR_PREDICT_NORMAL,
		(int)&pButtons[0], 0, 0);
	SendMsg(this, "GolfRule", M983(0x24a), (int)RB_STR_PREDICT_TOMAHWAK,
		(int)&pButtons[1], 0, 0);
	SendMsg(this, "GolfRule", M983(0x24a), (int)RB_STR_PREDICT_COBRA,
		(int)&pButtons[2], 0, 0);
	SendMsg(this, "GolfRule", M983(0x24a), (int)RB_STR_PREDICT_SPIKE,
		(int)&pButtons[3], 0, 0);
	for (int i = 0; i < 4; ++i)
	{
		if (pButtons[i] != 0)
		{
			FrButton* pButton = DYNAMIC_CAST(FrButton, (IObject*)pButtons[i]);
			if (pButton)
				m_buttons.push_back(pButton);
		}
	}
}

void ShotPreview::OnProcess(float delta)
{
	// 983 00846470 (o OnProcess do 645 com Shift+REPLAY/NUM_9 e botoes predict_* nao existe no 983)
#ifdef JP983_DEV_PREVIEW
	if (DevPreviewCtrlR983() && g_input->GetDown("REPLAY", true))
	{
		s_devPreview983 = !s_devPreview983;
		CGMToolkit::Instance()->Display(s_devPreview983 ? "Shot preview (GM): ON" : "Shot preview (GM): OFF", 0x6eecff);
		m_predictPos.clear();
		m_boundPos.clear();
		if (!IsSkipState())
			Predict(m_predictPos, m_boundPos, NULL);
		m_pSensor->Update();
		s_devSpinPos983.clear();
		s_devSpinBound983.clear();
	}
#endif
	if (IsSkipState())
		return;
	m_assist.Update(delta);
	m_pSensorCur->Update();
	bool wasChanged = m_bSensorChanged;
	if (*m_pSensor == *m_pSensorCur)
	{
		m_bSensorChanged = false;
	}
	else
	{
		m_bSensorChanged = true;
		if (m_pSensor->OnlySpinCurveChanged(*m_pSensorCur))
			m_bSensorMoved = m_pSensor->MovedSignificantly();
		else
			m_bSensorMoved = true;
#ifdef JP983_DEV_PREVIEW
		// B1396 (item 16c): o 983 testa o sensor ANTIGO (+0x44 = ultimo previsto), entao um passo de spin/curva
		// de um quadro so (Ctrl+setas, clique) nunca refaz a conta; a previsao GM refaz sempre que o sensor assenta
		if (DevPreviewOn983())
			m_bSensorMoved = true;
#endif
	}
	if (wasChanged != m_bSensorChanged)
	{
		if (m_bSensorChanged)
		{
			SendMsg(this, "Screen", 0x332, 0, 0, 0, 0);
			m_pSensor->Update();
			return;
		}
		if (m_bSensorMoved)
		{
			s_lastSpin983 = (int)(m_pSensor->m_spin * 30.0);
			s_lastCurve983 = (int)(m_pSensor->m_curve * 30.0);
			m_predictPos.clear();
			m_boundPos.clear();
			m_assist.Reset(false, false);
			WVector guide = m_assist.GetVector();
#ifdef JP983_DEV_PREVIEW
			if (DevPreviewOn983()) // previsao GM exata: sem o desvio aleatorio do Play Assist
				guide = WVector::ZERO;
#endif
			GetPVS().m_vec21d8_983 = guide; // B1480: ex-crus &GetPVS()+0x21d8/0x21dc/0x21e0
			int firstBound = 0;
			Predict(m_predictPos, m_boundPos, &firstBound);
#ifdef JP983_DEV_PREVIEW
			DEV_SPIN_PREDICT983(m_pSensor->m_spin);
#endif
			if (m_optionFlag & 0x20)
			{
				UpdateGuideSide983();
				SendMsg(this, "Screen", 0x331, m_guideSide, 0, 0, 0);
			}
			if (m_optionFlag & 0x40)
			{
				// 983 nao testa vazio (le o 1o elemento direto); aqui evita ler vector vazio
				if (!m_boundPos.empty())
				{
					WVector d = (GolfBall().m_pos - m_boundPos[0]) * (double)0.0263f;
					d = d * m_assist.GetScale();
					m_assist.UpdateBound983(m_boundPos[0], d.Magnitude());
				}
			}
		}
	}
	m_pSensor->Update();
}

// 983 00843ce0: de que lado da bola a linha do guia cai em relacao ao 1o ponto de quique
void ShotPreview::UpdateGuideSide983()
{
	float angle = GOLFDOC()->m_cameraData.angle; // B1480: ex-cru GOLFDOC()+0x11c (cl 14.44 desloca o CGolfDoc)
	WVector offset(-sinf(angle), 0.0f, cosf(angle));
	offset = offset * (GolfClub().GetRange(NULL) * 3.2f);
	WVector ballPos = GolfBall().m_pos;
	WVector ground = GetPVS().GetGroundPoint(ballPos + offset, true, NULL, true);
	WVector toGround = ground - ballPos;
	WVector toBound = m_boundPos.empty() ? WVector(0.0f, 0.0f, 0.0f) : m_boundPos[0] - ballPos;
	float delta = CalcDeltaAngle(toGround, toBound);
	if (fabsf(delta) > 7.105427357601002e-15f)
		m_guideSide = delta < 0.0f ? 1 : 2;
	else
		m_guideSide = 0;
}

void ShotPreview::HandleMsg(const MsgObject& msg)
{
	if (msg.message == 0x2fd)
	{
		std::string cmd((const char*)msg.param1);
		// 983 00844b00 so conhece os comandos abaixo; GET_*, CHANGE_PREDICT_TYPE e PM_REPLAY do 645 sao ignorados
		if (cmd == "ENABLE_DISPLAY")
		{
			m_bEnable = msg.param2;
		}
#ifdef JP983_DEV_PREVIEW
		// 645: radios predict_* (CGolfRule::SetRadioPredictShotPress) trocam o tipo da previsao GM e refazem a conta
		else if (cmd == "CHANGE_PREDICT_TYPE")
		{
			if (msg.param2 != m_predictType)
			{
				m_predictType = msg.param2;
				m_predictPos.clear();
				m_boundPos.clear();
				if (!IsSkipState())
					Predict(m_predictPos, m_boundPos, NULL);
				m_pSensor->Update();
			}
		}
#endif
		// 983 00844b00: liga/desliga bits de +0x54 (param 0 desliga, senao liga); mandados pelos controllers do item Play Assist
		else if (cmd == "TOGGLE_BALL_DIR_NOTIFY" || cmd == "TOGGLE_TRAJECTORY_PREVIEW" || cmd == "TOGGLE_BOUNDING_SPOT")
		{
			unsigned int bit = cmd == "TOGGLE_BALL_DIR_NOTIFY" ? 0x20 : (cmd == "TOGGLE_TRAJECTORY_PREVIEW" ? 2 : 0x40);
			if (msg.param2 == 0 && (m_optionFlag & bit))
				m_optionFlag ^= bit;
			else
				m_optionFlag |= bit;
		}
		// 983: o ultimo ramo e PM_SET_TRAJECTORY_FAKE_FACTOR: zera +0x4c e chama 008432c0(1,1) no objeto +0x60 (AssistGuide::Reset)
		else if (cmd == "PM_SET_TRAJECTORY_FAKE_FACTOR")
		{
			m_predictType = 0;
			m_assist.Reset(true, true);
		}
	}
}

float __fastcall MascotProperty983(unsigned char player, unsigned char type); // 00635ef0 (club.cpp)

void ShotPreviewType9Areas983(float dt); // golfrule.cpp (B1394)

C_ASSERT_VC10(offsetof(CGolfDoc, m_cameraData.angle) == 0x11c); // B1480
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar.power) == 0xe0 && offsetof(CGolfDoc, m_bar.calipers) == 0x100); // A749: ex-crus GOLFDOC()+0xe0/+0x100

bool ShotPreview::Predict(std::vector<WVector>& positions,
	std::vector<WVector>& boundPositions, int* pFirstBoundIndex)
{
	// 983 008451d0 (inicio): com o bit +0x54 usa o calibre (bar +0x100) quando nao e ~0, e normaliza de novo
	float ratio = (GOLFDOC()->m_bar.power - BAR_START) / BAR_LEN;
#ifdef JP983_DEV_PREVIEW
	bool dev = DevPreviewOn983();
#else
	bool dev = false;
#endif
	// B1619: m_vec21d8_983 e o erro proposital do Play Assist (OnProcess: guia aleatorio, BounceProcess soma ate o topo do voo);
	// a tacada real zera (0068dcd0), logo a previsao GM, que deve acertar, tambem
	if (dev)
		GetPVS().m_vec21d8_983 = WVector::ZERO;
	if (m_optionFlag != 0 || dev)
	{
		if (g_EPSILON <= Wabs(GetPowerBarPos()))
			ratio = GetPowerBarPos();
		ratio = (ratio - BAR_START) / BAR_LEN;
	}
	if (Wabs(ratio) < g_EPSILON || ratio < 0.0f)
		ratio = 1.0f;
	float powerRatio = sqrtf(ratio);
	float power = GolfClub().GetPower();
	power = power * powerRatio;
	GolfClub();
	WVector vel;
	m_pBall->Reset(vel);
	*m_pBall = GolfBall();
	CGolfBall& ball = GolfBall();
	ball.Reset(ball.m_pos);
	float spotAngle;
	SendMsg(this, "Spot", M983(0xfa), (int)&spotAngle, 0, 0, 0);
	float playerAngle;
	SendMsg(this, "Player", M983(0x110), (int)&playerAngle, 0, 0, 0);
	playerAngle = playerAngle + spotAngle;
	CGolfBall::sExpected* pre = ball.PreDataToWrite(0);
	pre->pos = ball.m_pos;
	pre->event = 0;
	pre->state = CGolfBall::BALL_BOUNCE;
	// putt (so GM): lancamento como o da tacada real com putter (0068c2f0 + BallSpeedFactor 00672700 que volta cedo):
	// GetBarClickPowerFactor = 1 (sem o fator do chao), sem tacada especial, sem desvio de curva nem da tela
	bool bPutt = dev && GolfClub().GetType() == 3;
	// B1402: a tacada real com putter sai rolando (GetInitBallState 3 = BALL_ROLL, golfrule 0068c2f0), nao quicando
	if (bPutt)
		pre->state = CGolfBall::BALL_ROLL;
	float maxPower = GetPVS().GetMaxPowerOnGround();
	if (!bPutt)
		power = sqrtf(maxPower) * power;
	ball.m_initSpecial = bPutt ? 0 : GetBallInitSpecialValueFromPredictType(
		(PredictType::Enum)m_predictType);
	if (ball.m_initSpecial == 0x40 || ball.m_initSpecial == 0x10)
		power = power * 1.3f;
#ifdef JP983_DEV_PREVIEW
	if (dev && !bPutt && s_devSpinCmd983 != 0) // B1403: variante com o comando de spin da barra (0x11/0x12, golfrule SetSpecialShot)
		ball.m_initSpecial |= s_devSpinCmd983;
#endif
	SendMsg(this, "Spin", M983(0x161), (int)&ball.m_initCurve, (int)&ball.m_initSpin,
		0, 0);
	unsigned char player = GOLFDOC()->m_currentPlayer;
	float curveAngle = bPutt ? 0.0f : -(ball.m_initCurve * (20.0f * g_DEGTORAD));
	if (!bPutt && !GOLFDOC()->IsCurrentPlayerEvent983(6)) // 983 008451d0 -> 00663870(6); o 645 testava item 0x18000028
	{
		// B1476: o handler 0x1c1 (gui_control) grava um WVector inteiro; com float de 4 bytes o cl 14.44 perdia &m_initSpecial na pilha
		WVector screen;
		SendMsg(this, "Screen", M983(0x1c1), (int)&screen, 0, 0, 0);
		ball.m_initCurve = ball.m_initCurve -
			((ball.m_initSpecial & 0x40) ? screen.x * 0.15f
										 : screen.x * 0.5f);
	}
	float absCurve = Abs(ball.m_initCurve);
	const WMatrix& rotLie = RotMat(GolfClub().GetLie(NULL), 0);
	float speed = (absCurve * 0.1f + 1.0f) * power;
	WVector dir0;
	dir0.x = speed * rotLie.xz;
	dir0.y = speed * rotLie.yz;
	dir0.z = speed * rotLie.zz;
	const WMatrix& rotYaw = RotMat(curveAngle + playerAngle, 1);
	vel = dir0 * rotYaw;
	pre->vel = vel;
	float rot;
	SendMsg(this, "Player", M983(0x126), (int)&rot, 0, 0, 0);
	int savedFrame = GetPVS().m_frame;
	GetPVS().m_frame =
		savedFrame - (int)(rot * -50.0f) + 0x32;
	*(unsigned long*)&GetPVS().m_frameTime = 0;
	ball.m_vel = pre->vel;
	ball.m_state = pre->state;
	ball.m_special = ball.m_initSpecial;
	ball.m_spin = 0.0f;
	ball.m_tomahawkFrame = -1;
	if (bPutt) // B1402: 0068c2f0 zera o quadro de rolagem (+0x88) quando o estado inicial e BALL_ROLL
		ball.m_rollFrame = 0;
	GetPVS().ResetVariable();
	unsigned char groundType = ball.m_groundType;
	if (GolfClub().GetApproachMode() == 4)
	{
		// 983 008451d0: (aux + bonus de alcance + mascote)*0.5 + GetTotalRange do CCardManager (sem RANGE_BUFF do 645), + poder do jogador, /170
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		float t = CSharedDoc::Instance()->GetAuxPartProperty(cur, 0) * 0.5f;
		t = CSharedDoc::Instance()->GetRangeBonus983(cur) * 0.5f + t;
		t = MascotProperty983(cur, 0) * 0.5f + t;
		CCardManager::Instance()->SetPlayerIndex(cur);
		bool bPower = false;
		if (GolfClub().m_powerShot != 0)
			bPower = true;
		float cardRange = CCardManager::Instance()->GetTotalRange(bPower);
		// B1619: a tacada real soma o card * 0.5 (B1389, 0068db5f); o Play Assist do 983 soma inteiro
		t = (dev ? cardRange * 0.5f : cardRange) + t;
		t = ((float)(int)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->power + t) / 170.0f;
		if (t <= 0.0f)
			t = 0.0f;
		ball.m_spin = ratio * ratio * (t + 1.5f);
	}
	ball.m_curve = ratio * ball.m_initCurve;
	// 983 008451d0: guarda o byte +0x4da (sPlayerData::bunker) do jogador da vez e devolve no fim
	unsigned char savedByte4da = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->bunker;
	for (;;)
	{
		if (ball.m_state == CGolfBall::BALL_STOP)
		{
			ball.m_groundType = groundType;
			ball.m_special = ball.m_initSpecial;
			bool bFirst = true;
			for (int i = 0; i < ball.m_renderNum; ++i)
			{
				const CGolfBall::sExpected* e = ball.PreData(i);
				positions.push_back(e->pos);
				if (e->event != 0)
				{
					boundPositions.push_back(e->pos);
					if (bFirst)
					{
						bFirst = false;
						if (pFirstBoundIndex)
							*pFirstBoundIndex = i;
					}
					// 983 008451d0: com o bit 2 (TRAJECTORY_PREVIEW) so vai ate o 1o quique
					if ((m_optionFlag & 2) && !dev) // GM: todos os quiques ate parar (645)
						break;
				}
			}
			if ((m_optionFlag & 2) == 0 || dev || boundPositions.empty())
			{
				positions.push_back(ball.PreData(ball.m_renderNum)->pos);
				boundPositions.push_back(ball.PreData(ball.m_renderNum)->pos);
			}
			if (pFirstBoundIndex && bFirst)
				*pFirstBoundIndex = (int)positions.size() - 1;
			ball.Reset(vel);
			GolfBall() = *m_pBall;
			GetPVS().m_frame = savedFrame - 1;
			*(unsigned long*)&GetPVS().m_frameTime = 0;
			GetPVS().PreProcess(0.02f, true);
			GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->bunker = savedByte4da;
			return true;
		}
		++ball.m_renderNum;
		CGolfBall::sExpected* e = ball.PreDataToWrite(ball.m_renderNum);
		e->sound[0] = 0;
		e->event = 0;
		e->effect = 0;
		if (dev) // B1394: so GM - areas tipo 9 como na tacada real (0068dd50)
			ShotPreviewType9Areas983(0.02f);
		GetPVS().PreProcess(0.02f, true);
		GetPVS().BallProcess(0.02f, (m_optionFlag & 1) != 0 || dev);
		// 983 008451d0: cobra/spike (special & 0x60) tambem entram aqui fora do approach
		if (GolfClub().GetApproachMode() == 4 || (ball.m_special & 0x60) != 0)
		{
			if ((ball.m_special & 0x20) == 0)
			{
				if (ball.m_special & 0x40)
				{
					if (ball.m_spikeFrame < 0)
					{
						float base = GetPVS().GetBasePosY(ball.m_pos,
							ball.GetRadius() + ball.GetRadius());
						ball.m_pos.y = Max(base, ball.m_pos.y);
					}
					else
					{
						ball.m_spin = ball.m_spin - 0.02f;
					}
				}
				else
				{
					if (ball.m_bounceFrame < 0)
					{
						if (GetPVS().GetObjColNum() == 0)
						{
							ball.m_spin = ball.m_spin -
								(0.5f - ball.m_initSpin * 0.1f) * 0.02f;
						}
						else
						{
							ball.m_spin = ball.m_spin - 0.002f;
						}
					}
					else
					{
						ball.m_spin = ball.m_spin - 0.002f;
					}
				}
			}
			else
			{
				if (ball.m_cobraFrame < 0)
				{
					float base = GetPVS().GetBasePosY(ball.m_pos,
						ball.GetRadius() + ball.GetRadius());
					ball.m_pos.y = Max(base, ball.m_pos.y);
				}
				else
				{
					ball.m_spin = ball.m_spin - 0.02f;
				}
			}
		}
		bool bClamp = false;
		if (ball.m_renderNum < 3000)
		{
			if (ball.m_renderNum > 0x5db)
			{
				float speedSq = ball.m_vel.z * ball.m_vel.z +
					ball.m_vel.y * ball.m_vel.y + ball.m_vel.x * ball.m_vel.x;
				if (0.1f <= sqrtf(speedSq) && GetPVS().GetObjColNum() < 10 &&
					GetPVS().GetMeshIndex(ball.m_pos, false) != -1)
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
			WVector gp =
				GetPVS().GetGroundPoint(ball.m_pos, false, NULL, false);
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
		if (ball.m_state == CGolfBall::BALL_BOUNCE && ball.m_vel.y < 0.0f &&
			ball.m_topFrame < 0)
			ball.m_topFrame = ball.m_renderNum;
		if (ball.m_tomahawkFrame < 0 && (ball.m_initSpecial & 0x10) &&
			(ball.m_special & 0x10) == 0)
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
				float dist = sqrtf(dx * dx + dz * dz);
				if ((GolfClub().GetRange(NULL) * powerRatio - 100.0f) * 3.2f <
					dist)
				{
					ball.m_cobraFrame = ball.m_renderNum;
					ball.m_vel.Normalize();
					switch (GolfClub().GetID())
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
				dir0.x = power * n.x;
				dir0.y = power * n.y;
				dir0.z = power * n.z;
				ball.m_vel.x = dir0.x;
				ball.m_vel.y = dir0.y;
				ball.m_vel.z = dir0.z;
			}
			const WVector& start = ball.PreData(0)->pos;
			float dx = ball.m_pos.x - start.x;
			float dz = ball.m_pos.z - start.z;
			if (ball.m_spikeFrame < 0 && 0.0f < sqrtf(dx * dx + dz * dz))
			{
				ball.m_spikeFrame = ball.m_renderNum;
				ball.m_vel.Normalize();
				ball.m_vel *= 72.5f;
				ball.m_vel *= powerRatio + powerRatio;
				ball.m_spin = 3.1f;
			}
			if (ball.m_topFrame >= 0 &&
				ball.m_topFrame + 0x3c < ball.m_renderNum &&
				ball.m_spikeDiveFrame < 0)
			{
				ball.m_spikeDiveFrame = ball.m_renderNum;
				float v;
				switch (GolfClub().GetID())
				{
				case 0:
					v = Min(GolfClub().GetPower() * powerRatio, 344.0f);
					ball.m_vel.y = -8.0f - (344.0f - v) * (21.5f / 112.0f);
					break;
				case 1:
					v = Min(GolfClub().GetPower() * powerRatio, 306.0f);
					ball.m_vel.y = -10.3f - (306.0f - v) * (20.5f / 105.0f);
					break;
				case 2:
					v = Min(GolfClub().GetPower() * powerRatio, 273.0f);
					ball.m_vel.y = -10.8f - (273.0f - v) * (2.0f / 100.0f);
					break;
				}
				ball.m_vel *= powerRatio * 7.0f;
				ball.m_spin = ball.m_initSpin;
			}
		}
	}
}

void ShotPreview::Display()
{
	// 983 00844aa0: so a faixa/linha da trajetoria (bit 2) e o marcador do quique (bit 0x40); moedas e indicadores do 645 nao existem aqui
	if (IsSkipState())
		return;
	if (m_predictPos.empty())
		return;
#ifdef JP983_DEV_PREVIEW
	if (DevPreviewOn983())
	{
		// 645 Display com bits 1|4|8: trajetoria inteira, marca no 1o quique e onde para, distancia ate o copo
		if (GOLFDOC()->m_gameMode & 0x200)
			m_assist.DrawFadeLine983(m_predictPos);
		else
			m_assist.DrawTrail983(m_predictPos);
		unsigned long color = GetGuideLineColor(m_predictPos.back());
		if (!m_boundPos.empty())
		{
			DrawIndicator(m_boundPos.front(), color, 0.8f);
			if (m_boundPos.size() > 1)
				DrawIndicator(m_boundPos.back(), color, 0.8f);
		}
		// B1403: onde para com seta baixo/cima na barra (back/top spin do PangYa), marcador menor
		if (!s_devSpinBound983.empty())
			DrawIndicator(s_devSpinBound983.back(), 0xffff60ff, 0.5f);
		return;
	}
#endif
	if (m_optionFlag & 2)
	{
		if (GOLFDOC()->m_gameMode & 0x200)
			m_assist.DrawFadeLine983(m_predictPos);
		else
			m_assist.DrawTrail983(m_predictPos);
	}
	if (m_optionFlag & 0x40)
		m_assist.Draw();
}
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // A748: ex-cru Doc()+0x10a99
