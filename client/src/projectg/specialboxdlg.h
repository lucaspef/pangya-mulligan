#pragma once
#include "frform.h"
class FrArea;
class FrEdit;
class FrGaugeBar;
class Bitmap;
namespace _gateway
{
	class FrSpecialBoxDlg : public FrForm
	{
		DECLARE_OBJECT(FrSpecialBoxDlg)

		FrSpecialBoxDlg();

		virtual ~FrSpecialBoxDlg() { }

		virtual bool OnInit();
		virtual void OnProc(const float fElapsed);

		void SetMessage(const char* msg);
		void SetMessageType(int type);
		void DisbleProgressBar();

	protected:
		void OnNotifyAreaInit(int param);
		void OnNotifyAreaOwnerdraw(int param);
		void OnExplaneEditInit(int param);
		void OnProgressBarInit(int param);

		FrArea* m_pNotifyArea;
		FrEdit* m_pExplaneEdit;
		FrGaugeBar* m_pProgressBar;
		const Bitmap* m_pKeyItemBitmap;
		bool m_bDisableProgress;
		float m_fOpenDelay;
		float m_fElapsed;

		DECLARE_FRESH_MSGMAP()
	};
}
