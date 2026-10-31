#include "minatl.h"
#include "vc10_layout.h"
#include "scratchdlg.h"
#include "frarea.h"
#include "frlistbox.h"
#include "frdesktop.h"
#include "frbutton.h"
#include "fredit.h"
#include "frscrollbar.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "wfont.h"
#include "wresrcmng.h"
#include "wview.h"
#include "packet.h"
#include "shareddoc.h"
#include "binstr.h"
#include "../shared/localize.h"


extern Fresh* g_pFresh;

// 983: o float2int fora de linha (00489c80) nao tem chamador; aqui ele e o _ftol embutido
#define float2int(f) ((int)(f))

extern float g_ptScratchItemData[6];
#define g_ptScratchItem ((WPoint*)g_ptScratchItemData)

// ---------------------------------------------------------------- FrScratchDlg

void FrScratchDlg::SetEnableRequestScratchItem(bool bEnable)
{
	m_bEnableRequest = bEnable;
}

IMPLEMENT_OBJECT(FrScratchDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrScratchDlg, FrForm)
ON_FRESH_VI("scratchdlg", FRCMD_INIT, FrScratchDlg::OnScratchDlgInit)
ON_FRESH_VI("scratch_bottom_02", FRCMD_INIT, FrScratchDlg::OnScratchBottomInit)
ON_FRESH_VV("scratch_bottom_02", FRCMD_OWNERDRAW,
	FrScratchDlg::OnScratchBottomOwnerDraw)
ON_FRESH_VI("scratch_cover_01", FRCMD_INIT, FrScratchDlg::OnScratchCover01Init)
ON_FRESH_VV("scratch_cover_01", FRCMD_OWNERDRAW,
	FrScratchDlg::OnScratchCover01OwnerDraw)
ON_FRESH_VI("scratch_cover_02", FRCMD_INIT, FrScratchDlg::OnScratchCover02Init)
ON_FRESH_VV("scratch_cover_02", FRCMD_OWNERDRAW,
	FrScratchDlg::OnScratchCover02OwnerDraw)
ON_FRESH_VI("scratch_card", FRCMD_INIT, FrScratchDlg::OnScratchCardInit)
ON_FRESH_BI("scratch_card", FRCMD_ENTERKEY, FrScratchDlg::OnScratchCardEnterKey)
ON_FRESH_VI("scratch_nextpage", FRCMD_INIT,
	FrScratchDlg::OnScratchNextPageBnInit)
ON_FRESH_VV("scratch_nextpage", FRCMD_LBUTTONUP,
	FrScratchDlg::OnScratchNextPageBnUp)
ON_FRESH_VI("serial_number", FRCMD_INIT, FrScratchDlg::OnSerialNumberInit)
ON_FRESH_BI("serial_number", FRCMD_ENTERKEY,
	FrScratchDlg::OnSerialNumberEnterKey)
ON_FRESH_VI("serial_number_bn", FRCMD_INIT, FrScratchDlg::OnSerialNumberBnInit)
ON_FRESH_VV("serial_number_bn", FRCMD_LBUTTONUP,
	FrScratchDlg::OnSerialNumberBnUp)
ON_FRESH_VI("about", FRCMD_INIT, FrScratchDlg::OnAboutBnInit)
ON_FRESH_VV("about", FRCMD_LBUTTONUP, FrScratchDlg::OnAboutBnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrScratchDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrScratchDlg::OnCloseBnUp)
END_FRESH_MSGMAP()

FrScratchDlg::FrScratchDlg()
{
	m_pCover[0] = NULL;
	m_pCover[1] = NULL;
	m_pScratchCard = NULL;
	m_pNextPage = NULL;
	m_pSerial = NULL;
	m_pSerialBn = NULL;

	static char* textures[1] = { "[font_wind.jpg" };
	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = textures;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);

	m_pCoverBmp[0] = NULL;
	m_pCoverBmp[1] = NULL;
	m_pCloseCover =
		g_pFresh->GetManager()->GetBitmap("SCRATCHDLG", "scratch_cover_close");
	m_pEraser =
		g_pFresh->GetManager()->GetBitmap("SCRATCHDLG", "scratch_eraser");
	m_pMinus =
		g_pFresh->GetManager()->GetBitmap("SCRATCHDLG", "scratch_text_minus");
	m_pPlus =
		g_pFresh->GetManager()->GetBitmap("SCRATCHDLG", "scratch_text_plus");
	// 983: sizeof 0x2d0 (push do operator new em 008ec8b0); campos lidos no construtor 008ec050
	C_ASSERT_VC10(sizeof(FrScratchDlg) == 0x2d0);
	C_ASSERT_VC10(offsetof(FrScratchDlg, m_pItemBmp) == 0x16c && offsetof(FrScratchDlg, m_rcItem) == 0x17c);
	C_ASSERT_VC10(offsetof(FrScratchDlg, m_rcEraser) == 0x18c && offsetof(FrScratchDlg, m_scratchCardCount) == 0x1a8);
	C_ASSERT_VC10(offsetof(FrScratchDlg, m_ball) == 0x1ac && offsetof(FrScratchDlg, m_ballCount) == 0x288);
	C_ASSERT_VC10(offsetof(FrScratchDlg, m_bEnableRequest) == 0x290 && offsetof(FrScratchDlg, m_changePos) == 0x29c);
	C_ASSERT_VC10(offsetof(FrScratchDlg, m_bChecked) == 0x2b0 && offsetof(FrScratchDlg, m_checkRect) == 0x2c0);
	// 983 (008ec050): listas vazias, campos na ordem do construtor do 983
	m_pItemBmp.clear();
	m_rcItem.clear();
	m_bPressed = false;
	m_scratchCardCount = 0;
	m_rcEraser = WRect(0.0f, 0.0f, 0.0f, 0.0f);
	m_ptOffset = WPoint(-10.0f, -10.0f);
	ClearScratchItem();
	m_changePos[0] = 0;
	m_changePos[1] = 0;
	m_resultTimer = 0;
	m_response = -1;
	m_bEnableRequest = true;
	m_bScratchEnabled = false;
	m_bSerialEnabled = true;
	m_bResultPending = false;
	m_changeIndex[0] = -1;
	m_changeIndex[1] = -1;
	m_changeAlpha[0] = 0;
	m_changeAlpha[1] = 0;
	ClearDataCheckScratchItem();

	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		WSendPacket send((enumClientPacket)0xc1);
		send.Encode1(2);
		send.Send(TO_GAME);
	}
}

FrScratchDlg::~FrScratchDlg()
{
	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/) &&
		WNetworkSystem::Instance() != NULL)
	{
		WSendPacket send((enumClientPacket)0xc1);
		send.Encode1(1);
		send.Send(TO_GAME);
	}
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
	OpenScratchResultDlg();
}

void FrScratchDlg::OnScratchDlgInit(int param)
{
}

void FrScratchDlg::OnScratchBottomInit(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrScratchDlg::OnScratchBottomOwnerDraw()
{
	DrawScratchItem();
	DrawChangeScratchCardNumber();
}

void FrScratchDlg::OnScratchCover01Init(int param)
{
	m_pCover[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pCover[0])
	{
		m_pCoverBmp[0] = m_pCover[0]->GetBgImg();
		ResetBmpScratchCover(0);
	}
}

void FrScratchDlg::OnScratchCover01OwnerDraw()
{
}

void FrScratchDlg::OnScratchCover02Init(int param)
{
	m_pCover[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pCover[1])
	{
		m_pCoverBmp[1] = m_pCover[1]->GetBgImg();
		ResetBmpScratchCover(1);
	}
}

void FrScratchDlg::OnScratchCover02OwnerDraw()
{
}

void FrScratchDlg::OnScratchCardInit(int param)
{
	m_pScratchCard = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrScratchDlg::OnScratchCardEnterKey(int param)
{
	return false;
}

void FrScratchDlg::OnScratchNextPageBnInit(int param)
{
	m_pNextPage = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNextPage)
		m_pNextPage->SetVisible(false);
}

void FrScratchDlg::OnScratchNextPageBnUp()
{
	// 983 (008ece00): o resultado sai pelo SetBmpResultText (4 = itens, 5 = nada)
	if (m_pNextPage)
	{
		m_pNextPage->Enable(false);
		m_bScratchEnabled = false;
		if (m_ball[0].typeId == 0)
		{
			SetBmpResultText(5);
			return;
		}
		SetBmpResultText(4);
		WPoint pos(0.0f, 0.0f);
		OnLButtonUp(pos);
	}
}

void FrScratchDlg::OnSerialNumberInit(int param)
{
	m_pSerial = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrScratchDlg::OnSerialNumberEnterKey(int param)
{
	return false;
}

void FrScratchDlg::OnSerialNumberBnInit(int param)
{
	m_pSerialBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrScratchDlg::OnSerialNumberBnUp()
{
	if (m_pSerialBn)
	{
		SendScratchSerialNumber();
		EnableScratchSerialNumber(false);
	}
}

void FrScratchDlg::OnAboutBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrScratchDlg::OnAboutBnUp()
{
	FrScratchAboutDlg* pDlg = CreateForm<FrScratchAboutDlg>(
		g_pFresh->GetManager(), this, "scratch_about", NULL);
	if (pDlg)
		pDlg->Open(NULL, 3);
}

void FrScratchDlg::OnCloseBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrScratchDlg::OnCloseBnUp()
{
	OnFreshOkay();
}

bool FrScratchDlg::OnInit()
{
	if (m_pEraser && m_pCover[0] && m_pCover[1])
	{
		m_rcEraser.x = (float)float2int(
			m_pCover[0]->GetRect().x - (m_pEraser->Width() >> 1));
		m_rcEraser.y = (float)float2int(
			m_pCover[0]->GetRect().y - (m_pEraser->Height() >> 1));
		m_rcEraser.w = (float)(m_pCoverBmp[0]->Width() + m_pEraser->Width() +
			m_pCoverBmp[1]->Width());
		m_rcEraser.h = (float)(m_pEraser->Height() + m_pCoverBmp[1]->Height());
	}
	InitScratchCard();
	SetBmpColsedScratchCover();
	return true;
}

void FrScratchDlg::OnProc(const float deltaTime)
{
	ProcessChangeScratchCardNumber(deltaTime);
}

void FrScratchDlg::OnMouseMove(const WPoint& pos)
{
	if (m_bPressed)
	{
		RequestScratchItem();
		OpenScratchCover(pos);
	}
}

bool FrScratchDlg::OnLButtonDown(const WPoint& pos)
{
	FrForm::OnLButtonDown(pos);
	if (m_pCover[0] && m_pCover[1])
	{
		WRect rect(m_pCover[0]->GetRect().x + 10.0f,
			m_pCover[0]->GetRect().y + 10.0f,
			m_bmpCover[1].Width() + m_bmpCover[0].Width() - 20.0f,
			m_bmpCover[0].Height() - 20.0f);
		if (rect.IsInRect(pos))
			m_bPressed = true;
	}
	return false;
}

bool FrScratchDlg::OnLButtonUp(const WPoint& pos)
{
	FrForm::OnLButtonUp(pos);
	m_bPressed = false;
	return false;
}

void FrScratchDlg::OnCancel()
{
}

void FrScratchDlg::InitScratchCard()
{
	m_scratchCardCount = 0;
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
		it++)
	{
		if (ItemManager()->GetCouponKind((*it).tid) == 3)
			m_scratchCardCount += (short)(*it).Common[0];
	}
	if (m_pScratchCard)
		m_pScratchCard->SetLine(1, MakeStr("%d", m_scratchCardCount), 0, false,
			0);
}

void FrScratchDlg::SetBmpColsedScratchCover()
{
	if (!m_bScratchEnabled)
	{
		if (m_scratchCardCount == 0)
		{
			memcpy(m_bmpCover[0].vram, m_pCloseCover->vram,
				m_pCloseCover->Height() * m_pCloseCover->pitch);
			m_pCover[0]->SetBgImg(&m_bmpCover[0]);
			GDI()->UpdateTextureCacheInfo(&m_bmpCover[0]);
			ResetBmpScratchCover(1);
			return;
		}
		ResetBmpScratchCover(0);
		ResetBmpScratchCover(1);
	}
}

void FrScratchDlg::ResetBmpScratchCover(int index)
{
	if (m_pCoverBmp[index])
	{
		if (m_bmpCover[index].bi == NULL)
			m_bmpCover[index].Create(m_pCoverBmp[index]->Width(),
				m_pCoverBmp[index]->Height(),
				m_pCoverBmp[index]->BitsPerPixel());
		memcpy(m_bmpCover[index].vram, m_pCoverBmp[index]->vram,
			m_pCoverBmp[index]->Height() * m_pCoverBmp[index]->pitch);
		m_pCover[index]->SetBgImg(&m_bmpCover[index]);
		GDI()->UpdateTextureCacheInfo(&m_bmpCover[index]);
	}
}

void FrScratchDlg::OpenScratchCover(const WPoint& pos)
{
	if (m_bScratchEnabled && m_pCover && m_pEraser && m_rcEraser.IsInRect(pos))
	{
		bool bChanged[2];
		bChanged[0] = false;
		bChanged[1] = false;
		for (unsigned y = 0; y < m_pEraser->Height(); y++)
		{
			for (unsigned x = 0; x < m_pEraser->Width(); x++)
			{
				if (((Bitmap*)m_pEraser)->GetAlpha(x, y) != 0)
				{
					bool bDone = false;
					for (int i = 0; i < 2; i++)
					{
						if (!bDone)
						{
							int px =
								(int)(((float)(int)x + pos.x -
											   m_pCover[i]->GetRect().x) +
									m_ptOffset.x);
							int py =
								(int)(((float)(int)y + pos.y -
											   m_pCover[i]->GetRect().y) +
									m_ptOffset.y);
							if (px >= 0 && (unsigned)px < m_bmpCover[i].Width() &&
								py >= 0 && (unsigned)py < m_bmpCover[i].Height())
							{
								m_bmpCover[i].SetAlpha(px, py, 0);
								bChanged[i] = true;
								bDone = true;
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < 2; i++)
		{
			if (bChanged[i])
				GDI()->UpdateTextureCacheInfo(&m_bmpCover[i]);
		}
	}
}

void FrScratchDlg::RequestScratchItem()
{
	if (m_scratchCardCount != 0 && m_bEnableRequest)
	{
		WSendPacket send((enumClientPacket)0x70);
		send.Send(TO_GAME);
		m_bEnableRequest = false;
		EnableScratchSerialNumber(false);
	}
}

void FrScratchDlg::EnableNextPageButton(bool bEnable)
{
	if (m_pNextPage)
	{
		m_pNextPage->SetVisible(bEnable);
		m_pNextPage->Enable(true);
	}
}

void FrScratchDlg::EnableScratchSerialNumber(bool bEnable)
{
	if (m_pSerialBn)
		m_pSerialBn->Enable(bEnable);
	if (m_pSerial)
	{
		m_pSerial->ClearLine();
		m_pSerial->Enable(bEnable);
		m_pSerial->SetKeyFocus(bEnable);
		m_pSerial->SetViewFocus(bEnable);
	}
	m_bSerialEnabled = bEnable;
}

void FrScratchDlg::SendScratchSerialNumber()
{
	if (0)
		((FrScratchDlg*)0)->FrScratchDlg::OnScratchNextPageBnUp();
	if (m_pSerial)
	{
		const char* serial = m_pSerial->GetLine(1, false);
		if (strlen(serial) == 13)
		{
			WSendPacket send((enumClientPacket)0x71);
			send.Encode4(13);
			send.EncodeBuffer(serial, 13);
			send.Send(TO_GAME);
			m_bSerialEnabled = false;
			return;
		}
		SetBmpResultText(0);
	}
}

void FrScratchDlg::ResponseScratchSerialNumber(int result, unsigned long guid)
{
	// 983 (008ecf20): os avisos de erro saem pelo SetBmpResultText
	m_response = result;
	switch (result)
	{
	case 0:
	{
		sBonusBall ball;
		ball.unused0 = 0;
		ball.unused10 = 0;
		ball.kind = 0;
		ball.value = 0;
		ball.typeId = 0x1a000030;
		ball.guid = guid;
		ball.count = 1;
		UpdateListMyItem(&ball);
		InitScratchCard();
		SetBmpColsedScratchCover();
		return;
	}
	case 1:
		SetBmpResultText(1);
		return;
	default:
		SetBmpResultText(0);
		return;
	case 3:
		SetBmpResultText(2);
		return;
	case 4:
		SetBmpResultText(3);
		return;
	}
}

void FrScratchDlg::SetScratchItem(int count, sBonusBall* pBall)
{
	// 983 (008ecfe0): sem o UpdateListMyItem de cada item (o 983 so desconta o cartao no ResponseScratchSerialNumber)
	if (pBall)
	{
		memcpy(m_ball, pBall, count * sizeof(sBonusBall));
		m_ballCount = count;
		m_bScratchEnabled = true;
		InitScratchCard();
		SetBmpScratchItem();
		SetRectScratchItem();
		SetRectCheckScratchItemRect();
	}
}

void FrScratchDlg::ClearScratchItem()
{
	memset(m_ball, 0, sizeof(m_ball));
	m_ballCount = 0;
}

void FrScratchDlg::SetBmpResultText(int index)
{
	if (index < 0 || index >= 6)
	{
		m_bResultPending = false;
		return;
	}
	if (index == 4)
	{
		OpenScratchResultDlg();
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		if (pForm)
		{
			pForm->Open((FRESH_PFN_RESULT)&FrScratchDlg::OnConfirmResult, 0xc0);
			pForm->SetFadeout(false);
			switch (index)
			{
			case 0:
				pForm->SetMessage(
					K2L_Compatibility("\\c0xffff0000\\c\xbd\xc3\xb8\xae\xbe\xf3 \xb9\xf8\xc8\xa3\\c0xff000000\\c\xb0\xa1 \xbf\xc3\xb9\xd9\xb8\xa3\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 1:
				pForm->SetMessage(
					K2L_Compatibility("\xc0\xcc\xb9\xcc \xbb\xe7\xbf\xeb\xc7\xd1 \\c0xffff0000\\c\xbd\xc3\xb8\xae\xbe\xf3 \xb9\xf8\xc8\xa3\\c0xff000000\\c\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 2:
				pForm->SetMessage(
					K2L_Compatibility("\\c0xffff0000\\c\xc0\xaf\xc8\xbf \xb1\xe2\xb0\xa3\\c0xff000000\\c\xc0\xcc \xc1\xf6\xb3\xad \xc4\xab\xb5\xe5\xc0\xc7 \xbd\xc3\xb8\xae\xbe\xf3 \xb9\xf8\xc8\xa3\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
				break;
			case 3:
				pForm->SetMessage(
					K2L_Compatibility("\xc0\xcc\xba\xa5\xc6\xae\xb7\xce \xb9\xde\xc0\xb8\xbd\xc5 \xc4\xed\xc6\xf9\xc0\xba \xc0\xaf\xc8\xbf\xb1\xe2\xb0\xa3 \xb3\xbb\xbf\xa1 \xc7\xd1 \xb0\xe8\xc1\xa4\xb4\xe7 \xc3\xd6\xb4\xeb 5\xb0\xb3\xb1\xee\xc1\xf6\xb8\xb8 \xbb\xe7\xbf\xeb\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9. \xc0\xcc\xb9\xcc \xc4\xed\xc6\xf9\xc0\xbb \xb8\xf0\xb5\xce \xbb\xe7\xbf\xeb\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				break;
			default:
				pForm->SetMessage(K2L_Compatibility("\xbf\xc0\xb7\xf9"), false);
				break;
			case 5:
				pForm->SetMessage(
					K2L_Compatibility("\xb4\xd9\xc0\xbd \xb1\xe2\xc8\xb8\xbf\xa1..."), false);
				break;
			}
		}
	}
	m_resultTimer = 0;
	m_bResultPending = true;
}

void FrScratchDlg::ClearDataCheckScratchItem()
{
	// 983 (008ebdd0): esvazia os quadrantes de cada item e depois as listas
	if (!m_bChecked.empty())
	{
		for (std::vector<std::vector<bool> >::iterator it = m_bChecked.begin(), e = m_bChecked.end(); it != e; ++it)
		{
			if (!it->empty())
				it->clear();
		}
		m_bChecked.clear();
	}
	if (!m_checkRect.empty())
	{
		for (std::vector<std::vector<WRect> >::iterator it = m_checkRect.begin(), e = m_checkRect.end(); it != e; ++it)
			it->clear();
		m_checkRect.clear();
	}
}

void FrScratchDlg::ProcessResultText(float deltaTime)
{
	if (m_response != 0 && m_bResultPending)
	{
		m_resultTimer += deltaTime;
		if (m_resultTimer > 2.0f)
		{
			m_resultTimer = 0;
			m_bResultPending = false;
			EnableScratchSerialNumber(true);
			EnableNextPageButton(false);
			if (!m_bEnableRequest)
			{
				ClearDataCheckScratchItem();
				ClearScratchItem();
				SetBmpColsedScratchCover();
				m_bEnableRequest = true;
			}
		}
	}
}

void FrScratchDlg::SetBmpScratchItem()
{
	// 983 (008e9670): uma imagem por item; itens 0x24 (todos) vem primeiro; icone que falta = "equipitem_itemslot_empty"
	m_pItemBmp.clear();
	if (m_ballCount <= 0)
	{
		m_pItemBmp.push_back(g_pFresh->GetBitmap("scratch_text_nothing"));
		return;
	}
	unsigned long typeIds[11] = { 0 };
	int setCount = 0;
	for (int i = 0; i < 11; i++)
	{
		// o 983 guarda na posicao do item (i), nao na posicao contada
		if ((m_ball[i].typeId & 0xfc000000) == 0x24000000)
		{
			setCount++;
			typeIds[i] = m_ball[i].typeId;
		}
	}
	if (setCount > 0)
	{
		for (int k = 0; k < setCount; k++)
		{
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeIds[k]);
			if (g_pFresh->GetBitmap(pItem->Icon) == NULL)
				m_pItemBmp.push_back(g_pFresh->GetBitmap("equipitem_itemslot_empty.tga"));
			else
				m_pItemBmp.push_back(g_pFresh->GetBitmap(pItem->Icon));
		}
		m_ballCount = setCount;
		return;
	}
	if (m_ballCount <= 0)
		return;
	int i = 0;
	sBonusBall* pBall = m_ball;
	while (pBall->typeId != 0)
	{
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(pBall->typeId);
		if (pItem)
		{
			if (g_pFresh->GetBitmap(pItem->Icon) == NULL)
				m_pItemBmp.push_back(g_pFresh->GetBitmap("equipitem_itemslot_empty"));
			else
				m_pItemBmp.push_back(g_pFresh->GetBitmap(pItem->Icon));
		}
		if ((pBall->typeId & 0xfc000000) == 0x24000000)
		{
			m_ballCount = 1;
			return;
		}
		i++;
		pBall++;
		if (m_ballCount <= i)
			return;
	}
	m_pItemBmp.push_back(g_pFresh->GetBitmap("scratch_text_nothing"));
}

// 983 (008e6820): canto do primeiro item; os itens ficam em fila, 64 px um do outro, centrados no dialogo
void FrScratchDlg::GetItemStartPos983(WPoint* pPos)
{
	WPoint center((float)(int)(m_rect.w * 0.5 + m_rect.x), (float)(int)(m_rect.h * 0.5 + m_rect.y));
	*pPos = WPoint(center.x - m_ballCount * 32.0, center.y - 40.0);
}

void FrScratchDlg::SetRectScratchItem()
{
	// 983 (008eafc0)
	m_rcItem.clear();
	if (m_pItemBmp.at(0))
	{
		WPoint pos(0.0f, 0.0f);
		GetItemStartPos983(&pos);
		for (int i = 0; i < m_ballCount; i++)
		{
			float off = 12.0f;
			if ((m_ball[i].typeId & 0xfc000000) != 0x18000000)
				off = 0.0f;
			const Bitmap* pBmp = m_pItemBmp.at(i);
			m_rcItem.push_back(WRect(pos.x + off,
				(pos.y - (float)(pBmp->Height() >> 1)) + off,
				(float)(int)pBmp->Width(), (float)(int)pBmp->Height()));
			pos.x += 64.0;
		}
	}
}

void FrScratchDlg::SetRectCheckScratchItemRect()
{
	// 983 (008ecc20): refaz as listas; um conjunto de 4 quadrantes por item
	ClearDataCheckScratchItem();
	if (m_pItemBmp[0])
	{
		WPoint off(24.0f, 24.0f);
		if (m_ballCount < 2)
		{
			if ((m_ball[0].typeId & 0xfc000000) != 0x18000000)
			{
				off.x = 0;
				off.y = 0;
			}
			SetRectCheckScratchItemRectByIndex(0, off);
			return;
		}
		for (int i = 0; i < m_ballCount; i++)
			SetRectCheckScratchItemRectByIndex(i, WPoint(24.0f, 24.0f));
	}
}

// 983: WRect zerado do .bss (00f14668) que varias unidades copiam como "retangulo vazio" (ver recycledlg.cpp)
static WRect s_rcEmpty983;

void FrScratchDlg::SetRectCheckScratchItemRectByIndex(int index, WPoint offset)
{
	// 983 (008ec440): acrescenta os 4 quadrantes do item; o 983 os poe todos no canto do 1o item (GetItemStartPos983)
	WRect cell = s_rcEmpty983;
	WRect rc = m_rcItem.at(index);
	cell.w = (float)(int)((rc.w - offset.x) * 0.25);
	cell.h = (float)(int)((rc.h - offset.y) * 0.25);
	std::vector<WRect> quad;
	for (int i = 0; i < 4; i++)
		quad.push_back(cell);
	m_checkRect.push_back(quad);
	WPoint pos(0.0f, 0.0f);
	GetItemStartPos983(&pos);
	m_checkRect[index][0].x = pos.x - m_checkRect[index][0].w;
	m_checkRect[index][0].y = pos.y - m_checkRect[index][0].h;
	m_checkRect[index][1].x = pos.x;
	m_checkRect[index][1].y = pos.y - m_checkRect[index][1].h;
	m_checkRect[index][2].x = pos.x - m_checkRect[index][2].w;
	m_checkRect[index][2].y = pos.y;
	m_checkRect[index][3].x = pos.x;
	m_checkRect[index][3].y = pos.y;
}

void FrScratchDlg::CheckScratchItemRect(const WPoint& pos)
{
	if (m_bScratchEnabled)
	{
		if (m_ballCount < 2)
		{
			for (int i = 0; i < 4; i++)
			{
				const WRect& rc = m_checkRect[2][i];
				if (rc.x <= pos.x && pos.x <= rc.w + rc.x && rc.y <= pos.y &&
					pos.y <= rc.h + rc.y)
				{
					m_bChecked[0][i] = true;
					return;
				}
			}
			return;
		}
		for (int i = 0; i < m_ballCount; i++)
		{
			for (int k = 0; k < 4; k++)
			{
				const WRect& rc = m_checkRect[i][k];
				if (rc.x <= pos.x && pos.x <= rc.w + rc.x && rc.y <= pos.y &&
					pos.y <= rc.h + rc.y)
				{
					m_bChecked[i][k] = true;
					break;
				}
			}
		}
	}
}

void FrScratchDlg::ProcessChangeScratchCardNumber(float deltaTime)
{
	int dAlpha = (int)(__int64)(deltaTime * 480.0f);
	float dPos = deltaTime * 20.0f;
	int i = 0;
	do
	{
		m_changeAlpha[i] = m_changeAlpha[i] - dAlpha;
		m_changePos[i] = m_changePos[i] - dPos;
		i++;
	} while (i < 2);
}

void FrScratchDlg::InitChangeScratchCardNumber(int index)
{
	if (index >= 0 && index < 2)
	{
		int slot = 0;
		if (m_changePos[0] > m_changePos[1])
			slot = 1;
		m_changeIndex[slot] = index;
		m_changePos[slot] = 0;
		m_changeAlpha[slot] = 512;
	}
}

void FrScratchDlg::DrawScratchItem()
{
	// 983 (008e7c90): todos os itens da lista, com a quantidade no canto
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI && !m_pItemBmp.empty() && !m_rcItem.empty())
	{
		if (m_ballCount < 2)
		{
			const Bitmap* pBmp = m_pItemBmp.at(0);
			WRect rc = s_rcEmpty983;
			rc = m_rcItem.at(0);
			pGDI->DrawTexture(pBmp, rc, 0xffffffff, 0);
			if (m_ball[0].count > 1)
			{
				const char* str = MakeStr("%d", m_ball[0].count);
				float x = rc.x + 40.0;
				for (unsigned i = 0; i < strlen(str); i++)
					x -= m_pFont->GetCharWidth(g_view, str[i] - '0');
				m_pFont->Print(g_view, x, rc.y + 30.0, str, 0, 0xffffffff, NULL);
			}
			return;
		}
		for (int i = 0; i < m_ballCount; i++)
		{
			const Bitmap* pBmp = m_pItemBmp.at(i);
			WRect rc = s_rcEmpty983;
			rc = m_rcItem.at(i);
			if (pBmp)
				pGDI->DrawTexture(pBmp, rc, 0xffffffff, 0);
			if (m_ball[i].count > 1)
			{
				const char* str = MakeStr("%d", m_ball[i].count);
				float x = m_rcItem[i].x + 40.0;
				for (unsigned k = 0; k < strlen(str); k++)
					x -= m_pFont->GetCharWidth(g_view, str[k] - '0');
				m_pFont->Print(g_view, x, rc.y + 30.0, str, 0, 0xffffffff, NULL);
			}
		}
	}
}

void FrScratchDlg::DrawChangeScratchCardNumber()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI)
	{
		for (int i = 0; i < 2; i++)
		{
			if (m_changeIndex[i] >= 0 &&
				(&m_pEraser)[1 + m_changeIndex[i]] != NULL)
			{
				unsigned alpha;
				if (m_changeAlpha[i] < 0)
				{
					alpha = 0;
					m_changeIndex[i] = -1;
				}
				else if (m_changeAlpha[i] < 256)
					alpha = (unsigned char)m_changeAlpha[i];
				else
					alpha = 0xff;
				const Bitmap* pBmp = (&m_pEraser)[1 + m_changeIndex[i]];
				// 983 008e6710: x + 348.0 (d17888; o 645 tinha 350), y + 113.0 + pos
				WRect rc(m_rect.x + 348.0, m_rect.y + 113.0 + m_changePos[i],
					pBmp->Width(), pBmp->Height());
				pGDI->DrawTexture(pBmp, rc, alpha << 24 | 0xffffff, 0);
			}
		}
	}
}

void FrScratchDlg::OpenScratchResultDlg()
{
	// 983 (008ec7e0): com um item so, o dialogo de resultado fica 70 px mais baixo; resposta 0xc2
	if (m_ball[0].typeId != 0)
	{
		FrScratchResultDlg* pDlg = CreateForm<FrScratchResultDlg>(
			g_pFresh->GetManager(), this, "scratchresultdlg", NULL);
		if (pDlg)
		{
			if (m_ballCount < 2)
				pDlg->SetRect(WRect(pDlg->GetRect().x, pDlg->GetRect().y,
					pDlg->GetRect().w, pDlg->GetRect().h - 70.0));
			pDlg->Open((FRESH_PFN_RESULT)&FrScratchDlg::OnConfirmResult, 0xc2);
			pDlg->SetFadeout(false);
			pDlg->SetResultItem(m_ball, m_ballCount);
		}
	}
}

bool FrScratchDlg::OnConfirmResult(int result, FrForm* form)
{
	if (result != 1 && result != 2)
		return true;
	FrScratchDlg* pDlg =
		(FrScratchDlg*)g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"scratchdlg");
	if (pDlg == NULL)
		return true;
	// 983 (008ebf60): pelo EnableNextPageButton e sem zerar m_scratchCardCount
	pDlg->EnableScratchSerialNumber(true);
	pDlg->EnableNextPageButton(false);
	pDlg->ClearDataCheckScratchItem();
	pDlg->ClearScratchItem();
	pDlg->SetBmpColsedScratchCover();
	pDlg->m_bEnableRequest = true;
	return true;
}

void FrScratchDlg::ProcessCheckScratchItemRect(float deltaTime)
{
	// 983 (008ecce0): 3 de 4 quadrantes raspados por item; o resultado sai pelo SetBmpResultText (4 = itens, 5 = nada)
	if (m_bScratchEnabled && !m_bResultPending)
	{
		int count = 0;
		if (m_ballCount < 2)
		{
			for (int i = 0; i < 4; i++)
			{
				if (m_bChecked[0][i])
					count++;
			}
			if (count < 3)
				return;
		}
		else
		{
			for (int i = 0; i < m_ballCount; i++)
			{
				for (int k = 0; k < 4; k++)
				{
					if (m_bChecked[i][k])
						count++;
				}
			}
			if (count < m_ballCount * 3)
				return;
		}
		m_bScratchEnabled = false;
		if (m_ball[0].typeId == 0)
		{
			SetBmpResultText(5);
			return;
		}
		SetBmpResultText(4);
		WPoint pos(0.0f, 0.0f);
		OnLButtonUp(pos);
	}
}

void FrScratchDlg::UpdateListMyItem(sBonusBall* pBall)
{
	if (pBall == NULL)
		return;
	bool bNew = true;
	std::list<sItemInfo>* pList = NULL;
	switch (pBall->typeId >> 26)
	{
	case 2:
	case 6:
		pList = &Doc()->m_myItemList;
		break;
	default:
		goto skip;
	case 5:
		pList = &Doc()->m_ballList;
		break;
	case 8:
		pList = &Doc()->m_cadItemList;
		break;
	case 9:
		return; // 983 008ea3b3: caso 9 vai direto ao fim (o 645 gravava m_ballCount = 1)
	case 0xe:
		pList = &Doc()->m_cutinList;
		break;
	case 0x1c:
		pList = &Doc()->m_auxPartList;
		break;
	}
	if (pList)
	{
		std::list<sItemInfo>::iterator it;
		for (it = pList->begin(); it != pList->end(); it++)
		{
			if ((*it).tid == pBall->typeId && (*it).ItemType == 0 &&
				pBall->kind == 0)
			{
				(*it).Common[0] += (short)pBall->count;
				bNew = false;
			}
		}
	}
skip:
	std::map<unsigned int, sCaddieInfo>::iterator itC;
	for (itC = Doc()->m_caddieMap.begin(); itC != Doc()->m_caddieMap.end(); ++itC)
	{
		if ((*itC).second.tid == pBall->typeId)
		{
			bNew = false;
			break;
		}
	}
	std::map<unsigned int, sItemInfo>::iterator itS;
	for (itS = Doc()->m_clubSetMap.begin(); itS != Doc()->m_clubSetMap.end();
		++itS)
	{
		if ((*itS).second.guid == pBall->guid)
		{
			bNew = false;
			break;
		}
	}
	// 983 008ea4c0: mascote ja na lista (mesmo guid) = nada a fazer
	std::map<unsigned int, sMascotInfo>::iterator itM;
	for (itM = Doc()->m_mascotMap.begin(); itM != Doc()->m_mascotMap.end();
		++itM)
	{
		if ((*itM).second.guid == pBall->guid)
			return;
	}
	if (bNew)
	{
		unsigned long tid = pBall->typeId;
		// HACK: equivalent unsigned group extraction avoids sharing the shifted
		// value with the later switch, preserving the original register lifetimes.
		unsigned long group = ((long)tid >> 26) & 0x3f;
		unsigned long key;
		if (group == 7)
		{
			key = pBall->guid;
			sCaddieInfo info;
			memset(&info, 0, sizeof(info));
			info.guid = key;
			info.tid = tid;
			info.Remain_Date = 0x1e;
			info.gift_flag = 0;
			info.Rent_flag = 0;
			info.Purchase = 0;
			Doc()->m_caddieMap[key] = info;
			return;
		}
		if (group == 0x10)
		{
			// 983 008ea5a0: mascote ganho na raspadinha entra em m_mascotMap
			key = pBall->guid;
			sMascotInfo info;
			memset(&info, 0, sizeof(info));
			info.guid = key;
			info.tid = tid;
			info.Remain_Date = 0;
			info.Purchase = 0;
			Doc()->m_mascotMap[key] = info;
			return;
		}
		if (group == 4)
		{
			sItemInfo info;
			info.guid = pBall->guid;
			info.tid = tid;
			key = pBall->guid;
			Doc()->m_clubSetMap[key] = info;
			return;
		}
		if (group == 0x12)
		{
			unsigned long furnitureId = pBall->guid;
			IFF_STRUCT::sFurniture* pFurn =
				Doc()->m_itemManager.FindFurniture(tid);
			if (pFurn)
			{
				sFurniture_List list;
				memset(&list, 0, sizeof(list));
				list.id = furnitureId;
				list.typeId = tid;
				list.x = pFurn->Pos[0];
				list.y = pFurn->Pos[1];
				list.z = pFurn->Pos[2];
				list.r = pFurn->Pos[3];
				list.bArrange = 0;
				Doc()->AddFurniture(list);
			}
			return;
		}
		else
		{
			sItemInfo info;
			short common[5] = { 1, 0, 0, 0, 0 };
			if (pBall->kind == 0)
				common[0] = pBall->count;
			else if (pBall->kind == 1 || pBall->kind == 2)
			{
				common[0] = pBall->value;
				common[4] = common[0];
			}
			info.guid = pBall->guid;
			info.tid = tid;
			memcpy(info.Common, common, sizeof(common));
			info.Purchase = 0;
			info.gift_flag = 0;
			info.Expired = 0; // 983 008ea370: and 0x8c limpa os bits 0 e 1
			info.ItemType = pBall->kind & 7;
			if (info.ItemType != 0)
			{
				info.Common[0] = 1;
				if (info.ItemType == 3)
					info.HourRemain = pBall->value;
				else if (info.ItemType == 4)
					info.HourRemain = pBall->value * 0x18;
				else if (info.ItemType == 5)
					info.HourRemain = pBall->value * 0x2d0;
			}
			switch (info.tid >> 26)
			{
			case 2:
				Doc()->AddMyPartsList(info);
				return;
			default:
				return;
			case 5:
				Doc()->m_ballList.push_back(info);
				break;
			case 6:
				Doc()->m_myItemList.push_back(info);
				break;
			case 8:
				Doc()->m_cadItemList.push_back(info);
				break;
			case 9:
				Doc()->m_setItemList.push_back(info);
				break;
			case 0xe:
				Doc()->m_cutinList.push_back(info);
				break;
			case 0x1c:
				Doc()->m_auxPartList.push_back(info);
				break;
			}
		}
	}
}

// ---------------------------------------------------------- FrScratchResultDlg

IMPLEMENT_OBJECT(FrScratchResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrScratchResultDlg, FrForm)
ON_FRESH_VI("scratchresultdlg", FRCMD_INIT,
	FrScratchResultDlg::OnScratchResultDlgInit)
ON_FRESH_VI("resultitem_icon", FRCMD_INIT,
	FrScratchResultDlg::OnResultItemIconInit)
ON_FRESH_VI("resultitem_icon", FRCMD_OWNERDRAW,
	FrScratchResultDlg::OnResultItemIconOwnerDraw)
ON_FRESH_VI("resultitem_name", FRCMD_INIT,
	FrScratchResultDlg::OnResultItemNameInit)
ON_FRESH_VI("resultitem_name_2nd", FRCMD_INIT, FrScratchResultDlg::OnResultItemName2ndInit983)
ON_FRESH_VI("msg01", FRCMD_INIT, FrScratchResultDlg::OnMsg01Init983)
ON_FRESH_VI("msg02", FRCMD_INIT, FrScratchResultDlg::OnMsg02Init983)
ON_FRESH_VI("ok", FRCMD_INIT, FrScratchResultDlg::OnOkInit983)
END_FRESH_MSGMAP()

FrScratchResultDlg::FrScratchResultDlg()
{
	C_ASSERT_VC10(sizeof(FrScratchResultDlg) == 0x150 && offsetof(FrScratchResultDlg, m_count) == 0x140); // 983: push 0x150 em 008eb243
	m_pIcon = NULL;
	m_pName[0] = NULL;
	m_pName[1] = NULL;
	m_pWnd120 = NULL;
	m_pWnd124 = NULL;
	m_pWnd128 = NULL;
	m_curIndex983 = -1;

	static char* textures[1] = { "[font_wind.jpg" };
	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = textures;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);

	m_balls.clear();
	m_count = 0;
}

FrScratchResultDlg::~FrScratchResultDlg()
{
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrScratchResultDlg::OnScratchResultDlgInit(int param)
{
}

bool FrScratchResultDlg::OnInit()
{
	return true;
}

// 983 008e91c0: a linha do topo da lista muda (rolagem) -> nomes dos itens nas 2 linhas de texto
void FrScratchResultDlg::OnProc(const float deltaTime)
{
	if (m_pIcon == NULL || m_pIcon->GetScrollBar() == NULL)
		return;
	int index = m_pIcon->GetScrollBar()->GetCurTopRow_Int();
	if (m_curIndex983 == index)
		return;
	m_curIndex983 = index;
	SetResultName983(0, index);
	if (m_pIcon->m_itemList.size() > 1)
		SetResultName983(1, m_curIndex983 + 1);
}

// 983 008e8d30
void FrScratchResultDlg::SetResultName983(int line, int index)
{
	if (m_pIcon == NULL)
		return;
	FrListBox::ITEM_LIST::iterator it = m_pIcon->m_itemList.begin();
	for (int i = 0; i != index; i++, ++it)
	{
		if (it == m_pIcon->m_itemList.end())
			return;
	}
	if (it == m_pIcon->m_itemList.end())
		return;
	FrListItem* item = *it;
	if (item == NULL || item->pData == NULL)
		return;
	unsigned long tid = ((sBonusBall*)item->pData)->typeId;
	if (tid == 0 || (tid >> 26 >= 5 && tid >> 26 <= 6))
		return;
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
	if (pItem == NULL || m_pName[line] == NULL)
		return;
	m_pName[line]->ClearLine();
	m_pName[line]->AddText(MakeStr(K2L_Compatibility("%s"), pItem->Name), false, true);
}

void FrScratchResultDlg::OnResultItemIconInit(int param)
{
	m_pIcon = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrScratchResultDlg::OnResultItemIconOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	sBonusBall* pData = (sBonusBall*)item->pData;
	if (!pData->typeId)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(pData->typeId);
	if (!pItem)
		return;
	float yoff = 0.0f;
	const Bitmap* pBmp = g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->Icon);
	if (!pBmp)
	{
		pBmp = g_pFresh->GetManager()->GetBitmap("ITEMS_FASHION", pItem->Icon);
		yoff = -12.0f;
		if (!pBmp)
			return;
	}
	unsigned h = pBmp->Height();
	unsigned w = pBmp->Width();
	WRect rc;
	rc.y = (float)(m_pIcon->GetItemHeight() / 2 - w / 2) + item->pos.y;
	rc.x = (m_pIcon->GetRect().w * 0.5 - (float)(h / 2)) + item->pos.x + yoff;
	rc.w = (float)h;
	rc.h = (float)w;
	pGDI->DrawTexture(pBmp, rc, 0xffffffff, 0);
	pGDI->SetTextColor(0xff000000, 0xffffffff);
	pGDI->SetTextStyle(4);
	for (int i = 0; i < m_count; i++)
	{
		sBonusBall b = m_balls[i];
		if (b.count > 1 && pData->typeId == b.typeId)
		{
			const char* str = MakeStr("%d", b.count);
			float x = rc.x + 40.0;
			for (unsigned k = 0; k < strlen(str); k++)
				x -= m_pFont->GetCharWidth(g_view, str[k] - '0', false);
			m_pFont->Print(g_view, x, rc.y + 30.0, str, 0, 0xffffffff, NULL);
		}
	}
}

// 983 008e6910
void FrScratchResultDlg::OnResultItemNameInit(int param)
{
	m_pName[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

// 983 008e6980
void FrScratchResultDlg::OnResultItemName2ndInit983(int param)
{
	m_pName[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

// 983 008e69f0/008e6a60/008e6ad0
void FrScratchResultDlg::OnMsg01Init983(int param) { m_pWnd120 = (FrWnd*)param; }
void FrScratchResultDlg::OnMsg02Init983(int param) { m_pWnd124 = (FrWnd*)param; }
void FrScratchResultDlg::OnOkInit983(int param) { m_pWnd128 = (FrWnd*)param; }

void FrScratchResultDlg::AdjustLine(int count, FrWnd* pWnd)
{
	// 983 (008e6b40): com um item so, a linha sobe 70 px
	if (pWnd)
	{
		WRect rc = pWnd->GetRect();
		if (count <= 1)
			rc.y -= 70.0;
		pWnd->SetRect(rc);
	}
}

void FrScratchResultDlg::SetResultItem(sBonusBall* pBall, int count)
{
	AdjustLine(count, m_pWnd120);
	AdjustLine(count, m_pWnd124);
	AdjustLine(count, m_pWnd128);
	if (count < 2)
	{
		WRect rc = m_pIcon->GetRect();
		rc.y += 20.0;
		m_pIcon->SetRect(rc);
	}
	if (pBall && count > 0)
	{
		for (int n = count; n > 0; n--, pBall++)
		{
			if (pBall->typeId != 0)
				m_balls.push_back(*pBall);
		}
		m_count = count;
		for (int i = 0; i < m_count; i++)
		{
			if (m_balls[i].typeId != 0 &&
				ItemManager()->FindCommonItem(m_balls[i].typeId))
				m_pIcon->AddItem(&m_balls[i]);
		}
	}
	else
	{
		m_balls.clear();
		m_count = 0;
	}
}

// ----------------------------------------------------------- FrScratchAboutDlg

IMPLEMENT_OBJECT(FrScratchAboutDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrScratchAboutDlg, FrForm)
ON_FRESH_VI("aboutviewer", FRCMD_INIT, FrScratchAboutDlg::OnAboutViewerInit)
ON_FRESH_VI("index01", FRCMD_INIT, FrScratchAboutDlg::OnIndex01Init)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnIndex01BnUp)
ON_FRESH_VI("index02", FRCMD_INIT, FrScratchAboutDlg::OnIndex02Init)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnIndex02BnUp)
ON_FRESH_VI("index03", FRCMD_INIT, FrScratchAboutDlg::OnIndex03Init)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnIndex03BnUp)
ON_FRESH_VI("index04", FRCMD_INIT, FrScratchAboutDlg::OnIndex04Init)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnIndex04BnUp)
ON_FRESH_VI("index05", FRCMD_INIT, FrScratchAboutDlg::OnIndex05Init)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnIndex05BnUp)
ON_FRESH_VI("backward", FRCMD_INIT, FrScratchAboutDlg::OnBackwardBnInit)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnBackwardBnUp)
ON_FRESH_VI("forward", FRCMD_INIT, FrScratchAboutDlg::OnForwardBnInit)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnForwardBnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrScratchAboutDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrScratchAboutDlg::OnOkBnUp)
END_FRESH_MSGMAP()

float g_ptScratchItemData[6] = { -17.0f, -38.0f, 23.0f, -38.0f, 3.0f, -38.0f };

FrScratchAboutDlg::FrScratchAboutDlg()
{
	for (int i = 0; i < 5; i++)
	{
		m_pIndex[i] = NULL;
	}
	m_index = 0;
	m_pBackward = NULL;
	m_pForward = NULL;
}

FrScratchAboutDlg::~FrScratchAboutDlg()
{
}

void FrScratchAboutDlg::OnAboutViewerInit(int param)
{
	m_pViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex01Init(int param)
{
	m_pIndex[0] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex01BnUp()
{
	SetImgAboutIndex(0);
}

void FrScratchAboutDlg::OnIndex02Init(int param)
{
	m_pIndex[1] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex02BnUp()
{
	SetImgAboutIndex(1);
}

void FrScratchAboutDlg::OnIndex03Init(int param)
{
	m_pIndex[2] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex03BnUp()
{
	SetImgAboutIndex(2);
}

void FrScratchAboutDlg::OnIndex04Init(int param)
{
	m_pIndex[3] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex04BnUp()
{
	SetImgAboutIndex(3);
}

void FrScratchAboutDlg::OnIndex05Init(int param)
{
	m_pIndex[4] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnIndex05BnUp()
{
	SetImgAboutIndex(4);
}

void FrScratchAboutDlg::OnBackwardBnInit(int param)
{
	m_pBackward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBackward->UseDblClick(false); // 983 008e6f50: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrScratchAboutDlg::OnBackwardBnUp()
{
	m_index--;
	SetImgAboutIndex(m_index);
	m_pBackward->SetPushDelay(0.0f);
}

void FrScratchAboutDlg::OnForwardBnInit(int param)
{
	m_pForward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pForward->UseDblClick(false); // 983 008e7010: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrScratchAboutDlg::OnForwardBnUp()
{
	m_index++;
	SetImgAboutIndex(m_index);
	m_pForward->SetPushDelay(0.0f);
}

void FrScratchAboutDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrScratchAboutDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrScratchAboutDlg::OnInit()
{
	SetImgAboutIndex(0);
	return true;
}

void FrScratchAboutDlg::SetImgAboutIndex(int index)
{
	for (int i = 0; i < 5; i++)
		m_pIndex[i]->Unselect(0);
	if (m_pBackward)
		m_pBackward->Enable(true);
	if (m_pForward)
		m_pForward->Enable(true);
	if (index <= 0)
	{
		index = 0;
		m_pIndex[0]->Select(0);
		if (m_pBackward)
			m_pBackward->Enable(false);
	}
	else if (index >= 4)
	{
		index = 4;
		m_pIndex[4]->Select(0);
		if (m_pForward)
			m_pForward->Enable(false);
	}
	m_pIndex[index]->Select(0);
	m_index = index;
	if (m_pViewer)
		m_pViewer->Open(MakeStr("help_scratch_%02d.jpg", index + 1));
}
