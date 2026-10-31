#pragma once

#include "frform.h"

class FrEdit;
class FrStatic;
class FrArea;

class FrNoteDlg : public FrForm
{
	DECLARE_OBJECT(FrNoteDlg)

	FrNoteDlg()
		: m_pNick(NULL),
		  m_pNote(NULL),
		  m_pNoteStatic(NULL),
		  m_pNickStatic(NULL),
		  m_pReceived(NULL),
		  m_pReceivedStatic(NULL),
		  m_pPaid(NULL),
		  m_elapsed(0.0f),
		  m_bFocused(false)
	{
	}

	void SetID(const char* nick, const char* received);
	void SetNick(const char* nick, const char* received);
	const char* GetNote();

protected:
	virtual void OnProc(const float deltaTime);

	void OnPaidInit(int param);
	void OnNickInit(int param);
	void OnNickStaticInit(int param);
	void OnReceivedInit(int param);
	void OnReceivedStaticInit(int param);
	void OnNoteInit(int param);
	void OnNoteLBtnDown();
	bool OnNoteEnterKey(int param);
	void OnNoteStaticInit(int param);
	void OnOKBtnUp();

	FrEdit* m_pNick;
	FrEdit* m_pNote;
	FrStatic* m_pNoteStatic;
	FrStatic* m_pNickStatic;
	FrEdit* m_pReceived;
	FrStatic* m_pReceivedStatic;
	FrArea* m_pPaid;
	float m_elapsed;
	bool m_bFocused;

	DECLARE_FRESH_MSGMAP()
};
