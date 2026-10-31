#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "formbar.h"
#include "packet.h"
#include "chatmsg.h"
#include "shareddoc.h"
#include "avatar_avatar.h"
#include "privatetradedlg.h"
#include "privatetrade.h"
#include "mathconsts.h"

extern Fresh* g_pFresh;

enum _ePrivateTradeState
{
	PRIVATE_TRADE_NONE,
	PRIVATE_TRADE_1,
	PRIVATE_TRADE_REQUEST,
	PRIVATE_TRADE_OPEN,
	PRIVATE_TRADE_4,
	PRIVATE_TRADE_5,
	PRIVATE_TRADE_6
};

class CPrivateTradeImpl : public FrCmdTarget
{
	friend class CPrivateTrade;

	void SetTraderNick(const std::string& nick) { m_traderNick = nick; }
	void SetTraderGuid(unsigned long guid) { m_traderGuid = guid; }
	const std::string& GetTraderNick() const { return m_traderNick; }
	const unsigned long GetTraderGuid() const { return m_traderGuid; }

	CPrivateTradeImpl();
	virtual ~CPrivateTradeImpl();

	void Reset();
	void Init();
	void Process(float delta);
	void Display();
	void CloseWnd();
	void ChatMsg(const char* msg);

	void SetState(_ePrivateTradeState state) { m_state = state; }
	_ePrivateTradeState GetState() const { return m_state; }

	void OnOpenTradeWnd();
	void OpenRequestWaitWnd(const std::string& nick);
	void OpenAcceptWaitWnd(const std::string& nick);
	bool OnResultTrade(int result, FrForm* pForm);
	bool OnResultWait(int result, FrForm* pForm);
	FrFormBar* OpenFormBar();

	_ePrivateTradeState m_state;
	float m_time;
	FrPrivateTrade* m_pTradeWnd;
	FrFormBar* m_pFormBar;
	unsigned long m_traderGuid;
	std::string m_traderNick;
};

CPrivateTradeImpl::CPrivateTradeImpl()
	: m_state(PRIVATE_TRADE_NONE),
	  m_time(1.0f),
	  m_pTradeWnd(NULL),
	  m_pFormBar(NULL),
	  m_traderGuid(0)
{
}

CPrivateTradeImpl::~CPrivateTradeImpl()
{
}

void CPrivateTradeImpl::Reset()
{
	m_pTradeWnd = NULL;
	m_pFormBar = NULL;
	m_state = PRIVATE_TRADE_NONE;
	m_time = 0;
	m_traderGuid = 0;
	m_traderNick = "";
}

void CPrivateTradeImpl::Init()
{
	m_state = PRIVATE_TRADE_NONE;
}

void CPrivateTradeImpl::Process(float delta)
{
}

void CPrivateTradeImpl::Display()
{
}

void CPrivateTradeImpl::OnOpenTradeWnd()
{
	CloseWnd();

	ntAvatar* pAvatar =
		ntAvatarManager::m_pInstance->FindAvatarByGuid(GetTraderGuid());
	if (pAvatar)
	{
		SetState(PRIVATE_TRADE_OPEN);
		if (m_pTradeWnd == NULL)
		{
			unsigned long tid = pAvatar->GetCharacterInfo().tid;
			m_pTradeWnd = CreateForm<FrPrivateTrade>(g_pFresh->manager, this,
				"FrPrivateTrade");
			m_pTradeWnd->SetCompanion(m_traderNick, m_traderGuid, tid);
			m_pTradeWnd->Open(
				(FRESH_PFN_RESULT)&CPrivateTradeImpl::OnResultTrade, 1);
		}
	}
	else
	{
		ChatMsg(
			K2L_Compatibility("\xb0\xc5\xb7\xa1 \xbb\xf3\xb4\xeb\xb0\xa1 \xb4\xeb\xc8\xad\xb9\xe6\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xb0\xc5\xb7\xa1\xb0\xa1 \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
	}
}

void CPrivateTradeImpl::OpenRequestWaitWnd(const std::string& nick)
{
	if (m_pFormBar)
		return;

	SetState(PRIVATE_TRADE_REQUEST);
	m_pFormBar = OpenFormBar();
	m_pFormBar->SetAutoCloseTimer(10.0f);
	m_pFormBar->EnableButton(true);
	m_pFormBar->SetDesc(MakeStr(
		K2L_Compatibility("%s\xb4\xd4\xbf\xa1\xb0\xd4 \xb0\xc5\xb7\xa1 \xbf\xe4\xc3\xbb\xc0\xcc \xbf\xd4\xbd\xc0\xb4\xcf\xb4\xd9."),
		nick.c_str()));
}

void CPrivateTradeImpl::OpenAcceptWaitWnd(const std::string& nick)
{
	if (m_pFormBar)
		return;

	SetState(PRIVATE_TRADE_REQUEST);
	m_pFormBar = OpenFormBar();
	m_pFormBar->SetAutoCloseTimer(12.0f);
	m_pFormBar->SetDesc(MakeStr(
		K2L_Compatibility("%s\xb4\xd4\xbf\xa1\xb0\xd4 \xb0\xc5\xb7\xa1\xb8\xa6 \xbf\xe4\xc3\xbb\xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9."),
		nick.c_str()));
}

bool CPrivateTradeImpl::OnResultTrade(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		m_pTradeWnd = NULL;
		SetState(PRIVATE_TRADE_6);
	}
	else
	{
		if (result == FrCANCEL)
		{
			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(2);
			packet.Encode4(m_traderGuid);
			packet.EncodeStr(m_traderNick);
			packet.Send(TO_GAME);

			ChatMsg(
				K2L_Compatibility("\xb0\xc5\xb7\xa1\xb0\xa1 \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));
		}

		if (result != FrNONE)
			Reset();
	}

	return true;
}

bool CPrivateTradeImpl::OnResultWait(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		m_pFormBar = NULL;

		WSendPacket packet((enumClientPacket)0xd9);
		packet.Encode1(4);
		packet.Encode4(m_traderGuid);
		packet.EncodeStr(m_traderNick);
		packet.Send(TO_GAME);

		OnOpenTradeWnd();
	}
	else
	{
		if (result == FrNO)
		{
			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(1);
			packet.Encode4(m_traderGuid);
			packet.EncodeStr(m_traderNick);
			packet.Send(TO_GAME);
		}

		if (result != FrNONE)
			Reset();
	}

	return true;
}

void CPrivateTradeImpl::ChatMsg(const char* msg)
{
	CChatMsg::Instance()->AddChatMsg(std::string(msg), 0xffff8000, false,
		false);
}

FrFormBar* CPrivateTradeImpl::OpenFormBar()
{
	FrFormBar* pBar =
		CreateForm<FrFormBar>(g_pFresh->manager, this, "FORM_BAR");
	pBar->SetRange(0, 1000);

	WRect rect = pBar->GetRect();
	pBar->SetRect(
		WRect(g_view->GetWidth() - rect.w - 30.0f, 80.0f, rect.w, rect.h));

	rect.y = 0;
	rect.x = 0;
	pBar->Attach(rect, "course_icon_o");
	pBar->Open((FRESH_PFN_RESULT)&CPrivateTradeImpl::OnResultWait, 1);

	WPoint pos = pBar->GetRelativeBarPos();
	pos.y = 30.0f;
	pBar->MoveBar(pos);

	return pBar;
}

void CPrivateTradeImpl::CloseWnd()
{
	if (m_pTradeWnd)
	{
		m_pTradeWnd->Close(FrNONE, true);
		m_pTradeWnd = NULL;
	}

	if (m_pFormBar)
	{
		m_pFormBar->Close(FrNONE, true);
		m_pFormBar = NULL;
	}
}

IMPLEMENT_ACTOR(CPrivateTrade, IActor)

CPrivateTrade::CPrivateTrade()
	: m_pImpl(NULL)
{
	m_pImpl = new CPrivateTradeImpl;
}

CPrivateTrade::~CPrivateTrade()
{
	delete m_pImpl;
}

void CPrivateTrade::OnLoad()
{
}

void CPrivateTrade::OnInit()
{
	m_pImpl->SetState(PRIVATE_TRADE_NONE);
	m_pImpl->Init();
}

void CPrivateTrade::OnReload()
{
	m_pImpl->Init();
}

void CPrivateTrade::OnProcess(float delta)
{
	m_pImpl->Process(delta);
}

void CPrivateTrade::OnDisplay()
{
	m_pImpl->Display();
}

void CPrivateTrade::OnDestroy()
{
	m_pImpl->CloseWnd();
}

// JP 983 (0077b1b0; frente C, lote 14): os itens vao como _sPrivateTradeItem inteiro (0x428 = 8 + 4 x 0x108);
// o KR 645 mandava 0x368 (itens de 0xd8).
C_ASSERT(sizeof(_sPrivateTradeItem) == 0x428);

void CPrivateTrade::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x247):
		switch (msg.param1)
		{
		case 0:
		{
			unsigned long guid = msg.param2;
			const char* nick = (const char*)msg.param3;

			if (Doc()->m_myInfo.info.dwGuid == guid ||
				m_pImpl->GetState() != PRIVATE_TRADE_NONE)
				break;

			m_pImpl->SetState(PRIVATE_TRADE_REQUEST);

			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(0);
			packet.Encode4(guid);
			packet.EncodeStr(std::string(nick));
			packet.Send(TO_GAME);

			m_pImpl->OpenAcceptWaitWnd(std::string(nick));
			m_pImpl->SetTraderGuid(guid);
			m_pImpl->SetTraderNick(nick);
			break;
		}
		case 1:
		{
			m_pImpl->SetState(PRIVATE_TRADE_OPEN);

			const void* pItems = (const void*)msg.param2;

			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(5);
			packet.Encode4(m_pImpl->GetTraderGuid());
			packet.EncodeStr(m_pImpl->GetTraderNick());
			packet.EncodeBuffer(pItems, sizeof(_sPrivateTradeItem));
			packet.Send(TO_GAME);
			break;
		}
		case 2:
		{
			m_pImpl->SetState(PRIVATE_TRADE_4);

			const void* pItems = (const void*)msg.param2;

			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(6);
			packet.Encode4(m_pImpl->GetTraderGuid());
			packet.EncodeStr(m_pImpl->GetTraderNick());
			packet.EncodeBuffer(pItems, sizeof(_sPrivateTradeItem));
			packet.Send(TO_GAME);
			break;
		}
		case 3:
		{
			m_pImpl->SetState(PRIVATE_TRADE_5);

			const void* pMyItems = (const void*)msg.param2;
			const void* pTraderItems = (const void*)msg.param3;

			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(7);
			packet.Encode4(m_pImpl->GetTraderGuid());
			packet.EncodeStr(m_pImpl->GetTraderNick());
			packet.EncodeBuffer(pMyItems, sizeof(_sPrivateTradeItem));
			packet.EncodeBuffer(pTraderItems, sizeof(_sPrivateTradeItem));
			packet.Send(TO_GAME);
			break;
		}
		}
		break;
	}
}

void CPrivateTrade::OnPacket(WReceivedPacket* pPacket)
{
	unsigned char type = pPacket->Decode1();
	switch (type)
	{
	case 0:
	{
		unsigned long guid = pPacket->Decode4();
		std::string nick = pPacket->DecodeStr();

		if (m_pImpl->GetState() != PRIVATE_TRADE_NONE)
		{
			WSendPacket packet((enumClientPacket)0xd9);
			packet.Encode1(3);
			packet.Encode4(guid);
			packet.EncodeStr(nick);
			packet.Send(TO_GAME);
		}
		else
		{
			m_pImpl->SetState(PRIVATE_TRADE_REQUEST);
			m_pImpl->SetTraderGuid(guid);
			m_pImpl->SetTraderNick(nick);

			m_pImpl->OpenRequestWaitWnd(nick);
		}
		break;
	}
	case 2:
		if (m_pImpl->GetTraderGuid())
			m_pImpl->ChatMsg(K2L_Compatibility( // 983 0077adb7
				"\xb0\xc5\xb7\xa1\xb0\xa1 \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."));

		m_pImpl->CloseWnd();
		m_pImpl->Reset();
		break;
	case 1:
		m_pImpl->ChatMsg(
			K2L_Compatibility("\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xb0\xc5\xb7\xa1\xb8\xa6 \xb0\xc5\xc0\xfd \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_pImpl->CloseWnd();
		m_pImpl->Reset();
		break;
	case 3:
		m_pImpl->ChatMsg(
			K2L_Compatibility("\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xba \xc7\xf6\xc0\xe7 \xb4\xd9\xb8\xa5 \xbb\xe7\xb6\xf7\xb0\xfa \xb0\xc5\xb7\xa1\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."));
		m_pImpl->CloseWnd();
		m_pImpl->Reset();
		break;
	case 4:
		m_pImpl->OnOpenTradeWnd();
		break;
	case 5:
	{
		pPacket->Decode4();
		std::string nick = pPacket->DecodeStr();
		_sPrivateTradeItem item;
		pPacket->DecodeBuffer(&item, sizeof(item));

		FrPrivateTrade* pWnd =
			(FrPrivateTrade*)g_pFresh->manager->GetDesktop()->FindChildForm(
				"FrPrivateTrade");
		if (pWnd)
			pWnd->UpdateFrTradeItem(item);
		break;
	}
	case 6:
	{
		pPacket->Decode4();
		std::string nick = pPacket->DecodeStr();
		_sPrivateTradeItem item;
		pPacket->DecodeBuffer(&item, sizeof(item));

		FrPrivateTrade* pWnd =
			(FrPrivateTrade*)g_pFresh->manager->GetDesktop()->FindChildForm(
				"FrPrivateTrade");
		if (pWnd)
			pWnd->FinalFrWait(item);
		break;
	}
	case 7:
	{
		pPacket->Decode4();
		std::string nick = pPacket->DecodeStr();
		_sPrivateTradeItem item;
		pPacket->DecodeBuffer(&item, sizeof(item));

		FrPrivateTrade* pWnd =
			(FrPrivateTrade*)g_pFresh->manager->GetDesktop()->FindChildForm(
				"FrPrivateTrade");
		if (pWnd)
			pWnd->FinalFrOK(item);
		break;
	}
	case 9:
		pPacket->Decode1();
		if (m_pImpl->GetTraderGuid())
			m_pImpl->ChatMsg(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xc5\xb7\xa1\xb8\xa6 \xb4\xf5\xc0\xcc\xbb\xf3 \xc1\xf8\xc7\xe0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));

		m_pImpl->CloseWnd();
		m_pImpl->Reset();
		break;
	}
}
