#pragma once

#include "frform.h"
#include <map>
#include <string>

#include "../shared/globalgamedefine.h"

class FrLogoutDlg;
class FrAutoConnect;
class cProgressBar;

enum gsLoginState_t
{
};

class FrServerDlg : public FrForm
{
	DECLARE_OBJECT(FrServerDlg)

	FrServerDlg();
	virtual ~FrServerDlg();

	virtual bool OnInit();
	virtual void OnProc(const float delta);
	virtual void OnCancel();

	void SetState(gsLoginState_t state);
	void SetLoginMessage(const char* msg);
	void SetLoginMessageAlert(const char* msg);
	void AddLoginReplyCount(int count);
	void BuildServerList();
	void BuildChannelList();
	void FailedToConnectServer();
	void OnNotify(const char* msg, unsigned long style);
	void ClickDisableList(bool bEnable)
	{
		if (m_pServerList)
			m_pServerList->Enable(bEnable);
	}
	void BeginAutoLogin();
	void FinishAutoLogin();
	bool OnResultAutoRecon(int result, FrForm* pForm);
	bool CloseByEnterChannel(bool bRet);
	void EnableChannelList983(); // 983 0065dc62: lista de canais (+0x118) desligada volta a ligar (msg 0x1d)

protected:
	void OnServerListInit(int param);
	void OnServerListBtnDown();
	void OnServerListOwnerDraw(int param);
	void OnChannelListInit(int param);
	void OnChannelListBtnDown();
	void OnChannelListOwnerDraw(int param);
	void OnConnectMessageInit(int param);
	void OnConnectCancleInit(int param);
	void OnConnectCancleUp();
	void OnMessageInit(int param);
	void OnCurServerInit(int param);
	void OnCloseBtnInit(int param);
	void OnPremiumTooltipInit983(int param); // 983 00af3c40 ("serverlist_premium_tooltip", INIT)
	void OnPremiumTooltipOwnerDraw983(int param); // 983 00af6770 (OWNERDRAW, D645)
	void OnCloseLBtnUp();

	void ConnectToServer();
	void TurnOnChannels();
	void TurnOffChannels();
	void OnWaitNotify(const char* msg, unsigned long style);
	void OpenQuitDlg();

	bool OnLoginDlgResult(int result, FrForm* pForm);
	bool OnNotifyResult(int result, FrForm* pForm);
	bool OnWaitNotifyResult(int result, FrForm* pForm);
	bool OnQuitDlgResult(int result, FrForm* pForm);

private:
	void ClearServerList();
	bool IsNotDrawServerList(FrListItem* pItem);
	bool IsDisableEnterChannel(unsigned long flag, unsigned char level);
	void DrawServerButton(const WPoint& pt, const sGameServerInfo* pInfo,
		bool bSelected);
	void DrawChannelButton(const WPoint& pt, const sChannelInfo* pInfo,
		bool bSelected);

protected:
	int m_pad983_110; // 983: sem leitura/escrita (ctor 00af4060 nao zera; +0x110 em af3751/af52a3 e de FrAutoConnect, ab2bea de outra FrForm)
	FrListBox* m_pServerList;
	FrListBox* m_pChannelList;
	FrStatic* m_pMessage;
	FrStatic* m_pConnectMessage;
	FrStatic* m_pCurServer;
	FrButton* m_pCloseBtn;
	// 983 (ctor 00af4060; A171): servico Premium no canal (00af3c40: controle +0x12c, "Premium service_on_00..02"/"_base"),
	// piscar em +0x140..0x148 (BuildChannelList 00af5020, OnProc 00af6c40); m_pLoginDlg desceu para +0x14c (00af32a0)
	FrWnd* m_pPremium983;
	const Bitmap* m_pImgPremiumOn983[3];
	const Bitmap* m_pImgPremiumBase983;
	int m_premiumFrame983;
	float m_premiumAlpha983;
	float m_premiumSpeed983;
	FrForm* m_pLoginDlg;
	gsLoginState_t m_state;
	float m_connectWaitTime;
	float m_reconnectDelay;
	bool m_pad983_15c; // 983: so zerado (ctor af4119, conexao af5651); sem leitura em 00af3000..00af8000; os cmp +0x15c do resto do exe sao this de outras classes
	bool m_bFirstLogin;
	bool m_bAlertMessage;
	bool m_bConnecting;
	float m_alertBlinkTime;
	sGameServerInfo m_curServer;
	sChannelInfo m_selChannel; // 983: +0x1c0, copia do canal escolhido (memset 0x55 no ctor 00af4228; memcpy em 00af6044)
	const Bitmap* m_pImgServerIcon[4];
	const Bitmap* m_pImgConnection;
	const Bitmap* m_pImgNew;
	tagWTITLEFONT m_fontInfo;
	WTitleFont* m_pFont;
	FrForm* m_pNotifyDlg;
	FrForm* m_pWaitDlg;
	FrLogoutDlg* m_pQuitDlg;
	cProgressBar* m_pProgressBar;
	FrAutoConnect* m_pAutoConnect;
	int m_loginReplyCount;
	FrButton* m_pConnectCancle;
	sGameServerInfo m_serverList[13]; // 983: 13 linhas vazias em +0x270 (ctor 00af40a8; BuildServerList 00af4c82)
	unsigned long m_pad983_71c; // 983: sem leitura/escrita (fim de sizeof 0x720; nenhum +0x71c em 00af3000..00af8000)
	static std::map<unsigned int, std::string> m_serverIconMap; // 983: global 00f03394 (OnInit 00af7626)

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbDrawchannelbuttoFunclets(); // HACK: EH funclets of the naked DrawChannelButton (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnchannellistbtnFunclets(); // HACK: EH funclets of the naked OnChannelListBtnDown (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOninitFunclets(); // HACK: EH funclets of the naked OnInit (nakedify)
#endif
};
