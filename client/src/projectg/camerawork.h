#pragma once

#include "singleton.h"
#include "wmath.h"
#include <vector>

class WPuppet;
class CGolfBall;
struct w_motion_data;

struct sCameraSelectionData
{
	int weight;
	bool use;
	bool flag;
};

extern sCameraSelectionData driveTable[];
extern sCameraSelectionData puttTable[];

class CCameraWork : public WSingleton<CCameraWork>
{
	// HACK: literal owners of camerawork.cpp (they reference private camera routines)
	friend class std::vector<unsigned long>;
	friend class std::vector<std::string>;
	friend class std::list<unsigned long>;
	friend class std::list<std::string>;
	friend class std::list<sItemInfo>;

public:
	enum eCamMode
	{
		CAMMODE_0
	};

	struct sInput
	{
		sInput();
	};

	CCameraWork();
	virtual ~CCameraWork();
#ifndef REBANG_SEMANTIC_ONLY
	CCameraWork(
		int); // HACK: see camerawork.cpp (emission order of the deleting destructor)
#endif

	const WVector& GetSourceCamera(); // 983 006cde90: lea eax, [ecx + 0x3c] (o 645 devolvia por valor)
	const WVector& GetDestCamera(); // 983 006cdea0: lea eax, [ecx + 0x60]
	void SetDestCamera(const WVector& dest);
	void UninstallCamera();
	void SetInstalledCamera(const WVector& pos);
	void StopBallSound();
	void Quake();
	void EarthQuake();

	bool CheckState();
	void Load();
	float LoadCamera(eCamMode mode, char* name, const WVector& pos);
	float LoadCameraMotion983(int mode, char* name); // 983 006d6be0
	void Process(float dt, float t);
	void InputProcess(float dt);
	void ReplayCamera(bool bNext);
	// 983 numbers the SetCamera modes from 204 one above ours (983 204 = this tee-back camera, 205 =
	// ToDefaultCamera, 206 = ZoomOutCamera, 207 = replay...); ours keeps the 645 numbers, so the 983 204
	// gets a number of its own (983 SetCamera 006da40c)
	enum { kTeeBackCamera983 = 0x1cc };
	void SelectCamera(int type);
	void SetCaddieCamera(const WMatrix& mat, const WVector& pos);
	bool ToggleCameraChecking()
	{
		m_bCameraChecking = !m_bCameraChecking;
		return m_bCameraChecking;
	}

protected:
	WPuppet* FindHoleInCam(unsigned long id);
	void InitVars();
	bool CheckGround(WVector& pos, WVector& dest);
	void ProcessBallSound();
	void ParsePuppetScript(char* script);
	void ScriptProcess(w_motion_data* motion, float from, float to);
	void MlandPreviewCamera983(); // 983 006d0880 (nome nosso): puppet do Mland (m_pPuppet983_240), do pino para o tee com Slerp
	void RankCamera983(); // 983 006ce360 (nome nosso): puppet de ranking (0x23c) e modo 5 do LoadCamera
	void SetCamera(int type, bool bForce);
	void CheckCameraType(bool bForce);
	void InitCtrlPoints(int type);

private:
	typedef void (CCameraWork::*CameraFn)(float, float);
	void CheckDriveCameraBallVisibilityAndSetBallVisibilitySafeCamera();
	void InterpolateBezier(WVector* out, float t, WVector* ctrl, int count);
	void PangyaACamera(float dt, float t);
	void ZoomOutCamera(float dt, float t);
	void FirstPersonCamera(float dt, float t);
	void StillFrameCamera(float dt, float t);
	void SwingReplayACamera(float dt, float t);
	void ExtraTimeCamera(float dt, float t);
	void ExtraTimeSlowCamera983(float dt, float t); // 983 006cecf0
	void DropCamera(float dt, float t);
	void ZoomInCamera(float dt, float t);
	void FixedAtSideCamera(float dt, float t);
	void PuttSecondACamera(float dt, float t);
	void SideCamera(float dt, float t);
	void FromColPointCamera(float dt, float t);
	void FixedCamera(float dt, float t);
	void PanCamera(float dt, float t);
	void CircleCamera(float dt, float t);
	void Chase_and_LookBackCamera(float dt, float t);
	void Drop_and_ZoomCamera(float dt, float t);
	void BezierCamera(float dt, float t);
	void FollowWithSwingCamera(float dt, float t);
	void SidePanWithZoomInCamera(float dt, float t);
	void FromMidPointCamera(float dt, float t);
	void FromRandomPointCamera(float dt, float t);
	void FollowCamera(float dt, float t);
	void LookBackCamera(float dt, float t);
	void Rail_or_SideCamera(float dt, float t);
	void FromHoleCupCamera(float dt, float t);
	void IntroBCamera(float dt, float t);
	void IntroCCamera(float dt, float t);
	void IntroDCamera(float dt, float t);
	void ToDefaultCamera(float dt, float t);
	void SwingReplayBCamera(float dt, float t);
	void IntroACamera(float dt, float t);
	void CobraACamera(float dt, float t);
	void FollowBallCamera(float dt, float t);
	void TopViewCamera(float dt, float t);
	void CobraBCamera(float dt, float t);
	void TomahawkACamera(float dt, float t);
	void BankFromHeadCamera(float dt, float t);
	void BallSideCamera(float dt, float t);
	void PuttSecondBCamera(float dt, float t);
	void CaddieChanceCamera(float dt, float t);
	void Chase_and_FollowCamera(float dt, float t);
	void JustLookingCamera(float dt, float t);
	void SpikeACamera(float dt, float t);
	void CaddyViewCamera(float dt, float t);

protected:
	// Layout reconstructed from the code (the PDB carries no type for this class).
	float m_f28;
	float m_f2c;
	float m_f30;
	bool m_b34;
	void* m_pfnCamera; // pointer to the active camera routine (member function)
	WVector m_srcCamera;
	WVector m_v48;
	WVector m_v54;
	WVector m_destCamera;
	int m_numDriveCam;
	int m_numPuttCam;
	int m_sumDriveWeight;
	int m_sumPuttWeight;
	std::vector<int> m_camCandidates;
	int m_i8c;
	int m_i90;
	int m_mode;
	unsigned int m_flags;
	float m_f9c;
	float m_fa0;
	float m_fa4;
	float m_fa8;
	float m_fac;
	int m_ib0;
	int m_ib4;
	int m_ib8;
	int m_ibc;
	WMatrix m_matA;
	WMatrix m_matB;
	WMatrix m_matSelect983; // 983 0x120: 0x30 novos (m_ctrl 645 0x120 -> 0x150); SelectCamera 006db82d copia g_camera (com m_matA/m_matB); 008b7da0 le +0x120..0x128
	WVector m_ctrl[4];
	char m_pad_150[0x180 - 0x150];
	WVector m_v180;
	float m_f18c;
	float m_f190;
	float m_f194;
	float m_f198;
	float m_f19c;
	bool m_b1a0;
	bool m_b1a1;
	bool m_b1a2;
	int m_i1a4;
	float m_f1a8;
	WMatrix m_matC;
	bool m_b1dc;
	bool m_bCameraChecking;
	bool m_b1de;
	bool m_b1df;
	WVector m_loadPos;
	struct sHoleCam
	{
		unsigned long id;
		WPuppet* pPuppet;
	} m_holeCam[4];
	WPuppet* m_pRankPuppet; // 983 0x23c (645 0x20c): LoadCamera 006d77d0 carrega aqui e copia para m_pPuppet
	WPuppet* m_pPuppet983_240; // 983: novo; LoadCamera caso 4 (00d1a558) e o destrutor 006d7a90 solta (s98; a s97 o pos em 0x23c)
	WPuppet* m_pPuppet;
	w_motion_data* m_pMotion;
	WVector m_ballStart;
	WQuat m_quatFrom;
	WQuat m_quatTo;
	WVector m_posFrom;
	WVector m_posTo;
	float m_f25c;
	float m_f260;
	float m_f264;
	float m_f268;
	bool m_b26c;
	bool m_b26d;
	char m_pad_26e[0x270 - 0x26e];
	std::vector<int> m_vecA;
	char m_pad_280[0x284 - 0x280];
	std::vector<int> m_vecB;
	int m_i294;
	int m_i298;

	bool m_b29c;
	char m_pad_29d[0x2a0 - 0x29d];
	float m_f2a0;
	int m_i2a4;
	WVector* m_pLoadPos;
	int m_i2ac;
	bool m_b2b0;
	bool m_b2b1;
	char m_pad_2b2[0x2b4 - 0x2b2];
	int m_loadCamMode983; // 983 +0x2e8: eCamMode do ultimo LoadCamera (3 = rot extra no movimento, 5 = memorial_cam: so +0x120) (006d781e; -1 se desconhecido); -1 no construtor 006d7fd3 (sizeof 0x2ec: push 0x2ec em 0068c0e9/008c0c08)
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCcamerawork2Funclets(); // HACK: EH funclets of the naked CCameraWork (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbProcessFunclets(); // HACK: EH funclets of the naked Process (nakedify)
#endif
};
