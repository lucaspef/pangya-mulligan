#pragma once

class SpecialPrizeItem
{
public:
	SpecialPrizeItem() { InitalizeItem(); }
	virtual ~SpecialPrizeItem() { }
	bool getflag() { return m_flag; }
	void changeflag() { m_flag = !m_flag; }

	float getvalue() { return m_value; }
	void setvalue(float value) { m_value = value; }
	void InitalizeItem()
	{
		m_flag = false;
		m_value = 1.0f;
	}

private:
	bool m_flag;
	float m_value;
};
