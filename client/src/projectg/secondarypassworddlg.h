#pragma once
#include <string>
#include <vector>
#include "frform.h"

namespace _gateway
{
	enum eSecondPwdInputType
	{
		SECONDPWD_INPUT_CREATE = 0,
		SECONDPWD_INPUT_LOGIN = 1
	};

	class FrPasswordInputDlg : public FrForm
	{
	public:
		DECLARE_OBJECT(FrPasswordInputDlg)

		FrPasswordInputDlg();
		virtual ~FrPasswordInputDlg();

		virtual bool OnInit();

	protected:
		void OnInitNumberSlot0(int param);
		void OnInitNumberSlot1(int param);
		void OnInitNumberSlot2(int param);
		void OnInitNumberSlot3(int param);
		void OnInitNumberSlot4(int param);
		void OnInitNumberSlot5(int param);
		void OnInitNumberSlot6(int param);
		void OnInitNumberSlot7(int param);
		void OnInitNumberSlot8(int param);
		void OnInitNumberSlot9(int param);
		void OnBackSpace_ButtonInit(int param);
		void OnClear_ButtonInit(int param);
		void OnCreatePassword_ButtonInit(int param);
		void OnConfirm_ButtonInit(int param);
		void OnChangePassword_ButtonInit(int param);
		void OnReport_ButtonInit(int param);

		void OnLButtonUpNumberSlot0();
		void OnLButtonUpNumberSlot1();
		void OnLButtonUpNumberSlot2();
		void OnLButtonUpNumberSlot3();
		void OnLButtonUpNumberSlot4();
		void OnLButtonUpNumberSlot5();
		void OnLButtonUpNumberSlot6();
		void OnLButtonUpNumberSlot7();
		void OnLButtonUpNumberSlot8();
		void OnLButtonUpNumberSlot9();
		void OnBackSpace_LButtonUp();
		void OnClear_LButtonUp();
		void OnCreatePassword_LButtonUp();
		void OnConfirm_LButtonUp();
		void OnChangePassword_LButtonUp();
		void OnReport_LButtonUp();

		void OnConfirmFirstPassword_ButtonInit(int param);
		void OnConfirmFirstPassword_LButtonUp();
		void OnCaption_AreaInit(int param);
		void OnExplaneBlock_AreaInit(int param);
		void OnExplaneBlock_AreaOwnerdraw(int param);
		void OnPasswordInput_EditInit(int param);
		void OnPasswordConfirm_EditInit(int param);
		void OnInputAgain_ButtonInit(int param);
		void OnInputAgain_LButtonUp();
		void OnClose_ButtonInit(int param);
		void OnClose_LButtonUp();

	public:
		void SetDlgType(eSecondPwdInputType type) { m_dlgType = type; }

	private:
		enum eInputSequence
		{
			INPUT_NONE = 0,
			INPUT_FIRST = 1,
			INPUT_CONFIRM = 2
		};

		void InitVariable();
		void LoadResource();
		bool ConvertCreateInput();
		bool ConvertConfirmInput();
		bool SetButtonProperty(int index, int param);
		void PushNumButton(int slot);
		void InsertPasswordChar(int number);
		void ErasePasswordChar();
		void ClearPassword();
		bool ChangeInputSequence(eInputSequence sequence);
		bool CheckInputPassword();
		void ArrangeButtonImage();
		void DrawCreateDlgExplane(FrGraphicInterface* gdi);
		void DrawLoginDlgExplane(FrGraphicInterface* gdi);
		void CheckEnableCreateButton();

		eSecondPwdInputType m_dlgType;
		eInputSequence m_inputSequence;
		FrArea* m_pCaption;
		FrEdit* m_pPasswordEdit[2];
		FrEdit* m_pCurrentEdit;
		FrArea* m_pExplaneBlock;
		FrButton* m_pButton[19];
		const Bitmap* m_pBitmap[13];
		std::vector<int> m_numberList;
		std::string m_password;
		std::string m_firstPassword;

		DECLARE_FRESH_MSGMAP()
	};

	class FrSelectCertification : public FrForm
	{
	public:
		DECLARE_OBJECT(FrSelectCertification)

		FrSelectCertification();
		virtual ~FrSelectCertification();

		virtual bool OnInit();

	protected:
		void OnRequestCertify_ButtonInit(int param);
		void OnRequestCertify_LButtonDown();
		void OnPassCertify_ButtonInit(int param);
		void OnPassCertify_LButtonDown();
		void OnLeft_EditInit(int param);
		void OnRight_EditInit(int param);
		void OnInFrame_AreaInit(int param);
		void OnInFrame_AreaOwnerDraw(int param);

		FrButton* m_pButton[2];
		FrEdit* m_pEdit[2];
		FrArea* m_pInFrame;
		const Bitmap* m_pBitmap[4];

	private:
		DECLARE_FRESH_MSGMAP()
	};

	class FrExplanePassword : public FrForm
	{
	public:
		DECLARE_OBJECT(FrExplanePassword)

		FrExplanePassword();
		virtual ~FrExplanePassword();

	protected:
		void OnOpenPasswordCreate_ButtonInit(int param);
		void OnOpenPasswordCreate_LButtonUp();
		void OnExplane_EditInit(int param);

		FrButton* m_pOpenCreateBtn;
		FrEdit* m_pExplaneEdit;

	private:
		DECLARE_FRESH_MSGMAP()
	};

	class FrPasswordNotifyDlg : public FrForm
	{
	public:
		DECLARE_OBJECT(FrPasswordNotifyDlg)

		FrPasswordNotifyDlg();
		virtual ~FrPasswordNotifyDlg();

		virtual bool OnInit();

	protected:
		void OnIcon_AreaInit(int param);
		void OnIcon_AreaOwnerdraw(int param);
		void OnSystemMsg_EditInit(int param);
		void OnClose_ButtonInit(int param);
		void OnClose_LButtonUp();

	public:
		void SetMessage(const char* message);

		void SetNotifyType(int type) { m_notifyType = type; }

	protected:
		int m_notifyType;
		int m_iconType;
		int m_pad645_118; // 645: sem leitura/escrita (645 e nosso); a classe nao existe no 983 (sem nome nem bitmaps 2ndPW_* no dump)
		FrArea* m_pIcon;
		FrEdit* m_pSystemMsg;
		FrButton* m_pCloseBtn;
		const Bitmap* m_pBitmap[3];
		std::string m_message;
		bool m_bCustomMessage;

	private:
		DECLARE_FRESH_MSGMAP()
	};

}
