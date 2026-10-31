#pragma once

#include "wlist.h"
#include "wfont.h"
#include "wresrcmng.h"
#include <map>
#include <string>
#include "finitestatemachine.h"
#include "encrypttypes.h"
#include "caddiebonus.h"
#include "../shared/globalgamedefine.h"

class WOverlay;
class CSimpleUI;

class CFontInfo
{
public:
	struct sFontInfo : WTITLEFONT
	{
		int id;
		char keycode[32];
	};

	// 983: variante da fonte para uma escala de tela (valor do mapa do sFont; 006e1830/006e2600)
	struct sScaledFont
	{
		float scale;      // +0x00 escala da imagem (2.0 = "_x2")
		std::string name; // +0x04
		WTitleFont* pFont; // +0x20 criada na 1a vez que a escala e pedida
	};

	struct sFont
	{
		WTitleFont* pFont;
		sFontInfo* pInfo;
		std::map<float, sScaledFont> scaled; // 983 +0x08 (0x10)

		void AddScaled(float scale, const char* name); // 006e2600
		void FindScaled(float scale, WTitleFont** font, float* fontScale); // 006e1830
	};

	CFontInfo();
	virtual ~CFontInfo();

	// 983 (vtable 00d1a7a4): 5 virtuais, sem o GetScreenPos do 645
	virtual void Process(float dt) { }
	virtual void Display() { }
	virtual void Init(char* name, sFont* font = NULL);
	void SetColor(unsigned long color) { m_color = color; }
	virtual void Reset() { }

	void Clear();
	float GetTextWidth(const char* text, float spacing, float scale);
	float GetExactTextWidth(const char* text, float spacing, float scale);
	float Print(float x, float y, const char* text, float spacing, int type,
		float scale, sFont* font);
	// 983: texto na escala da tela (Dim de scltype `sclType` sobre tela / 640x480), com a fonte "_xN" mais perto
	float GetScaledTextWidth(int sclType, const char* text, float spacing, float scale,
		float* charX, float* charW, int type); // 006e1de0
	float PrintScaled(int sclType, float x, float y, const char* text, float spacing,
		float scale, int type); // 006e2190

protected:
	char*& Buffer() { return m_buffer; }

private:
	void AddFont(const char* name, int a, int b, int c, int d, int e,
		char* chars);

protected:
	static WList<sFontInfo*> m_fontList;
	static bool m_loaded;

protected:
	sFont m_font;           // +0x04 (0x18)
	unsigned long m_color;  // +0x1c
	int m_align;            // +0x20 horizontal: 1 centro, 2 direita
	int m_valign;           // +0x24 983: vertical do PrintScaled: 1 centro, 2 embaixo
	char* m_buffer;         // +0x28
};

class CDistToHoleInfo : public CFontInfo
{
public:
	CDistToHoleInfo();

	virtual void Process(float dt);
	virtual void Display();
	virtual void Reset();
	virtual void ResetUiPosition();

	void Fix();

private:
	float m_distance;
	int m_state;
	float m_timer1;
	float m_timer2;
	float m_x;
	float m_y;
	float m_scale;
	float m_targetX;
	float m_targetY;
	float m_targetScale;
	float m_curX;
	float m_curY;
	float m_curScale;
	float m_alpha;
	float m_offset;
	float m_pad54;
};

class CHoleCupInfo : public CFontInfo
{
public:
	CHoleCupInfo();
	virtual ~CHoleCupInfo();

	virtual void Reset();
	virtual void Process(float dt);
	virtual void Display();
	void GetScreenPos(float* x, float* y); // 983: fora da vtable (00d088a8 tem 5)

	void Fix();
	void ResetUiPosition();

private:
	float m_x;
	float m_y;
	float m_height;
	WOverlay* m_overlay[3];
	int m_state;
	CDistToHoleInfo* m_distInfo;

	friend class CFontOverlay;
};

class CTimeOutInfo : public CFontInfo
{
public:
	CTimeOutInfo();

	virtual void Reset();
	virtual void Process(float dt);
	virtual void Display();

private:
	int m_state;
	float m_alpha;
	float m_timer;
};

class CClubRangeInfo : public CFontInfo
{
public:
	CClubRangeInfo();

	virtual void Display();

	WVector m_pos;
};

class CWindInfo : public CFontInfo
{
public:
	CWindInfo();

	virtual void Display();
};

class CRemainTimeInfo : public CFontInfo
{
public:
	CRemainTimeInfo();

	virtual void Display();
};

class CTotalDistInfo : public CFontInfo
{
public:
	CTotalDistInfo();

	virtual void Display();

	float m_distance;
};

class CPowerInfo : public CFontInfo
{
public:
	CPowerInfo();

	virtual void Display();

#ifdef REBANG_GUI_CONTROL_CPP
	void SetPos(const WVector& pos) { m_pos = pos; }
#endif

	WVector m_pos;
};

class CClubTypeInfo : public CFontInfo
{
public:
	CClubTypeInfo();

	virtual void Display();
};

class CShotCountInfo : public CFontInfo
{
public:
	CShotCountInfo();
	virtual ~CShotCountInfo();

	virtual void Process(float dt);

	void StartFloating();
	void Display(float x, float y, unsigned char index);

private:
	WOverlay* m_coin;
	unsigned char m_alpha;
	float m_timer;
	int m_count;
	sFont m_fontRed;
	sFont m_fontGrey;
	sFont m_fontBlue;
	void* m_pTikiData983; // 983 +0x84: CContentsDoc::GetContainer(0xaa), sala tiki (gameType 0xb)
};

class CFlyInfo : public CFontInfo
{
public:
	CFlyInfo();
	virtual ~CFlyInfo();

	virtual void Process(float dt);
	virtual void Display();

private:
	void CalcDistance();

	float m_time;
	CSimpleUI* m_groundUI;
	CSimpleUI* m_textUI;
	WOverlay* m_overlay;
	float m_alphaGround;
	float m_alphaTextA;
	float m_alphaTextB;
	float m_carry;
	float m_total;
	char m_ground[8];
};

class CBonusInfo : public CFontInfo
{
public:
	CBonusInfo();
	virtual ~CBonusInfo();

	virtual void Process(float dt);
	virtual void Display();

private:
	WOverlay* m_pangIcon;
	WOverlay* m_pcBangIcon;
	float m_time;
	int m_unused24;
	__int64 m_pang;
	__int64 m_bonusPang;
	__int64 m_expPang;
	__int64 m_skinsPang;
	__int64 m_total;
	std::map<unsigned long, WOverlay*> m_itemIcons;
	WOverlay* m_localIcon;
};

class CAccuracyInfo : public CFontInfo
{
public:
	CAccuracyInfo();
	virtual ~CAccuracyInfo();

	virtual void Process(float dt);
	virtual void Display();

	void SetPower();
	void SetAccuracy();

private:
	char*& Buffer2() { return m_buffer2; }

	bool m_active;      // 983 +0x2c
	float m_alpha;      // 983 +0x30
	float m_x;          // 983 +0x34 (x da batida na barra)
	char* m_buffer2;    // 983 +0x38
};

class CGuildInfo : public CFontInfo
{
public:
	struct sScore
	{
		int a;
		int b;

		void Set(int x, int y)
		{
			a = x;
			b = y;
		}
	};

	CGuildInfo();
	virtual ~CGuildInfo();

	virtual void Process(float dt);
	virtual void Display();

	void SetScore(int a, int b);

protected:
	char*& Buffer2() { return m_buffer2; }

private:
	int m_unused18;
	float m_alpha;
	float m_scale;
	sFont m_fontRed;
	sFont m_fontBlue;
	char* m_buffer2;
	sScore m_score;
	sScore m_prevScore;
	const char* m_sfx;
};

class CHoleInfo : public CFontInfo
{
public:
	CHoleInfo();
	virtual ~CHoleInfo();

	virtual void Process(float dt);
	virtual void Display();
	virtual void Reset();

	void ResetUiPosition();

	void Flicker();
	void Fix();

private:
	struct sSlide
	{
		float startX;
		float startY;
		float endX;
		float endY;
		float x;
		float y;
		float scale;
	};

	sSlide m_slide[4];
	float m_alpha[2];
	float m_time;
	int m_flicker;
	int m_state;
	CSimpleUI* m_ui[5];
	CSimpleUI* m_pCupUI983; // 983 +0xc4: "[cuplevel.jpg" (Cup/Scr), sala tiki (gameType 0xb)
	void* m_pTikiData983; // 983 +0xc8: CContentsDoc::GetContainer(0xaa)
	WOverlay* m_overlay[12];
	unsigned char m_player;
	int m_emote;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCholeinfoFunclets(); // HACK: EH funclets of the naked CHoleInfo (nakedify)
	CHoleInfo(struct
		RbVtableUse*); // HACK: see fontinfo.cpp (emission order of ??_GCHoleInfo)
#endif
};

class CExtraHoleInfo : public CFontInfo
{
public:
	CExtraHoleInfo();
	virtual ~CExtraHoleInfo();

	virtual void Display();

	void SetShotTime(unsigned long time);
	void UpdateLeader();

private:
	unsigned long m_shotTime[4];
	CSimpleUI* m_ui;
	unsigned char m_leader;
	unsigned long m_holeTime[4];
};

class CHolePoseSkins : public CFontInfo
{
public:
	CHolePoseSkins();
	virtual ~CHolePoseSkins();

	virtual void Process(float dt);
	virtual void Display();

	void SetPose(int pose, int count);

private:
	WOverlay* m_win[3];
	WOverlay* m_lose[2];
	WOverlay* m_draw[2];
	WOverlay* m_poseA;
	WOverlay* m_poseB;
	int m_count;
	int m_pose;
	float m_alpha;
	float m_delay;
	int m_state;
};

class CFontOverlay
{
public:
	CFontOverlay();
	~CFontOverlay();

	void Init();
	void Process(float dt);
	void Display();
	void ResetUiPosition();

#ifdef REBANG_GUI_CONTROL_CPP // HACK: other includers keep their IL (emission order)
	enum eInfo
	{
		INFO_HOLECUP,
		INFO_FLY,
		INFO_HOLE,
		INFO_ACCURACY,
		INFO_TIMEOUT
	};

	void FixHoleInfo();
	void FixHoleCupInfo();
	void FlickerHoleInfo();
	void ResetHoleInfo();
	void ResetHoleCupInfo();
	void ResetTimeOutInfo();
	void GetHoleInfoPos(float* x, float* y);
	void SetAccuracy();
	void SetActive(eInfo info, bool active);
#endif

private:
	CHoleCupInfo* m_holeCup;
	CFlyInfo* m_fly;
	CHoleInfo* m_hole;
	CAccuracyInfo* m_accuracy;
	CTimeOutInfo* m_timeOut;
	bool m_showHoleCup;
	bool m_showFly;
	bool m_showA;
	bool m_showAccuracy;
	bool m_showTimeOut;
};

// JP 983: CCalipers reescrita (= 839; construtor 00538b30, vtable 00d08e20, 0x568). O 645 desenhava a barra com o
// "{indicator.bmp"/"callipers_o.png" em posicoes fixas; o 983 guarda 12 modos (entradas protegidas por WCrypticValue), mede
// a posicao pelo CUiFrameSetManager ("d_default_pangya_x", "d_calipers_center_x"), desenha os quadros "f_*calipers*" e
// arredonda o limite ao passo do modo (SnapValue983). O CScreen so chama Process/Display (o modo decide se aparece).
class CCalipers : public CFontInfo
{
public:
	// modo (0x68): passo da barra, formato do texto da distancia, fase minima da tacada para o texto e se a barra mexe
	struct sMode
	{
		sMode() {} // 0052fdb0
		sMode(const char* name, float step, const char* format, int minPhase, bool active); // 00533e90
		sMode& operator=(const sMode& other); // 00537680

		std::string m_name;              // +0x00 (sempre "" no 983)
		WCrypticValue<float> m_step;     // +0x1c passo em pixels da barra de 360 (36, 6 ou 1)
		std::string m_format;            // +0x2c "%.0fy"/"%.1fy" ("" = sem texto)
		WCrypticValue<int> m_minPhase;   // +0x48 texto so com GOLFDOC+0x188 >= este valor (4 = nunca)
		WCrypticValue<bool> m_active;    // +0x58 barra controlavel (teclas/mouse) e quadros de tecla
	};

	CCalipers(); // 00538b30
	virtual ~CCalipers(); // 0052fe50

	virtual void Process(float dt); // 00534270
	virtual void Display(); // 00539a50
	virtual void Reset(); // 0053d6c0

	bool IsVisiblePowerBar() const { return (m_visible == 1) ? true : false; }
	void VisiblePowerBar(int visible) { m_visible = visible; }
	void SetMode983(int mode); // 00526e60: modos 2*mode e 2*mode+1 (taco/putter) fora do item; CScreen::OnLoad: 4 (tipo 0xb) ou 2
	void SetCalipersLimit(float limit); // 0052b6c0
	void ResetBar(); // 0052b920
	void SetEvent(int input); // 00526e90
	void MoveLeft(); // 0052b780
	void MoveRight(); // 0052b870
	void KeyInputEnd(); // 0052b900
	void ExecuteKeyAction(int action); // 0052fef0
	void ChangeKeyState(); // 0052ff20
	void ProcKeyState(float dt); // 00531300
	void DrawShotPowerRange(float& x, bool limit); // 00533fc0: texto da distancia
	void DrawLimitLine(float& x, bool line); // 005397a0: indicador "f_*_calipers_indicator*"

private:
	float GetStep(); // 0052b690: passo do modo atual (36 sem modo)
	// 00528c70: com passo > 1 arredonda (valor - 140) ao multiplo do passo mais proximo; limita a [lo, hi] e tira a fracao
	static void SnapValue983(float& out, float value, float lo, float hi, float step);

	sMode m_modes[12];        // +0x2c
	sMode* m_mode;            // +0x50c modo atual (Process)
	sMode* m_modeBase;        // +0x510 SetMode983
	FiniteStateMachine m_fsm; // +0x514
	float m_limit;            // +0x52c
	float m_target;           // +0x530
	int m_prevKeys;           // +0x534
	unsigned int m_keys;      // +0x538
	int m_prevState;          // +0x53c
	unsigned long m_state;    // +0x540
	unsigned char m_started;  // +0x544
	unsigned char m_reset;    // +0x545
	float m_repeat;           // +0x548
	float m_speed;            // +0x54c
	int m_visible;            // +0x550
	WOverlay* m_overlay;      // +0x554 "callipers_o.png" (o 983 so testa se carregou)
	unsigned char m_moving;   // +0x558
	float m_mouseX;           // +0x55c
	float m_flashTime;        // +0x560
	unsigned char m_flashing; // +0x564
	unsigned char m_canFlash; // +0x565
};

// JP 983: regua de distancia do Hole-in-One (tipo 0xb com realGameType 0xe) = membro +0x260 do CSpot (new 0x18 no OnLoad
// 00729f80; desenhada pelo CSpot 0072c430). Com a camera baixa perto do pino desenha 41 tracos no centro da tela.
class CHoleRuler983
{
public:
	CHoleRuler983() : m_bValid(false) {} // 00529170
	void Display(WView* view); // 0052bf60

private:
	bool m_bValid;    // +0x00 chao sob a camera ja calculado
	float m_x;        // +0x04 camera.pivot.x do calculo
	float m_z;        // +0x08 camera.pivot.z do calculo
	WVector m_ground; // +0x0c
};

class CPangInfo : public CFontInfo
{
public:
	CPangInfo();
	virtual ~CPangInfo();

	struct sPangCase
	{
		sPangCase();

		unsigned int flags;
		int field4;
		union
		{
			struct
			{
				int field8;
				int fieldC;
			};
			__int64 value;
		};
		int field10;
		float field14;
		float field18;
		float field1C;
		int field20;
		float field24;
		float field28;
		float field2C;
		float field30;
		unsigned char flag34;
		unsigned char pad35[3];
	};

	struct sPangSlot
	{
		int state;
		int pad4;
		__int64 total;
		int field10;
		float field14;
		float field18;
		float field1C;
		int overlayIndex;
		_WRECT rect24;
		unsigned char alpha34;
		unsigned char pad35[3];
	};

	void Restore();
	void Skip();

protected:
	void ResetVars(int flags, int state);
	void ProcessHappyBonus(float dt);
	void ProcessAztecBonus(float dt);
	void ProcessAngelBonus(float dt);
	void ProcessDevilBonus(float dt);
	void MakeDecision(int index, bool bonus);
	void Insert(int index);
	void Display_TotalPang();
	void Display_TotalGimmickNum();
	void SuperPangya();
	void AngelWingBonus();
	void DevilWingBonus();
	void MascotBonus();
	void MascotCase983(unsigned char strokes);
	void BallBonus983();
	void AbilityItemBonus();
	void RecoveryBonus();
	void LongDrive();
	void BeamImpact();
	void SpecialShot();
	void NiceRecovery();
	void Multiply(int index, int value);
	void ClashCombo();
	void DormieBonus();
	void HappyBonus();
	void AztecBonusWater();
	void GimmickItem(GIMMICK_ITEM_TYPE type);
	void AztecBonus();
	void BonusCase4();
	void ClearBonus();
	void CheckReadyToMakeDecision();
	void HoleIn();
	void Record(unsigned char index, __int64* values);
	__int64 GetHoleInPang(int a, int b);
	void UpdateBallState();
	void ProcessTotal(float dt);
	void ProcessCases(float dt);
	void GetHoleInOverlayInfo(int a, int b, unsigned int& id, float& x,
		float& y);
	void GetSpecialShotOverlayInfo(unsigned long flags, unsigned char type,
		unsigned int& id, float& x, float& y);
	void CheckReadyToFinalize(float dt);
	void Approach();
	void LongPutt();
	void LongChipIn();

public:
	void ChangeTurn();
	void ShowMyPang();
	void PreCalc(bool param);
	void PreCalcMatchHoleInBonus();
	void PreCalcTeamHoleInBonus();
	void ResetUiPosition();
	virtual void Process(float dt);
	virtual void Display();
#ifdef REBANG_GUI_CONTROL_CPP
	// clang-format off
	bool IsFinalized() { unsigned char finalized = (unsigned char)(m_flags >> 4); finalized &= 1; return finalized; }
	// clang-format on
#endif

protected:
private:
	WOverlay* m_overlay[2];
	unsigned int m_flags;
	const char* m_sound;
	__int64 m_gain[4];
	__int64 m_pang[4];
	__int64 m_shown[4];
	__int64 m_shownTotal[4];
	sPangSlot m_slot[4];
	sPangCase m_case[25];
	float m_dirX;
	float m_dirY;
	float m_dirZ;
	int m_f6d4;
	unsigned char m_pad6d8[8];
	float m_f6e0;
	float m_f6e4;
	float m_f6e8;
	float m_f6ec;
	int m_f6f0;
	int m_f6f4;
	float m_f6f8;
	float m_f6fc;
	float m_uiRect[16];
	float m_bonusTime[4];
	float m_f750;
	int m_f754;
	unsigned char m_f758;
	unsigned char m_pad759[7];
	__int64 m_extraPang[4];
	int m_f780;
	int m_f784;
	int m_f788;
	float m_f78c;
	float m_f790;
	float m_f794;
	float m_f798;
	CCaddieBonus m_caddieBonus;
	void* m_pTikiData983; // 983 +0x7fc: CContentsDoc::GetContainer(0xaa), sala tiki (gameType 0xb; construtor 005384c0)
	unsigned char m_applied[25]; // 983 this+0x800+i: case ja somado em ProcessCases
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCpanginfoFunclets(); // HACK: EH funclets of the naked CPangInfo (nakedify)
	CPangInfo(struct
		RbVtableUse*); // HACK: see fontinfo.cpp (emission order of ??_GCPangInfo)
#endif
};

#ifdef REBANG_GUI_CONTROL_CPP // HACK: other includers keep their IL (emission order)
inline void CFontOverlay::FixHoleInfo()
{
	m_hole->Fix();
}
inline void CFontOverlay::FixHoleCupInfo()
{
	m_holeCup->Fix();
}
inline void CFontOverlay::FlickerHoleInfo()
{
	m_hole->Flicker();
}
inline void CFontOverlay::ResetHoleInfo()
{
	m_hole->Reset();
}
inline void CFontOverlay::ResetHoleCupInfo()
{
	m_holeCup->Reset();
}
inline void CFontOverlay::ResetTimeOutInfo()
{
	m_timeOut->Reset();
}
inline void CFontOverlay::GetHoleInfoPos(float* x, float* y)
{
	m_holeCup->GetScreenPos(x, y);
}
inline void CFontOverlay::SetAccuracy()
{
	m_accuracy->SetAccuracy();
}
inline void CFontOverlay::SetActive(eInfo info, bool active)
{
	(&m_showHoleCup)[info] = active;
}
#endif
