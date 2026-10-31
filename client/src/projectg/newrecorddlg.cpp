#include "minatl.h"
#include "newrecorddlg.h"
#include "courseorder.h"
#include "frlistbox.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "commonutil.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"
#include "jp/shareddocjp.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrNewRecordDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrNewRecordDlg, FrForm)

ON_FRESH_VI("courselist", FRCMD_INIT, FrNewRecordDlg::OnCourseListInit)
ON_FRESH_VI("courselist", FRCMD_OWNERDRAW,
	FrNewRecordDlg::OnCourseListOwnerDraw)

END_FRESH_MSGMAP()

void FrNewRecordDlg::OnCourseListInit(int param)
{
	m_pCourseList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pCourseList == NULL)
		return;

	std::map<unsigned int, IFF_STRUCT::sCourse>& courseMap =
		ItemManager()->m_CourseMap;

	for (int i = 0; i < 21; ++i)
	{
		std::map<unsigned int, IFF_STRUCT::sCourse>::iterator it =
			courseMap.find(s_courseOrder[i] | 0x28000000);

		if (it == courseMap.end())
			continue;

		IFF_STRUCT::sCourse* pCourse = &it->second;

		if (pCourse->c.Final == false)
			continue;

		m_pCourseList->AddItem(pCourse);
	}
}

// 983 00731d00 (__fastcall): vetor de estatistica por tipo de partida e lista
static int MapStatList983(unsigned char type, bool second)
{
	switch (type)
	{
	case 0: return second ? 5 : 1;
	case 5: return second ? 4 : 0;
	case 0xa: return second ? 7 : 3;
	case 0xb: return second ? 11 : 9;
	case 0x33: return second ? 6 : 2;
	case 0x34: return second ? 10 : 8;
	}
	return -1;
}

void FrNewRecordDlg::OnCourseListOwnerDraw(int param)
{
	if (param == NULL)
		return;

	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI == NULL)
		return;

	FrListItem* pItem = (FrListItem*)param;
	IFF_STRUCT::sCourse* pCourse = (IFF_STRUCT::sCourse*)pItem->pData;

	if (pCourse == NULL)
		return;

	const Bitmap* pBitmap =
		pManager->GetBitmap("COURSE", MakeStr("small_%s", pCourse->c.Icon));

	if (pBitmap == NULL)
		return;

	int index = pCourse->c.TypeId & 0x3FFFFFF;

	// 983 00ae3b90: o melhor score vem de um dos 12 vetores de 22 mapas (mapStat em diante), indice por
	// 00731d00(tipo, 2a lista): tipo 0x34 na sala 0x14/0x15, senao 0x33 no classico ou 5; 2a lista = 0063bba0.
	// Sem vetor (indice -1) fica 127 (sem recorde).
	bool bClassic = S5::CLASSICSRV::IsClassicGame983() == 1;
	bool bType34 = (unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1;
	bool bSecond = DOCJP()->IsTrajectoryPreviewOn();
	unsigned char type = bType34 ? 0x34 : (bClassic ? 0x33 : 5);

	int bestScore = 127;
	int list = MapStatList983(type, bSecond);
	if (list != -1)
		bestScore = (&Doc()->m_myInfo.mapStat[0])[list * 22 + index].cBestScore;
	WRect rect(pItem->pos.x, pItem->pos.y, (float)pBitmap->Width(),
		(float)pBitmap->Height());

	pGDI->DrawTexture(pBitmap, rect,
		(bestScore >= 0 || bestScore == 127) ? 0x60000000 : 0xFFFFFFFF, 0);
}
