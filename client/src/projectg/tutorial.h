#pragma once

#include <deque>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include "woverlay.h"
#include "taskmain.h"

class AnimatedOverlay
{
public:
	AnimatedOverlay();
	~AnimatedOverlay();

	void SetSourceOverlay(const char* filename, int cols, int rows, int mode,
		float width, float height);
	void Activate(bool active, bool fade, bool loop);
	bool IsActivate() { return m_bTarget; }
	void Process(float delta);
	void Render();

protected:
	friend class CTutorial;
	void GetNextOverlay();

	WRect m_dest;
	float m_frameTime;
	WRect m_src;
	float m_elapsed;
	int m_frame;
	int m_frameCount;
	bool m_bShow;
	bool m_bTarget;
	bool m_bLoop;
	int m_fade;
	float m_alpha;
	WOverlay* m_pOverlay;
	int m_cols;
	std::vector<WOverlay*> m_overlays;
};

class FrQuizDlg;
class CTutorialContainer983;

enum eMacroType
{
	MACRO_00,
	MACRO_01,
	MACRO_02,
	MACRO_03,
	MACRO_04,
	MACRO_05,
	MACRO_06,
	MACRO_07,
	MACRO_08,
	MACRO_09,
	MACRO_0A,
	MACRO_0B,
	MACRO_0C,
	MACRO_0D,
	MACRO_0E,
	MACRO_0F,
	MACRO_10,
	MACRO_11,
	MACRO_12,
	MACRO_13,
	MACRO_14,
	MACRO_15,
	MACRO_16,
	MACRO_17,
	MACRO_18,
	MACRO_19,
	MACRO_1A,
	MACRO_1B,
	MACRO_1C,
	MACRO_1D,
	MACRO_1E,
	MACRO_1F,
};

struct sMacro
{
	eMacroType type;
	char str[32];
	int value;
};

class CTutorial : public CTaskMain
{
	DECLARE_OBJECT(CTutorial)

public:
	virtual ~CTutorial();
	virtual void OnInit();
	virtual void OnLoad();
	virtual void OnProcess(float);
	virtual void OnDisplay();
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject&);

protected:
	CTutorial();

	bool OnTipCaddieReopenQuizResult(int, FrForm*);
	bool OnTipCaddieNoneResult(int, FrForm*);
	bool OnQuizFormResult(int, FrForm*);
	bool OnTipFormResult(int, FrForm*);
	bool OnMissionFormResult(int, FrForm*);
	bool OnPracticeFormResult(int, FrForm*);
	bool OnCompleteFormResult(int, FrForm*);
	bool OnReviewFormResult(int, FrForm*);
	bool OnSkipMissionFormResult(int, FrForm*);
	bool OnMissionFailedFormResult(int, FrForm*);
	bool OnTipCaddieFormResult(int, FrForm*);
	bool CheckMissionComplete();
	void MissionComplate();
	void PrepareMission();
	void SkipMission();
	void ReviewTutorial();
	void MoveNextStep();
	void ChanceOverlay();
	void OnImageLoad();
	void Again_Msg(bool);
	void OpenTipCaddieForm(const char*, FRESH_PFN_RESULT);
	void OpenMissionForm();
	void OpenQuizForm(int);
	void OpenMissionFailedForm();
	char* GetLine(char*, char*);
	void BlockInput();
	void FreeInput();
	void IsPractice();
	void PriviewOverlay();
	void PushMacroCamera();
	void PushMacroClub();
	void PushMacroShot();
	void PushMacroGreen();
	void PushMacroGauge();
	void PushMacroGround();
	void PushMacroItemUse();
	void PushMacroScore();
	void PushMacroHoleIn();
	void PushMacroWindReading();
	void PushMacroBallMove();
	void PushMacroBallSlope();
	void PushMacroHitPointMission();
	void PushMacroPowerSpin();
	void PushMacroPowerCurve();
	void PushMacroHitPoint();

private:
	bool CheckCameraTarget();
	bool CheckClubTarget();
	WVector* GetPoint(const char*);
	void ClearMacro();
	void InitTutorialMode(int);
	void PopMacro();
	void PopSpecial();
	void PushMacro(eMacroType, const char*, int, int);

protected:
	std::queue<sMacro*> m_specials;
	std::queue<sMacro> m_macros;
	int m_macroWait;
	char m_pad20c[0x10];
	bool m_b21c;
	int m_state;
	int m_mode;
	int m_228;
	int m_22c;
	std::map<int, std::string> m_lines;
	int m_missionState;
	float m_240;
	float m_244;
	FrForm* m_pForm248;
	FrForm* m_pNotifyForm;
	FrForm* m_pReviewForm;
	FrForm* m_pTipForm;
	FrWnd* m_pTipCaddieWnd;
	int m_25c;
	int m_260;
	FrForm* m_pForm264;
	WOverlay* m_pOverlay;
	FrQuizDlg* m_pQuizDlg;
	int m_270;
	int m_vals[10];
	bool m_b29c;
	int m_2a0;
	bool m_b2a4;
#pragma pack(push, 1)
	struct sPacked
	{
		int a;
		int b;
		int c;
		short d;
		bool e;
	} m_packed;
#pragma pack(pop)
	bool m_b2b4;
	int m_2b8;
	float m_2bc;
	float m_2c0;
	float m_2c4;
	float m_2c8;
	float m_2cc;
	int m_2d0;
	bool m_b2d4;
	bool m_b2d5;
	float m_2d8;
	int m_2dc;
	bool m_bQuizCorrect;
	AnimatedOverlay m_overlays[26];
	AnimatedOverlay m_overlays2[3];
	AnimatedOverlay m_overlay3;
	AnimatedOverlay m_overlays4[2];
	bool m_bInputBlocked;
	CTutorialContainer983* m_pTutorialDoc983; // 983: documento 0xa4 (OnInit 007ccf20); zerado no fim do construtor (007cdc98)
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbPopmacroFunclets(); // HACK: EH funclets of the naked PopMacro (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOpentipcaddieforFunclets(); // HACK: EH funclets of the naked OpenTipCaddieForm (nakedify)
#endif
};
