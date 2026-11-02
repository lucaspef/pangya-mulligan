#include "minatl.h"
#include "winnernotice.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "wfont.h"
#include "woverlay.h"
#include "wmath.h"
#include "chatmsg.h"
#include "onelineboard.h"
#include "capturedbg.h"
#include "golftask.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "gdeventnotice.h"
#include "gamedatadb.h"
#include "tinyxml.h"
#include "binstr.h"
#include "fremoticon.h"
#include "frgraphicinterface.h"

extern Fresh* g_pFresh;

CWinnerNotice::CWinnerNotice()
{
	WinnerNotice_Init();
}

CWinnerNotice::~CWinnerNotice()
{
	WinnerNotice_Clear();
}

void CWinnerNotice::WinnerNotice_Init()
{
	m_pLeft = g_resrcmng->GetOverlay("notice_l.tga", 0);
	m_pMid = g_resrcmng->GetOverlay("notice_m.tga", 0);
	m_pRight = g_resrcmng->GetOverlay("notice_r.tga", 0);

	m_pFont = CChatMsg::Instance()->GetMaskedFont();
	m_it = m_list.end();
	m_x = -5555.0f;
	m_y = -50.0f;
	m_pData = NULL;
	m_reserved = 0;
}

void CWinnerNotice::WinnerNotice_Clear()
{
	m_it = m_list.end();
	sequence_delete(m_list.begin(), m_list.end());

	if (g_resrcmng && m_pLeft)
	{
		g_resrcmng->Release(m_pLeft);
		m_pLeft = NULL;
	}
	if (g_resrcmng && m_pMid)
	{
		g_resrcmng->Release(m_pMid);
		m_pMid = NULL;
	}
	if (g_resrcmng && m_pRight)
	{
		g_resrcmng->Release(m_pRight);
		m_pRight = NULL;
	}
}

void CWinnerNotice::Process(float delta)
{
	delta /= CProjectG::Instance()->GetGameSpeed();

	if (m_it == m_list.end())
	{
		m_it = m_list.begin();
		if (m_it == m_list.end())
		{
			if (m_y > -50.0f)
			{
				m_y -= delta * 30.0f;

				if (m_y < -50.0f)
					m_y = -50.0f;
			}

			if (IS_KINDOF(CGolfTask, AfxGetTask()))
				COneLineBoard::Instance()->FadeIn();

			return;
		}
	}

	if (CCapturedBg::Instance() && CCapturedBg::Instance()->IsCaptured())
	{
		FrEmoticon* pEmoticon = g_pFresh->GetManager()->GetEmoticon();
		if (pEmoticon)
			pEmoticon->SetAnim(false);
		return;
	}

	if (m_y < -0.1f)
	{
		m_y += delta * 30.0f;

		if (!(m_y > 0.0f))
			return;
	}

	m_y = 0.0f;

	sWinnerNotice* pNotice = *m_it;

	if (m_x < -5000.0f)
		m_x = g_view->GetWidth() * 0.5f + 150.0f;

	m_x -= delta * 30.0f;

	if (m_x + pNotice->width < g_view->GetWidth() * 0.5f - 150.0f)
	{
		m_x = g_view->GetWidth() * 0.5f + 150.0f;

		if (--pNotice->repeat < 1)
		{
			delete *m_it;
			m_it = m_list.erase(m_it);
		}
		else
		{
			++m_it;
		}
	}
}

void CWinnerNotice::Display()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (!m_pLeft || !m_pMid || !m_pRight)
		return;

	float alpha = (m_y + 50.0f) * 0.02f;

	if (Wabs(alpha) < g_EPSILON)
		return;

	WRect src(0.0f, 0.0f, 1.0f, 1.0f);
	WRect dst(g_view->GetWidth() * 0.5f - 150.0f - 20.0f, m_y,
		m_pLeft->GetWidth(), m_pLeft->GetHeight());
	m_pLeft->Render(g_view, src, dst, 0, 0xa0ffffff, 0.0f, 0);

	dst.x += dst.w;
	dst.w = 300.0f - m_pRight->GetWidth() + 20.0f;
	dst.h = (float)m_pMid->GetHeight();
	m_pMid->Render(g_view, src, dst, 0, 0xa0ffffff, 0.0f, 0);

	dst.x += dst.w;
	dst.w = (float)m_pRight->GetWidth();
	dst.h = (float)m_pRight->GetHeight();
	m_pRight->Render(g_view, src, dst, 0, 0xa0ffffff, 0.0f, 0);

	if (IS_EXACTKINDOF(CGolfTask, AfxGetTask()))
	{
		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(-1, 0);
	}
	else
	{
		pGDI->SetTextStyle(0);
	}

	if (m_it != m_list.end())
	{
		sWinnerNotice* pNotice = *m_it;
		FrEmoticon* pEmoticon = g_pFresh->GetManager()->GetEmoticon();

		WRect clip(g_view->GetWidth() * 0.5f - 150.0f, 0.0f, 300.0f, 40.0f);
		pEmoticon->SetClippingArea(&clip);
		if (pEmoticon)
			pEmoticon->SetAnim(true);
		pEmoticon->SetAnimTime(pNotice->time);

		for (int i = 0; i < pNotice->count; i++)
		{
			pGDI->SetTextColor(-1, pNotice->msg[i].color);
			pGDI->SetTextStyle(pNotice->msg[i].style);
			int x = (int)m_x;
			pEmoticon->PrintText(WPoint(x + GetTextTailPoint(i), 7.0f), 0,
				pNotice->msg[i].text.c_str(), -1);
		}

		pEmoticon->SetAnim(false);
		pEmoticon->SetClippingArea(NULL);
	}

	pGDI->SetTextColor(-1, -1);
	pGDI->SetTextStyle(0);
}

bool CWinnerNotice::GetColornText(const char* text, std::string& out,
	unsigned long& color)
{
	color = -1;

	cTokenV token;
	if (token.Init(text, strlen(text)))
	{
		char* pText;
		bool bRet = token.GetTokenFullSep(&pText, "\\c", 2);

		if ((pText && *pText) || bRet)
		{
			if (bRet)
			{
				char* pTok;
				token.GetTokenFullSep(&pTok, "\\c", 2);

				unsigned long value = -1;
				cTokenV colorToken;
				bool bLoop = colorToken.Init(pTok, strlen(pTok));

				while (bLoop)
				{
					bLoop = colorToken.GetToken(&pTok, ",", 1);

					if (pTok && *pTok)
					{
						if (strlen(pTok) > 1)
							sscanf(pTok, "%x", &value);
					}
				}

				color = value;

				std::string str(text);
				out = str.substr(str.rfind("\\c") + 2);
			}
			else
			{
				out = text;
			}

			return true;
		}
	}

	return false;
}

float CWinnerNotice::GetTextTailPoint(int count)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	sWinnerNotice* pNotice = *m_it;

	float width = 0;
	for (int i = 0; i < count; i++)
		width += pGDI->GetTextExtendOld983(pNotice->msg[i].text.c_str());

	return width;
}

void CWinnerNotice::AddWinnerNotice(int repeat)
{
	sWinnerNotice* pNotice = new sWinnerNotice;
	pNotice->clear_data();

	pNotice->count = m_notice.count;
	pNotice->repeat = repeat;

	for (int i = 0; i < m_notice.count; i++)
	{
		pNotice->msg[i].color = m_notice.msg[i].color;
		pNotice->msg[i].style = m_notice.msg[i].style;
		pNotice->msg[i].text = m_notice.msg[i].text;
		pNotice->width +=
			m_pFont->GetTextWidthOld983(g_view, m_notice.msg[i].text.c_str());
	}

	pNotice->time = timeGetTime();

	int size = m_list.size();
	m_list.push_back(pNotice);

	if ((!CCapturedBg::Instance() || !CCapturedBg::Instance()->IsCaptured()) &&
		size == 0 && m_list.size() == 1 && m_y < -0.1f)
		g_audio->PlaySfx("goldentime_notice", NULL, 0, NULL, NULL, 0.5f,
			200.0f);
}

void CWinnerNotice::SetWinnerData(int type, std::string xml)
{
	if (xml.empty()) // 983 008ac130
		return;

	if (m_pData == NULL)
		m_pData = (CGDEventNotice*)CGameDataDB::Instance()->GetGameData(
			GAMEDATA_EVENTNOTICE, "WinnerEventNotice");

	TiXmlDocument doc;
	ParseXmlData(doc, xml.c_str());
	m_notice.clear_data();

	sNoticeMsg* msg = m_notice.msg;
	if (doc.FirstChildElement("PARAMS") == NULL)
	{
		// 983: sem PARAMS o proprio texto vira o aviso
		msg[0].text = xml;
		m_notice.count = 1;
		AddWinnerNotice(1);
		return;
	}

	int n = 0;
	switch (type)
	{
	case 1:
		msg[0].color = 0xff00ffff;
		msg[0].text = GetItemNamefromXml(doc, 1);
		{
			sNoticeMsg& m = msg[1];
			GetColornText(m_pData->GetText(0x18), m.text, m.color);
		}
		msg[2].color = 0xff00ffff;
		msg[2].text = GetItemNamefromXml(doc);
		{
			sNoticeMsg& m = msg[3];
			GetColornText(m_pData->GetText(0x19), m.text, m.color);
		}
		n = 4;
		break;
	case 2:
		msg[0].color = 0xff00ffff;
		msg[0].text = GetItemNamefromXml(doc, 1);
		{
			sNoticeMsg& m = msg[1];
			GetColornText(m_pData->GetText(0x1b), m.text, m.color);
		}
		msg[2].color = 0xff00ffff;
		GetItemCountfromXml(doc, msg[2].text, false);
		{
			sNoticeMsg& m = msg[3];
			GetColornText(m_pData->GetText(0x1c), m.text, m.color);
		}
		n = 4;
		break;
	case 11:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(1), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(2), m.text, m.color);
		}
		msg[3].color = 0xffff0000;
		msg[3].style = 2;
		msg[3].text = GetItemNamefromXml(doc);
		msg[4].color = 0xffff0000;
		msg[4].style = 2;
		GetItemCountfromXml(doc, msg[4].text);
		{
			sNoticeMsg& m = msg[5];
			GetColornText(m_pData->GetText(3), m.text, m.color);
		}
		n = 6;
		break;
	case 12:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(4), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(5), m.text, m.color);
		}
		msg[3].color = 0xffff0000;
		msg[3].style = 2;
		msg[3].text = GetItemNamefromXml(doc);
		msg[4].color = 0xffff0000;
		msg[4].style = 2;
		GetItemCountfromXml(doc, msg[4].text);
		{
			sNoticeMsg& m = msg[5];
			GetColornText(m_pData->GetText(6), m.text, m.color);
		}
		n = 6;
		break;
	case 13:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(7), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(8), m.text, m.color);
		}
		n = 3;
		break;
	case 14:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(9), m.text, m.color);
	}
		{
			sNoticeMsg& m = msg[1];
			GetColornText(m_pData->GetText(10), m.text, m.color);
		}
		msg[2].color = 0xff050a9b;
		msg[2].style = 2;
		GetDaynTimefromXml(doc, msg[2].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[3];
			GetColornText(m_pData->GetText(11), m.text, m.color);
		}
		msg[4].color = 0xffff0000;
		msg[4].style = 2;
		msg[4].text = GetItemNamefromXml(doc);
		msg[5].color = 0xffff0000;
		msg[5].style = 2;
		GetItemCountfromXml(doc, msg[5].text);
		{
			sNoticeMsg& m = msg[6];
			GetColornText(m_pData->GetText(12), m.text, m.color);
		}
		msg[7].color = 0xffff9bcd;
		msg[7].style = 2;
		GetUserNamefromXml(doc, msg[7].text);
		{
			sNoticeMsg& m = msg[8];
			GetColornText(m_pData->GetText(13), m.text, m.color);
		}
		n = 9;
		break;
	case 15:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(14), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(15), m.text, m.color);
		}
		msg[3].color = 0xff050a9b;
		msg[3].style = 2;
		GetDaynTimefromXml(doc, msg[3].text, EVENT_TIME, true);
		{
			sNoticeMsg& m = msg[4];
			GetColornText(m_pData->GetText(16), m.text, m.color);
		}
		n = 5;
		break;
	case 16:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(17), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_DATE, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(18), m.text, m.color);
		}
		msg[3].color = 0xff050a9b;
		msg[3].style = 2;
		GetDaynTimefromXml(doc, msg[3].text, EVENT_DATE, true);
		msg[4].color = 0xff050a9b;
		msg[4].style = 2;
		GetDaynTimefromXml(doc, msg[4].text, EVENT_TIME, true);
		{
			sNoticeMsg& m = msg[5];
			GetColornText(m_pData->GetText(19), m.text, m.color);
		}
		n = 6;
		break;
	case 17:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(20), m.text, m.color);
	}
		n = 1;
		break;
	case 18:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(21), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetDaynTimefromXml(doc, msg[1].text, EVENT_TIME, false);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(22), m.text, m.color);
		}
		n = 3;
		break;
	case 20:
		msg[0].text = xml;
		n = 1;
		break;
	case 21:
	{
		sNoticeMsg& m = msg[0];
		GetColornText(m_pData->GetText(0x1d), m.text, m.color);
	}
		msg[1].color = 0xff050a9b;
		msg[1].style = 2;
		GetRunningTimefromXml(doc, msg[1].text);
		{
			sNoticeMsg& m = msg[2];
			GetColornText(m_pData->GetText(0x1e), m.text, m.color);
		}
		n = 3;
		break;
	case 10000:
		GetMessagefromXml(doc, msg[0].text);
		n = 1;
		break;
	default:
		return;
	}

	// 983: com todos os textos vazios nao entra na fila
	int empty = 0;
	for (int i = 0; i < n; i++)
	{
		if (msg[i].text.empty())
			empty++;
	}
	if (empty < n)
	{
		m_notice.count = n + 1;
		AddWinnerNotice(1);
	}
}

void CWinnerNotice::ParseXmlData(TiXmlDocument& doc, const char* xml)
{
	doc.Clear();
	doc.Parse(xml);
}

void CWinnerNotice::GetEventNumfromXml(TiXmlDocument& doc, std::string& out,
	bool bNext)
{
	char buf[50] = "";

	TiXmlElement* pElem;
	if (bNext)
		pElem = doc.FirstChildElement("PARAMS")->FirstChildElement("NEXTTIMES");
	else
		pElem = doc.FirstChildElement("PARAMS")->FirstChildElement("TIMES");

	const char* text = NULL;
	if (pElem)
		text = pElem->FirstChild()->Value();

	sprintf(buf, "%s \xc2\xf7\xbc\xf6 \xc0\xcc\xba\xa5\xc6\xae",
		text ? text : "0");
	out = buf;
}

void CWinnerNotice::GetDaynTimefromXml(TiXmlDocument& doc, std::string& out,
	EventTimeFlag flag, bool bNext)
{
	char buf[100] = "";

	TiXmlElement* pElem;
	if (bNext)
		pElem =
			doc.FirstChildElement("PARAMS")->FirstChildElement("EVENTNEXTTIME");
	else
		pElem = doc.FirstChildElement("PARAMS")->FirstChildElement(
			"EVENTSTARTTIME");
	if (!pElem)
	{
		out = "0";
		return;
	}

	int year, month, day, hour, minute, second;
	sscanf(pElem->FirstChild()->Value(), "%d-%d-%d %d:%d:%d.%d", &year, &month,
		&day, &hour, &minute, &second, &second);

	switch (flag)
	{
	case EVENT_DATE:
		sprintf(buf, K2L_Compatibility("%d \xbf\xf9 %d \xc0\xcf"), month, day);
		break;
	case EVENT_TIME:
		sprintf(buf, K2L_Compatibility("%02d\xbd\xc3%02d\xba\xd0"), hour, minute);
		break;
	}

	out = buf;
}

const char* CWinnerNotice::GetItemNamefromXml(TiXmlDocument& doc, int kind)
{
	const char* name; // 983 008aa600
	if (kind == 0)
		name = "TYPEID";
	else if (kind == 1)
		name = "BOX_TYPEID";
	else
		return "";

	TiXmlElement* pElem =
		doc.FirstChildElement("PARAMS")->FirstChildElement(name);
	if (!pElem)
		return "";

	unsigned long typeId = atoi(pElem->FirstChild()->Value());
	return ItemManager()->GetItemName(typeId);
}

void CWinnerNotice::GetItemCountfromXml(TiXmlDocument& doc, std::string& out,
	bool bParen)
{
	TiXmlElement* pQty =
		doc.FirstChildElement("PARAMS")->FirstChildElement("QTY");
	TiXmlElement* pType =
		doc.FirstChildElement("PARAMS")->FirstChildElement("TYPEID");

	if (pType == NULL)
	{
		out = "";
	}
	else if (pQty == NULL)
	{
		out = "";
	}
	else
	{
		char buf[100] = "";
		if (atoi(pType->FirstChild()->Value()) == 0x1a000010)
		{
			// 983 008ab4b0
			sprintf(buf, K2L_Compatibility(bParen ? "(%d \xc6\xce)" : "%d \xc6\xce"),
				atoi(pQty->FirstChild()->Value()));
			out = buf;
		}
		else if (atoi(pQty->FirstChild()->Value()) > 1)
		{
			sprintf(buf, bParen ? "(%d)" : "%d", atoi(pQty->FirstChild()->Value()));
			out = buf;
		}
		else
		{
			out = "";
		}
	}
}

void CWinnerNotice::GetRunningTimefromXml(TiXmlDocument& doc, std::string& out)
{
	// 983 008aba40: minutos
	TiXmlElement* pElem =
		doc.FirstChildElement("PARAMS")->FirstChildElement("RunningTime");
	if (!pElem)
	{
		out = "";
		return;
	}

	int total = atoi(pElem->FirstChild()->Value());
	int hour = 0;
	int minute = 0;
	if (total > 0)
	{
		minute = total;
		if (total > 59)
		{
			hour = total / 60;
			minute = total % 60;
		}
	}

	char buf[100] = "";
	if (minute == 0)
		sprintf(buf, K2L_Compatibility("%d\xbd\xc3\xb0\xa3"), hour);
	else if (hour < 1)
		sprintf(buf, K2L_Compatibility("%d\xba\xd0"), minute);
	else
		sprintf(buf, K2L_Compatibility("%d\xbd\xc3\xb0\xa3 %d\xba\xd0"), hour, minute);
	out = buf;
}

void CWinnerNotice::GetMessagefromXml(TiXmlDocument& doc, std::string& out)
{
	// 983 008ab680
	TiXmlElement* pElem =
		doc.FirstChildElement("PARAMS")->FirstChildElement("MESSAGE");
	out = pElem ? pElem->FirstChild()->Value() : "";
}

void CWinnerNotice::GetUserNamefromXml(TiXmlDocument& doc, std::string& out)
{
	std::string nick;

	TiXmlElement* pElem =
		doc.FirstChildElement("PARAMS")->FirstChildElement("NICKNAME");
	if (!pElem)
	{
		out = "";
		return;
	}

	for (; pElem; pElem = pElem->NextSiblingElement())
	{
		NickNameEdit((char*)pElem->FirstChild()->Value(), true, nick);
		out += "(" + nick + ")";
	}
}

void CWinnerNotice::NickNameEdit(char* nick, bool bCut, std::string& out)
{
	std::string name;
	std::string stars("***");
	char buf[50] = "";

	name = "";
	if (bCut)
	{
		mbscut(buf, nick, strlen(nick) - 2);
		name = buf;
		name += stars;
		out = name;
	}
	else
	{
		name = nick + 2;
		out = stars + name;
	}
}
