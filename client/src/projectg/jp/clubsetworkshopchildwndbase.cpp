// JP 983: unidade 004ca630–004cccd0 (ClubSetWorkshopChildWndBase + ClubSetWorkshopHelper, base das páginas da oficina de tacos).
// Classes do 839/983, sem par no 645; ver include/jp/clubsetworkshopchildwndbase.h. Sessão B77 (F3.7.9, 2º lote). Não escritos:
// os moldes de vector/string que o compilador gera sozinho (004ca6e0…004cb130, 004cb3a0…004cb550, 004cbb70…004cc5b0).
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "titlefontex.h"
#include "shareddoc.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopmaindlg.h"
#include "clubsetworkshopchildwndbase.h"
class WView;
extern WView* g_pMainView983; // 00f02458
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c (retângulo de origem padrão)

// ---------------------------------------------------------------- ClubSetWorkshopHelper (1ª parte)

// 004ca630
unsigned long ClubSetWorkshopHelper::GetTypeId983(unsigned int index)
{
	if (index < m_typeIds.size())
		return m_typeIds[index];
	return 0;
}

// ---------------------------------------------------------------- ClubSetWorkshopChildWndBase

// 004ca660
void ClubSetWorkshopChildWndBase::SetShow983(bool bShow)
{
	for (unsigned int i = 0; i < m_wnds.size(); i++)
	{
		FrWnd* p = m_wnds[i];
		if (p)
			p->SetVisible(bShow);
	}
	m_bShow = bShow;
}

// 004ca6c0
void ClubSetWorkshopChildWndBase::OnBeginWork983()
{
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->DisableTabs983();
}

// 004ca6d0
void ClubSetWorkshopChildWndBase::OnEndWork983()
{
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->EnableTabs983();
}

// ---------------------------------------------------------------- ClubSetWorkshopHelper

// 004cb170
WOverlay* ClubSetWorkshopHelper::FindOverlay983(std::string name)
{
	for (unsigned int i = 0; i < m_overlays.size(); i++)
	{
		if (m_overlays[i].bUse == true && m_overlays[i].name == name)
			return m_overlays[i].pOverlay;
	}
	return NULL;
}

// 004cb280: [no+0xc] = typeId; Maintenance 004c3bd0/004c3c60 leem a quantidade com movsx word [item+0xc]
C_ASSERT(offsetof(sItemInfo, typeId) == 0x4);
C_ASSERT(offsetof(sItemInfo, Common) == 0xc);
sItemInfo* ClubSetWorkshopHelper::FindMyItem983(unsigned int index)
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); it++)
	{
		if ((*it).typeId == GetTypeId983(index))
			return &(*it);
	}
	return NULL;
}

// 004cb2d0
sCards* ClubSetWorkshopHelper::FindCard983(unsigned int index)
{
	std::list<sCards>::iterator it;
	for (it = Doc()->m_cardList.begin(); it != Doc()->m_cardList.end(); it++)
	{
		if ((*it).tid == GetTypeId983(index))
			return &(*it);
	}
	return NULL;
}

// 004cb320
sItemInfo* ClubSetWorkshopHelper::FindMyItemByGuid983(unsigned long guid)
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); it++)
	{
		if ((*it).guid == guid)
			return &(*it);
	}
	return NULL;
}

// 004cb350
sItemInfo* ClubSetWorkshopHelper::FindClubSetByGuid983(unsigned long guid)
{
	std::map<unsigned int, sItemInfo>::const_iterator it;
	for (it = Doc()->m_clubSetMap.begin(); it != Doc()->m_clubSetMap.end(); ++it)
	{
		if ((*it).second.guid == guid)
			return (sItemInfo*)&(*it).second;
	}
	return NULL;
}

// 004cb5a0
void ClubSetWorkshopHelper::DrawOverlay983(std::string name, int x, int y, int w)
{
	int left = x;
	WOverlay* p = FindOverlay983(name);
	if (p)
	{
		int width = w;
		if (!width)
			width = p->GetWidth();
		_WRECT dst = { (float)left, (float)y, (float)width, (float)p->GetHeight() };
		p->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, 0xffffffff, 0.0f, 0);
	}
}

// ---------------------------------------------------------------- ClubSetWorkshopChildWndBase

// 004cb690
ClubSetWorkshopChildWndBase::ClubSetWorkshopChildWndBase()
{
	Reset983();
}

// 004cb6f0
ClubSetWorkshopChildWndBase::~ClubSetWorkshopChildWndBase()
{
	Reset983();
}

// ---------------------------------------------------------------- ClubSetWorkshopHelper

// 004cb970
bool ClubSetWorkshopHelper::CreateFonts983(int count)
{
	if (m_fonts.size() > 0)
		return false;
	for (int i = 0; i < count; i++)
		m_fonts.push_back(new CWindFont());
	return true;
}

// ---------------------------------------------------------------- ClubSetWorkshopChildWndBase

// 004cbb30
FrWnd* ClubSetWorkshopChildWndBase::AddCtrl983(IObject* pObj)
{
	FrWnd* p = m_pForm->AddCtrlByType983(pObj);
	if (p)
		m_wnds.push_back(p);
	return p;
}

// 004cc110
void ClubSetWorkshopChildWndBase::AddCtrlByName983(std::string name)
{
	FrWnd* p = FindCtrl983(name.c_str());
	if (!p)
		return;
	m_wnds.push_back(p);
}

// ---------------------------------------------------------------- ClubSetWorkshopHelper

// 004cc630
ClubSetWorkshopHelper::ClubSetWorkshopHelper()
{
	m_overlays.clear();
	m_lastOverlay = 0;
	m_lastFrame = 0;
	m_fonts.clear();
	m_pCardNumFont = NULL;
}

// 004cc6f0
ClubSetWorkshopHelper::~ClubSetWorkshopHelper()
{
	for (unsigned int i = 0; i < m_overlays.size(); i++)
	{
		if (g_resrcmng && m_overlays[i].pOverlay)
		{
			g_resrcmng->Release(m_overlays[i].pOverlay);
			m_overlays[i].pOverlay = NULL;
		}
	}
	m_overlays.clear();
	for (unsigned int i = 0; i < m_fonts.size(); i++)
	{
		if (m_fonts[i])
		{
			delete m_fonts[i];
			m_fonts[i] = NULL;
		}
	}
	if (m_pCardNumFont)
	{
		delete m_pCardNumFont;
		m_pCardNumFont = NULL;
	}
}

// 004cc9c0
int ClubSetWorkshopHelper::LoadOverlay983(std::string name, bool bUse)
{
	WOverlay* p = g_resrcmng->GetOverlay(name.c_str(), 0x80000000);
	if (p)
	{
		m_lastOverlay = m_overlays.size();
		sOverlay983 info;
		info.name = name;
		info.pOverlay = p;
		info.bUse = bUse;
		m_overlays.push_back(info);
		return m_lastOverlay;
	}
	return -1;
}

// 004ccaf0
int ClubSetWorkshopHelper::LoadFrame983(std::string left, std::string mid, std::string right, int width, int offsetX, int offsetY)
{
	m_lastFrame = m_frames.size();
	sFrame983 f;
	f.left = LoadOverlay983(left, true);
	f.mid = LoadOverlay983(mid, true);
	f.right = LoadOverlay983(right, true);
	int lw = m_overlays[f.left].pOverlay->GetWidth();
	int rw = m_overlays[f.right].pOverlay->GetWidth();
	f.midW = width - rw - lw;
	f.leftW = lw;
	f.rightW = rw;
	f.offsetX = offsetX;
	f.offsetY = offsetY;
	m_frames.push_back(f);
	return m_lastFrame;
}
