#include "minatl.h"
#include "vc10_layout.h"
#include "frrandomboxprogress.h"
#include "frgaugebar.h"
#include "frstatic.h"
#include "frarea.h"
#include "actor.h"
#include "fresh.h"
#include "projectg.h"
extern Fresh* g_pFresh; // 00f02340
IMPLEMENT_OBJECT(FrRandomBoxProgress, FrForm)

BEGIN_FRESH_MSGMAP(FrRandomBoxProgress, FrForm)

ON_FRESH_VI("waitbar", FRCMD_INIT, FrRandomBoxProgress::OnProgressInit)
ON_FRESH_VI("cancle", FRCMD_LBUTTONUP, FrRandomBoxProgress::OnCancelUp)
ON_FRESH_VI("explane", FRCMD_INIT, FrRandomBoxProgress::OnExplaneStatic_Init)
ON_FRESH_VI("image", FRCMD_INIT, FrRandomBoxProgress::OnImageInit983)

END_FRESH_MSGMAP()

C_ASSERT_VC10(sizeof(FrRandomBoxProgress) == 0x130);

FrRandomBoxProgress::FrRandomBoxProgress()
{
	m_elapsedTime = 0;
	m_pProgress = NULL;
	m_totalTime = 0;
	m_pExplaneStatic = NULL;
	m_pImage983 = NULL;
}

FrRandomBoxProgress::~FrRandomBoxProgress()
{
}

// 983 (008c5350): vazia; o texto do explane nao aparece mais
void FrRandomBoxProgress::SetExplane(const char* text)
{
}

void FrRandomBoxProgress::OnProgressInit(int param)
{
	m_pProgress = DYNAMIC_CAST(FrGaugeBar, param);
	m_pProgress->SetRange(0, 100, 0);
}

void FrRandomBoxProgress::OnCancelUp(int param)
{
	FrForm::OnCancel();
}

void FrRandomBoxProgress::OnExplaneStatic_Init(int param)
{
	m_pExplaneStatic = DYNAMIC_CAST(FrStatic, param);
	if (m_pExplaneStatic)
	{
		m_pExplaneStatic->SetVisible(false);
		m_pExplaneStatic->Enable(false);
	}
}

void FrRandomBoxProgress::OnImageInit983(int param)
{
	m_pImage983 = DYNAMIC_CAST(FrArea, param);
}

// 983 (0060c960, chamado do OnOpenNewRandomBoxDlgResult 00610e00): icone do IFF (Icon +0x49) no "image"; icone mais baixo
// que a area desce a area em (h - altura)/2
void FrRandomBoxProgress::SetBoxTid983(unsigned long tid)
{
	m_boxTid = tid;
	if (m_pImage983 == NULL)
		return;
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);
	if (pItem == NULL)
		return;
	const Bitmap* pIcon = g_pFresh->GetBitmap(pItem->c.Icon);
	if (pIcon == NULL)
		return;
	float iconH = (float)pIcon->Height();
	const WRect& rc = m_pImage983->GetRect();
	if (iconH < rc.h)
		m_pImage983->SetRect(WRect(rc.x, (rc.h - iconH) * 0.5f + rc.y, rc.w, rc.h));
	m_pImage983->SetBgImg(pIcon);
}

// 983 (008c5370): sem o ajuste da barra pelo explane; 3 a 5 segundos
bool FrRandomBoxProgress::OnInit()
{
	srand(GetTickCount());
	m_totalTime = rand() % 3 + 3;

	return true;
}

void FrRandomBoxProgress::OnProc(const float dt)
{
	m_elapsedTime += dt;

	if (m_elapsedTime > m_totalTime)
	{
		m_pProgress->SetPos(100);

		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		WSendPacket send((enumClientPacket)0xef); // JP 983 (s63): 0xf1 no KR 645
		send.Encode4(m_boxTid);
		send.Send(TO_GAME);

		m_elapsedTime = 0;
		m_totalTime = 0;

		FrForm::OnCancel();
	}
	else
	{
		int pos = (int)((m_elapsedTime / m_totalTime) * 100.0f);
		if (pos < 100)
			m_pProgress->SetPos(pos);
	}
}
