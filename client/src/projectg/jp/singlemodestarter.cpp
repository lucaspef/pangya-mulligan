// JP 983: unidade 00521710–00521f70 (FrSingleModeStarter). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B40: construtor, registro, mapa de mensagens (13 entradas) e os 14 métodos. O destrutor do 983 é o do FrForm.
#include "minatl.h"
#include "projectg.h"
#include "singlemodestarter.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "fresh.h"
#include "frbutton.h"
#include "frarea.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrSingleModeStarter, FrForm)

BEGIN_FRESH_MSGMAP(FrSingleModeStarter, FrForm)

ON_FRESH_VI("expl", FRCMD_INIT, FrSingleModeStarter::OnExplInit)
ON_FRESH_VV("expl", FRCMD_LBUTTONDOWN, FrSingleModeStarter::OnExpl)
ON_FRESH_VV("close", FRCMD_LBUTTONDOWN, FrSingleModeStarter::OnClose)
ON_FRESH_VI("img_course", FRCMD_INIT, FrSingleModeStarter::OnImgCourseInit)
ON_FRESH_VI("img_chipin", FRCMD_INIT, FrSingleModeStarter::OnImgChipinInit)
ON_FRESH_VI("btn_course", FRCMD_INIT, FrSingleModeStarter::OnBtnCourseInit)
ON_FRESH_VI("btn_repeat", FRCMD_INIT, FrSingleModeStarter::OnBtnRepeatInit)
ON_FRESH_VI("btn_chipin", FRCMD_INIT, FrSingleModeStarter::OnBtnChipinInit)
ON_FRESH_VI("btn_tutorial", FRCMD_INIT, FrSingleModeStarter::OnBtnTutorialInit)
ON_FRESH_VV("btn_course", FRCMD_LBUTTONDOWN, FrSingleModeStarter::OnBtnCourse)
ON_FRESH_VV("btn_repeat", FRCMD_LBUTTONDOWN, FrSingleModeStarter::OnBtnRepeat)
ON_FRESH_VV("btn_chipin", FRCMD_LBUTTONDOWN, FrSingleModeStarter::OnBtnChipin)
ON_FRESH_VV("btn_tutorial", FRCMD_LBUTTONUP, FrSingleModeStarter::OnBtnTutorial)

END_FRESH_MSGMAP()

// 00e0a974: imagens do botão "expl" por estado (normal, sob o cursor)
static const char* s_explBtn[2][2] = {
	{ "single_start_btn_expl_off_Normal", "single_start_btn_expl_off_Hover" },
	{ "single_start_btn_expl_on_Normal", "single_start_btn_expl_on_Hover" },
};

void FrSingleModeStarter::OnClose()
{
	Close(FrCANCEL, true);
}

void FrSingleModeStarter::OnBtnCourse()
{
	m_mode = 0;
	Close(FrYES, true);
}

void FrSingleModeStarter::OnBtnRepeat()
{
	m_mode = 4;
	Close(FrYES, true);
}

void FrSingleModeStarter::OnBtnChipin()
{
	m_mode = 6;
	Close(FrYES, true);
}

FrSingleModeStarter::FrSingleModeStarter()
	: m_pExplBtn(NULL)
	, m_expl(0)
	, m_mode(0xff)
{
	m_pImg[0] = NULL;
	m_pImg[1] = NULL;
	m_pBmp[0][0] = g_pFresh->GetBitmap("single_start_img_course");
	m_pBmp[0][1] = g_pFresh->GetBitmap("single_start_img_chipin");
	m_pBmp[1][0] = g_pFresh->GetBitmap("single_start_expl_course");
	m_pBmp[1][1] = g_pFresh->GetBitmap("single_start_expl_chipin");
	m_offset[0][0].x = 45.0f;
	m_offset[0][0].y = 5.0f;
	m_offset[1][0].x = -45.0f;
	m_offset[1][0].y = -5.0f;
	m_offset[0][1].x = 0.0f;
	m_offset[1][1].x = 0.0f;
	m_offset[0][1].y = 5.0f;
	m_offset[1][1].y = -5.0f;
}

void FrSingleModeStarter::ToggleExpl983()
{
	m_expl = (m_expl + 1) % 2;
	if (m_pImg[0])
	{
		m_pImg[0]->SetBgImg(m_pBmp[m_expl][0]);
		const WRect& rect = m_pImg[0]->GetRect();
		WRect rc(rect.x + m_offset[m_expl][0].x, m_offset[m_expl][0].y + rect.y, rect.w, rect.h);
		m_pImg[0]->SetRect(rc);
	}
	if (m_pImg[1])
	{
		m_pImg[1]->SetBgImg(m_pBmp[m_expl][1]);
		const WRect& rect = m_pImg[1]->GetRect();
		WRect rc(m_offset[m_expl][1].x + rect.x, m_offset[m_expl][1].y + rect.y, rect.w, rect.h);
		m_pImg[1]->SetRect(rc);
	}
	if (m_pExplBtn)
	{
		m_pExplBtn->SetButtonImg(s_explBtn[m_expl][0], FrButton::NORMAL);
		m_pExplBtn->SetButtonImg(s_explBtn[m_expl][1], FrButton::OVER);
		m_pExplBtn->SetButtonImg(s_explBtn[m_expl][0], FrButton::PRESSED);
	}
	if (m_pBtn[0])
	{
		m_pBtn[0]->SetVisible(m_expl == 0);
		m_pBtn[1]->SetVisible(m_expl == 0);
		m_pBtn[2]->SetVisible(m_expl == 0);
		m_pBtn[3]->SetVisible(m_expl == 0);
	}
}

void FrSingleModeStarter::OnExplInit(int param)
{
	m_pExplBtn = DYNAMIC_CAST(FrButton, param);
	if (m_pExplBtn)
	{
		m_pExplBtn->SetPushDelay(0.0f);
		m_pExplBtn->UseDblClick(false);
	}
}

void FrSingleModeStarter::OnExpl()
{
	ToggleExpl983();
}

void FrSingleModeStarter::OnImgCourseInit(int param)
{
	m_pImg[0] = DYNAMIC_CAST(FrArea, param);
}

void FrSingleModeStarter::OnImgChipinInit(int param)
{
	m_pImg[1] = DYNAMIC_CAST(FrArea, param);
}

void FrSingleModeStarter::OnBtnCourseInit(int param)
{
	m_pBtn[0] = DYNAMIC_CAST(FrButton, param);
}

void FrSingleModeStarter::OnBtnRepeatInit(int param)
{
	m_pBtn[1] = DYNAMIC_CAST(FrButton, param);
}

void FrSingleModeStarter::OnBtnChipinInit(int param)
{
	m_pBtn[2] = DYNAMIC_CAST(FrButton, param);
}

void FrSingleModeStarter::OnBtnTutorialInit(int param)
{
	m_pBtn[3] = DYNAMIC_CAST(FrButton, param);
}

void FrSingleModeStarter::OnBtnTutorial()
{
	// mensagem 99 ao ator do lobby: abrir o tutorial
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 99, 0, 0, 0, 0, 0);
	Close(FrYES, true);
}
