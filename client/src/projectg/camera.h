#pragma once

#include "actor.h"

class CCamera : public IActor
{
public:
	virtual ~CCamera() { }

protected:
	DECLARE_OBJECT(CCamera)

public:
	virtual void OnInit();
	virtual void OnProcess(float delta);

protected:
	virtual void HandleMsg(const MsgObject& msg);

	CCamera();

	struct sSpotCam
	{
		sSpotCam() { }

		WVector pos;
		float angle;
		WVector ground;
		float pitch;
		WVector dir;
		float rate;
		float time;
		bool bFollow;
	};

	void LookAt(WVector& eye, WVector& at);
	float GetChangeRate();
	float GetHeightRate(float height);
	void GetRotateAcc(float dt);
	void GetMoveAcc(float dt);
	void GetHorizonAcc(float dt);
	float GetCameraAngle();
	void YawFixedCamera(float amount);
	void PitchFixedCamera(float amount);
	void SetSpotCamera(bool bSpot);
	void ProcessMousePick(float dt);
	void ProcessTutorGuidePick(float dt); // 983 00714a10
#ifndef REBANG_SEMANTIC_ONLY
	// HACK: unwind funclets of the naked ProcessMousePick (see camera.cpp)
	static void __cdecl RbProcessMousePickFunclets();
#endif
	void ProcessToSpot(float dt);
	void Translate(float dt);
	void UpDown(float dt);
	void RotateEnvironment(float angle);
	void RotateForGallery(float angle);
	void Rotate(float dt);
	void FixedCamera(float dt);
	void ProcessInput(float dt);
	void ProcessForcedMove(float dt);

	int m_rotDir;
	float m_rotTotal;
	float m_rotRemain;
	int m_moveDir;
	int m_horizonDir;
	float m_rotAcc;
	float m_moveAcc;
	float m_horizonAcc;
	WVector m_eye;
	WVector m_target;
	float m_rate;
	float m_curve;
	float m_maxHeight;
	float m_groundHeight;
	bool m_bFixed;
	float m_homeRate;
	float m_baseRate;
	bool m_bEnable;
	float m_timer;
	bool m_bFreeLook;
	float m_yaw;
	float m_pitch;
	float m_idleTime;
	bool m_bSendAngle;
	float m_angleStart;
	float m_angleEnd;
	sSpotCam m_spot;
};
