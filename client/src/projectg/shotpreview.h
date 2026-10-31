#pragma once

#include <vector>
#include <string>
#include "actor.h"
#include "scenemanager.h"
#include "tweaker.h"
#include "objectfactory.h"

// Previsao de tacada de desenvolvedor do 645 so para GM (pedido do dono 28/10; nao existe no 983 puro): Ctrl+R liga/desliga a
// trajetoria completa com quiques e distancia ate o copo e mostra os radios predict_* (Normal/Tomahawk/Cobra/Spike). Play Assist intacto.
#define JP983_DEV_PREVIEW
#ifdef JP983_DEV_PREVIEW
bool DevPreviewCtrlR983(); // conta GM com Ctrl apertado: o R e da previsao (nao do replay nem da revisao de tacadas)
bool DevPreviewOn983();
#endif

class CGolfBall;
class C3dFont;
class FrButton;
class WPuppet;

class ShotPreview : public IActor,
					public CRenderFuncPtr,
					public tweaker_cmd_target
{
	friend IObject* ShotPreviewMakeInstance();

public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	struct PredictType
	{
		enum Enum
		{
			NORMAL,
			TOMAHAWK,
			COBRA,
			SPIKE
		};
	};

	class PangCoin
	{
	public:
		PangCoin(const WVector& pos, float scale, bool bDown);
		~PangCoin();

		void SetVisible(bool bVisible);
		void Restore();
		void Update(float dt);
		void Render();

	private:
		static int _PetReferenceCounter;

		WPuppet* m_pPuppet;
		std::string m_name;
		WMatrix m_matrix;
		float m_angle;
		float m_scale;
		bool m_bVisible;
		float m_radius;
		WVector m_pos;
	};

	// 983 this+0x60 (008430e0/008432c0/008433c0/00842da0/00843440): marcador/guia do Play Assist (assist_top/assist_side/assist_sphere)
	class AssistGuide
	{
	public:
		AssistGuide();
		void Reset(bool scale, bool randomDir);
		void Update(float dt);
		WVector GetVector();
		float GetClubScale();
		void SetScale(float scale) { m_scale = scale; } // 983 +0x1c (OnInit 008450b0)
		float GetScale() const { return m_scale; } // 983 +0x1c (OnProcess 00846470 le em ShotPreview+0x7c)
		void Draw();
		void DrawTrail983(const std::vector<WVector>& pts); // 983 00843f80
		void DrawFadeLine983(const std::vector<WVector>& pts); // 983 00842f90
		void UpdateBound983(const WVector& pos, float dist); // 983 008446c0

	private:
		void RandomizeDir();

		int m_texTop;
		int m_texTrajectory;
		int m_ten;
		WVector m_dir;
		float m_scale;
		bool m_bActive;
		float m_f24, m_f28, m_f2c, m_f30, m_f34, m_f38, m_f3c, m_f40, m_size;
		WVector m_pos;
		WPuppet* m_pSide;
		WPuppet* m_pSphere;
		WPuppet* m_pDraw;
	};

	class UpdateSensor
	{
	public:
		UpdateSensor();

		bool operator==(const UpdateSensor& other) const;
		bool OnlySpinCurveChanged(const UpdateSensor& other) const; // 983 00842ac0
		bool MovedSignificantly() const; // 983 00842ea0
		void Update();

	private:
		// layout do 983 (00844590): sensor de 0x24 bytes; o clubIndex (+9) cai sobre o 2o byte do angle (+8), como no original
		friend class ShotPreview;
		float m_spin;
		float m_curve;
		float m_angle;
		WVector m_wind;
		int m_powerShot;
		unsigned long m_item;
		float m_barPos;
	};

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void Display();

	std::string TwAddOption(const tweaker_param& param);
	std::string TwSubOption(const tweaker_param& param);

private:
	ShotPreview();
	virtual ~ShotPreview();

	virtual void HandleMsg(const MsgObject& msg);

	unsigned int GetOptionFlag() const { return m_optionFlag; }
	void SetOptionFlag(unsigned int flag) { m_optionFlag = flag; }
	bool IsSkipState();
	void UpdateGuideSide983(); // 983 00843ce0
	unsigned short GetBallInitSpecialValueFromPredictType(
		PredictType::Enum type);
	bool Predict(std::vector<WVector>& positions,
		std::vector<WVector>& boundPositions, int* pFirstBoundIndex);
	void DrawIndicator(const WVector& pos, unsigned long color, float scale);

	CGolfBall* m_pBall;
	std::vector<WVector> m_predictPos;
	std::vector<WVector> m_boundPos;
	UpdateSensor* m_pSensor; // 983 +0x44 (referencia)
	UpdateSensor* m_pSensorCur; // 983 +0x48 (lido a cada quadro)
	bool m_bSensorChanged; // 983 +0x58
	bool m_bSensorMoved; // 983 +0x234
	int m_guideSide; // 983 +0x5c (00843ce0): 0 centro, 1/2 lado do guia; mandado na msg 0x331
	AssistGuide m_assist; // 983 this+0x60
	float m_elapsed;
	int m_indicatorTex;
	int m_predictType;
	unsigned int m_optionFlag;
	int m_bEnable;
	C3dFont* m_pFont;
	std::vector<FrButton*> m_buttons;
	std::vector<PangCoin*> m_coins;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnprocessFunclets(); // HACK: EH funclets of the naked OnProcess (nakedify)
#endif
};
