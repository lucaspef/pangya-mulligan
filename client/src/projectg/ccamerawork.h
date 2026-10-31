#pragma once

// Partial reconstruction (only what realmyroommain uses so far).
class CCameraWork
{
public:
	static CCameraWork* m_pInstance;
	void Load();
	void SelectCamera(int id);
	void SetManualType(int type) { *(int*)((char*)this + VC10_CAMWORK_I2AC) = type; } // 983 +0x2e0 (645 0x2ac): m_i2ac, construtor 006d7fc2

	unsigned char m_pad0[0x20];
	WMatrix m_mat;
	float m_fov;
};
