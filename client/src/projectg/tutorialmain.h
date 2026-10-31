#pragma once
// JP 983: o CTutorialMain e um FrTaskForm (RTTI do COL 00d740ec: CTutorialMain : FrTaskForm; vtables 00d28dbc/00d28dac/
// 00d28da4; sizeof 0x460 = operator new de 007d5090/007d5170; unidade 007d1fc0-007d5540). No 645 era o ator principal
// (CTaskMain) da CTutorialTask; no 983 abre como janela por cima do lobby (form "tutorialmain", CTutorialWidget da B) e
// o estado da licao escolhida foi para o documento 0xa4 (CTutorialContainer983, +0x458). Sessao 124.
#include "taskmain.h"
#include "jp/taskform.h"
#include <mmsystem.h>
#include <list>
#include <string>
#include <vector>

class CExhibition;
class FrListBox;
class FrArea;
class FrQuestGift;
class FrQuestInfoDlg;
class CTutorialContainer983;
class MsgObject;

class CPipinTime
{
public:
	// 983: inline (o construtor/destrutor do CTutorialMain, o ShowPet e o OnCloseWidget983 embutem); copias fora de
	// linha em 007d1fc0/007d1fd0/007d3be0/007d3c00
	CPipinTime() : m_start(0), m_current(0), m_bActive(false) { }
	~CPipinTime() { }
	void Reset()
	{
		m_start = 0;
		m_current = 0;
		m_bActive = false;
	}
	void SetTime() { m_current = m_start = timeGetTime(); }

	unsigned long m_start;
	unsigned long m_current;
	bool m_bActive;
	std::string m_name;
};

class CTutorialMain : public FrTaskForm
{
	DECLARE_OBJECT(CTutorialMain)
public:
	CTutorialMain(); // 007d4820
	virtual ~CTutorialMain(); // 007d4380

	struct sTutorialInfo
	{
		int id;
		unsigned char enabled;
		unsigned char done;
	};
	struct sTutorialMap;
	struct sCategoryInfo;
	struct sArrowInfo;
	class CQuestInfo
	{
	public:
		CQuestInfo();
		~CQuestInfo();
		std::vector<int> children;
		unsigned char total;
		unsigned char count;
	};

	virtual void Enable(bool enable); // 007d2460 (slot 6): FrWnd::Enable + FrWnd::SetVisible
	virtual bool OnOpenWidget983(); // 007d2490 (+0x98): pega o documento 0xa4 e registra as imagens
	virtual bool OnCloseWidget983(); // 007d4030 (+0x9c): zera o relogio do pipin e a lista
	virtual bool OnMsg983(const MsgObject& msg); // 007d40a0 (+0xa0)

	void CheckNodeStates();
	void OpenQuest(int category, int index);
	void OpenQuestGift();
	void EnableAllBtn();
	void HidePet();
	void ShowPet();
	int GetSelectedBtn(int area, WPoint pt);
	void DecideNextMotion(unsigned char motion);
	void SetCompleteStates();
	bool IsCompleteQuest(int category, int index);
	bool IsCompleteCategory(int category);

protected:
	virtual bool OnInit(); // 007d4ba0 (slot 32)
	virtual void OnProc(float dt); // 007d4e80 (slot 9)

	void LoadTutorialMainImages();
	void ProcessAnimateIcon();
	void OnTutorialAreaPipin(int param);
	void OnTutorialAreaPipinOwnerDraw(int param);
	void OnTutorialListInit(int param);
	void OnTutorialListOwnerDraw(int param);
	void OnTutorialListLBtnDown();
	void OnTutorialListLBtnUp();
	void OnCloseWndUp983(); // 007d22b0: botao "close_wnd"

public:
	void OpenGradeGift(unsigned char grade);

protected:
	bool OnGiftDlgResult(int result, FrForm* form);
	void InitGolfTutorial();
	sTutorialInfo* FindTutorialList(int& id);
	void CheckEnable(int index);
	void MissionClear(int& id, unsigned long& flags);
	void MissionComplete(unsigned long flags);
	void OpenTutorialInfoDlg();
	void StartSelectedTutorial();
	bool OnTutorialInfoDlgResult(int result, FrForm* form);
	bool OnQuestionFormResult(int result, FrForm* form);
	void DrawCourseArrow();
	void SetQuestInformation(sTutorialMap* map);
	void SetQuestInformation(int a, int b);

private:
	int IsInArea(int area, WPoint pt);

	DECLARE_FRESH_MSGMAP()

#pragma pack(push, 1)
	struct sSelect
	{
		unsigned short a;
		unsigned char b;
		void Clear()
		{
			unsigned char zero = 0;
			a = zero;
			b = zero;
		}
	};
#pragma pack(pop)

	static sCategoryInfo m_sCategoryItem[3];
	static sTutorialMap m_Tutorials[14];
	struct sTutorialResult
	{
		unsigned char id;
		int image;
	};
	static sTutorialResult m_CompleteItems[3];
	static sArrowInfo m_ArrowItems[10];

	// 983: o FrTaskForm tem 0x120 (m_pTask em +0x110 + alinhamento 16 do FrForm); os campos do 645 (que comecavam em
	// +0x1e0 depois do CTaskMain) estao 0xc0 antes
	FrListBox* m_pTutorialList; // +0x120
	char m_pad124[0x20];
	FrForm* m_pNotifyForm; // +0x144
	FrQuestInfoDlg* m_pInfoDlg; // +0x148
	FrQuestGift* m_pQuestGift; // +0x14c
	char m_pad150[4];
	FrArea* m_pAreaPipin; // +0x154
	char m_pad158[4];
	CExhibition* m_pExhibition; // +0x15c
	float m_cameraDir; // +0x160
	bool m_bCameraReturn; // +0x164
	char m_pad165[3];
	unsigned long m_cameraReturnTime; // +0x168
	unsigned long m_lastTime; // +0x16c
	const Bitmap* m_pBitmaps[64]; // +0x170
	WPoint m_areaRects[8]; // +0x270 (983: o construtor copia cada par de um WPoint temporario)
	WPoint m_areaRects2[6]; // +0x2b0
	float m_arrowX[5]; // +0x2e0
	sSelect m_select; // +0x2f4 (645 +0x3cc; o MissionClear/MissionComplete do 983 gravam o byte 0 aqui)
	char m_pad2f7;
	// 645 m_questState/m_questCategory/m_questBegin/m_questEnd/m_reserved3cf/m_giftGrade (+0x3b8..+0x3d3) sairam:
	// o 983 guarda categoria/indice no documento 0xa4 (m_pTutorialDoc983) e o presente pendente em +0x14 dele
	CQuestInfo m_QuestInfo[14]; // +0x2f8
	std::list<sTutorialInfo> m_tutorialList; // +0x410
	unsigned char m_curQuest; // +0x41c
	char m_pad41d[3];
	CPipinTime m_pipinTime; // +0x420
	int m_reserved524; // +0x448
	float m_iconOffset; // +0x44c
	float m_reserved52c; // +0x450 (983: gravado como float 0)
	unsigned char m_categoryCount; // +0x454
	// 983 +0x458: documento 0xa4 do CContentsDoc (CTutorialContainer; pedido C58.1), pego no OnOpenWidget983 (007d2490)
	CTutorialContainer983* m_pTutorialDoc983;
	int m_pad45c; // +0x45c
};
