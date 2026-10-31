#pragma once

#include "frform.h"

class FrListBox;
class FrEdit;
class TiXmlDocument;
struct FrListItem;

class FrGuildDlg : public FrForm
{
	DECLARE_OBJECT(FrGuildDlg)

	FrGuildDlg();
	virtual ~FrGuildDlg();

protected:
	virtual bool OnInit();

	void OnBoardInit(int param);
	void OnBoardOwnerDraw(int param);
	void OnBoardBtnDown();
	void OnBoardBtnUp();
	void OnViewInit(int param);
	void OnViewBtnUp();

	bool DownLoad(const char* localFile, const char* remoteFile,
		const char* localPath, const char* remotePath);
	void LoadRSS(const char* filename);
	void ParseRSS(TiXmlDocument& doc);
	void UnloadRSS();
	void SelectTitle(FrListItem* pItem);

	FrListBox* m_pBoard;
	FrEdit* m_pView;

	DECLARE_FRESH_MSGMAP()
};
