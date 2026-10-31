#include "fremoticon.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "wresrcmng.h"
#include "tinyxml.h"
#include "../../shared/token.h"
#include "wlocalize.h"	// 983: K2L_Compatibility (00b4ec60)
#include <mmsystem.h>

// HACK: stands in for an unknown erased inline use of __rtti_obj
static __declspec(thread) int __rtti_tls_pad;

extern "C" __declspec(dllimport) int __cdecl strcmpi(const char*, const char*);

inline const char* cTokenV::GetBuf() const
{
	return m_pcBuf;
}

// 983 (00c09140/00c097e0...): o ")" e o prefixo de espelho passam por K2L; se o texto local tem 2 bytes
// (caractere Shift-JIS largo), o 983 compara os 2 bytes, senao so o 1o
static bool IsRiteBefore983(const char* buf, int pos, const char* pRite)
{
	if (strlen(pRite) == 2)
		return pos > 1 && buf[pos - 2] == pRite[0] && buf[pos - 1] == pRite[1];
	return buf[pos - 1] == *pRite;
}

// 983 00c09ab2: K2L("-(flip)" coreano) no inicio da chave = icone espelhado (o 645 procurava "-" em qualquer lugar)
static bool IsFlipKey983(const char* key)
{
	static const char* pFlip = K2L_Compatibility("-(\xc7\xc3\xb8\xb3)");
	size_t len = strlen(key);
	if (strlen(pFlip) == 2)
		return len >= 3 && key[0] == pFlip[0] && key[1] == pFlip[1];
	return len >= 2 && key[0] == pFlip[0];
}

FrEmoticon::FrEmoticon(FrWndManager* pManager)
{
	m_pManager = pManager;
	m_overlay = 0;
	m_EnableAnim = false;
	m_AnimStart = 0;
	m_selWidth = 26;
	m_selHeight = 24;
	m_selXNum = 9;
	m_selYNum = 10;
	m_selNum = 90;
}

FrEmoticon::~FrEmoticon()
{
	std::vector<sEmoticon*>::iterator e;
	for (e = m_Emoticons.begin(); e != m_Emoticons.end(); e++)
	{
		(*e)->Frames.clear();
		delete *e;
	}
	m_Emoticons.clear();
	std::map<std::string, WOverlay*>::iterator i;
	for (i = m_ResMap.begin(); i != m_ResMap.end(); i++)
		if (g_resrcmng && (*i).second)
		{
			g_resrcmng->Release((*i).second);
			(*i).second = 0;
		}
	m_ResMap.clear();
	if (g_resrcmng && m_overlay)
	{
		g_resrcmng->Release(m_overlay);
		m_overlay = 0;
	}
}

bool FrEmoticon::Init()
{
	LoadXml("emoticon.xml");
	if (!m_overlay)
		m_overlay = g_resrcmng->GetOverlay("emoticon.tga", 0);
	return true;
}

bool FrEmoticon::Draw(int icon, float x, float y, unsigned long diffuse,
	bool flip)
{
	return Draw(icon, WRect(x, y, m_selWidth, m_selHeight), diffuse, flip);
}

void FrEmoticon::SetClippingArea(WRect* clip)
{
	m_pManager->GetGDI()->SetClippingArea(clip);
	m_overlay->SetClippingArea(clip);
	for (std::map<std::string, WOverlay*>::iterator i = m_ResMap.begin();
		i != m_ResMap.end(); i++)
		i->second->SetClippingArea(clip);
}

bool FrEmoticon::Draw(int icon, const WRect& dst, unsigned long diffuse,
	bool flip)
{
	if (icon < 0)
		return false;
	FrGraphicInterface* pGdi = m_pManager->GetGDI();
	if (!pGdi)
		return false;
	unsigned long frame = (timeGetTime() - m_AnimStart) / (1000 / m_Fps);
	sEmoticon* emo = m_Emoticons[icon];
	int selected;
	if (m_EnableAnim && emo->Frames.size())
		selected = emo->Frames[frame % emo->Frames.size()];
	else
		selected = icon;
	int selX = selected % m_selXNum;
	int selY = (selected % m_selNum) / m_selXNum;
	WOverlay* overlay =
		m_EnableAnim && emo->Frames.size() ? emo->Overlay : m_overlay;
	int width = overlay->GetWidth();
	int height = overlay->GetHeight();
	unsigned long color = (int(pGdi->GetAlpha() * 255) << 24) | 0xffffff;
	if (flip)
		overlay->Render(g_view,
			WRect(float((selX + 1) * m_selWidth) / width,
				float(selY * m_selHeight) / height, float(-m_selWidth) / width,
				float(m_selHeight) / height),
			dst, 0, color, 0, 0);
	else
		overlay->Render(g_view,
			WRect(float(selX * m_selWidth) / width,
				float(selY * m_selHeight) / height, float(m_selWidth) / width,
				float(m_selHeight) / height),
			dst, 0, color, 0, 0);
	return true;
}

float FrEmoticon::PrintText(const WPoint& pos, unsigned long align,
	const char* text, unsigned long emoDiffuse)
{
	FrGraphicInterface* pGdi = m_pManager->GetGDI();
	if (!pGdi)
		return 0.0f;
	pGdi->SetTextColor((pGdi->GetTextColor() & 0xffffff) |
			(emoDiffuse & 0xff000000),
		(pGdi->GetTextOutlineColor() & 0xffffff) | (emoDiffuse & 0xff000000));
	WPoint p;
	switch (align)
	{
	case 0:
		p.x = pos.x;
		p.y = pos.y;
		break;
	case 1:
		p.x = pos.x - GetTextWidth(text) * 0.5f;
		p.y = pos.y;
		break;
	case 2:
		p.x = pos.x - GetTextWidth(text);
		p.y = pos.y;
		break;
	}
	cTokenV token;
	token.Init(text, strlen(text));
	static const char* pLeft = (0, K2L_Compatibility("("));
	static const char* pRite = (0, K2L_Compatibility(")"));
	static const char* pBoth = (0, K2L_Compatibility("()"));
	while (token.GetPos() < token.GetLen())
	{
		char* str;
		bool found = token.GetToken(&str, pLeft, 1);
		p.x += pGdi->PrintText(p, 0, str, 0);
		if (token.GetPos() >= token.GetLen())
		{
			if (found)
				p.x += pGdi->PrintText(p, 0, pLeft, 0);
			break;
		}
		while ((found = token.GetToken(&str, pBoth, 2)))
		{
			if (token.GetPos() == 0 ||
				IsRiteBefore983(token.GetBuf(), token.GetPos(), pRite))
				break;
			p.x += pGdi->PrintText(p, 0, pLeft, 0);
			p.x += pGdi->PrintText(p, 0, str, 0);
		}
		int icon = GetIconIndex(str);
		if (found && icon != -1)
		{
			bool flip = IsFlipKey983(str);
			int height = m_selHeight;
			float y = p.y - (height - pGdi->GetFontHeight());
			float h = height;
			float w = m_selWidth;
			Draw(icon, WRect(p.x, y, w, h), emoDiffuse, flip);
			p.x += m_selWidth;
		}
		else
		{
			p.x += pGdi->PrintText(p, 0, pLeft, 0);
			p.x += pGdi->PrintText(p, 0, str, 0);
			if (found)
				p.x += pGdi->PrintText(p, 0, pRite, 0);
		}
	}
	return p.x - pos.x;
}

float FrEmoticon::PrintText11(const WPoint& pos, unsigned long align,
	const char* text, unsigned long emoDiffuse)
{
	FrGraphicInterface* pGdi = m_pManager->GetGDI();
	if (!pGdi)
		return 0.0f;
	pGdi->SetTextColor((pGdi->GetTextColor() & 0xffffff) |
			(emoDiffuse & 0xff000000),
		(pGdi->GetTextOutlineColor() & 0xffffff) | (emoDiffuse & 0xff000000));
	WPoint p;
	switch (align)
	{
	case 0:
		p.x = pos.x;
		p.y = pos.y;
		break;
	case 1:
		p.x = pos.x - GetTextWidth11(text) * 0.5f;
		p.y = pos.y;
		break;
	case 2:
		p.x = pos.x - GetTextWidth11(text);
		p.y = pos.y;
		break;
	}
	cTokenV token;
	token.Init(text, strlen(text));
	static const char* pLeft = (0, K2L_Compatibility("("));
	static const char* pRite = (0, K2L_Compatibility(")"));
	static const char* pBoth = (0, K2L_Compatibility("()"));
	while (token.GetPos() < token.GetLen())
	{
		char* str;
		bool found = token.GetToken(&str, pLeft, 1);
		p.x += pGdi->PrintText11(p, 0, str, 0);
		if (token.GetPos() >= token.GetLen())
		{
			if (found)
				p.x += pGdi->PrintText11(p, 0, pLeft, 0);
			break;
		}
		while ((found = token.GetToken(&str, pBoth, 2)))
		{
			if (token.GetPos() == 0 ||
				IsRiteBefore983(token.GetBuf(), token.GetPos(), pRite))
				break;
			p.x += pGdi->PrintText11(p, 0, pLeft, 0);
			p.x += pGdi->PrintText11(p, 0, str, 0);
		}
		int icon = GetIconIndex(str);
		if (found && icon != -1)
		{
			// 983 00c09cf0: aqui (PrintText11) o espelho continua strstr(str,"-"), nao o prefixo do PrintText 00c097e0
			bool flip = strstr(str, "-") != NULL;
			int height = m_selHeight;
			float y = p.y - (height - pGdi->GetFontHeight());
			float h = height;
			float w = m_selWidth;
			Draw(icon, WRect(p.x, y, w, h), emoDiffuse, flip);
			p.x += m_selWidth;
		}
		else
		{
			p.x += pGdi->PrintText11(p, 0, pLeft, 0);
			p.x += pGdi->PrintText11(p, 0, str, 0);
			if (found)
				p.x += pGdi->PrintText11(p, 0, pRite, 0);
		}
	}
	return p.x - pos.x;
}

float FrEmoticon::GetTextWidth(const char* text)
{
	FrGraphicInterface* pGdi = m_pManager->GetGDI();
	if (!pGdi)
		return 0.0f;
	float width = 0.0f;
	cTokenV token;
	token.Init(text, strlen(text));
	static const char* pLeft = (0, K2L_Compatibility("("));
	static const char* pRite = (0, K2L_Compatibility(")"));
	static const char* pBoth = (0, K2L_Compatibility("()"));
	while (token.GetPos() < token.GetLen())
	{
		char* str;
		bool found = token.GetToken(&str, pLeft, 1);
		width += pGdi->GetTextExtendOld983(str);
		if (token.GetPos() >= token.GetLen())
		{
			if (found)
				width += pGdi->GetTextExtendOld983(pLeft);
			break;
		}
		while ((found = token.GetToken(&str, pBoth, 2)))
		{
			if (token.GetPos() == 0 ||
				IsRiteBefore983(token.GetBuf(), token.GetPos(), pRite))
				break;
			width += pGdi->GetTextExtendOld983(pLeft);
			width += pGdi->GetTextExtendOld983(str);
		}
		if (found && GetIconIndex(str) != -1)
			width += m_selWidth;
		else
		{
			width += pGdi->GetTextExtendOld983(pLeft);
			width += pGdi->GetTextExtendOld983(str);
			if (found)
				width += pGdi->GetTextExtendOld983(pRite);
		}
	}
	return width;
}

float FrEmoticon::GetTextWidth11(const char* text)
{
	FrGraphicInterface* pGdi = m_pManager->GetGDI();
	if (!pGdi)
		return 0.0f;
	float width = 0.0f;
	cTokenV token;
	token.Init(text, strlen(text));
	static const char* pLeft = (0, K2L_Compatibility("("));
	static const char* pRite = (0, K2L_Compatibility(")"));
	static const char* pBoth = (0, K2L_Compatibility("()"));
	while (token.GetPos() < token.GetLen())
	{
		char* str;
		bool found = token.GetToken(&str, pLeft, 1);
		width += pGdi->GetTextExtendOld11_983(str);
		if (token.GetPos() >= token.GetLen())
		{
			if (found)
				width += pGdi->GetTextExtendOld11_983(pLeft);
			break;
		}
		while ((found = token.GetToken(&str, pBoth, 2)))
		{
			if (token.GetPos() == 0 ||
				IsRiteBefore983(token.GetBuf(), token.GetPos(), pRite))
				break;
			width += pGdi->GetTextExtendOld11_983(pLeft);
			width += pGdi->GetTextExtendOld11_983(str);
		}
		if (found && GetIconIndex(str) != -1)
			width += m_selWidth;
		else
		{
			width += pGdi->GetTextExtendOld11_983(pLeft);
			width += pGdi->GetTextExtendOld11_983(str);
			if (found)
				width += pGdi->GetTextExtendOld11_983(pRite);
		}
	}
	return width;
}

int FrEmoticon::GetIconIndex(const char* key)
{
	// 983 00c08db0: nome e apelidos passam por K2L (vazio = pula); prefixo de espelho de 2 bytes se o K2L for largo
	if (!key || !*key)
		return -1;
	static const char* pFlip = K2L_Compatibility("-(\xc7\xc3\xb8\xb3)");
	static const bool bWideFlip = strlen(pFlip) == 2;
	const char* flipKey = NULL;
	if (!bWideFlip)
	{
		if (*key == '-')
			flipKey = key + 1;
	}
	else if (strlen(key) > 1 && key[0] == pFlip[0] && key[1] == pFlip[1])
		flipKey = key + 2;
	for (int i = 0; i < m_Emoticons.size(); i++)
	{
		sEmoticon* emo = m_Emoticons[i];
		const char* name = K2L_Compatibility(emo->Name.c_str());
		if (*name && (!strcmpi(name, key) || (flipKey && !strcmpi(name, flipKey))))
			return i;
		for (int j = 0; j < 2; j++)
		{
			const char* alias = K2L_Compatibility(emo->Alias[j].c_str());
			if (*alias && (!strcmpi(alias, key) || (flipKey && !strcmpi(alias, flipKey))))
				return i;
		}
	}
	return -1;
}

const char* FrEmoticon::GetIconName(int index)
{
	return m_Emoticons[index]->Name.c_str();
}
int FrEmoticon::GetIconNum()
{
	return m_Emoticons.size();
}
void FrEmoticon::GetAlias(std::string (*buffer)[2], int index)
{
	if (strlen(m_Emoticons[index]->Alias[0].c_str()))
		(*buffer)[0] = m_Emoticons[index]->Alias[0];
	if (strlen(m_Emoticons[index]->Alias[1].c_str()))
		(*buffer)[1] = m_Emoticons[index]->Alias[1];
}

void FrEmoticon::ParseAnimFrame(const std::string& Frames,
	std::vector<int>& Vector)
{
	char Seps[] = " ";
	char* token = strtok(const_cast<char*>(Frames.c_str()), Seps);
	while (token)
	{
		Vector.push_back(atoi(token));
		token = strtok(0, Seps);
	}
}

bool FrEmoticon::LoadXml(const char* FileName)
{
	TiXmlDocument Xml;
	if (m_Emoticons.size())
		return true;
	if (!Xml.LoadFileEx(FileName))
		return false;
	TiXmlNode* Root = Xml.FirstChild("emoticon");
	if (!Root)
		return false;
	Root->ToElement()->Attribute("fps", &m_Fps);
	TiXmlNode* DefNode = Root->FirstChild("def");
	while (DefNode)
	{
		TiXmlElement* Def = DefNode->ToElement();
		sEmoticon* Emoticon = new sEmoticon;
		Emoticon->Name = Def->Attribute("name");
		Emoticon->Alias[0] = Def->Attribute("alias");
		Emoticon->Alias[1] = Def->Attribute("alias2");
		TiXmlNode* AnimNode = DefNode->FirstChild("anim");
		if (AnimNode)
		{
			TiXmlElement* Anim = AnimNode->ToElement();
			std::string Res = Anim->Attribute("res");
			ParseAnimFrame(Anim->Attribute("frame"), Emoticon->Frames);
			std::map<std::string, WOverlay*>::iterator i = m_ResMap.find(Res);
			if (i == m_ResMap.end())
			{
				WOverlay* overlay = g_resrcmng->GetOverlay(Res.c_str(), 0);
				m_ResMap[Res] = overlay;
				Emoticon->Overlay = overlay;
			}
			else
				Emoticon->Overlay = m_ResMap[Res];
		}
		m_Emoticons.push_back(Emoticon);
		DefNode = DefNode->NextSibling();
	}
	return true;
}
