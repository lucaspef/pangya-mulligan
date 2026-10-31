#pragma once

class CRing
{
public:
	CRing();
	CRing(const char* texture);
	~CRing();

	void Init();
	void Process(float dt);
	void Display();
	void SetCenter(const WVector& center);
	void SetActive(bool bActive) { m_bActive = bActive; }

protected:
	unsigned long m_reserved;
	int m_num;
	WVector* m_pos;
	WVector* m_pos2;
	WVector m_center;
	float m_radius;
	int m_texHandle;
	float m_angle;
	float m_height;
	float m_scale;
	bool m_bActive;
};
