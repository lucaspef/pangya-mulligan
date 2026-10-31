#include "minatl.h"
#include "valentineeventdlg.h"
#include "detaildlg.h"
#include "frbutton.h"
#include "frarea.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "shareddoc.h"
#include "packet.h"
#include "wfont.h"
#include "wresrcmng.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrValEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrValEventDlg, FrForm)
ON_FRESH_VI("back_img", FRCMD_INIT, FrValEventDlg::OnBackImgInit)
ON_FRESH_VI("back_img", FRCMD_OWNERDRAW, FrValEventDlg::OnBackImgOwnerDraw)
ON_FRESH_VI("gauge_img", FRCMD_OWNERDRAW, FrValEventDlg::OnGaugeImgOwnerDraw)
ON_FRESH_VI("nuri_btn", FRCMD_INIT, FrValEventDlg::OnNuriBtnInit)
ON_FRESH_VV("nuri_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnNuriBtnLBDown)
ON_FRESH_VI("azer_btn", FRCMD_INIT, FrValEventDlg::OnAzerBtnInit)
ON_FRESH_VV("azer_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnAzerBtnLBDown)
ON_FRESH_VI("max_btn", FRCMD_INIT, FrValEventDlg::OnMaxBtnInit)
ON_FRESH_VV("max_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnMaxBtnLBDown)
ON_FRESH_VI("kaz_btn", FRCMD_INIT, FrValEventDlg::OnKazBtnInit)
ON_FRESH_VV("kaz_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnKazBtnLBDown)
ON_FRESH_VI("hana_btn", FRCMD_INIT, FrValEventDlg::OnHanaBtnInit)
ON_FRESH_VV("hana_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnHanaBtnLBDown)
ON_FRESH_VI("cecil_btn", FRCMD_INIT, FrValEventDlg::OnCecilBtnInit)
ON_FRESH_VV("cecil_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnCecilBtnLBDown)
ON_FRESH_VI("kooh_btn", FRCMD_INIT, FrValEventDlg::OnKoohBtnInit)
ON_FRESH_VV("kooh_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnKoohBtnLBDown)
ON_FRESH_VI("arin_btn", FRCMD_INIT, FrValEventDlg::OnArinBtnInit)
ON_FRESH_VV("arin_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnArinBtnLBDown)
ON_FRESH_VI("lucia_btn", FRCMD_INIT, FrValEventDlg::OnLuciaBtnInit)
ON_FRESH_VV("lucia_btn", FRCMD_LBUTTONDOWN, FrValEventDlg::OnLuciaBtnLBDown)
END_FRESH_MSGMAP()

FrValEventDlg::FrValEventDlg()
{
	m_nuriHeart = 0;
	m_azerHeart = 0;
	m_maxHeart = 0;
	m_kazHeart = 0;
	m_hanaHeart = 0;
	m_cecilHeart = 0;
	m_koohHeart = 0;
	m_arinHeart = 0;
	m_luciaHeart = 0;
}

FrValEventDlg::~FrValEventDlg()
{
}

bool FrValEventDlg::OnInit()
{
	if (0)
		DYNAMIC_CAST(FrButton, (FrWnd*)0);
	SetCaption("White Day Event");
	return FrForm::OnInit();
}

void FrValEventDlg::BindButton(FrButton*& pButton, int param)
{
	pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->SetPushDelay(0.1f);
		pButton->SetVisible(false);
	}
}

void FrValEventDlg::BuildHeartCount()
{
	std::list<sItemInfo>::iterator it;
	unsigned char count;
	std::list<sItemInfo>::iterator itEnd = Doc()->m_myItemList.end();
	for (it = Doc()->m_myItemList.begin(); it != itEnd; ++it)
	{
		switch ((*it).tid)
		{
		case 0x1a0000be:
			count = (unsigned char)(*it).Common[0];
			m_nuriHeart = count;
			if (count >= 10)
				m_nuriHeart = 10;
			break;
		case 0x1a0000bf:
			count = (unsigned char)(*it).Common[0];
			m_azerHeart = count;
			if (count >= 10)
				m_azerHeart = 10;
			break;
		case 0x1a0000c0:
			count = (unsigned char)(*it).Common[0];
			m_maxHeart = count;
			if (count >= 10)
				m_maxHeart = 10;
			break;
		case 0x1a0000c1:
			count = (unsigned char)(*it).Common[0];
			m_kazHeart = count;
			if (count >= 10)
				m_kazHeart = 10;
			break;
		case 0x1a0000b6:
			count = (unsigned char)(*it).Common[0];
			m_hanaHeart = count;
			if (count >= 10)
				m_hanaHeart = 10;
			break;
		case 0x1a0000b7:
			count = (unsigned char)(*it).Common[0];
			m_cecilHeart = count;
			if (count >= 10)
				m_cecilHeart = 10;
			break;
		case 0x1a0000b8:
			count = (unsigned char)(*it).Common[0];
			m_koohHeart = count;
			if (count >= 10)
				m_koohHeart = 10;
			break;
		case 0x1a0000b9:
			count = (unsigned char)(*it).Common[0];
			m_arinHeart = count;
			if (count >= 10)
				m_arinHeart = 10;
			break;
		case 0x1a0000ba:
			count = (unsigned char)(*it).Common[0];
			m_luciaHeart = count;
			if (count >= 10)
				m_luciaHeart = 10;
			break;
		case 0x1a0000bb:
			count = (unsigned char)(*it).Common[0];
			m_nellHeart = count;
			if (count >= 10)
				m_nellHeart = 10;
			break;
		}
	}
}

void FrValEventDlg::DrawInDlg(const char* name, WPoint& pos)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		WPoint base = WPoint(m_rect.x + 2.0f, m_rect.y + 23.0f);
		const Bitmap* bitmap = g_pFresh->GetBitmap(name);
		if (bitmap)
			gdi->DrawTexture(bitmap,
				WRect(base.x + pos.x, base.y + pos.y, (float)bitmap->Width(),
					(float)bitmap->Height()),
				0xffffffff, 0);
	}
}

void FrValEventDlg::DrawGauge(WPoint& pos, unsigned char count)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		const Bitmap* bitmap = g_pFresh->GetBitmap("Heart_Gage");
		WPoint base = WPoint(m_rect.x + 2.0f, m_rect.y + 23.0f);
		float offset = (float)(10 - count) * (float)bitmap->Height() * 0.1f;
		gdi->DrawTexture(bitmap,
			WRect(base.x + pos.x, offset + pos.y + base.y,
				(float)bitmap->Width(), (float)bitmap->Height() - offset),
			0xffffffff, 0);
	}
}

void FrValEventDlg::OnBackImgInit(int param)
{
	BuildHeartCount();
}

void FrValEventDlg::OnBackImgOwnerDraw(int param)
{
	if (g_pFresh->GetManager()->GetGDI())
	{
		WPoint pos;
		pos.x = 0.0f;
		pos.y = 0.0f;
		DrawInDlg("Frame_WhiteDay_01", pos);
		pos.x = 256.0f;
		pos.y = 0.0f;
		DrawInDlg("Frame_WhiteDay_02", pos);
		pos.x = 512.0f;
		pos.y = 0.0f;
		DrawInDlg("Frame_WhiteDay_03", pos);
		pos.x = 0.0f;
		pos.y = 256.0f;
		DrawInDlg("Frame_WhiteDay_04", pos);
		pos.x = 256.0f;
		pos.y = 256.0f;
		DrawInDlg("Frame_WhiteDay_05", pos);
		pos.x = 512.0f;
		pos.y = 256.0f;
		DrawInDlg("Frame_WhiteDay_06", pos);
	}
}

void FrValEventDlg::OnGaugeImgOwnerDraw(int param)
{
	if (g_pFresh->GetManager()->GetGDI())
	{
		WPoint pos;
		if (m_giftFlag & 0x20)
		{
			pos.x = 25.0f;
			pos.y = 373.0f;
			DrawGauge(pos, 10);
		}
		else if (m_nuriHeart < 10)
		{
			pos.x = 25.0f;
			pos.y = 373.0f;
			DrawGauge(pos, m_nuriHeart);
			pos.x = 25.0f;
			pos.y = 267.0f;
			DrawInDlg("Character_Nuri", pos);
		}
		if (m_giftFlag & 0x40)
		{
			pos.x = 227.0f;
			pos.y = 373.0f;
			DrawGauge(pos, 10);
		}
		else if (m_azerHeart < 10)
		{
			pos.x = 227.0f;
			pos.y = 373.0f;
			DrawGauge(pos, m_azerHeart);
			pos.x = 227.0f;
			pos.y = 267.0f;
			DrawInDlg("Character_Azer", pos);
		}
		if (m_giftFlag & 0x80)
		{
			pos.x = 126.0f;
			pos.y = 373.0f;
			DrawGauge(pos, 10);
		}
		else if (m_maxHeart < 10)
		{
			pos.x = 126.0f;
			pos.y = 373.0f;
			DrawGauge(pos, m_maxHeart);
			pos.x = 126.0f;
			pos.y = 267.0f;
			DrawInDlg("Character_Max", pos);
		}
		if (m_giftFlag & 0x100)
		{
			pos.x = 328.0f;
			pos.y = 373.0f;
			DrawGauge(pos, 10);
		}
		else if (m_kazHeart < 10)
		{
			pos.x = 328.0f;
			pos.y = 373.0f;
			DrawGauge(pos, m_kazHeart);
			pos.x = 328.0f;
			pos.y = 267.0f;
			DrawInDlg("Character_Kaz", pos);
		}
		pos.x = 18.0f;
		pos.y = 366.0f;
		DrawInDlg("Frame_Candy_01", pos);
		pos.x = 274.0f;
		pos.y = 366.0f;
		DrawInDlg("Frame_Candy_02", pos);
	}
}

void FrValEventDlg::CheckButtonClick()
{
	if (m_nuriHeart >= 10)
	{
		if (m_giftFlag & 0x20)
		{
			m_pNuriBtn->SetVisible(false);
			m_pNuriBtn->Enable(false);
		}
		else
		{
			m_pNuriBtn->SetVisible(true);
			m_pNuriBtn->Enable(true);
		}
	}
	if (m_azerHeart >= 10)
	{
		if (m_giftFlag & 0x40)
		{
			m_pAzerBtn->SetVisible(false);
			m_pAzerBtn->Enable(false);
		}
		else
		{
			m_pAzerBtn->SetVisible(true);
			m_pAzerBtn->Enable(true);
		}
	}
	if (m_maxHeart >= 10)
	{
		if (m_giftFlag & 0x80)
		{
			m_pMaxBtn->SetVisible(false);
			m_pMaxBtn->Enable(false);
		}
		else
		{
			m_pMaxBtn->SetVisible(true);
			m_pMaxBtn->Enable(true);
		}
	}
	if (m_kazHeart >= 10)
	{
		if (m_giftFlag & 0x100)
		{
			m_pKazBtn->SetVisible(false);
			m_pKazBtn->Enable(false);
			return;
		}
		m_pKazBtn->SetVisible(true);
		m_pKazBtn->Enable(true);
	}
}

void FrValEventDlg::OnNuriBtnInit(int param)
{
	BindButton(m_pNuriBtn, param);
}

void FrValEventDlg::OnNuriBtnLBDown()
{
	if (m_pNuriBtn)
		m_pNuriBtn->SetVisible(false);
	WSendPacket packet((enumClientPacket)0xdb); // JP 983 (s63): 0xdd no KR 645
	packet.Encode4(0);
	packet.Send(TO_GAME);
	m_giftFlag |= 0x20;
}

void FrValEventDlg::OnAzerBtnInit(int param)
{
	BindButton(m_pAzerBtn, param);
}

void FrValEventDlg::OnAzerBtnLBDown()
{
	if (m_pAzerBtn)
		m_pAzerBtn->SetVisible(false);
	WSendPacket packet((enumClientPacket)0xdb); // JP 983 (s63): 0xdd no KR 645
	packet.Encode4(2);
	packet.Send(TO_GAME);
	m_giftFlag |= 0x40;
}

void FrValEventDlg::OnMaxBtnInit(int param)
{
	BindButton(m_pMaxBtn, param);
}

void FrValEventDlg::OnMaxBtnLBDown()
{
	if (m_pMaxBtn)
		m_pMaxBtn->SetVisible(false);
	WSendPacket packet((enumClientPacket)0xdb); // JP 983 (s63): 0xdd no KR 645
	packet.Encode4(4);
	packet.Send(TO_GAME);
	m_giftFlag |= 0x80;
}

void FrValEventDlg::OnKazBtnInit(int param)
{
	BindButton(m_pKazBtn, param);
}

void FrValEventDlg::OnKazBtnLBDown()
{
	if (m_pKazBtn)
		m_pKazBtn->SetVisible(false);
	WSendPacket packet((enumClientPacket)0xdb); // JP 983 (s63): 0xdd no KR 645
	packet.Encode4(7);
	packet.Send(TO_GAME);
	m_giftFlag |= 0x100;
}

void FrValEventDlg::OnHanaBtnInit(int param)
{
	BindButton(m_pHanaBtn, param);
	if (m_pHanaBtn && m_hanaHeart >= 10)
	{
		m_pHanaBtn->SetVisible(true);
		m_pHanaBtn->Enable(true);
		m_pHanaBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnHanaBtnLBDown()
{
	if (0)
		BindButton(m_pHanaBtn, 0);
}

void FrValEventDlg::OnArinBtnInit(int param)
{
	BindButton(m_pArinBtn, param);
	if (m_pArinBtn && m_arinHeart >= 10)
	{
		m_pArinBtn->SetVisible(true);
		m_pArinBtn->Enable(true);
		m_pArinBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnArinBtnLBDown()
{
	if (0)
		BindButton(m_pArinBtn, 0);
}

void FrValEventDlg::OnCecilBtnInit(int param)
{
	BindButton(m_pCecilBtn, param);
	if (m_pCecilBtn && m_cecilHeart >= 10)
	{
		m_pCecilBtn->SetVisible(true);
		m_pCecilBtn->Enable(true);
		m_pCecilBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnCecilBtnLBDown()
{
	if (0)
		BindButton(m_pCecilBtn, 0);
}

void FrValEventDlg::OnKoohBtnInit(int param)
{
	BindButton(m_pKoohBtn, param);
	if (m_pKoohBtn && m_koohHeart >= 10)
	{
		m_pKoohBtn->SetVisible(true);
		m_pKoohBtn->Enable(true);
		m_pKoohBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnKoohBtnLBDown()
{
	if (0)
		BindButton(m_pKoohBtn, 0);
}

void FrValEventDlg::OnLuciaBtnInit(int param)
{
	BindButton(m_pLuciaBtn, param);
	if (m_pLuciaBtn && m_luciaHeart >= 10)
	{
		m_pLuciaBtn->SetVisible(true);
		m_pLuciaBtn->Enable(true);
		m_pLuciaBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnLuciaBtnLBDown()
{
	if (0)
		BindButton(m_pLuciaBtn, 0);
}

void FrValEventDlg::OnNellBtnInit(int param)
{
	BindButton(m_pNellBtn, param);
	if (m_pNellBtn && m_nellHeart >= 10)
	{
		m_pNellBtn->SetVisible(true);
		m_pNellBtn->Enable(true);
		m_pLuciaBtn->SetVisible(false);
	}
}

void FrValEventDlg::OnNellBtnLBDown()
{
	if (0)
		BindButton(m_pNellBtn, 0);
}

class NtNotifyImgDlg : public FrForm
{
	DECLARE_OBJECT(NtNotifyImgDlg)

public:
	NtNotifyImgDlg() { m_pImg = NULL; }
	virtual ~NtNotifyImgDlg() { }

	void SetBgImg(const char* name)
	{
		if (m_pImg)
			m_pImg->SetBgImg(name);
	}

	FrArea* m_pImg;

protected:
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnCaptionInit(int param);
	void OnImgInit(int param);

	DECLARE_FRESH_MSGMAP()
};

IMPLEMENT_OBJECT(NtNotifyImgDlg, FrForm)

BEGIN_FRESH_MSGMAP(NtNotifyImgDlg, FrForm)
ON_FRESH_VI("close", FRCMD_INIT, NtNotifyImgDlg::OnCloseBtnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, NtNotifyImgDlg::OnCloseBtnUp)
ON_FRESH_VI("caption", FRCMD_INIT, NtNotifyImgDlg::OnCaptionInit)
ON_FRESH_VI("img", FRCMD_INIT, NtNotifyImgDlg::OnImgInit)
END_FRESH_MSGMAP()

void NtNotifyImgDlg::OnCloseBtnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtNotifyImgDlg::OnCloseBtnUp()
{
	OnFreshOkay();
}

void NtNotifyImgDlg::OnCaptionInit(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtNotifyImgDlg::OnImgInit(int param)
{
	m_pImg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

IMPLEMENT_OBJECT(NtValentineEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(NtValentineEventDlg, FrForm)
ON_FRESH_VI("close", FRCMD_INIT, NtValentineEventDlg::OnCloseBtnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, NtValentineEventDlg::OnCloseBtnUp)
ON_FRESH_VI("detail", FRCMD_INIT, NtValentineEventDlg::OnDetailBtnInit)
ON_FRESH_VV("detail", FRCMD_LBUTTONUP, NtValentineEventDlg::OnDetailBtnUp)
ON_FRESH_VI("caption", FRCMD_INIT, NtValentineEventDlg::OnCaptionInit)
ON_FRESH_VI("bg1", FRCMD_INIT, NtValentineEventDlg::OnBg1Init)
ON_FRESH_VI("bg2", FRCMD_INIT, NtValentineEventDlg::OnBg2Init)
ON_FRESH_VI("bg3", FRCMD_INIT, NtValentineEventDlg::OnBg3Init)
ON_FRESH_VI("bg4", FRCMD_INIT, NtValentineEventDlg::OnBg4Init)
ON_FRESH_VI("valen0", FRCMD_INIT, NtValentineEventDlg::OnValen0Init)
ON_FRESH_VI("valen1", FRCMD_INIT, NtValentineEventDlg::OnValen1Init)
ON_FRESH_VI("valen2", FRCMD_INIT, NtValentineEventDlg::OnValen2Init)
ON_FRESH_VI("valen3", FRCMD_INIT, NtValentineEventDlg::OnValen3Init)
ON_FRESH_VV("valen0", FRCMD_OWNERDRAW, NtValentineEventDlg::OnValen0OwnerDraw)
ON_FRESH_VV("valen1", FRCMD_OWNERDRAW, NtValentineEventDlg::OnValen1OwnerDraw)
ON_FRESH_VV("valen2", FRCMD_OWNERDRAW, NtValentineEventDlg::OnValen2OwnerDraw)
ON_FRESH_VV("valen3", FRCMD_OWNERDRAW, NtValentineEventDlg::OnValen3OwnerDraw)
ON_FRESH_VI("white0", FRCMD_INIT, NtValentineEventDlg::OnWhite0Init)
ON_FRESH_VI("white1", FRCMD_INIT, NtValentineEventDlg::OnWhite1Init)
ON_FRESH_VI("white2", FRCMD_INIT, NtValentineEventDlg::OnWhite2Init)
ON_FRESH_VI("white3", FRCMD_INIT, NtValentineEventDlg::OnWhite3Init)
ON_FRESH_VV("white0", FRCMD_OWNERDRAW, NtValentineEventDlg::OnWhite0OwnerDraw)
ON_FRESH_VV("white1", FRCMD_OWNERDRAW, NtValentineEventDlg::OnWhite1OwnerDraw)
ON_FRESH_VV("white2", FRCMD_OWNERDRAW, NtValentineEventDlg::OnWhite2OwnerDraw)
ON_FRESH_VV("white3", FRCMD_OWNERDRAW, NtValentineEventDlg::OnWhite3OwnerDraw)
ON_FRESH_VI("combine0", FRCMD_INIT, NtValentineEventDlg::OnCombine0BtnInit)
ON_FRESH_VV("combine0", FRCMD_LBUTTONUP, NtValentineEventDlg::OnCombine0BtnUp)
ON_FRESH_VI("combine1", FRCMD_INIT, NtValentineEventDlg::OnCombine1BtnInit)
ON_FRESH_VV("combine1", FRCMD_LBUTTONUP, NtValentineEventDlg::OnCombine1BtnUp)
ON_FRESH_VI("combine2", FRCMD_INIT, NtValentineEventDlg::OnCombine2BtnInit)
ON_FRESH_VV("combine2", FRCMD_LBUTTONUP, NtValentineEventDlg::OnCombine2BtnUp)
ON_FRESH_VI("combine3", FRCMD_INIT, NtValentineEventDlg::OnCombine3BtnInit)
ON_FRESH_VV("combine3", FRCMD_LBUTTONUP, NtValentineEventDlg::OnCombine3BtnUp)
END_FRESH_MSGMAP()

NtValentineEventDlg::NtValentineEventDlg()
{
	char* textures[1];
	textures[0] = "[font_hole_skins_red.jpg";
	WTitleFont* pFont = g_resrcmng->GetTitleFont();
	m_fontInfo.filename = textures;
	m_pFont = pFont;
	m_fontInfo.flag = 0;
	m_fontInfo.fonth = 16;
	m_fontInfo.fontw = 16;
	m_fontInfo.numPages = 1;
	m_fontInfo.texw = 128;
	m_fontInfo.texh = 32;
	m_fontInfo.pCharSet = "1234567890-,";
	m_pFont->Create(&m_fontInfo);
	for (int i = 0; i < 8; ++i)
	{
		m_pCountArea[i] = NULL;
		m_count[i] = 0;
	}
}

NtValentineEventDlg::~NtValentineEventDlg()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void NtValentineEventDlg::SetUserData(int valen0, int valen1, int valen2,
	int valen3)
{
	m_count[0] = valen0;
	m_count[1] = valen1;
	m_count[2] = valen2;
	m_count[3] = valen3;
}

void NtValentineEventDlg::SetUserDataWhite(int white0, int white1, int white2,
	int white3)
{
	m_count[4] = white0;
	m_count[5] = white1;
	m_count[6] = white2;
	m_count[7] = white3;
	UpdateCombinerState();
}

void NtValentineEventDlg::UpdateCombinerState()
{
	for (int i = 0; i < 4; ++i)
	{
		if (m_count[i] < 5 || m_count[i + 4] < 5)
			m_pCombineBtn[i]->SetVisible(false);
		else
			m_pCombineBtn[i]->SetVisible(true);
	}
}

void NtValentineEventDlg::OnCloseBtnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtValentineEventDlg::OnCloseBtnUp()
{
	OnFreshOkay();
}

void NtValentineEventDlg::OnDetailBtnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtValentineEventDlg::OnDetailBtnUp()
{
	NtDetailDlg* dlg = CreateForm<NtDetailDlg>(g_pFresh->GetManager(), this,
		"detail_popup", NULL);
	dlg->Open((FRESH_PFN_RESULT)&NtValentineEventDlg::OnDetailDlgResult, 1);
}

void NtValentineEventDlg::OnCaptionInit(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnBg1Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnBg2Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnBg3Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnBg4Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnValen0Init(int param)
{
	m_pCountArea[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnValen1Init(int param)
{
	m_pCountArea[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnValen2Init(int param)
{
	m_pCountArea[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnValen3Init(int param)
{
	m_pCountArea[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnValen0OwnerDraw()
{
	FrArea* area = m_pCountArea[0];
	if (area)
	{
		char* text = MakeStr("%d", m_count[0]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnValen1OwnerDraw()
{
	FrArea* area = m_pCountArea[1];
	if (area)
	{
		char* text = MakeStr("%d", m_count[1]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnValen2OwnerDraw()
{
	FrArea* area = m_pCountArea[2];
	if (area)
	{
		char* text = MakeStr("%d", m_count[2]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnValen3OwnerDraw()
{
	FrArea* area = m_pCountArea[3];
	if (area)
	{
		char* text = MakeStr("%d", m_count[3]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnWhite0Init(int param)
{
	m_pCountArea[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnWhite1Init(int param)
{
	m_pCountArea[5] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnWhite2Init(int param)
{
	m_pCountArea[6] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnWhite3Init(int param)
{
	m_pCountArea[7] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void NtValentineEventDlg::OnWhite0OwnerDraw()
{
	FrArea* area = m_pCountArea[4];
	if (area)
	{
		char* text = MakeStr("%d", m_count[4]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnWhite1OwnerDraw()
{
	FrArea* area = m_pCountArea[5];
	if (area)
	{
		char* text = MakeStr("%d", m_count[5]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnWhite2OwnerDraw()
{
	FrArea* area = m_pCountArea[6];
	if (area)
	{
		char* text = MakeStr("%d", m_count[6]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

void NtValentineEventDlg::OnWhite3OwnerDraw()
{
	FrArea* area = m_pCountArea[7];
	if (area)
	{
		char* text = MakeStr("%d", m_count[7]);
		float width = m_pFont->GetTextWidthOld983(g_view, text);
		m_pFont->Print(g_view, area->GetRect().w + area->GetRect().x - width,
			area->GetRect().y, text, 0, 0xffffffff, NULL);
	}
}

bool NtValentineEventDlg::OnDetailDlgResult(int result, FrForm* form)
{
	return true;
}

bool NtValentineEventDlg::OnNotifyDlgResult(int result, FrForm* form)
{
	UpdateCombinerState();
	return true;
}

void NtValentineEventDlg::OnCombine0BtnInit(int param)
{
	m_pCombineBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtValentineEventDlg::OnCombine0BtnUp()
{
	WSendPacket packet((enumClientPacket)0x6f);
	packet.Encode4(0);
	packet.Send(TO_GAME);
	NtNotifyImgDlg* dlg = CreateForm<NtNotifyImgDlg>(g_pFresh->GetManager(),
		this, "notify_img", NULL);
	if (dlg)
	{
		dlg->SetBgImg("event_result_03");
		dlg->Open((FRESH_PFN_RESULT)&NtValentineEventDlg::OnNotifyDlgResult, 1);
	}
	m_count[0] = (m_count[0] - 5 < 0) ? 0 : m_count[0] - 5;
	m_count[4] = (m_count[4] - 5 < 0) ? 0 : m_count[4] - 5;
}

void NtValentineEventDlg::OnCombine1BtnInit(int param)
{
	m_pCombineBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtValentineEventDlg::OnCombine1BtnUp()
{
	WSendPacket packet((enumClientPacket)0x6f);
	packet.Encode4(1);
	packet.Send(TO_GAME);
	NtNotifyImgDlg* dlg = CreateForm<NtNotifyImgDlg>(g_pFresh->GetManager(),
		this, "notify_img", NULL);
	if (dlg)
	{
		dlg->SetBgImg("event_result_04");
		dlg->Open((FRESH_PFN_RESULT)&NtValentineEventDlg::OnNotifyDlgResult, 1);
	}
	m_count[1] = (m_count[1] - 5 < 0) ? 0 : m_count[1] - 5;
	m_count[5] = (m_count[5] - 5 < 0) ? 0 : m_count[5] - 5;
}

void NtValentineEventDlg::OnCombine2BtnInit(int param)
{
	m_pCombineBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void NtValentineEventDlg::OnCombine2BtnUp()
{
	WSendPacket packet((enumClientPacket)0x6e);
	packet.Encode4(0x1a000016);
	packet.Send(TO_GAME);
	NtNotifyImgDlg* dlg = CreateForm<NtNotifyImgDlg>(g_pFresh->GetManager(),
		this, "notify_img", NULL);
	if (dlg)
	{
		dlg->SetBgImg("event_result_02");
		dlg->Open((FRESH_PFN_RESULT)&NtValentineEventDlg::OnNotifyDlgResult, 1);
	}
	m_count[2] = (m_count[2] - 5 < 0) ? 0 : m_count[2] - 5;
	m_count[6] = (m_count[6] - 5 < 0) ? 0 : m_count[6] - 5;
}

void NtValentineEventDlg::OnCombine3BtnInit(int param)
{
	m_pCombineBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (00b10030): o quarto botao combina como os outros (pacote 0x6e com 0x1a000015, aviso "event_result_01",
// tira 5 de m_count[3] e m_count[7]); no fonte do 645 o corpo era vazio
void NtValentineEventDlg::OnCombine3BtnUp()
{
	WSendPacket packet((enumClientPacket)0x6e);
	packet.Encode4(0x1a000015);
	packet.Send(TO_GAME);
	NtNotifyImgDlg* dlg = CreateForm<NtNotifyImgDlg>(g_pFresh->GetManager(),
		this, "notify_img", NULL);
	if (dlg)
	{
		dlg->SetBgImg("event_result_01");
		dlg->Open((FRESH_PFN_RESULT)&NtValentineEventDlg::OnNotifyDlgResult, 1);
	}
	m_count[3] = (m_count[3] - 5 < 0) ? 0 : m_count[3] - 5;
	m_count[7] = (m_count[7] - 5 < 0) ? 0 : m_count[7] - 5;
}

IMPLEMENT_OBJECT(ntHolesEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(ntHolesEventDlg, FrForm)
ON_FRESH_VI("prize0", FRCMD_INIT, ntHolesEventDlg::OnPrize0Init)
ON_FRESH_VI("prize1", FRCMD_INIT, ntHolesEventDlg::OnPrize1Init)
ON_FRESH_VI("prize2", FRCMD_INIT, ntHolesEventDlg::OnPrize2Init)
ON_FRESH_VI("prize3", FRCMD_INIT, ntHolesEventDlg::OnPrize3Init)
ON_FRESH_VI("prize4", FRCMD_INIT, ntHolesEventDlg::OnPrize4Init)
ON_FRESH_VV("gauge", FRCMD_OWNERDRAW, ntHolesEventDlg::OnGaugeOwnerDraw)
END_FRESH_MSGMAP()

ntHolesEventDlg::ntHolesEventDlg()
{
	static char* textures[1] = { "[font_bong.jpg" };
	tagWTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	fontInfo.filename = textures;
	fontInfo.fontw = 9;
	fontInfo.fonth = 16;
	fontInfo.numPages = 1;
	fontInfo.texw = 128;
	fontInfo.texh = 32;
	fontInfo.pCharSet = "1234567890,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&fontInfo);
}

ntHolesEventDlg::~ntHolesEventDlg()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void ntHolesEventDlg::OnPrize0Init(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area && Doc()->m_myInfo.stat.eventValue >= 100)
		area->SetBgImg("holes_prize_00");
}

void ntHolesEventDlg::OnPrize1Init(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area && Doc()->m_myInfo.stat.eventValue >= 300)
		area->SetBgImg("holes_prize_01");
}

void ntHolesEventDlg::OnPrize2Init(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area && Doc()->m_myInfo.stat.eventValue >= 500)
		area->SetBgImg("holes_prize_02");
}

void ntHolesEventDlg::OnPrize3Init(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area && Doc()->m_myInfo.stat.eventValue >= 700)
		area->SetBgImg("holes_prize_03");
}

void ntHolesEventDlg::OnPrize4Init(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area && Doc()->m_myInfo.stat.eventValue >= 1000)
		area->SetBgImg("holes_prize_04");
}

void ntHolesEventDlg::OnGaugeOwnerDraw()
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		const Bitmap* bitmap =
			g_pFresh->GetManager()->GetBitmap("EVENT", "gauge");
		if (bitmap)
		{
			float gauge = Doc()->m_myInfo.stat.eventValue / 3.0; // 983 00b103cf
			if (333.0f < gauge)
				gauge = 333.0f;
			gdi->DrawTexture(bitmap,
				WRect(0.0f, 0.0f, (float)bitmap->Width(),
					(float)bitmap->Height()),
				WRect(m_rect.x + 93.0f, m_rect.y + 289.0f, gauge, 8.0f),
				0xffffffff, 0);
			float x = m_rect.x + 427.0f;
			float y = m_rect.y + 137.0f;
			char* text = MakeStr("%d", Doc()->m_myInfo.stat.eventValue);
			float width = m_pFont->GetTextWidthOld983(g_view, text);
			m_pFont->Print(g_view, x - width * 0.5f, y, text, 0, 0xffffffff,
				NULL);
		}
	}
}

ntMightyMACEventDlg::ntMightyMACEventDlg()
{
}

ntMightyMACEventDlg::~ntMightyMACEventDlg()
{
}
