#pragma once
#include "singleton.h"

class COneLineBoard : public WSingleton<COneLineBoard>
{
public:
	COneLineBoard();
	virtual ~COneLineBoard();

	void AddOnelineMsg(const std::string& nick, const std::string& msg);
	void SetRect(const WRect& rect) { m_rect = rect; }
	void SetMessage(const char* msg) { m_message = msg; }
	std::string& GetMessageA() { return m_message; }
	void SetWaitMsgNum(int num) { m_waitMsgNum = num; }
	void SetWaitMsgTime(int time) { m_waitMsgTime = time; }
	int GetWaitMsgNum() { return m_waitMsgNum; }
	int GetWaitMsgTime() { return m_waitMsgTime; }
	void LoadBg();
	void ShowBg(bool bShow) { m_bShowBg = bShow; }
	void FadeOut();
	void FadeIn();
	float GetFadeFactor();

	void Clear();
	void Process(float delta);
	void Display();

	struct sOnelineMsg
	{
		std::string msg;
		float width;
		float x;
		unsigned long time;
	};

protected:
	void DrawBg();

	bool m_bShowBg;
	int m_fadeMode;
	float m_fadeFactor;
	bool m_bAnimate;
	std::list<sOnelineMsg*>::iterator m_it;
	std::list<sOnelineMsg*>::iterator m_unusedIt;
	std::list<sOnelineMsg*> m_msgList;
	WRect m_rect;
	std::string m_message;
	int m_waitMsgNum;
	int m_waitMsgTime;
	unsigned long m_reserved;
	const Bitmap* m_pBgBitmap;
};
