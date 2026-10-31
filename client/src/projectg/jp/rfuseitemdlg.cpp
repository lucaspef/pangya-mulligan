// JP 983: unidade 005ccb80–005ce010 — RFUseItemDlg ("usar o item?") e RFNoticeDlg (aviso com ok), os diálogos modais comuns
// da RF ("common_use_item_dialog.xml" / "common_notice_dialog.xml"). Sessão B137. Nomes meus (…983), menos as classes e os da RF.
#include "stdafx.h"
#include "refresh/rfuseitemdlg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "soundmanager.h"

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

RFUseItemDlg* RFUseItemDlg::ms_pInstance;
RFWnd* RFUseItemDlg::s_pParent983;
RFWnd* RFNoticeDlg::s_pParent983;

// 005ccb80
void RFUseItemDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005ccb90
void RFUseItemDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005ccba0
void RFUseItemDlg::OnOk983(EventArgs& e)
{
	Close(1);
}

// 005ccbb0
void RFUseItemDlg::OnCancel983(EventArgs& e)
{
	Close(2);
}

// 005ccbc0
void RFNoticeDlg::OnOk983(EventArgs& e)
{
	Close(1);
}

// 005ccbd0
RFUseItemDlg::RFUseItemDlg()
{
	ms_pInstance = this;
	m_pItemImg983 = NULL;
	m_pItemNum983 = NULL;
	m_pMessage983 = NULL;
}

// 005ccc70
RFUseItemDlg::~RFUseItemDlg()
{
	ms_pInstance = NULL;
}

// 005ccce0
RFNoticeDlg::RFNoticeDlg()
{
	m_pMessage983 = NULL;
}

// 005ccd70
RFNoticeDlg::~RFNoticeDlg()
{
}

// 005cd2c0
void RFUseItemDlg::SetItem983(sItemInfo* pItem)
{
	if (pItem)
	{
		if (m_pItemImg983)
		{
			IFF_ITEM_COMMON* pIff = (IFF_ITEM_COMMON*)ItemManager()->FindItem(pItem->typeId);
			m_pItemImg983->SetPicture(MultiByteToWide983(MakeStr("%s.tga", pIff->Icon)));
		}
		if (m_pItemNum983)
			m_pItemNum983->SetLabel(MultiByteToWide983(MakeStr("x%d", pItem->Common[0])));
	}
}

// 005cd470
void RFUseItemDlg::SetIffItem983(IFF_STRUCT::sItem* pIff)
{
	if (pIff)
	{
		if (m_pItemImg983)
			m_pItemImg983->SetPicture(MultiByteToWide983(MakeStr("%s.tga", pIff->c.Icon)));
		if (m_pItemNum983)
			m_pItemNum983->SetLabel(MultiByteToWide983(MakeStr("x%d", pIff->COM[0])));
	}
}

// 005cd610
void RFNoticeDlg::SetMessage983(const char* text)
{
	if (m_pMessage983)
		m_pMessage983->SetLabel(MultiByteToWide983(text));
}

// 005cd6f0
RFUseItemDlg* RFUseItemDlg::Create983(RFWnd* parent)
{
	RFUseItemDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFUseItemDlg>(L"common_use_item_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFUseItemDlg>(L"common_use_item_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFUseItemDlg>(L"common_use_item_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005cd830
void RFUseItemDlg::SetMessage983(const std::wstring& text)
{
	if (m_pMessage983)
		m_pMessage983->SetText(text.c_str());
}

// 005cd900
void RFUseItemDlg::SetMessage983(const char* text)
{
	if (m_pMessage983)
		m_pMessage983->SetText(MultiByteToWide983(text).c_str());
}

// 005cda40
void RFUseItemDlg::OnInitialUpdate()
{
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFUseItemDlg::OnOk983, this));
	RegisterTo(L"cancel_btn", RFButton::EventClicked, Subscriber(&RFUseItemDlg::OnCancel983, this));
	m_pItemImg983 = GetChild983<RFPicture>(L"item_img");
	m_pItemNum983 = GetChild983<RFLabel>(L"item_num_label");
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005cdd60
RFNoticeDlg* RFNoticeDlg::Create983(RFWnd* parent)
{
	RFNoticeDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFNoticeDlg>(L"common_notice_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFNoticeDlg>(L"common_notice_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFNoticeDlg>(L"common_notice_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005cdea0
void RFNoticeDlg::OnInitialUpdate()
{
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFNoticeDlg::OnOk983, this));
	m_pMessage983 = GetChild983<RFLabel>(L"message");
}
