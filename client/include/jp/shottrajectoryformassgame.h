// JP 983: ShotTrajectoryForMassGame (0064b1a0–0064f1a0) — rastro das tacadas dos outros jogadores nos modos de muita gente:
// cada tacada chega pela mensagem 0x312 (registro de 0x57 bytes), entra numa fila, é simulada de novo na bola global e vira
// uma linha que avança e some. Classe nova do 983 (existe no KR 839); layout medido no construtor 0064d830 (`new` de 0x410).
#pragma once

#include <deque>
#include <vector>
#include "actor.h"
#include "scenemanager.h"
#include "golfball.h"

class CFxSequence;
class WOverlay;

// 0046f540: fator de alcance (modo de jogo, opções, valor) — unidade vizinha, corpo não escrito aqui
float __fastcall RangeScale983(int mode, unsigned long flags, float value);

// métodos do 983 que faltam nas classes da frente A (pedido B → A da sessão B20)
class CClubJpShot
{
public:
	void SetShotOverride983(float a, float b); // 006ade10: bloco de 0x6c bytes em CClub+0x80 com dois valores cifrados
	void ClearShotOverride983(); // 006ac640: apaga o bloco
};

#pragma pack(push, 1)
// registro da tacada (param1 da mensagem 0x312); 0x57 bytes
struct sMassGameShot
{
	float ratio; // +0x00 fração da barra de força
	WVector vel; // +0x04
	unsigned char state; // +0x10
	WVector pos; // +0x11
	WVector wind; // +0x1d
	float initCurve; // +0x29
	float initSpin; // +0x2d
	unsigned long special; // +0x31 (0x10 tomahawk, 0x20 cobra, 0x40 spike)
	float spin; // +0x35
	float curve; // +0x39
	unsigned char groundType; // +0x3d
	unsigned char clubId; // +0x3e
	float clubPower; // +0x3f
	float clubRange; // +0x43
	float clubB; // +0x47 (2º argumento de 006ade10)
	float clubA; // +0x4b (1º argumento de 006ade10)
	float power; // +0x4f
	unsigned long frame; // +0x53
};
#pragma pack(pop)

class ShotTrajectoryForMassGame : public IActor, public CRenderFuncPtr
{
	friend IObject* ShotTrajectoryForMassGameMakeInstance();

public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	enum { MAX_TRACK = 12 };

	// 0x30 bytes
	struct sTrack
	{
		int cur; // +0x00 ponto até onde a linha já chegou
		float progress; // +0x04
		float frac; // +0x08
		int effectIndex; // +0x0c ponto do 1º quique (−1 = efeito já disparado)
		unsigned long color; // +0x10
		unsigned long special; // +0x14
		bool bActive; // +0x18
		float delay; // +0x1c
		std::vector<float> alpha; // +0x20
		sTrack() : cur(0), progress(0), frac(0), effectIndex(-1), color(0), special(0), bActive(false), delay(0) { } // 0064cd10
	};

	// 0x0c bytes; efeito do tomahawk/cobra/spike na ponta da linha
	struct sShotFx
	{
		bool bActive; // +0x00
		CFxSequence* pSeq; // +0x04
		int state; // +0x08 (0 início, 1 subindo, 2 descendo, 3 quique pedido, 4 fim)

		sShotFx() : bActive(false), pSeq(NULL), state(0) { } // 0064b250
		~sShotFx() { Close(); }
		void Close(); // 0064b710
		void Update(WVector from, WVector to); // 0064b750
	};

	ShotTrajectoryForMassGame(); // 0064d830
	virtual ~ShotTrajectoryForMassGame(); // 0064dad0

	virtual void OnLoad(); // 0064b2a0
	virtual void OnInit(); // 0064dd80
	virtual void OnReload(); // 0064b2b0
	virtual void OnProcess(float delta); // 0064ee70
	virtual void Display(); // 0064be90

protected:
	virtual void HandleMsg(const MsgObject& msg); // 0064d6e0

private:
	bool IsHidden(); // 0064bd00
	void LoadConfig(); // 0064cd80
	bool BuildTrajectory(const sMassGameShot* shot, std::vector<WVector>* positions, int* pFirstBoundIndex); // 0064dde0
	void PopShot(); // 0064ec10

	CGolfBall* m_pBall; // +0x1c cópia de guarda da bola global
	std::vector<WVector> m_pos[MAX_TRACK]; // +0x20
	sTrack m_track[MAX_TRACK]; // +0xe0
	sShotFx m_fx[MAX_TRACK]; // +0x320
	int m_pad983_3b0; // +0x3b0: só zerado (ctor 0064d830, OnInit 0064dd80); nenhum método do 983 lê
	std::deque<sMassGameShot> m_queue; // +0x3b4 (0x18 bytes)
	WOverlay* m_pPlayerPos; // +0x3cc
	float m_queueTime; // +0x3d0
	int m_hIndicator; // +0x3d4
	int m_bShow; // +0x3d8
	float m_speed; // +0x3dc
	int m_hLine; // +0x3e0
	float m_elapsed; // +0x3e4
	unsigned long m_color[4]; // +0x3e8 comum, tomahawk, cobra, spike
	float m_queueCheckTime; // +0x3f8
	float m_fadeOutSpeed; // +0x3fc
	float m_lineThick; // +0x400
	bool m_bShowEffect; // +0x404
	int m_posReduce; // +0x408
	bool m_bWaitMax; // +0x40c
};
