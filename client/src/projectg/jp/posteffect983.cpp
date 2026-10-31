// JP 983: pos-efeito da cena (CTask+0x68). Ver posteffect983.h.
#include "posteffect983.h"
#include "vc10_layout.h"
#include "wview.h"
#include "wvideo.h"
#include "wresrcmng.h"
#include "clientsetting.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "taskmanager.h"
#include "lobbytask.h"
#include "cfile.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

extern WView* g_view;

C_ASSERT_VC10(sizeof(CPostEffect983) == 0x32254); // 983: operator new(0x32254) em 009bb4e9
C_ASSERT(offsetof(CPostEffect983, m_bright) == 0x08);
C_ASSERT(offsetof(CPostEffect983, m_rimParam) == 0x2c);
C_ASSERT(offsetof(CPostEffect983, m_vtx) == 0x34);
C_ASSERT(offsetof(CPostEffect983, m_vl) == 0x194);
C_ASSERT(offsetof(CPostEffect983, m_view) == 0x1a8);
C_ASSERT(offsetof(CPostEffect983, m_shaderParam) == 0x14);
C_ASSERT_VC10(offsetof(WView, SCREEN_XS) == 0x19c); // 005ade80 le view16+0x19c/+0x1a0
C_ASSERT(sizeof(WRenderToTextureParam::RtTexInfo) == 0xc); // GetTexture(1) = view+0x1b7c (005adaf0/005adde0)
C_ASSERT(offsetof(sOption, viAmbientControl) == 0x6c); // 005aff1e (o Setup le viRimLight em +0x70)
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.pCourse) == 0x10590);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594); // 005b0030/005afcf8: byte [doc+0x10594] == 8
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024);          // 005b0030: [[doc+0x11024]+0x158]
C_ASSERT_VC10(offsetof(CGolfDoc, m_gameMode) == 0x158);

static WVideoDev* PeDevice()
{
	return g_view->GetVideoDevice(); // 983: **(this+0x1d0) = *(view0.resrcMng)
}

CPostEffect983::CPostEffect983(WView* view)
{
	// 005afb00: 29 WTextureView (ctor 00bb1670) + caps do device
	WVideoDev* dev = view->GetVideoDevice();
	m_bEnable = dev->IsSupportMrt() && dev->IsSupportVs() && dev->IsSupportPs();
	m_bDof = false;
	m_bExtraPass = false;
	m_bHdr = false;
	m_bHdrStar = false;
	m_bRimLight = false;
	m_bFxaa = false;
	m_bright = 3;
}

CPostEffect983::~CPostEffect983()
{
	// 009b74d0: so os destrutores dos 29 WTextureView (automatico)
}

// quad da tela inteira w x h deslocado meio pixel (v0, v1, v3, v2 = leque); 983: 00d66ca0 = double 0.5
static void PeQuad(WTVertex* v, WTVertex** vl, float w, float h)
{
	const float off = 0.5f;
	vl[0] = &v[0];
	vl[1] = &v[1];
	vl[2] = &v[3];
	vl[3] = &v[2];
	for (int i = 0; i < 4; i++)
	{
		v[i].x = (i & 1) ? w - off : -off;
		v[i].y = (i & 2) ? h - off : -off;
		v[i].z = 0.001f; // 00d08dcc
		v[i].rhw = 1.0f;
		v[i].tu = (i & 1) ? 1.0f : 0.0f;
		v[i].tv = (i & 2) ? 1.0f : 0.0f;
	}
}

void CPostEffect983::InitQuad()
{
	// fim do 005b01f0 e comeco do 005b10d0: deslocado meio pixel
	m_pad983_1a4 = 0;
	PeQuad(m_vtx, m_vl, m_view[VIEW_SCENE].SCREEN_XS, m_view[VIEW_SCENE].SCREEN_YS);
}

// 005ad0d0 (n = 4): 4x4 amostras centradas, ((col, lin) - 1.5) / (w, h)
static void PeOffsets(float (*ofs)[2], float w, float h)
{
	for (int i = 0; i < 16; i++)
		ofs[i][0] = ((i & 3) - 1.5f) / w, ofs[i][1] = ((i >> 2) - 1.5f) / h;
}

static void PeSize(WRenderToTextureSizeInfo& s, bool abs, float w, float h)
{
	s.m_isAbsolute = abs;
	s.m_width = w;
	s.m_height = h;
}

bool CPostEffect983::Setup(WView* view)
{
	// 005b01f0
	if (!m_bEnable)
		return false;
	const sOption* opt = COption::Instance()->GetOption(); // 004927b0 = &COption::m_opt
	m_bHdr = opt->viHdr == 1 && m_bEnable;
	m_bHdrStar = opt->viHdrStarEffect == 1 && m_bHdr && m_bEnable;
	m_bDof = opt->viDof == 1 && m_bEnable;
	m_bRimLight = opt->viRimLight == 1 && m_bEnable;
	m_bFxaa = opt->viFXAA == 1 && m_bEnable;
	m_bright = (int)opt->vfBright;
	LoadShaders();

	const float sw = view->SCREEN_XS, sh = view->SCREEN_YS;
	WRenderToTextureSizeInfo full, quarter, eighth, abs0;
	PeSize(full, false, 1.0f, 1.0f);       // 00e42db0
	PeSize(quarter, false, 0.25f, 0.25f);  // 00e42dc8
	PeSize(eighth, false, 0.125f, 0.125f); // 00e42dd4
	PeSize(abs0, true, 0.0f, 0.0f);        // 00e42de0
	for (int i = 0; i < VIEW_COUNT; i++)
	{
		// tabela do 005b01f0: tamanho do RT e createParam983 (1 = fp16, D3DFMT 113) por view
		const WRenderToTextureSizeInfo* size = &full;
		int fp16 = 0;
		float vw = sw, vh = sh;
		if (i >= 8 && i <= 15)
			fp16 = 1;                            // +0xde88, o vetor de 6 em +0xfa24 e +0x19fcc
		else if (i == 16)
			size = &quarter, fp16 = 1;           // +0x1bb68 (400x225)
		else if (i >= 17 && i <= 20)
		{
			size = &abs0, fp16 = 1;              // +0x1d704[4]: 1, 4, 16, 64
			vw = vh = (float)(1 << ((i - 17) * 2));
		}
		else if (i == 21)
			size = &abs0, fp16 = 1, vw = vh = 1.0f; // +0x24574
		else if (i == 22 || i == 23)
			size = &quarter;                     // +0x26110, +0x27cac
		else if (i >= 24 && i <= 27)
			size = &eighth;                      // +0x29848 e o vetor de 3 em +0x2b3e4
		else if (i == 28)
			fp16 = 1;                            // +0x306b8

		WTextureView& tv = m_view[i];
		tv.SetResourceManager(g_resrcmng);
		tv.SetViewport(vw, vh);
		WTextureView::TextureParam param; // 005afa70
		param.m_numRts = 1;
		param.m_rtTexInfo[0].Set(0x4000, *size, false, 0);
		param.m_rtTexInfo[0].m_createParam983 = fp16;
		if (i == VIEW_SCENE)
		{
			param.m_numRts = 2;
			param.m_rtTexInfo[1].Set(0x4000, full, true, 0xffffffff);
			param.m_depthSurfInfo.Set(
				WRenderToTextureParam::DepthSurfInfo::SurfaceUsage(1), full, true, 1.0f);
		}
		tv.SetTextureParam(param);
	}
	InitQuad();
	return true;
}

bool CPostEffect983::IsActive() const
{
	// 005b0030
	const sOption* opt = COption::Instance()->GetOption();
	if (!(m_bDof || m_bExtraPass || m_bHdr || m_bRimLight || opt->viAmbientControl == 1 || m_bFxaa))
		return false;
	CSharedDoc* doc = CSharedDoc::Instance(); // 983: DAT_00e47e2c
	return doc->m_golfGame.gameType != 8 && doc->m_pGolfDoc->m_gameMode != 0x200;
}

void CPostEffect983::BeginScene()
{
	// 005b0fe0
	if (!m_bEnable)
		return;
	WVideoDev* dev = PeDevice();
	if (IsActive())
	{
		m_view[VIEW_SCENE].Begin();
		dev->BeginUsingCustomRenderState();
		unsigned macro = 0;
		if (m_bDof)
		{
			macro = 8;
			dev->SetCustomFxParamInt(WFxParameterType(0x19), m_dofNear);
			dev->SetCustomFxParamInt(WFxParameterType(0x1a), m_dofRange + m_dofNear);
		}
		if (m_bExtraPass)
			macro |= 0x10;
		if (!m_bRimLight)
		{
			dev->SetCustomFxParamInt(WFxParameterType(0x21), 0x32);
			dev->SetCustomFxParamInt(WFxParameterType(0x22), -1);
		}
		else
		{
			dev->SetCustomFxParamInt(WFxParameterType(0x21), m_rimParam);
			dev->SetCustomFxParamInt(WFxParameterType(0x22), m_rimColor);
		}
		dev->SetGlobalRenderState(0, 0, macro);
		return;
	}
	dev->SetCustomFxParamInt(WFxParameterType(0x21), 0x32);
	dev->SetCustomFxParamInt(WFxParameterType(0x22), -1);
}

void CPostEffect983::EndScene()
{
	// 005b10d0
	if (!m_bEnable || !IsActive())
		return;
	WVideoDev* dev = PeDevice();
	m_view[VIEW_SCENE].End();
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
	InitQuad();
	unsigned tex = m_view[VIEW_SCENE].GetTexture(0); // 983: view0+0x1b70
	float w = m_view[VIEW_SCENE].SCREEN_XS, h = m_view[VIEW_SCENE].SCREEN_YS;
	if (m_bDof)
	{
		DofStage(tex);
		w = m_view[VIEW_DOF].SCREEN_XS, h = m_view[VIEW_DOF].SCREEN_YS;
		tex = m_view[VIEW_DOF].GetTexture(0);
	}
	if (m_bExtraPass)
	{
		ExtraPassStage(tex);
		w = m_view[VIEW_EXTRA_PASS].SCREEN_XS, h = m_view[VIEW_EXTRA_PASS].SCREEN_YS;
		tex = m_view[VIEW_EXTRA_PASS].GetTexture(0);
	}
	if (m_bHdr)
	{
		HdrStage(tex);
		w = m_view[VIEW_HDR_OUT].SCREEN_XS, h = m_view[VIEW_HDR_OUT].SCREEN_YS;
		tex = m_view[VIEW_HDR_OUT].GetTexture(0);
	}
	dev->BeginUsingCustomRenderState();
	if (m_bFxaa)
	{
		float inv[2] = { 1.0f / w, 1.0f / h };
		dev->SetCustomFxMacro(0x4000);
		dev->SetCustomFxParamVector2(WFxParameterType(0x13), *(const WVector2D*)inv);
	}
	g_view->DrawPolygonFan(m_vl, (tex & 0x7ff) | 0x60300000, 0, true);
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
}

void CPostEffect983::DofStage(unsigned tex)
{
	// 005adaf0: blur separavel (view0 -> view1 em x, view1 -> view2 em y; macro 0x8000000) e depois view4 = cena (tex)
	// misturada com a borrada (view2) pela profundidade (RT1 da view0).
	WVideoDev* dev = PeDevice();
	WTVertex vtx[4];
	WTVertex* vl[5] = {}; // 983: lista terminada em NULL (005adaf0 local_194 = 0); WView::DrawPolygonFan conta ate o NULL
	const float w = m_view[1].SCREEN_XS, h = m_view[1].SCREEN_YS;
	PeQuad(vtx, vl, w, h);
	const float step[2][2] = { { 1.0f / w, 0.0f }, { 0.0f, 1.0f / h } };
	for (int i = 0; i < 2; i++)
	{
		WTextureView& dst = m_view[1 + i];
		dev->BeginUsingCustomRenderState();
		dev->SetCustomFxMacro(0x8000000);
		dev->SetCustomFxParamVector2(WFxParameterType(0x13), *(const WVector2D*)step[i]);
		if (dst.Begin())
		{
			dst.DrawPolygonFan(vl, (m_view[i].GetTexture(0) & 0x7ff) | 0x66300000, 0, true);
			dst.End();
		}
		dev->EndUsingCustomRenderState();
	}
	WTextureView& out = m_view[VIEW_DOF];
	out.Begin();
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxParamTexture(WFxParameterType(0xc), m_view[VIEW_SCENE].GetTexture(1)); // view0+0x1b7c
	dev->SetCustomFxParamTexture(WFxParameterType(0xd), m_view[2].GetTexture(0));
	dev->SetGlobalRenderState(0, 0, 1);
	out.DrawPolygonFan(m_vl, (tex & 0x7ff) | 0x60300000, 0, true);
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
	out.End();
}

void CPostEffect983::ExtraPassStage(unsigned tex)
{
	// 005adde0
	WVideoDev* dev = PeDevice();
	WTextureView& out = m_view[VIEW_EXTRA_PASS];
	out.Begin();
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxParamTexture(WFxParameterType(0xc), m_view[VIEW_SCENE].GetTexture(1));
	dev->SetGlobalRenderState(0, 0, 2);
	out.DrawPolygonFan(m_vl, (tex & 0x7ff) | 0x60300000, 0, true);
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
	out.End();
}

void CPostEffect983::HdrStage(unsigned tex)
{
	// 005b00a0: estagios do HDR e depois view28 = cena (tex) + view15 (0xc) + view21 (0xd), macro 0x2000000
	WVideoDev* dev = PeDevice();
	HdrDownScale(tex);
	HdrStage2();
	HdrStage3();
	HdrStage4(tex);
	const float k = (float)(1.0 - (4.0 - m_bright) * 0.1); // 00d00ec8 = 4.0, 00cff040 = 0.1
	WTextureView& out = m_view[VIEW_HDR_OUT];
	out.Begin();
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxParamTexture(WFxParameterType(0xc), m_view[15].GetTexture(0));
	dev->SetCustomFxMacro(0x2000000);
	dev->SetCustomFxParamTexture(WFxParameterType(0xd), m_view[21].GetTexture(0));
	dev->SetCustomFxParamInt(WFxParameterType(0x1b), m_shaderParam[2]);
	dev->SetCustomFxParamInt(WFxParameterType(0x1c), (int)(m_shaderParam[3] * k));
	dev->SetCustomFxParamInt(WFxParameterType(0x1d), (int)(m_shaderParam[4] * k));
	dev->SetCustomFxParamInt(WFxParameterType(0x1e), m_shaderParam[5]);
	out.DrawPolygonFan(m_vl, (tex & 0x7ff) | 0x60300000, 0, true);
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
	out.End();
}

void CPostEffect983::HdrDownScale(unsigned tex)
{
	// 005ade80: view16 = cena reduzida, 16 amostras 4x4 (005ad0d0 com o tamanho do device)
	WVideoDev* dev = PeDevice();
	WTextureView& out = m_view[16];
	WTVertex vtx[4];
	WTVertex* vl[5] = {};
	PeQuad(vtx, vl, out.SCREEN_XS, out.SCREEN_YS);
	float ofs[16][2]; // 983: buffer de 0x80 (x, y)
	PeOffsets(ofs, (float)dev->GetWidth(), (float)dev->GetHeight());
	dev->SetFxSampleOffsets(ofs);
	out.Begin();
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxParamInt(WFxParameterType(0x17), 0x10);
	dev->SetCustomFxMacro(0x400000);
	out.DrawPolygonFan(vl, (tex & 0x7ff) | 0x66380000, 0, true);
	dev->ClearFxSampleOffsets();
	dev->EndUsingCustomRenderState();
	out.End();
}

void CPostEffect983::HdrStage2()
{
	// 005ae0b0: luminancia media: view20 (64x64) = view16 em 3x3 (macro 0x800000); depois view19, 18, 17 (16, 4, 1) = 4x4
	// da anterior (0x1000000; a ultima, 1x1, com 0x1800000)
	WVideoDev* dev = PeDevice();
	WTVertex vtx[4];
	WTVertex* vl[5] = {};
	float ofs[16][2] = {}; // 983: memset 0x80 antes do 3x3
	for (int i = 20; i >= 17; i--)
	{
		WTextureView& dst = m_view[i];
		WTextureView& src = m_view[i == 20 ? 16 : i + 1];
		PeQuad(vtx, vl, dst.SCREEN_XS, dst.SCREEN_YS);
		if (i == 20)
		{
			const float sx = 1.0f / (dst.SCREEN_XS * 3.0f), sy = 1.0f / (dst.SCREEN_YS * 3.0f); // 00d66cf8 = 3.0
			for (int j = 0; j < 9; j++)
				ofs[j][0] = (j / 3 - 1) * sx, ofs[j][1] = (j % 3 - 1) * sy;
		}
		else
			PeOffsets(ofs, src.SCREEN_XS, src.SCREEN_YS);
		dev->SetFxSampleOffsets(ofs);
		dst.Begin();
		dev->BeginUsingCustomRenderState();
		if (i != 20)
			dev->SetCustomFxParamInt(WFxParameterType(0x17), 0x10);
		dev->SetCustomFxMacro(i == 20 ? 0x800000 : i == 17 ? 0x1800000 : 0x1000000);
		dst.DrawPolygonFan(vl, (src.GetTexture(0) & 0x7ff) | (i == 20 ? 0x60300000 : 0x60380000), 0, true);
		dev->ClearFxSampleOffsets();
		dev->EndUsingCustomRenderState();
		dst.End();
	}
}

void CPostEffect983::HdrStage3()
{
	// 005ae690: view21 (1x1) = luminancia adaptada; 1o quadro (textura 0) so copia a view17, depois mistura a view17 (0xc)
	// com a propria view21 (macro 0x40000000)
	WVideoDev* dev = PeDevice();
	WTextureView& out = m_view[21];
	WTVertex vtx[4];
	WTVertex* vl[5] = {};
	PeQuad(vtx, vl, out.SCREEN_XS, out.SCREEN_YS);
	const unsigned lum = m_view[17].GetTexture(0);
	for (int pass = out.GetTexture(0) ? 1 : 0; pass < 2; pass++)
	{
		out.Begin();
		dev->BeginUsingCustomRenderState();
		if (pass)
		{
			dev->SetCustomFxMacro(0x40000000);
			dev->SetCustomFxParamTexture(WFxParameterType(0xc), lum);
		}
		out.DrawPolygonFan(vl, ((pass ? out.GetTexture(0) : lum) & 0x7ff) | 0x60380000, 0, true);
		dev->EndUsingCustomRenderState();
		out.End();
	}
}

void CPostEffect983::HdrStage4(unsigned tex)
{
	// 005af370: brilho (view15). view8 = cena (macro 0x4000000, param 0x23 = shaderParam[0]); 3 x blur separavel
	// view8 -> 6 <-> 7 (macro 0x8000000, quad estatico 00e11860 do tamanho da view6). Sem estrela a ultima passada
	// (y) vai direto para a view15; com estrela, view9..14 = 6 raios da view7 (macro 0x80000000, param 0x24 =
	// shaderParam[1], 0x15 = passo (cos, sin)(2 pi i / 6) x 8 / largura do raio; raio 3 com y = 0) e view15 = soma
	// (macro 0x84000000, raios 10..14 em 0xc..0x10, 9 na fonte).
	WVideoDev* dev = PeDevice();
	WTVertex vtx[4], bvtx[4];
	WTVertex *vl[5] = {}, *bvl[5] = {};
	PeQuad(vtx, vl, m_view[VIEW_SCENE].SCREEN_XS, m_view[VIEW_SCENE].SCREEN_YS);
	PeQuad(bvtx, bvl, m_view[6].SCREEN_XS, m_view[6].SCREEN_YS);
	const float step[2][2] = { { 1.0f / m_view[6].SCREEN_XS, 0.0f }, { 0.0f, 1.0f / m_view[6].SCREEN_YS } };
	WTextureView& bright = m_view[8];
	bright.Begin();
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxMacro(0x4000000);
	dev->SetCustomFxParamInt(WFxParameterType(0x23), m_shaderParam[0]);
	bright.DrawPolygonFan(vl, (tex & 0x7ff) | 0x66300000, 0, true);
	dev->SetGlobalRenderState(0, 0, 0);
	dev->EndUsingCustomRenderState();
	bright.End();
	WTextureView* const src[6] = { &m_view[8], &m_view[6], &m_view[7], &m_view[6], &m_view[7], &m_view[6] };
	WTextureView& out = m_view[15];
	for (int n = 0; n < 6; n++)
	{
		const int i = n & 1;
		WTextureView& dst = (n == 5 && !m_bHdrStar) ? out : m_view[6 + i];
		dev->BeginUsingCustomRenderState();
		dev->SetCustomFxMacro(0x8000000);
		dev->SetCustomFxParamVector2(WFxParameterType(0x13), *(const WVector2D*)step[i]);
		if (dst.Begin())
		{
			dst.DrawPolygonFan(bvl, (src[n]->GetTexture(0) & 0x7ff) | 0x66300000, 0, true);
			dst.End();
		}
		dev->EndUsingCustomRenderState();
	}
	if (!m_bHdrStar)
		return;
	for (int i = 0; i < 6; i++)
	{
		WTextureView& ray = m_view[9 + i];
		const float a = (float)(i * 2.0 * 3.1415927410125732 / 6.0); // 00d04d40 = pi, 00d05158 = 6.0
		float d[2];
		d[0] = (float)(cos(a) * 8.0) / ray.SCREEN_XS; // 00d05118 = 8.0
		d[1] = (float)(sin(a) * 8.0) / ray.SCREEN_XS; // 983: os dois / SCREEN_XS
		if (i == 3)
			d[1] = 0.0f;
		ray.Begin();
		dev->BeginUsingCustomRenderState();
		dev->SetCustomFxMacro(0x80000000);
		dev->SetCustomFxParamInt(WFxParameterType(0x24), m_shaderParam[1]);
		dev->SetCustomFxParamVector2(WFxParameterType(0x15), *(const WVector2D*)d);
		ray.DrawPolygonFan(vl, (m_view[7].GetTexture(0) & 0x7ff) | 0x66300000, 0, true);
		dev->EndUsingCustomRenderState();
		ray.End();
	}
	dev->BeginUsingCustomRenderState();
	dev->SetCustomFxMacro(0x84000000);
	for (int i = 0; i < 5; i++)
		dev->SetCustomFxParamTexture(WFxParameterType(0xc + i), m_view[10 + i].GetTexture(0));
	if (out.Begin())
	{
		out.DrawPolygonFan(vl, (m_view[9].GetTexture(0) & 0x7ff) | 0x66300000, 0, true);
		out.End();
	}
	dev->EndUsingCustomRenderState();
}

// ---- A256: ligado no CTask::Display/~CTask (taskmanager.cpp) com JP983_POSTEFFECT; exe de teste a256-1.

void CPostEffect983::LoadShaders()
{
	// 005afce0: <campo>_shader.txt (no lobby "blue"); fora do lobby sem campo nao muda nada (gameType 8)
	CSharedDoc* doc = CSharedDoc::Instance();
	if (doc->m_golfGame.gameType == 8)
		return;
	char name[0x80] = "blue";
	CTask* task = AfxGetTask();
	if (!task || !IS_KINDOF(CLobbyTask, task)) // RTTI 00f030b0
		strcpy(name, doc->m_golfGame.pCourse ? (const char*)doc->m_golfGame.pCourse + 0xc0 : "");
	char path[0x100];
	sprintf(path, "%s_shader.txt", name);
	WVideoDev* dev = PeDevice();
	int* sp = m_shaderParam;
	cFile* file = g_resrcmng->GetCFile(path, -1);
	if (!file)
	{
		const int def[6] = { 50, 50, 60, 50, 20, 50 };
		memcpy(sp, def, sizeof(def));
		m_dofNear = 100;
		m_dofRange = 500;
		dev->SetRimLightParam0(255.0f); // 00cfec5c
		dev->SetRimLightParam1(255.0f);
		dev->SetRimLightParam2(0.0f);
		dev->SetRimLightWidth(10);
		m_rimParam = m_rimColor = 50;
		return;
	}
	int dofNear, dofRange, width;
	float p0, p1, p2;
	file->Scan("%d %d %d %d %d %d\n", &sp[0], &sp[1], &sp[2], &sp[3], &sp[4], &sp[5]);
	file->Scan("%d %d\n", &dofNear, &dofRange);
	m_dofNear = dofNear * 10;
	m_dofRange = dofRange * 10;
	file->Scan("%f %f %f %d\n", &p0, &p1, &p2, &width);
	dev->SetRimLightParam0(p0);
	dev->SetRimLightParam1(p1);
	dev->SetRimLightParam2(p2);
	dev->SetRimLightWidth(width);
	dev->SetRimLight(COption::Instance()->GetOption()->viAmbientControl == 1); // 983 le +0x6c, nao o viRimLight
	file->Scan("%d %d\n", &m_rimParam, &m_rimColor);
	CloseCFile(file);
}
