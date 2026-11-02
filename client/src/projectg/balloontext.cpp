#include "minatl.h"
#include "balloontext.h"
#include "avatar_avatar.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fremoticon.h"
#include "wresrcmng.h"
#include "wfont.h"
#include "woverlay.h"
#include "commonutil.h"
#include "mathconsts.h"

#define BALLOON_COLOR(r, g, b, a) \
	((unsigned long)(((unsigned char)((a) * 255.0f) << 24) | \
		((unsigned char)((r) * 255.0f) << 16) | \
		((unsigned char)((g) * 255.0f) << 8) | (unsigned char)((b) * 255.0f)))

#define BALLOONMGR ntBalloonTextManager::m_pInstance

extern Fresh* g_pFresh;
extern WView* g_view;

static int s_emoticonEnd = ')';
static char s_stripBuffer[1024];

// (the naked version follows ntBalloonText::Render, see the HACK there)
ntBalloonText::ntBalloonText(unsigned long guid, const char* text,
	const float* const color, float lifeTime)
{
	m_guid = guid;
	m_bNewText = false;
	m_x = 0;
	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
	m_color[3] = color[3];
	m_lifeTime = lifeTime;
	m_elapsed = 0;
	SetText(text);
}

ntBalloonText::~ntBalloonText()
{
}

void ntBalloonText::SetText(const char* text)
{
	if (text == NULL || *text == 0)
		return;

	if (*text == '!')
	{
		text++;
		m_bThink = true;
	}
	else
	{
		m_bThink = false;
	}

	const char* stripped = StripSpecialText(text);
	if (*stripped == 0)
		return;

	m_bNewText = true;
	m_text = stripped;
	m_elapsed = 0;
}

const char* ntBalloonText::StripSpecialText(const char* text) const
{
	int len = strlen(text);
	int i = 0;
	int j = 0;
	while (i < len)
	{
		char c = text[i++];
		if (c == '/')
		{
			i++;
			while ((unsigned char)text[i] > ' ')
				i++;
			i++;
		}
		else
		{
			s_stripBuffer[j++] = c;
		}
	}
	s_stripBuffer[j] = 0;
	return s_stripBuffer;
}

bool ntBalloonText::Process(float dt)
{
	m_elapsed += dt;
	if (IsVisible())
		return true;
	return false;
}

void ntBalloonText::Render(WView* view)
{
	std::string lines[32];

	ntAvatar* pAvatar = ntAvatarManager::m_pInstance->FindAvatarByGuid(m_guid);
	if (pAvatar == NULL)
		return;

	int x, y;
	if (!pAvatar->GetBoneScreenCoord("Bip01 HeadNub", &x, &y))
		return;

	FrGraphicInterface* pGDI = g_pFresh->manager->GetGDI();
	FrEmoticon* pEmoticon = g_pFresh->manager->GetEmoticon();
	pEmoticon->SetAnim(true);

	if (!IsVisible())
	{
		if (pGDI)
			pGDI->SetScale(1.0f);
		return;
	}

	float scale;
	if (m_elapsed < 0.1)
		scale = m_elapsed / 0.1;
	else if (m_lifeTime - m_elapsed < 0.1)
		scale = (m_lifeTime - m_elapsed) / 0.1;
	else
		scale = 1.0f;

	int lineCount = 0;
	int maxWidth = 0;
	int i = 0;
	int pos = 0;
	int len = strlen(m_text.c_str());
	bool lead = false;
	bool emoticon = false;
	int emoticonLen = 0;
	char line[1024];
	char key[32];
	line[0] = 0;

	while (i < len)
	{
		char c = m_text.c_str()[i++];
		if (c == '\\' && !lead)
		{
			c = m_text.c_str()[i++];
			if (c == 'n')
			{
				lineCount++;
				continue;
			}
			if (c == 0)
				break;
			continue;
		}

		if (c == '(' && !lead)
		{
			const char* end = strchr(m_text.c_str() + i, s_emoticonEnd);
			unsigned int keyLen = end - m_text.c_str() - i;
			if (end && keyLen < 32)
			{
				memset(key, 0, 32);
				memcpy(key, m_text.c_str() + i, keyLen);
				if (pEmoticon->GetIconIndex(key) != -1)
				{
					emoticon = true;
					emoticonLen = keyLen + 2;
				}
			}
		}
		else if (lead && (unsigned char)line[pos - 1] == 0x81 &&
			(unsigned char)c == 0x69)
		{
			// full-width "(" ... ")" (0x8169 .. 0x816a)
			int last = strlen(m_text.c_str() + 0) - 1;
			for (int j = i; j < last; j++)
			{
				if ((unsigned char)m_text.c_str()[j] != 0x81 ||
					(unsigned char)m_text.c_str()[j + 1] != 0x6a)
					continue;
				unsigned int keyLen = j - i;
				if (keyLen - 1 <= 0x1e)
				{
					memcpy(key, m_text.c_str() + i, keyLen);
					key[keyLen] = 0;
					if (pEmoticon->GetIconIndex(key) != -1)
					{
						emoticon = true;
						emoticonLen = keyLen + 4;
					}
				}
				break;
			}
		}

		line[pos++] = c;
		unsigned char uc = (unsigned char)c;
		if ((uc >= 0x80 && uc < 0xa0) || (unsigned char)(uc + 0x20) <= 0x1c)
		{
			if (!lead)
			{
				lead = true;
				continue;
			}
		}
		line[pos] = 0;

		if (emoticon)
		{
			if (pos + emoticonLen > 20)
			{
				int back = lead ? 2 : 1;
				i -= back;
				line[pos - back] = 0;
				pos = 0;
				lines[lineCount++] = line;
			}
			emoticon = false;
		}
		else if (pos > 20)
		{
			int back = lead ? 2 : 1;
			i -= back;
			line[pos - back] = 0;
			pos = 0;
			lines[lineCount++] = line;
		}

		BALLOONMGR->m_pFont->SetScale(scale);
		int width = (int)BALLOONMGR->m_pFont->GetTextWidthOld983(g_view, line);
		if (width > maxWidth)
			maxWidth = width;
		if (lead)
			lead = false;
	}

	if (line[0])
		lines[lineCount++] = line;

	int textHeight = (int)(lineCount * scale * 15.0);
	if (m_bNewText)
	{
		m_x = x;
		m_bNewText = false;
	}
	if (x < m_x)
		m_x = x;
	else if (x > m_x + maxWidth)
		m_x = x - maxWidth;

	y += (int)((1.0f - scale) * 10.0);
	unsigned long color =
		BALLOON_COLOR(m_color[0], m_color[1], m_color[2], m_color[3]);

	int textX = m_x;
	int textY;
	if (!m_bThink)
	{
		int left = textX - BALLOONMGR->m_frameLeft;
		textY = y - BALLOONMGR->m_pTail->GetHeight() - textHeight -
			BALLOONMGR->m_frameBottom + 5;
		RECT rc;
		rc.left =
			(int)(left + (textX - BALLOONMGR->m_frameLeft - left) * scale);
		rc.top = y - BALLOONMGR->m_frameTop -
			BALLOONMGR->m_pTail->GetHeight() - textHeight -
			BALLOONMGR->m_frameBottom + 5;
		rc.right =
			BALLOONMGR->m_frameRight + BALLOONMGR->m_frameLeft + maxWidth;
		rc.bottom =
			BALLOONMGR->m_frameTop + textHeight + BALLOONMGR->m_frameBottom;
		WOverlay::DrawFrameOverlay9(view, BALLOONMGR->m_pFrame, rc, color);

		WRect dst(x - BALLOONMGR->m_pTail->GetWidth() / 2,
			y - (float)BALLOONMGR->m_pTail->GetHeight(),
			BALLOONMGR->m_pTail->GetWidth(), BALLOONMGR->m_pTail->GetHeight());
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		BALLOONMGR->m_pTail->RenderWithShear(view, src, dst, 0.0f, 0x80000,
			color);
	}
	else
	{
		textY = y - BALLOONMGR->m_pThinkTail->GetHeight() - textHeight - 19;
		int left = textX - BALLOONMGR->m_frameLeft;
		RECT rc;
		rc.left = (int)(left + (textX - left - 24) * scale);
		rc.top = y - BALLOONMGR->m_pThinkTail->GetHeight() - textHeight - 43;
		rc.right = maxWidth + 48;
		rc.bottom = textHeight + 48;
		WOverlay::DrawFrameOverlay1(view, BALLOONMGR->m_pThinkFrame, 24.0f,
			24.0f, 24.0f, 24.0f, rc, color);

		WRect dst(x - BALLOONMGR->m_pThinkTail->GetWidth() / 2,
			y - (float)BALLOONMGR->m_pThinkTail->GetHeight(),
			BALLOONMGR->m_pThinkTail->GetWidth(),
			BALLOONMGR->m_pThinkTail->GetHeight());
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		BALLOONMGR->m_pThinkTail->RenderWithShear(view, src, dst, 0.0f, 0x80000,
			color);
	}

	for (int k = 0; k < lineCount; k++)
	{
		if (pGDI)
		{
			pGDI->SetScale(scale);
			unsigned long oldColor = pGDI->GetTextColor();
			pGDI->SetTextStyle(0);
			pGDI->SetTextColor(BALLOON_COLOR(0, 0, 0, m_color[3]), 0xffffffff);
			WPoint pt(textX, textY + k * scale * 15.0f);
			pEmoticon->PrintText(pt, 0, lines[k].c_str(),
				BALLOON_COLOR(1, 1, 1, m_color[3]));
			pGDI->SetScale(1.0f);
			pGDI->SetTextColor(oldColor, 0xffffffff);
		}
		else
		{
			BALLOONMGR->m_pFont->SetScale(scale);
			BALLOONMGR->m_pFont->Print(view, textX, textY + k * scale * 15.0f,
				lines[k].c_str(), 0, 0xff000000, NULL);
		}
	}

	BALLOONMGR->m_pFont->Flush(view);
}

void ntBalloonTextManager::Init()
{
	// 983 007b7c80: "fnt_japan_12.wft" (00d268cc), igual CChatMsg::InitFont 0099f1a0; o 645 usava fnt_<dotum>_12.wft
	m_pFont = g_resrcmng->GetFixedFont("fnt_japan_12.wft");
	if (m_pFont)
		m_pFont->SetMode(WFont::MASKED);

	for (int i = 0; i < 9; i++)
		m_pFrame[i] =
			g_resrcmng->GetOverlay(MakeStr("chat_frame_%02d.tga", i + 1), 0);

	m_pThinkFrame = g_resrcmng->GetOverlay("framethink_02.tga", 0);
	m_pThinkTail = g_resrcmng->GetOverlay("framethink_01.tga", 0);

	m_frameLeft = m_pFrame[3]->GetWidth();
	m_frameTop = m_pFrame[1]->GetHeight();
	m_frameRight = m_pFrame[5]->GetWidth();
	m_frameBottom = m_pFrame[7]->GetHeight();

	m_pTail = g_resrcmng->GetOverlay("chat_frame_00.tga", 0);
}

void ntBalloonTextManager::Shutdown()
{
	DeleteAll();

	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}

	for (int i = 0; i < 9; i++)
	{
		if (g_resrcmng && m_pFrame[i])
		{
			g_resrcmng->Release(m_pFrame[i]);
			m_pFrame[i] = NULL;
		}
	}

	if (g_resrcmng && m_pTail)
	{
		g_resrcmng->Release(m_pTail);
		m_pTail = NULL;
	}
	if (g_resrcmng && m_pThinkFrame)
	{
		g_resrcmng->Release(m_pThinkFrame);
		m_pThinkFrame = NULL;
	}
	if (g_resrcmng && m_pThinkTail)
	{
		g_resrcmng->Release(m_pThinkTail);
		m_pThinkTail = NULL;
	}
}

void ntBalloonTextManager::DeleteAll()
{
	std::list<ntBalloonText*>::iterator it;
	for (it = m_list.begin(); it != m_list.end();)
	{
		delete *it;
		it = m_list.erase(it);
	}
	m_list.clear();
}

void ntBalloonTextManager::Process(float dt)
{
	std::list<ntBalloonText*>::iterator it;
	for (it = m_list.begin(); it != m_list.end(); it++)
		(*it)->Process(dt);
}

void ntBalloonTextManager::Render()
{
	std::list<ntBalloonText*>::iterator it;
	for (it = m_list.begin(); it != m_list.end(); ++it)
		(*it)->Render(g_view);
}

ntBalloonText* ntBalloonTextManager::CreateBalloonText(unsigned long guid,
	const char* text, const float* const color, float lifeTime)
{
	ntBalloonText* balloon = new ntBalloonText(guid, text, color, lifeTime);
	m_list.push_back(balloon);
	return balloon;
}

void ntBalloonTextManager::DeleteBalloonText(ntBalloonText* balloon)
{
	std::list<ntBalloonText*>::iterator it =
		std::find(m_list.begin(), m_list.end(), balloon);
	if (it != m_list.end())
	{
		m_list.erase(it);
		delete balloon;
	}
}
