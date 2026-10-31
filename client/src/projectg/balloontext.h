#pragma once

#include <string>
#include <list>
#include "singleton.h"

class WView;
class WFont;
class WOverlay;

class ntBalloonText
{
public:
	ntBalloonText(unsigned long guid, const char* text,
		const float* const color, float lifeTime);
	~ntBalloonText();

	void SetText(const char* text);
	void Render(WView* view);
	bool Process(float dt);
	bool IsVisible() const { return m_elapsed < m_lifeTime ? true : false; }
	void SetTransparency(float alpha) { m_color[3] = alpha; }

private:
	const char* StripSpecialText(const char* text) const;

	unsigned long m_guid;
	std::string m_text;
	bool m_bNewText;
	int m_x;
	bool m_bThink;
	float m_color[4];
	float m_lifeTime;
	float m_elapsed;
};

class ntBalloonTextManager : public WSingleton<ntBalloonTextManager>
{
public:
	using WSingleton<ntBalloonTextManager>::m_pInstance;

	ntBalloonTextManager();
	virtual ~ntBalloonTextManager();

	void Init();
	void Shutdown();
	void DeleteAll();
	void Process(float dt);
	void Render();
	ntBalloonText* CreateBalloonText(unsigned long guid, const char* text,
		const float* const color, float lifeTime);
	void DeleteBalloonText(ntBalloonText* balloon);

	std::list<ntBalloonText*> m_list;
	WFont* m_pFont;
	WOverlay* m_pFrame[9];
	WOverlay* m_pTail;
	WOverlay* m_pThinkFrame;
	WOverlay* m_pThinkTail;
	int m_frameLeft;
	int m_frameTop;
	int m_frameRight;
	int m_frameBottom;
};
