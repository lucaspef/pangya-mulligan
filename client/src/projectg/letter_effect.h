#pragma once

#include "actor.h"
#include "wlist.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "wfont.h"
#include "fx.h"

class WTitleFont;

// Full-screen text overlays ("letters") shown during a round: hole result
// banners, special shot titles, replay/fast-forward icons and the like.
class CLetter : public IActor
{
public:
	virtual ~CLetter() { }

	DECLARE_OBJECT(CLetter)

public:
	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CLetter();

	struct sNumber
	{
		bool bShow;
		int value;
		float scale;
		unsigned long color;
	};

	struct sText
	{
		WOverlay* pOverlay;
		WRect dst;
		WRect src;
		float life;
		char name[16];
		bool bWaitInput;
		sNumber number;

		sText() { pOverlay = NULL; }

		~sText()
		{
			if (pOverlay)
			{
				g_resrcmng->Release(pOverlay);
				pOverlay = NULL;
			}
		}
	};

	void AddOverlay(const char* name, const char* file, WRect* rect,
		bool bWaitInput, WList<sText*>* pList = NULL);
	void AddPangyaOverlay983(unsigned long clubTid);
	static float GetPangyaScale983();
	void ScalePangya983(sText* pText);
	bool StartSrank983();
	void SetPos(const char* name, float x, float y);
	void Print(float x, float y, const char* text, float space, int type,
		float scale, unsigned long color);
	void DisplayReplay();

	sText* m_pCurrent;
	WList<sText*> m_texts;
	WList<sText*> m_pangyaTexts983; // 983 +0x3c: letreiros PangYa! (0075ff40), com escala 0075f9c0
	float m_alpha;
	int m_state;
	float m_fadeTime;
	bool m_bShow;
	int m_align;
	int m_unused50;
	WTITLEFONT m_fontInfo;
	WTitleFont* m_pFont;
	sText* m_pFastForward;
	sText* m_pReplay;
	float m_replayTime;
	// 983: logo S-rank do PangYa! (0x1fe com param3 == 3; construtor 00760f10, OnProcess 0075f720, OnDisplay 0075fab0)
	WOverlay* m_pSrankDeco983;  // +0xa4 [text_srankdeco1.jpg
	WOverlay* m_pSrankDeco3983; // +0xa8 [S_Rank_deco_3.jpg
	CPvsFxBox m_fxBox983;       // +0xac pangya_logo.seq
	WRect m_srankRect983;       // +0x4ec
	WMatrix m_srankCamera983;   // +0x4fc
	int m_srankFade983;         // +0x52c 1 entra, 3 sai
	int m_srank983;             // +0x530
	bool m_bSrankFaded983;      // +0x534
	float m_srankAlpha983;      // +0x538
};
