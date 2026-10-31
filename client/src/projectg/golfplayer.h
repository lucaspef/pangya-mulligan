#pragma once

#include <list>
#include <string>
#include <vector>

#include "actor.h"
#include "extendedeffect.h"
#include "fxinterpreter.h"
#include "petscriptobject.h"

class CPetFrame;
class CTailEffect;
class CPartTidList;
class CFxSpray;
class CFxSequence;
class WBone;
class WPuppet;
struct w_motion_data;
struct w_bound_box;
struct sTexturePart;

// class instantiation order of the original (the allocator members of these two come first)
typedef char RbInstSprays[sizeof(std::allocator<CFxSpray*>)];
typedef char RbInstSequences[sizeof(std::allocator<CFxSequence*>)];

// 983: RTTI `CGolfPlayer : IActor, IPetScriptObject` (2a vtable 00d20540 em +0x18)
class CGolfPlayer : public IActor, public IPetScriptObject
{
	friend IObject* CGolfPlayerMakeInstance();

public:
	static const WRTTI m_RTTI;
	virtual ~CGolfPlayer() { }
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();

	struct sCondBBSpray
	{
		unsigned long id;
		CFxSpray* pSpray;
	};

	struct sCondBBSequence
	{
		unsigned long id;
		CFxSequence* pSequence;
	};

	struct sPetInfo
	{
		sPetInfo();
		~sPetInfo();

		float m_motionTime;       // 0x00
		w_motion_data* m_pMotion; // 0x04
		WMatrix m_matrix;         // 0x08
		bool m_bFlag0;            // 0x38
		char m_nextMotion[0x23];  // 0x39
		union
		{
			unsigned int m_flags; // 0x5c
			struct
			{
				unsigned int m_flagBits0 : 7;
				unsigned int m_bFlagBit7 : 1;
				unsigned int m_bFlagBit8 : 1;
			};
		};
		int m_val1;                  // 0x60
		float m_shotTimes[16];       // 0x64
		sTexturePart* m_pTexPart;    // 0xa4
		sTexturePart* m_pCurTexPart; // 0xa8
		float m_scale;               // 0xac
		float m_scaleRate;           // 0xb0
	};

	struct Object
	{
		Object();
	};

	struct sClub
	{
		sClub();

		WVector m_ballPos; // 0x00
		float m_gripPos;   // 0x0c
	};

	struct sTailSpot
	{
		WBone* m_pBone;
		void* m_pPoint;
	};

protected:
	CGolfPlayer();

	WMatrix GetHeadMatrix();
	WMatrix GetFootMatrix(int foot);
	void SearchGround();
	void SetLight();
	void SetLight(int index);
	void RegisterClub(int index);
	void CheckShotScript();
	void ParseScript_ShowVis(int index, bool bShow, const char* script);
	void PlayShotSfx(int index);
	bool Decide_PowerShot();
	void RefreshAllUccClothes();
	void SetPendingTexPart(int index, char* name);
	void ParseBoundingBoxScript(int index, bool b0, bool b1, bool b2);
	void CloseBoundingBoxFx(int index, unsigned long id);
	void ParsePuppetScript(char* script, int index, bool bSkip);
	void ScriptProcess(w_motion_data* pMotion, float from, float to, int index,
		bool bAll, bool bSkip);
	void ScoreEvent();
	void SetMotion(const char* name, float time, int index);
	void SetSwingType();
	void Shot_PowerShot();
	void Cancel_PowerShot();
	void SetPowerShotReady983(const char* motion, int level, bool bCombo); // 983 00743390
	void PowerShotInput();
	bool ShowAbilityParts(CPartTidList* pTids);
	void InputProcess(float delta);
	bool SearchGround(const WVector& pos, float angle);
	unsigned char HasStandGround(const WVector& pos);
	bool IsAnimFxRunning(int index, unsigned long id);
	void ResetComboState();
	void StartComboState();
	void EndComboState();

public:
	virtual void HandleMsg(const MsgObject& msg);

	// 983: IPetScriptObject (o slot 6, "*swing", nao e redefinido)
	virtual void OnScriptSnd(const char*& p);                     // 0073e0b0
	virtual void OnScriptShot(const char*& p);                    // 00740c50
	virtual void OnScriptTail(const char*& p);                    // 00741660
	virtual void OnScriptShowClub(const char*& p);                // 0073e160
	virtual void OnScriptHideClub(const char*& p);                // 0073e190
	virtual void OnScriptLockClub(const char*& p);                // 0073f420
	virtual void OnScriptFx(const char*& p, bool bSkip, int cmd); // 00741890
	virtual void OnScriptPTex(const char*& p);                    // 00742360
	virtual void OnScriptShowBone(const char*& p, int cmd);       // 0073d840
	virtual void OnScriptPlayIf(const char*& p);                  // 00742d00
	virtual void OnScriptOpa(const char*& p);                     // 0073d860
	virtual void OnScriptShowVis(const char*& p, int cmd);        // 0073da40

private:
	int GetAbilityPartsSize();
	void AddAbilityParts(unsigned long tid, unsigned long id);
	void UpdatePetMatrix983(); // 983 007410c0

protected:
	// offsets verified from the original binary (no PDB type available)
	// 983: sizeof 0x29c (push em 007417c0). Medido pelo construtor 00740ca0 e pelos votos (out/F3.5/s56/votos): de 0x38 a 0x84 tudo
	// -0xc4; de 0x90 em diante -0xbc. Os comentarios de deslocamento a direita, daqui para baixo, sao os do 645.
	// 983: 0x18 = vtable da base IPetScriptObject, 0x1c = IPetScriptObject::m_pScriptFrame
	FxInterpreter* m_pFxInterpreter983; // 983: 0x20 = new FxInterpreter(NULL, 0) em 00740dd5; substitui os 5 vetores de fx do 645
	int m_ringFxTid983[4]; // 983: 0x24..0x30, por jogador: tid do anel com efeito; *playbbfx o repassa em 0x335 (00741a1b); zerado no ctor e no 0x15a (sem gravador achado no 983)
	int m_valF8;                                // 0xf8
	WPuppet* m_pClubPuppet;                     // 0xfc
	Object m_obj100;                            // 0x100
	char m_pad101[3];                           // 0x101
	WMatrix m_matrix;                           // 0x104
	WVector m_pos;                              // 0x134
	float m_angle;                              // 0x140
	CPetFrame* m_pPetFrame;                     // 0x144
	sPetInfo* m_pPetInfo;                       // 0x148
	int m_scriptIndex983; // 983: 0x88, indice do boneco cujo script esta sendo lido (0073da6c); zerado no fim do construtor
	CPetFrame* m_pScriptFrame983; // 983: 0x8c = &m_pPetFrame[m_scriptIndex983] (0073da7f)
	float m_ballDist;                           // 0x14c
	w_bound_box* m_pTailSpots[2];               // 0x150
	CTailEffect* m_pTail;                       // 0x158
	bool m_bFlag0;                              // 0x15c
	bool m_bFlag15d;                            // 0x15d
	bool m_bFlag15e;                            // 0x15e
	char m_swingTypeName[0x10];                 // 0x15f
	char m_motionName[0x20];                    // 0x16f
	bool m_bSwingPending;                       // 0x18f
	bool m_bFlag190;                            // 0x190
	sClub m_clubs[4][4];                        // 0x194
	float m_footAngle;                          // 0x294
	WBone* m_pFootBones[2];                     // 0x298
	bool m_bFlag2a0;                            // 0x2a0
	LightSet m_lightSet;                        // 0x2a4
	WMatrix m_clubMatrix;                       // 0x2c0
	bool m_bFlag2f0;                            // 0x2f0
	float m_val2f4;                             // 0x2f4
	std::list<sTexturePart*> m_pendingTexParts; // 0x2f8
	bool m_bFlag304;                            // 0x304
	char m_pad305[0x30c - 0x305];               // 0x305
	float m_headAngle;                          // 0x30c
	WBone* m_pHeadBone;                         // 0x310
	bool m_bCombo;                              // 0x314
	float m_comboTime;                          // 0x318
	int m_comboCount;                           // 0x31c
	int m_val320;                               // 0x320
	unsigned long m_powerShotKeyTime;           // 0x324
	bool m_bPowerShot;                          // 0x328
	bool m_bFlag329;                            // 0x329
	int m_powerShotKeyState;                    // 0x32c
	bool m_bFlag330;                            // 0x330
	CExtendedEffect m_extEffect; // 983: 0x278 (o vetor m_abilityParts que vinha antes saiu daqui)
	char m_pad983_298[4]; // 983: 0x298..0x29c sem leitura/escrita (CExtendedEffect = 0x20 no 983: map +4..0x14, list +0x14..0x20, ctor 006e0270; os [reg+0x298] do exe sao de outras classes, out/B1553)
	// ---- 645: membros que o 983 NAO tem (la o trabalho de fx e do FxInterpreter em 0x20, e o m_abilityParts saiu). PROVISORIO: ficam
	// `static` (um conjunto para todos os jogadores, comportamento ERRADO com mais de um CGolfPlayer) so para a versao C das funcoes do
	// 645 ainda nao reescritas compilar (HandleMsg, ParseBoundingBoxScript, OnScriptFx, ...) sem aumentar o objeto: com isso o sizeof e
	// 0x29c como no 983. Somem quando essas funcoes forem reescritas pelo 983.
	static std::vector<sCondBBSpray> m_condSpray[4];   // 645: 0x18
	static std::vector<sCondBBSequence> m_condSeq[4];  // 645: 0x58
	static std::vector<unsigned long> m_condIds[4];    // 645: 0x98
	static std::vector<CFxSpray*> m_sprays;            // 645: 0xd8
	static std::vector<CFxSequence*> m_sequences;      // 645: 0xe8
	static std::vector<std::pair<unsigned long, unsigned long> >
		m_abilityParts; // 645: 0x334
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbParsepuppetscripFunclets(); // HACK: EH funclets of the naked ParsePuppetScript (nakedify)
#endif
};
