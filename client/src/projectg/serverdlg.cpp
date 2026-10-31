#include "minatl.h"
#include "vc10_layout.h"
#include "networksystem.h"
#include "packet.h"
#include "logininfo.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "binstr.h"
#include "serverdlg.h"
#include "shareddoc.h"
#include "logoutdlg.h"
#include "lobbytask.h"
#include "taskmanager.h"
#include "fresh.h"
#include "frbutton.h"
#include "frdesktop.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frscrollbar.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "woverlay.h"
#include "projectg.h"
#include "messengerdlg.h"
#include "skinnedtheme.h"
#include "s5/utilities.h"
#include "../shared/localize.h"
#include "packetversion.h"
#include "uwin.h"
#include "jp/shareddocjp.h" // D645: GetRemainHours983 (dica do Premium)
namespace S5 { void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); } // 00b307f0 (D645)

bool __fastcall ServerListCompare(const void* a, const void* b);

// Inline in the original shareddoc.h; defined here to keep it out of the PCH.
inline int CSharedDoc::GetTimeBlock()
{
	// HACK: dead reference, defers this to the end of the main pass like the
	// PCH inline of the original.
	if (0)
		ServerListCompare(NULL, NULL);
	return m_myInfo.info.iTimeBlock;
}

extern Fresh* g_pFresh;

// D875: m_dwStyle (protegido) da lista de canais; o 983 le +0x44 cru (cl 14.44 0x40: std::list menor no FrWnd)
struct FrListBoxStyle983 : public FrListBox
{
	using FrWnd::m_dwStyle;
};
C_ASSERT_VC10(offsetof(FrListBoxStyle983, m_dwStyle) == 0x44);


#define MARK_NEW "mark_new"

unsigned int __stdcall XorWithKey(char* data, unsigned int size);

class cProgressBar
{
public:
	struct sProgressImg
	{
		WRect rect;
		const Bitmap* pImg;
	};

	cProgressBar()
		: m_min(0), m_max(0), m_pos(0), m_marginX(0), m_marginY(0)
	{
	}
	~cProgressBar() { }

	void Initialize(const WRect& rect, float marginX, float marginY)
	{
		m_rect = rect;
		m_marginX = marginX;
		m_marginY = marginY;
	}
	void SetImg(const char* left, const char* middle, const char* right,
		const char* bar)
	{
		m_img[0].pImg = g_pFresh->GetBitmap(left);
		m_img[1].pImg = g_pFresh->GetBitmap(middle);
		m_img[2].pImg = g_pFresh->GetBitmap(right);
		m_img[3].pImg = g_pFresh->GetBitmap(bar);

		for (int i = 0; i < 4; i++)
		{
			if (m_img[i].pImg)
				m_img[i].rect = WRect(m_rect.x, m_rect.y,
					m_img[i].pImg->Width(), m_img[i].pImg->Height());
		}

		m_img[2].rect.x = m_rect.x + m_rect.w - m_img[2].rect.w;
		m_img[1].rect.x = m_rect.x + m_img[0].rect.w;
		m_img[1].rect.w = m_rect.w - m_img[0].rect.w - m_img[2].rect.w;

		m_img[3].rect.x = m_rect.x + m_marginX;
		m_img[3].rect.y = m_rect.y + m_marginY;
		m_img[3].rect.w = m_rect.w - m_marginX * 2;
		m_img[3].rect.h = m_img[1].rect.h - m_marginY * 2;
	}
	void SetRange(int min, int max)
	{
		m_min = min;
		m_max = max;
	}
	void SetPos(int pos)
	{
		if (pos < m_min)
			m_pos = m_min;
		else
			m_pos = pos;

		if (pos > m_max)
			m_pos = m_max;
		else
			m_pos = pos;
	}
	void Display()
	{
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (gdi)
		{
			int range = m_max - m_min;
			if (range)
			{
				WRect rect = m_img[3].rect;
				rect.w = rect.w * m_pos / range;
				if (m_img[3].pImg)
					gdi->DrawTexture(m_img[3].pImg, rect, 0xffffffff, 0);
			}
			if (m_img[0].pImg)
				gdi->DrawTexture(m_img[0].pImg, m_img[0].rect, 0xffffffff, 0);
			if (m_img[1].pImg)
				gdi->DrawTexture(m_img[1].pImg, m_img[1].rect, 0xffffffff, 0);
			if (m_img[2].pImg)
				gdi->DrawTexture(m_img[2].pImg, m_img[2].rect, 0xffffffff, 0);
		}
	}

private:
	sProgressImg m_img[4];
	WRect m_rect;
	int m_min;
	int m_max;
	int m_pos;
	float m_marginX;
	float m_marginY;
};

class FrAutoConnect : public FrForm
{
	DECLARE_OBJECT(FrAutoConnect)

	FrAutoConnect()
		: m_pProgressBar(NULL)
	{
		m_pProgressBar = new cProgressBar;
		m_retryTime = 0;
		m_elapsed = 0;
		m_state = 1;
	}
	virtual ~FrAutoConnect()
	{
		if (m_pProgressBar)
		{
			delete m_pProgressBar;
			m_pProgressBar = NULL;
		}
	}
	void Retry() { m_state = 0; }
	virtual bool OnInit()
	{
		FrForm::OnInit();

		m_pProgressBar->Initialize(
			WRect(m_rect.x + 30, m_rect.y + 60, m_rect.w - 60, 30), 3, 3);
		m_pProgressBar->SetImg("gauge_left_frm", "gauge_middle_frm",
			"gauge_right_frm", "gauge_bar_frm");
		m_pProgressBar->SetRange(0, 1000);

		return true;
	}
	virtual void OnProc(const float delta);

protected:
	void OnDrawArea()
	{
		if (!g_pFresh->GetManager()->GetGDI())
			return;

		m_pProgressBar->Display();
	}
	void OnInitStaticMsg(int param)
	{
		m_pStaticMsg = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	}
	void OnInitBt1(int param)
	{
		FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		pButton->SetVisible(false);
	}
	void OnInitBt2(int param)
	{
		FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		pButton->SetVisible(false);
	}

protected:
	cProgressBar* m_pProgressBar;
	FrStatic* m_pStaticMsg;
	int m_state;
	float m_elapsed;
	float m_retryTime;

	DECLARE_FRESH_MSGMAP()
};

void FrAutoConnect::OnProc(const float delta)
{
	FrForm::OnProc(delta);

	if (m_pStaticMsg)
		m_pStaticMsg->SetCaption(
			MakeStr(K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xcf\xb0\xed "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. %c"),
				0x20));

	if (m_state == 2)
		return;

	m_elapsed += delta;
	if (m_elapsed > 10.0f)
	{
		Close((eFormRet)1, true);

		FrWnd* pForm =
			g_pFresh->GetManager()->GetDesktop()->FindChildForm("server");
		if (pForm && IS_KINDOF(FrServerDlg, pForm))
		{
			FrServerDlg* pServerDlg = (FrServerDlg*)pForm;
			pServerDlg->OnNotify(K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 "
								 "\xc1\xa2\xbc\xd3\xc7\xd2 \xbc\xf6 "
								 "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				2);
			pServerDlg->FailedToConnectServer();
		}
		return;
	}

	m_pProgressBar->SetPos((int)(m_elapsed * 100.0f));

	if (m_state == 1)
	{
		if (m_elapsed > 1.0f)
		{
			m_elapsed = 1.0f;
			int r = rand();
			m_state = 0;
			m_retryTime = (float)(r % 1000 + 100) * 0.001f;
		}
		return;
	}

	if (m_elapsed > m_retryTime)
	{
		m_state = 2;
		m_retryTime = (float)(rand() % 1000 + 100) * 0.001f + m_elapsed;

		std::string id = Doc()->m_myInfo.info.sID;
		std::string password = Doc()->m_password;

		unsigned long version = 0x783aa8b8; // 983 00af53cc (PY_PACKET_VERSION do 645: 0x77def61b)
		XorWithKey((char*)&version, sizeof(version));

		WSendPacket send((enumClientPacket)2);
		send.EncodeStr(id);
		send.Encode4(Doc()->m_myInfo.info.dwUID);
		send.Encode4(LOGININFO()->AuthUid());
		send.Encode2(0x6696);
		send.EncodeStr(Doc()->GetLoginAuthKey());
		send.EncodeStr(std::string("JP.R7.983.00")); // 983 00af546f
		send.Encode4(version);

		// 983 00af54b0: endereço MAC no lugar do IsPcBang do 645
		char mac[0x400] = "";
		std::string strMac;
		if (UWIN::GetLocalMacAddress(mac, sizeof(mac), ':') > 0)
		{
			mac[17] = '\0';
			strMac = mac;
		}
		send.EncodeStr(strMac);
		send.Send(TO_GAME);
	}
}

// encryption.h:61 in the original, which emits it right after its only caller.
unsigned int __stdcall XorWithKey(char* data, unsigned int size)
{
	const char* key = "{782AE110-2EEF-4c61-B030-A53F17634F7D}";
	unsigned int i = 0;
	unsigned int j;

	// HACK: dead reference, makes this function wait for its caller
	// FrAutoConnect::OnProc.
	if (0)
		((FrAutoConnect*)0)->FrAutoConnect::OnProc(0);

	if (size < 0x26)
	{
		for (j = 0; j < 0x26; j++)
		{
			data[i++] ^= key[j];
			if (i >= size)
				i = 0;
		}
		return 0x26;
	}

	for (j = 0; j < size; j++)
	{
		data[j] ^= key[i++];
		if (i >= 0x26)
			i = 0;
	}
	return size;
}

IMPLEMENT_OBJECT(FrAutoConnect, FrForm)

BEGIN_FRESH_MSGMAP(FrAutoConnect, FrForm)

ON_FRESH_VV("drawingArea", FRCMD_OWNERDRAW, FrAutoConnect::OnDrawArea)
ON_FRESH_VI("proc_msg", FRCMD_INIT, FrAutoConnect::OnInitStaticMsg)
ON_FRESH_VI("yes", FRCMD_INIT, FrAutoConnect::OnInitBt1)
ON_FRESH_VI("no", FRCMD_INIT, FrAutoConnect::OnInitBt2)

END_FRESH_MSGMAP()

IMPLEMENT_OBJECT(FrServerDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrServerDlg, FrForm)

ON_FRESH_VI("serverlist", FRCMD_INIT, FrServerDlg::OnServerListInit)
ON_FRESH_VI("serverlist", FRCMD_OWNERDRAW, FrServerDlg::OnServerListOwnerDraw)
ON_FRESH_VV("serverlist", FRCMD_LBUTTONDOWN, FrServerDlg::OnServerListBtnDown)
ON_FRESH_VI("channellist", FRCMD_INIT, FrServerDlg::OnChannelListInit)
ON_FRESH_VI("channellist", FRCMD_OWNERDRAW, FrServerDlg::OnChannelListOwnerDraw)
ON_FRESH_VV("channellist", FRCMD_LBUTTONDOWN, FrServerDlg::OnChannelListBtnDown)
ON_FRESH_VI("connectmessage", FRCMD_INIT, FrServerDlg::OnConnectMessageInit)
ON_FRESH_VI("connect_cancle", FRCMD_INIT, FrServerDlg::OnConnectCancleInit)
ON_FRESH_VV("connect_cancle", FRCMD_LBUTTONUP, FrServerDlg::OnConnectCancleUp)

ON_FRESH_VI("message", FRCMD_INIT, FrServerDlg::OnMessageInit)
ON_FRESH_VI("curserver", FRCMD_INIT, FrServerDlg::OnCurServerInit)
ON_FRESH_VI("cancel", FRCMD_INIT, FrServerDlg::OnCloseBtnInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrServerDlg::OnCloseLBtnUp)
// 983 (mapa 00e3b6a8): servico Premium; OWNERDRAW 00af6770 (faixa + dica com o tempo do item 0x1a100002, D645)
ON_FRESH_VI("serverlist_premium_tooltip", FRCMD_INIT, FrServerDlg::OnPremiumTooltipInit983)
ON_FRESH_VI("serverlist_premium_tooltip", FRCMD_OWNERDRAW, FrServerDlg::OnPremiumTooltipOwnerDraw983)

END_FRESH_MSGMAP()

bool __fastcall ServerListCompare(const void* a, const void* b)
{
	const sGameServerInfo* pA = (const sGameServerInfo*)a;
	const sGameServerInfo* pB = (const sGameServerInfo*)b;

	// 983 00af30f0 (A171): servidores 0x80 (Grand Prix) primeiro; depois pelo id, decrescente (645: pelos usuarios)
	if ((pA->property & 0x80) && !(pB->property & 0x80))
		return true;

	if (!(pA->property & 0x80) && (pB->property & 0x80))
		return false;

	if (!pB)
		return false;

	return pA->id > pB->id;
}

FrServerDlg::FrServerDlg()
{
	m_pServerList = NULL;
	m_pChannelList = NULL;
	m_connectWaitTime = 0;
	m_reconnectDelay = 0;
	m_state = (gsLoginState_t)0;
	m_pMessage = NULL;
	m_pCurServer = NULL;
	m_pCloseBtn = NULL;
	m_pad983_15c = false;
	m_pNotifyDlg = NULL;
	m_pWaitDlg = NULL;
	m_pLoginDlg = NULL;
	m_bAlertMessage = false;
	m_alertBlinkTime = 0;
	m_bConnecting = false;
	// 983 00af4060 (A171): Premium (+0x12c..+0x148) zerado, alfa 255 e passo 30; buffer +0x1c0 zerado no fim
	m_pPremium983 = NULL;
	memset(m_pImgPremiumOn983, 0, sizeof(m_pImgPremiumOn983));
	m_pImgPremiumBase983 = NULL;
	m_premiumFrame983 = 0;
	m_premiumAlpha983 = 255.0f;
	m_premiumSpeed983 = 30.0f;

	memset(m_pImgServerIcon, 0, sizeof(m_pImgServerIcon));
	m_pImgConnection = NULL;

	m_pFont = g_resrcmng->GetTitleFont();
	char* fontFile = "[font_angel.jpg";

	m_fontInfo.filename = &fontFile;
	m_fontInfo.flag = 0;
	m_fontInfo.fonth = 11;
	m_fontInfo.fontw = 6;
	m_fontInfo.numPages = 1;
	m_fontInfo.texw = 128;
	m_fontInfo.texh = 32;
	m_fontInfo.pCharSet = "1234567890";
	m_pFont->Create(&m_fontInfo);

	m_bFirstLogin = (Doc()->m_curChannel.Uid == 0xff);
	m_loginReplyCount = 0;
	m_pAutoConnect = NULL;
	m_pProgressBar = NULL;
	m_pConnectCancle = NULL;
	memset(m_serverList, 0, sizeof(m_serverList));
	memset(&m_selChannel, 0, sizeof(m_selChannel));

	C_ASSERT_VC10(offsetof(FrServerDlg, m_pLoginDlg) == 0x14c); C_ASSERT_VC10(offsetof(FrServerDlg, m_pad983_15c) == 0x15c);
	C_ASSERT_VC10(offsetof(FrServerDlg, m_curServer) == 0x164);
	C_ASSERT_VC10(offsetof(FrServerDlg, m_pImgServerIcon) == 0x218);
	C_ASSERT_VC10(offsetof(FrServerDlg, m_pProgressBar) == 0x260);
	C_ASSERT_VC10(offsetof(FrServerDlg, m_serverList) == 0x270);
	C_ASSERT_VC10(sizeof(FrServerDlg) == 0x720);
}

std::map<unsigned int, std::string> FrServerDlg::m_serverIconMap; // 983: 00f03394
const char* __fastcall GetGameString(int key); // 00540a10 (unidade da frente B: include/jp/gamestringmanager.h)

FrServerDlg::~FrServerDlg()
{
	ClearServerList();

	if (m_pProgressBar)
	{
		delete m_pProgressBar;
		m_pProgressBar = NULL;
	}

	m_serverIconMap.clear();

	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrServerDlg::ClearServerList()
{
	// HACK: keeps the unreferenced wmath.h constants emitted, as in the original
	(void)&g_HUGE;
	(void)&g_EPSILON;
	m_pServerList->ClearItem();
	memset(m_serverList, 0, sizeof(m_serverList));
}

void FrServerDlg::SetState(gsLoginState_t state)
{
	m_state = state;
	if (state == 4)
		m_bConnecting = false;
}

void FrServerDlg::OnCancel()
{
	if (Doc()->m_curChannel.Uid == 0xff)
	{
		SetLoginMessageAlert(
			K2L_Compatibility("\xc3\xa4\xb3\xce\xc0\xbb \xbc\xb1\xc5\xc3\xc7\xd8 "
			"\xc1\xd6\xbc\xbc\xbf\xe4."));
		OpenQuitDlg();
		return;
	}

	FrForm::Close(true);
}

bool FrServerDlg::OnInit()
{
	m_serverIconMap[0] = std::string("list_bongdal");
	m_serverIconMap[1] = std::string("list_pipin");
	m_serverIconMap[2] = std::string("list_titangboo");
	m_serverIconMap[3] = std::string("list_dolphiny");
	m_serverIconMap[4] = std::string("list_lolo");
	m_serverIconMap[5] = std::string("list_quma");
	m_serverIconMap[6] = std::string("list_tiki");
	m_serverIconMap[7] = std::string("list_cadie");
	m_serverIconMap[8] = std::string("list_caren");
	m_serverIconMap[9] = std::string(""); // 983 00af75d0: 10/11 vazios do 645 sairam, 0xa..0x16 novos
	m_serverIconMap[10] = std::string("list_minty");
	m_serverIconMap[11] = std::string("list_kuropon");
	m_serverIconMap[12] = std::string("list_cesilia");
	m_serverIconMap[13] = std::string("list_daisuke");
	m_serverIconMap[14] = std::string("list_erika");
	m_serverIconMap[15] = std::string("list_kaz");
	m_serverIconMap[16] = std::string("list_lolo");
	m_serverIconMap[17] = std::string("list_ken");
	m_serverIconMap[18] = std::string("list_kooh");
	m_serverIconMap[19] = std::string("list_max");
	m_serverIconMap[20] = std::string("list_caren");
	m_serverIconMap[21] = std::string("list_arin");
	m_serverIconMap[22] = std::string("list_lucia");
	m_serverIconMap[120] = std::string("");
	m_serverIconMap[128] = std::string("");

	m_connectWaitTime = 0;

	BuildServerList();
	BuildChannelList();
	EnableDrag(false);

	// 983 00af75d0 (00af7f1c..00af806a): tempo e flags 4/8 cada um num texto inteiro pelo K2L (MakeStr com o tempo
	// restante); sem os blocos 0x10/0x80, sem ", " e sem o "procure o suporte" do 645
	if (IsLocalContent(S3_TRADE_BLOCK))
	{
		bool bAny = false;
		std::string text;
		std::string remain;

		if (Doc()->GetTimeBlock() > 0)
		{
			unsigned int days = Doc()->GetTimeBlock() / 24;
			int hours = Doc()->GetTimeBlock() % 24;
			if (days > 0)
				remain += MakeStr(K2L_Compatibility("%d\xc0\xcf "), days);

			if (hours > 0)
				remain += MakeStr(K2L_Compatibility("%d\xbd\xc3\xb0\xa3 "), hours);

			if (Doc()->m_myInfo.info.dwFlagBlock & 4)
			{
				text = MakeStr(K2L_Compatibility("\\c0xffff0000\\c\xb0\xb3\xc0\xce \xbb\xf3\xc1\xa1 \xba\xed\xb7\xb0"
					"\\c0xff000000\\c \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9. \xba\xed\xb7\xb0 "
					"\xc7\xd8\xc1\xa6\xbd\xc3\xb0\xa3\xb1\xee\xc1\xf6 %s\xb3\xb2\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.\n"),
					remain.c_str());
				bAny = true;
			}

			if (Doc()->m_myInfo.info.dwFlagBlock & 8)
			{
				text += MakeStr(K2L_Compatibility("\\c0xffff0000\\c\xc3\xa4\xc6\xc3 \xba\xed\xb7\xb0"
					"\\c0xff000000\\c \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9. \xba\xed\xb7\xb0 "
					"\xc7\xd8\xc1\xa6\xbd\xc3\xb0\xa3\xb1\xee\xc1\xf6 %s\xb3\xb2\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.\n"),
					remain.c_str());
				bAny = true;
			}

			if (bAny)
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x23), (int)text.c_str(), 0, 0, 0, 0);
		}
	}

	return true;
}

void FrServerDlg::OnProc(const float delta)
{
	if (m_bAlertMessage)
	{
		m_alertBlinkTime += delta;

		if (m_alertBlinkTime > 0.5f)
		{
			m_pMessage->SetVisible(!m_pMessage->IsVisible());
			m_alertBlinkTime = 0;
		}
	}

	switch (m_state)
	{
	case 0:
	case 4:
		m_reconnectDelay -= delta;
		if (m_reconnectDelay > 0.0f)
			m_reconnectDelay = 0;
		break;
	case 1:
		m_reconnectDelay -= delta;
		if (m_reconnectDelay < 0.0f)
		{
			ConnectToServer();
			break;
		}
		m_bConnecting = true;
		break;
	case 2:
		if (NET()->IsConnectionComplete(WNetworkSystem::NET_GAME))
		{
			if (!NET()->IsConnected(WNetworkSystem::NET_GAME))
				SetState((gsLoginState_t)4);
		}
		break;
	case 3:
		SetState((gsLoginState_t)4);
		break;
	case 5:
		SetState((gsLoginState_t)3); // 983 00af6c40
		break;
	case 14:
		SetLoginMessage(
			K2L_Compatibility("\xc0\xd4\xc0\xe5 \xc0\xda\xb0\xdd\xc0\xbb \xb9\xde\xc0\xba "
			"\xc0\xaf\xc0\xfa\xb8\xb8 \xc1\xa2\xbc\xd3\xc7\xd2 \xbc\xf6 "
			"\xc0\xd6\xb4\xc2 \xbc\xad\xb9\xf6\xc0\xd4\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 15:
		SetLoginMessage(
			K2L_Compatibility("\xc0\xd4\xc0\xe5 \xc1\xa6\xc7\xd1 \xbd\xc3\xb0\xa3\xc0\xcc "
			"\xc1\xf6\xb3\xaa\xbc\xad \xbc\xad\xb9\xf6\xc1\xa2\xbc\xd3\xc0\xbb "
			"\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 16:
		SetLoginMessage(
			K2L_Compatibility("\xb4\xeb\xc8\xb8 \xc2\xfc\xb0\xa1\xc0\xda\xb0\xa1 "
			"\xbe\xc6\xb4\xcf\xb6\xf3\xbc\xad "
			"\xbc\xad\xb9\xf6\xc1\xa2\xbc\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 "
			"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 20:
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x20), 0, 0, 0, 0, 0);
		break;
	case 17:
		SetLoginMessage("PangYa is not available from your area.");
		FailedToConnectServer();
		break;
	case 9:
		SetLoginMessage(
			K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xdd\xc1\xf6\xb5\xc8 "
			"\xbe\xc6\xc0\xcc\xb5\xf0\xc0\xd4\xb4\xcf\xb4\xd9. "
			"\xb0\xed\xb0\xb4\xc1\xf6\xbf\xf8\xbc\xbe\xc5\xcd\xb7\xce "
			"\xb9\xae\xc0\xc7\xba\xce\xc5\xb9\xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 18:
		SetLoginMessage(
			K2L_Compatibility("\xb0\xe8\xc1\xa4 \xc0\xcc\xc0\xfc\xc0\xcc \xb5\xc7\xbe\xee "
			"\xc1\xa2\xbc\xd3\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 8:
		SetLoginMessage(
			K2L_Compatibility("\xbf\xb5\xb1\xb8\xc0\xfb\xc0\xb8\xb7\xce "
			"\xbb\xe7\xbf\xeb\xb1\xdd\xc1\xf6\xb5\xc8 "
			"\xbe\xc6\xc0\xcc\xb5\xf0\xc0\xd4\xb4\xcf\xb4\xd9. "
			"\xb0\xed\xb0\xb4\xc1\xf6\xbf\xf8\xbc\xbe\xc5\xcd\xb7\xce "
			"\xb9\xae\xc0\xc7\xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		FailedToConnectServer();
		break;
	case 19:
		SetLoginMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 "
			"\xc1\xa2\xbc\xd3\xc7\xcf\xb1\xe2 \xbe\xee\xb7\xc1\xbf\xee "
			"\xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3 "
			"\xc8\xc4\xbf\xa1 \xb4\xd9\xbd\xc3 \xc1\xa2\xbc\xd3\xc7\xd8 "
			"\xc1\xd6\xbc\xbc\xbf\xe4."));
		FailedToConnectServer();
		break;
	case 12:
	case 13:
		FailedToConnectServer();
		break;
	}

	if (m_pConnectMessage->IsVisible())
	{
		m_connectWaitTime += delta;
		if (m_connectWaitTime > 10.0f)
			m_pConnectCancle->Enable(true);
	}

	// 983 00af6c40 (A171): a faixa Premium esmaece no 1o quadro depois do BuildChannelList
	if (m_premiumFrame983 == 1)
	{
		float alpha = m_premiumAlpha983 - m_premiumSpeed983 * delta;
		m_premiumAlpha983 = alpha;
		if (alpha < 0.0f)
			m_premiumAlpha983 = 0;
	}
}

void FrServerDlg::AddLoginReplyCount(int count)
{
	if (m_loginReplyCount == 0)
	{
		m_connectWaitTime = 0;
		m_pConnectMessage->SetVisible(true);

		if (m_pConnectCancle)
			m_pConnectCancle->SetVisible(true);
	}

	// 983 00af3170: total 0x20 (645: 0x15)
	if (count == 0x20)
	{
		m_connectWaitTime = 0;
		m_loginReplyCount = 0x20;
		m_pConnectMessage->SetVisible(false);

		if (m_pConnectCancle)
		{
			m_pConnectCancle->SetVisible(false);
			m_pConnectCancle->Enable(false);
		}
	}
	else
	{
		m_loginReplyCount += 2;
		if (m_loginReplyCount > 0x20)
			m_loginReplyCount = 0x1f;

		int pos = m_loginReplyCount * 100 / 0x20;
		m_pConnectMessage->SetCaption(
			K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa4\xba\xb8\xb8\xa6 "
			"\xbf\xe4\xc3\xbb\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pProgressBar->SetPos(pos);
	}
}

void FrServerDlg::BeginAutoLogin()
{
	if (m_pAutoConnect)
	{
		m_pAutoConnect->Retry();
	}
	else
	{
		m_pAutoConnect = CreateForm<FrAutoConnect>(g_pFresh->GetManager(), this,
			"GHOST_LOADING");
		m_pAutoConnect->EnableDrag(false);
		m_pAutoConnect->Open((FRESH_PFN_RESULT)&FrServerDlg::OnResultAutoRecon,
			0);
	}
}

bool FrServerDlg::OnResultAutoRecon(int result, FrForm* pForm)
{
	m_pAutoConnect = NULL;
	return true;
}

void FrServerDlg::FinishAutoLogin()
{
	if (m_pAutoConnect)
		m_pAutoConnect->Close(true);
}

bool FrServerDlg::CloseByEnterChannel(bool bRet)
{
	if (m_bFirstLogin && !g_pFresh->IsCurrentLayout("SERVERLIST"))
	{
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);

		// 983 00af4315/00af4336: TOPPAGE + mensagem 0x8b (645: ROOMLIST)
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0,
			0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x8b, 4, 0, 0, 0);
	}

	return FrForm::Close(bRet);
}

// 983: ConnectToServer(serverUID) do 645 nao existe (sem chamadores; LTCG o descartou)
void FrServerDlg::ConnectToServer()
{
	m_pServerList->Enable(false);
	m_pChannelList->Enable(false);

	Doc()->m_gameServerAddr = m_curServer.addr;
	Doc()->m_gameServerPort = m_curServer.port;
	Doc()->m_gameServerUID = m_curServer.id;

	if (NET()->IsConnected(WNetworkSystem::NET_GAME))
		NET()->ForceShutDown(WNetworkSystem::NET_GAME);

	NET()->SendMessage(WNetworkSystem::NET_GAME, NetworkUnit::eEVENT_NEXT);

	MESSENGER()->SetServerListWaitingState(false);

	m_pad983_15c = false;
	m_connectWaitTime = 0;
	if (m_pConnectMessage)
		m_pConnectMessage->SetVisible(false);

	memcpy(&Doc()->m_curGameServer, &m_curServer, sizeof(sGameServerInfo));

	m_state = (gsLoginState_t)2;
}

void FrServerDlg::BuildServerList()
{
	ClearServerList();

	unsigned int count = 0;
	for (GAMESERVER_LIST_IT it = Doc()->m_gameServerList.begin();
		it != Doc()->m_gameServerList.end(); ++it)
	{
		if (((*it).property & 0x80) && !((*it).property & 0x10))
		{
			m_pServerList->AddItem(&*it);
			count++;
		}
	}

	for (GAMESERVER_LIST_IT it = Doc()->m_gameServerList.begin();
		it != Doc()->m_gameServerList.end(); ++it)
	{
		if (((*it).property & 0x80) && !((*it).property & 0x10))
			continue;

		if ((*it).property & 0x10)
		{
			bool bSuperGM = (Doc()->m_myInfo.info.dwIdentity >> 10) & 1;
			if (!bSuperGM && IsLocalContent(S3_GM_TOOLKIT))
			{
				bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 2) & 1;
				if (!bGM)
					continue;

				strcat((*it).name, K2L_Compatibility(" [\xb8\xc1\xc5\xe4]")); // 983 00af4c23
			}
		}

		m_pServerList->AddItem(&*it);
		count++;
	}

	if (count < 13) // 983 00af4c82: completa ate 13 linhas (645: 9)
	{
		for (unsigned int i = 0; i < 13 - count; i++)
			m_pServerList->AddItem(&m_serverList[i]);
	}

	m_pServerList->SortItem(ServerListCompare);
	m_pServerList->GetScrollBar()->ScrollToFirst();
}

bool FrServerDlg::OnLoginDlgResult(int result, FrForm* pForm)
{
	m_pLoginDlg = NULL;
	return true;
}

void FrServerDlg::OnServerListInit(int param)
{
	m_pServerList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pServerList)
	{
		m_pServerList->SetPushSound(NULL);

		m_pImgServerIcon[0] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "icon_expevent");
		m_pImgServerIcon[1] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "icon_pangevent");
		m_pImgServerIcon[2] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "icon_angelwing");
		m_pImgServerIcon[3] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "icon_masteryevent"); // 983 00af3981 (645: icon_community, fora dos paks do 983)
		m_pImgConnection =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "icon_connection");
	}
}

void FrServerDlg::OnServerListBtnDown()
{
	// 983 00af4ce0 (A171)
	if (!m_pServerList)
		return;

	FrListItem* pItem = m_pServerList->GetItemUnderCursor();
	if (!pItem)
		return;

	sGameServerInfo* pServer = (sGameServerInfo*)pItem->pData;
	if (!pServer || !pServer->name[0])
		return;

	if (Doc()->m_curGameServer.id == pServer->id)
		return;

	if (m_reconnectDelay > 0)
		return;

	if (!(Doc()->m_myInfo.info.dwIdentity & 0x8014))
	{
		if (pServer->property & 0x40)
		{
			// 983: servidor de novatos ate o nivel 20 (645: 5); o 2o teste do 645 (property & 4, nivel < 6) saiu
			if ((unsigned char)Doc()->m_myInfo.stat.Level > 0x14)
			{
				SetLoginMessageAlert(K2L_Compatibility(
					"\\c0xffff0000\\c\xb7\xe7\xc5\xb0\\c0xff000000\\c\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if (Doc()->m_myInfo.stat.Level == 0)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xb7\xe7\xc5\xb0"
					"F \xb7\xb9\xba\xa7\xc0\xba \xc0\xd4\xc0\xe5\xc7\xd2 "
					"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}
		}

		if ((pServer->property & 8) && Doc()->m_pcBang == 0)
		{
			SetLoginMessageAlert(MakeStr(K2L_Compatibility(
				"\\c0xffff0000\\c\xb8\xb8%d\xbc\xbc \xc0\xcc\xbb\xf3\\c0xff000000\\c\xb8\xb8 "
				"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 "
				"\xbc\xba\xc0\xce\xbc\xad\xb9\xf6\xc0\xd4\xb4\xcf\xb4\xd9."),
				19));
			return;
		}

		// 983 00af4e0c: lista de salas do tipo 1 (CSharedDoc+0x11630, 0062bf50) so entra em servidor com property 0x800
		if (Doc()->m_roomListKind983 == 1 && !(pServer->property & 0x800))
		{
			SetLoginMessageAlert(GetGameString(0x7557));
			return;
		}
	}

	int maxUser = pServer->maxUser;
	int curUser = pServer->curUser;
	if (curUser >= maxUser - 200 &&
		(curUser >= maxUser - 150 || rand() % 3 != 0) &&
		!(Doc()->m_myInfo.info.dwIdentity & 0x14))
	{
		SetLoginMessageAlert(
			K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa6\xc7\xd1 \xc0\xce\xbf\xf8\xc0\xcc "
			"\xb8\xf0\xb5\xce \xc3\xa1\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_reconnectDelay = 1.0f;
	}
	else
	{
		m_pAutoConnect = NULL;
		m_curServer = *pServer;
		m_reconnectDelay = 0;

		Doc()->ClearCurrentChannel(true);

		// 983 00af4ece: o pedido do servidor escolhido sai daqui (0x3 ao login; 0x119, novo, se ja esta no servidor de jogo)
		if (NET()->IsConnected(WNetworkSystem::NET_LOGIN))
		{
			WSendPacket send(3);
			send.Encode4(m_curServer.id);
			send.Send(TO_LOGIN);
		}
		else if (NET()->IsConnected(WNetworkSystem::NET_GAME))
		{
			WSendPacket send((enumClientPacket)0x119);
			send.Encode4(m_curServer.id);
			send.Send(TO_GAME);
		}
		else
		{
			SetState((gsLoginState_t)1);
		}

		m_pChannelList->ClearItem();
		m_pCloseBtn->SetVisible(false);
		m_pMessage->SetVisible(false);
		m_bAlertMessage = false;

		g_audio->PlaySfx("ui_desktop_icon_click");
	}

	// 983 00af4fd2: sem o m_sendGiftList.clear e sem o S5::UTIL::SetServerProperty do 645
	Doc()->m_mailList.clear();
	m_loginReplyCount = 0;
	if (m_pProgressBar)
		m_pProgressBar->SetPos(10);
}

bool FrServerDlg::IsNotDrawServerList(FrListItem* pItem)
{
	if (!m_pServerList)
		return true;

	if (!pItem)
		return true;

	if (!pItem->pData)
		return true;

	return false;
}

void FrServerDlg::DrawServerButton(const WPoint& pt,
	const sGameServerInfo* pInfo, bool bSelected)
{
	bool bDisable = m_reconnectDelay > 0;

	THEME_BUTTONS buttons;
	S5::THEME::GetServerSelectButton(pInfo->property, buttons);

	std::string imgName;
	if (bDisable)
		imgName = buttons.press;
	else if (bSelected)
		imgName = buttons.over;
	else
		imgName = buttons.normal;

	const Bitmap* pBitmap = g_pFresh->GetBitmap(imgName.c_str());

	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	// 983 00af57ae (A171): largura do item da lista (FrListBox+0xe0), nao a do bitmap
	gdi->DrawTexture(pBitmap,
		WRect(pt, (float)m_pServerList->GetItemWidth(), pBitmap->Height()),
		0xffffffff, 0);

	if (strcmp(pInfo->name, "") != 0)
	{
		std::map<unsigned int, std::string>::iterator it =
			m_serverIconMap.find(pInfo->iconIndex);
		if (it != m_serverIconMap.end())
		{
			std::string& iconName = it->second;
			if (iconName.length())
			{
				const Bitmap* pIcon = g_pFresh->GetBitmap(iconName.c_str());
				gdi->DrawTexture(pIcon,
					WRect(pt, pIcon->Width(), pIcon->Height()), 0xffffffff, 0);
			}

			gdi->SetTextStyle(1);

			char name[64] = { 0 }; // 983 00af58f4: 64/32/63 (645: 32/16/31)
			if (strlen(pInfo->name) > 32)
			{
				mbscut(name, pInfo->name, 32);
				strcat(name, "...");
			}
			else
			{
				strncpy(name, pInfo->name, 63);
			}

			gdi->PrintText(WPoint(pt.x + 51.0f, pt.y + 10.0f), 0, name, 0);
			gdi->SetTextStyle(0);
		}
	}
}

void FrServerDlg::OnServerListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (IsNotDrawServerList(pItem))
		return;

	sGameServerInfo* pServer = (sGameServerInfo*)pItem->pData;

	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();

	DrawServerButton(pItem->pos, pServer, pItem->underCursor);

	if (strcmp(pServer->name, "") == 0)
		return;

	gdi->SetTextStyle(1);

	if (Doc()->m_curGameServer.id == pServer->id)
	{
		gdi->DrawTexture(m_pImgConnection,
			WRect(pItem->pos.x + 9.0f, pItem->pos.y + 3.0f,
				m_pImgConnection->Width(), m_pImgConnection->Height()),
			0xffffffff, 0);

		if (pServer->angelicWingsNum > 0)
		{
			float x = m_rect.x;
			float y = m_rect.y;
			gdi->DrawTexture(m_pImgServerIcon[2],
				WRect(x + 420.0f, y + 83.0f, m_pImgServerIcon[2]->Width(),
					m_pImgServerIcon[2]->Height()),
				0xffffffff, 0);

			const char* text = MakeStr("%d", pServer->angelicWingsNum);
			m_pFont->GetTextWidthOld983(g_view, text);
			m_pFont->Print(g_view, x + 440.0f, y + 90.0f, text, 0, 0xffffffff,
				0);
		}
	}

	// 983 00af5bb5 (A171): icones de evento em posicoes fixas (645: deslizantes a partir de 143, com o bit 0x40); 0x80 novo
	if (pServer->eventFlags & 0x10)
	{
		gdi->DrawTexture(m_pImgServerIcon[0],
			WRect(pItem->pos.x + 113.0f, pItem->pos.y + 8.0f,
				m_pImgServerIcon[0]->Width(), m_pImgServerIcon[0]->Height()),
			0xffffffff, 0);
	}

	if (pServer->eventFlags & 0x4)
	{
		gdi->DrawTexture(m_pImgServerIcon[0],
			WRect(pItem->pos.x + 128.0f, pItem->pos.y + 8.0f,
				m_pImgServerIcon[0]->Width(), m_pImgServerIcon[0]->Height()),
			0xa0ffffff, 0);
	}

	if (pServer->eventFlags & 0x2)
	{
		gdi->DrawTexture(m_pImgServerIcon[1],
			WRect(pItem->pos.x + 143.0f, pItem->pos.y + 8.0f,
				m_pImgServerIcon[1]->Width(), m_pImgServerIcon[1]->Height()),
			0xffffffff, 0);
	}

	if (pServer->eventFlags & 0x8)
	{
		gdi->DrawTexture(m_pImgServerIcon[2],
			WRect(pItem->pos.x + 158.0f, pItem->pos.y - 5.0f + 8.0f,
				m_pImgServerIcon[2]->Width(), m_pImgServerIcon[2]->Height()),
			0xffffffff, 0);
	}

	if (pServer->eventFlags & 0x80)
	{
		gdi->DrawTexture(m_pImgServerIcon[3],
			WRect(pItem->pos.x + 158.0f, pItem->pos.y + 8.0f,
				m_pImgServerIcon[3]->Width(), m_pImgServerIcon[3]->Height()),
			0xffffffff, 0);
	}

	if (m_pConnectMessage->IsVisible() && m_pProgressBar)
		m_pProgressBar->Display();

	gdi->SetTextStyle(0);
}

void FrServerDlg::BuildChannelList()
{
	if (m_pWaitDlg)
		m_pWaitDlg->Close(true);

	if (m_pNotifyDlg)
		m_pNotifyDlg->Close(true);

	m_bConnecting = false;
	m_pChannelList->ClearItem();

	std::list<sChannelInfo>::iterator it = Doc()->m_channelList.begin();
	if (it != Doc()->m_channelList.end())
	{
		for (; it != Doc()->m_channelList.end(); ++it)
			m_pChannelList->AddItem(&*it);

		TurnOnChannels();
		m_pChannelList->GetScrollBar()->ScrollToFirst();
		// 983 00af50a7 (A171): faixa Premium pelo bit 14 da identidade
		if (m_pPremium983)
			m_pPremium983->SetVisible((Doc()->m_myInfo.info.dwIdentity >> 14) & 1);
		m_premiumFrame983++;
		return;
	}

	TurnOffChannels();
}

void FrServerDlg::OnChannelListInit(int param)
{
	m_pChannelList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pChannelList)
		m_pImgNew = g_pFresh->GetManager()->GetBitmap("ITEMS", MARK_NEW);
}

void FrServerDlg::OnChannelListBtnDown()
{
	FrListItem* pItem = m_pChannelList->GetItemUnderCursor();
	if (!pItem)
		return;

	const sChannelInfo* pChannel = (const sChannelInfo*)pItem->pData;
	if (!pChannel)
		return;

	if (Doc()->m_curChannel.Uid == pChannel->Uid)
	{
		Close(true);
		return;
	}

	if (!(Doc()->m_myInfo.info.dwIdentity & 0x14))
	{
		if (Doc()->m_curGameServer.property & 0x20)
		{
			unsigned long games = Doc()->m_myInfo.stat.dwGameCount;
			if (games < 1)
				games = 1;

			float ratio =
				(float)Doc()->m_myInfo.stat.dwNoMannerGameCount / (float)games;
			if (ratio >= 0.4f)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xb7\xfc\xc0\xcc "
					"40%\xc0\xcc\xbb\xf3\xc0\xce \xc0\xaf\xc0\xfa\xb4\xc2 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if ((pChannel->Type & 0x800) && Doc()->m_myInfo.stat.Level > 10)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xb7\xe7\xc5\xb0/\xba\xf1\xb1\xe2\xb3\xca\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if (pChannel->Type & 0x1000)
			{
				if (Doc()->m_myInfo.stat.Level < 11 ||
					Doc()->m_myInfo.stat.Level > 20)
				{
					SetLoginMessageAlert(
						K2L_Compatibility("\xc1\xd6\xb4\xcf\xbe\xee/\xbd\xc3\xb4\xcf\xbe\xee\xb8\xb8 "
						"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
						"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
					return;
				}
			}

			if ((pChannel->Type & 0x2000) && Doc()->m_myInfo.stat.Level < 21)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee\xc0\xcc\xbb\xf3 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}
		}
		else
		{
			if ((pChannel->Type & 0x800) && Doc()->m_myInfo.stat.Level > 5)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xb7\xe7\xc5\xb0\xb8\xb8 \xc0\xd4\xc0\xe5\xc7\xd2 "
					"\xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if ((pChannel->Type & 0x1000) &&
				(Doc()->m_myInfo.stat.Level < 6 ||
					Doc()->m_myInfo.stat.Level > 15))
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xba\xf1\xb1\xe2\xb3\xca/\xc1\xd6\xb4\xcf\xbe\xee\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if ((pChannel->Type & 0x2000) &&
				(Doc()->m_myInfo.stat.Level < 11 ||
					Doc()->m_myInfo.stat.Level > 20))
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xc1\xd6\xb4\xcf\xbe\xee/\xbd\xc3\xb4\xcf\xbe\xee\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if ((pChannel->Type & 0x200) && Doc()->m_myInfo.stat.Level > 15)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xc1\xd6\xb4\xcf\xbe\xee \xc0\xcc\xc7\xcf\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}

			if ((pChannel->Type & 0x400) && Doc()->m_myInfo.stat.Level < 11)
			{
				SetLoginMessageAlert(
					K2L_Compatibility("\xc1\xd6\xb4\xcf\xbe\xee \xc0\xcc\xbb\xf3\xb8\xb8 "
					"\xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 "
					"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}
		}
	}

	// 983 00af6044: limpa o set do doc (+0x10a20), copia o canal para o dialogo (+0x1c0) e para o doc (+0x10b2a),
	// desabilita a lista de canais antes do pacote
	Doc()->m_failedGameServerIds983.clear();
	m_pMessage->SetVisible(false);

	memcpy(&m_selChannel, pChannel, sizeof(m_selChannel));
	Doc()->m_selChannel = *pChannel;

	if (Doc()->m_bReturnLobbyNotice)
	{
		if (m_pChannelList)
			m_pChannelList->Enable(false);

		WSendPacket packet((enumClientPacket)0x83);
		packet.Encode1(pChannel->Uid);
		packet.Send(TO_GAME);
	}
	else
	{
		if (m_pChannelList)
			m_pChannelList->Enable(false);

		WSendPacket packet((enumClientPacket)4);
		packet.Encode1(pChannel->Uid);
		packet.Send(TO_GAME);
	}

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));

	if (NET()->IsConnected(WNetworkSystem::NET_MSN) &&
		MESSENGER()->GetComplete() == 4)
	{
		const sChannelInfo* pSelected = &Doc()->m_selChannel;
		Doc()->m_roomInfo.roomGuid = 0xffff;
		Doc()->m_roomInfo.gameType = 0xff;

		sUserPosition position;
		position.iServerGUID = Doc()->m_gameServerUID;
		position.iChannelUid = pSelected->Uid;
		wsprintf(position.csChannelName, pSelected->Name);
		position.iRoomIdx = 0xffff;
		position.iRoomType = -1;

		WSendPacket msnPacket(0x23);
		msnPacket.EncodeBuffer(&position, sizeof(sUserPosition));
		msnPacket.Send(TO_MSN);
	}

	// 983 00af6225: pacote 0x16e (vazio) ao servidor de jogo e de novo a mensagem 1
	WSendPacket enterPacket((enumClientPacket)0x16e);
	enterPacket.Send(TO_GAME);

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));

	m_pCloseBtn->SetVisible(false);
}

bool FrServerDlg::IsDisableEnterChannel(unsigned long flag, unsigned char level)
{
	if ((flag & 0x800 && Doc()->m_myInfo.stat.Level > 5) ||
		(flag & 0x1000 &&
			(Doc()->m_myInfo.stat.Level < 6 ||
				Doc()->m_myInfo.stat.Level > 15)) ||
		(flag & 0x2000 &&
			(Doc()->m_myInfo.stat.Level < 11 ||
				Doc()->m_myInfo.stat.Level > 20)) ||
		(flag & 0x200 && Doc()->m_myInfo.stat.Level > 15) ||
		(flag & 0x400 && Doc()->m_myInfo.stat.Level < 11))
	{
		return true;
	}

	return false;
}

void FrServerDlg::DrawChannelButton(const WPoint& pt, const sChannelInfo* pInfo,
	bool bSelected)
{
	bool bDisable =
		IsDisableEnterChannel(pInfo->Type, Doc()->m_myInfo.stat.Level);

	// 983 00af633a: tambem desabilitado se nao e o canal escolhido (+0x204 = Uid da copia em +0x1c0) e a lista tem o
	// bit 1 de +0x44 ligado, ou se o nivel esta fora de [+0x4d, +0x51] do canal
	if (pInfo->Uid != m_selChannel.Uid &&
		((*(const unsigned long*)&((FrListBoxStyle983*)m_pChannelList)->m_dwStyle >> 1) & 1))
		bDisable = true;
	{
		unsigned char level = Doc()->m_myInfo.stat.Level;
		if (level < pInfo->MinLevel ||
			level > pInfo->MaxLevel)
			bDisable = true;
	}

	THEME_BUTTONS buttons;
	S5::THEME::GetChannelSelectButton(S5::UTIL::GetServerProperty(), buttons);

	std::string imgName;
	if (bDisable)
		imgName = buttons.press;
	else if (bSelected)
		imgName = buttons.over;
	else
		imgName = buttons.normal;

	const Bitmap* pBitmap = g_pFresh->GetBitmap(imgName.c_str());

	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	gdi->DrawTexture(pBitmap,
		WRect(pt.x, pt.y, pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);

	if (!stricmp(pInfo->Name, K2L_Compatibility("\xb7\xe7\xc5\xb0")) ||
		!stricmp(pInfo->Name, K2L_Compatibility("\xb7\xe7\xc5\xb0\xc3\xa4\xb3\xce")))
		gdi->DrawTexture(m_pImgNew,
			WRect(pt.x, pt.y, m_pImgNew->Width(), m_pImgNew->Height()),
			0xffffffff, 0);

	gdi->SetTextStyle(0);
	gdi->PrintText(WPoint(pt.x + 76.0f, pt.y + 15.0f), 0, pInfo->Name, 0);
}

void FrServerDlg::OnChannelListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem)
	{
		const sChannelInfo* pChannel = (const sChannelInfo*)pItem->pData;
		if (pChannel)
		{
			FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();

			DrawChannelButton(pItem->pos, pChannel, pItem->underCursor);

			float x = pItem->pos.x + 76.0f;
			float y = pItem->pos.y + 31.0f;

			int step = pChannel->Max_Num / 18;
			if (step == 0)
				step = 1;

			int bars = min(18, (pChannel->Current_Num + step - 1) / step);

			WOverlay::DrawBox(g_view, WRect(x, y, 110.0f, 6.0f), 0, 0xff535353,
				0);

			for (int i = 0; i < bars; i++)
			{
				unsigned long color = pChannel->Max_Num > pChannel->Current_Num
					? 0xfffeaf24
					: 0xffff0000;
				WRect bar(x + i * 6 + 1.0f, y + 1.0f, 6.0f, 4.0f);
				WOverlay::DrawBox(g_view, WRect(bar), 0, color, 0);
			}

			if (Doc()->m_curChannel.Uid == pChannel->Uid &&
				!stricmp(Doc()->m_curChannel.Name, pChannel->Name))
				gdi->DrawTexture(m_pImgConnection,
					WRect(pItem->pos.x + 9.0f, pItem->pos.y + 3.0f,
						m_pImgConnection->Width(), m_pImgConnection->Height()),
					0xffffffff, 0);
		}
	}
}

void FrServerDlg::OnConnectMessageInit(int param)
{
	m_pConnectMessage = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pConnectMessage)
	{
		m_pConnectMessage->SetVisible(false);
		if (!m_pProgressBar)
		{
			WRect rect = m_pConnectMessage->GetRect();
			WRect barRect = rect;
			barRect.y = rect.y - 25;
			barRect.w = 180;
			barRect.h = 30;

			m_pProgressBar = new cProgressBar;
			m_pProgressBar->Initialize(barRect, 3, 3);
			m_pProgressBar->SetImg("gauge_left_frm", "gauge_middle_frm",
				"gauge_right_frm", "gauge_bar_frm");
			m_pProgressBar->SetRange(0, 100);
			m_pProgressBar->SetPos(10);

			rect.x += 25;
			m_pConnectMessage->SetRect(rect);
		}
	}
}

void FrServerDlg::OnConnectCancleInit(int param)
{
	m_pConnectCancle = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	m_pConnectCancle->Enable(false);
	m_pConnectCancle->SetVisible(false);
}

void FrServerDlg::OnConnectCancleUp()
{
	m_pConnectCancle->Enable(false);
	m_pConnectCancle->SetVisible(false);

	if (m_pConnectMessage->IsVisible())
		m_pConnectMessage->SetVisible(false);

	if (m_pProgressBar)
		m_pProgressBar->SetPos(0);

	FailedToConnectServer();
}

void FrServerDlg::OnMessageInit(int param)
{
	m_pMessage = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrServerDlg::OnCurServerInit(int param)
{
	m_pCurServer = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrServerDlg::OnCloseBtnInit(int param)
{
	m_pCloseBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (g_pFresh->IsCurrentLayout("SERVERLIST"))
		m_pCloseBtn->SetVisible(false);
}

void FrServerDlg::OnPremiumTooltipInit983(int param)
{
	// 983 00af3c40 (A171)
	m_pPremium983 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pPremium983->SetVisible(false);
	m_pImgPremiumOn983[0] = g_pFresh->GetBitmap("Premium service_on_00");
	m_pImgPremiumOn983[1] = g_pFresh->GetBitmap("Premium service_on_01");
	m_pImgPremiumOn983[2] = g_pFresh->GetBitmap("Premium service_on_02");
	m_pImgPremiumBase983 = g_pFresh->GetBitmap("Premium service_base");
	m_premiumAlpha983 = 255.0f;
	m_premiumSpeed983 = 30.0f;
	m_premiumFrame983 = 0;
}

// D645 (983 00af6770): com o mouse sobre a area, a dica e o tempo que falta do item premium 0x1a100002 (como o 005978b0
// do lobby); fora dela, dica vazia. No 1o quadro depois do BuildChannelList desenha a faixa base + 3 bitmaps
// "Premium service_on_0x" centrados com o alpha +0x144 (0..255); alpha <= 0 passa ao quadro 2 (nao desenha mais).
void FrServerDlg::OnPremiumTooltipOwnerDraw983(int)
{
	if (m_pPremium983)
	{
		const WRect& rc = m_pPremium983->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (!(rc.x < mouse.x) || rc.x + rc.w < mouse.x || !(rc.y < mouse.y) || rc.y + rc.h < mouse.y)
			m_pPremium983->SetToolTipText(std::string(""));
		else
		{
			const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
			std::list<sItemInfo>::const_iterator it;
			for (it = items.begin(); it != items.end(); ++it)
				if (it->typeId == 0x1a100002)
					break;
			if (it != items.end())
			{
				SYSTEMTIME end = { 0 };
				S5::SecToSysTime983(&end, *(__int64*)((char*)&*it + 0x20)); // 983: +0x28/+0x2c do no da lista
				__int64 hours = DOCJP()->GetRemainHours983(&end);
				WORD d = 0, h = 0;
				if (hours != 0)
				{
					d = (WORD)(hours / 24);
					h = (WORD)(hours - d * 24);
				}
				std::string tip;
				if (d == 0 && h == 0)
					tip = K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n"); // d005b0
				else
					tip = MakeStr(K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), d, h); // d0ba30
				m_pPremium983->SetToolTipText(tip);
			}
		}
	}
	const Bitmap* b0 = m_pImgPremiumOn983[0];
	const Bitmap* b1 = m_pImgPremiumOn983[1];
	const Bitmap* b2 = m_pImgPremiumOn983[2];
	const Bitmap* base = m_pImgPremiumBase983;
	if (m_premiumFrame983 != 1 || !b0 || !b1 || !b2 || !base)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	unsigned long color = ((int)m_premiumAlpha983 << 24) | 0xFFFFFF;
	float w = g_view->GetWidth();
	float h = g_view->GetHeight();
	float w0 = b0->Width(), w1 = b1->Width(), w2 = b2->Width();
	float h0 = b0->Height();
	float x = w * 0.5 - (w2 + w1 + w0) * 0.5;
	float y = h * 0.5 - h0 * 0.5;
	pGDI->DrawTexture(base, WRect(0.0f, y - 20.0, w, h0 + 40.0), color, 0);
	pGDI->DrawTexture(b0, WRect(x, y, w0, h0), color, 0);
	pGDI->DrawTexture(b1, WRect(w0 + x, y, w1, b1->Height()), color, 0);
	pGDI->DrawTexture(b2, WRect(w1 + w0 + x, y, w2, b2->Height()), color, 0);
	if (m_premiumAlpha983 <= 0.0f)
		m_premiumFrame983++;
}

void FrServerDlg::OnCloseLBtnUp()
{
	if (Doc()->m_curChannel.Uid == 0xff)
	{
		SetLoginMessageAlert(
			K2L_Compatibility("\xc3\xa4\xb3\xce\xc0\xbb \xbc\xb1\xc5\xc3\xc7\xd8 "
			"\xc1\xd6\xbc\xbc\xbf\xe4."));
		OpenQuitDlg();
		return;
	}

	FrForm::Close(true);
}

void FrServerDlg::SetLoginMessage(const char* msg)
{
	m_pMessage->SetCaption(msg);
	m_pMessage->SetVisible(true);
	m_pMessage->SetTextColor(0xff000000, 0xffffffff);
	m_bAlertMessage = false;
}

void FrServerDlg::SetLoginMessageAlert(const char* msg)
{
	m_pMessage->SetCaption(msg);
	m_pMessage->SetVisible(true);
	m_pMessage->SetTextColor(0xffff0000, 0xffffffff);
	m_bAlertMessage = true;
}

void FrServerDlg::TurnOnChannels()
{
	m_pCurServer->SetCaption(Doc()->m_curGameServer.name);
	m_pMessage->SetVisible(false);
	m_pCurServer->SetVisible(true);
	m_pChannelList->SetVisible(true);
	m_pChannelList->Enable(true);
	m_pServerList->Enable(true);
}

void FrServerDlg::TurnOffChannels()
{
	m_pMessage->SetVisible(true);
	m_pCurServer->SetVisible(false);
	m_pChannelList->SetVisible(false);
	m_pServerList->Enable(true);
}

void FrServerDlg::FailedToConnectServer()
{
	if (NET()->IsConnected(WNetworkSystem::NET_GAME))
		NET()->ForceShutDown(WNetworkSystem::NET_GAME);

	Doc()->m_curGameServer.id = 0;

	m_bConnecting = false;

	// 983 00af4600: SetState(0), legenda vazia e SendMsgToMainActor(0x2f0) no fim (00af46cd +0x30)
	SetState((gsLoginState_t)0);
	m_pCurServer->SetCaption("");
	m_pMessage->SetVisible(true);
	m_pCurServer->SetVisible(true);
	m_pChannelList->SetVisible(false);

	m_pServerList->Enable(true);

	CTask* pTask = AfxGetTask();
	if (pTask)
		pTask->SendMsgToMainActor(MsgObject(NULL, 0x2f0, 0, 0, 0, 0, 0));
}

void FrServerDlg::OnNotify(const char* msg, unsigned long style)
{
	if (m_pNotifyDlg)
		m_pNotifyDlg->Close(true);

	m_pNotifyDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
	m_pNotifyDlg->SetMessage(msg, false);
	m_pNotifyDlg->Open((FRESH_PFN_RESULT)&FrServerDlg::OnNotifyResult, style);
}

bool FrServerDlg::OnNotifyResult(int result, FrForm* pForm)
{
	m_pNotifyDlg = NULL;
	return true;
}

void FrServerDlg::OnWaitNotify(const char* msg, unsigned long style)
{
	if (m_pWaitDlg)
		m_pWaitDlg->Close(true);

	m_pWaitDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "wait");
	m_pWaitDlg->SetMessage(msg, false);
	m_pWaitDlg->Open((FRESH_PFN_RESULT)&FrServerDlg::OnWaitNotifyResult, style);
}

bool FrServerDlg::OnWaitNotifyResult(int result, FrForm* pForm)
{
	m_pWaitDlg = NULL;
	return true;
}

void FrServerDlg::OpenQuitDlg()
{
	m_pQuitDlg = CreateForm<FrLogoutDlg>(g_pFresh->GetManager(), this, "quit");
	m_pQuitDlg->DisableServerSelect();
	m_pQuitDlg->Open((FRESH_PFN_RESULT)&FrServerDlg::OnQuitDlgResult, 1);
}

bool FrServerDlg::OnQuitDlgResult(int result, FrForm* pForm)
{
	m_pQuitDlg = NULL;
	return true;
}

void FrServerDlg::EnableChannelList983()
{
	if (m_pChannelList && !m_pChannelList->IsEnabled())
		m_pChannelList->Enable(true);
}
