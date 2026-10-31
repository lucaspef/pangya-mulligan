// JP 983: unidade 00521f70–00523250 (FrSingleTypeDlg). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B41: construtor, destrutor, registro, mapa de mensagens (22 entradas, 5 delas com handler da FrGameRoomDlg), OnProc,
// OnInit e os 24 métodos. 005231b0 (sem chamador) fica no fim, como função livre.
#include "minatl.h"
#include "projectg.h"
#include "singletypedlg.h"
#include "holeselector.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "holeinonegamedoc.h"
#include "icontentsdoc.h"
#include "lobbyjp.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "packet.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frbutton.h"
#include "frgroupbox.h"
#include "frcombobox.h"
#include "bitmap.h"

extern Fresh* g_pFresh;

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
#define DOC983_U32(off) (*(unsigned long*)DOCPTR983(off))

// 00be7950 (frwnd.cpp da frente A, sem nome no mapa): liga o byte 00f1ab38. Nome meu; pedido B → A (B41).
class FrWnd983
{
public:
	static void SetF1ab38_983();
};

// transparência de outra janela: o 983 lê e grava FrWnd+0x80 direto (`FrWnd::SetAlpha` 00522110 existe fora de linha nesta
// unidade, sem chamador aqui)
class FrWndAlpha983 : public FrWnd
{
public:
	float& Alpha983() { return m_wndAlpha; }
};
#define WNDALPHA983(p) (((FrWndAlpha983*)(p))->Alpha983())

// item da lista de CSharedDoc+0x375c (só os dois campos lidos aqui)
struct sDocItem983
{
	int m_00;
	unsigned long m_typeId; // nó +0x0c
	int m_08;
	short m_count; // nó +0x14
};
#define DOC983_ITEMS() (*(std::list<sDocItem983>*)DOCPTR983(0x375c))

inline float Abs983(const float& f)
{
	return (*(unsigned long*)&f & 0x80000000) ? -f : f;
}

IMPLEMENT_OBJECT(FrSingleTypeDlg, FrGameRoomDlg)

BEGIN_FRESH_MSGMAP(FrSingleTypeDlg, FrGameRoomDlg)

ON_FRESH_VI("mode_title", FRCMD_INIT, FrSingleTypeDlg::OnModeTitleInit)
ON_FRESH_VI("course_ctrl", FRCMD_INIT, FrSingleTypeDlg::OnCourseCtrlInit)
ON_FRESH_VI("course_ctrl.bg_course", FRCMD_INIT, FrSingleTypeDlg::OnBgCourseInit)
ON_FRESH_VV("course_ctrl.bg_course", FRCMD_OWNERDRAW, FrSingleTypeDlg::OnBgCourseDraw)
ON_FRESH_VI("course_ctrl.fix_hole", FRCMD_INIT, FrSingleTypeDlg::OnFixHoleInit)
ON_FRESH_VV("course_ctrl.fix_hole", FRCMD_LBUTTONUP, FrSingleTypeDlg::OnFixHole)
ON_FRESH_VV("course_ctrl.fix_hole", FRCMD_OWNERDRAW, FrSingleTypeDlg::OnFixHoleDraw)
ON_FRESH_VI("course_ctrl.fix_holecup_pos", FRCMD_INIT, FrSingleTypeDlg::OnFixHolecupPosInit)
ON_FRESH_VV("course_ctrl.fix_holecup_pos", FRCMD_LBUTTONUP, FrSingleTypeDlg::OnFixHolecupPos)
ON_FRESH_VI("chipin_ctrl", FRCMD_INIT, FrSingleTypeDlg::OnChipinCtrlInit)
ON_FRESH_VI("chipin_ctrl.slope", FRCMD_INIT, FrSingleTypeDlg::OnSlopeInit)
ON_FRESH_VV("chipin_ctrl.slope", FRCMD_LBUTTONDOWN, FrSingleTypeDlg::OnSlope)
ON_FRESH_VI("chipin_ctrl.size", FRCMD_INIT, FrSingleTypeDlg::OnSizeInit)
ON_FRESH_VV("chipin_ctrl.size", FRCMD_LBUTTONDOWN, FrSingleTypeDlg::OnSize)
ON_FRESH_VI("chipin_ctrl.ticket", FRCMD_INIT, FrSingleTypeDlg::OnTicketInit)
ON_FRESH_VV("chipin_ctrl.ticket", FRCMD_OWNERDRAW, FrSingleTypeDlg::OnTicketDraw)
ON_FRESH_VI("assist", FRCMD_INIT, FrGameRoomDlg::OnAssistInit)
ON_FRESH_VV("assist", FRCMD_LBUTTONUP, FrGameRoomDlg::OnAssist)
ON_FRESH_VI("assist_tip", FRCMD_INIT, FrGameRoomDlg::OnAssistTipInit)
ON_FRESH_VI("course_ctrl.modify", FRCMD_INIT, FrGameRoomDlg::OnModifyInit)
ON_FRESH_VV("course_ctrl.modify", FRCMD_LBUTTONUP, FrGameRoomDlg::OnModify)
ON_FRESH_VV("close_single", FRCMD_LBUTTONUP, FrSingleTypeDlg::OnCloseSingle)

END_FRESH_MSGMAP()

// 00e0aca8: nome da ordem dos buracos (CSharedDoc+0x1094f; 4 = repetição, sem imagem)
static const char* s_modeName[4] = { "front", "back", "random", "shuffle" };
// 00e0acb8: formato do número do buraco fixo (com dois dígitos, o "!" é o "1" estreito da fonte)
static const char* s_holeFmt[2] = { "%d", "!%d" };

void FrSingleTypeDlg::OnProc(const float deltaTime)
{
	FrGameRoomDlg::OnProc(deltaTime);
	if (g_input->GetDown("ESCAPE", true) == 1)
	{
		Close(FrOK, false);
		g_input->ExclusiveGetDownUseDone("ESCAPE");
	}
}

void FrSingleTypeDlg::Reset983()
{
	ResetRoom983();
	m_pNumFont = NULL;
	memset(m_ticket, 0, sizeof(m_ticket));
	if (m_pNumFont == NULL)
		m_pNumFont = new CSingleNumCourseFont;
	if (m_pHoleFont == NULL)
		m_pHoleFont = new CRepeatHoleFont;
	memset(m_ticket, 0, sizeof(m_ticket));
}

void FrSingleTypeDlg::Release983()
{
	if (m_pNumFont)
	{
		delete m_pNumFont;
		m_pNumFont = NULL;
	}
	ReleaseRoom983();
}

void FrSingleTypeDlg::OnCloseSingle()
{
	Close(FrOK, true);
}

FrSingleTypeDlg::FrSingleTypeDlg()
{
	Reset983();
}

FrSingleTypeDlg::~FrSingleTypeDlg()
{
	Release983();
}

void FrSingleTypeDlg::FadeIn983()
{
	if (m_pBackGround && !(WNDALPHA983(m_pBackGround) > 0.800000011920929))
	{
		float d = (float)(GDI()->GetAlpha() - 1.0);
		if (Abs983(d) < 0.00001f)
			WNDALPHA983(m_pBackGround) = (float)(WNDALPHA983(m_pBackGround) + 0.019999999552965164);
		if (WNDALPHA983(m_pBackGround) > 0.800000011920929)
			WNDALPHA983(m_pBackGround) = 1.0f;
	}
}

void FrSingleTypeDlg::OnModeTitleInit(int param)
{
	m_pModeTitle = DYNAMIC_CAST(FrArea, param);
}

void FrSingleTypeDlg::OnPet0Init(int param)
{
	m_pPet[0] = DYNAMIC_CAST(FrArea, param);
}

void FrSingleTypeDlg::OnCourseCtrlInit(int param)
{
	m_pCourseCtrl = DYNAMIC_CAST(FrGroupBox, param);
}

void FrSingleTypeDlg::OnChipinCtrlInit(int param)
{
	m_pChipinCtrl = DYNAMIC_CAST(FrGroupBox, param);
}

void FrSingleTypeDlg::OnBgCourseInit(int param)
{
	m_pBgCourse = DYNAMIC_CAST(FrArea, param);
}

void FrSingleTypeDlg::OnFixHoleInit(int param)
{
	m_pFixHole = DYNAMIC_CAST(FrButton, param);
	if (m_pFixHole)
		m_pFixHole->SetVisible(false);
}

void FrSingleTypeDlg::OnFixHolecupPosInit(int param)
{
	m_pFixHolecupPos = DYNAMIC_CAST(FrButton, param);
	if (m_pFixHolecupPos)
		m_pFixHolecupPos->SetVisible(false);
}

// inclinação do chip-in: item 0 da lista = ligada
void FrSingleTypeDlg::OnSlope()
{
	int idx = m_pSlope->GetCurrentIdx();
	_gateway::_golfgame::CHoleInOneGameDoc* pDoc;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pDoc);
	pDoc->m_8c = idx == 0;
}

void FrSingleTypeDlg::OnSizeInit(int param)
{
	m_pSize = DYNAMIC_CAST(FrComboCtlEx, param);
}

// tamanho do copo do chip-in: 1 (normal) a 9
void FrSingleTypeDlg::OnSize()
{
	int idx = m_pSize->GetCurrentIdx();
	_gateway::_golfgame::CHoleInOneGameDoc* pDoc;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pDoc);
	pDoc->m_7c = idx + 1;
}

void FrSingleTypeDlg::OnTicketInit(int param)
{
	m_pTicket = DYNAMIC_CAST(FrArea, param);
}

void FrSingleTypeDlg::OnTicketDraw()
{
	FrArea* pArea = m_pTicket;
	if (pArea && m_pNumFont)
		m_pNumFont->Print((float)(pArea->GetRect().x + 31.0), (float)(pArea->GetRect().y + 35.0), m_ticket, TFA_CENTER, 0.0f,
			1.0f);
}

// resultado da janela do buraco: manda a troca se o buraco escolhido for outro (pacote 0x0a, sub-tipo 0x0b)
bool FrSingleTypeDlg::OnHoleSelected(int ret, FrForm* pForm)
{
	if (ret == FrOK)
	{
		FrHoleSelector* pSel = DYNAMIC_CAST(FrHoleSelector, pForm);
		if (pSel)
		{
			unsigned char hole = pSel->GetHole983();
			if (hole != DOCJP()->GetFixHole983())
			{
				WSendPacket packet((enumClientPacket)0x0a);
				packet.Encode2(0xffff);
				packet.Encode1(1);
				packet.Encode1(0x0b);
				packet.Encode1(hole);
				packet.Send((eSendTo)0);
			}
		}
	}
	return true;
}

// "bg_course": número de buracos, tempo em minutos e a imagem da ordem dos buracos
void FrSingleTypeDlg::OnBgCourseDraw()
{
	FrArea* pArea = m_pBgCourse;
	if (pArea == NULL || m_pNumFont == NULL)
		return;

	const char* text = MakeStr("%d", DOC983_U8(0x1094b));
	m_pNumFont->Print((float)(pArea->GetRect().x + 74.0), (float)(pArea->GetRect().y + 8.0), text, TFA_RIGHT, 0.0f, 1.0f);
	text = MakeStr("%d", DOC983_U32(0x10955) / 60000);
	m_pNumFont->Print((float)(pArea->GetRect().x + 195.0), (float)(pArea->GetRect().y + 8.0), text, TFA_RIGHT, 0.0f, 1.0f);

	unsigned char mode = DOC983_U8(0x1094f);
	if (mode == 4)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL || mode >= 4)
		return;
	const Bitmap* pBmp = g_pFresh->GetBitmap(MakeStr("single_txt_%s", s_modeName[mode]));
	if (pBmp)
	{
		WRect rc((float)(pArea->GetRect().x + 96.0), (float)(pArea->GetRect().y + 37.0), (float)pBmp->Width(),
			(float)pBmp->Height());
		pGDI->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
}

// botão do buraco fixo: abre a janela "hole_selector" logo abaixo dele
void FrSingleTypeDlg::OnFixHole()
{
	if (m_pFixHole == NULL)
		return;
	FrHoleSelector* pSel = CreateForm<FrHoleSelector>(g_pFresh->GetManager(), this, "hole_selector", NULL);
	if (pSel == NULL)
		return;

	float h = (float)(m_pFixHole->GetRect().h + 3.0);
	WPoint btn;
	btn.x = m_pFixHole->GetRect().x;
	btn.y = m_pFixHole->GetRect().y;
	WPoint pos;
	pos.x = (float)(btn.x - 68.0);
	pos.y = h + btn.y;
	unsigned char* pCourse = &DOC983_U8(0x10950);
	unsigned char hole = DOCJP()->GetFixHole983();
	pSel->EnableDrag(false);
	pSel->SetCourse983(*pCourse, hole);
	pSel->Open((FRESH_PFN_RESULT)&FrSingleTypeDlg::OnHoleSelected, pos, FrESCAPE | FrNOTAIL | FrRET_CANCEL);
}

void FrSingleTypeDlg::OnFixHoleDraw()
{
	if (m_pFixHole == NULL)
		return;
	if (DOCJP()->GetFixHole983() > 0x12 || m_pHoleFont == NULL)
		return;

	const char* fmt = s_holeFmt[DOCJP()->GetFixHole983() < 10 ? 0 : 1];
	const char* text = MakeStr(fmt, (unsigned char)(DOCJP()->GetFixHole983() % 10));
	m_pHoleFont->Print((float)(m_pFixHole->GetRect().w * 0.5 + m_pFixHole->GetRect().x),
		(float)(m_pFixHole->GetRect().y + 4.0), text, TFA_CENTER, 0.0f, 1.0f);
}

// botão da posição do copo: troca o byte CSharedDoc+0x11609 e avisa o servidor (pacote 0x0a, sub-tipo 0x0c, valor 6 ou 7)
void FrSingleTypeDlg::OnFixHolecupPos()
{
	if (m_pFixHolecupPos == NULL)
		return;

	unsigned long value = 6;
	DOC983_U8(0x11609) = DOC983_U8(0x11609) == 0;
	if (DOC983_U8(0x11609))
		value = 7;
	WSendPacket packet((enumClientPacket)0x0a);
	packet.Encode2(0xffff);
	packet.Encode1(1);
	packet.Encode1(0x0c);
	packet.Encode4(value);
	packet.Send((eSendTo)0);
}

void FrSingleTypeDlg::OnSlopeInit(int param)
{
	m_pSlope = DYNAMIC_CAST(FrComboCtlEx, param);
	if (m_pSlope == NULL || DOC983_U8(0x109b8) == 0x13)
		return;
	_gateway::_golfgame::CHoleInOneGameDoc* pDoc;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pDoc);
	pDoc->m_8c = false;
}

// arruma a sala pelo tipo de jogo: 0x13 = percurso/repetição (CSharedDoc+0x1094f == 4 é a repetição); o resto é o chip-in
void FrSingleTypeDlg::Setup983()
{
	if (m_pBackGround)
		WNDALPHA983(m_pBackGround) = 0.0f;

	if (DOC983_U8(0x109b8) == 0x13)
	{
		if (m_pChipinCtrl)
			m_pChipinCtrl->SetVisible(false);
		if (DOC983_U8(0x1094f) == 4)
		{
			if (m_pModeTitle)
				m_pModeTitle->SetBgImg("single_title_hole_r");
			if (m_pBgCourse)
				m_pBgCourse->SetBgImg("single_map_bg_hole");
			if (m_pFixHole)
				m_pFixHole->SetVisible(true);
			if (m_pFixHolecupPos)
			{
				m_pFixHolecupPos->SetVisible(true);
				Fn503430_983(7);
			}
		}
	}
	else
	{
		if (m_pModeTitle)
			m_pModeTitle->SetBgImg("single_title_chipin_r");
		_gateway::_golfgame::CHoleInOneGameDoc* pDoc;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pDoc);
		if (m_pSlope)
		{
			m_pSlope->AddTexItem("single_txt_apply");
			m_pSlope->AddTexItem("single_txt_cancel");
			m_pSlope->SelectItem(1);
			pDoc->m_8c = false;
		}
		if (m_pSize)
		{
			m_pSize->AddTexItem("single_txt_normalsize");
			m_pSize->AddTexItem("single_num_chipin_x2");
			m_pSize->AddTexItem("single_num_chipin_x3");
			m_pSize->AddTexItem("single_num_chipin_x4");
			m_pSize->AddTexItem("single_num_chipin_x5");
			m_pSize->AddTexItem("single_num_chipin_x6");
			m_pSize->AddTexItem("single_num_chipin_x7");
			m_pSize->AddTexItem("single_num_chipin_x8");
			m_pSize->AddTexItem("single_num_chipin_x9");
			m_pSize->SelectItem(0);
			pDoc->m_7c = 1;
		}

		int count = 0;
		std::list<sDocItem983>::iterator it = DOC983_ITEMS().begin();
		for (; it != DOC983_ITEMS().end(); ++it)
		{
			if (it->m_typeId == 0x1a00017e)
				break;
		}
		if (it != DOC983_ITEMS().end())
			count = it->m_count;
		strncpy(m_ticket, MakeStr("x%d", count), 7);
		if (m_pCourseCtrl)
			m_pCourseCtrl->SetVisible(false);
	}
	SetCourse983(DOC983_U8(0x10950));
}

bool FrSingleTypeDlg::OnInit()
{
	Setup983();
	FrWnd983::SetF1ab38_983();
	if (CLobbyState983::Instance983())
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x59, 0x0b, 0, 0, 0);
	ShowPet(DOC983_U32(0x81a));
	return FrForm::OnInit();
}

// mapa de CSharedDoc+0x3890 (uid → ficha com um objeto em +4). PROVISÓRIO: o tamanho da ficha e a classe do objeto não foram
// levantados (os moldes 00507b90 `find` e 0050cd40 `operator[]` são da unidade da FrGameRoomDlg); só a posição [1] da vtable
// do objeto é usada aqui, com dois floats.
struct IRoomObj983
{
	virtual void V0_983() = 0;
	virtual void V1_983(float a, float b) = 0;
};
// D825: a ficha e o sRoomSlot (0x2dc, m_roomSlotMap); o objeto em +4 e o pExhibition. Antes o mapa era lido com ficha de
// 8 bytes (no de outro tamanho: _Isnil fora do lugar e operator[] inseria no de 8 bytes no mapa real).
inline bool HasRoomEntry983(CSharedDoc* pDoc, unsigned long uid)
{
	return pDoc->m_roomSlotMap.find(uid) != pDoc->m_roomSlotMap.end();
}
inline IRoomObj983* RoomEntryObj983(CSharedDoc* pDoc, unsigned long uid)
{
	return (IRoomObj983*)pDoc->m_roomSlotMap[uid].pExhibition;
}

// 005231b0 (sem chamador no 983): chama a posição [1] do objeto do próprio jogador com (0, 0). O 983 ainda lê o bit 1 de
// CSharedDoc+0x812 e joga fora (resto de uma função embutida); não reproduzido.
void ResetMyRoomObj983()
{
	CSharedDoc* pDoc = Doc();
	unsigned long uid = DOC983_U32(0x81a);
	if (HasRoomEntry983(pDoc, uid))
	{
		if (RoomEntryObj983(pDoc, uid))
			RoomEntryObj983(Doc(), DOC983_U32(0x81a))->V1_983(0.0f, 0.0f);
	}
}
