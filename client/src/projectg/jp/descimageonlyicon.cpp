// JP 983: unidade 00b13af9–00b14aa0 (CDescImageOnlyIcon + CIconManager do 983). Classes do 983; ver include/jp/topicon.h.
// Sessão B56. Ficam fora: 00b13800–00b13af4 (fim da unidade anterior: posições do presente de aniversário em Lua e a
// "BirthdayEvent"), os moldes de vector<CTopIcon*> / vector<std::string> (00b13f90–00b147b0) e 00b14950 (push_back de
// vector<CTopIcon*> fora de linha), que o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frwnd.h"
#include "frwndinl.h"
#include "frbutton.h"
#include "frform.h"
#include "noticedlg.h"
#include "tinyxml.h"
#include "lobbyjp.h"
#include "extraresource.h"
#include "topicon.h"

extern Fresh* g_pFresh; // 00f02340
extern int g_lobbyModal983; // 00e78cac (logo depois do CLobbyState983::ms_pInstance; dono não levantado)

//////////////////////////////////////////////////////////////////////////////// CDescImageOnlyIcon

// 00b13b00
void CDescImageOnlyIcon::SetIconImage(FrButton& button)
{
	button.SetVisible(true);
	button.SetPushSound("ui_desktop_icon_click");
	CTopIcon::SetIconImage(button);
}

// 00b13b30
bool CDescImageOnlyIcon::OnNoticeResult(int result, FrForm* pForm)
{
	m_pNoticeDlg = NULL;
	return true;
}

// 00b13b40
void CDescImageOnlyIcon::OnButtonDown()
{
	if (m_pNoticeDlg)
		return;

	m_pNoticeDlg = CreateForm<FrNoticeDlg>(g_pFresh->GetManager(), this, "notice_popup", NULL);
	if (!m_pNoticeDlg)
		return;

	if (!m_files.empty())
	{
		if (m_files.size() > 1)
			m_pNoticeDlg->SetImages983(m_files);
		else
			m_pNoticeDlg->LoadOnly(m_files[0].c_str());
	}
	m_pNoticeDlg->SetCaption983(m_caption);
	m_pNoticeDlg->Open((FRESH_PFN_RESULT)&CDescImageOnlyIcon::OnNoticeResult, 1);
}

// 00b13c20
bool CDescImageOnlyIcon::LoadCaption(TiXmlElement* pElem)
{
	TiXmlElement* pCaption = pElem->FirstChildElement("caption");
	if (!pCaption)
		return false;

	std::string name = pCaption->Attribute("name");
	if (name.empty())
		return false;
	m_caption = name;
	return true;
}

// 00b13ce0
CDescImageOnlyIcon::CDescImageOnlyIcon()
{
	m_pNoticeDlg = NULL;
}

// 00b13d90
CDescImageOnlyIcon::~CDescImageOnlyIcon()
{
}

// 00b13e60
bool CDescImageOnlyIcon::Load(TiXmlElement* pElem)
{
	if (!CTopIcon::Load(pElem))
		return false;

	for (TiXmlElement* pFile = pElem->FirstChildElement("file"); pFile; pFile = pFile->NextSiblingElement("file"))
	{
		std::string file = CExtraResource::Instance()->GetFile(pFile, true);
		if (file.size() > 0)
			m_files.push_back(file);
	}
	LoadCaption(pElem);
	return true;
}

//////////////////////////////////////////////////////////////////////////////// CIconManager

// 00b141f0
void CIconManager::CalcRect983(unsigned long index, WRect* pRect)
{
	if (!pRect)
		return;

	float height = CUiPointer983::Instance983()->GetScreenSize983().h;
	int rows = (int)((height - 340.0) / 80.0);
	pRect->x = (float)(index / rows * 80 + 5);
	pRect->y = (float)(index % rows * 80 + 80) + 100.0 + 80.0;
}

// 00b14290
void CIconManager::UpdateLayout983()
{
	if (!CLobbyState983::Instance983() || g_lobbyModal983)
		return;

	for (unsigned int i = 0; i < m_icons.size(); ++i)
	{
		CTopIcon* pIcon = m_icons[i];
		if (!pIcon)
			continue;
		FrButton* pButton = pIcon->GetButton();
		if (!pButton)
			continue;
		WRect rect;
		pButton->GetClientRect(rect);
		CalcRect983(i, &rect);
		pButton->SetClientRect(rect);
	}
}

// 00b14320
void CIconManager::OnButtonDown(unsigned long index)
{
	if (m_icons.size() < index)
		return;
	m_icons[index]->OnButtonDown();
}

// 00b144d0
void CIconManager::InitalizeTopIcon(unsigned long index, int wnd)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)wnd);
	if (!pButton)
		return;

	if (index >= m_icons.size())
	{
		pButton->SetVisible(false);
		pButton->Enable(false);
		return;
	}
	CTopIcon* pIcon = m_icons[index];
	if (pIcon == NULL)
	{
		pButton->SetVisible(false);
		pButton->Enable(false);
		return;
	}
	WRect rect;
	pButton->GetClientRect(rect);
	CalcRect983(index, &rect);
	pButton->SetClientRect(rect);
	pIcon->SetIconImage(*pButton);
}

// 00b147f0
CIconManager::CIconManager()
{
	for (unsigned int i = 0; i < m_icons.size(); ++i)
		m_icons[i] = NULL;
}

// 00b14840
CIconManager::~CIconManager()
{
	for (std::vector<CTopIcon*>::iterator it = m_icons.begin(); it != m_icons.end(); ++it)
		delete *it;
}

// 00b148f0
std::string CIconManager::GetIconName983(unsigned long index) const
{
	if (m_icons.size() <= index)
		return "";
	return m_icons[index]->GetName();
}

// 00b149f0
void CIconManager::LoadExtra983(TiXmlElement* pElem)
{
	if (!pElem)
		return;

	for (TiXmlElement* pChild = pElem->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement())
	{
		CTopIcon* pIcon = CTopIcon::Create(pChild);
		if (pIcon)
			m_icons.push_back(pIcon);
	}
}
