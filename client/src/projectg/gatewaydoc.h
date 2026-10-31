#pragma once

#include <vector>

#include "contentsdoc.h"
#include "gatewayinterface.h"

enum eSecondPasswordState;
class FrNoticeDlg;

namespace _gateway
{
	class FrExplanePassword;
	class FrPasswordInputDlg;
	class FrSelectCertification;
	class FrPasswordNotifyDlg;
	class CGatewayControlDoc : public IContentsDataContainer
	{
	public:
#ifdef REBANG_ICONTENTSDOC
		// HACK: see IContentsDataContainer (contentsdoc.h)
		CGatewayControlDoc(int type = 0)
			: m_type(type)
		{
			if (0)
				GUILD_INFO();
		}
		virtual CGatewayHandler* GetGatewayHandler() { return NULL; }
		virtual ~CGatewayControlDoc()
		{
			if (0)
				GUILD_INFO();
		}
#else
		CGatewayControlDoc(int type = 0);
		virtual ~CGatewayControlDoc();

		virtual CGatewayHandler* GetGatewayHandler() { return NULL; }
#endif
		// JP 983 (pedido B13): dois inteiros depois do vptr. +0x04 = máscara da tarefa que pediu o manipulador, gravada
		// pelo CGatewayActor::QueryHandler (0091e96d); +0x08 = tipo do documento, gravado pelo construtor do 983
		// (004b5f90, que recebe o tipo).
		void SetTaskMask983(int taskMask) { m_taskMask = taskMask; }
		int GetTaskMask983() const { return m_taskMask; }
		// tipo 0 = classe do 645 que o 983 não tem mais (CSecondPwdDoc, CTradeDoc)
		localContentType_t GetType983() const { return (localContentType_t)m_type; }

	protected:
		int m_taskMask; // +0x04
		int m_type; // +0x08
	};

#ifdef REBANG_GATEWAYDOC_CPP
	// HACK: gatewaydoc.cpp only (other includers keep their IL). Defined before the derived
	// classes: they wait for the IContentsDataContainer inlines of the PCH, and the inline
	// destructors of the derived classes wait for them (emission order).
	inline CGatewayControlDoc::CGatewayControlDoc(int type)
		: m_type(type)
	{
	}

	inline CGatewayControlDoc::~CGatewayControlDoc()
	{
	}
#endif

	class CSecondPwdDoc : public CGatewayControlDoc
	{
	public:
		CSecondPwdDoc();
		virtual ~CSecondPwdDoc() { }

		virtual bool Initialize();
		virtual bool Release();

		void InitializeForLogout();

		virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; }

		void SetPasswordState(eSecondPasswordState state)
		{
			m_passwordState = state;
		}
		eSecondPasswordState GetPasswordState() { return m_passwordState; }

		void SetRemainDay(int day) { m_remainDay = day; }
		int GetRemainDay() { return m_remainDay; }

		void IncreseInvalidInputPassword() { ++m_invalidInputCount; }

		void SetSecondPasswordConfirm() { m_bSecondPasswordConfirm = true; }
		bool GetSecondPasswordConfirm() { return m_bSecondPasswordConfirm; }

		void SetCertifyFlag() { m_bCertifyFlag = true; }
		bool GetCertifyFlag() { return m_bCertifyFlag; }

	private:
		CGatewayHandler* m_pHandler;
		eSecondPasswordState m_passwordState;
		int m_remainDay;
		int m_invalidInputCount;
		bool m_bSecondPasswordConfirm;
		bool m_bCertifyFlag;
	};

	class CGolfItemDoc : public CGatewayControlDoc
	{
	public:
		CGolfItemDoc(int type = 0xa9); // 983: 009204f0, criador 00920640 passa 0xa9
		virtual ~CGolfItemDoc() { }

		virtual bool Initialize();
		virtual bool Release();

		virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; }

		void DeceaseCalipersCount();
		void DecreaseItemCount2c983(); // 0091fff0
		void SetItemCount2c983(int count) { m_2c = count; }
		int GetItemCount2c983() const { return m_2c; }
		void ResetCalipersVariable();

		void SetCalipersCount(int count) { m_calipersCount = count; }
		int GetCalipersCount() const { return m_calipersCount; }

		void EnableCalipers(bool bEnable)
		{
			m_usableItemList[0] = (bEnable == true); // 983: indice 0 (009197f0/00919810); o time booster saiu
		}
		bool GetEnableCalipers() const
		{
			return m_usableItemList[0] == 1 ? true : false;
		}

		void IncreaseSpecialBoxCount() { ++m_specialBoxCount; }
		void ResetSpecialBoxCount() { m_specialBoxCount = 0; }
		int GetSpecialBoxCount() const { return m_specialBoxCount; }

		void SetOwnSpecialBoxCount(int count) { m_ownSpecialBoxCount = count; }
		int GetOwnSpecialBoxCount() const { return m_ownSpecialBoxCount; }

	private:
		// JP 983: sizeof 0x30 (new do criador 00920640). O construtor só zera +0x1c/+0x20/+0x24 (o vetor); +0x10 é o
		// contador do calibrador (0091ffd0). Ordem dos dois contadores de caixa (+0x14/+0x18) provável, não medida;
		// +0x2c é um contador novo com o mesmo decremento (0091fff0), uso não levantado.
		CGatewayHandler* m_pHandler; // +0x0c
		int m_calipersCount; // +0x10
		int m_specialBoxCount; // +0x14
		int m_ownSpecialBoxCount; // +0x18
		std::vector<int> m_usableItemList; // +0x1c
		int m_2c; // +0x2c
	};

	class CTradeDoc : public CGatewayControlDoc
	{
	public:
		CTradeDoc();
		virtual ~CTradeDoc();

		virtual bool Initialize();
		virtual bool Release();

		void SetPackageSale(bool bPackageSale)
		{
			m_bPackageSale = bPackageSale;
		}
		bool GetPackageSale() const { return m_bPackageSale; }

		virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; }

	private:
		CGatewayHandler* m_pHandler;
		bool m_bPackageSale;
	};

	class CSpecialBoxDoc : public CGatewayControlDoc
	{
	public:
		virtual ~CSpecialBoxDoc() { }
#ifdef REBANG_ICONTENTSDOC
		// HACK: see IContentsDataContainer (contentsdoc.h)
		CSpecialBoxDoc(int type = 0xab)
			: CGatewayControlDoc(type)
		{
		}
#else
		CSpecialBoxDoc(int type = 0xab); // 983: 009201f0, criador 00920260 passa 0xab
#endif

		virtual bool Initialize();
		virtual bool Release();

		virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; }

	private:
		CGatewayHandler* m_pHandler;
	};

	// JP 983 (pedido B126.1): documento 0xae do evento da oficina de 2013 (RTTI .?AVCWorkShop2013Doc@_gateway@@, vtable 00d3c7b4,
	// construtor 00920280, 0x10 bytes pelo `new` do criador 009202c0, registro 00920900). O manipulador é o
	// CWorkShopEvent2013Handler da B (jp/workshopevent2013handler.h), que não deriva do nosso CGatewayHandler: guardado pela visão
	// CGatewayHandlerJp, como o CArinBirthDay2014Doc da B.
	class CWorkShop2013Doc : public CGatewayControlDoc
	{
	public:
		virtual ~CWorkShop2013Doc() { } // 00920400
#ifdef REBANG_ICONTENTSDOC
		CWorkShop2013Doc(int type = 0xae)
			: CGatewayControlDoc(type)
		{
		}
#else
		CWorkShop2013Doc(int type = 0xae); // 00920280
#endif

		virtual bool Initialize(); // 009202e0
		virtual bool Release(); // 00920030

		virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; } // 009202a0

	private:
		CGatewayHandler* m_pHandler; // +0x0c
	};

	class CSecondaryPassword : public CGatewayHandler, public FrCmdTarget
	{
	public:
		CSecondaryPassword();
		virtual ~CSecondaryPassword();

		virtual bool Initialize();
		virtual bool Execute(const MsgObject& msg);

		bool OnPasswordExplaneFormResult(int result, FrForm* pForm);
		bool OnInputPasswordFormResult(int result, FrForm* pForm);
		bool OnSelectCertifyFormResult(int result, FrForm* pForm);
		bool OnSystemMsgFormResult(int result, FrForm* pForm);

	private:
		void OnClosePasswordDlg();
		void OnCloseSelectCertifyDlg();
		void OnWaitGameServerList();
		void OnRequestStoreCertifyFlag(int param);
		void OnOpenSelfCertifyWebPage();
		void OnNotifyPasswordInfo(int param);
		void OnRequestAvailableContents(int param);
		void OnNotifyCannotUseContents();
		void OnNotifySecondPasswordConfirm();
		void OnRequestQualifyLogin(int param);
		void OnRequestRegistPassword(int param);
		void OnOpenPasswordExplaneDlg();
		void OnOpenPasswordCreateDlg();
		void OnOpenPasswordLoginDlg();
		void OnOpenSelectCertifyDlg();
		void OnRequestLoginPassword(int param);
		void OnOpenSystemMsg(int msg);
		void OnOpenSystemMsg(int msg, const char* text);
		void OnResultRegistPassword(int param);
		void OnRequestCheckCertifyFlag();
		void OnResultLoginPassword(int param);
		void OnRequestCheckPasswordValid(int param);
		void OnNotifySystemMsg(int param);

		FrExplanePassword* m_pExplanePasswordDlg;
		FrPasswordInputDlg* m_pPasswordInputDlg;
		FrSelectCertification* m_pSelectCertifyDlg;
		FrPasswordNotifyDlg* m_pPasswordNotifyDlg;
	};

	class CGolfItemHandler : public CGatewayHandler, public FrCmdTarget
	{
	public:
		CGolfItemHandler();
		virtual ~CGolfItemHandler();

		virtual bool Initialize();
		virtual bool Execute(const MsgObject& msg);

	protected:
		// 983: OnCloseWizCityNoticeResult/OnRequestWizCityNoticeDlg sairam (sem "hat_event_explanation" no exe)

	private:
		int CalcRemainMoreGetSpecialBoxCount(int count);
		void OnRequestResetCalipers();
		void OnGetCountCalipers(int param);
		void OnSetCountCalipers(int param);
		void OnDecreaseCalipers();
		void OnIncSpecialBox();
		void OnGetSpecialBox(int param);
		void OnResetSpecialBox();
		void OnCheckSomeMoreGetSpecialBox();
		void OnGetFlagCalipers(int param);
		void OnSetFlagCalipersEnable();
		void OnSetFlagCalipersDisable();
		// 983: sub-codigos 0xc, 0xd, 0xe e 0x11 (nomes provisorios)
		void OnGetFlagItem2c983(int param);
		void OnSetCountItem2c983(int param);
		void OnDecreaseItem2c983();
		void OnSetValue24_983(int param);
		// 983: sub-codigos 0x10, 0x12 e 0x17-0x1f (s72; nomes provisorios)
		void OnSelectSetPart983(int param); // 0x10 00919ab0
		void OnCheckGaugeFull983(int param); // 0x12 00919b50
		void OnRollPartTable17_983(int param); // 0x17 00919b70
		void OnNoticeUser18_983(int param); // 0x18 0091a100
		void OnFireController19_983(int param); // 0x19 00919d90
		void OnRollPartTable1a_983(int param); // 0x1a 00919c10
		void OnNoticeUser1b_983(int param); // 0x1b 0091a2a0
		void OnFindMyChar1c_983(int param); // 0x1c 00919d50
		void OnNoticeUser1d_983(int param); // 0x1d 0091a5e0
		void OnRollPartTable1e_983(int param); // 0x1e 00919cb0
		void OnNoticeUser1f_983(int param); // 0x1f 0091a440
		void SetGaugeFull983(); // 009198d0
		bool IsSetPartGround983(int kind); // 00919970

		int m_value24_983; // +0x24 (no 645: FrNoticeDlg* m_pWizCityNoticeDlg); 00919ab0 guarda aqui o item em uso
	};

	class CTradeHandler : public CGatewayHandler, public FrCmdTarget
	{
	public:
		CTradeHandler();
		virtual ~CTradeHandler();

		virtual bool Initialize();
		virtual bool Execute(const MsgObject& msg);

	private:
		void OnSetEnablePackageSale();
		void OnSetDisablePackageSale();
		void OnGetPackageSale(int param);
		void OnNotifyTradeErrorCode(int param);
		void OnUpdateTradeMinusItem(int param);
		void OnUpdateTradeSellItem(int param);
		void OnUpdateTradeBuyItem(int param);
	};
}
