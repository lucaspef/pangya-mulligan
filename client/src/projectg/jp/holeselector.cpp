// JP 983: unidade 005100c0–00510a10 (FrHoleSelector). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B40: construtor, destrutor, registro, mapa de mensagens ("hole_preview", "holeIds") e os 10 métodos.
#include "minatl.h"
#include "projectg.h"
#include "holeselector.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frarea.h"
#include "bitmap.h"
#include "woverlay.h"
#include "wresrcmng.h"

extern Fresh* g_pFresh;
extern WView* g_view;

// 0083be90 (faixa do treasuredlg.cpp da frente A, sem nome no mapa): retângulo do quadro `index` numa folha de `texW` × `texH`
// cortada em quadros de `w` × `h`. Devolve false se a folha não tem o quadro. Nome meu; pedido B → A (B40).
bool __fastcall CalcFrameUV983(_WRECT* pOut, int texW, int texH, float w, float h, int index);
// 00e4296c: retângulo {0, 0, 1, 1} de dados (dono não identificado). Nome meu.
extern _WRECT g_fullUV983;

static const unsigned char s_holeIds[18] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }; // 00d06bd0

IMPLEMENT_OBJECT(FrHoleSelector, FrForm)

BEGIN_FRESH_MSGMAP(FrHoleSelector, FrForm)

ON_FRESH_VI("hole_preview", FRCMD_INIT, FrHoleSelector::OnPreviewInit)
ON_FRESH_VI("hole_preview", FRCMD_OWNERDRAW, FrHoleSelector::OnPreviewDraw)
ON_FRESH_VI("holeIds", FRCMD_INIT, FrHoleSelector::OnHoleInit)
ON_FRESH_VV("holeIds", FRCMD_LBUTTONDOWN, FrHoleSelector::OnHoleClick)
ON_FRESH_VI("holeIds", FRCMD_OWNERDRAW, FrHoleSelector::OnHoleDraw)
ON_FRESH_VI("holeIds", FRCMD_HOVEROFF, FrHoleSelector::OnHoleHoverOff)

END_FRESH_MSGMAP()

static WSize s_previewSize(140.0f, 93.0f); // 00e09348

FrHoleSelector::~FrHoleSelector()
{
	if (g_resrcmng && m_pSheet[0])
	{
		g_resrcmng->Release(m_pSheet[0]);
		m_pSheet[0] = NULL;
	}
	if (g_resrcmng && m_pSheet[1])
	{
		g_resrcmng->Release(m_pSheet[1]);
		m_pSheet[1] = NULL;
	}
}

bool FrHoleSelector::Close(eFormRet result, bool bFade)
{
	return FrForm::Close(result, false);
}

FrHoleSelector::FrHoleSelector()
	: m_pHoleList(NULL)
	, m_pPreview(NULL)
	, m_pCur(NULL)
{
	m_pSheet[0] = NULL;
	m_pSheet[1] = NULL;
	m_hole = 0x13;
	m_pBtn[0] = g_pFresh->GetBitmap("single_btn_hole_Normal");
	m_pBtn[1] = g_pFresh->GetBitmap("single_btn_hole_Hover");
	m_pBtn[2] = g_pFresh->GetBitmap("single_btn_hole_Select");
}

void FrHoleSelector::OnPreviewInit(int param)
{
	m_pPreview = DYNAMIC_CAST(FrArea, param);
}

void FrHoleSelector::OnPreviewDraw(int param)
{
	if (m_pPreview == NULL || m_pCur == NULL)
		return;

	WPoint pos(m_pPreview->GetRect().x, m_pPreview->GetRect().y);
	WRect rc(pos, s_previewSize);
	m_pCur->Render(g_view, m_uv, rc, 0, ((unsigned long)(Alpha983() * 255.0) << 24) | 0xffffff, 0.0f, 0);
}

void FrHoleSelector::OnHoleInit(int param)
{
	m_pHoleList = DYNAMIC_CAST(FrListBox, param);
	if (m_pHoleList == NULL)
		return;

	for (int i = 0; i < 18; ++i)
		m_pHoleList->AddItem((void*)&s_holeIds[i]);
	m_textX = (float)(m_pHoleList->GetItemWidth() / 2);
	m_textY = (float)((float)(m_pHoleList->GetItemHeight() / 2) - 5.0);
}

void FrHoleSelector::OnHoleDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL || pItem->pData == NULL)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	const Bitmap* pBtn = m_pBtn[0];
	unsigned long color = 0xff000000;
	if (pItem->selected)
	{
		pBtn = m_pBtn[2];
		color = 0xffffffff;
	}
	else if (pItem->underCursor)
		pBtn = m_pBtn[1];
	if (pBtn)
	{
		WRect rc(pItem->pos.x, pItem->pos.y, (float)pBtn->Width(), (float)pBtn->Height());
		pGDI->DrawTexture(pBtn, rc, 0xffffffff, 0);
	}
	if (pItem->pData)
	{
		unsigned char hole = *(unsigned char*)pItem->pData;
		unsigned long oldColor = pGDI->GetTextColor();
		unsigned long oldOutline = pGDI->GetTextOutlineColor();
		pGDI->SetTextColor(FrALPHA(color, Alpha983()), oldOutline);
		WPoint pt(m_textX + pItem->pos.x, m_textY + pItem->pos.y);
		pGDI->PrintText11(pt, 1, MakeStr("%d", hole), NULL);
		pGDI->SetTextColor(oldColor, oldOutline);
	}
}

void FrHoleSelector::SetPreview983(unsigned char hole)
{
	int index = hole;
	int sheet = 0;
	m_pCur = NULL;
	if (index > 10)
	{
		sheet = 1;
		if (index > 0x13)
			index = 9;
		else
			index -= 10;
	}
	else if (index < 1)
		index = 1;

	m_pCur = m_pSheet[sheet];
	if (m_pCur)
		CalcFrameUV983(&m_uv, m_pCur->GetWidth(), m_pCur->GetHeight(), s_previewSize.w, s_previewSize.h, index - 1);
	else
		m_uv = g_fullUV983;
}

void FrHoleSelector::SetCourse983(unsigned char course, unsigned char hole)
{
	int last = hole;
	if (last >= 0x13)
		last = 0x13;
	m_hole = last;

	unsigned long id = (course > 0x7f && course != 0xfd) ? course - 0x80 : course;
	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(id | 0x28000000);
	if (pCourse && course < 0x16)
	{
		if (m_pSheet[0] == NULL)
			m_pSheet[0] = g_resrcmng->GetOverlay(MakeStr("%s_1.png", pCourse->c.Name), 0);
		if (m_pSheet[1] == NULL)
			m_pSheet[1] = g_resrcmng->GetOverlay(MakeStr("%s_2.png", pCourse->c.Name), 0);
	}
	if (m_pHoleList)
		m_pHoleList->SelectItem(m_pHoleList->FindItem((void*)&s_holeIds[m_hole - 1]), true);
	SetPreview983(m_hole);
}

void FrHoleSelector::OnMouseMove(const WPoint& mousePos)
{
	FrForm::OnMouseMove(mousePos);

	unsigned char hole = m_hole;
	if (m_pHoleList && m_pHoleList->GetItemUnderCursor() && m_pHoleList->GetItemUnderCursor()->pData)
		hole = *(unsigned char*)m_pHoleList->GetItemUnderCursor()->pData;
	SetPreview983(hole);
}

void FrHoleSelector::OnHoleClick()
{
	FrListItem* pSel = m_pHoleList->GetSelected();
	if (pSel && pSel->pData)
	{
		m_hole = *(unsigned char*)pSel->pData;
		SetPreview983(m_hole);
	}
	else if (m_pHoleList)
		m_pHoleList->SelectItem(m_pHoleList->FindItem((void*)&s_holeIds[m_hole - 1]), true);
	Close(FrOK, true);
}

void FrHoleSelector::OnHoleHoverOff(int param)
{
	SetPreview983(m_hole);
}

// 0083be90 (B653): o 983 divide o indice pelo quociente texH/h (primeiro ftol) e usa o resto como linha; mantido assim.
bool __fastcall CalcFrameUV983(_WRECT* pOut, int texW, int texH, float w, float h, int index)
{
	float fw = (float)texW;
	float fh = (float)texH;
	if (!(w <= fw) || !(h <= fh))
		return false;
	int a = (int)(fh / h);
	int b = (int)(fw / w);
	if (b * a < index)
		return false;
	pOut->x = (float)(index / a) * w / fw;
	pOut->y = (float)(index % a) * h / fh;
	pOut->w = w / fw;
	pOut->h = h / fh;
	return true;
}
