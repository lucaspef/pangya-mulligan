#pragma once

#include "actor.h"
#include "scenemanager.h"

class C3dFont;
class CSimpleUI;
class WOverlay;
class WFont;

class CClubRangeInfo;
class CHoleRuler983;
struct sRivalData;

class CSpot : public IActor, public CRenderFuncPtr
{
public:
	virtual ~CSpot() { }

	DECLARE_OBJECT(CSpot)

public:
	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void Display();
	virtual void OnDisplay();

protected:
	CSpot();

	virtual void HandleMsg(const MsgObject& msg);

	void ClipIndicator(int index);
	void DrawRange(bool bClose);
	void Rotate();
	void DrawHoleIndicator(bool bClose);
	void DrawIndicator(const WVector& pos, int color);

private:
	struct sRivalPos
	{
		sRivalPos();

		float rate;
		float t;
		unsigned char hole;
		WVector pos;
	};

	static float GetMovingRate(float t);
	static const unsigned long m_adwColors[5];
	bool _ValidTopViewLineDraw();
	bool UpdateRivals();
	void ProcessRival(unsigned char r, float delta); // 0072cd20
	void ProcessRivalSimple(int r, float delta); // 007297d0
	void ProcessRivalAI(int r, float delta); // 0072a630
	void SetRivalHole(int r, unsigned char hole); // 007294d0
	void ResetRivals(); // 00729680
	bool SetRivalStartPos(unsigned long uid, const float* xz); // 0072d030 (mensagem 0x128, Hole-in-One)

	void DrawQuad(const WVector& pos, float size, unsigned long color); // 00728a80
	void DrawLabel(const WVector& pos, unsigned long color, const char* text,
		int flag); // 00728c10
	bool IsAtTee(const sRivalData& rv); // 00729620
	static void __stdcall GetRivalColor(unsigned char flags, unsigned long* color); // 007282a0
	void UpdateColor(); // 00729970
	void DisplayTutorial(); // 00729af0
	void DisplayPlayers3D(); // 0072a9c0
	void DisplayDistance(); // 00729e90
	void DisplayRivalNames(); // 0072b210
	void DisplayPlayers2D(); // 0072b8c0
	void DisplayTopView(); // 0072c430

	C3dFont* m_pFontWind;
	C3dFont* m_pFontPlayer;
	WVector m_pos;
	WVector m_oldPos;
	WVector m_msgPos;
	float m_angle;
	float m_rate;
	WVector m_offset0;
	WVector m_offset1;
	WVector m_basePos;
	float m_lerp;
	unsigned long m_color;
	float m_rotAngle;
	bool m_bActive;
	float m_markSize;
	float m_markSize2;
	CClubRangeInfo* m_pRangeInfo;
	unsigned long m_color2;
	float m_w;
	float m_h;
	int m_texIndicator;
	int m_texFalling;
	int m_texSlope;
	int m_texTarget;
	float m_alpha;
	float m_ratio;
	float m_anim;
	float m_animDir;
	WOverlay* m_pOvFall1;
	WOverlay* m_pOvFall2;
	WOverlay* m_pOvLine;
	WOverlay* m_pOvPlayerPos;
	CSimpleUI* m_pSimpleUI;
	float m_rotTimer;
	int m_texScore;
	WTVertex* m_pVtx[4];
	int m_unusedE8;
	WTVertex m_vtx[4];
	bool m_bDirty;
	int m_unused190;
	sRivalPos* m_pRivals;
	bool m_bShowPlayers;
	WFont* m_pFont;
	CHoleRuler983* m_pRuler; // 983 +0x260: so no Hole-in-One com realGameType 0xe (OnLoad 00729f80)
	std::vector<int> m_vec983_264; // 983 +0x264: vector zerado no ctor 0072ca70 e liberado no dtor 0072cba0; sem outro acesso no 983 (tipo do elemento desconhecido)
};
