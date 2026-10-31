#pragma once
// JP 983: pos-efeito da cena (HDR/estrela, DOF, rim light, FXAA) — objeto em CTask+0x68, criado no 1o CTask::Display
// (009bb490: new 0x32254 + ctor 005afb00 + Setup 005b01f0). Nao existe no 645. Nomes meus.
// Porte em andamento: Setup/IsActive/BeginScene/EndScene/LoadShaders/DOF/005adde0/HDR 005b00a0+005ade80+005ae0b0+005ae690+005af370
// feitos; em units.txt; CTask::Display/~CTask (A256) ligam com JP983_POSTEFFECT (+ 009b11b0 apos a cena, A257).
#include "wtextureview.h"

class WView;

class CPostEffect983
{
public:
	enum
	{
		VIEW_SCENE = 0,     // +0x1a8: cena (2 RTs + profundidade)
		VIEW_DOF = 4,       // +0x7018: saida do DOF (005adaf0)
		VIEW_EXTRA_PASS = 5,      // +0x8bb4: saida do estagio do flag 2 (005adde0)
		VIEW_HDR_OUT = 28,  // +0x306b8: saida do HDR (005b00a0)
		VIEW_COUNT = 29
	};

	CPostEffect983(WView* view);      // 005afb00 (g_view)
	~CPostEffect983();                // 009b74d0
	bool Setup(WView* view);         // 005b01f0 (g_view)
	bool IsActive() const;            // 005b0030 (no mapa: CHoleWnd983::IsReady983, nome errado)
	void BeginScene();                // 005b0fe0
	void EndScene();                  // 005b10d0

	bool m_bEnable;      // +0x00: IsSupportMrt && IsSupportVs && IsSupportPs
	bool m_bDof;         // +0x01: viDof
	bool m_bExtraPass;        // +0x02: nunca ligado pelo Setup
	bool m_bHdr;         // +0x03: viHdr
	bool m_bHdrStar;     // +0x04: viHdrStarEffect
	bool m_bRimLight;    // +0x05: viRimLight
	bool m_bFxaa;        // +0x06: viFXAA
	int m_bright;        // +0x08: (int)vfBright
	int m_dofNear;       // +0x0c: fx param 0x19 (shader.txt x10)
	int m_dofRange;      // +0x10: fx param 0x1a = near + range (shader.txt x10)
	int m_shaderParam[6]; // +0x14: 1a linha do <campo>_shader.txt (padrao 50 50 60 50 20 50)
	int m_rimParam;      // +0x2c: fx param 0x21 (padrao 0x32)
	int m_rimColor;      // +0x30: fx param 0x22 (padrao -1)
	WTVertex m_vtx[4];   // +0x34: quad da tela (0x58 cada)
	WTVertex* m_vl[4];   // +0x194: v0, v1, v3, v2
	int m_pad983_1a4;    // +0x1a4: 983 so zera (005b0ed5 Setup, 005b1127 EndScene, antes do quad); sem leitor
	WTextureView m_view[VIEW_COUNT]; // +0x1a8 .. +0x32254

	void LoadShaders();                 // 005afce0 (no mapa: CHoleWnd983::Refresh983, nome errado); tambem CGolfBg::DoReLoad
private:
	void InitQuad();
	void DofStage(unsigned tex);        // 005adaf0
	void ExtraPassStage(unsigned tex);       // 005adde0
	void HdrStage(unsigned tex);        // 005b00a0
	void HdrDownScale(unsigned tex);    // 005ade80: cena -> view16 (4x4)
	void HdrStage2();                   // 005ae0b0: luminancia view20..17
	void HdrStage3();                   // 005ae690: luminancia adaptada view21
	void HdrStage4(unsigned tex);       // 005af370: brilho/estrela view8, 6/7, 9..14 -> view15
};
