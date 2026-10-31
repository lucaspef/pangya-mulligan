#pragma once

#include <vector>
#include <list>

#include "fx.h"

#include "parttidlist.h"
#include <list>
#include <vector>
#include "fxbox.h"
#include "parttidlist.h"
#include "wflag.h"

class CPetFrame;
class CFxSpray;
class CFxSequence;
struct sTexturePart;

enum eExhibitType;
struct w_motion_data;

enum eExhibitType
{
	EXHIBIT_CHARACTER = 0,
	EXHIBIT_CADDIE = 1,
	EXHIBIT_BALL = 2,
	EXHIBIT_CLUB = 3,
	EXHIBIT_MATCH = 4,
	EXHIBIT_ITEM = 5,
	EXHIBIT_MASCOT = 6,
	EXHIBIT_FURNITURE = 7,
	EXHIBIT_FURNITURE_BASE = 8,
	EXHIBIT_FURNITURE_WALL = 9,
	EXHIBIT_FURNITURE_FLOOR = 10,
	EXHIBIT_FURNITURE_ROOM = 11,
	EXHIBIT_CARD = 12,
	EXHIBIT_FURNITURE_TEXTURE = 13,
};

class FxInterpreter;

// JP 983: classe abstrata (vtable 00d3454c: [0] destrutor, [1] Display, [2] SetModel = purecall) com duas derivadas que so
// escolhem o modo de desenho (CExhibitionByHardwareTnL / CExhibitionBySoftwareTnL, abaixo). Os efeitos passam pelo
// FxInterpreter do objeto (+0x548): sairam os vetores de sprays/sequencias e as asas do 645. Layout medido no construtor
// 00877e30 e no histograma da unidade 00873800-00878c90 (out/F3.5/s115/RELATORIO.md); sizeof 0x64c (C_ASSERT no .cpp).
class CExhibition
{
public:
	CExhibition(float x, float y, float anchor, unsigned long uid, bool bAutoRecover); // 00877e30
	virtual ~CExhibition() = 0; // 00877180
	virtual void Display(float x, float y) = 0;
	virtual bool SetModel(CPartTidList& parts, unsigned long* auxParts,
		const char* motion, float delay, int option) = 0;

	bool SetModel(unsigned long typeId);
	bool SetModel(const char* name, eExhibitType type);
	void ParseBoundingBoxScript(); // 645 (so no ramo de asm; o 983 nao tem: AttachFx)
	void AttachFx();
	void AttachBallFx983(); // 983 00875310: efeito da bola (sBall::Fx[0]) nas bolas especiais
	void AttachAngelWingFx();

	void Process(float dt, bool bInput);
	int ProcessBongdariShop(float dt);
	void DisplayClip(const WRect& rect, float x, float y, float scale);
	// 983 00873f10 (nova; FrBongdariShopDlg 008b9587): DisplayClip com a camera por valor e o modo de projecao reposto
	void DisplayCamera983(bool unused, float nearClip, float farClip, float fov, const WRect& clip, WMatrix camera);

	bool SetMotion(const char* name, bool bContinue, float time);
	bool NextMotion();
	void SetArea(float w, float h);
	void SetDistance(float distance, bool bZoom, bool bFixed);
	void SetDirection(float direction);
	void SetDefaultDirection(float direction);
	const char* GetMotionName();
	w_motion_data* GetMotionData();
	const char* GetNextCharMotion(int index);
	const char* GetNextCaddieMotion(int index);

	void SetCenter(float x, float y);
	const WVector2D& GetCenter();
	void SetArea(float radius);
	float GetDistance();
	void Zoom(float delta);
	bool IsDirectionChanged();
	void SetAlpha(unsigned char alpha);
	void SetChatFacial(int facial, unsigned long time);
	bool ChangeBoneTexture(const char* bone, const char* from, const char* to);
	bool ChangeBoneTexture(const char* from, const char* to);

	void ResetCameraRecoverTime() { m_autoRotateTimer = 0.0f; }

	bool IsNextMotion() { return m_bMotionChanged; }

	void SetDistanceRange(float fMin, float fMax)
	{
		m_minDistance = fMin;
		m_maxDistance = fMax;
	}
#ifdef REBANG_TUTORIALMAIN_CPP
	// HACK: tutorialmain emits this one wave late (after the PCH inlines); the dead reference defers it.
	void SetControlFlag(unsigned long flag)
	{
		if (0)
			cos(0.0f);
		m_flags.Set(flag);
	}
#else
	void SetControlFlag(unsigned long flag) { m_flags.Set(flag); }
#endif
	void SetAutoRecoverCamera(bool bEnable) { m_bAutoRotate = bEnable; }
	void SetEnableRotate(bool bEnable) { m_bEnableRotate = bEnable; }
	void SetEnableZoom(bool bEnable) { m_bEnableZoom = bEnable; }

	WPuppet* GetModel() { return m_pPuppet; }
	CPetFrame* GetPetFrame() { return m_pPetFrame; }
	float GetInitialMotionLength() { return m_motionLength; }

	unsigned long GetTypeId() const { return m_tid; }
	float GetDirection() { return m_yaw; }
	WMatrix& GetMatrix() { return m_modelMat; }

protected:
	// corpos comuns das derivadas (nomes meus): o modo so muda o ultimo argumento do CPetFrame::Render e o 4o do Build
	void DisplayTnL(bool bHardwareTnL, float x, float y); // 00874af0
	bool SetModelTnL(bool bHardwareTnL, CPartTidList& parts, unsigned long* auxParts,
		const char* motion, float delay, int option); // 00876d90

	void Init();
	void ReleasePet();
	void UpdateCamera(float dt);
	void Animate();
	void BillboardToCamera983(); // 00874850: gira o modelo em y para encarar a camera (m_bPetBillboard983)
	bool InValidArea();
	float GetBaseAngle(float x, float y);
	void SetScreenPos(WPuppet* pPuppet);
	void ScriptProcess(const w_motion_data* motion, float from, float to,
		bool bInit);
	void ParsePuppetScript(const char* script);
	void ParseScript_ShowVis(bool bShow, const char* groups);
	sTexturePart* FindTexturePart(char* target, char* texture);
	void SetPendingTexPart(char* target, const char* texture);

public:
	float m_idleTimer;
	float m_motionLength;
	int m_motionCursor;
	int m_charIndex;
	int m_caddieIndex;
	int m_rotateState;
	bool m_bAutoRotate;
	bool m_bLeftDown;
	bool m_bRightDown;
	float m_alpha;
	float m_lastYaw;
	float m_pitch;
	float m_yaw;
	float m_pitchSpeed;
	float m_yawSpeed;
	float m_defaultPitch;
	float m_defaultYaw;
	float m_distance;
	float m_targetDistance;
	float m_defaultDistance;
	float m_scale;
	float m_autoRotateTimer;
	float m_pitchLimit;
	int m_unused54;
	float m_areaW;
	float m_areaH;
	WVector2D m_center;
	float m_areaRadius;
	WMatrix m_camera;
	float m_motionTime;
	w_motion_data* m_pMotion;
	CPetFrame* m_pPetFrame;
	WPuppet* m_pPuppet;
	WMatrix m_modelMat;
	int m_msgTexture;
	LightSet m_light;
	float m_minDistance;
	float m_maxDistance;
	CPvsFxBox m_fxBox;
	FxInterpreter* m_pFxInterpreter; // 983 +0x548: new 0x1e0 no construtor, ligado ao m_fxBox
	std::list<sTexturePart*> m_texParts;
	int m_type;
	unsigned long m_tid;
	unsigned long* m_pUids;
	WFlags m_flags;
	bool m_bMotionChanged;
	float m_dirTimer;
	float m_facialTimer;
	bool m_bDirChanged;
	bool m_bChatFacial;
	bool m_unused4b2; // 983 +0x576: o construtor nao grava mais
	CPartTidList m_partTids; // +0x578
	bool m_bEnableZoom; // +0x640
	bool m_bEnableRotate;
	bool m_bMotionToggle983; // +0x642 (nome da B: mensagem 0x199 com o mesmo movimento)
	bool m_bRandomMotion983; // +0x643 (nome da B: troca o movimento de espera sozinho)
	unsigned long m_uid; // +0x644: chave do CMotionAdvisor (SetMotion)
	bool m_bPetBillboard983; // +0x648: "*petbillboard" do script do boneco (00876407); o Animate vira o modelo para a camera
};

// vtable 00d05e9c. Os 22 lugares do 983 que criam um boneco usam esta; 21 deles embutem o construtor com uid -1
// (fora de linha: 004ff080, ordem (x, y, ancora, bool, uid)).
class CExhibitionByHardwareTnL : public CExhibition
{
public:
	CExhibitionByHardwareTnL(float x, float y, float anchor, bool bAutoRecover, unsigned long uid = 0xffffffff)
		: CExhibition(x, y, anchor, uid, bAutoRecover)
	{
	}
	virtual ~CExhibitionByHardwareTnL() { } // 004ff0d0
	virtual void Display(float x, float y) { DisplayTnL(true, x, y); } // 00874e70
	virtual bool SetModel(CPartTidList& parts, unsigned long* auxParts, const char* motion, float delay, int option)
	{
		return SetModelTnL(true, parts, auxParts, motion, delay, option); // 00877120
	}
	using CExhibition::SetModel;
};

// vtable 00d13f30 (destrutor 006047a0); ninguem constroi no 983
class CExhibitionBySoftwareTnL : public CExhibition
{
public:
	CExhibitionBySoftwareTnL(float x, float y, float anchor, bool bAutoRecover, unsigned long uid = 0xffffffff)
		: CExhibition(x, y, anchor, uid, bAutoRecover)
	{
	}
	virtual ~CExhibitionBySoftwareTnL() { }
	virtual void Display(float x, float y) { DisplayTnL(false, x, y); } // 00874e90
	virtual bool SetModel(CPartTidList& parts, unsigned long* auxParts, const char* motion, float delay, int option)
	{
		return SetModelTnL(false, parts, auxParts, motion, delay, option); // 00877150
	}
	using CExhibition::SetModel;
};
