#pragma once
#include <list>
#include <string>
class CNoticeBoard : public WSingleton<CNoticeBoard>
{
public:
	struct sNotice
	{
		std::string text;
		int count;
		float width;
		unsigned long color;
	};

	CNoticeBoard();
	virtual ~CNoticeBoard();

	void Process(float delta);
	void Display();
	void AddNotice(const char* text, int count, bool bImmediately);
	bool GetColornText(const char* src, std::string& text,
		unsigned long& color);

protected:
	WFont* m_pFont;
	WOverlay* m_pBoardLeft;
	WOverlay* m_pBoardMiddle;
	WOverlay* m_pBoardRight;
	std::list<sNotice*> m_noticeList;
	std::list<sNotice*>::iterator m_curNotice;
	float m_textPosX;
	float m_boardPosY;
};
