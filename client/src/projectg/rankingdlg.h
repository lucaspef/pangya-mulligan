#pragma once

#include "frcmdtarget.h"

class FrRankingDlg;

// HACK: the dialog classes are only visible to rankingdlg.cpp. Their inline bodies (GetRTTI, implicit members)
// add IL to every other includer and move the PCH inlines of those units (realmyroommainui).
#ifdef REBANG_RANKINGDLG_CPP
#include <list>
#include <string>
#include "frform.h"
#include "fredit.h"
#include "anotherhand.h"
#include "../shared/globalgamedefine.h"

class FrTextButton;
class FrButton;
class FrViewer;
class FrStatic;
class FrListBox;
class FrComboBox;
class FrArea;
class FrGaugeBar;
class Bitmap;
class CExhibition;
class CPartTidList;
class FrEmoticonDlg;
struct FrListItem;

namespace ranktype
{
	struct sListInfo
	{
		unsigned long uid;
		unsigned long rank;
		unsigned long prevRank;
		unsigned long value;
		unsigned short level;
		std::string id;
		std::string nickname;
	};

	struct sRankInfo
	{
		unsigned long rank;
		unsigned long prevRank;
		int value;
	};

#pragma pack(push, 1)
	struct sRankUserInfo
	{
		unsigned long uid;
		char id[0x16];
		char nickname[0x16];
		unsigned char level;
		unsigned char m_pad31983; // 983: byte do pacote 0x138a sem leitor (rankingdlg 00b20000-00b2c000, 0x138a 009d7805, RequestUccInfo 007e9f30 = +0x32)
		sCharacterInfo charInfo;
		unsigned char m_pad233983; // 983: 1 byte (charInfo 0x201 termina em +0x233; rank em +0x234 = diálogo +0x3a0, medido em 00b24c00-00b2b400)
		sRankInfo rank[6]; // 983: 6 (o pacote 0x138a lê 0x27c bytes, push em 009d7865; no 645 eram 5 e 0x22c)
	};
#pragma pack(pop)
	C_ASSERT(offsetof(sRankUserInfo, charInfo) == 0x32);
	C_ASSERT(offsetof(sRankUserInfo, rank) == 0x234);
	C_ASSERT(sizeof(sRankUserInfo) == 0x27c);
}

struct sTypeName
{
	int m_type;
	const char* m_name;
};

class FrRankingDlg : public FrForm, public CHandOwner
{
public:
	DECLARE_OBJECT(FrRankingDlg)
	FrRankingDlg();
	virtual ~FrRankingDlg();

protected:
	virtual bool OnInit();
	virtual void OnCancel();
	virtual void OnProc(const float delta);
	virtual void OnAnotherHand(int key, void* param);

	void ResetControl();
	void SetVisibleByRankType();
	void SetVisibleUserInfo(bool bVisible);
	void SetListItemInfo(FrListItem* pItem);
	void SetPetCharacter();
	void DrawUserInfoLine(int type, int line, WPoint& pos);
	void ShowUserInfo();
	void SynchronizeButtons();
	void SetAllControlsEnable(bool bEnable);
	void CloseAllComboBtns();
	void HidePet(bool bHide);
	void SendRequestRank(bool bFirst);
	void SendRequestUserInfo(unsigned long uid, const char* nickname);
	void SendRequestSearchUser(const char* nickname);

	void OnTermType_S5_BtnInit(int param);
	void OnTermType_S5_BtnUp();
	void OnCourseAreaInit(int param);
	void OnCourseAreaOwnerDraw(int param);
	void OnRankList_Init(int param);
	void OnRankList_OwnerDraw(int param);
	void OnRankList_LBtnDown();
	void OnRankList_PrevPageInit(int param);
	void OnRankList_PrevPageBtnUp();
	void OnRankList_NextPageInit(int param);
	void OnRankList_NextPageBtnUp();
	void OnRankList_CurrentPageInit(int param);
	void OnRankList_TotalPageInit(int param);
	void OnRankList_MyInfo_Init(int param);
	void OnRankList_MyInfo_LBtnDown();
	void OnRankList_MyInfo_OwnerDraw(int param);
	void OnRankList_NoInRank_Init(int param);
	void OnRankList_SearchUserInit(int param);
	void OnRankList_SearchUserBtnUp();
	void OnRankList_UserIdEditInit(int param);
	void OnRankList_UserIdEditDown();
	void OnEmoticonInit(int param);
	void OnEmoticonBtnUp();
	void OnLanguageInit(int param);
	void OnAboutBtnInit(int param);
	void OnAboutBtnUp();
	void OnCloseBtnUp();
	void OnBottomMessageInit(int param);
	void OnPetCharacterInit(int param);
	void OnPetCharacterOwnerDraw();
	void OnPetLoadInit(int param);
	void OnUpperMiniInfoInit(int param);
	void OnUpperMiniInfoOwnerDraw();
	void OnClassType_ComboInit(int param);
	void OnClassType_ComboDown(int param);
	void OnRankTypeListInit(int param);
	void OnRankTypeListLDown();
	void OnRankTypeListOwnerDraw(int param);
	void OnRankTypeViewerInit(int param);
	void OnRankTypeViewerDown();
	void OnRankTypeViewerOwnerDraw();
	void OnGeneralSubTypeListInit(int param);
	void OnGeneralSubTypeListLDown();
	void OnGeneralSubTypeListOwnerDraw(int param);
	void OnGeneralSubTypeViewerInit(int param);
	void OnGeneralSubTypeViewerDown();
	void OnGeneralSubTypeViewerOwnerDraw();
	void OnCourseTypeListInit(int param);
	void OnCourseTypeListLDown();
	void OnCourseTypeListOwnerDraw(int param);
	void OnCourseTypeViewerInit(int param);
	void OnCourseTypeViewerDown();
	void OnCourseTypeViewerOwnerDraw();
	void OnRecordTypeListInit(int param);
	void OnRecordTypeListLDown();
	void OnRecordTypeListOwnerDraw(int param);
	void OnRecordTypeViewerInit(int param);
	void OnRecordTypeViewerDown();
	void OnRecordTypeViewerOwnerDraw();
	void OnSearchTypeListInit(int param);
	void OnSearchTypeListLDown();
	void OnSearchTypeListOwnerDraw(int param);
	void OnSearchTypeViewerInit(int param);
	void OnSearchTypeViewerDown();
	void OnSearchTypeViewerOwnerDraw();
	void OnUserRankEditInit(int param);
	void OnGhostBtnInit(int param);
	void OnGhostBtnUp();
	bool OnEmoticonResult(int result, FrForm* pForm);
	bool OnAboutDlgResult(int result, FrForm* pForm);

	static int s_iRankType;
	static int s_iSubType;
	static int s_iTermType;
	static int s_iClassType;
	static int s_iPageNum;
	static int s_iPageTotal;
	static int s_iSearchType;
	static const int s_iListCountPerPage;

public:
	static void SetRankTypes(unsigned char rankType, unsigned char subType,
		unsigned char termType, unsigned char classType, int pageNum,
		int pageTotal);
	void UpdateRankList();
	void UpdateUserInfo();
	void DisconnectRankServer();
	void UpdateMyInfo();
	void SetBottomMsg(const char* msg);
	void OnCustomProc(float delta);
	void OnNotify(const char* msg, unsigned long type);
	void ClearRankList();
	void SelectRankList(int index);

	std::list<ranktype::sListInfo> m_rankList;
	ranktype::sListInfo m_myInfo;
	ranktype::sRankUserInfo m_userInfo;

protected:
	FrLine m_courseTypeLine[20];
	FrLine m_searchTypeLine[2];
	FrLine m_recordTypeLine[5];
	FrLine m_rankTypeLine[3];
	FrLine m_genSubTypeLine[6];
	FrButton* m_pTermS5Btn;
	FrComboBox* m_pClassTypeCombo;
	FrListBox* m_pRankList;
	FrListBox* m_pMyInfoList;
	FrArea* m_pCourseArea;
	FrWnd* m_pNoMyRankWnd;
	FrButton* m_pSearchUserBtn;
	FrEdit* m_pUserIdEdit;
	FrButton* m_pEmoticonBtn;
	FrArea* m_pLanguageArea;
	FrEdit* m_pCurPageEdit;
	FrEdit* m_pTotalPageEdit;
	FrEdit* m_pUserRankEdit;
	FrGaugeBar* m_pPetLoadGauge;
	FrStatic* m_pBottomMsg;
	FrEmoticonDlg* m_pEmoticonDlg;
	FrForm* m_pAboutDlg;
	FrListBox* m_pRankTypeList;
	FrViewer* m_pRankTypeViewer;
	const Bitmap* m_pRankTypeViewerImg[3];
	FrListBox* m_pGenSubTypeList;
	FrViewer* m_pGenSubTypeViewer;
	const Bitmap* m_pGenSubTypeViewerImg[3];
	FrListBox* m_pCourseTypeList;
	FrViewer* m_pCourseTypeViewer;
	const Bitmap* m_pCourseTypeViewerImg[3];
	FrListBox* m_pRecordTypeList;
	FrViewer* m_pRecordTypeViewer;
	const Bitmap* m_pRecordTypeViewerImg[3];
	FrListBox* m_pSearchTypeList;
	FrViewer* m_pSearchTypeViewer;
	const Bitmap* m_pSearchTypeViewerImg[3];
	FrArea* m_pPetCharacterArea;
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	FrButton* m_pGhostBtn;
	FrArea* m_pUpperMiniInfo;
	CAnotherHand m_anotherHand;
	float m_userInfoDelay;

private:
	DECLARE_FRESH_MSGMAP()
public:
};


class FrRankingAboutDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrRankingAboutDlg)
	FrRankingAboutDlg();
	virtual ~FrRankingAboutDlg();

	void SetImgAboutIndex(int index);

protected:
	virtual bool OnInit();

	void OnAboutViewerInit(int param);
	void OnIndex01Init(int param);
	void OnIndex01BnUp();
	void OnIndex02Init(int param);
	void OnIndex02BnUp();
	void OnIndex03Init(int param);
	void OnIndex03BnUp();
	void OnIndex04Init(int param);
	void OnIndex04BnUp();
	void OnBackwardBnInit(int param);
	void OnBackwardBnUp();
	void OnForwardBnInit(int param);
	void OnForwardBnUp();
	void OnOkBnInit(int param);
	void OnOkBnUp();

	FrTextButton* m_pIndexBtn[4];
	int m_aboutIndex;
	FrButton* m_pBackwardBtn;
	FrButton* m_pForwardBtn;
	FrViewer* m_pAboutViewer;

private:
	DECLARE_FRESH_MSGMAP()
};
#endif // REBANG_RANKINGDLG_CPP

class CRankingInfo : public WSingleton<CRankingInfo>, public FrCmdTarget
{
public:
	CRankingInfo();

	bool Open();
	void Close();
	void Process(float delta);
	void OnNotify(const char* msg, unsigned long type);

	FrRankingDlg* GetDlg() { return m_pDlg; }

protected:
	bool OnRankingDlgResult(int result, FrForm* pForm);

	FrRankingDlg* m_pDlg;
};

inline CRankingInfo* RANKING()
{
	return CRankingInfo::Instance();
}

inline FrRankingDlg* RANKINGDLG()
{
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: RANKING() is owned by a later unit in the original
	return CRankingInfo::Instance()->GetDlg();
#else
	return RANKING()->GetDlg();
#endif
}
