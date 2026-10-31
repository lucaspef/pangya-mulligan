#pragma once

#include <string>
#include <vector>
#include "frform.h"
#include "noticeimage.h"

class FrButton;
class FrViewer;
class FrListBox;
class FrArea;

class FrNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrNoticeDlg)

	FrNoticeDlg();
	virtual ~FrNoticeDlg();

	void LoadInitFile(const char* filename); // 983: 00ae4cd0 (le o arquivo e chama RefreshImages983)
	void LoadOnly(const char* filename); // 983: 00ae5610
	// 983 (pedido B56): titulo do aviso (00ae4d00) e lista de imagens ja baixadas (00ae5710; so as nao vazias, sem limpar)
	void SetCaption983(const std::string& caption);
	void SetImages983(const std::vector<std::string>& files);
	void Prev();
	void Next();

	unsigned long GetTypeId() const { return m_typeId; }

protected:
	virtual bool OnInit();
	virtual void OnProc(const float deltaTime); // 983: 00ae5200

	void OnCaptionInit(int param); // 983: 00ae48b0
	void OnCancelInit(int param);
	void OnViewInit(int param);
	void OnViewBtnDown();
	void OnNumInit(int param);
	void OnNumLBtnUp();
	void OnNumOwnerDraw(int param);
	void OnPrevInit(int param);
	void OnPrevBtnUp();
	void OnNextInit(int param);
	void OnNextBtnUp();

	void RefreshImages983(); // 983: 00ae45e0 (o resto do LoadInitFile do 645: indice, setas e numeros)
	void SetScrollBar();
	void OpenImage();

	FrArea* m_pCaption; // 983: "caption" 0x110 (handler 00ae48b0); nome meu
	FrButton* m_pCancel;
	FrViewer* m_pViewer;
	std::vector<sNoticeImage> m_images;
	unsigned char m_curIndex;
	FrButton* m_pPrev;
	FrButton* m_pNext;
	FrListBox* m_pNumList;
	int m_openedIndex;
	bool m_bScrollBar;
	unsigned long m_typeId;
	int m_startIndex;

	DECLARE_FRESH_MSGMAP()
};
