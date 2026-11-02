#pragma once

#include <list>
#include <string>
#include "singleton.h"

class WFont;
class WOverlay;
class TiXmlDocument;
class CGDEventNotice;

class CWinnerNotice : public WSingleton<CWinnerNotice>
{
public:
	enum EventTimeFlag
	{
		EVENT_DATE,
		EVENT_TIME
	};

	struct sNoticeMsg
	{
		unsigned long color;
		int style;
		std::string text;

		void initdata()
		{
			color = -1;
			style = 0;
			text = "";
		}
	};

	struct sWinnerNotice
	{
		sNoticeMsg msg[10];
		std::string str0;
		std::string str1;
		std::string str2;
		std::string str3;
		int repeat;
		float width;
		unsigned long time;
		int reserved0;
		int reserved1;
		int reserved2;
		int reserved3;
		int count;

		void clear_data()
		{
			memset(msg, 0, sizeof(msg));

			for (int i = 0; i < 10; i++)
				msg[i].initdata();
			str0 = "";
			str1 = "";
			str2 = "";
			width = 0;
			time = 0;
			reserved0 = 0;
			reserved1 = 0;
			reserved2 = 0;
			reserved3 = 0;
			count = 0;
			repeat = 1;
		}
	};

	CWinnerNotice();
	virtual ~CWinnerNotice();

	void Process(float delta);
	void Display();
	bool GetColornText(const char* text, std::string& out,
		unsigned long& color);
	float GetTextTailPoint(int count);
	void AddWinnerNotice(int repeat);
	void SetWinnerData(int type, std::string xml);

protected:
	void WinnerNotice_Init();
	void WinnerNotice_Clear();
	void ParseXmlData(TiXmlDocument& doc, const char* xml);
	void GetEventNumfromXml(TiXmlDocument& doc, std::string& out, bool bNext);
	void GetDaynTimefromXml(TiXmlDocument& doc, std::string& out,
		EventTimeFlag flag, bool bNext);
	const char* GetItemNamefromXml(TiXmlDocument& doc, int kind = 0);
	void GetItemCountfromXml(TiXmlDocument& doc, std::string& out,
		bool bParen = true);
	void GetRunningTimefromXml(TiXmlDocument& doc, std::string& out);
	void GetMessagefromXml(TiXmlDocument& doc, std::string& out);
	void GetUserNamefromXml(TiXmlDocument& doc, std::string& out);
	void NickNameEdit(char* nick, bool bCut, std::string& out);

	WFont* m_pFont;
	WOverlay* m_pLeft;
	WOverlay* m_pMid;
	WOverlay* m_pRight;
	std::list<sWinnerNotice*> m_list;
	std::list<sWinnerNotice*>::iterator m_it;
	float m_x;
	float m_y;
	CGDEventNotice* m_pData;
	sWinnerNotice m_notice;
	int m_reserved;
};
