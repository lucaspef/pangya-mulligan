#pragma once

#include "singleton.h"

class WOverlay;
class WFont;
struct sItemInfo;

class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	CHelpMsg();
	virtual ~CHelpMsg();

	void Process(float dt);
	void Display();
	void SetHelpMsg(int id, char* extra);
	void SetEmoticonHelpMsg(char** lines);
	void SetItemHelpMsg(unsigned long itemId);
	void OnChangeTurn();
	void CheckReplayAvailable();
	void ResetUiPosition();

private:
	float CalcBaseY();
	void CalcWindowSize(char* title, char* body);
	int GetLine(int* start, int* end, unsigned long* color, char* text,
		bool trim);

	WOverlay* m_pOverlay;
	char* (*m_table)[3];
	int m_curId;
	WFont* m_pFont;
	WRect m_src[3];
	WRect m_dst[3];
	unsigned long m_itemId;
	char m_title[32];
	char m_body[256];
	char m_sub[16];
	char m_extra[64];
	float m_idleTime;
	bool m_bItem;
	sItemInfo* m_pReplayItem;
};
