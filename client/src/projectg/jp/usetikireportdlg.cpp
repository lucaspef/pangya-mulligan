// JP 983: unidade 00afa390-00afa9ec (FrUseTikiReportDlg; classe em include/jp/usetikireportdlg.h). Frente C, sessao C47 (C-PKT
// lote 40): no 983 os envios 0xaa (usar o relatorio do Tiki) e 0xf (sair da partida) que o 645 fazia no CLobbyMain
// (OnRoomUseReportDlgResult / OnRoomExtResQuitDlgResult) moram nesta janela. Mapa de mensagens 00e3c088 (4 entradas; ponteiros
// gravados pelo inicializador 00cf1cb0).
#include "minatl.h"
#include "projectg.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "wlocalize.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "fredit.h"
#include "usetikireportdlg.h"
#include <list>
#include <vector>

C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) + offsetof(sRoomInfo, gameType) == 0x1094c); // A741: era cru Doc()+0x1094c

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrUseTikiReportDlg, FrForm)

// 00afa3a0
FrUseTikiReportDlg::FrUseTikiReportDlg()
	: m_110(0), m_pNotice(NULL), m_pAmount(NULL)
{
}

BEGIN_FRESH_MSGMAP(FrUseTikiReportDlg, FrForm)

ON_FRESH_VI("notice", FRCMD_INIT, FrUseTikiReportDlg::OnNoticeInit)
ON_FRESH_VI("amount", FRCMD_INIT, FrUseTikiReportDlg::OnAmountInit)
ON_FRESH_VV("yes", FRCMD_LBUTTONUP, FrUseTikiReportDlg::OnYes983)
ON_FRESH_VV("no", FRCMD_LBUTTONUP, FrUseTikiReportDlg::OnNo983)

END_FRESH_MSGMAP()

// 00afa400
void FrUseTikiReportDlg::SetNotice983(const char* text)
{
	m_pNotice->AddText(text, false, true, 0, 0);
}

// 00afa420
void FrUseTikiReportDlg::SetAmount983(const char* text)
{
	m_pAmount->AddText(text, true, true, 0, 0);
}

// 00afa520
void FrUseTikiReportDlg::OnNoticeInit(int param)
{
	m_pNotice = DYNAMIC_CAST(FrEdit, param);
}

// 00afa590
void FrUseTikiReportDlg::OnAmountInit(int param)
{
	m_pAmount = DYNAMIC_CAST(FrEdit, param);
}

// 00afa600 (645: CLobbyMain::OnRoomExtResQuitDlgResult): confirmou a saida -> pacote 0xf; com o relatorio de caddie (0x14) em
// sala de tipo 4/5 vao o pang e o bonus do meu sRivalData
bool FrUseTikiReportDlg::OnQuitConfirmResult983(int result, FrForm* form)
{
	if (result == 1)
	{
		FrWnd* pOk = form->FindChildByName("ok");
		if (pOk)
			pOk->Enable(false);

		WSendPacket packet((enumClientPacket)0xf);
		packet.Encode1(0);
		packet.Encode2(0xffff);

		bool bEncoded = false;
		if (IsLocalContent((localContentType_t)0x14))
		{
			unsigned char gameType = Doc()->m_roomInfo.gameType;
			if (gameType == 4 || gameType == 5)
			{
				for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
					it != Doc()->m_rivalList.end(); ++it)
				{
					if (it->oid == MyGuid(false))
					{
						packet.Encode8(it->totalPang);
						packet.Encode8(it->totalBonusPang);
						bEncoded = true;
						break;
					}
				}
			}
		}

		if (!bEncoded)
		{
			packet.Encode8(0);
			packet.Encode8(0);
		}

		packet.Send(TO_GAME);

		Doc()->m_bGameOver = false;
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x1, 0, 0, 0, 0, 0);
		Close(FrNO, true);
	}

	return true;
}

// 00afa790 (645: CLobbyMain::OpenGameExtResQuitForm): "sair no meio do jogo?"
void FrUseTikiReportDlg::OnNo983()
{
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_tiki_sub", NULL);
	pForm->SetMessage(
		K2L_Compatibility("\xbe\xc6\xc1\xf7 \xb0\xd4\xc0\xd3\xc0\xcc \xc1\xf8\xc7\xe0 \xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9.\n\xb4\xd9\xb8\xa5 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xcc \xb8\xf0\xb5\xce \xbf\xcf\xc1\xd6\xc7\xd2 \xb6\xa7\xb1\xee\xc1\xf6\n\xb1\xe2\xb4\xd9\xb7\xc1 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0.\n\n\xc1\xf6\xb1\xdd \xb7\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xb0\xd4 \xb5\xc7\xb8\xe9,\n\\c0xffff0000\\c\xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xc0\xb2\\c0xff000000\\c\xc0\xba \xbf\xc3\xb6\xf3\xb0\xa1\xc1\xf6 \xbe\xca\xc1\xf6\xb8\xb8\n\xb0\xd4\xc0\xd3 \xc1\xdf \xc8\xb9\xb5\xe6\xc7\xd1 \\c0xffff0000\\c\xb0\xe6\xc7\xe8\xc4\xa1, \xbe\xc6\xc0\xcc\xc5\xdb, \xb1\xe2\xb7\xcf \xb5\xee\xc0\xcc\n\xb8\xf0\xb5\xce \xb9\xab\xc8\xbf \xc3\xb3\xb8\xae\\c0xff000000\\c \xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb7\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	pForm->Open((FRESH_PFN_RESULT)&FrUseTikiReportDlg::OnQuitConfirmResult983, 0x801);
}

// 00afa810 (645: CLobbyMain::OnRoomUseReportDlgResult com result 1): gasta um relatorio do Tiki (0x1a000041) e manda as
// estatisticas do buraco (pacote 0xaa)
void FrUseTikiReportDlg::OnYes983()
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); ++it)
	{
		if (it->tid == 0x1a000041)
			break;
	}
	if (it != Doc()->m_myItemList.end() && it->Common[0] > 0)
	{
		if (--it->Common[0] < 1)
			Doc()->m_myItemList.erase(it);
	}

	WSendPacket packet((enumClientPacket)0xaa);
	packet.EncodeBuffer(&Doc()->m_holeStatistics[0], sizeof(sPangYaUserStatistics));
	packet.Send(TO_GAME);

	FrWnd* pYes = FindChildByName("yes");
	if (pYes)
		pYes->Enable(false);

	Doc()->m_bGameOver = false;
	AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x1, 0, 0, 0, 0, 0);
	Close(FrOK, false);
}
