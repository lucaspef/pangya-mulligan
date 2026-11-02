// JP 983: unidade 006b1ee0–006b2bf0 (CTutorGuide). Classe nova do 983; ver include/jp/tutorguide.h. Sessão B55. A faixa é
// compilada com LTCG (os laços das marcas chamam com `ecx` andando e o contador em `eax`/`edx`): os laços ficam DIFERENTE.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "actor.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "wview.h"
#include "woverlay.h"
#include "packet.h"
#include "golfruleholeinone.h"
#include "tutorguide.h"

extern const _WRECT g_rectUnit983_e4296c; // 00e4296c (retângulo de origem padrão)

// modo de jogo do CGolfDoc do 983 (+0x158 = m_gameMode no VC10; A740)
#define GOLFDOC983_GAMEMODE() (GOLFDOC()->m_gameMode)
C_ASSERT_VC10(offsetof(CGolfDoc, m_gameMode) == 0x158);
// tamanho da tela na vista principal (+0x19c largura, +0x1a0 altura)
#define VIEWW983() (*(float*)((char*)g_view + VC10_OFS_BO(0x19c)))
#define VIEWH983() (*(float*)((char*)g_view + VC10_OFS_BO(0x1a0)))

// 006b2060: `WView::InFrustum(const WVector&)` do 983 é embutida (cópia fora de linha nesta unidade); a da frente A é
// declarada fora de linha. Planos em +0xbc.
class WViewTutorJp : public WView
{
public:
	bool InFrustum983(const WVector& pos)
	{
		for (int i = 0; i < 6; ++i)
		{
			float d = frustum[i].x * pos.x + frustum[i].y * pos.y + pos.z * frustum[i].z + frustum[i].dis;
			if (d > 0.0f)
				return false;
		}
		return true;
	}
};
#define VIEW983() ((WViewTutorJp*)g_view)

IMPLEMENT_ACTOR(CTutorGuide, IActor)

void sTutorMark::Release()
{
	m_hIndicator = 0;
	m_hMark = 0;
	if (g_resrcmng && m_pIndicator)
	{
		g_resrcmng->Release(m_pIndicator);
		m_pIndicator = NULL;
	}
	if (g_resrcmng && m_pMark)
	{
		g_resrcmng->Release(m_pMark);
		m_pMark = NULL;
	}
}

void sTutorMark::ResetTime()
{
	m_time = 0;
}

void sTutorMark::Show()
{
	ResetTime();
	m_bShowIndicator = true;
	m_bShowMark = true;
}

void sTutorMark::Hide()
{
	m_bShowIndicator = false;
	m_bShowMark = false;
}

void CTutorGuide::OnInit()
{
}

void CTutorGuide::OnDestroy()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Release();
}

void CTutorGuide::OnDisplay()
{
}

void CTutorGuide::ResetTimeAll()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].ResetTime();
}

void CTutorGuide::ShowAll()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Show();
}

void CTutorGuide::HideAll()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Hide();
}

bool CTutorGuide::IsBefore(float t)
{
	if (m_time < t)
		return true;
	return false;
}

void sTutorMark::Load(int index)
{
	m_hIndicator = g_resrcmng->LoadTexture(MakeStr("%s_%02d.png", "tutor_Indicator", index + 1), 0, 0, NULL);
	m_hMark = g_resrcmng->LoadTexture(MakeStr("%s_%02d.png", "player_mark", index + 1), 0, 0, NULL);
	m_pIndicator = g_resrcmng->GetOverlay(MakeStr("%s_%02d.png", "tutor_Indicator", index + 1), 0);
	m_pMark = g_resrcmng->GetOverlay(MakeStr("%s_%02d.png", "player_mark", index + 1), 0);
	m_rcIndicator = WRect(0.0f, 0.0f, m_pIndicator->GetWidth(), m_pIndicator->GetHeight());
	m_rcMark = WRect(0.0f, 0.0f, m_pMark->GetWidth(), m_pMark->GetHeight());
}

void sTutorMark::Reset()
{
	m_pos.z = 0;
	m_bActive = false;
	m_pos.y = 0;
	m_pos.x = 0;
	m_bShowIndicator = false;
	m_bShowMark = false;
	m_rcIndicator.x = 0;
	m_rcIndicator.y = 0;
	m_rcMark.x = 0;
	m_rcMark.y = 0;
}

void sTutorMark::Set(const WVector& pos, int index)
{
	ResetTime();
	m_bActive = true;
	m_pos = pos;
	m_bShowIndicator = true;
	m_bShowMark = true;
}

// a marca fica em cima do ponto e o indicador em cima da marca (os dois centrados em x)
void sTutorMark::UpdateScreenPos()
{
	WVector p = g_view->Projection(m_pos);
	m_rcIndicator.x = p.x - m_rcIndicator.w * 0.5;
	m_rcIndicator.y = p.y - m_rcIndicator.h;
	m_rcMark.x = p.x - m_rcMark.w * 0.5;
	m_rcMark.y = p.y - m_rcMark.h;
}

// quadrado na profundidade do ponto, deslocado para cima pela altura de `rc` (só `y`, `w` e `h` são usados)
void sTutorMark::Draw(int hTex, const WRect* rc, unsigned long color)
{
	WTVertex tv;
	g_view->Projection2(&tv, m_pos);
	if (tv.z <= 0.0f || tv.z >= 1.0f)
		return;
	WVector p = g_view->Projection(m_pos);
	float off = p.y - (rc->y + rc->h + rc->h * 0.5);
	m_vtx[0].diffuse = color;
	m_vtx[1].diffuse = color;
	m_vtx[2].diffuse = color;
	m_vtx[3].diffuse = color;
	m_vtx[0].z = tv.z;
	m_vtx[0].rhw = tv.rhw;
	m_vtx[1].z = tv.z;
	m_vtx[1].rhw = tv.rhw;
	m_vtx[2].z = tv.z;
	m_vtx[2].rhw = tv.rhw;
	m_vtx[3].z = tv.z;
	m_vtx[3].rhw = tv.rhw;
	float y = tv.y + off;
	float w = rc->w;
	float h = rc->h;
	for (int i = 0; i < 4; ++i)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
		m_vtx[i].x = (i & 2) ? tv.x + w * 0.5 : tv.x - w * 0.5;
		m_vtx[i].y = (i & 1) ? y - h : y;
	}
	if (m_vtx[0].x > VIEWW983() - 0.5)
		return;
	if (m_vtx[2].x < 0.5f)
		return;
	if (m_vtx[0].y < 0.5f)
		return;
	if (m_vtx[1].y > VIEWH983() - 0.5)
		return;
	g_view->DrawPolygonFan(m_pVtx, (hTex & 0x7ff) | 0x6400000, 0, true);
}

void CTutorGuide::OnPreLoadInit()
{
	CSceneManager::Instance()->RegisterElement(OBJ_EFFECT, NULL, this,
		"\xc6\xa9\xc5\xcd\xb0\xa1\xc0\xcc\xb5\xe5" /* "튜터가이드" */, 0, NULL);
}

void CTutorGuide::OnLoad()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Load(i);
}

void CTutorGuide::ResetAll()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Reset();
}

void sTutorMark::Init()
{
	m_pVtx[0] = &m_vtx[0];
	m_pVtx[1] = &m_vtx[1];
	m_pVtx[2] = &m_vtx[3];
	m_pVtx[3] = &m_vtx[2];
	m_pVtx[4] = NULL;
	m_hIndicator = 0;
	m_hMark = 0;
	m_pIndicator = NULL;
	m_pMark = NULL;
	Reset();
}

// o indicador balança: sobe (cos(10 t) + 1) * 0,3 + 1 vezes 10 pixels por quadro, fora do modo 0x200
void sTutorMark::Update(float delta)
{
	if (!m_bActive)
		return;
	m_time += delta;
	UpdateScreenPos();
	float s = cosf(m_time * 10.0);
	s = (s + 1.0f) * 0.3f + 1.0f;
	if (VIEW983()->InFrustum983(m_pos) && GOLFDOC983_GAMEMODE() != 0x200)
	{
		m_rcIndicator.y -= s * 10.0;
		m_rcMark.y -= s * 10.0;
	}
}

void sTutorMark::DrawIndicator()
{
	if (!m_bShowIndicator && (GOLFDOC983_GAMEMODE() & 0x200))
		return;
	if (GOLFDOC983_GAMEMODE() & 0x200)
		m_pIndicator->Render(g_view, g_rectUnit983_e4296c, m_rcIndicator, 0, 0xffffffff, 0.0f, 0);
	else
		Draw(m_hIndicator, &m_rcIndicator, 0xffffffff);
}

void sTutorMark::DrawMark()
{
	if (!m_bShowMark && (GOLFDOC983_GAMEMODE() & 0x200))
		return;
	if (GOLFDOC983_GAMEMODE() & 0x200)
		m_pMark->Render(g_view, g_rectUnit983_e4296c, m_rcMark, 0, 0xffffffff, g_DEGTORAD * 0.0, 0);
	else
		Draw(m_hMark, &m_rcMark, 0xffffffff);
}

void CTutorGuide::OnProcess(float delta)
{
	if (g_pMainFrame983->m_gameSpeed == 0.0f)
		return;
	float d = delta / g_pMainFrame983->m_gameSpeed;
	m_time += d;
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Update(d);
}

void CTutorGuide::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x97:
		ResetTimeAll();
		break;
	case 0x1ce:
		ShowAll();
		break;
	case 0x30f:
		if (!IsBefore(0.0f))
		{
			WVector pos = *(const WVector*)msg.param1;
			WSendPacket packet((enumClientPacket)0x12e);
			packet.EncodeBuffer(&pos.x, 4);
			packet.EncodeBuffer(&pos.y, 4);
			packet.EncodeBuffer(&pos.z, 4);
			packet.Send(TO_GAME);
			m_time = 0;
		}
		break;
	case 0x310:
	{
		WVector pos = *(const WVector*)msg.param1;
		m_mark[msg.param2].Set(pos, msg.param2);
	}
		break;
	case 0x311:
		ResetAll();
		break;
	}
}

void CTutorGuide::InitAll()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Init();
}

void sTutorMark::Display()
{
	if (!m_bActive || !VIEW983()->InFrustum983(m_pos))
		return;
	if (GOLFDOC983_GAMEMODE() & 0x200)
	{
		DrawIndicator();
		DrawMark();
		return;
	}
	if (VIEW983()->InFrustum983(m_pos))
	{
		DrawIndicator();
		DrawMark();
	}
}

CTutorGuide::CTutorGuide()
{
	m_time = 0;
	InitAll();
}

CTutorGuide::~CTutorGuide()
{
}

void CTutorGuide::Display()
{
	for (int i = 0; i < MAX_MARK; ++i)
		m_mark[i].Display();
}
