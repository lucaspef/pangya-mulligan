#pragma once

#include <list>
#include <map>

#include "../shared/globalgamedefine.h"

#include "actor.h"
#include "frcmdtarget.h"
#include "anotherhand.h"
#include "taskmain.h"
#include "scenemanager.h"
#include "wmath.h"
#include "wtypes.h"
#include "frlistbox.h"

class WResource;
class WTitleFont;
class CCameraWork;
struct sSpecialTrophy;
class Bitmap;
class CCardBook;
class CExhibition;
class CFurniture_AbilityDlg;
class CFxSequence;
class CPartTidList;
class CPost_ReciveDlg;
class CPost_SendDlg;
class CRealMyRoom_AuthorityDlg;
class FrArea;
class FrGaugeBar;
class FrListBox;
class FrGroupBox;
class FrWnd;
class FrContextMenuCtrl;
class FrButton;
class FrCardRemove;
class FrCardUseDlg;
class FrDowngradeDlg;
class FrEdit;
class FrEnchantDlg;
class FrFortuneDlg;
class FrGraphicInterface;
class FrItemAttachCardDlg;
class FrItemStorageAccessPanel;
class FrItemStorageNotice;
class FrMascotInfoDlg;
class FrNoteDlg;
class FrFortuneOpenDlg;
class FrOpenCardPackDlg;
class FrPangGiftDlg;
class FrReplayModeDlg;
class FrStatic;
class FrTabButton;
class FrUccCopyDlg;
class FrUccDrawDlg;
class FrUniteResultDlg;
class FrViewer;
class WOverlay;
struct sCaddieInfo;
struct sGiftInfo;
struct sItemInfo;

// 0x6c bytes copied from CSharedDoc +0x628 on OnInit
struct sMyRoomEnterInfo
{
	unsigned long data[27];
};

// Nota: so CADDIE=1/MASCOT=2 sao usados (ReinitializePC, msg.param1); CRealMyRoomMain e do 645, ligado mas nao instanciado no 983 (My Room vivo = CRealMyRoomForm983 em jp/), entao o resto fica sem nome
enum ePCBangItem
{
	PCBANGITEM_0,
	PCBANGITEM_CADDIE,
	PCBANGITEM_MASCOT
};

class CRealMyRoomMain : public CTaskMain,
						public CRenderFuncPtr,
						public CHandOwner
{
	DECLARE_OBJECT(CRealMyRoomMain)

	friend class CRealMyRoom_Avatar;

public:
	struct miniBtn_t
	{
		miniBtn_t();
	};

	CRealMyRoomMain();
	virtual ~CRealMyRoomMain();

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float dt);
	virtual void OnDisplay();
	virtual void OnDestroy();
	virtual void DoReLoad();
	virtual void Display();

protected:
	void DrawMouseCursor();
	void CheckUIPosition();
	void CalculationCameraMat(WMatrix& mat, WVector at, WVector dir,
		WVector eye, float fovTarget, float fovCur, float step);
	void LoadBg(const char* name, int flag);
	void ProcessCamera(float dt);
	void ToggleTopViewMode();
	void RefreshAllUccClothes();

	// access as in the PDB public names
	virtual void OnAnotherHand(int key, void* param);

public:
	virtual void HandleMsg(const MsgObject& msg);

protected:
	virtual void OnUnderBar_MyRoomUp();
	virtual void CloseAllDialogs();

public:
	// Nota: abas sem uso no codigo (classe 645 nao instanciada no 983, ver ePCBangItem); valores reais so se a classe voltar a viver
	enum selTab_t
	{
		SELTAB_0
	};
	enum statTab_t
	{
		STATTAB_0
	};
	enum mainTab_t
	{
		MAINTAB_0
	};
	enum subTab_t
	{
		SUBTAB_0
	};
	enum eFurniture_Tab
	{
		FTAB_0
	};

	bool IsItemStorageVisible() const;
	void ChangeFlags(unsigned long);
	bool GetFlags(unsigned long);
	bool OnResultItemStorageAccessPanel(int, FrForm*);
	void SetUIState(unsigned long);
	void OpenItemStorageAccessPanel(unsigned char);
	void SetItemStorageVisible(bool);
	bool GetArrangement(unsigned long, unsigned long);
	bool OnResultItemStorageNotice(int, FrForm*);
	void OpenInformation(unsigned long*, sGiftInfo*, unsigned long*);
	void OpenItemStorageNotice();
	void OpenInformation(FrListItem*);
	void RMR_UpdateDB();

protected:
	void CloseLayout();
	void HidePet(bool);
	void InitMyRoomControls();
	void SetPetAreaRect();
	void ChangeSelTab(selTab_t);
	void SetItemDesc();
	void TurnOnItemSelect();
	void TurnOffItemSelect();
	void TurnOnStat();
	void TurnOffStat();
	void OnSelItemOwnerDraw(int);
	void OnSelItemUp();
	void OnCardBookOwnerDraw(int);
	void SetCardBookVisible(bool);
	bool OnCardBoxDlgResult(int, FrForm*);
	void OnTabCardPackLBDown();
	void OnTabCardVolume1LBDown();
	void OnTabCardVolume2LBDown();
	void OnTabCardVolume3LBDown();
	void OnTabCardVolume4LBDown();
	void BoundCardPackPage(FrButton*, unsigned char);
	void CardPackBehavior(unsigned char, unsigned char);
	void OnCardPackLBDown1();
	void OnCardPackLBDown2();
	void OnCardPackLBDown3();
	void OnCardPackLBDown4();
	void OnCardPackLBDown5();
	void OnCardPackLBDown6();
	void OnCardPackLBDown7();
	void OnCardPackLBDown8();
	void OnCardPackRBDown1();
	void OnCardPackRBDown2();
	void OnCardPackRBDown3();
	void OnCardPackRBDown4();
	void OnCardPackRBDown5();
	void OnCardPackRBDown6();
	void OnCardPackRBDown7();
	void OnCardPackRBDown8();
	void CardBehavior(unsigned char, unsigned char);
	void OnCardLBDown1();
	void OnCardLBDown2();
	void OnCardLBDown3();
	void OnCardLBDown4();
	void OnCardLBDown5();
	void OnCardLBDown6();
	void OnCardLBDown7();
	void OnCardLBDown8();
	void OnCardLBDown9();
	void OnCardLBDown10();
	void OnCardLBDown11();
	void OnCardLBDown12();
	void OnCardRBDown1();
	void OnCardRBDown2();
	void OnCardRBDown3();
	void OnCardRBDown4();
	void OnCardRBDown5();
	void OnCardRBDown6();
	void OnCardRBDown7();
	void OnCardRBDown8();
	void OnCardRBDown9();
	void OnCardRBDown10();
	void OnCardRBDown11();
	void OnCardRBDown12();
	void DrawStatus(FrGraphicInterface*, FrListItem*, short* const,
		short* const);
	void DrawStatus2(FrGraphicInterface*, FrListItem*, short* const,
		short* const, float);
	void DrawStatusCard(FrGraphicInterface*, FrListItem*, short* const, float);
	bool OnCardRemoveDlgResult(int, FrForm*);
	bool OnEnchantDlgResult(int, FrForm*);
	bool OnMascotInfoDlgResult(int, FrForm*);
	bool OnUpgradeDlgResult(int, FrForm*);
	bool OnFunritureSaveMsgdlgResult(int, FrForm*);
	void SafeLoadPet(unsigned long*, bool);
	__forceinline void HidePetNow();
	void SetCameraDisplayType(int nType);
	void OOnBtnMenuButtonUP();
	void OnBtn_MenuListRBtnDown();
	void OnBtn_MenuListMouseMove();
	void OnBtn_MenuListDClick();
	void SetMiddleUIControl(bool);
	void Furniture_ShowUI(bool);
	void Furniture_ShowUI_Left(bool);
	void OnRightMenuRBtnDown();
	bool OnResultDlg_Report(int, FrForm*);
	bool OnUccCopyDlgResult(int, FrForm*);
	bool OnOpenEventXmasGiftDlgResult(int, FrForm*);
	void OnOwnerDrawItemStoragePang(int);
	void OpenLayout(const char*);
	void LoadMyRoomImages();
	void OnMiddleUIInit(int);
	void OnTabTxt1Init(int);
	void OnTabTxt2Init(int);
	void OnTabTxt3Init(int);
	void OnSelectedNameInit(int);
	void CharacterViewControl(bool);
	void OnPetInit(int);
	void OnSkinPetInit(int);
	void OnSkinPet2Init(int);
	void OnFurnitureArrangeInit(int);
	void OnItemDescBgInit2(int);
	void OnItemDescBgOwnerDraw2(int);
	void OnItemDescBgInit(int);
	void OnItemDescBgOwnerDraw(int);
	void OnItemDescInit2(int);
	void OnItemNameInit(int);
	void OnItemDescInit(int);
	void OnItemSlotBoxInit(int);
	void OnEquipCancelInit(int);
	void OnEquipResetInit(int);
	void OnItemSlotInit(int);
	void OnItemSlotBtnDown();
	void OnGiftBgInit(int);
	void OnGiftDescInit(int);
	void SetGiftDescString(const char*, const char*, const char*, const char*);
	void OnStatFrameInit(int);
	void OnStatTextInit(int);
	void OnPowerBarInit(int);
	void OnControlBarInit(int);
	void OnImpactBarInit(int);
	void OnSpinBarInit(int);
	void OnCurveBarInit(int);
	void OnSelFrameInit(int);
	void OnStatBoxInit(int);
	void OnSelItemIconInit(int);
	void OnSelItemInit(int);
	void OnSelItemPrevInit(int);
	void OnSelItemNextInit(int);
	void OnStatPartInit(int);
	void OnStatPartOwnerDraw(int);
	void OnStatAllInit(int);
	void OnStatAllOwnerDraw(int);
	void OnTabMainInit(int);
	void OnTabSubInit(int);
	void OnCardBookInit(int);
	void OnCardBookIndexInit(int);
	void OnCardBookIndexLBDown();
	void OnCardBookIndexMouseOver();
	void OnCardUseInit(int);
	void OnCardPropertyInit(int);
	void OnTabCardPackInit(int);
	void OnTabCardVolume1Init(int);
	void OnTabCardVolume2Init(int);
	void OnTabCardVolume3Init(int);
	void OnTabCardVolume4Init(int);
	void OnCardPackInit1(int);
	void OnCardPackInit2(int);
	void OnCardPackInit3(int);
	void OnCardPackInit4(int);
	void OnCardPackInit5(int);
	void OnCardPackInit6(int);
	void OnCardPackInit7(int);
	void OnCardPackInit8(int);
	void BoundCardPage(FrButton*, unsigned char);
	void OnCardInit1(int);
	void OnCardInit2(int);
	void OnCardInit3(int);
	void OnCardInit4(int);
	void OnCardInit5(int);
	void OnCardInit6(int);
	void OnCardInit7(int);
	void OnCardInit8(int);
	void OnCardInit9(int);
	void OnCardInit10(int);
	void OnCardInit11(int);
	void OnCardInit12(int);
	void OnItemListInit(int);
	void OnItemListMouseMove();
	void OnPowerEditInit(int);
	void OnControlEditInit(int);
	void OnImpactEditInit(int);
	void OnSpinEditInit(int);
	void OnCurveEditInit(int);
	void OnUpgradeInit(int);
	void OnDowngradeInit(int);
	void Net_StartPeriodItem(unsigned long);
	bool OnPangGiftDlgResult(int, FrForm*);
	bool OnReturnGiftDlgResult(int, FrForm*);
	bool OnMoveGiftDlgResult(int, FrForm*);
	bool OnNoteDlgResult(int, FrForm*);
	bool OnOpenTikiReport(int, FrForm*);
	bool OnOpenNewYearMoneyDlgResult(int, FrForm*);
	bool OnOpenEventGiftDlgResult(int, FrForm*);
	bool OnOpenS4EventGiftDlgResult(int, FrForm*);
	void AutoScroll();
	bool OnFortuneDlgResult(int, FrForm*);
	void OnFurniturePupInit(int);
	void OnFurniturePdownInit(int);
	void OnFurnitureListNumInit(int);
	void OnFurnitureListNumBtnUp();
	void OnFurnitureListMaxNumInit(int);
	void OnFurniture_BtnListNumInit(int);
	void OnFurnitureResetInit(int);
	void OnFurnitureBackInit(int);
	void OnFurnitureSaveInit(int);
	void OnFurnitureOutInit(int);
	void OnFurnitureBackGroundInit1(int);
	void OnFurnitureBackGroundInit2(int);
	void OnFurnitureBackGroundInit3(int);
	void OnFurnitureBackGroundInit4(int);
	void OnFurnitureBackGroundInit5(int);
	void OnFurnitureBackGroundInit6(int);
	void OnFurnitureBackGroundInit7(int);
	void OnFurnitureBackGroundInit8(int);
	void OnFurnitureScrollInit1(int);
	void OnFurnitureScrollInit2(int);
	void OnFurnitureScrollInit3(int);
	void OnFurnitureTabMainInit(int);
	void OnFurnitureListInit(int);
	void OnBtnMenuInit(int);
	void OnBtn_MenuListOwnerDraw(int);
	void SetOverAndUnderBarVisible(bool);
	void OnEffectViewInit(int);
	void OnEffectViewOnwnerDraw(int);
	void OnRightMenuInit(int);
	void OpenEventXmasGiftBox(sItemInfo*);
	void OnOwnerDrawItemStorageBGI(int);
	void OnInitItemStoragePang(int);
	void OnInitItemStoragePangMinus(int);
	void OnLBDownItemStoragePangMinus();
	void OnInitItemStoragePangPlus(int);
	void OnLBDownItemStoragePangPlus();
	void OnInitItemStorageList(int);
	void OnOwnerDrawItemStorageList(int);
	void OnLBDownItemStorageList();
	void OnDBClickItemStorageList();
	void OnMouseMoveItemStorageList();
	void OnInitItemStorageIndexMsg(int);
	void OnOwnerDrawItemStorageIndexMsg(int);
	void OnLBDownItemStorageIndexMsg();
	void OnInitItemStoragePageNext(int);
	void OnLBDownItemStoragePageNext();
	void OnInitItemStoragePagePrev(int);
	void OnLBDownItemStoragePagePrev();
	void OnInitItemStorageClose(int);
	void OnInitItemStorageDragDrop(int);
	void OnOwnerDrawItemStorageDragDrop(int);
	bool OnRentalExtensionDlgResult(int, FrForm*);
	void ShowPet(unsigned long);
	void InitMyRoomDialogs();
	void InitMyRoomVariables();
	void DrawSkinFrame();
	void DrawSkinSlot();
	void OnItemSlotOwnerDraw(int);
	void OnGiftPageLBDown();
	void OnCardPackListDraw(int);
	void OnCardPackListDown();
	void OnCardUseLBDown();
	void OpenTikiReport(sItemInfo*);
	void OpenNewYearMoney(sItemInfo*);
	void OpenEventGiftBox(sItemInfo*);
	void OpenS4EventGiftBox(sItemInfo*);
	void OnTooltipHoverOn(int);
	void OnTooltipHoverOff(int);
	void GetTotalStatus(unsigned char* const, unsigned char* const,
		unsigned char* const);
	void GetCharacterStatus(unsigned char* const, unsigned char* const);
	void GetCaddieStatus(unsigned char* const, unsigned char* const);
	void DrawSelectItemList(FrListItem*);
	void Net_EquipCharacter();
	void Net_EquipItem();
	void Net_EquipGear();
	bool OnOfflineShopEditMsgDlgResult(int, FrForm*);
	void OnFurnitureListOwnerDraw(int);
	void UIDisplayControl(bool);
	void UpdateRightMenu();
	void PopupRightMenu(int, int);
	void OnUnderBar_BackUp();
	void FurnitureUse();
	bool OnFurniture_AbilityDlgResult(int, FrForm*);
	bool OnCardPackDlgResult(int, FrForm*);
	bool OnItemAttachCardDlgResult(int, FrForm*);
	bool OnUccDrawDlgResult(int, FrForm*);
	bool OnOpenNewRandomBoxDlgResult(int, FrForm*);
	void OnLBDownItemStorageClose();
	void OnMyRoomInit(int);
	void BuildCardPackList();
	void OnCardPackListInit(int);
	void OnUpgradeUp();
	void OnDowngradeUp();
	bool UnselectItemFromItemList(unsigned long);
	void Net_EquipDresserParts();
	void Net_EquipSkin();
	bool OnFurnitrueListDeleteDlgResult(int, FrForm*);
	void EquipItem(sItemInfo*);
	bool IsInItemList(unsigned long);
	bool OnFortuneOpenDlgResult(int, FrForm*);
	bool OnReplayModeOpenDlgResult(int, FrForm*);
	void BuildDisplayStandFurniture();
	void ChangeFMainTab(eFurniture_Tab, bool);
	void OnFurnitureListBtnDown();
	void SetListMenu();
	void FurnitureDisplayControl(bool);
	void FurnitureRun();
	bool OnPostDlgResult(int, FrForm*);
	bool OnAuthDlgResult(int, FrForm*);
	void DrawPartsSlot(FrGraphicInterface*, FrListItem*);
	bool OnCardDlgResult(int, FrForm*);
	void OpenNewRandomBox(sItemInfo*);
	void OnRBDownItemStorageList();
	void OnCardPackListRbUp();
	void OnItemListOwnerDraw(int);
	void OnItemListDClick();
	void GetClubSetStatus(unsigned char* const, unsigned char* const);
	void SetLevelBar();
	void ShowMainModel();
	int GetItemNumInMyPocket(unsigned long);
	void Net_EquipCaddie();
	void Net_EquipMascot();
	// JP 983 (pedido C27): tipos 9 e 10 do pacote 0x20; eram funcoes livres da C em include/packets/realmyroompackets.h
	void Net_EquipCutIn983(); // 0060e810
	void Net_EquipPoster983(); // 0060e6c0: compara m_userEquip.poster com o do CSharedDoc
public:
	// JP 983: virtual 006165d0, slot 39 da vtable 00d1497c = FrTaskForm::OnCloseWidget983 (+0x9c; o my room do 983 e um
	// FrTaskForm, form "realmyroom"). Aqui nao e virtual (a base continua CTaskMain); o destrutor chama.
	bool OnCloseWidget983();
	void OnMasteryMUp983(); // 983: 0060d460, LBUTTONUP "updown_group.mastery_m" (pacote 0x157; PKT, frente C)
	// JP 983 (pedido B89): resultados dos dialogos do reset do taco (FrClubSetResetWnd da frente B), repassados ao s_pInstance
	bool OnClubSetResetSelected983(int, FrForm*); // 0060ac10 -> OnSelectResult983
	bool OnClubSetResetQuestion983(int, FrForm*); // 0060ac30 -> OnQuestionResult983
	bool OnClubSetResetProgress983(int, FrForm*); // 0060ac50 -> OnProgressResult983
	bool OnClubSetResetCloseQuestion983(int, FrForm*); // 0060ac70 -> OnCloseQuestion983
	bool OnClubSetResetDone983(int, FrForm*); // 00605eb0: apaga o s_pInstance
	void DisplayExhibition983(); // 0060d9b0, tambem do FrUccDrawDlg::OnDraw 007e2150
protected:
	bool OnCaddieInfoDlgResult(int, FrForm*);
	void OpenCaddieInfo(const sCaddieInfo&);
	void UnequipItem(unsigned long*);
	void TurnOnItemStatus();
	void TurnOffItemStatus();
	void OnFurniturePupButtonUp();
	void OnFurniturePdownButtonUp();
	void OnFurniture_BtnListNumBtnUp();
	void OnFurnitureResetButtonUp();
	void OnFurnitureTabMainButtonUp();
	void OnBtn_MenuListInit(int);
	void PostDlgCreate();
	bool OnPostSendDlgResult(int, FrForm*);
	bool OnPostSendDlg2Result(int, FrForm*);
	void DeleteCardImages();
	void OnInitItemStorageBGI(int);
	void DeleteCutinImages();
	void LoadItemImages();
	void ChangeStatTab(statTab_t);
	int ChangeClubset(unsigned long);
	void ChangeAztec(unsigned long);
	const Bitmap* GetItemIcon(unsigned long);
	void DrawSelItem(FrGraphicInterface*, unsigned long);
	void DrawAztecList(FrGraphicInterface*, unsigned long);
	void DrawClubList(FrGraphicInterface*, unsigned long);
	void OnEquipCancelUp();
	void OnEquipResetUp();
	void OnItemSlotDClick();
	void OnStatPartUp();
	void OnStatAllUp();
	void MoveGift(sGiftInfo*);
	bool OnFurnitureListEnterKey(int);
	void OnFurnitureBackButtonUp();
	void LoadCardImages();
	WOverlay* GetCardImg(unsigned long);
	void DrawCard();
	WOverlay* GetCutinImg(unsigned long);
	void BuildDisplayStand(subTab_t);
	bool OnReplayDelOpenDlgResult(int, FrForm*);
	void OnFurnitureSaveButtonUp();
	void DrawCutinFrame();
	__declspec(noinline) void ChangeSubTab(subTab_t, bool);
	void ChangeMainTab(mainTab_t, bool);
	void ChangeCharacter(unsigned long);
	bool ChangeCaddie(unsigned long);
	void ChangeMascot(unsigned long);
	void DrawCharacterList(FrGraphicInterface*, unsigned long);
	void DrawCaddieList(FrGraphicInterface*, unsigned long);
	void DrawMascotList(FrGraphicInterface*, unsigned long);
	void OnPetOwnerDraw(int);
	void OnSelItemIconOwnerDraw(int);
	void OnSelItemPrevUp();
	void OnSelItemNextUp();
	void OnTabMainButtonUp();
	void OnTabSubButtonUp();
	void OnGiftPageOwnerDraw(int);
	void UnselectItemList(FrListItem*);
	void SetMiddleUIVisible(bool);
	void Reinitialize(bool);
	void ReinitializePC(ePCBangItem);
	bool SelectItemList(FrListItem*);
	bool UnselectItemList(unsigned long);
	void ControlUiHideMode(int);
	void OnMyRoomFinish();
	void OnFurnitureArrangeButtonUP();
	void OnItemListBtnDown();
	void OnItemListRBtnDown();
	bool SelectItemList(unsigned long);
	void OnFurnitureOutButtonUp();
	void SelectMenu(unsigned char);
	void FurnitureFunctionChange();
	void OnUnderBar_ItemStorageUp();
	void OnUnderBar_GiftUp();
	bool OnAuthDlgFaileResult(int, FrForm*);
	void OnBtn_MenuListBtnDown();
	void OnRightMenuLBtnDown();

private:
	void ExecuteMiniButtonDown(sItemInfo*);

public:
protected:
	DECLARE_FRESH_MSGMAP()

	// Nota: layout dos offsets do 645 (ctor em realmyroommain.cpp); sem par no 983 por ser classe nao instanciada (My Room vivo em jp/)
	LightSet m_lightSet;
	WTL::CString m_bgName;
	union
	{
		unsigned char m_b_208;
		bool m_b208;
	};
	bool m_b209;
	unsigned char m_pad20A[2];
	int m_glowBallTex;
	union
	{
		FrWnd* m_pFrWnd_210;
		void* m_p210;
	};
	WVector m_v214;
	WVector m_v220;
	float m_fovCur;
	float m_fovTarget;
	float m_f234;
	float m_f238;
	float m_f23c;
	float m_f240;
	float m_f244;
	unsigned long m_f248;
	unsigned long m_pad24c[3];
	bool m_bTopView;
	unsigned char m_pad259[3];
	float m_topViewDist;
	unsigned long m_pad260[1];
	WVector m_vTopView;
	float m_time270;
	float m_f274;
	float m_f278;
	float m_f27c;
	float m_f280;
	union
	{
		int m_n_284;
		unsigned long m_f284;
	};
	WTitleFont* m_pTitleFont2;
	unsigned long m_pad28C[11];
	unsigned long m_f2b8;
	int m_resD;
	CFxSequence* m_pFxSeq;
	unsigned char m_b_2C4;
	unsigned char m_pad2C5[3];
	WVector m_v2c8;
	WVector m_vCamTarget;
	float m_fCamFov;
	FrGroupBox* m_pFrGroupBox_2E4;
	FrStatic* m_pFrStatic_2E8;
	FrViewer* m_pFrViewer_2EC;
	FrViewer* m_pFrViewer_2F0;
	FrTabButton* m_pFrTabButton_2F4;
	FrTabButton* m_pFrTabButton_2F8;
	unsigned long m_n_2FC;
	unsigned long m_n_300;
	FrListBox* m_pFrListBox_304;
	FrArea* m_pFrArea_308;
	FrArea* m_pFrArea_30C;
	FrArea* m_pFrArea_310;
	FrWnd* m_pFrWnd_314;
	unsigned long m_n_318;
	FrArea* m_pFrArea_31C;
	FrArea* m_pFrArea_320;
	FrButton* m_pFrButton_324;
	FrGroupBox* m_pFrGroupBox_328;
	FrGaugeBar* m_pFrGaugeBar_32C;
	FrGaugeBar* m_pFrGaugeBar_330;
	FrGaugeBar* m_pFrGaugeBar_334;
	FrGaugeBar* m_pFrGaugeBar_338;
	FrGaugeBar* m_pFrGaugeBar_33C;
	FrStatic* m_pFrStatic_340;
	FrStatic* m_pFrStatic_344;
	FrStatic* m_pFrStatic_348;
	FrStatic* m_pFrStatic_34C;
	FrStatic* m_pFrStatic_350;
	FrButton* m_pFrButton_354;
	FrButton* m_pFrButton_358;
	FrArea* m_pFrArea_35C;
	FrEdit* m_pFrEdit_360;
	FrViewer* m_pFrViewer_364;
	FrEdit* m_pFrEdit_368;
	FrEdit* m_pFrEdit_36C;
	FrArea* m_pFrArea_370;
	FrArea* m_pFrArea_374;
	FrArea* m_pFrArea_378;
	FrListBox* m_pFrListBox_37C;
	FrButton* m_pFrButton_380;
	FrGroupBox* m_pFrGroupBox_384;
	FrButton* m_pFrButton_388;
	FrButton* m_pFrButton_38C;
	FrButton* m_pFrButton_390;
	FrButton* m_pFrButton_394;
	FrWnd* m_pFrWnd_398;
	FrViewer* m_pFrViewer_39C;
	FrEdit* m_pFrEdit_3A0;
	FrArea* m_pFrArea_3A4;
	FrArea* m_pFrArea_3A8;
	FrTabButton* m_pFrTabButton_3AC;
	FrListBox* m_pFrListBox_3B0;
	FrListBox* m_pFrListBox_3B4;
	FrButton* m_pFrButton_3B8;
	FrButton* m_pFrButton_3BC;
	union
	{
		FrEdit* m_pFrEdit_3C0;
		void* m_p3c0;
	};
	FrEdit* m_pFrEdit_3C4;
	FrButton* m_pFrButton_3C8;
	FrButton* m_pFrButton_3CC;
	FrButton* m_pFrButton_3D0;
	FrButton* m_pFrButton_3D4;
	FrButton* m_pFrButton_3D8;
	FrArea* m_pFrArea_3DC;
	FrArea* m_pFrArea_3E0;
	FrArea* m_pFrArea_3E4;
	FrArea* m_pFrArea_3E8;
	FrArea* m_pFrArea_3EC;
	FrArea* m_pFrArea_3F0;
	FrArea* m_pFrArea_3F4;
	FrArea* m_pFrArea_3F8;
	FrArea* m_pFrArea_3FC;
	FrArea* m_pFrArea_400;
	FrArea* m_pFrArea_404;
	unsigned long m_n_408;
	unsigned long m_n_40C;
	unsigned long m_n_410;
	unsigned long m_n_414;
	unsigned long m_n_418;
	unsigned long m_n_41C;
	FrButton* m_pFrButton_420;
	unsigned long m_n_424;
	FrEnchantDlg* m_pFrEnchantDlg_428;
	FrDowngradeDlg* m_pFrDowngradeDlg_42C;
	FrDowngradeDlg* m_pFrDowngradeDlg_430;
	FrPangGiftDlg* m_pFrPangGiftDlg_434;
	FrMascotInfoDlg* m_pFrMascotInfoDlg_438;
	FrNoteDlg* m_pFrNoteDlg_43C;
	FrFortuneDlg* m_pFrFortuneDlg_440;
	FrFortuneOpenDlg* m_pFortuneOpenDlg;
	FrReplayModeDlg* m_pFrReplayModeDlg_448;
	FrCardUseDlg* m_pFrCardUseDlg_44C;
	FrItemAttachCardDlg* m_pFrItemAttachCardDlg_450;
	FrOpenCardPackDlg* m_pFrOpenCardPackDlg_454;
	unsigned long m_pad458[1];
	FrUccDrawDlg* m_pFrUccDrawDlg_45C;
	FrUccCopyDlg* m_pFrUccCopyDlg_460;
	FrUniteResultDlg* m_pFrUniteResultDlg_464;
	FrCardRemove* m_pFrCardRemove_468;
	unsigned long m_pad46C[2];
	CPost_ReciveDlg* m_pCPost_ReciveDlg_474;
	CPost_SendDlg* m_pCPost_SendDlg_478;
	CRealMyRoom_AuthorityDlg* m_pCRealMyRoom_AuthorityDlg_47C;
	CFurniture_AbilityDlg* m_pCFurniture_AbilityDlg_480;
	const Bitmap* m_pBitmap_484;
	const Bitmap* m_pBitmap_488;
	const Bitmap* m_pBitmap_48C;
	const Bitmap* m_pBitmap_490;
	unsigned long m_pad494[3];
	const Bitmap* m_pStatusBarImg[5][2];
	const Bitmap* m_pBitmap_4C8;
	const Bitmap* m_pBitmap_4CC;
	const Bitmap* m_pBitmap_4D0;
	const Bitmap* m_pBitmap_4D4;
	const Bitmap* m_pBitmap_4D8;
	const Bitmap* m_pBitmap_4DC;
	const Bitmap* m_pStatTabImg[4][3];
	const Bitmap* m_pSelectImg[5][3];
	const Bitmap* m_pBitmap_54C;
	const Bitmap* m_pBitmap_550;
	const Bitmap* m_pBitmap_554;
	const Bitmap* m_pBitmap_558;
	const Bitmap* m_pBitmap_55C;
	const Bitmap* m_pSlotImg[3];
	const Bitmap* m_pBitmap_56C;
	const Bitmap* m_pBitmap_570;
	const Bitmap* m_pBitmap_574;
	unsigned long m_pad578[4];
	const Bitmap* m_pBitmap_588;
	const Bitmap* m_pBitmap_58C;
	const Bitmap* m_pBitmap_590;
	const Bitmap* m_pBitmap_594;
	const Bitmap* m_pBitmap_598;
	const Bitmap* m_pBitmap_59C;
	const Bitmap* m_pBitmap_5A0;
	const Bitmap* m_pBitmap_5A4;
	const Bitmap* m_pBitmap_5A8;
	const Bitmap* m_pBitmap_5AC;
	const Bitmap* m_pBitmap_5B0;
	const Bitmap* m_pBitmap_5B4;
	const Bitmap* m_pBitmap_5B8;
	const Bitmap* m_pBitmap_5BC;
	const Bitmap* m_pBitmap_5C0;
	const Bitmap* m_pBitmap_5C4;
	const Bitmap* m_pBitmap_5C8;
	const Bitmap* m_pBitmap_5CC;
	const Bitmap* m_pBitmap_5D0;
	const Bitmap* m_pBitmap_5D4;
	const Bitmap* m_pBitmap_5D8;
	const Bitmap* m_pBitmap_5DC;
	const Bitmap* m_pBitmap_5E0;
	const Bitmap* m_pBitmap_5E4;
	const Bitmap* m_pBitmap_5E8;
	const Bitmap* m_pBitmap_5EC;
	const Bitmap* m_pBitmap_5F0;
	const Bitmap* m_pBitmap_5F4;
	const Bitmap* m_pBitmap_5F8;
	const Bitmap* m_pBitmap_5FC;
	const Bitmap* m_pBitmap_600;
	const Bitmap* m_pBitmap_604;
	const Bitmap* m_pBitmap_608;
	const Bitmap* m_pBitmap_60C;
	const Bitmap* m_pBitmap_610;
	const Bitmap* m_pBitmap_614;
	const Bitmap* m_pBitmap_618;
	const Bitmap* m_pBitmap_61C;
	const Bitmap* m_pBitmap_620;
	const Bitmap* m_pBitmap_624;
	const Bitmap* m_pBitmap_628;
	const Bitmap* m_pBitmap_62C;
	const Bitmap* m_pBitmap_630;
	const Bitmap* m_pBitmap_634;
	const Bitmap* m_pBitmap_638;
	unsigned long m_pad63C[1];
	const Bitmap* m_pBitmap_640;
	const Bitmap* m_pBitmap_644;
	const Bitmap* m_pBitmap_648;
	const Bitmap* m_pBitmap_64C;
	const Bitmap* m_pBitmap_650;
	FrWnd* m_pFrWnd_654;
	FrWnd* m_pFrWnd_658;
	unsigned long m_pad65C[1];
	FrWnd* m_pFrWnd_660;
	FrWnd* m_pFrWnd_664;
	FrWnd* m_pFrWnd_668;
	FrWnd* m_pFrWnd_66C;
	FrWnd* m_pFrWnd_670;
	FrWnd* m_pFrWnd_674;
	FrWnd* m_pFrWnd_678;
	FrWnd* m_pFrWnd_67C;
	FrWnd* m_pFrWnd_680;
	FrArea* m_pFrArea_684;
	unsigned long m_n_688;
	unsigned long m_pad68C[2];
	union
	{
		FrWnd* m_pFrWnd_694;
		unsigned long m_f694;
	};
	float m_f_698;
	float m_f_69C;
	float m_f_6A0;
	float m_f_6A4;
	std::map<unsigned long, const Bitmap*> m_iconMap;
	std::map<unsigned int, WOverlay*> m_cardImgMap;
	std::map<unsigned int, WOverlay*> m_cutinImgMap;
	unsigned long m_pad6CC[3];
	unsigned char m_b_6D8;
	unsigned char m_pad6D9[3];
	char* m_pCharStat;
	unsigned long* m_pParts;
	unsigned long
		m_partUid[24]; // CPartTidList::m_uid of the equipped character
	unsigned long* m_pAuxParts;
	unsigned long m_tidChar;
	int m_n_74C;
	unsigned long m_tidCaddie;
	int m_n_754;
	unsigned long m_n_758;
	int m_n_75C;
	int m_n_760;
	int m_n_764;
	unsigned long m_tidClub;
	short* m_pClubStat;
	unsigned long m_tidAztec;
	int m_n_774;
	int m_roomMode;
	int m_n_77C;
	selTab_t m_selTab_t_780;
	unsigned char m_b_784;
	unsigned char m_pad785[3];
	WPoint m_WPoint_788;
	float m_f_790;
	float m_f_794;
	float m_f_798;
	float m_f_79C;
	eFurniture_Tab m_eFurniture_Tab_7A0;
	int m_n_7A4;
	int m_n_7A8;
	sGiftInfo* m_psGiftInfo_7AC;
	union
	{
		int m_n_7B0;
		unsigned long m_f7b0;
	};
	int m_n_7B4;
	unsigned long m_n_7B8;
	int m_n_7BC;
	unsigned long m_pad7C0[4];
	unsigned long m_n_7D0;
	unsigned long m_n_7D4;
	int m_n_7D8;
	CExhibition* m_pExhibition;
	CAnotherHand m_hand;
	std::map<unsigned int, sCharacterInfo> m_mapChar;
	union
	{
		sUserEquip m_userEquip;
		sMyRoomEnterInfo m_enterInfo;
	};
	WRect m_WRect_9E0;
	const Bitmap* m_pBitmap_9F0;
	const Bitmap* m_pBitmap_9F4;
	unsigned long m_pad9F8[1];
	WRect m_WRect_9FC;
	const Bitmap* m_pBitmap_A0C;
	const Bitmap* m_pBitmap_A10;
	unsigned long m_padA14[1];
	WRect m_WRect_A18;
	const Bitmap* m_pBitmap_A28;
	const Bitmap* m_pBitmap_A2C;
	unsigned long m_padA30[1];
	WRect m_WRect_A34;
	const Bitmap* m_pBitmap_A44;
	const Bitmap* m_pBitmap_A48;
	unsigned long m_padA4C[1];
	WRect m_WRect_A50;
	const Bitmap* m_pBitmap_A60;
	const Bitmap* m_pBitmap_A64;
	unsigned long m_padA68[1];
	WRect m_WRect_A6C;
	const Bitmap* m_pBitmap_A7C;
	const Bitmap* m_pBitmap_A80;
	unsigned long m_padA84[8];
	WRect m_WRect_AA4;
	const Bitmap* m_pBitmap_AB4;
	const Bitmap* m_pBitmap_AB8;
	unsigned long m_padABC[1];
	WRect m_WRect_AC0;
	const Bitmap* m_pBitmap_AD0;
	const Bitmap* m_pBitmap_AD4;
	unsigned long m_padAD8[1];
	WRect m_WRect_ADC;
	const Bitmap* m_pBitmap_AEC;
	const Bitmap* m_pBitmap_AF0;
	unsigned long m_padAF4[1];
	WRect m_WRect_AF8;
	const Bitmap* m_pBitmap_B08;
	const Bitmap* m_pBitmap_B0C;
	unsigned long m_padB10[5];
	const Bitmap* m_pBitmap_B24;
	const Bitmap* m_pBitmap_B28;
	unsigned long m_padB2C[5];
	const Bitmap* m_pBitmap_B40;
	const Bitmap* m_pBitmap_B44;
	unsigned long m_padB48[5];
	const Bitmap* m_pBitmap_B5C;
	const Bitmap* m_pBitmap_B60;
	unsigned long m_padB64[1];
	std::list<sSpecialTrophy> m_trophyList;
	int m_n_B74;
	int m_n_B78;
	unsigned long m_n_B7C;
	unsigned long m_n_B80;
	unsigned long m_n_B84;
	unsigned long m_n_B88;
	unsigned long m_n_B8C;
	float m_scrollPos[2];
	union
	{
		int m_scrollDir[2];
		struct
		{
			unsigned int m_n_B98;
			unsigned int m_n_B9C;
		};
	};
	unsigned long m_fba0;
	unsigned int m_giftTotal;
	unsigned int m_giftCount;
	unsigned int m_giftSel;
	unsigned int m_giftPage;
	WRect m_giftPrevRect;
	WRect m_giftSlotRect[6];
	unsigned char m_b_C24;
	unsigned char m_b_C25;
	unsigned char m_b_C26;
	unsigned char m_padC27[1];
	WRect m_WRect_C28;
	WRect m_WRect_C38;
	unsigned long m_fc48;
	union
	{
		int m_n_C4C;
		unsigned long m_fc4c;
	};
	CCameraWork* m_pCamMain;
	unsigned long m_fc54;
	CCameraWork* m_pCamList[4];
	CCameraWork* m_pCamSel;
	unsigned long m_fc6c;
	float m_clipNear;
	float m_clipFar;
	WVector m_vLineA0;
	WVector m_vLineA1;
	unsigned long m_padc90[3];
	WVector m_vLineB0;
	WVector m_vLineB1;
	unsigned long m_padcb4[9];
	WVector m_vCamLook;
	bool m_bCamFollow;
	unsigned char m_padCE5[3];
	unsigned long m_fce8;
	unsigned long m_padCEC[8];
	WMatrix m_matCam;
	float m_fovCam;
	unsigned long m_padD40[1];
	unsigned char m_b_D44;
	unsigned char m_b_D45;
	unsigned char m_b_D46;
	unsigned char m_padD47[5];
	union
	{
		int m_n_D4C;
		unsigned long m_fd4c;
	};
	int m_n_D50;
	bool m_b_D54;
	bool m_b_D55;
	bool m_b_D56;
	bool m_b_D57;
	bool m_b_D58;
	unsigned char m_padD59[3];
	unsigned long m_n_D5C;
	WResource* m_resA[6];
	WResource* m_resB[2];
	WResource* m_resC[2];
	unsigned char m_b_D88;
	unsigned char m_padD89[3];
	CCardBook* m_pCardBook;
	FrItemStorageNotice* m_pFrItemStorageNotice_D90;
	FrItemStorageAccessPanel* m_pFrItemStorageAccessPanel_D94;
	bool m_b_D98;
	unsigned char m_padD99[3];
	unsigned long m_n_D9C;
	unsigned long m_n_DA0;
	FrArea* m_pFrArea_DA4;
	FrArea* m_pFrArea_DA8;
	FrButton* m_pFrButton_DAC;
	FrButton* m_pFrButton_DB0;
	FrListBox* m_pFrListBox_DB4;
	FrListBox* m_pFrListBox_DB8;
	FrButton* m_pFrButton_DBC;
	FrButton* m_pFrButton_DC0;
	FrButton* m_pFrButton_DC4;
	FrArea* m_pFrArea_DC8;
	FrListBox* m_pFrListBox_DCC;
	// total so far 0xdd0
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnreturngiftdlgrFunclets(); // HACK: EH funclets of the naked OnReturnGiftDlgResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOngiftpagelbdownFunclets(); // HACK: EH funclets of the naked OnGiftPageLBDown (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpostsenddlgresFunclets(); // HACK: EH funclets of the naked OnPostSendDlgResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbRmrupdatedbFunclets(); // HACK: EH funclets of the naked RMR_UpdateDB (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbChangecharacterFunclets(); // HACK: EH funclets of the naked ChangeCharacter (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbNetequipdresserpFunclets(); // HACK: EH funclets of the naked Net_EquipDresserParts (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbFurnitureuseFunclets(); // HACK: EH funclets of the naked FurnitureUse (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbShowpetFunclets(); // HACK: EH funclets of the naked ShowPet (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbHandlemsgFunclets(); // HACK: EH funclets of the naked HandleMsg (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbBuilddisplaystan2Funclets(); // HACK: EH funclets of the naked BuildDisplayStand (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbChangemaintabFunclets(); // HACK: EH funclets of the naked ChangeMainTab (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSelectmenuFunclets(); // HACK: EH funclets of the naked SelectMenu (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpreloadinitFunclets(); // HACK: EH funclets of the naked OnPreLoadInit (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnloadFunclets(); // HACK: EH funclets of the naked OnLoad (nakedify)
#endif
};

inline bool CRealMyRoomMain::IsItemStorageVisible() const
{
	return m_b_D98;
}
