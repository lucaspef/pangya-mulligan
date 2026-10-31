#include "minatl.h"
#include "ground_info.h"
#include "clientsetting.h"
#include "club.h"
#include "quadtree.h"
#include "golfball.h"
#include "wresrcmng.h"
#include "wbone.h"
#include "mathconsts.h"
#include "jp/uiframesetmanager.h"

// HACK: CQuadTree methods not declared in quadtree.h are reached through a
// derived helper + /alternatename, because more names in quadtree.h change
// the code generated for projectg.cpp.
class CQuadTreeEx : public CQuadTree
{
public:
	float GetMaxPowerOnGround(const WVector& pos);
	void SetCondition();
};
#pragma comment(linker, \
	"/alternatename:?GetMaxPowerOnGround@CQuadTreeEx@@QAEMABVWVector@@@Z=?GetMaxPowerOnGround@CQuadTree@@QAEMABVWVector@@@Z")
#pragma comment(linker, \
	"/alternatename:?SetCondition@CQuadTreeEx@@QAEXXZ=?SetCondition@CQuadTree@@QAEXXZ")
#define PVSEX() (static_cast<CQuadTreeEx&>(GetPVS()))

// HACK: views of the attacher fields of CFxSpray/CFxSequence, because naming
// them in fxspray.h/fxsequence.h changes the code generated for projectg.cpp.
struct sFxSprayAttach
{
	unsigned char m_unused0[8];
	const Attacher* pAttacher;
	float x;
	float y;
	WMatrix* mat[4];
	WVector* pos[4];
};

struct sFxSequenceAttach
{
	unsigned char m_unused0[0x14];
	const Attacher* pAttacher;
	float x;
	float y;
};

namespace
{
	// HACK: the original translation unit also referenced BAR_START.
	inline bool IsBarStart(float f)
	{
		return f == BAR_START;
	}
}

// 983: WView::GetFOV é a função 00b4fa30 (wview.cpp), sem cópia inline aqui

extern WView* g_view;
extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;

// 983 00527220: carrega a fonte, o buffer de 0x20 e as imagens "_x2"/"_x3" (006e2600); Process/Display/Reset do 983 sao
// vazios (00525e10, 005272c0, 00525940), como os herdados.
// Sem corpo, o construtor era stub no exe e a vtable nunca era posta (chamada virtual no membro m_maxPowerInfo = lixo).
CGroundMaxPowerInfo::CGroundMaxPowerInfo()
{
	Init("[font_condition.jpg", 0);
	Buffer() = new char[0x20];
	m_font.AddScaled(2.0f, "[font_condition_x2.jpg");
	m_font.AddScaled(3.0f, "[font_condition_x3.jpg");
}

// 983 00529110 (destrutor de apagar): so o do CFontInfo
CGroundMaxPowerInfo::~CGroundMaxPowerInfo()
{
}

// 983 00539d00: cada letra vira um quadrado no cilindro de raio `radius` e altura `height` em volta do anel, centrado no
// angulo `angle`; a largura angular do texto e 0.1 * largura / (pi * escala da tela). Desenha 2 vezes (+0 e +pi).
void CGroundMaxPowerInfo::PrintOnRing(int percent, float radius, float height, float angle, const WMatrix& mat)
{
	static WTVertex vtx[4];
	static WTVertex* vl[5] = { &vtx[0], &vtx[1], &vtx[3], &vtx[2], NULL };

	sprintf(Buffer(), "%d%%", percent);
	unsigned int len = strlen(Buffer());
	if (len == 0 || len > 4)
	{
		return;
	}
	float charX[4], charW[4], end[4], start[4];
	float width = GetScaledTextWidth(4, Buffer(), -3.0f, 1.0f, charX, charW, 1);
	CUiFrameSetManager::Dim dim;
	dim.Set(1.0f, 4);
	float vh = g_view->GetHeight();
	float vw = g_view->GetWidth();
	float screen = dim.Calc(CUiFrameSetManager::Size((float)(vw / 640.0), (float)(vh / 480.0)));
	float arc = (float)(0.1 * width) / (float)((double)g_PI * screen);
	for (unsigned int i = 0; i < len; i++)
	{
		start[i] = (float)((charX[i] / width - 0.5) * arc + angle);
		end[i] = charW[i] * arc / width + start[i];
	}
	for (int k = 0; k < 4; k++)
	{
		vtx[k].diffuse = 0xff00ff96;
	}
	float half = (float)(height * 0.5);
	WTitleFont* font = NULL;
	float fontScale = 1.0f;
	m_font.FindScaled(screen, &font, &fontScale);
	for (unsigned int pass = 0; pass < 2; pass++)
	{
		for (unsigned int i = 0; i < len; i++)
		{
			int tex;
			float uv[4];
			font->GetCharTexUV983(Buffer()[i], &tex, uv);
			for (int k = 0; k < 4; k++)
			{
				float a = ((k & 1) ? end[i] : start[i]) + (pass ? g_PI : 0.0f);
				WVector v = RotMat(a, 1).za * radius;
				v.y = (k & 2) ? -half : half;
				vtx[k].SetPosition(v * mat);
				vtx[k].tu = (k & 1) ? uv[0] + uv[2] : uv[0];
				vtx[k].tv = (k & 2) ? uv[1] + uv[3] : uv[1];
			}
			g_view->DrawPolygonFan(vl, tex & 0x7ff | 0x300000, 0, 0);
		}
	}
}

// 983 006e46d0: posicao e camera pelo ResetUiPosition (medidas do CUiFrameSetManager), nao mais 58 / barra + 4
CBase::CBase()
	: m_pCurBall(NULL), m_texLine(0), m_uiX(0.0f), m_uiY(0.0f)
{
	ResetUiPosition();
	Load();
	m_texLine = g_resrcmng->LoadTexture("{condition_line.png", 0, 0, 0);
	m_spin.x = m_spin.y = 0.0f;
	m_angle = 0.0f;
	m_light.type = 2;
	m_rot = 0.0f;
	m_light.ambient = 0x808080;
	m_light.diffuse = 0xffffff;
	m_light.nearOne = WVector(-1.0f, -1.0f, 1.0f).Normalize();
	m_normal.Reset();
	m_matrix.Reset();
	m_vl[0] = &m_vtx[0];
	m_vl[1] = &m_vtx[1];
	m_vl[2] = &m_vtx[3];
	m_vl[3] = &m_vtx[2];
	m_vl[4] = NULL;
	m_penalty = 0;
	m_segments = 12;
	m_fx.Init(g_resrcmng);
	m_fx.SetCamera(&g_camera);
	m_fx.m_frameTime = 1000.0f / 30.0f;
}

CBase::~CBase()
{
	std::list<sBall*>::iterator it;
	for (it = m_balls.begin(); it != m_balls.end(); it++)
	{
		if (*it)
		{
			if (g_resrcmng && (*it)->pPuppet)
			{
				g_resrcmng->Release((*it)->pPuppet, false);
				(*it)->pPuppet = NULL;
			}
			delete *it;
		}
	}
	m_balls.clear();
	if (m_texLine)
	{
		g_resrcmng->Release(m_texLine);
		m_texLine = 0;
	}
	m_fx.ClearAll(false);
}

void CBase::Load()
{
	std::list<sBall*>::iterator it;
	for (it = m_balls.begin(); it != m_balls.end(); it++)
	{
		if (*it)
		{
			if (g_resrcmng && (*it)->pPuppet)
			{
				g_resrcmng->Release((*it)->pPuppet, false);
				(*it)->pPuppet = NULL;
			}
			delete *it;
		}
	}
	m_balls.clear();
	m_fx.ClearAll(false);

	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state != 3)
			AddBall(Doc()->m_userInfo[i].userEquip.tidBall);
	}
}

// 983 006e4110: centro da bola pelas medidas "d_ball_center_x/y" (tela / 640x480); sem elas fica o valor anterior.
// A distancia da camera e -1.8 corrigida pela altura da tela (ScaleForView983), nao mais pela largura / 640.
void CBase::ResetUiPosition()
{
	float vh = g_view->GetHeight();
	float vw = g_view->GetWidth();
	CUiFrameSetManager::Size scl((float)(vw / 640.0), (float)(vh / 480.0));
	CUiFrameSetManager::NamedDim* x = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* y = CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
	if (x != NULL && y != NULL)
	{
		m_uiX = x->Calc(scl);
		m_uiY = y->Calc(scl);
	}
	m_fx.SetScreenCenter(m_uiX, m_uiY);
	m_camera.Reset();
	float pz = CUiFrameSetManager::Instance()->ScaleForView983(g_view, -1.8f);
	m_camera.pivot.x = m_camera.pivot.y = 0.0f;
	m_camera.pivot.z = pz;
}

// HACK: other units still name the local static that stood for this literal
// while Process was naked asm.
#pragma comment(linker, \
	"/alternatename:?rb_r_41490fdb@?1??Process@CBase@@QAEXM@Z@4MB=__real@41490fdb")

void CBase::Process(float dt)
{
	if (GOLFDOC()->m_tutorialMode >= 12)
	{
		if (GolfBall().m_playFrame < 0)
		{
			AfxGetTask()->GetActor("Spin") << MsgObject(NULL, M983(0x161),
				(int)&m_spin.x, (int)&m_spin.y, 0, 0, 0);
			m_spin *= dt * 12.566371f;
			if (GolfClub().GetType() != 3)
				m_matrix *= RotMat(m_spin.x, 1) * RotMat(m_spin.y, 0);
		}
		else
		{
			m_matrix = RotMat(GolfBall().m_curveRot, 1) *
				RotMat(GolfBall().m_spinRot, 0);
		}
		m_angle += dt * 0.31415927f;
		dt *= 1000.0f;
		m_fx.ProcessAttacher(dt);
		m_fx.Process(dt);
	}
}

void CBase::Display()
{
	float clipNear = g_view->GetClipNearValue();
	clipNear = Max(0.0f, clipNear);
	float clipFar = g_view->GetClipFarValue();
	// 983 006e37c0: projeção perspectiva e FOV fixo de 90° durante o desenho (repostos no fim)
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float fov = g_view->GetFOV();
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetFOV(1.5707964f);
	g_view->SetClip(0.1f, 10.0f, false);
	g_view->SetCamera(m_camera);
	g_view->SetScreenCenter(m_uiX, m_uiY);
	g_view->UpdateCamera();

	if (m_pCurBall && m_pCurBall->pPuppet)
	{
		m_pCurBall->pPuppet->SetMatrix(m_matrix);
		m_pCurBall->pPuppet->Transform(NULL, 1.0f, 0);
		m_pCurBall->pPuppet->UpdateLightSource(&m_light, false, NULL);
		m_pCurBall->pPuppet->Render(g_view, true, 1.0f, true, true, false);
	}

	if (GolfBall().m_playFrame < 0)
		DrawRing();

	m_fx.Display(g_view);
	g_view->Flush(0, 1); // B1582: 983 006e37c0
	g_view->SetProjectionMode(oldMode);
	g_view->SetCamera(g_camera);
	g_view->SetClip(clipNear, clipFar, false);
	g_view->SetFOV(fov);
	g_view->SetScreenCenter(g_view->GetWidth() * 0.5f,
		g_view->GetHeight() * 0.5f);
	g_view->UpdateCamera();
}

// 983 006e3210: o numero da penalidade (textura "[font_conditoin.jpg" do 645) virou o "NN%" do m_maxPowerInfo;
// os segmentos 0 e 1 de cada metade ficam vazios (o texto passa por eles) e os demais levam a linha
void CBase::DrawRing()
{
	g_view->SetScreenCenter(m_uiX, m_uiY);
	g_view->UpdateCamera();

	int segs;
	int penalty;
	if (GolfClub().GetType() == 3)
	{
		segs = 12;
		penalty = 0;
	}
	else
	{
		segs = m_segments;
		penalty = m_penalty;
	}

	WMatrix base = GetBaseMatrix();
	m_maxPowerInfo.PrintOnRing(100 - penalty, 0.17f, 0.102f, (float)((double)(g_PI * 2.0f) / (float)segs), base);

	for (int i = 0; i < segs; i++)
	{
		if (i % (segs / 2) < 2)
		{
			continue;
		}
		for (int j = 0; j < 4; j++)
		{
			WVector v;
			v = RotMat((float)((float)((j & 1) + i) * ((double)(g_PI * 2.0f)) / (float)segs), 1).za * 0.17f;
			v.y = (j & 2) ? -0.051f : 0.051f;
			m_vtx[j].SetPosition(v * base);
			m_vtx[j].diffuse = 0xff00ff96;
			m_vtx[j].tu = (j & 1) ? 1.0f : 0.0f;
			m_vtx[j].tv = (j & 2) ? 0.0f : 1.0f;
		}
		g_view->DrawPolygonFan(m_vl, m_texLine & 0x7ff, 0, 0);
	}

	g_view->SetScreenCenter(g_view->GetWidth() * 0.5f,
		g_view->GetHeight() * 0.5f);
	g_view->UpdateCamera();
}

void CBase::SetNormal(const WVector& normal)
{
	m_normal = normal;
	GolfBall().m_groundType =
		GetPVS().GetGroundType(GolfBall().m_pos, true, NULL);
	PVSEX().SetCondition();
	if (GolfBall().m_groundType == 2)
	{
		GOLFDOC()->m_cameraData.pitch = -13.0f * g_DEGTORAD;
	}
	else
	{
		GOLFDOC()->m_cameraData.pitch = 0.0f;
		PLAYER(GOLFDOC()->m_currentPlayer)->putt[GOLFDOC()->m_currentHole - 1] =
			0;
	}
	if (GolfBall().m_groundType == 2)
		m_penalty = 0;
	else
		m_penalty = 100 -
			(int)(PVSEX().GetMaxPowerOnGround(GolfBall().m_pos) * 100.0f +
				0.1f);
	m_segments = m_penalty != 0 ? 16 : 12;
}

// 983 (006e34b0): tipo de chao da bola e penalidade, sem trocar a normal nem a camera (o fim do SetNormal); chamado pela
// mensagem 0x35e do CScreen (so do 983)
void CBase::UpdateGroundPenalty983()
{
	GolfBall().m_groundType =
		GetPVS().GetGroundType(GolfBall().m_pos, true, NULL);
	PVSEX().SetCondition();
	if (GolfBall().m_groundType == 2)
		m_penalty = 0;
	else
		m_penalty = 100 -
			(int)(PVSEX().GetMaxPowerOnGround(GolfBall().m_pos) * 100.0f +
				0.1f);
	m_segments = m_penalty != 0 ? 16 : 12;
}

WMatrix CBase::GetBaseMatrix()
{
	if (0)
		Doc();
	WMatrix m;
	m.ya = m_normal;
	m.xa = WCrossProduct(m.ya, WVector::UNIT_POS_Z);
	m.za = WCrossProduct(m.xa, m.ya);
	m.xa.Normalize();
	m.za.Normalize();
	m.pivot.Reset();
	m = (RotMat(m_angle * -2.0f, 1) * m) * RotMat(m_rot * -1.0f, 1);
	return m;
}

CBase::sBall* CBase::FindBall(unsigned long tid)
{
	std::list<sBall*>::iterator it =
		std::find_if(m_balls.begin(), m_balls.end(), sFindBall(tid));
	if (it != m_balls.end())
		return *it;
	return NULL;
}

CBase::sBall* CBase::AddBall(unsigned long tid)
{
	sBall* ball = FindBall(tid);
	if (ball)
		return ball;
	{
		IFF_STRUCT::sBall* iffBall = ItemManager()->FindBall(tid);
		if (iffBall)
		{
			ball = new sBall;
			ball->pBall = iffBall;
			char name[40];
			sprintf(name, "%s.pet", iffBall->Data);
			ball->pPuppet = g_resrcmng->GetPuppet(name, false, true, false);
			m_balls.push_back(ball);
		}
	}
	return ball;
}

CBase::sBall* CBase::ChangeBall(unsigned long tid)
{
	if (0)
		Load();
	sBall* ball = AddBall(tid);
	m_fx.ClearAll(false);
	if (ball)
	{
		m_pCurBall = ball;
		AttachFx();
	}
	return ball;
}

void CBase::ChangeTurn()
{
	ChangeBall(
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall);
}

void CBase::AttachFx()
{
	if (0)
		FindBall(0);
	if (!m_pCurBall || !m_pCurBall->pBall || !m_pCurBall->pPuppet)
		return;

	switch (m_pCurBall->pBall->c.TypeId)
	{
	case 0x14000001:
	case 0x14000008:
	case 0x14000009:
	case 0x1400000e:
	case 0x1400003c:
	case 0x1400003d:
		return;
	}

	if (m_pCurBall->pBall->Fx[0][0])
	{
		int len = strlen(m_pCurBall->pBall->Fx[0]);
		if (len < 5)
			return;

		Attacher attacher;
		attacher.x = m_uiX;
		attacher.y = m_uiY;

		WBone* bone =
			m_pCurBall->pPuppet->FindBone(m_pCurBall->pBall->FxBone[0]);
		if (bone)
		{
			attacher.mat[0] = &bone->GetMatrix();
		}
		else
		{
			w_bound_box* box;
			if (m_pCurBall->pBall->FxBone[0][0])
				box = m_pCurBall->pPuppet->m_bbList.Find(
					m_pCurBall->pBall->FxBone[0]);
			else
				box = m_pCurBall->pPuppet->m_bbList.Find("center");
			if (box)
			{
				if (box->bone)
					attacher.mat[0] = &box->bone->GetMatrix();
				attacher.pos[0] = &box->info->spherePivot;
			}
		}

		if (!strcmpi(m_pCurBall->pBall->Fx[0] + len - 3, "spr"))
		{
			CFxSpray* spray = m_fx.LoadSpray(m_pCurBall->pBall->Fx[0], NULL);
			if (spray)
			{
				sFxSprayAttach* a = (sFxSprayAttach*)spray;
				a->pAttacher = &attacher;
				a->x = attacher.x;
				a->y = attacher.y;
				memcpy(a->mat, attacher.mat, sizeof(attacher.mat));
				memcpy(a->pos, attacher.pos, sizeof(attacher.pos));
				m_fx.AddSpray(spray);
			}
		}
		else
		{
			CFxSequence* seq =
				m_fx.LoadSequence(m_pCurBall->pBall->Fx[0], true, NULL);
			if (seq)
			{
				sFxSequenceAttach* a = (sFxSequenceAttach*)seq;
				a->pAttacher = &attacher;
				a->x = attacher.x;
				a->y = attacher.y;
				m_fx.AddSequence(seq);
			}
		}
	}
}
