#pragma once
#include "../shared/titlemanager.h"

class Title
{
public:
	Title() { }
	virtual ~Title() { }
	virtual bool CanEquip() { return false; }
};

#define DECLARE_TITLE(num) \
	class Title##num : public Title \
	{ \
	public: \
		Title##num(); \
		virtual ~Title##num(); \
		virtual bool CanEquip(); \
	};

#define IMPLEMENT_TITLE(num) \
	Title##num::Title##num() \
	{ \
	} \
	Title##num::~Title##num() \
	{ \
	} \
	bool Title##num::CanEquip()

DECLARE_TITLE(21)
DECLARE_TITLE(22)
DECLARE_TITLE(23)
DECLARE_TITLE(24)
DECLARE_TITLE(25)
DECLARE_TITLE(26)
DECLARE_TITLE(27)
DECLARE_TITLE(28)
DECLARE_TITLE(50)
DECLARE_TITLE(51)
DECLARE_TITLE(52)
DECLARE_TITLE(53)
DECLARE_TITLE(69)
DECLARE_TITLE(70)
DECLARE_TITLE(71)
DECLARE_TITLE(72)
DECLARE_TITLE(73)
DECLARE_TITLE(74)
DECLARE_TITLE(380)
DECLARE_TITLE(381)
DECLARE_TITLE(123)
DECLARE_TITLE(124)
DECLARE_TITLE(125)
DECLARE_TITLE(126)
DECLARE_TITLE(357)
DECLARE_TITLE(358)
DECLARE_TITLE(359)
DECLARE_TITLE(360)
DECLARE_TITLE(361)

void RegisterTitles(TitleManager* titlemanager);
