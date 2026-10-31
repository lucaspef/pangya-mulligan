#pragma once

#include "frform.h"

class NewsReader;
class FrButton;
class FrEdit;

class FrGuildNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrGuildNoticeDlg)

	FrGuildNoticeDlg();
	virtual ~FrGuildNoticeDlg();

	void DataBind(const NewsReader* reader, unsigned int index);
	void Prev();
	void Next();

protected:
	void SetArticle(unsigned int index);

	void OnClose_Init(int param);
	void OnIndex_Init(int param);
	void OnArticle_Init(int param);
	void OnPrev_Init(int param);
	void OnNext_Init(int param);
	void OnClose_LBtnUp();
	void OnPrev_LBtnUp();
	void OnNext_LBtnUp();

	FrButton* m_pClose;
	FrEdit* m_pIndex;
	FrEdit* m_pArticle;
	FrButton* m_pPrev;
	FrButton* m_pNext;
	const NewsReader* m_pReader;
	unsigned int m_curArticle;

	DECLARE_FRESH_MSGMAP()
};
