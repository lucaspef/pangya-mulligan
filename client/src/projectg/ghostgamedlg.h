#pragma once

#include "frform.h"
#include "ghosttypes.h"
#include "formbar.h"

class FrArea;
class FrWnd;
class FrListBox;
class FrStatic;
class WTitleFont;
struct FrListItem;

class FrRecentMatch : public FrForm
{
public:
	DECLARE_OBJECT(FrRecentMatch)

	struct sRecentMatchForm
	{
		char gender;
		std::string name;
		int win;
		unsigned char grade;
		SYSTEMTIME time;
		int score;
	};

	FrRecentMatch();
	virtual ~FrRecentMatch();

	void Init(const std::string& name, int course, int ball, int pang,
		int score);
	void AddHistory(const sRecentMatchForm& form);
	void AddFinish();
	void Exception()
	{
		m_pText->SetCaption(
			"\xbc\xad\xb9\xf6\xc0\xc7 \xbf\xe4\xc3\xbb\xc0\xcc \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3\xc8\xc4 \xb4\xd9\xbd\xc3 \xc0\xcc\xbf\xeb\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
	}

protected:
	void OnInitBgArea(int param);
	void OnDrawBgArea();
	void OnInitHistoryList(int param);
	void OnDrawHistoryList(int param);
	void OnInitStaticText1(int param);

	const Bitmap* m_pCourseImg;
	const Bitmap* m_pBallImg;
	const Bitmap* m_pPangImg;
	int m_pang;
	int m_score;
	FrListBox* m_pHistoryList;
	std::list<sRecentMatchForm*> m_history;
	FrArea* m_pBgArea;
	FrStatic* m_pText;
	WTitleFont* m_pFont;
	std::string m_name;

	DECLARE_FRESH_MSGMAP()
};

class FrGhostTimeBar : public FrFormBar
{
public:
	DECLARE_OBJECT(FrGhostTimeBar)

	FrGhostTimeBar();
	virtual ~FrGhostTimeBar();

	virtual bool Open(FRESH_PFN_RESULT pFnResult, unsigned long flag);

protected:
	virtual void OnProc(const float deltaTime);

	void OnInitMouseArea(int param);
	void OnDrawMouseArea();

	float m_time;
	float m_closeTime;
	float m_gauge;
	bool m_bTimeOver;
	FrArea* m_pMouseArea;
	const Bitmap* m_pMouseImg;

	DECLARE_FRESH_MSGMAP()
};

class FrGhostGameMapDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrGhostGameMapDlg)

	struct sImgInfo
	{
		sImgInfo(float _x, float _y)
			: x(_x),
			  y(_y),
			  alpha(0.0f),
			  lockAlpha(0.0f),
			  bPlayable(false),
			  bHasRecord(false),
			  bLocked(false),
			  bDrawn(false)
		{
		}

		void AnimateLock(float deltaTime);

		float x;
		float y;
		float alpha;
		float lockAlpha;
		bool bPlayable;
		bool bHasRecord;
		bool bLocked;
		int courseIndex; // 20 = posicao vazia
		bool bDrawn;
	};

	struct sImgTag
	{
		void Animate(float deltaTime);
		void Reset();

		const Bitmap* pBg;
		const Bitmap* pImg;
		WRect rect;
		float destX;
		float y;
		float width;
		float height;
		float offset;
		float alpha;
		float glow;
	};

	FrGhostGameMapDlg();
	virtual ~FrGhostGameMapDlg();

	void Init(unsigned long uid, const std::string& name,
		const std::list<GHOST_GAME_BRIEF>& briefList);
	void* GetSelectItem();
	std::string GetGhostNick() { return m_name; }
	virtual void OnProc(const float deltaTime);

protected:
	void OnDownOK();
	void OnDownCancel();
	void OnInitRecentMatch(int param);
	void OnInitSelMapList(int param);
	void OnInitAreaSelecText(int param);
	void OnDrawSelecText();
	void OnInitAreaMap(int param);
	void OnDrawAreaBg(int param);
	void OnInitAreaBg(int param);
	void OnDrawSelMapList(int param);
	void OnDownSelMapList();
	void OnDrawChoiceMap(int param);
	void OnDownRecentMatch();

	std::vector<sImgInfo> m_imgInfo;
	std::vector<sImgTag> m_imgTag;
	std::list<GHOST_GAME_BRIEF> m_briefList;
	unsigned long m_ownerUid; // 983: uid do dono dos fantasmas (Init; 5o Encode4 do 0xc2 sub 9)
	std::string m_unused983_140; // 983 +0x140: so ctor 0081fa20/dtor 0081f1f0 constroem/destroem; nenhum uso
	std::string m_name;
	WRect m_areaRect;
	FrListBox* m_pSelMapList;
	int m_selected;
	FrArea* m_pAreaMap;
	float m_tipAlpha; // alpha do ghost_map_select_tip (sobe 127.5/s ate 255)
	FrWnd* m_pRecentBtn; // botao "recent" (so visivel quando o dono e o proprio jogador)

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbInit2Funclets(); // HACK: EH funclets of the naked Init (nakedify)
#endif
};

class FrGaugeBar;
class FrGraphicInterface;

class FrGhostAward : public FrForm
{
public:
	DECLARE_OBJECT(FrGhostAward)

	class cPangFx
	{
	public:
		cPangFx();
		~cPangFx();

		void Create(const WRect& rect);
		bool IsAlive();
		void Proc(float deltaTime);
		void Render(FrGraphicInterface* device);

		const Bitmap* m_pImg;
		WRect m_rect;
		float m_speed;
		float m_alpha;
		float m_destY;
	};

	struct sResultInfo
	{
		std::string name;
		WPoint pos;
		std::string levelImg;
		int score;
		unsigned long exp;
		bool bGhost;
	};

	FrGhostAward();
	virtual ~FrGhostAward();

	virtual void OnProc(const float deltaTime);
	void Init(bool bUser);
	bool IsFinishCalcBonus();
	int GetAutoBonus() { return m_autoBonus; }
	int GetClickBonus() { return m_clickBonus; }
	unsigned char GetClickCount() { return m_clickCount; }
	int GetBasePang() { return m_basePang; }

protected:
	void OnInitDescText(int param);
	void OnInitGaugeBar(int param);
	void OnInitGaugeText(int param);
	void OnInitPouchArea(int param);
	void OnDrawScore(int param);
	void OnDrawPouch(int param);

	void SetResultInfo(sResultInfo& info, int index);
	void AwardProc(float deltaTime);

	FrGhostTimeBar* m_pTimeBar;
	FrArea* m_pPouchArea;
	FrGaugeBar* m_pExpGauge;
	FrStatic* m_pDescText;
	FrStatic* m_pGaugeText;
	int m_pad983_124[2]; // 983 +0x124/+0x128: sem leitura/escrita (nem no ctor 0081e320)
	std::list<cPangFx*> m_pangFxList;
	float m_calcBonusTime;
	unsigned char m_clickCount;
	WTitleFont* m_pFont;
	int m_autoBonus;
	int m_clickBonus;
	int m_basePang;
	float m_shownBasePang;
	int m_gainExp;
	int m_remainExp;
	sResultInfo m_myResult;
	sResultInfo m_ghostResult;
	int m_pad983_1f4; // 983 +0x1f4: sem leitura/escrita (new 0x200 em 0081f503)

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOndrawpouchFunclets(); // HACK: EH funclets of the naked OnDrawPouch (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbInitFunclets(); // HACK: EH funclets of the naked Init (nakedify)
#endif
};
