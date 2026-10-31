#pragma once
#include "vc10_layout.h"

#include <list>
#include "fx.h"
#include "fontinfo.h"
#include "../shared/classdefine.h"

// 983: classe nova (RTTI CGroundMaxPowerInfo : CFontInfo, vtable 00d08b38, construtor 00527220, 0x2c bytes).
// Sem membros próprios: os 0x2c são os do CFontInfo do 983.
class CGroundMaxPowerInfo : public CFontInfo
{
public:
	CGroundMaxPowerInfo();
	virtual ~CGroundMaxPowerInfo();

	// 983 00539d00: "NN%" em volta do anel do CBase (2 vezes, a 180 graus), letra a letra em 3D (nome meu)
	void PrintOnRing(int percent, float radius, float height, float angle, const WMatrix& mat);
};
C_ASSERT_VC10(sizeof(CGroundMaxPowerInfo) == 0x2c);

class CBase
{
public:
	struct sBall
	{
		IFF_STRUCT::sBall* pBall;
		WPuppet* pPuppet;

		sBall()
			: pBall(NULL), pPuppet(NULL)
		{
		}
	};

	struct sFindBall
	{
		unsigned long tid;

		sFindBall(unsigned long _tid)
			: tid(_tid)
		{
		}
		bool operator()(sBall* ball) { return ball->pBall->c.TypeId == tid; }
	};

	CBase();
	~CBase();

	void Load();
	void ResetUiPosition();
	void Process(float dt);
	void Display();
	void SetNormal(const WVector& normal);
	void UpdateGroundPenalty983(); // 983: 006e34b0 (mensagem 0x35e do CScreen)
	WMatrix GetBaseMatrix();
	sBall* ChangeBall(unsigned long tid);
	void ChangeTurn();
#ifdef REBANG_GUI_CONTROL_CPP
	void SetAngle(float angle) { m_rot = angle; }
#endif

protected:
	void DrawRing();
	sBall* FindBall(unsigned long tid);
	void AttachFx();
	sBall* AddBall(unsigned long tid);

	std::list<sBall*> m_balls;
	sBall* m_pCurBall;
	float m_uiX;
	float m_uiY;
	unsigned char m_unused18[4];
	WVector m_normal;
	WMatrix m_camera;
	LightSet m_light;
	float m_rot;
	int m_texLine;
	CGroundMaxPowerInfo m_maxPowerInfo; // 983: 0x7c; nome meu
	int m_segments;
	int m_penalty;
	WVector2D m_spin;
	WMatrix m_matrix;
	float m_angle;
	CPvsFxBox m_fx;
	WTVertex* m_vl[5];
	WTVertex m_vtx[4];
};
