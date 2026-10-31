#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "capturedbg.h"
#include "scenemanager.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "chatmsg.h"
#include "3dfont.h"
#include "simpleoverlay.h"
#include "wfont.h"
#include "../shared/sharedtables.h"
#include "fallingspot.h"
#include "fontinfo.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fremoticon.h"
extern Fresh* g_pFresh; // 00f02340

// HACK: stands in for an unknown erased inline use of __rtti_obj
static __declspec(thread) int __rtti_tls_pad;

sPlayerData* PLAYER(unsigned char index);
sTeamData* TEAM(unsigned char index);

// HACK: PLAYER is an inline of shareddoc.inl that the original calls out of line in the player quad
// loop of CSpot::Display, where the compiler now expands it: those two calls go through an alias.
sPlayerData* RbFallingSpotPlayer(unsigned char index);
#pragma comment(linker, \
	"/alternatename:?RbFallingSpotPlayer@@YIPAUsPlayerData@@E@Z=?PLAYER@@YIPAUsPlayerData@@E@Z")
bool IsMyTurn(bool check);
unsigned char MyIndex();
const WVector& GetHoleCupPos(); // 983: devolve referencia

const unsigned long CSpot::m_adwColors[5] = { 0xffff0000, 0xffff8a00,
	0xffffde00, 0xff95fb21, 0xff02ff42 };

bool IsValidHole(unsigned char hole);
extern WMatrix g_camera;

bool CClub::IsCalcRandomPower()
{
	return m_bCalcRandomPower;
}

// HACK: Parse the font deletion before the helpers to preserve its COMDAT position.
inline void RbFallingSpotFontDeletion(C3dFont* font)
{
	delete font;
}

float CSpot::GetMovingRate(float t)
{
	return 3.0 * t * t - (t + t) * t * t;
}

void WView::DrawProjPolygonFan(WTVertex** vertices, int count, int flags)
{
	// HACK: Dead reference so this body is emitted after the PCH inline atan2f, as in the original.
	if (0)
		atan2f(0.0f, 0.0f);
	DrawPolygonFan(vertices, count, flags, true);
}

bool WView::InFrustum(const WVector& pos)
{
	for (int i = 0; i < 6; ++i)
	{
		if (frustum[i] * pos > 0.0f)
			return false;
	}
	return true;
}

CSpot::CSpot()
	: m_pFontWind(NULL),
	  m_pFontPlayer(NULL),
	  m_pRangeInfo(NULL),
	  m_texIndicator(0),
	  m_texFalling(0),
	  m_texSlope(0),
	  m_texTarget(0),
	  m_pOvLine(NULL),
	  m_pOvPlayerPos(NULL),
	  m_pSimpleUI(NULL),
	  m_texScore(0),
	  m_pRivals(NULL),
	  m_pFont(NULL),
	  m_pRuler(NULL)
{
	AddSkipList(0x1400);
	m_pVtx[0] = &m_vtx[0];
	m_pVtx[1] = &m_vtx[1];
	// D704: 983 0072ca70 grava +0x8 = 3 (o 645 punha 1): desenha DEPOIS da cena e do DOF (CTask::Display i==2);
	// com 1 a cena cobria a linha/marcador/distancia da vista de cima (DisplayTopView)
	m_dispPriority = DISP_PRIORITY_3;
	m_unusedE8 = 0;
	m_pOvFall1 = NULL;
	m_pVtx[2] = &m_vtx[3];
	m_pVtx[3] = &m_vtx[2];
	m_pOvFall2 = NULL;
}

void CSpot::OnPreLoadInit()
{
	m_bShowPlayers = true;
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, this,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1", 0, NULL);
}

void CSpot::OnDestroy()
{
	if (g_resrcmng && m_pOvLine)
	{
		g_resrcmng->Release(m_pOvLine);
		m_pOvLine = NULL;
	}
	if (g_resrcmng && m_pOvFall1)
	{
		g_resrcmng->Release(m_pOvFall1);
		m_pOvFall1 = NULL;
	}
	if (g_resrcmng && m_pOvFall2)
	{
		g_resrcmng->Release(m_pOvFall2);
		m_pOvFall2 = NULL;
	}
	if (g_resrcmng && m_texIndicator)
	{
		g_resrcmng->Release(m_texIndicator);
		m_texIndicator = 0;
	}
	if (g_resrcmng && m_texFalling)
	{
		g_resrcmng->Release(m_texFalling);
		m_texFalling = 0;
	}
	if (g_resrcmng && m_texSlope)
	{
		g_resrcmng->Release(m_texSlope);
		m_texSlope = 0;
	}
	if (g_resrcmng && m_pOvPlayerPos)
	{
		g_resrcmng->Release(m_pOvPlayerPos);
		m_pOvPlayerPos = NULL;
	}
	if (g_resrcmng && m_texScore)
	{
		g_resrcmng->Release(m_texScore);
		m_texScore = 0;
	}
	if (g_resrcmng && m_texTarget)
	{
		g_resrcmng->Release(m_texTarget);
		m_texTarget = 0;
	}

	if (m_pSimpleUI)
	{
		delete m_pSimpleUI;
		m_pSimpleUI = NULL;
	}
	if (m_pFontPlayer)
	{
		delete m_pFontPlayer;
		m_pFontPlayer = NULL;
	}
	if (m_pFontWind)
	{
		delete m_pFontWind;
		m_pFontWind = NULL;
	}
	if (m_pRangeInfo)
	{
		delete m_pRangeInfo;
		m_pRangeInfo = NULL;
	}
	if (m_pRivals)
	{
		delete[] m_pRivals;
		m_pRivals = NULL;
	}
	if (m_pRuler)
	{
		delete m_pRuler;
		m_pRuler = NULL;
	}
}

CSpot::sRivalPos::sRivalPos()
{
}

void CSpot::OnInit()
{
	m_color = 0;
	m_angle = 0.0f;
	m_rotAngle = 0.0f;
	m_pos.z = 0.0f;
	m_pos.y = 0.0f;
	m_pos.x = 0.0f;
	m_basePos.z = 0.0f;
	m_basePos.y = 0.0f;
	m_basePos.x = 0.0f;
	m_oldPos.z = 0.0f;
	m_oldPos.y = 0.0f;
	m_oldPos.x = 0.0f;
	m_bActive = true;
	m_bDirty = true;
	m_rate = 1.0f;
	m_markSize = 2.0f;
	m_markSize2 = 12.0f;
	m_w = 20.0f;
	m_h = 20.0f;
	m_lerp = 0.0f;
	m_alpha = 0.0f;
	m_rotTimer = 0.0f;
	m_anim = 0.0f;
	m_animDir = 1.0f;
	m_offset0.z = 0.0f;
	m_offset0.y = 0.0f;
	m_offset0.x = 0.0f;

	float a = Random(-3.1415927f, 3.1415927f);
	WVector dir;
	dir.x = cosf(a);
	dir.z = sinf(a);
	float r = Random(0.0f, (GolfClub().GetRange() * 3.2f) * 0.02f);
	WVector v(dir.x * r, 0.0f * r, r * dir.z);
	m_offset1 = v;
}

void CSpot::ClipIndicator(int index)
{
	if (m_vtx[index].x < 0.5f)
	{
		m_vtx[index].tu = (0.5f - m_vtx[index].x) / m_w + m_vtx[index].tu;
		m_vtx[index].x = 0.5f;
	}
	else if (m_vtx[index].x > g_view->GetWidth() - 0.5f)
	{
		m_vtx[index].tu = m_vtx[index].tu -
			((m_vtx[index].x - g_view->GetWidth()) + 0.5f) / m_w;
		m_vtx[index].x = g_view->GetWidth() - 0.5f;
	}

	if (m_vtx[index].y < 0.5f)
	{
		m_vtx[index].tv = (0.5f - m_vtx[index].y) / m_h + m_vtx[index].tv;
		m_vtx[index].y = 0.5f;
	}
	else if (m_vtx[index].y > g_view->GetHeight() - 0.5f)
	{
		m_vtx[index].tv = m_vtx[index].tv -
			((m_vtx[index].y - g_view->GetHeight()) + 0.5f) / m_h;
		m_vtx[index].y = g_view->GetHeight() - 0.5f;
	}
}

bool IsValidHole(unsigned char hole)
{
	for (unsigned char i = 0; i < CSharedDoc::Instance()->m_golfGame.holes; i++)
	{
		if (CSharedDoc::Instance()->m_holeOrder[i] == hole)
			return true;
	}
	return false;
}

IMPLEMENT_ACTOR(CSpot, IActor)

// 983: inline em OnDisplay (0072c430 chama InFrustum 3x e sqrt 2x direto)
__forceinline bool CSpot::_ValidTopViewLineDraw()
{
	WView* view = g_view;
	if (!view->InFrustum(GolfBall().m_pos))
	{
		if (!view->InFrustum(m_pos))
		{
			WVector cam = g_camera.pivot;
			float ballZ = GolfBall().m_pos.z;
			float dx = cam.x - GolfBall().m_pos.x;
			float dz = cam.z - ballZ;
			float px = m_pos.x - GolfBall().m_pos.x;
			float pz = m_pos.z - ballZ;
			if (sqrtf(px * px + pz * pz) < sqrtf(dx * dx + dz * dz))
				return false;
		}
	}
	return true;
}

void CSpot::DrawRange(bool bClose)
{
	// 983 00728711: so o putter (tipo 3) pula; o teste de gameType 3 era do 645
	if (GolfClub().GetType() == 3)
		return;

	float range = (GolfClub().GetRange() * 3.2f) * 0.05f;
	float h = range * 0.1f;
	for (int i = 0; i < 4; i++)
	{
		int hi = i & 2;
		float sx = hi ? 1.0f : -1.0f;
		WVector v;
		v.x = sx * m_alpha * range;
		v.y = h;
		int lo = i & 1;
		float sz = lo ? 1.0f : -1.0f;
		v.z = sz * m_alpha * range;
		v = v * RotMat(m_rotAngle * 0.7f, 1);
		m_vtx[i].x = v.x + m_pos.x;
		m_vtx[i].y = v.y + m_pos.y;
		m_vtx[i].z = v.z + m_pos.z;
		m_vtx[i].tu = hi ? 1.0f : 0.0f;
		m_vtx[i].tv = lo ? 0.0f : 1.0f;
		unsigned long alpha = (unsigned long)((1.0f - m_alpha) * 255.0f);
		m_vtx[i].diffuse = (alpha << 24) | 0xffffff;
	}
	g_view->DrawPolygonFan(m_pVtx, (m_texFalling & 0x7ff) | (bClose ? 0xc00000 : 0x800000), 0x200,
		false);
}

void CSpot::Rotate()
{
	m_color = 0;
	WVector d = (GolfClub().GetRange() * 3.2f) * RotMat(m_angle, 1).za;
	WVector p(d.x + GolfBall().m_pos.x, d.y + GolfBall().m_pos.y,
		d.z + GolfBall().m_pos.z);
	m_basePos = GetPVS().GetGroundPoint(p, true, (int*)&m_color, true);
	if (WisEqual(m_lerp, 0.0f, g_EPSILON))
		m_pos = m_basePos;
}

void CSpot::DrawHoleIndicator(bool bClose)
{
	WVector pin = GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].pin;
	WVector a;
	WVector b;
	a.x = pin.x - m_pos.x;
	a.y = 0.0f;
	b.y = 0.0f;
	b.x = pin.z - m_pos.z;
	b.z = a.x * -1.0f;
	float f = (sqrtf(b.x * b.x + a.x * a.x) - 32.0f) * 0.015625f;
	float t0 = Between<float>(0.0f, f, 1.0f) * 229.5f;
	float g = 1.0f -
		sqrtf((g_camera.xm - m_pos.x) * (g_camera.xm - m_pos.x) +
			(g_camera.ym - m_pos.y) * (g_camera.ym - m_pos.y) +
			(g_camera.zm - m_pos.z) * (g_camera.zm - m_pos.z)) *
			0.0015625f;
	float t1 = Between<float>(0.0f, g, 1.0f) * 229.5f;
	a.z = b.x;
	a.Normalize();
	b.Normalize();
	for (int i = 0; i < 4; i++)
	{
		WVector p = m_pos;
		p.y = p.y + 2.0f;
		float k = 1.0f;
		if ((i & 2) == 0)
			k = -1.0f;
		float sgn = (i & 1) ? 1.0f : -1.0f;
		WVector off = (b * k + sgn * a) * 10.0f;
		float z = m_pos.z + off.z;
		m_vtx[i].z = z;
		m_vtx[i].x = off.x + p.x;
		m_vtx[i].y = off.y + p.y;
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 0.0f : 1.0f;
		m_vtx[i].diffuse = ((unsigned char)(int)Min<float>(t0, t1) << 24) |
			(m_color2 & 0xffffff);
	}
	g_view->DrawPolygonFan(m_pVtx, (m_texTarget & 0x7ff) | (bClose ? 0x400000 : 0), 0x200, false);
}

void CSpot::DrawIndicator(const WVector& pos, int color)
{
	WTVertex v;
	g_view->Projection2(&v, pos);
	if (v.sz <= 0.0f || v.sz >= 1.0f)
		return;
	// 983: profundidade normalizada entre os planos de corte nos 4 vertices (vz)
	float vz = (TransformZ(pos, g_view->invcamera) - g_view->GetClipNearValue()) /
		(g_view->GetClipFarValue() - g_view->GetClipNearValue());
	for (int k = 0; k < 4; k++)
		m_vtx[k].vz = vz;
	m_vtx[0].rhw = v.rhw;
	m_vtx[0].sz = v.sz;
	m_vtx[0].diffuse = color;
	m_vtx[1].diffuse = color;
	m_vtx[2].diffuse = color;
	m_vtx[3].diffuse = color;
	m_vtx[1].sz = v.sz;
	m_vtx[1].rhw = v.rhw;
	m_vtx[2].sz = v.sz;
	m_vtx[2].rhw = v.rhw;
	m_vtx[3].sz = v.sz;
	m_vtx[3].rhw = v.rhw;
	float hw = m_w * 0.5f;
	m_vtx[0].tv = 1.0f;
	float left = v.sx - hw;
	m_vtx[2].tu = 1.0f;
	m_vtx[2].tv = 1.0f;
	m_vtx[3].tu = 1.0f;
	m_vtx[0].sx = left;
	m_vtx[0].tu = 0.0f;
	m_vtx[1].sx = left;
	m_vtx[0].sy = v.sy;
	m_vtx[1].tu = 0.0f;
	float top = v.sy - m_h;
	m_vtx[1].tv = 0.0f;
	m_vtx[2].sy = v.sy;
	m_vtx[3].tv = 0.0f;
	float right = hw + v.sx;
	m_vtx[1].sy = top;
	m_vtx[3].sy = top;
	m_vtx[2].sx = right;
	m_vtx[3].sx = right;
	if (left > g_view->GetWidth() - 0.5f || right < 0.5f || v.sy < 0.5f ||
		top > g_view->GetHeight() - 0.5f)
		return;
	for (int i = 0; i < 4; i++)
		ClipIndicator(i);
	g_view->DrawProjPolygonFan(m_pVtx, (m_texIndicator & 0x7ff) | 0x6400000, 0);
}

void CSpot::OnLoad()
{
	m_pRangeInfo = new CClubRangeInfo;
	m_pFontWind = new C3dFont;
	m_pFontWind->LoadTexture("[font_wind.jpg", 16, 16,
		"1234567890ym%-.?/:", 3.0f, -30.0f);
	m_pFontPlayer = new C3dFont;
	m_pFontPlayer->LoadTexture("[nplayer.jpg", 32, 16, "0123rb", 0.0f, -30.0f);
	m_pOvFall1 = g_resrcmng->GetOverlay("[fall_indicator1.png", 0);
	m_pOvFall2 = g_resrcmng->GetOverlay("[fall_indicator2.png", 0);
	m_pOvLine = g_resrcmng->GetOverlay("spot_line.bmp", 0);
	m_pOvPlayerPos = g_resrcmng->GetOverlay("{player_pos.bmp", 0);

	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	case 3:
	case 7:
		m_pSimpleUI = new CSimpleUI("[nplayer.jpg");
		m_pSimpleUI->Add("1", WRect(0.0f, 0.0f, 32.0f, 16.0f), false);
		m_pSimpleUI->Add("2", WRect(32.0f, 0.0f, 32.0f, 16.0f), false);
		m_pSimpleUI->Add("3", WRect(0.0f, 16.0f, 32.0f, 16.0f), false);
		m_pSimpleUI->Add("4", WRect(32.0f, 16.0f, 32.0f, 16.0f), false);
		break;
	case 1:
		m_pSimpleUI = new CSimpleUI("[nplayer.jpg");
		m_pSimpleUI->Add("r", WRect(0.0f, 32.0f, 32.0f, 16.0f), false);
		m_pSimpleUI->Add("b", WRect(32.0f, 32.0f, 32.0f, 16.0f), false);
		break;
	case 2:
	case 8:
		break;
	case 6:
	{
		int n = Doc()->m_rivalList.size();
		m_pRivals = new sRivalPos[n];
		for (unsigned char i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			if (Doc()->m_myInfo.info.dwGuildId ==
				Doc()->m_rivalList[i].guildUID)
			{
				m_pRivals[i].rate = 0.0f;
				m_pRivals[i].t = -1.0f;
				m_pRivals[i].hole = Doc()->m_holeOrder[0];
				m_pRivals[i].pos =
					GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].tee;
			}
		}
		break;
	}
	case 4:
	case 5:
	case 9:
	case 10:
	{
		int n = Doc()->m_rivalList.size();
		m_pRivals = new sRivalPos[n];
		for (unsigned char i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			m_pRivals[i].rate = 0.0f;
			if (Doc()->m_golfGame.gameType == 10)
				m_pRivals[i].t = 0.0f;
			m_pRivals[i].hole = Doc()->m_holeOrder[0];
			m_pRivals[i].pos = Doc()->m_rivalList[i].ballPos;
		}
		break;
	}
	case 0xb:
	{
		// 983 OnLoad 00729f80 caso 0xb + 00729680: Hole-in-One aloca os rivais zerados (t = -1); 0x128/0x129 os preenchem
		int n = Doc()->m_rivalList.size();
		m_pRivals = (sRivalPos*)new char[n * sizeof(sRivalPos)];
		ResetRivals();
		break;
	}
	}

	m_texIndicator = g_resrcmng->LoadTexture("{indicator.bmp", 0, 0, NULL);
	m_texFalling = g_resrcmng->LoadTexture("[falling.jpg", 0, 0, NULL);
	m_texSlope = g_resrcmng->LoadTexture("[slope.jpg", 0, 0, NULL);
	m_texTarget = g_resrcmng->LoadTexture("[target_hole_posi.jpg", 0, 0, NULL);
	if (GOLFDOC()->m_tutorialMode < 15)
		m_texScore = g_resrcmng->LoadTexture("[score_target.jpg", 0, 0, NULL);
	m_pFont = CChatMsg::Instance()->GetMaskedFont();
	// 983 00729f80 (fim): regua so no Hole-in-One de sala realGameType 0xe
	if (Doc()->m_golfGame.gameType == 0xb && Doc()->m_roomInfo.realGameType == 0xe)
		m_pRuler = new CHoleRuler983;
}

// 983 007282a0: cor do rival por flags (modo 4)
void __stdcall CSpot::GetRivalColor(unsigned char flags, unsigned long* color)
{
	if ((flags & 2) != 0)
		*color = 0xa0f50000;
	else if ((flags & 8) != 0)
		*color = 0xa0ffd200;
	else if ((flags & 4) != 0)
		*color = 0xa000c0ff;
}

// 983 00728a80: quad horizontal (m_vtx/m_pVtx) de lado 2*size centrado em pos, textura slope
void CSpot::DrawQuad(const WVector& pos, float size, unsigned long color)
{
	float lo = -1.0f * size;
	for (int i = 0; i < 4; i++)
	{
		WVector p = pos;
		p.x += (i & 1) ? size : lo;
		p.z += (i & 2) ? size : lo;
		m_vtx[i].SetPosition(p);
		m_vtx[i].diffuse = color;
		m_vtx[i].tu = (i & 1) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 2) ? 0.0f : 1.0f;
	}
	g_view->DrawPolygonFan(m_pVtx, (m_texSlope & 0x7ff) | 0x400000, 0x200,
		false);
}

// 983 00728c10: numero/letra do jogador no mundo (fonte "0123rb")
void CSpot::DrawLabel(const WVector& pos, unsigned long color, const char* text,
	int flag)
{
	if (CProjectG::Instance()->HidePI())
		return;
	m_pFontPlayer->SetPos(pos);
	m_pFontPlayer->SetColor(color);
	m_pFontPlayer->Printf(g_view, flag, 0x400000, text);
}

// 983 00729620: bola do rival ainda no tee do buraco dele
bool CSpot::IsAtTee(const sRivalData& rv)
{
	const WVector& tee = GOLFDOC()->GetHoleData(rv.hole).tee;
	return rv.ballPos.x == tee.x && rv.ballPos.z == tee.z;
}

// 983 007282a0 e 00729970: cor pulsante do indicador (m_color2)
void CSpot::UpdateColor()
{
	if (m_color == -1)
	{
		m_color2 = 0xffff0000;
		return;
	}
	WVector d = GolfBall().m_pos - m_pos;
	float h = Between<float>(-4.0f, (d.y * 0.3125f) * 0.9144f, 4.0f);
	float t = (h + 4.0f) * 0.5f;
	int idx = (int)(__int64)t;
	float frac = t - (int)(__int64)t;
	if (Wabs(frac) < g_EPSILON)
		m_color2 = m_adwColors[idx];
	else
		m_color2 = ULblend(m_adwColors[idx], m_adwColors[idx + 1],
			(unsigned char)(frac * 255.0f));
}

// 983: condicao comum de 0072a9c0 / 0072b210 (3D e nomes dos rivais), inline nas duas
#define CSPOT_CAN_SHOW_WORLD() \
	(m_bShowPlayers && \
		((GOLFDOC()->m_gameMode & 0x8dca) != 0 || \
			((GOLFDOC()->m_gameMode & 0x200) == 0 && \
				((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2 || \
					(GOLFDOC()->m_gameMode & 0x2000) == 0))))

// 983: condicao por rival (4/5/9/10) de 0072a9c0 / 0072b210; `spect` = realGameType 0x14/0x15
#define CSPOT_RIVAL_VISIBLE(i, spect) \
	((Doc()->m_rivalList[i].state == 0 && \
		 m_pRivals[i].hole == GOLFDOC()->m_currentHole && \
		 0.0f <= m_pRivals[i].t) || \
		((spect) && Doc()->m_rivalList[i].state != 1 && \
			Doc()->m_rivalList[i].state != 3 && \
			Doc()->m_rivalList[i].state != 2 && \
			m_pRivals[i].hole == Doc()->m_rivalList[i].hole))

// 983 00729af0 (primeira parte de Display): spot do tutorial ou puppet do buraco
void CSpot::DisplayTutorial()
{
	if (GOLFDOC()->m_bTutorialPin983 == 0)
		return;
	int mode = GOLFDOC()->m_tutorialMode;
	if (mode == 2 || mode == 6 || mode == 0xb || mode == 10 || mode == 0xd ||
		mode == 0xe)
	{
		float size = (mode == 2) ? 2.0f : 1.0f;
		float e = size * 44.8f;
		WVector c = GOLFDOC()->m_tutorialPin983;
		for (int i = 0; i < 4; i++)
		{
			WVector p = c;
			p.y += 1.0f;
			p.x += (i & 1) ? e : -e;
			p.z += (i & 2) ? e : -e;
			m_vtx[i].SetPosition(p);
			m_vtx[i].diffuse = 0xffffffff;
			m_vtx[i].tu = (i & 1) ? 1.0f : 0.0f;
			m_vtx[i].tv = (i & 2) ? 0.0f : 1.0f;
		}
		g_view->DrawPolygonFan(m_pVtx, (m_texScore & 0x7ff) | 0x400000, 0x200,
			false);
	}
	else
	{
		WMatrix m;
		m = RotMat(m_rotTimer, 1);
		m *= 0.5f;
		m.pivot = GOLFDOC()->m_tutorialPin983;
		GOLFDOC()->m_pCameraTargetPuppet->SetMatrix(m);
		GOLFDOC()->m_pCameraTargetPuppet->Transform(NULL, 1.0f, 0);
		GOLFDOC()->m_pCameraTargetPuppet
			->UpdateLightSource(&g_lightset, false, NULL);
		GOLFDOC()->m_pCameraTargetPuppet
			->Render(g_view, true, 1.0f, false, true, false);
	}
}

// 983 0072a9c0: indicadores 3D dos outros jogadores / times / rivais
void CSpot::DisplayPlayers3D()
{
	if (!CSPOT_CAN_SHOW_WORLD())
		return;
	unsigned char i;
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	case 3:
	case 7:
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (GOLFDOC()->GetPlayer(i)->state == 0 &&
				i != GOLFDOC()->m_currentPlayer &&
				PLAYER(i)->stroke[GOLFDOC()->m_currentHole - 1] != 0)
			{
				unsigned long color = PLAYER(i)->color | 0x80000000;
				DrawQuad(PLAYER(i)->pos, 0.5f, color);
				DrawIndicator(PLAYER(i)->pos, color);
				DrawLabel(PLAYER(i)->pos, color, MakeStr("%d", i), 0);
			}
		}
		break;
	case 1:
		for (i = 0; i < 2; i++)
		{
			if (TEAM(i)->state == 0 &&
				i != PLAYER(GOLFDOC()->m_currentPlayer)->team &&
				TEAM(i)->stroke[GOLFDOC()->m_currentHole - 1] != 0)
			{
				unsigned long color = TEAM(i)->color | 0x80000000;
				DrawQuad(TEAM(i)->pos, 0.5f, color);
				DrawIndicator(TEAM(i)->pos, color);
				DrawLabel(TEAM(i)->pos, color, i == 0 ? "r" : "b", 0);
			}
		}
		break;
	case 4:
	case 5:
	case 9:
	case 10:
	{
		if (CProjectG::Instance()->HidePI())
			break;
		bool spect =
			(unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2;
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (!CSPOT_RIVAL_VISIBLE(i, spect))
				continue;
			if (!(rv.oid != MyGuid(true) || rv.bGrandPrixAI983 != 0 ||
					(Doc()->m_golfGame.gameType == 10 &&
						(GOLFDOC()->m_gameMode & 0x800) != 0)))
				continue;
			float f = m_pRivals[i].t;
			float rate = GetMovingRate(f);
			WVector gp = GetPVS().GetGroundPoint(
				(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
				NULL, true);
			gp.y += GolfBall().m_radius;
			if (!g_view->InFrustum(gp))
				continue;
			unsigned long color;
			switch (Doc()->m_golfGame.gameType)
			{
			case 5:
				color = (rv.team != 0) ? 0x800060ff : 0x80ff0000;
				break;
			case 10:
				if (rv.oid == MyGuid(true))
					color = 0x80ff0000;
				else
					color = (rv.approachTime != -1) ? 0x80ffffff : 0x80505050;
				break;
			case 4:
				color = 0x80ffffff;
				GetRivalColor(rv.shotSpecial983, &color);
				if (spect && rv.bGrandPrixAI983 != 0)
					color = 0x80ffffff;
				break;
			default:
				color = 0x80ffffff;
				break;
			}
			if (spect && IsAtTee(rv))
				continue;
			DrawQuad(gp, 0.5f, color);
			DrawIndicator(gp, color);
		}
		break;
	}
	case 6:
		if (CProjectG::Instance()->HidePI())
			break;
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (rv.state == 0 && rv.oid != MyGuid(true) &&
				m_pRivals[i].hole == GOLFDOC()->m_currentHole &&
				0.0f <= m_pRivals[i].t &&
				Doc()->m_myInfo.info.dwGuildId == rv.guildUID)
			{
				float rate = GetMovingRate(m_pRivals[i].t);
				WVector gp = GetPVS().GetGroundPoint(
					(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
					NULL, true);
				gp.y += GolfBall().m_radius;
				if (!g_view->InFrustum(gp))
					continue;
				unsigned long color =
					(rv.team != 0) ? 0x800060ff : 0x80ff0000;
				DrawQuad(gp, 0.5f, color);
				DrawIndicator(gp, color);
			}
		}
		break;
	}
}

// 983 00729e90 (fim de Display): distancia em jardas, indicador de queda, buraco e alcance
void CSpot::DisplayDistance()
{
	if (!m_bActive || GOLFDOC()->m_tutorialMode < 1 ||
		(GOLFDOC()->m_gameMode & 0x200) != 0)
		return;
	UpdateColor();
	DrawIndicator(m_pos, m_color2);
	WVector d = GolfBall().m_pos - m_pos;
	m_pFontWind->SetColor(m_color2);
	m_pFontWind->SetPos(m_pos);
	float dist = d.z * d.z;
	m_pFontWind->Printf(g_view, -5, 0x400000,
		MakeStr("%.0fy", (double)(sqrtf(dist + d.x * d.x) * 0.3125f)));
	DrawHoleIndicator(true);
	DrawRange(true);
}

void CSpot::Display()
{
	if (Skipped())
		return;
	if (CCapturedBg::Instance()->IsCaptured())
		return;
	// 983 0072c3f0 (como 0072ca30): 00728340 = IS_REVIEW_CAMERA_MODE; na camera de revisao nao desenha o 3D
	bool review = false;
	SendMsg(this, "ShotReview", 0x2fd, (int)"IS_REVIEW_CAMERA_MODE", (int)&review, 0, 0);
	if (review)
		return;
	DisplayTutorial();
	DisplayPlayers3D();
	DisplayDistance();
}

void CSpot::OnDisplay()
{
	if (Skipped())
		return;
	if (CCapturedBg::Instance()->IsCaptured())
		return;
	// B749: 983 0072ca30 chama 00728340 (ShotReview IS_REVIEW_CAMERA_MODE) e nao desenha nomes/2D/vista de cima na camera de revisao
	bool review = false;
	SendMsg(this, "ShotReview", 0x2fd, (int)"IS_REVIEW_CAMERA_MODE", (int)&review, 0, 0);
	if (review)
		return;
	DisplayRivalNames();
	DisplayPlayers2D();
	DisplayTopView();
}

// 983 0072b210: nomes dos rivais (4/5/9/10 e 6) sobre a projecao do ponto 3D
void CSpot::DisplayRivalNames()
{
	if (!CSPOT_CAN_SHOW_WORLD())
		return;
	unsigned char i;
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 9:
	case 10:
	{
		if (CProjectG::Instance()->HidePI())
			break;
		bool spect =
			(unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2;
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (!CSPOT_RIVAL_VISIBLE(i, spect) || (spect && IsAtTee(rv)))
				continue;
			if (!(rv.oid != MyGuid(true) || rv.bGrandPrixAI983 != 0 ||
					(Doc()->m_golfGame.gameType == 10 &&
						(GOLFDOC()->m_gameMode & 0x800) != 0)))
				continue;
			float rate = GetMovingRate(m_pRivals[i].t);
			WVector gp = GetPVS().GetGroundPoint(
				(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
				NULL, true);
			if (!g_view->InFrustum(gp))
				continue;
			gp.y += GolfBall().m_radius;
			WVector sp = g_view->Projection(gp);
			if (Doc()->m_golfGame.gameType == 10 &&
				Doc()->IsCurrentMissionBlind() && rv.oid != MyGuid(false))
				continue;
			m_pFont->Print(g_view,
				sp.x - m_pFont->GetTextWidthOld983(g_view, rv.nickname) * 0.5f,
				sp.y - 32.0f, rv.nickname, 0x20300000, 0xffffffff, NULL);
		}
		break;
	}
	case 6:
		if (CProjectG::Instance()->HidePI())
			break;
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (rv.state == 0 && rv.oid != MyGuid(true) &&
				m_pRivals[i].hole == GOLFDOC()->m_currentHole &&
				0.0f <= m_pRivals[i].t &&
				Doc()->m_myInfo.info.dwGuildId == rv.guildUID)
			{
				float rate = GetMovingRate(m_pRivals[i].t);
				WVector gp = GetPVS().GetGroundPoint(
					(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
					NULL, true);
				if (!g_view->InFrustum(gp))
					continue;
				gp.y += GolfBall().m_radius;
				WVector sp = g_view->Projection(gp);
				m_pFont->Print(g_view,
					sp.x -
						m_pFont->GetTextWidthOld983(g_view, rv.nickname) * 0.5f,
					sp.y - 32.0f, rv.nickname, 0x20300000, 0xffffffff, NULL);
			}
		}
		break;
	}
	m_pFont->Flush(g_view);
}

// 983 0072b8c0: marcadores 2D (vista de cima, modo 0x200) dos jogadores / times / rivais
void CSpot::DisplayPlayers2D()
{
	if (!m_bShowPlayers || (GOLFDOC()->m_gameMode & 0x200) == 0)
		return;
	unsigned char i;
	WVector sp;
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	case 3:
	case 7:
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (GOLFDOC()->GetPlayer(i)->state == 0 &&
				i != GOLFDOC()->m_currentPlayer &&
				PLAYER(i)->stroke[GOLFDOC()->m_currentHole - 1] != 0)
			{
				sp = g_view->Projection(PLAYER(i)->pos);
				if (0.0f < sp.z && sp.z < 1.0f)
				{
					m_pOvPlayerPos->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
						WRect(sp.x - 4.0f, sp.y - 4.0f, 8.0f, 8.0f), 0,
						PLAYER(i)->color | 0x80000000, 0.0f, 0);
					m_pSimpleUI->SetColor(PLAYER(i)->color | 0x80000000);
					m_pSimpleUI->Render(MakeStr("%d", i + 1), sp.x, sp.y - 12.0f,
						1.0f, 0x22);
				}
			}
		}
		break;
	case 1:
		for (i = 0; i < 2; i++)
		{
			if (TEAM(i)->state == 0 &&
				i != PLAYER(GOLFDOC()->m_currentPlayer)->team &&
				TEAM(i)->stroke[GOLFDOC()->m_currentHole - 1] != 0)
			{
				sp = g_view->Projection(TEAM(i)->pos);
				if (0.0f < sp.z && sp.z < 1.0f)
				{
					m_pOvPlayerPos->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
						WRect(sp.x - 4.0f, sp.y - 4.0f, 8.0f, 8.0f), 0,
						TEAM(i)->color | 0x80000000, 0.0f, 0);
					m_pSimpleUI->SetColor(TEAM(i)->color | 0x80000000);
					m_pSimpleUI->Render(i == 0 ? "r" : "b", sp.x, sp.y - 12.0f,
						1.0f, 0x22);
				}
			}
		}
		break;
	case 4:
	case 5:
	case 9:
	case 10:
	{
		if (CProjectG::Instance()->HidePI())
			break;
		FrWndManager* mgr = g_pFresh->GetManager();
		FrGraphicInterface* gdi = mgr->GetGDI();
		FrEmoticon* emo = mgr->GetEmoticon();
		float oldAlpha = 1.0f;
		if (gdi)
		{
			oldAlpha = gdi->GetAlpha();
			gdi->SetAlpha(1.0f);
			gdi->SetTextStyle(2);
			gdi->SetTextColor(0xffffffff, 0xff000000);
		}
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (rv.state != 0 ||
				!(rv.oid != MyGuid(true) || rv.bGrandPrixAI983 != 0) ||
				m_pRivals[i].hole != GOLFDOC()->m_currentHole ||
				!(0.0f <= m_pRivals[i].t))
				continue;
			float rate = GetMovingRate(m_pRivals[i].t);
			sp = g_view->Projection(GetPVS().GetGroundPoint(
				(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
				NULL, true));
			if (!(0.0f < sp.z && sp.z < 1.0f))
				continue;
			unsigned long color;
			switch (Doc()->m_golfGame.gameType)
			{
			case 5:
				color = (rv.team != 0) ? 0x800060ff : 0x80ff0000;
				break;
			case 10:
				color = (rv.approachTime != -1) ? 0x80ffffff : 0x80505050;
				break;
			case 4:
				color = 0x80ffffff;
				GetRivalColor(rv.shotSpecial983, &color);
				if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <
						2 &&
					rv.bGrandPrixAI983 != 0)
					color = 0x80ffffff;
				break;
			default:
				color = 0x80ffffff;
				break;
			}
			m_pOvPlayerPos->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(sp.x - 4.0f, sp.y - 4.0f, 8.0f, 8.0f), 0, color, 0.0f,
				0);
			if (emo == NULL)
				gdi->PrintText(WPoint(sp.x -
									 gdi->GetTextExtendOld983(rv.nickname) *
										 0.5f,
								   sp.y - 15.0f),
					0, rv.nickname, NULL);
			else
				emo->PrintText(WPoint(sp.x - emo->GetTextWidth(rv.nickname) *
												 0.5f,
								   sp.y - 15.0f),
					0, rv.nickname, 0xffffffff);
		}
		if (gdi)
			gdi->SetAlpha(oldAlpha);
		break;
	}
	case 6:
	{
		if (CProjectG::Instance()->HidePI())
			break;
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (gdi)
		{
			gdi->GetAlpha();
			gdi->SetAlpha(1.0f);
			gdi->SetTextStyle(2);
			gdi->SetTextColor(0xffffffff, 0xff000000);
		}
		for (i = 0; i < Doc()->m_rivalList.size(); i++)
		{
			const sRivalData& rv = Doc()->m_rivalList[i];
			if (!((rv.state == 0 && rv.oid != MyGuid(true) &&
					  m_pRivals[i].hole == GOLFDOC()->m_currentHole &&
					  0.0f <= m_pRivals[i].t &&
					  Doc()->m_myInfo.info.dwGuildId == rv.guildUID) ||
					rv.bGrandPrixAI983 != 0))
				continue;
			float rate = GetMovingRate(m_pRivals[i].t);
			sp = g_view->Projection(GetPVS().GetGroundPoint(
				(1.0f - rate) * m_pRivals[i].pos + rate * rv.ballPos, false,
				NULL, true));
			if (0.0f < sp.z && sp.z < 1.0f)
			{
				m_pOvPlayerPos->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
					WRect(sp.x - 4.0f, sp.y - 4.0f, 8.0f, 8.0f), 0,
					(rv.team != 0) ? 0x800060ff : 0x80ff0000, 0.0f, 0);
				m_pFont->Print(g_view,
					sp.x -
						m_pFont->GetTextWidthOld983(g_view, rv.nickname) * 0.5f,
					sp.y - 15.0f, rv.nickname, 0, 0xffffffff, NULL);
				m_pFont->Flush(g_view);
			}
		}
		break;
	}
	}
}

// 983 0072c430: linha da vista de cima (bola -> alvo), marcador e alcance
void CSpot::DisplayTopView()
{
	if (!m_bActive || GOLFDOC()->m_tutorialMode < 1 ||
		(GOLFDOC()->m_gameMode & 0x200) == 0)
		return;
	UpdateColor();
	WVector ball = GolfBall().m_pos;
	WVector p;
	if (IsMyTurn(false) && !WisEqual(m_rate, 1.0f, g_EPSILON))
	{
		p = m_rate * m_pos + (1.0f - m_rate) * m_oldPos;
		WVector mp = g_view->Projection(m_msgPos);
		CMouseCursor::Instance()->ForceMove(mp.x, mp.y, -1.0f);
	}
	else
		p = m_pos;
	WVector d = p - ball;
	WVector q = m_ratio * d + ball;
	float h = g_view->camera.pivot.y - g_view->GetClipNearValue();
	if (h < ball.y)
		ball.y = h;
	if (h < q.y)
		q.y = h;
	if (h < p.y)
		p.y = h;
	WTVertex vb, vq, vp;
	g_view->Projection2(&vb, ball);
	g_view->Projection2(&vq, q);
	g_view->Projection2(&vp, p);
	float ang = atan2f(vp.sy - vb.sy, vp.sx - vb.sx) + 1.5707964f;
	WOverlay* ov = (m_color == -1) ? m_pOvFall2 : m_pOvFall1;
	float len = sqrtf((vq.sx - vp.sx) * (vq.sx - vp.sx) +
		(vq.sy - vp.sy) * (vq.sy - vp.sy));
	m_pOvLine->RenderWithAxis(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
		WRect(vp.sx - m_markSize * 0.5f, vp.sy, m_markSize, len), vp.sx, vp.sy,
		ang, 0, 0xa0000000, 0);
	len = sqrtf((vb.sx - vq.sx) * (vb.sx - vq.sx) +
		(vb.sy - vq.sy) * (vb.sy - vq.sy));
	m_pOvLine->RenderWithAxis(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
		WRect(vq.sx - m_markSize * 0.5f, vq.sy, m_markSize, len), vq.sx, vq.sy,
		ang, 0, m_color2, 0);
	ov->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
		WRect(vp.sx - m_markSize2 * 0.5f, vp.sy - m_markSize2 * 0.5f,
			m_markSize2, m_markSize2),
		0, m_color2, 0.0f, 0);
	if (m_pRuler) // 983 0072c98c
		m_pRuler->Display(g_view);
	if (m_color != -1)
	{
		DrawHoleIndicator(false);
		DrawRange(false);
	}
	if (WisEqual(m_rate, 1.0f, g_EPSILON))
	{
		m_pRangeInfo->SetColor(m_color2);
		m_pRangeInfo->m_pos = m_pos;
		m_pRangeInfo->Display();
	}
}

bool CSpot::UpdateRivals()
{
	if (m_pRivals)
	{
		delete[] m_pRivals;
		m_pRivals = NULL;
	}
	int n = Doc()->m_rivalList.size();
	if (n > 0)
	{
		m_pRivals = (sRivalPos*)new char[n * sizeof(sRivalPos)];
		for (unsigned char i = 0; i < n; i++)
		{
			m_pRivals[i].rate = 0.0f;
			if (Doc()->m_golfGame.gameType == 10)
				m_pRivals[i].t = 0.0f;
			m_pRivals[i].hole = Doc()->m_rivalList[i].hole;
			m_pRivals[i].pos = Doc()->m_rivalList[i].ballPos;
		}
		return true;
	}
	return false;
}

void CSpot::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0xfa):
		*(float*)msg.param1 = CalcDeltaAngle(m_pos - GolfBall().m_pos,
			m_basePos - GolfBall().m_pos);
		break;
	case M983(0xf4):
		*(WVector*)msg.param1 = m_pos;
		break;
	case M983(0xf5):
		m_bDirty = true;
		break;
	case M983(0xf6):
	{
		float angle = *(float*)msg.param1;
		m_msgPos = *(WVector*)msg.param2;
		m_oldPos = m_pos;
		m_basePos = GolfBall().m_pos +
			(GolfClub().GetRange() * 3.2f) * RotMat(angle, 1).za;
		m_rate = 0.0f;
		m_alpha = 0.4f;
		break;
	}
	case M983(0xf7):
		m_rate = *(float*)msg.param1;
		break;
	case M983(0xf8):
		if (Doc()->m_golfGame.gameType == 10)
			m_pRivals[msg.param1].hole = (unsigned char)msg.param2;
		if (m_pRivals[msg.param1].hole == (unsigned long)msg.param2)
		{
			m_pRivals[msg.param1].rate = 50.0f / (float)msg.param3;
			m_pRivals[msg.param1].t = 0.0f;
			m_pRivals[msg.param1].pos = Doc()->m_rivalList[msg.param1].ballPos;
			if (Doc()->m_golfGame.gameType == 10 &&
				Doc()->m_rivalList[msg.param1].approachTime == -1)
				m_pRivals[msg.param1].t = 1.0f;
		}
		break;
	case M983(0xf9):
		SetRivalHole(msg.param1, (unsigned char)msg.param2);
		break;
	case M983(0xfb):
		UpdateRivals();
		break;
	case 0x126: // 983 0072d7cd: distancia horizontal bola -> spot (jardas), nunca negativa
	{
		WVector d(GolfBall().m_pos.x - m_pos.x, 0.0f, GolfBall().m_pos.z - m_pos.z);
		float dist = d.Magnitude() * 0.3125f;
		*(float*)msg.param1 = dist > 0.0f ? dist : 0.0f;
		break;
	}
	case 0x128: // Hole-in-One: posicao inicial do rival (CHoleInOneGameHandler)
		SetRivalStartPos(msg.param1, (const float*)msg.param2);
		break;
	case 0x129: // Hole-in-One: animacao da tacada do rival (golftask, modo 0xb)
		m_pRivals[msg.param1].rate = 50.0f / (float)msg.param2;
		m_pRivals[msg.param1].t = 0.0f;
		break;
	case M983(0x163):
		m_bActive = true;
		break;
	case M983(0x164):
		m_bActive = false;
		break;
	case M983(0x190):
		m_bActive = msg.param1 == 1;
		break;
	case M983(0x1a1):
		m_bDirty = true;
		m_angle = *(float*)msg.param1 + m_angle;
		break;
	case 0x2718:
		m_angle = *(float*)msg.param1;
		m_bDirty = true;
		break;
	}
}

// 983 chama GolfBall() fora de linha (0046f770); o inline do 645 (static local) nao existe la
CGolfBall& __fastcall GolfBallExt();
#pragma comment(linker, "/alternatename:?GolfBallExt@@YIAAVCGolfBall@@XZ=?GolfBall@@YIAAVCGolfBall@@XZ")
float __fastcall WVectorLenExt(const WVector& v); // 0046e520, fora de linha no 983
#pragma comment(linker, "/alternatename:?WVectorLenExt@@YIMABVWVector@@@Z=?WVectorLen@@YIMABVWVector@@@Z")

// 983 0072cd20: movimento de um rival por quadro (fila de gameType 10 = aproximacao; 4/5/6/9 = rival simples/IA)
void CSpot::ProcessRival(unsigned char r, float delta)
{
	WVector d;
	switch (Doc()->m_golfGame.gameType)
	{
	case 10:
	{
		if (m_pRivals[r].t < 1.0f)
			m_pRivals[r].t = delta * m_pRivals[r].rate + m_pRivals[r].t;
		if (r == MyIndex())
		{
			if (GolfBallExt().m_playFrame >= 0 &&
				GolfBallExt().m_playFrame < GolfBallExt().m_renderNum)
			{
				d = GetHoleCupPos() - GolfBallExt().m_pos;
				d.y = 0.0f;
				Doc()->m_rivalList[r].approachDistance =
					(int)floorf((WVectorLenExt(d) * 10.0f) * 0.3125f);
			}
		}
		else if (m_pRivals[r].t > 1.0f)
		{
			m_pRivals[r].t = 1.0f;
			Doc()->m_rivalList[r].approachDistance =
				Doc()->m_rivalList[r].approachResultDistance;
			SendMsg(this, "Score", M983(0x2a), 0, 0, 0, 0);
		}
		else if (m_pRivals[r].t < 1.0f)
		{
			float rate = GetMovingRate(m_pRivals[r].t);
			d = GetHoleCupPos() -
				((1.0f - rate) * m_pRivals[r].pos +
					rate * Doc()->m_rivalList[r].ballPos);
			d.y = 0.0f;
			Doc()->m_rivalList[r].approachDistance =
				(int)floorf((WVectorLenExt(d) * 10.0f) * 0.3125f);
		}
		break;
	}
	case 0xb:
		break;
	default:
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) > 1)
		{
			ProcessRivalSimple(r, delta);
			return;
		}
		if (m_pRivals[r].hole == GOLFDOC()->m_currentHole)
			ProcessRivalAI(r, delta);
		break;
	}
}

// 983 007297d0: avanca t; ao chegar ao fim, so troca de buraco se a bola do rival ja caiu no copo
void CSpot::ProcessRivalSimple(int r, float delta)
{
	WVector d;
	if (m_pRivals[r].t < 1.0f)
		m_pRivals[r].t = m_pRivals[r].rate * delta + m_pRivals[r].t;
	if (m_pRivals[r].t > 1.0f)
	{
		m_pRivals[r].t = 1.0f;
		d = Doc()->m_rivalList[r].ballPos -
			GOLFDOC()->m_pHoleData[m_pRivals[r].hole - 1].pin;
		d.y = 0.0f;
		CGolfBall& ball = GolfBall();
		if (sqrtf(d.x * d.x + d.z * d.z) < ball.m_radius * 2.0f)
		{
			m_pRivals[r].t = -1.0f;
			m_pRivals[r].hole =
				Doc()->m_holeOrder[GetHoleIndex(m_pRivals[r].hole)];
		}
	}
}

// 983 0072a630: como o Simple, mas avisa o AIPlayer (0x34f/0x348/0x34d) e o Score (0x171)
float __fastcall Scale983(int map, unsigned long channelType, float value); // 0046f540 (jp/aiplayer.cpp)

void CSpot::ProcessRivalAI(int r, float delta)
{
	WVector d;
	bool flag;
	if (m_pRivals[r].t < 1.0f)
	{
		m_pRivals[r].t = m_pRivals[r].rate * delta + m_pRivals[r].t;
		if (m_pRivals[r].t == 1.0f)
			m_pRivals[r].t = m_pRivals[r].t + (float)0.1;
	}
	if (m_pRivals[r].t > 1.0f)
	{
		m_pRivals[r].t = 1.0f;
		d = Doc()->m_rivalList[r].ballPos -
			GOLFDOC()->m_pHoleData[m_pRivals[r].hole - 1].pin;
		d.y = 0.0f;
		// 983 0072a630: limite = 2 * raio do copo (Scale983 0046f540 do raio da bola; x3 fora do mapa 0xb), nao 2 * raio da bola
		float cup = Scale983(Doc()->m_golfGame.gameType,
			Doc()->m_curChannel.Type,
			GolfBall().m_radius);
		if (sqrtf(d.x * d.x + d.z * d.z) >= cup + cup)
		{
			flag = false;
			GetActor("AIPlayer") << MsgObject(NULL, 0x34f, r, (int)&flag, 0, 0, 0);
			if (flag)
				GetActor("AIPlayer") << MsgObject(NULL, 0x348, r, 0, 0, 0, 0);
		}
		else
		{
			m_pRivals[r].t = -1.0f;
			m_pRivals[r].hole =
				Doc()->m_holeOrder[GetHoleIndex(m_pRivals[r].hole)];
			flag = false;
			GetActor("AIPlayer") << MsgObject(NULL, 0x34f, r, (int)&flag, 0, 0, 0);
			if (flag)
			{
				SetRivalHole(r, m_pRivals[r].hole);
				AfxGetTask()->GetActor("AIPlayer") << MsgObject(NULL, 0x34d, r, 0, 0, 0, 0);
				AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x171, r, 0, 0, 0, 0);
			}
		}
	}
}

// 983 00729680: rivais zerados (t = -1 = ainda sem tacada)
void CSpot::ResetRivals()
{
	int n = Doc()->m_rivalList.size();
	for (int i = 0; i < n; i++)
	{
		m_pRivals[i].rate = 0.0f;
		m_pRivals[i].t = -1.0f;
		m_pRivals[i].hole = 0;
		m_pRivals[i].pos.x = 0.0f;
		m_pRivals[i].pos.y = 0.0f;
		m_pRivals[i].pos.z = 0.0f;
	}
}

// 983 0072d030: so na primeira vez (pos ainda zero) grava a posicao xz do rival uid no buraco atual
bool CSpot::SetRivalStartPos(unsigned long uid, const float* xz)
{
	sRivalPos& rival = m_pRivals[Doc()->GetIndex(uid)];
	if (rival.pos.Magnitude() != 0.0f)
		return false;
	rival.t = 0.0f;
	rival.pos.x = xz[0];
	rival.pos.z = xz[1];
	rival.pos.y = 0.0f;
	rival.hole = GOLFDOC()->m_currentHole;
	return true;
}

// 983 007294d0: o rival r passa a jogar o buraco hole (mensagem 0x123); so aceita buraco da ordem da partida
void CSpot::SetRivalHole(int r, unsigned char hole)
{
	unsigned char i = 0;
	if (Doc()->m_golfGame.holes != 0)
	{
		while (Doc()->m_holeOrder[i] != hole)
		{
			i++;
			if (Doc()->m_golfGame.holes <= i)
				return;
		}
		m_pRivals[r].hole = hole;
		m_pRivals[r].t = -1.0f;
		m_pRivals[r].pos = GOLFDOC()->m_pHoleData[m_pRivals[r].hole - 1].tee;
		Doc()->m_rivalList[r].ballPos = m_pRivals[r].pos;
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2)
		{
			bool flag = false;
			GetActor("AIPlayer") << MsgObject(NULL, 0x34f, r, (int)&flag, 0, 0, 0);
			if (flag)
				Doc()->m_rivalList[r].hole = hole;
		}
	}
}

void CSpot::OnProcess(float delta)
{
	if (CCapturedBg::Instance()->IsCaptured())
		return;

	// 983: nao ha mais o "vagar" do gameType 3 (m_offset0/1 ficam sem uso)
	m_lerp = 0.0f;
	if (!GolfClub().IsCalcRandomPower())
	{
		m_pos = m_basePos;
		*(float*)((char*)&GolfClub() + 0x64) = 0.0f;
	}

	if (GOLFDOC()->m_bTutorialPin983 != 0)
	{
		m_rotTimer = delta * g_PI + m_rotTimer;
		if (m_rotTimer > 6.2831855f)
			m_rotTimer = m_rotTimer - 6.2831855f;
	}

	if (GolfClub().Updated())
		m_bDirty = true;

	if (!g_input->Get("LCONTROL", true) && !g_input->Get("RCONTROL", true) &&
		g_input->GetDown("HIDE", true))
		m_bShowPlayers = !m_bShowPlayers;

	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb:
	{
		for (unsigned char r = 0; r < Doc()->m_rivalList.size(); r++)
		{
			if (Doc()->m_rivalList[r].state == 0 &&
				(m_pRivals[r].hole == GOLFDOC()->m_currentHole ||
					(unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2))
				ProcessRival(r, delta);
		}
		break;
	}
	default:
		break;
	}

	if (m_bActive)
	{
		m_rotAngle = delta * g_PI + m_rotAngle;
		if (m_rotAngle > 6.2831855f)
			m_rotAngle = m_rotAngle - 6.2831855f;
		m_alpha = delta * 0.5f + m_alpha;
		if (m_alpha > 1.0f)
			m_alpha = 0.0f;
		if (m_bDirty)
		{
			if (Wabs(m_rate - 1.0f) < g_EPSILON)
			{
				if (GolfClub().Updated())
					m_alpha = 0.0f;
				m_bDirty = false;
				Rotate();
			}
		}
		if (GolfClub().GetType() != 3)
		{
			if (Wabs(m_rate - 1.0f) < g_EPSILON)
			{
				m_anim = delta * m_animDir + delta * m_animDir + m_anim;
				if (m_anim > 1.0f)
				{
					m_anim = 1.0f;
					m_animDir = -1.0f;
				}
				else if (m_anim < 0.0f)
				{
					m_anim = 0.0f;
					m_animDir = 1.0f;
				}
				// HACK: the original stores m_anim twice into the slot of delta and reads it back
				// around the calls: the address of delta escapes and the first store is volatile
				if (0)
					g_view->Projection2(NULL, *(WVector*)&delta);
				*(volatile float*)&delta = m_anim;
				delta = m_anim;
				float lo =
					GetPVS().GetMinPowerOnGround() * (1.0f - delta * delta);
				m_ratio = GetPVS().GetMaxPowerOnGround() * (delta * delta) + lo;
				return;
			}
		}
		m_ratio = 1.0f;
	}
}
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // A748: ex-cru Doc()+0x10a99
