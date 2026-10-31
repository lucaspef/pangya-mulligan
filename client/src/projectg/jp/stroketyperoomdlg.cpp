// JP 983: unidade 00523250–005235f0 (FrStrokeTypeRoomDlg). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B40: construtor, registro, mapa de mensagens (11 entradas, 7 delas com handler da FrGameRoomDlg), OnInit, OnProc e os
// quatro leitores "group_stroke.petN". O destrutor do 983 é o da FrGameRoomDlg.
#include "minatl.h"
#include "projectg.h"
#include "stroketyperoomdlg.h"
#include "frarea.h"
#include "frviewer.h"
#include "frgroupbox.h"

IMPLEMENT_OBJECT(FrStrokeTypeRoomDlg, FrGameRoomDlg)

BEGIN_FRESH_MSGMAP(FrStrokeTypeRoomDlg, FrGameRoomDlg)

ON_FRESH_VV("invite", FRCMD_LBUTTONUP, FrGameRoomDlg::OnInvite)
ON_FRESH_VI("group_stroke.pet0", FRCMD_INIT, FrStrokeTypeRoomDlg::OnPet0Init)
ON_FRESH_VI("group_stroke.pet1", FRCMD_INIT, FrStrokeTypeRoomDlg::OnPet1Init)
ON_FRESH_VI("group_stroke.pet2", FRCMD_INIT, FrStrokeTypeRoomDlg::OnPet2Init)
ON_FRESH_VI("group_stroke.pet3", FRCMD_INIT, FrStrokeTypeRoomDlg::OnPet3Init)
ON_FRESH_VI("group_stroke.overgauge", FRCMD_INIT, FrGameRoomDlg::OnOvergaugeInit)
ON_FRESH_VI("group_stroke.user", FRCMD_INIT, FrGameRoomDlg::OnUserInit)
ON_FRESH_VI("group_stroke.user", FRCMD_OWNERDRAW, FrGameRoomDlg::OnUserDraw)
ON_FRESH_VV("group_stroke.user", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnUserLButtonDown)
ON_FRESH_VV("group_stroke.user", FRCMD_RBUTTONUP, FrGameRoomDlg::OnUserRButtonUp)
ON_FRESH_VV("group_stroke.user", FRCMD_DBLCLICK, FrGameRoomDlg::OnUserDblClick)

END_FRESH_MSGMAP()

FrStrokeTypeRoomDlg::FrStrokeTypeRoomDlg()
{
	ResetRoom983();
}

bool FrStrokeTypeRoomDlg::OnInit()
{
	m_pGroupStroke->SetVisible(true);
	if (m_pBg)
		m_pBg->Open("gameroom_background.tga");
	return FrGameRoomDlg::OnInit();
}

void FrStrokeTypeRoomDlg::OnProc(const float deltaTime)
{
	FrGameRoomDlg::OnProc(deltaTime);
}

void FrStrokeTypeRoomDlg::OnPet0Init(int param)
{
	m_pPet[0] = DYNAMIC_CAST(FrArea, param);
}

void FrStrokeTypeRoomDlg::OnPet1Init(int param)
{
	m_pPet[1] = DYNAMIC_CAST(FrArea, param);
}

void FrStrokeTypeRoomDlg::OnPet2Init(int param)
{
	m_pPet[2] = DYNAMIC_CAST(FrArea, param);
}

void FrStrokeTypeRoomDlg::OnPet3Init(int param)
{
	m_pPet[3] = DYNAMIC_CAST(FrArea, param);
}
