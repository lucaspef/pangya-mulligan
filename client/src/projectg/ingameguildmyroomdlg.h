#pragma once

#include <string>
#include "frform.h"

class FrButton;
class FrGaugeBar;
class FrComboBox;
class FrArea;
class FrEdit;
class FrListBox;
class FrStatic;
class FrViewer;
class WTitleFont;

namespace _guild
{

	class FrCreateGuildKit : public FrForm
	{
	public:
		DECLARE_OBJECT(FrCreateGuildKit)

		FrCreateGuildKit();
		virtual ~FrCreateGuildKit();

		void SetConfirmGuildName(const char* name);

	protected:
		void OnInitExistGuildNameButton(int param);
		void OnLButtonUpExistGuildNameButton();
		void OnExistGuildName_EnterKey(int param);
		void OnGuildRegister_ButtonInit(int param);
		void OnGuildRegister_LButtonUp();
		void OnCancel_ButtonInit(int param);
		void OnCancel_LButtonUp();
		void OnInitInputGuildNameEdit(int param);
		void OnInitInputGuildIntroduceEdit(int param);

		FrButton* m_pButton[3];
		FrEdit* m_pEdit[2];
		std::string m_confirmedName;

	private:
		void InitControl();

		DECLARE_FRESH_MSGMAP()
	};

	class FrRegisterGuildMark : public FrForm
	{
	public:
		DECLARE_OBJECT(FrRegisterGuildMark)

		FrRegisterGuildMark();
		virtual ~FrRegisterGuildMark();

		virtual bool OnInit();
		void RequestUpload(int guildIdx, const std::string& path);

	protected:
		void OnMarkSearch_ButtonInit(int param);
		void OnMarkSearch_LButtonUp();
		void OnConfirm_ButtonInit(int param);
		void OnConfirm_LButtonUp();
		void OnCancel_ButtonInit(int param);
		void OnCancel_LButtonUp();
		void OnMarkViewer_AreaInit(int param);
		void OnMarkViewer_OwnerDraw(int param);
		void OnMarkPath_EditInit(int param);

		FrButton* m_pButton[3];
		FrEdit* m_pMarkPath;
		FrArea* m_pMarkViewer;
		Bitmap* m_pMarkBitmap;
		std::string m_markPath;
		int m_windowMode;
		bool m_bMarkSelected;

	private:
		DECLARE_FRESH_MSGMAP()
	};

	class FrChangeGuildName : public FrForm
	{
	public:
		DECLARE_OBJECT(FrChangeGuildName)

		FrChangeGuildName();
		virtual ~FrChangeGuildName();

		bool IsHaveGuild();
		void SetConfirmedName(const char* name);

	protected:
		void OnOriginalName_StaticInit(int param);
		void OnInitExistGuildNameButton(int param);
		void OnLButtonUpExistGuildNameButton();
		void OnGuildNameConfirm_ButtonInit(int param);
		void OnGuildNameConfirm_LButtonUp();
		void OnGuildNameCancel_ButtonInit(int param);
		void OnGuildNameCancel_LButtonUp();
		void OnInitChangeGuildNameEdit(int param);

		FrStatic* m_pOriginalName;
		FrEdit* m_pGuildNameEdit;
		FrButton* m_pButton[3];
		std::string m_confirmedName;

	private:
		DECLARE_FRESH_MSGMAP()
	};
}
