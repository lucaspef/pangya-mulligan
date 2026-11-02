#include "minatl.h"
#include "projectg.h"
#include "w3dspr.h"
#include "lensflare.h"

CLensFlare::w_flare_set CLensFlare::ms_lenzTable[7] = {
	{ 0, 0.5f,   0.03f,  0xff0b0b0f },
	{ 0, 0.4f,   0.05f,  0xff0b0b0f },
	{ 0, 0.2f,   0.032f, 0xff1d1008 },
	{ 0, -0.47f, 0.03f,  0xff110901 },
	{ 0, -0.65f, 0.06f,  0xff050f0c },
	{ 0, -1.0f,  0.2f,   0xff110e01 },
	{ 1, -1.3f,  0.4f,   0xff191919 },
};


static const char* s_overlayFile[] = { "~lensflare01.bmp", "~lensflare02.bmp" };
static const char* s_sprFile[] = { "~glaring_yellowish.bmp", "~sun.bmp",
	"~lensflare03.bmp", "~lensflare04.bmp" };

CLensFlare::CLensFlare()
{
	memset(m_overlay, 0, sizeof(m_overlay));
	memset(m_spr, 0, sizeof(m_spr));
}

CLensFlare::~CLensFlare()
{
	int i;
	for (i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_overlay[i])
		{
			g_resrcmng->Release(m_overlay[i]);
			m_overlay[i] = NULL;
		}
	}

	for (i = 0; i < 4; i++)
	{
		if (g_resrcmng && m_spr[i])
		{
			g_resrcmng->Release(m_spr[i]);
			m_spr[i] = NULL;
		}
	}
}

void CLensFlare::Init(const WVector& sunDir)
{
	float scale[4];

	// 983 006f9220: d08dd0/d08974/cfe8d4/d091f8 (645: 8, 1.5, 1.5, 1)
	scale[0] = 16.0f;
	scale[1] = 1.5f;
	scale[2] = 3.0f;
	scale[3] = 0.75f;

	for (unsigned int i = 0;
		i < sizeof(s_overlayFile) / sizeof(s_overlayFile[0]); i++)
	{
		m_overlay[i] = g_resrcmng->GetOverlay(s_overlayFile[i], 0);
		m_overlay[i]->SetCoordMode(0x2200);
	}

	for (unsigned int j = 0; j < sizeof(s_sprFile) / sizeof(s_sprFile[0]); j++)
	{
		m_spr[j] = g_resrcmng->Get3DSpr(s_sprFile[j], 0);
		int tex = g_resrcmng->LoadTexture(s_sprFile[j], 0, 0, 0);
		float width = g_resrcmng->GetTextureWidth(tex) * scale[j];
		float height = g_resrcmng->GetTextureHeight(tex) * scale[j];

		m_size[j].w = width;
		m_size[j].h = height;
		m_spr[j]->SetRect(width, height, width * 0.5f, height * 0.5f); // 983 006f9339: so no Init

		if (g_resrcmng && tex)
		{
			g_resrcmng->Release(tex);
			tex = 0;
		}
	}

	SetSunDirection(sunDir);
	m_angle = 0.0f;
}

void CLensFlare::SetSunDirection(const WVector& sunDir)
{
	m_sunDir = sunDir;
	m_sunDir.Normalize();
	m_alpha = Wabs(m_sunDir.y) * 0.8f + 0.2f;
	m_spr[2]->SetColor(((int)(255.0f * m_alpha) << 24) | 0xffffff);
	m_spr[3]->SetColor(((int)(m_alpha * 255.0f) << 24) | 0xffffff);
}

void CLensFlare::Process(float delta)
{
	m_angle += delta * g_PI;

	if (m_angle > 376.99112f)
		m_angle -= 376.99112f;
}

// IsInRect inlinado no 983 (006f8eb0 e no 1o teste do Render)
static __forceinline bool InRect983(const WRect& r, float x, float y)
{
	return r.x <= x && x <= r.x + r.w && r.y <= y && y <= r.y + r.h;
}

// 983 006f8e00: posicao do sol = sunDir * 1000 + pivot da camera, projetada
void CLensFlare::ProjectSun983(WView* view, WVector* pos, WTVertex* p)
{
	*pos = m_sunDir * 1000.0f + view->camera.pivot;
	view->Projection2(p, *pos);
}

// 983 006f8eb0: o ponto do sol cai em [-0.7*size, 1.4*size + tela] (m_size[1])?
bool CLensFlare::IsSunInSize983(WView* view)
{
	WVector pos;
	WTVertex p;
	ProjectSun983(view, &pos, &p);

	WRect rect(m_size[1].w * -0.7f, m_size[1].h * -0.7f,
		m_size[1].w * 1.4f + view->GetWidth(),
		m_size[1].h * 1.4f + view->GetHeight());
	return InRect983(rect, p.sx, p.sy);
}

// 983 006f93a0: so com o sol na frente da camera; o sol fica a 1000 do pivot,
// entao o clip far do ceu (CalcClipFarByView) cortava os sprites pequenos
void CLensFlare::Render(WView* view)
{
	const WVector& za = view->camera.za;
	if (za.z * m_sunDir.z + za.x * m_sunDir.x + za.y * m_sunDir.y <= 0.0f)
		return;

	float clipNear = view->GetClipNearValue();
	float clipFar = view->GetClipFarValue();
	view->SetClip(clipNear, 2000.0f, false);
	view->UpdateCamera();
	RenderSprites983(view);
	view->SetClip(clipNear, clipFar, false);
	view->UpdateCamera();
}

// 983 006f8f80: sem teste de clip/clima; o laco de overlays do 645 virou RenderOverlay983 (006f9420)
void CLensFlare::RenderSprites983(WView* view)
{
	float width = view->GetWidth();
	float height = view->GetHeight();

	WVector pos;
	WTVertex p;
	ProjectSun983(view, &pos, &p);

	WRect rect(width * -1.4f, height * -1.4f, width * 3.8f, height * 3.8f);
	WRect rect2(width * -0.4f, height * -0.4f, width * 1.8f, height * 1.8f);

	if (!InRect983(rect, p.sx, p.sy))
		return;

	m_spr[0]->SetPos(pos);
	m_spr[0]->Render(view, 0x20b00000, W3dSpr::CAMERA_XY_ALIGN);

	if (!rect2.IsInRect(p.sx, p.sy))
		return;

	m_spr[1]->SetPos(pos);
	m_spr[1]->Render(view, 0x20800000, W3dSpr::CAMERA_XY_ALIGN);

	m_spr[2]->SetPos(pos);
	m_spr[2]->Render(view, 0x20800000, W3dSpr::CAMERA_XY_ALIGN);

	if (!IsSunInSize983(view))
		return;

	float angle[5];
	angle[0] = m_angle / 20.0f + 1.4137167f;
	angle[1] = 0.78539819f - m_angle / 20.0f;
	angle[2] = m_angle / 30.0f + 3.926991f;
	angle[3] = 2.984513f - m_angle / 30.0f;
	angle[4] = m_angle / 60.0f;

	m_spr[3]->SetPos(pos);

	for (unsigned int i = 0; i < 5; i++)
	{
		float scale = ((rand() % 800 - 400) / 1000.0f + 1.0f) * 2.0f;
		float h = m_size[3].h * scale; // 983 006f918c: +0x44/+0x48
		float w = m_size[3].w * scale;
		m_spr[3]->SetRect(w, h, w * 0.5f, h * 0.5f);
		m_spr[3]->Render(view, angle[i], 0x20800000);
	}
}

// 983 006f9420 (de CSky 007060f0 <- CSceneManager::Render 0x200, depois do Flush(0, 1)): as 7 manchas 2D,
// sem SetClip; tamanho = altura da tela * escala (645: largura * largura / 640) e flag 0x800000 (645: 0xc00000)
void CLensFlare::RenderOverlay983(WView* view)
{
	const WVector& za = view->camera.za;
	if (za.z * m_sunDir.z + za.x * m_sunDir.x + za.y * m_sunDir.y <= 0.0f)
		return;
	if (!IsSunInSize983(view))
		return;

	float width = view->GetWidth();
	float height = view->GetHeight();

	WVector pos;
	WTVertex p;
	ProjectSun983(view, &pos, &p);

	WRect src(0.0f, 0.0f, 1.0f, 1.0f); // 983 00e4296c
	WRect dst;
	float dx = p.sx / width - 0.5f;
	float dy = p.sy / height - 0.5f;

	for (int j = 0; j < 7; j++)
	{
		WOverlay* overlay = m_overlay[ms_lenzTable[j].overlay];
		float size = ((rand() % 500) / 10000.0f + 1.0f) * ms_lenzTable[j].scale;
		dst.w = height * size;
		dst.h = dst.w;
		dst.x = (ms_lenzTable[j].pos * dx + 0.5f) * width - dst.w * 0.5f;
		dst.y = (ms_lenzTable[j].pos * dy + 0.5f) * height - dst.w * 0.5f;
		overlay->Render(view, src, dst, 0x800000, ms_lenzTable[j].color, 0.0f, 0);
	}
}
