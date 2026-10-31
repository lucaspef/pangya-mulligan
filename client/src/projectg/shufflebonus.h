#pragma once

#include "singleton.h"

class WOverlay;

class CShuffleBonus : public WSingleton<CShuffleBonus>
{
public:
	enum eVaildType
	{
		VAILDTYPE_0,
		VAILDTYPE_1,
	};

	CShuffleBonus();
	virtual ~CShuffleBonus();

	void Render(WOverlay* pOverlay, float alpha);
	void ShowNoticeMsg();

private:
	bool _Validity(eVaildType type);
};
