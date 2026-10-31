// JP 983: unidade 005a7010–005aaf50 (CMyRoomWidget). Classe nova do 983; ver include/jp/myroomwidget.h. Sessão B53: os 9
// métodos próprios. O `OnPacket` 005a8360 (10409 B) NÃO está aqui (pedido B → A da B52). O resto da faixa (005a7060–005a8350
// menos os daqui) são cópias fora de linha usadas pelo `OnPacket`: `CItemStorage::Instance` 005a7060 / 005a7070, moldes de
// `std::vector` e `std::map`, e os construtores de três blocos de pacote (005a73d0 de 0x26 bytes, 005a7400 de 0xac, 005a7530).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "actor.h"
#include "taskmanager.h"
#include "itemstorage.h"
#include "wview.h"
#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60
#include "frform.h"
#include "taskform.h"
#include "../shared/titlemanager.h" // C720: Title::CanEquip (006166b0 caso 0xe)
#include "titles_client.h"
#include "myroomwidget.h"
#include "fortunedlg.h"
#include "s5/rentalextensiondlg.h" // 983 0061fa90
#include "fortuneopendlg.h"
#include "caddieinfodlg.h"
#include "resultdlg.h" // FrUniteResultDlg (0x282)
#include "mascotinfodlg.h"
#include "frwndmanager.h"
#include "taskwidget.h"
#include "frstatic.h"
#include "statushelper.h"
#include "frtabbutton.h"
#include "../../shared/localize.h"
#include "frlistbox.h"
#include "jp/itemsearchdlg.h" // sItemSearchResult983 (0x2d3)
#include "frscrollbar.h"
#include "frdesktop.h"
#include "frgraphicinterface.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "fresh.h"
extern Fresh* g_pFresh; // 00f02340
#include "shareddoc.h"
#include "jp/shareddocjp.h"
#include "packet.h"
#include "contentsdoc.h" // CGiftBoxEvent (0061aae0 0x52/0x55)
#include "../s5/utilities.h"
#include "../s5/supplypack.h" // FrSupplyPackDlg (0061aae0 0x105/0x14b)
#include "gatewayactor.h" // s_gatewayActor (0061aae0 0x15b)
#include "createcharacteroption.h"
#include "mascot.h"
#include "messengerdlg.h" // CMessengerInfo (0060cb27)
#include "clientsetting.h"
// D308: offsetof(m_myInfo.userEquip.tidItemSlot)==0xa36 provado (jpbuild obj ok); o 0xa36 cru abaixo e o mesmo campo
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.tidItemSlot) == 0xa36);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip) == 0xa26 && offsetof(CSharedDoc, m_myInfo.userEquip.tidSkin) == 0xa76 && offsetof(CSharedDoc, m_myInfo.userEquip.guidMascot) == 0xa8e && offsetof(CSharedDoc, m_myInfo.userEquip.poster) == 0xa92); // A736: crus Doc()+0xa26..0xa96 -> membros
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.sNick) == 0x7b8 && offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812 && offsetof(CSharedDoc, m_rmrAuthority) == 0x1070c && sizeof(sRealMyRoomAuthority) == 0x6b && offsetof(CSharedDoc, m_listTradeItem) == 0x111d8); // A743
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.stat.Level) == 0x91d && offsetof(CSharedDoc, m_myInfo.info.dwEmblemVer) == 0x82e); // A747: ex-Doc+0x91d crus
#include "jp/grandprixroomdlg.h"
#include "cardsystem.h"
#include "deleteitem.h"
#include "setitemdlg.h"
#include "ucclibrary.h"
#include "uccmanager.h"
#include "uccdrawdlg.h"
#include "uccclothes.h"
std::string __fastcall GenerateIconFilename(unsigned long typeId, const char* uccIndex); // 007eafd0
#include "ucccopydlg.h"
#include "selfdesignexhibit.h"
#include "ccardbook.h"
#include "cardboxdlg.h"
#include "refresh/rfcharmasteryaboutdlg.h"
#include "downgradedlg.h"
#include "enchantdlg.h"
#include "jp/clubsetworkshopmaindlg.h"
#include "jp/clubsetinfodlg.h"
#include "jp/clubsetresetwnd.h" // A712: reset do taco 0x247/0x24b (00e74254)
#include "tooltip.h"
#include "exhibition.h"
#include "petframe.h"
#include "wbone.h"
bool __fastcall EmptyCompares(const void* a, const void* b); // realmyroommainui.cpp (00604b40)
#include "refresh/rflobbydesktop.h"
#include "avatar_avatar.h" // A799: ntAvatar::SetState (007a4820)
#include "refresh/rflobbycharacterselectwnd.h" // 005d3b90 (0x2d7)
#include "jp/clubsetstat.h"
#include "jp/lobbyjp.h" // DOCLOBBYJP (taco/bola em uso do doc)
#include "realmyroom_authoritydlg.h"
#include "replaymodedlg.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10
#include "itemcursor983.h" // C721: lista da aba 0x26 (+0xac0)
#include "frrandomboxprogress.h" // "newRandomBox" (0060c310) do botao giftmove_btn" // SelectMenu(6): "realmyroom_authority" (0060c150)
#include "docofs983.h" // A747: Doc+0x832 (byte de bits do info) ancorado no m_myInfo (VS 2022)

// 00e0f64c / 00e0f668: dois textos montados pelo mesmo inicializador estático (00cd1672); o 1º não tem leitor no 983
static const std::string s_widgetName("CMyRoomWidget");
static const std::string s_formName("realmyroom");

IMPLEMENT_OBJECT(CMyRoomWidget, CTaskWidget)

// 005a7010
CMyRoomWidget::CMyRoomWidget()
{
}

// 005a7040
void CMyRoomWidget::Process(float delta)
{
	CTaskWidget::Process(delta);
}

// 005a7790
void CMyRoomWidget::Activate()
{
	if (m_pForm)
	{
		if (AfxGetTask()->GetActor("Lobby"))
		{
			WRect rect = m_pForm->GetRect();
			rect.x = (g_view->GetWidth() - 800.0) * 0.3f + rect.x;
			m_pForm->SetRect(rect);
		}
		m_pForm->Open(NULL, 1);
	}
	CTaskWidget::Activate();
}

// 005a7870
void CMyRoomWidget::Close()
{
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x2af, 0, 0, 0, 0, 0);
	AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x2af, 0, 0, 0, 0, 0);
	if (CItemStorage::Instance())
		delete CItemStorage::Instance();
}

// 005a7b00
CMyRoomWidget::~CMyRoomWidget()
{
	CMyRoomWidget::Close();
}

// 005a7f60
void CMyRoomWidget::Open()
{
	m_formName = s_formName;
	new CItemStorage;
	CTaskWidget::Open();
}

template <class T> static void Bind983(T& dst, T w)
{
	dst = w;
	if (w)
		w->SetVisible(false);
}

// Form "realmyroom" do 983 (FrTaskForm de 0xae0 B; criador 00620b90 -> 00620a00, ctor 0061f1f0, vtable 00d1497c).
// ESQUELETO (B339): so registra o criador na fabrica de forms para o My Room abrir sem o erro 0x1389. A logica (itens, avatar,
// armazem, pacotes) ainda esta no CRealMyRoomMain (645) e migra para ca por metodos do 983 (out/F5/myroom/DIFERENCAS.md).
void MyRoomPreview983(char* form, unsigned long tid, bool force);
void MyRoomPetClear983(char* form);
void MyRoomShowPet983(char* form, unsigned long tid);
void MyRoomPetSelectWnd983(char* form);
void MyRoomPetDraw983(char* form);
extern int g_lobbyModal983; // 00e78cac
int MyRoomPetFrameSetParts983(CPetFrame* pFrame, CPartTidList* pList, unsigned long tid, unsigned long uid); // lobbyplayer.cpp
void OpenCardUseResult983(FrCmdTarget* pOwner, unsigned long tid); // carddlg.cpp (carddlg.h x cardboxdlg.h nao convivem)
void MyRoomPetFrameSetHairClr983(CPetFrame* pFrame, CPartTidList* pList, unsigned char color); // lobbyplayer.cpp
bool MyRoomPetFrameSetAuxPart983(CPetFrame* pFrame, unsigned char slot, unsigned long tid); // lobbyplayer.cpp (A679)

static bool IsOpenableBox983(unsigned long tid); // botao giftmove_btn da aba 10 (A709)
// A799 (983 00620cfc / 0061d5fe): My Room aberto no Lounge (00e78cac == 1). Ao abrir, o avatar ganha o estado 2 (0x63 sub 8);
// ao fechar (dtor) o desktop RF volta a camada 0 (+0x184, que OnMyRoomClick983 pos em 1) e o avatar perde o 2. Sem isso o
// m_layer ficava 1 com 0 forms e a barra do Lounge nao recebia mais entrada (dono14: botoes travam depois do My Room).
static void LoungeMyRoomState983(bool open);
static bool HasGiftButton983(unsigned long tid); // 00618572: RandomBox + Supply Pack/Key of Fortune (A710)

class CRealMyRoomForm983 : public FrTaskForm
{
	DECLARE_OBJECT(CRealMyRoomForm983)

public:
	CRealMyRoomForm983()
	{
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_pEnchantDlg983) == 0x1f4 && offsetof(CRealMyRoomForm983, m_pFortuneDlg983) == 0x20c);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_pClubWorkDlg983) == 0x23c && offsetof(CRealMyRoomForm983, m_pItemSearchBtn983) == 0x248 && offsetof(CRealMyRoomForm983, m_searchUid983) == 0x250);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_infoX983) == 0x440 && offsetof(CRealMyRoomForm983, m_pSelCharPCL983) == 0x48c && offsetof(CRealMyRoomForm983, m_selCharItemIds983) == 0x494);
		C_ASSERT(offsetof(CRealMyRoomForm983, m_pClubWorkDlg983) == sizeof(FrTaskForm) + (0x23c - 0x120)); // = MRF_OFS983(0x23c) do myroompreview983.cpp
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_storageGuidChar983) == 0x724 && offsetof(CRealMyRoomForm983, m_skinTid983) == 0x964 && offsetof(CRealMyRoomForm983, m_skinVScroll983) == 0x988 && offsetof(CRealMyRoomForm983, m_subTabDirty983) == 0xa12);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_viewNormal983) == 0xa35 && offsetof(CRealMyRoomForm983, m_viewMode983) == 0xa3c && offsetof(CRealMyRoomForm983, m_viewA74983) == 0xa74 && offsetof(CRealMyRoomForm983, m_pCardBook983) == 0xa78);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_storageOpen983) == 0xa84 && offsetof(CRealMyRoomForm983, m_pStorageBgi983) == 0xa90 && offsetof(CRealMyRoomForm983, m_pCardPackList983) == 0xab8); // A866
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_pSelCharAux983) == 0x4f4 && offsetof(CRealMyRoomForm983, m_previewOther983) == 0x528);
		C_ASSERT(offsetof(CRealMyRoomForm983, m_pSelCharAux983) >= 0x494 + 4 && offsetof(CRealMyRoomForm983, m_handRest983) <= 0x590); // crus +0x490/0x494 e +0x590.. fora dos membros no cl novo
		C_ASSERT_VC10(sizeof(CAnotherHand) == 0x188 && offsetof(CRealMyRoomForm983, m_pad983_588) + sizeof(CAnotherHand) == offsetof(CRealMyRoomForm983, m_docMapCopy983) && offsetof(CRealMyRoomForm983, m_guidCaddie983) == 0x720); // A868
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_mainTab983) == 0x52c && offsetof(CRealMyRoomForm983, m_subTab983) == 0x530);
		C_ASSERT(offsetof(CRealMyRoomForm983, m_subTab983) == sizeof(FrTaskForm) + (0x530 - 0x120)); // = MRF_OFS983(0x530) do myroompreview983.cpp
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_listOfs983) == 0x53c && offsetof(CRealMyRoomForm983, m_itemHideDescPos983) == 0x54c && offsetof(CRealMyRoomForm983, m_tikiGuid983) == 0x578 && offsetof(CRealMyRoomForm983, m_tikiParam983) == 0x580);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_cardTab983) == 0x554 && offsetof(CRealMyRoomForm983, m_openTid983) == 0x564);
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_pPreview983) == 0x584 && offsetof(CRealMyRoomForm983, m_previewBusy983) == 0x58c);
		C_ASSERT(offsetof(CRealMyRoomForm983, m_pPreview983) == sizeof(FrTaskForm) + (0x584 - 0x120) && offsetof(CRealMyRoomForm983, m_previewBusy983) == sizeof(FrTaskForm) + (0x58c - 0x120)); // = MRF_OFS983 do preview
		C_ASSERT_VC10(offsetof(CRealMyRoomForm983, m_cutinDirty983) == 0x124 && offsetof(CRealMyRoomForm983, m_pItemList983) == 0x144 && offsetof(CRealMyRoomForm983, m_pSkinWnd983) == 0x150 && offsetof(CRealMyRoomForm983, m_pPet983) == 0x18c && offsetof(CRealMyRoomForm983, m_pad983_1f0) == 0x1f0 && offsetof(CRealMyRoomForm983, m_pEnchantDlg983) == 0x1f4); // A864
		C_ASSERT(offsetof(CRealMyRoomForm983, m_pSkinWnd983) == sizeof(FrTaskForm) + (0x150 - 0x120) && offsetof(CRealMyRoomForm983, m_pSkinPet983) == sizeof(FrTaskForm) + (0x184 - 0x120) && offsetof(CRealMyRoomForm983, m_pPet983) == sizeof(FrTaskForm) + (0x18c - 0x120)); // = MRF_OFS983 do preview
		memset((char*)this + sizeof(FrTaskForm), 0, sizeof(*this) - sizeof(FrTaskForm));
		m_searchUid983 = 0xffffffff; // 00605518
	}
	// X do My Room apaga o form antes da widget: ~FrTaskForm zera CTaskWidget::m_pForm e o Close() da widget nao chega aqui (C180.1);
	// m_pTask != NULL = OnCloseWidget983 ainda nao rodou
	virtual ~CRealMyRoomForm983() { LoungeMyRoomState983(false); if (m_pTask) CRealMyRoomForm983::OnCloseWidget983(); delete CardBook983(); delete m_pArtifacts983; delete m_pTrophies983; }
	CCardBook*& CardBook983() { return m_pCardBook983; } // CCardBook do 983 (ShowCardHolic983 00605720 / CCardBook::Show 007f6d20)
	void ShowCardHolic983(bool show); // 00605720
	void SetViewMode983(int mode); // 006208f0
	void ShowUccHiddenCtrls983(bool bShow); // 00620e00 / 00610c00: controles atras do "ucc_draw"
	void SelectMenu983(int menu); // 00622630 (0, 6, 9)
	bool OnAuthDlgResult983(int result, FrForm* form); // 00610980
	bool OnAuthDlgFailResult983(int result, FrForm* form); // 00622940
	void OpenStoragePanel983(unsigned char type); // 00610ec0
	bool OnStoragePanelResult983(int result, FrForm* form); // 00605d70
	void BindStorageUI983(); // CItemStorage: 007fe140 nos INIT 0060a420..0060ab70/00615dd0/00620c30, 007fe1a0 em 00607b20..
	void BuildCardPackList983();
	void OnCardBookDraw983(int) { if (CardBook983()) CardBook983()->Draw(); }
	void OnCardBookBackDraw983(int) { if (CardBook983()) CardBook983()->DrawBackground(); } // 006056e0
	void OnCardPackListDraw983(int param);
	void OnCardPackListDown983();
	void OnItemListDown983();
	void OnItemListRDown983(); // 00622330 (item_list, RBUTTONDOWN)
	bool OnFortuneResult983(int result, FrForm* form); // 0060fa40
	bool OnRentalExtensionResult983(int result, FrForm* form); // 00611110
	bool OnFortuneOpenResult983(int result, FrForm* form); // 0061ed70
	bool OnReplayPlayResult983(int result, FrForm* form); // 006122f0
	bool OnReplayDeleteResult983(int result, FrForm* form); // 0061ce40
	bool OnUccDrawResult983(int result, FrForm* form); // 00610c00
	bool OnUccCopyResult983(int result, FrForm* form); // 00605d40
	bool OnUccListBtnDown983(FrListItem* pItem); // 00620e00, abas 0x16/0x1b
	void OpenReplayDlg983(sRecordedItemInfo* pReplay, const char* form, int mode, FRESH_PFN_RESULT pfn);
	bool OnCaddieInfoResult983(int result, FrForm* form); // 00613a50
	bool OnClubInfoResult983(int result, FrForm* form); // 00605ea0
	bool OnMascotInfoResult983(int result, FrForm* form); // 00605580
	void OpenCaddieInfo983(const sCaddieInfo& info); // 00613b00
	void OpenTikiReport983(sItemInfo* pInfo); // 00611fa0
	bool OnOpenTikiReport983(int result, FrForm* form); // 0060f5c0
	bool OnTikiReportResult983(int result, FrForm* form); // 00605d30
	void OpenInformation983(FrListItem* item); // 0061a8a0
	void ShowInformation983(unsigned long typeId, const sItemInfo* info = NULL); // 00615010
	void OnCardPackListRDown983(); // 00615c10
	void OnStorageListRDown983(); // 00615fa0
	void OnStorageListOwnerDraw983(int param); // 0060a6b0 (D667)
	void OnStorageListDown983(); // 0060a6e0
	void OnStorageListDClick983(); // 0060a7d0
	void OnStorageListMove983(); // 0060a830
	void OnItemListMove983(); // 00607010
	void OnItemSearchBtnUp983() { AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d8, 0, 0, 0, 0, 0)); } // 0060cdc0
	void OnStorageOpenBtnUp983() { SelectMenu983(9); } // 006229b0
	void ItemEquip983(FrListBox* box, FrListItem* item, bool equip);
	void OnCardTabDown983(int tab);
	void OnCardPackTab983() { OnCardTabDown983(0); }
	void OnCardVol1Tab983() { OnCardTabDown983(1); }
	void OnCardVol2Tab983() { OnCardTabDown983(2); }
	void OnCardVol3Tab983() { OnCardTabDown983(3); }
	void OnCardVol4Tab983() { OnCardTabDown983(4); }
	void OnCardVol5Tab983() { OnCardTabDown983(5); }
	void OnCardSpTab983() { OnCardTabDown983(100); } // 00605900: ButtonTabControl(100)
	void OnCardIndexDown983() { if (CardBook983()) CardBook983()->PageIndexBehavior(g_pFresh->GetManager()->GetMousePos(), 1); }
	void OnCardIndexMove983() { if (CardBook983()) CardBook983()->PageIndexBehavior(g_pFresh->GetManager()->GetMousePos(), 2); }
	void OnCardUseDown983(); // 006142e0 (card_use, LBUTTONDOWN)
	void OnMasteryMUp983(); // 0060d460 (updown_group.mastery_m, LBUTTONUP)
	void OnUpgradeUp983() { OnUpDownUp983(0); } // 0060d220 (updown_group.upgrade, LBUTTONUP)
	void OnDowngradeUp983() { OnUpDownUp983(1); } // 0060d340 (updown_group.downgrade, LBUTTONUP)
	void OnUpDownUp983(int mode);
	void OnClubWorkMUp983(); // 0061f850 (updown_group.clubwork_m, LBUTTONUP)
	bool OnClubWorkResult983(int result, FrForm* form); // 0061e4a0
	bool OnUpDownResult983(int result, FrForm* form) { m_pUpDownDlg983 = NULL; return true; } // 00605590
	bool OnEnchantResult983(int result, FrForm* form) { m_pEnchantDlg983 = NULL; return true; } // 00605570
	void OnEquipCancelUp983(); // 0061a640 (middle_ui.sbox.equip_cancel, LBUTTONUP)
	void OnEquipResetUp983(); // 0061a6c0 (middle_ui.sbox.equip_reset, LBUTTONUP)
	void OnTooltipHoverOn983(int param); // 0060ce20 (equip_cancel/equip_reset/upgrade/downgrade/clubwork_m, HOVERON)
	void OnTooltipHoverOff983(int param = 0); // 0060cf70 (HOVEROFF; NULL = sempre limpa)
	void OnCardSlotDown983(unsigned char slot, unsigned char type); // 006107f0 (645 CardBehavior)
	// card_slot1..12 LBUTTONDOWN 00612380..00612430: 006107f0(slot, 1); o 983 nao tem RBUTTONDOWN nas casas
	void OnCardSlot1Down983() { OnCardSlotDown983(0, 1); }
	void OnCardSlot2Down983() { OnCardSlotDown983(1, 1); }
	void OnCardSlot3Down983() { OnCardSlotDown983(2, 1); }
	void OnCardSlot4Down983() { OnCardSlotDown983(3, 1); }
	void OnCardSlot5Down983() { OnCardSlotDown983(4, 1); }
	void OnCardSlot6Down983() { OnCardSlotDown983(5, 1); }
	void OnCardSlot7Down983() { OnCardSlotDown983(6, 1); }
	void OnCardSlot8Down983() { OnCardSlotDown983(7, 1); }
	void OnCardSlot9Down983() { OnCardSlotDown983(8, 1); }
	void OnCardSlot10Down983() { OnCardSlotDown983(9, 1); }
	void OnCardSlot11Down983() { OnCardSlotDown983(10, 1); }
	void OnCardSlot12Down983() { OnCardSlotDown983(11, 1); }
	virtual bool OnOpenWidget983() { return true; }
	virtual bool OnCloseWidget983(); // 006165d0 (slot 39): so os 0x0020 sub 9 e sub 2 (B431, C174.1)
	virtual bool OnInit(); // 00620c30 (slot 32): parte ja portada, ver .cpp
	void SetMainTab983(int tab, bool select); // 0061e4e0
	void SelectSubTab983(int sub, bool force); // 0061d7a0 (so visibilidade; listas/helpers ainda nao portados)
	void AddMainTabs983(); // INIT 00607080 de middle_ui.tab_main
	void FillItemList983(int sub); // 0061af80 casos 2-4 (preenchimento; a selecao dos slots equipados nao esta portada)
	void SelectCharacter983(); // 0061eaa0
	void SelectCharacterByKey983(unsigned long key, int p3); // 0061eaa0 (com chave, chamada pela msg 0x48)
	int SelectClub983(unsigned long uid); // 0061cc80
	void SelectBall983(unsigned long tid); // 0061ce00
	void InitForm983(bool keepCaddieMascot); // 00620770
	virtual bool OnMsg983(const MsgObject& msg); // 00623500 (+0xa0): 0x48/49/4a/4b/75/e5/e6/eb, 0x297/0x2a7 (cartas)
	void OnSearchPick983(unsigned long tid, unsigned long uid); // 0062485f (0x2d6 da FrItemSearchDlg)
	void OnSearchChoose983(unsigned long tid, unsigned long uid); // 00624a7d (0x2d7 da FrItemSearchDlg)
	bool SelectCaddie983(unsigned long key); // 0061c960
	void SelectMascot983(unsigned long key); // 0061cb70 (parcial)
	void OnTabMainUp983(); // 0061f530 (middle_ui.tab_main, LBUTTONUP)
	void OnTabSubUp983(); // 0061f6b0 (middle_ui.tab_sub, LBUTTONUP)
	void UnequipItem983(unsigned long* slot); // 00615ab0
	void EquipActiveItem983(sItemInfo* info); // 00613d50
	void OpenNewRandomBox983(unsigned long tid); // 00615ca0
	void DeleteCutin983(sItemInfo* pInfo); // 006108e0
	void GiftButtonDown983(sItemInfo* pInfo); // 0061aae0 (A710: RandomBox, Supply Pack, Key of Fortune; A711: ano novo, Natal, S4)
	void OpenNewYearMoney983(sItemInfo* pInfo); // 006120c0
	bool OnOpenNewYearMoneyDlgResult983(int result, FrForm* pForm); // 0060f6f0
	void OpenEventXmasGiftBox983(sItemInfo* pInfo); // 00610d20
	bool OnOpenEventXmasGiftDlgResult983(int result, FrForm* pForm); // 00605d60
	void OpenS4EventGiftBox983(sItemInfo* pInfo); // 00612110
	bool OnOpenS4EventGiftDlgResult983(int result, FrForm* pForm); // 0060f8f0
	bool OnOpenNewRandomBoxDlgResult983(int result, FrForm* pForm); // 00610e00
	void OnItemListDClick983(); // 00614be0
	void OnItemSlotDClick983(); // 0061a710
	void OnItemSlotOwnerDraw983(int param); // 0060d0d0 (middle_ui.sbox.item_slot, OWNERDRAW: barra dos itens ativos equipados)
	void DrawCutin983(); // 0061ee90
	void DrawSkinBack983(); // 0060da80
	void DrawSkinSlot983(); // 0060de60
	void OnPetOwnerDraw983(int param); // 0061f6d0 (middle_ui.pet, OWNERDRAW: previa 3D)
	void OnItemListOwnerDraw983(int param); // 006166b0 (item_list, OWNERDRAW; B410: so fundo + icone + nome, sem botoes/ucc/rental)
	// 0060ca50 (slot 9 da vtbl): a previa (+0x584) roda Process a cada quadro (D323.1); sem isso o CExhibition nunca monta a pose.
	// A secao critica +0x5b8 do 983 nao e criada aqui (um so thread), so a flag +0x58c em volta.
	virtual void OnProc(const float deltaTime)
	{
		FrTaskForm::OnProc(deltaTime);
		CExhibition* pv = m_pPreview983;
		if (pv)
		{
			int tab = m_mainTab983;
			m_previewBusy983 = 1;
			pv->Process(deltaTime, !(tab == 1 || tab == 4));
			m_previewBusy983 = 0;
		}
		// 983 0060cb27: ESC sem janela de ESC fecha o mensageiro
		if (g_input->GetDown("ESCAPE", true) == 1 && !g_pFresh->GetManager()->HasEscKeyWindow())
			CMessengerInfo::Instance()->Close(false);
		// 983 0060cb5c: rolagem da skin animada da aba 4 (lida em DrawSkinBack983)
		if (m_mainTab983 == 4)
		{
			for (int i = 0; i < 2; i++)
			{
				float speed = i == 0 ? 138.0f : 228.0f;
				int dir = i == 0 ? m_skinHScroll983 : m_skinVScroll983;
				if (dir == 0)
					m_skinScroll983[i] = 0.0f;
				else if (dir == 1)
					m_skinScroll983[i] -= speed * 0.25f * deltaTime;
				else if (dir == 2)
					m_skinScroll983[i] += speed * 0.25f * deltaTime;
			}
		}
		// 983 0060cbd9: as dicas da lista sao regravadas no desenho; aqui saem a cada quadro para nao ficar presas fora do item
		switch (CIconToolTip::GetInstance()->GetCurTipIndex())
		{
		case TOOLTIP_EQUIP:
		case TOOLTIP_MOVEGIFT:
		case TOOLTIP_DELETE:
		case TOOLTIP_REEMPLOY:
		case TOOLTIP_REPLY:
		case TOOLTIP_HANDSEL:
		case TOOLTIP_OPENGIFT:
		case TOOLTIP_REPORT:
		case TOOLTIP_CARD:
		case UCC_TOOLTIP_DRAW:
		case UCC_TOOLTIP_COPY:
		case UCC_TOOLTIP_REDRAW:
		case UCC_TOOLTIP_EXHIBIT:
		case RMR_RM_DECO:
			CIconToolTip::GetInstance()->ClearToolTip();
			break;
		}
		// 983 0060cc93: 0x2c2 modo 2 ao "ItemBuff" (dica do mini botao, 0x4c)
		char buffTip = 0;
		AfxGetTask()->GetActor("ItemBuff") << MsgObject(NULL, M983(0x246), 2, (int)&buffTip, 0, 0, 0);
		if (buffTip)
			CIconToolTip::GetInstance()->ClearToolTip();
	}
	void OnCloseWndUp983() { Close(true); } // 00604eb0 (close_wnd, cmd 4): vtable[2](1) + SetB7a983(false) (feito no Close)

	DECLARE_FRESH_MSGMAP()

public:
	// A864: +0x120..+0x1f0 (publicos: a tabela s_myRoomBinds983 grava por offsetof)
	unsigned long m_pad983_120; // +0x120: sem leitor no fonte
	char m_cutinDirty983; // +0x124: cut-in mudou (0063f4b0 liga; SelectCharacterByKey983 manda 0x20 sub 9 e zera)
	char m_pad983_125[3];
	FrWnd* m_pMiddleUi983; // +0x128: "middle_ui"
	FrWnd* m_pSelHide983; // +0x12c: escondida ao selecionar bola/taco (sem bind no 983 INIT)
	FrWnd* m_pad983_130; // +0x130: sem leitor no fonte
	FrTabButton* m_pTabMain983; // +0x134: "middle_ui.tab_main"
	FrTabButton* m_pTabSub983; // +0x138: "middle_ui.tab_sub"
	FrWnd* m_pad983_13c; // +0x13c: sem leitor no fonte
	FrWnd* m_pad983_140; // +0x140: sem leitor no fonte
	FrListBox* m_pItemList983; // +0x144: "middle_ui.item_list"
	FrWnd* m_pTabTxt2a983; // +0x148: "middle_ui.tabtxt2"
	FrWnd* m_pTabTxt2b983; // +0x14c: "middle_ui.tabtxt2" (2o bind)
	FrWnd* m_pSkinWnd983; // +0x150: janela da skin/pet atual (= +0x184/+0x188/+0x18c)
	FrWnd* m_pad983_154; // +0x154: sem leitor no fonte
	FrWnd* m_pStatGroup983; // +0x158: "stat_group"
	FrWnd* m_pEPower983; // +0x15c: "stat_group.epower"
	FrWnd* m_pEControl983; // +0x160: "stat_group.econtrol"
	FrWnd* m_pEImpact983; // +0x164: "stat_group.eimpact"
	FrWnd* m_pESpin983; // +0x168: "stat_group.espin"
	FrWnd* m_pECurve983; // +0x16c: "stat_group.ecurve"
	FrWnd* m_pItemDescBg983; // +0x170: "middle_ui.item_desc_bg"
	FrWnd* m_pItemDesc983; // +0x174: "middle_ui.item_desc"
	FrWnd* m_pItemHideDescBg983; // +0x178: "item_hide_desc_bg"
	FrWnd* m_pItemHideDesc983; // +0x17c: "item_hide_desc"
	FrWnd* m_pItemHideName983; // +0x180: "item_hide_name"
	FrWnd* m_pSkinPet983; // +0x184: "middle_ui.skin_pet"
	FrWnd* m_pSkinPet2_983; // +0x188: "middle_ui.skin_pet2"
	FrWnd* m_pPet983; // +0x18c: "middle_ui.pet"
	FrListBox* m_pItemSlot983; // +0x190: "middle_ui.sbox.item_slot"
	FrWnd* m_pSBox983; // +0x194: "middle_ui.sbox"
	FrWnd* m_pUpDownGroup983; // +0x198: "updown_group"
	FrButton* m_pEquipCancel983; // +0x19c: "middle_ui.sbox.equip_cancel"
	FrButton* m_pEquipReset983; // +0x1a0: "middle_ui.sbox.equip_reset"
	FrButton* m_pUpgrade983; // +0x1a4: "updown_group.upgrade"
	FrButton* m_pDowngrade983; // +0x1a8: "updown_group.downgrade"
	FrWnd* m_pCaddieInfoBtn983; // +0x1ac: desligada com o caddie_info aberto (sem bind no 983 INIT)
	FrButton* m_pClubWork983; // +0x1b0: "updown_group.clubwork_m"
	FrWnd* m_pMastery983; // +0x1b4: "updown_group.mastery_m"
	FrWnd* m_pGiftBg983; // +0x1b8: "middle_ui.gift_bg"
	FrWnd* m_pGiftDesc983; // +0x1bc: "middle_ui.gift_desc"
	FrWnd* m_pStatText983; // +0x1c0: "stat_group.stat_text"
	FrWnd* m_pFTabMain983; // +0x1c4: "ftab_main"
	FrWnd* m_pFItemList983; // +0x1c8: "fitem_list"
	FrWnd* m_pad983_1cc; // +0x1cc: sem leitor no fonte
	FrWnd* m_pad983_1d0; // +0x1d0: sem leitor no fonte
	FrWnd* m_pad983_1d4; // +0x1d4: sem leitor no fonte
	FrWnd* m_pad983_1d8; // +0x1d8: sem leitor no fonte
	FrWnd* m_pMainBg1_983; // +0x1dc: "middle_ui.main_bg1"
	FrWnd* m_pMainBg2_983; // +0x1e0: "middle_ui.main_bg2"
	FrWnd* m_pMainBg3_983; // +0x1e4: "middle_ui.main_bg3"
	FrWnd* m_pMainBg4_983; // +0x1e8: "middle_ui.main_bg4"
	FrWnd* m_pChatBg983; // +0x1ec: "chat_bg"
	FrWnd* m_pad983_1f0; // +0x1f0: sem leitor no fonte
private:
	// dialogos filhos (983 zera +0x1f4..+0x20c juntos em 00612200)
	FrEnchantDlg* m_pEnchantDlg983; // +0x1f4: "enchant" (00624166), resultado 00605570
	FrDowngradeDlg* m_pUpDownDlg983; // +0x1f8: "downgrade" das setas upgrade/downgrade (0060d2dc/0060d3fc), resultado 00605590
	FrDowngradeDlg* m_pUpDownDlg2_983; // +0x1fc: o 983 so zera (00612229) e le no 0xf8 (00624125)
	unsigned long m_pad983_200; // +0x200: o 983 so zera (0061223b), sem leitor
	FrForm* m_pMascotInfoDlg983; // +0x204: info do mascote (0061a954), resultado 00605580
	FrForm* m_pForm208_983; // +0x208: dialogo com resultado em 00614f4d..00614fdf (nao portado)
	FrForm* m_pFortuneDlg983; // +0x20c: "fortune" (006223f0..0062241a), resultado 0060fa40
public:
	// A865: +0x210..+0x4f0 (publicos: a tabela s_myRoomBinds983 grava +0x248 por offsetof)
	FrFortuneOpenDlg* m_pFortuneOpenDlg983; // +0x210: "fortune_open_new" (00624d2b), resultado OnFortuneOpenResult983
	FrReplayModeDlg* m_pReplayDlg983; // +0x214: "replaymode"/"replaytape_del" (00620e00 caso 0x18)
	char m_pad983_218[0x228 - 0x218]; // +0x218: sem leitor no fonte
	FrUccDrawDlg* m_pUccDrawDlg983; // +0x228: "ucc_draw" (00620e00), resultado 00610c00
	FrUccCopyDlg* m_pUccCopyDlg983; // +0x22c: "ucc_copy" (00620e00), resultado 00605d40
	FrUniteResultDlg* m_pUniteResultDlg983; // +0x230: "uniteresult" do Tiki Report (0x282), resultado 00605d30
	char m_pad983_234[0x23c - 0x234]; // +0x234: sem leitor no fonte
	FrClubSetWorkshopMainDlg* m_pClubWorkDlg983; // +0x23c: oficina "clubset_workshop" (0061f850), resultado 0061e4a0
	FrClubSetInfoDlg* m_pClubInfoDlg983; // +0x240: "information" do taco (sub-aba 7), resultado OnClubInfoResult983
	CRealMyRoom_AuthorityDlg* m_pAuthDlg983; // +0x244: "realmyroom_authority" (00622730)
	FrWnd* m_pItemSearchBtn983; // +0x248: "item_search_btn" (00607200 bind)
	unsigned long m_pad983_24c; // +0x24c: bitmap SelectItem_Mark (00609190); aqui GetBitmap no desenho
	unsigned long m_searchUid983; // +0x250: uid escolhido na FrItemSearchDlg (00624a7d, 0x2d7)
	char m_pad983_254[0x440 - 0x254]; // +0x254: sem leitor no fonte
	float m_infoX983; // +0x440: pos.x - 300 do item da OpenInformation983
	float m_infoY983; // +0x444: pos.y - 450
	char m_pad983_448[0x48c - 0x448]; // +0x448: sem leitor no fonte
	char* m_pSelCharPCL983; // +0x48c: PCL do personagem escolhido
	unsigned long* m_pSelCharParts983; // +0x490: tidParts (typeIds) do personagem escolhido
	unsigned long m_selCharItemIds983[24]; // +0x494: copia de ItemIdList (uids)
	// selecao corrente das listas (A861; 983 SelectCharacter/Caddie/Mascot/Club/Ball 0061c5f0..0061cf40)
	unsigned long* m_pSelCharAux983; // +0x4f4: tidAuxParts do personagem escolhido (aneis, 0061f8b0 casos 5/0x16)
	unsigned long m_selCharTid983; // +0x4f8: tid do personagem (previa)
	unsigned long m_selCharGuid983; // +0x4fc: guid do personagem
	unsigned long m_selCaddieTid983; // +0x500: tid do caddie
	unsigned long m_selCaddieKey983; // +0x504: chave do caddie em m_caddieMap
	unsigned long m_selCaddieShowTid983; // +0x508: tidPart (ou tid) da previa do caddie
	unsigned long m_selMascotGuid983; // +0x50c: guid do mascote
	unsigned long m_selMascotKey983; // +0x510: chave do mascote
	unsigned long m_selClubKey983; // +0x514: chave do taco
	unsigned long m_selClubTid983; // +0x518: tid do taco
	char* m_pSelClubStats983; // +0x51c: &item+8 do taco
	char* m_pSelClubWork983; // +0x520: &item+0xa6 do taco
	unsigned long m_selBallTid983; // +0x524: tid da bola
	int m_previewOther983; // +0x528: 1 = previa mostra caddie/mascote/taco/bola em vez do personagem
	// abas (A862; 983 SetMainTab 0061e4e0 / SelectSubTab 0061d7a0, msg 0xe5); myroompreview983.cpp le +0x530 por MRF_OFS983
	int m_mainTab983; // +0x52c: aba principal (0 Clothes .. 4)
	int m_subTab983; // +0x530: sub-aba (lista de itens; 0x16 = inicial)
	// A867: +0x534..+0x554 (983 00604f40/006077c0/006078f0/00607990/0060e340)
	int m_pad983_534; // +0x534: so zerado (006052b0; 00605630(0) do init 00620770), sem leitor no 983
	char m_pad983_538[4]; // +0x538: byte so zerado (006050e0)
	float m_listOfs983[2]; // +0x53c: deslocamento x/y da lista por sub-aba (00604f40), lido em 006077c0/0060e340
	float m_itemDescPos983[2]; // +0x544: pos de middle_ui.item_desc (x, y+h), 006078f0
	float m_itemHideDescPos983[2]; // +0x54c: pos de item_hide_desc (00607990 / 0060e340)
	// A863: aba do Card Holic, dialogos notify_yesno e previa 3D; myroompreview983.cpp le +0x584/+0x58c por MRF_OFS983
	unsigned long m_cardTab983; // +0x554: aba do FrTabButton +0x134 (modo 4; 006226c0 zera)
	unsigned long m_newYearGuid983; // +0x558: guid do item (006120c0, resultado 0060f6f0)
	unsigned long m_pad983_55c; // +0x55c: sem leitor no fonte
	unsigned long m_giftGuid983; // +0x560: guid do presente (00610d20, resultado 00605d60)
	unsigned long m_openTid983; // +0x564: tid do item a abrir (00612110 / 00615ca0)
	char m_pad983_568[0x578 - 0x568]; // +0x568: sem leitura/escrita no 983
	// A867: Tiki report (00611fa0 grava; 0060f5c0 = OnOpenTikiReport manda 0xab {guid, param} e zera)
	unsigned long m_tikiGuid983; // +0x578: guid do item
	unsigned long m_tikiTid983; // +0x57c: tid do item
	unsigned long m_tikiParam983; // +0x580: Common[1] * 0x8000 + Common[2]
	CExhibition* m_pPreview983; // +0x584: previa 3D (CExhibition / CPetFrame), apagada em 00605530
	unsigned long m_pad983_588; // +0x588: vptr do CAnotherHand do 983 (nao criado aqui, ver A868 abaixo)
	int m_previewBusy983; // +0x58c: 1 = previa ocupada (Process/ShowPet)
	// A868: o 983 embute um CAnotherHand em +0x588..+0x710 (ctor 0098ad90 em 0061f32e); aqui a previa e sincrona e ele nao e criado.
	// +0x588 = vptr, +0x58c = m_bUserLock/m_bQueseLock; o resto (m_handle ate m_aWork) fica reservado. Lista: out/F5_limpeza/visoes_pendentes.tsv
	char m_handRest983[0x710 - 0x590]; // +0x590: CAnotherHand::m_handle ..
	unsigned long m_docMapCopy983[3]; // +0x710: std::map copiado de doc+0x3704 (cabeca +0x714, tamanho +0x718; 0061f33c/00620c52); nao portado
	unsigned long m_pad983_71c; // +0x71c: sem leitura/escrita no 983
	unsigned long m_guidCaddie983; // +0x720: cache de doc+0xa26 sUserEquip::guidCaddie (copia 00620c82, compara 00614d52; nunca gravado aqui)
	// A866: +0x724..+0xab8 (publicos: a tabela s_myRoomBinds983 grava +0xa90..+0xab8 por offsetof)
	int m_storageGuidChar983; // +0x724: guid do personagem comparado em 00615dd0 (nunca gravado aqui)
	char m_pad983_728[0x954 - 0x728]; // +0x728: sem leitor no fonte
	std::list<sSpecialTrophy>* m_pTrophies983; // +0x954: 983 tem a lista aqui em linha (0061f3a0); aba 0x13
	char m_pad983_958[0x964 - 0x958]; // +0x958: sem leitor no fonte
	unsigned long m_skinTid983[6]; // +0x964: as 6 skins equipadas (doc+0xa76), 0061d7a0
	float m_skinScroll983[2]; // +0x97c: deslocamento x/y da skin animada
	int m_skinHScroll983; // +0x984: HScroll da skin (+0x964)
	int m_skinVScroll983; // +0x988: VScroll
	char m_pad983_98c[0xa12 - 0x98c]; // +0x98c: sem leitor no fonte
	bool m_subTabDirty983; // +0xa12: refaz a sub-aba mesmo igual (0061d7a0)
	char m_pad983_a13[0xa35 - 0xa13]; // +0xa13: sem leitor no fonte
	char m_viewNormal983; // +0xa35: modo 1 (006208f0)
	char m_pad983_a36[0xa3c - 0xa36]; // +0xa36: sem leitor no fonte
	int m_viewMode983; // +0xa3c: modo da janela (1 = normal, 2, 4 = bit 8), 006208f0
	int m_prevViewMode983; // +0xa40: modo anterior
	char m_viewFlags983[4]; // +0xa44: 00605a50 zera; bits 1/2/4/8 do modo
	char m_viewFlagA48983; // +0xa48: 00605a50 zera
	char m_pad983_a49[0xa74 - 0xa49]; // +0xa49: sem leitor no fonte
	char m_viewA74983; // +0xa74
	char m_pad983_a75[0xa78 - 0xa75]; // +0xa75: sem leitor no fonte
	CCardBook* m_pCardBook983; // +0xa78: CCardBook do 983 (ShowCardHolic983 00605720 / CCardBook::Show 007f6d20)
	char m_pad983_a7c[0xa80 - 0xa7c]; // +0xa7c: sem leitor no fonte
	FrItemStorageAccessPanel* m_pStorageAccess983; // +0xa80: "item_storage_access_panel" (00610ec0)
	char m_storageOpen983; // +0xa84: armazem de itens aberto (m_b_D98 do 645)
	char m_pad983_a85[0xa90 - 0xa85]; // +0xa85: sem leitor no fonte
	FrWnd* m_pStorageBgi983; // +0xa90: "item_storage_bgi" (00615dd0)
	FrWnd* m_pStoragePang983; // +0xa94: "item_storage_pang"
	FrWnd* m_pPangMinus983; // +0xa98: "pang_minus"
	FrWnd* m_pPangPlus983; // +0xa9c: "pang_plus"
	FrListBox* m_pStorageList983; // +0xaa0: "item_storage_list" (m_pFrListBox_DB4 do 645)
	FrWnd* m_pStorageIndex983; // +0xaa4: "item_storage_index"
	FrWnd* m_pStoragePageNext983; // +0xaa8: "item_storage_page_next"
	FrWnd* m_pStoragePagePrev983; // +0xaac: "item_storage_page_prev"
	FrWnd* m_pStorageClose983; // +0xab0: "item_storage_close"
	FrWnd* m_pStorageDragDrop983; // +0xab4: "item_storage_dragdrop"
	FrListBox* m_pCardPackList983; // +0xab8: "cardpacklist" (00614260)
private:
	bool m_bExhibitWait983; // +0xabc: 1 entre o pedido de venda do Self Design (0x291) e a resposta (0x292)
	char m_pad983_abd[0xac0 - 0xabd]; // +0xabd: sem leitor no fonte
	CItemCursor983* m_pArtifacts983; // +0xac0: 983 tem a lista aqui em linha (ctor 004797a0 em 0061f3b1); aba 0x26
	char m_pad983_ac4[0xae0 - 0xac4]; // +0xac4: sem leitor no fonte
};
// A750: os crus (char*)this+0x124..0xab8 caem todos no m_pad (no cl novo FrTaskForm 0x110 -> objeto 0xad0, ainda >= 0xabc)
C_ASSERT_VC10(sizeof(FrTaskForm) == 0x120 && sizeof(CRealMyRoomForm983) == 0xae0);
C_ASSERT(sizeof(FrTaskForm) <= 0x124 && sizeof(CRealMyRoomForm983) >= 0xab8 + 4);

// A769: o 983 (00606e90) grava FrListBox::m_useDummy inline (byte em +0x114 no VC10); visao so para o membro protegido
class FrListBoxDummy983 : public FrListBox
{
public:
	void SetUseDummy983() { C_ASSERT_VC10(offsetof(FrListBoxDummy983, m_useDummy) == 0x114); m_useDummy = true; }
};

// Controles do layout "realmyroom" ligados a campos do 983 pelos handlers INIT do msgmap 00e1347c (tabela gerada por
// tools/f37/myroom_binds.py -> out/F5/myroom/binds983.tsv). off = deslocamento no objeto; hide = o INIT so esconde o controle.
// O 983 grava o ponteiro durante a criacao do layout; aqui a busca por nome e feita no OnInit (equivalente). B340
struct MyRoomBind983
{
	const char* path;
	unsigned int off;
	bool hide;
};
static const MyRoomBind983 s_myRoomBinds983[] =
{
	{"middle_ui.item_list", offsetof(CRealMyRoomForm983, m_pItemList983), false}, // 00606e90 bind+logica
	{"middle_ui.tab_main", offsetof(CRealMyRoomForm983, m_pTabMain983), false}, // 00607080 bind+logica
	{"middle_ui.tab_sub", offsetof(CRealMyRoomForm983, m_pTabSub983), false}, // 00607190 bind
	{"item_search_btn", offsetof(CRealMyRoomForm983, m_pItemSearchBtn983), false}, // 00607200 bind
	{"chat_bg", offsetof(CRealMyRoomForm983, m_pChatBg983), false}, // 00607270 bind
	{"middle_ui.main_bg1", offsetof(CRealMyRoomForm983, m_pMainBg1_983), false}, // 006072f0 bind
	{"middle_ui.main_bg2", offsetof(CRealMyRoomForm983, m_pMainBg2_983), false}, // 00607360 bind
	{"middle_ui.main_bg3", offsetof(CRealMyRoomForm983, m_pMainBg3_983), false}, // 006073d0 bind
	{"middle_ui.main_bg4", offsetof(CRealMyRoomForm983, m_pMainBg4_983), false}, // 00607440 bind
	{"middle_ui", offsetof(CRealMyRoomForm983, m_pMiddleUi983), false}, // 006074b0 bind
	{"middle_ui.tabtxt2", offsetof(CRealMyRoomForm983, m_pTabTxt2a983), false}, // 00607520 bind
	{"middle_ui.tabtxt2", offsetof(CRealMyRoomForm983, m_pTabTxt2b983), false}, // 00607590 bind
	{"middle_ui.pet", offsetof(CRealMyRoomForm983, m_pPet983), false}, // 00607600 bind
	{"middle_ui.skin_pet", offsetof(CRealMyRoomForm983, m_pSkinPet983), false}, // 00607670 bind
	{"middle_ui.skin_pet2", offsetof(CRealMyRoomForm983, m_pSkinPet2_983), false}, // 006076e0 bind
	{"middle_ui.item_desc_bg", offsetof(CRealMyRoomForm983, m_pItemDescBg983), false}, // 00607750 bind
	{"middle_ui.item_desc", offsetof(CRealMyRoomForm983, m_pItemDesc983), false}, // 006078f0 bind+logica
	{"item_hide_desc", offsetof(CRealMyRoomForm983, m_pItemHideDesc983), false}, // 00607990 bind+logica
	{"item_hide_name", offsetof(CRealMyRoomForm983, m_pItemHideName983), false}, // 00607a40 bind
	{"middle_ui.sbox", offsetof(CRealMyRoomForm983, m_pSBox983), false}, // 00607ac0 bind+logica
	{"middle_ui.sbox.equip_cancel", offsetof(CRealMyRoomForm983, m_pEquipCancel983), false}, // 00607b60 bind+logica
	{"middle_ui.sbox.equip_reset", offsetof(CRealMyRoomForm983, m_pEquipReset983), false}, // 00607c10 bind+logica
	{"middle_ui.sbox.item_slot", offsetof(CRealMyRoomForm983, m_pItemSlot983), false}, // 00607cc0 bind
	{"middle_ui.gift_bg", offsetof(CRealMyRoomForm983, m_pGiftBg983), false}, // 00607d40 bind
	{"middle_ui.gift_desc", offsetof(CRealMyRoomForm983, m_pGiftDesc983), false}, // 00607dc0 bind
	{"stat_group", offsetof(CRealMyRoomForm983, m_pStatGroup983), false}, // 00607e40 bind
	{"stat_group.stat_text", offsetof(CRealMyRoomForm983, m_pStatText983), false}, // 00607ed0 bind
	{"stat_group.arrow_m", 0, true}, // 00607f40
	{"stat_group.epower", offsetof(CRealMyRoomForm983, m_pEPower983), false}, // 00607fa0 bind
	{"stat_group.econtrol", offsetof(CRealMyRoomForm983, m_pEControl983), false}, // 00608010 bind
	{"stat_group.eimpact", offsetof(CRealMyRoomForm983, m_pEImpact983), false}, // 00608080 bind
	{"stat_group.espin", offsetof(CRealMyRoomForm983, m_pESpin983), false}, // 006080f0 bind
	{"stat_group.ecurve", offsetof(CRealMyRoomForm983, m_pECurve983), false}, // 00608160 bind
	{"updown_group", offsetof(CRealMyRoomForm983, m_pUpDownGroup983), false}, // 006081d0 bind
	{"updown_group.upgrade", offsetof(CRealMyRoomForm983, m_pUpgrade983), false}, // 00608240 bind
	{"updown_group.downgrade", offsetof(CRealMyRoomForm983, m_pDowngrade983), false}, // 006082c0 bind
	{"updown_group.clubwork_m", offsetof(CRealMyRoomForm983, m_pClubWork983), false}, // 00608340 bind
	{"updown_group.mastery_m", offsetof(CRealMyRoomForm983, m_pMastery983), false}, // 006083c0 bind
	{"ftab_main", offsetof(CRealMyRoomForm983, m_pFTabMain983), false}, // 006091c0 bind+logica
	{"fitem_list", offsetof(CRealMyRoomForm983, m_pFItemList983), false}, // 006092c0 bind+logica
	{"item_hide_desc_bg", offsetof(CRealMyRoomForm983, m_pItemHideDescBg983), false}, // 00609350 bind
	{"card_book", 0, true}, // 006098a0
	{"cardpage_index", 0, true}, // 00609910
	{"card_property", 0, true}, // 006099d0
	{"card_pack", 0, true}, // 00609a40
	{"card_use", 0, true}, // 00609ad0
	{"card_volume1", 0, true}, // 00609b50
	{"card_volume2", 0, true}, // 00609be0
	{"card_volume3", 0, true}, // 00609c70
	{"card_volume4", 0, true}, // 00609d00
	{"card_sp", 0, true}, // 00609d90
	{"card_volume5", 0, true}, // 00609e20
	{"item_storage_pang", offsetof(CRealMyRoomForm983, m_pStoragePang983), false}, // 0060a420 bind
	{"pang_minus", offsetof(CRealMyRoomForm983, m_pPangMinus983), false}, // 0060a4a0 bind+logica
	{"pang_plus", offsetof(CRealMyRoomForm983, m_pPangPlus983), false}, // 0060a560 bind+logica
	{"item_storage_list", offsetof(CRealMyRoomForm983, m_pStorageList983), false}, // 0060a620 bind
	{"item_storage_index", offsetof(CRealMyRoomForm983, m_pStorageIndex983), false}, // 0060a850 bind
	{"item_storage_page_next", offsetof(CRealMyRoomForm983, m_pStoragePageNext983), false}, // 0060a940 bind+logica
	{"item_storage_page_prev", offsetof(CRealMyRoomForm983, m_pStoragePagePrev983), false}, // 0060aa00 bind+logica
	{"item_storage_close", offsetof(CRealMyRoomForm983, m_pStorageClose983), false}, // 0060aac0 bind+logica
	{"item_storage_dragdrop", offsetof(CRealMyRoomForm983, m_pStorageDragDrop983), false}, // 0060ab70 bind
	{"cardpacklist", offsetof(CRealMyRoomForm983, m_pCardPackList983), false}, // 00614260 bind
	{"item_storage_bgi", offsetof(CRealMyRoomForm983, m_pStorageBgi983), false}, // 00615dd0 bind+logica
};

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B404 temporario
// B411: rastro temporario - arvore de controles do layout (nome, visivel, rect) para achar o painel azul / quadrados cinza
struct Dump983 { int depth; };
static bool DumpChild983(FrWnd* w, void* parm)
{
	int depth = ((Dump983*)parm)->depth;
	std::string nm;
	w->GetWindowName(nm);
	const WRect& r = w->GetRect();
	jp983_rastro("B411 T %*s%s vis=%d %.0f,%.0f %.0fx%.0f", depth * 2, "", nm.c_str(), (int)w->IsVisible(), r.x, r.y, r.w, r.h);
	if (depth < 4)
	{
		Dump983 d = { depth + 1 };
		w->EnumerateChildWindow(DumpChild983, &d);
	}
	return true;
}

static FrWnd* FindByPath983(FrWnd* root, const char* path)
{
	char buf[128];
	strncpy(buf, path, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	FrWnd* w = root;
	for (char* tok = strtok(buf, "."); tok && w; tok = strtok(NULL, "."))
		w = w->FindChildByName(tok);
	return w;
}

// 00620c30: 007f3860 (ccardbook.cpp, copia a lista do CSharedDoc) e o ramo
// g_lobbyModal983 (00e78cac)==1 ainda NAO portados; aqui so os INIT do msgmap (campos de controle) e os campos fixos +0x52c/+0x530.
// (0061eaa0/0061e4e0/0061d7a0 = SelectCharacter983/SetMainTab983/SelectSubTab983, a parte; SelectSubTab983 parcial)
// 0098c3f0: sprintf em anel de 64 buffers de 0x240; aqui um anel local
static const char* TabImg983(const char* fmt, int n)
{
	static char s_ring[16][64];
	static unsigned s_i = 0;
	char* p = s_ring[s_i++ & 15];
	sprintf(p, fmt, n);
	return p;
}

// 00c07e10 com o trio n/o/d de "stext_%d_n"
static void AddSubTab983(FrTabButton* bar, int id)
{
	const char* img[3];
	img[0] = TabImg983("stext_%d_n", id);
	img[1] = TabImg983("stext_%d_n", id);
	img[2] = TabImg983("stext_%d_n", id);
	bar->AddTab(img, id);
}

// INIT 00607080 (middle_ui.tab_main): 6 abas, imagens stab_%d_n / _d / _d; a 1a usa o 7 e as outras 2..6 viram indices 1..5
void CRealMyRoomForm983::AddMainTabs983()
{
	FrTabButton* bar = m_pTabMain983;
	if (!bar)
		return;
	for (int k = 0; k < 6; ++k)
	{
		int img = (k == 0) ? 7 : k + 1;
		const char* names[3];
		names[0] = TabImg983("stab_%d_n", img);
		names[1] = TabImg983("stab_%d_d", img);
		names[2] = TabImg983("stab_%d_d", img);
		bar->AddTab(names, k);
	}
}

// 0061e4e0: troca a aba principal (+0x52c) e remonta as sub-abas (middle_ui.tab_sub, +0x138)
static void SendDresserParts983(CSharedDoc* sd);
void CRealMyRoomForm983::SetMainTab983(int tab, bool select)
{
	char* self = (char*)this;
	int& cur = m_mainTab983;
	if (!select)
	{
		if (cur == tab)
			return;
	}
	else
	{
		FrTabButton* main = m_pTabMain983;
		if (main)
			main->Select(cur);
	}
	{
		int curSub = m_subTab983;
		if (cur == 0 && tab != 0 && (curSub < 6 || curSub == 0x16 || curSub == 0x17 || curSub == 0x1b))
			SendDresserParts983(Doc()); // C238.1: sair de Clothes pela aba principal -> 0x0020 sub 0
	}
	cur = tab;
	FrTabButton* sub = m_pTabSub983;
	if (sub)
		sub->ClearTab();
	FrWnd* box = m_pUpDownGroup983;
	bool boxOn = false;
	int sel = -1;
	if (sub)
	{
		switch (cur)
		{
		case 0:
			AddSubTab983(sub, 0x16);
			for (int i = 2; i < 6; ++i)
				AddSubTab983(sub, i);
			if (IsLocalContent((localContentType_t)0x58))
				AddSubTab983(sub, 0x1b);
			AddSubTab983(sub, 0x17);
			boxOn = true; sel = 1;
			break;
		case 1:
			for (int i = 7; i < 11; ++i)
				AddSubTab983(sub, i);
			if (IsLocalContent((localContentType_t)0x52))
				AddSubTab983(sub, 0x1c);
			break;
		case 2:
			AddSubTab983(sub, 0xc);
			boxOn = true; sel = 1;
			break;
		case 3:
			AddSubTab983(sub, 0xd);
			boxOn = true; sel = 1;
			break;
		case 4:
			for (int i = 0xe; i < 0x12; ++i)
				AddSubTab983(sub, i);
			AddSubTab983(sub, 0x22);
			AddSubTab983(sub, 0x23);
			AddSubTab983(sub, 0x20);
			sel = 0;
			break;
		case 5:
			for (int i = 0x13; i < 0x16; ++i)
				AddSubTab983(sub, i);
			if (IsLocalContent((localContentType_t)0x14))
				AddSubTab983(sub, 0x19);
			if (IsLocalContent((localContentType_t)0x50))
				AddSubTab983(sub, 0x18);
			AddSubTab983(sub, 0x26);
			sel = 0;
			break;
		}
	}
	if (sel >= 0)
	{
		// 00605010 (realmyroommainui, visibilidade dos botoes de mastery/clubwork)(sel, 0): +0x1b4 recebe sel; +0x1b0, +0x1a4, +0x1a8 recebem 0
		FrWnd* w[4] = { m_pMastery983, ((FrWnd*)m_pClubWork983), ((FrWnd*)m_pUpgrade983), ((FrWnd*)m_pDowngrade983) };
		if (w[0]) w[0]->SetVisible(sel != 0);
		for (int i = 1; i < 4; ++i)
			if (w[i]) w[i]->SetVisible(false);
		if (box)
			box->SetVisible(boxOn);
	}
	FrWnd* a = m_pItemDesc983;
	FrWnd* c = m_pItemDescBg983;
	if (a) a->SetVisible(false);
	if (c) c->SetVisible(false);
	{
		static const int firstSub[6] = { 0x16, 7, 0xc, 0xd, 0xe, 0x13 };
		SelectSubTab983(cur == 0 ? 0x16 : firstSub[cur], true);
	}
}

// 983: o mesmo espelho do gameroomdlg.cpp (005e48a0 = CStatusHelper::GetMyFreeSlot983, alias em rfapelidos983.cpp)
class CStatusHelperJp
{
public:
	int GetBonus983(unsigned long type);
};

// Vis�o do FrWnd para o m_pScrBar (protegido) da lista: o 983 l� +0x90 direto (igual ao cardcomposedlg.cpp)
class FrWndScrJp983 : public FrWnd
{
public:
	FrScrollBar* GetScrBar983() { return m_pScrBar; }
};

// 0061a750: "%d/%d" (status / status + vagas livres) nos 5 textos stat_group.epower..ecurve (+0x15c..+0x16c);
// grava a string direto e liga +0x100 (m_smallFont) = SetCaption11, não SetCaption (C179: números grandes)
static void UpdateStats983(char* self)
{
	CStatusHelper helper;
	for (int i = 0; i < 5; ++i)
	{
		int cur = helper.GetStatus983(i);
		int free = ((CStatusHelperJp*)&helper)->GetBonus983(i);
		FrStatic* t = ((FrStatic**)&((CRealMyRoomForm983*)self)->m_pEPower983)[i];
		if (t)
		{
			char buf[32];
			sprintf(buf, "%d/%d", cur, cur + free);
			t->SetCaption11(buf);
		}
	}
}

// 00b2e310: sub-aba -> mascara de categorias (S5::UnderCategory)
// 006336a0: cut-in escondido (0x1e00000 sempre, 0x1f00000 sem identidade 0x2804, senao IsNonVisibleItem)
static bool CutinHidden983(CSharedDoc* doc, unsigned long tid)
{
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1e00000)
		return true;
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1f00000)
		return !(doc->m_myInfo.info.dwIdentity & 0x2804);
	return ItemManager()->IsNonVisibleItem(tid) == true;
}

static unsigned long CutinTid983(CSharedDoc* doc, unsigned long uid)
{
	for (std::list<sItemInfo>::iterator o = doc->m_cutinList.begin(); o != doc->m_cutinList.end(); ++o)
		if (o->id == uid)
			return o->typeId;
	return 0;
}

// 0063f450(uid, 0xff): uid esta nos cutIn[4] (+0x1b8) do personagem atual
static bool CutinEquipped983(CSharedDoc* doc, unsigned long uid)
{
	sCharacterInfo* cur = doc->FindChar983((unsigned long)-1);
	if (!cur)
		return false;
	for (int i = 0; i < 4; ++i)
		if (cur->cutIn[i] == uid)
			return true;
	return false;
}

// 0063f4b0 (equipar cut-in uid/tid no personagem atual): cut-in 0x38 da lista do doc, do personagem atual; troca o slot de mesma
// categoria (CutinInfo+0x10 em comum) ou usa o 1o livre
static bool CutinEquip983(CSharedDoc* doc, unsigned long uid, unsigned long tid)
{
	if (tid == 0 || uid == 0xffffffff || (tid & 0xfc000000) != 0x38000000)
		return false;
	unsigned long ctid = CutinTid983(doc, uid);
	if (ctid == 0 || CutinEquipped983(doc, uid))
		return false;
	IFF_STRUCT::S5::sCutinInformation* ci = ItemManager()->FindCutinInformation(ctid);
	sCharacterInfo* cur = doc->FindChar983((unsigned long)-1);
	if (!ci || !cur || (cur->tid & 0x3ffffff) != (unsigned long)ci->CharIndex)
		return false;
	for (int i = 0; i < 4; ++i)
	{
		IFF_STRUCT::S5::sCutinInformation* oc = ItemManager()->FindCutinInformation(CutinTid983(doc, cur->cutIn[i]));
		if (oc && (ci->Condition & oc->Condition))
		{
			cur->cutIn[i] = uid;
			return true;
		}
	}
	for (int i = 0; i < 4; ++i)
		if (cur->cutIn[i] == 0 || cur->cutIn[i] == 0xffffffff)
		{
			cur->cutIn[i] = uid;
			return true;
		}
	return false;
}

// 0063f620 (tirar cut-in uid): zera o slot se o cut-in e do personagem atual
static bool CutinUnequip983(CSharedDoc* doc, unsigned long uid)
{
	sCharacterInfo* cur = doc->FindChar983((unsigned long)-1);
	if (!cur)
		return false;
	for (int i = 0; i < 4; ++i)
		if (cur->cutIn[i] == uid)
		{
			IFF_STRUCT::S5::sCutinInformation* oc = ItemManager()->FindCutinInformation(CutinTid983(doc, uid));
			if (oc && (cur->tid & 0x3ffffff) != (unsigned long)oc->CharIndex)
				return false;
			cur->cutIn[i] = 0;
			return true;
		}
	return false;
}

static unsigned long MyRoomTabMask983(int sub)
{
	switch (sub)
	{
	case 1: case 0xe: case 0x17: return 0;
	case 2: case 0xf: return 1;
	case 3: case 0x10: return 2;
	case 4: return 0x10;
	case 5: return 0x2c;
	case 6: return 0x40;
	case 0x11: return 3;
	case 0x16: return 0x3bf;
	case 0x1b: return 0x380;
	case 0x1d: return 7;
	case 0x1e: return 9;
	case 0x1f: return 8;
	case 0x23: return 6;
	}
	return 0;
}

static void SelectEquippedSlots983(CSharedDoc* doc, FrListBox* box, unsigned long* slotType, unsigned long* slotUid)
{
	std::map<unsigned int, sCharacterInfo>::iterator ci = doc->m_charMap.find(doc->m_myInfo.userEquip.guidChar);
	for (int s = 0; ci != doc->m_charMap.end() && slotType && s < 0x18; ++s)
	{
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
		{
			sItemInfo* info = (sItemInfo*)(*li)->pData;
			if (!info)
				continue;
			unsigned int idx = (info->typeId >> 13) & 0x1f;
			bool hasUid = idx < 24 && ci->second.ItemIdList[idx] != 0;
			bool match = hasUid ? (info->typeId == slotType[s] && info->id == slotUid[s]) : (info->typeId == slotType[s]);
			if (match)
			{
				box->SelectItem(*li, false);
				break;
			}
		}
	}
}

// 0061af80 casos 2-4: esvazia o FrListBox (+0x144) e poe as pecas de m_partsList que nao estao equipadas (lista +0x111d8 do
// CSharedDoc, uid em +8 do elemento) e passam no filtro da aba (char em +0x4f8). Copia os 3 ushorts de FindPart(+0x228..22c)
// para +0xa4..a8 do item. Seleciona (SelectItem) os slots equipados; casos 5+ nao portados.

void CRealMyRoomForm983::FillItemList983(int sub)
{
	FrListBox* box = m_pItemList983;
	if (!box)
		return;
	box->ClearItem();
	CSharedDoc* doc = Doc();
	if (!doc)
		return;
	if (sub == 0x13)
	{
		// 0061af80 caso 0x13: trofeus da temporada + antigos (kind 0xb, j de 2 a 0), especiais, especiais antigos, contagens +0x3828; ao contrario
		C_ASSERT_VC10(offsetof(CSharedDoc, m_oldSpecialTrophyList) == 0x108d0 && offsetof(CSharedDoc, m_itemCounts983) == 0x3828);
		if (m_pTrophies983 == NULL)
			m_pTrophies983 = new std::list<sSpecialTrophy>;
		std::list<sSpecialTrophy>& trophies = *m_pTrophies983;
		trophies.clear();
		for (int i = 0; i < 13; ++i)
			for (int j = 2; j >= 0; --j)
				if (doc->m_myInfo.trophy.Trophy[i][j] != 0 || doc->m_oldSeasonTrophy.Trophy[i][j] != 0)
				{
					sSpecialTrophy t;
					t.guid = 0;
					t.tid = ((i | 0x2c00) << 16) | j;
					t.count = doc->m_myInfo.trophy.Trophy[i][j] + doc->m_oldSeasonTrophy.Trophy[i][j];
					trophies.push_back(t);
				}
		trophies.insert(trophies.end(), doc->m_specialTrophyList.begin(), doc->m_specialTrophyList.end());
		trophies.insert(trophies.end(), doc->m_oldSpecialTrophyList.begin(), doc->m_oldSpecialTrophyList.end());
		for (std::list<sItemCount983>::iterator it = doc->m_itemCounts983.begin(); it != doc->m_itemCounts983.end(); ++it)
		{
			sSpecialTrophy t = { it->uid, it->typeId, (unsigned long)it->count };
			trophies.push_back(t);
		}
		for (std::list<sSpecialTrophy>::reverse_iterator it = trophies.rbegin(); it != trophies.rend(); ++it)
			box->AddItem(&*it);
		return;
	}
	if (sub == 0x26)
	{
		// 0061af80 caso 0x26: 00478f40 refaz a lista de artefatos de m_myItemList; a linha guarda o no inteiro (item + flag de mana +0xc4)
		if (m_pArtifacts983 == NULL)
			m_pArtifacts983 = new CItemCursor983(&doc->m_myItemList);
		m_pArtifacts983->Reload983(&doc->m_myItemList);
		for (sArtifactNode983* n = m_pArtifacts983->m_pHead->pNext; n != m_pArtifacts983->m_pHead; n = n->pNext)
			box->AddItem(&n->value);
		return;
	}
	if (sub == 0xb)
	{
		// caso 0xb: m_caddieMap (+0x3718); seleciona o caddie do doc (+0xa26) ou o ja escolhido (+0x504). D
		unsigned long& selCaddie = m_selCaddieKey983;
		// D341: +0x504 residual (ctor) que nao e chave do mapa nao pode bloquear a selecao do caddie equipado do doc
		if (selCaddie != 0 && doc->m_caddieMap.find(selCaddie) == doc->m_caddieMap.end())
			selCaddie = 0;
		for (std::map<unsigned int, sCaddieInfo>::iterator it = doc->m_caddieMap.begin(); it != doc->m_caddieMap.end(); ++it)
		{
			FrListItem* li = box->AddItem(&it->second);
			if (selCaddie == 0)
			{
				if (doc->m_myInfo.userEquip.guidCaddie == it->first)
				{
					selCaddie = it->first;
					box->SelectItem(li, true);
				}
			}
			else if (selCaddie == it->first)
				box->SelectItem(li, true);
		}
		return;
	}
	if (sub == 0xd)
	{
		// caso 0xd: m_mascotMap (+0x3728); seleciona o mascote do doc (+0xa8e). D
		for (std::map<unsigned int, sMascotInfo>::iterator it = doc->m_mascotMap.begin(); it != doc->m_mascotMap.end(); ++it)
		{
			FrListItem* li = box->AddItem(&it->second);
			if (doc->m_myInfo.userEquip.guidMascot == it->first)
				box->SelectItem(li, true);
		}
		return;
	}
	if (sub == 0x22)
	{
		// 0061af80 caso 0x22: m_cutinList visiveis (009da070, 006336a0 != 1) cujo personagem (CutinInfo+0x18) e o da aba (+0x4f8) ou >= 100;
		// os que estao nos cutIn[4] do personagem atual (0063f450) ficam selecionados
		unsigned long charTid = m_selCharTid983 & 0x3ffffff;
		for (std::list<sItemInfo>::iterator it = doc->m_cutinList.begin(); it != doc->m_cutinList.end(); ++it)
		{
			unsigned long tid = it->typeId;
			if ((tid & 0xfc000000) != 0x38000000)
				continue;
			IFF_STRUCT::S5::sCutinInformation* ci = ItemManager()->FindCutinInformation(tid);
			if (!ci || CutinHidden983(doc, tid))
				continue;
			if (charTid != (unsigned long)ci->CharIndex && ci->CharIndex <= 99)
				continue;
			FrListItem* li = box->AddItem(&*it);
			if (CutinEquipped983(doc, it->id))
				box->SelectItem(li, false);
		}
		return;
	}
	if (sub == 0xe || sub == 0xf || sub == 0x10 || sub == 0x11 || sub == 0x23)
	{
		// 0061af80 casos 0xe-0x11/0x23: m_cutinList (+0x3798) filtrada pela categoria da aba (00b2e310; 5 = todas); 00b2d330 da o slot
		// (categoria do tid 0x38xxxxxx: 0-4, 6->5) e a peca equipada naquele slot (doc+0xa76+slot*4) fica selecionada
		unsigned long mask = MyRoomTabMask983(sub);
		for (std::list<sItemInfo>::iterator it = doc->m_cutinList.begin(); it != doc->m_cutinList.end(); ++it)
		{
			unsigned long tid = it->typeId;
			if (mask != 5 && ((tid >> 0x16) & 0xf) != mask)
				continue;
			FrListItem* li = box->AddItem(&*it);
			int slot = -1;
			if ((tid & 0xfc000000) == 0x38000000)
			{
				unsigned long c = (tid >> 0x16) & 0xf;
				if (c <= 4) slot = (int)c; else if (c == 6) slot = 5;
			}
			if (slot >= 0 && doc->m_myInfo.userEquip.tidSkin[slot] == tid)
				box->SelectItem(li, true);
		}
		return;
	}
	if (sub == 0x1d || sub == 0x1e || sub == 0x1f)
	{
		// 0061af80 casos 0x1d-0x1f: com NT REAL_MYROOM (bit 0x57), m_furnitureList (+0x37d4) cuja categoria (tid>>11&0x7ff) = mascara da aba (00b2e310)
		if (!IsLocalContent(S4_REAL_MYROOM))
			return;
		unsigned long mask = MyRoomTabMask983(sub);
		for (std::list<sFurniture_List>::iterator it = doc->m_furnitureList.begin(); it != doc->m_furnitureList.end(); ++it)
			if (((it->typeId >> 0xb) & 0x7ff) == mask)
				box->AddItem(&*it);
		return;
	}
	if (sub == 0x1c)
	{
		// 0061af80 caso 0x1c: com NT REAL_MYROOM (00a7a8d0), m_cardList (+0x37e0) inteira. cauda 006141b0/007f58a0 portada (B458)
		if (IsLocalContent(S4_REAL_MYROOM))
			for (std::list<sCards>::iterator it = doc->m_cardList.begin(); it != doc->m_cardList.end(); ++it)
				box->AddItem(&*it);
		BuildCardPackList983(); // 006141b0 (so mexe se cardpacklist +0xab8 existe)
		// 0061c65x: com NT REAL_MYROOM e o CCardBook (+0xa78) ja aberto, 007f58a0 = ReBuildCurrentPage
		CCardBook* book = CardBook983();
		if (IsLocalContent(S4_REAL_MYROOM) && book && book->IsShow())
			book->ReBuildCurrentPage();
		return;
	}
	if (sub == 0x20 || sub == 0x21)
	{
		// 0061af80 caso 0x20: m_furnitureList inteira menos as categorias 7-9; categoria 0xe com o tid em doc+0xa92/0xa96 vem marcada (+0xd);
		// caso 0x21: toda a lista
		for (std::list<sFurniture_List>::iterator it = doc->m_furnitureList.begin(); it != doc->m_furnitureList.end(); ++it)
		{
			unsigned long cat = (it->typeId >> 0xb) & 0x7ff;
			if (sub == 0x20 && cat >= 7 && cat <= 9)
				continue;
			FrListItem* li = box->AddItem(&*it);
			if (sub == 0x20 && cat == 0xe && (doc->m_myInfo.userEquip.poster[0] == it->typeId || doc->m_myInfo.userEquip.poster[1] == it->typeId))
				li->selected = true;
		}
		return;
	}
	if (sub == 0x18)
	{
		// 0061af80 caso 0x18: 00640fe0 rele save/ e lista m_recordedItemList (+0x3768)
		if (!IsLocalContent(S4_REPLAY_SYSTEM))
			return;
		doc->ReplaySavFileCheck();
		for (std::list<sRecordedItemInfo>::iterator it = doc->m_recordedItemList.begin(); it != doc->m_recordedItemList.end(); ++it)
			box->AddItem(&*it);
		return;
	}
	if (sub == 0x19)
	{
		// 0061af80 caso 0x19: com NT REAL_MYROOM (00a7a8d0), da m_myItemList (+0x375c) so o item 0x1a000042
		if (!IsLocalContent(S4_REAL_MYROOM))
			return;
		for (std::list<sItemInfo>::iterator it = doc->m_myItemList.begin(); it != doc->m_myItemList.end(); ++it)
			if ((it->typeId & 0x2000000) && it->typeId == 0x1a000042)
				box->AddItem(&*it);
		return;
	}
	if (sub == 0x14)
	{
		// 0061af80 caso 0x14: m_myItemList (+0x375c); so os itens de cupom (GetCouponKind != 0)
		for (std::list<sItemInfo>::iterator it = doc->m_myItemList.begin(); it != doc->m_myItemList.end(); ++it)
			if (doc->m_itemManager.GetCouponKind(it->typeId) != 0)
				box->AddItem(&*it);
		return;
	}
	if (sub == 0x15)
	{
		// 0061af80 caso 0x15: m_questList (+0x3834), todas as missoes
		for (std::list<sQuest>::iterator it = doc->m_questList.begin(); it != doc->m_questList.end(); ++it)
			box->AddItem(&*it);
		return;
	}
	if (sub == 7)
	{
		// casos 7: m_clubSetMap (+0x3744); seleciona o taco do jogo (+0xa2e) ou o ja escolhido (+0x514)
		unsigned long& selClub = m_selClubKey983;
		for (std::map<unsigned int, sItemInfo>::iterator it = doc->m_clubSetMap.begin(); it != doc->m_clubSetMap.end(); ++it)
		{
			FrListItem* li = box->AddItem(&it->second);
			if (selClub == 0)
			{
				if (DOCLOBBYJP()->GetClubSetId983() == it->first)
				{
					selClub = it->first;
					box->SelectItem(li, true);
				}
			}
			else if (selClub == it->first)
				box->SelectItem(li, true);
		}
		return;
	}
	if (sub == 8)
	{
		// caso 8: m_ballList (+0x3750); bola em uso +0xa32 -> selecionada em +0x524
		unsigned long& selBall = m_selBallTid983;
		unsigned long usedBall = doc->m_myInfo.userEquip.tidBall;
		bool flag = (doc->m_myInfo.info.dwIdentity >> 0xe & 1) != 0;
		if (flag)
			for (std::list<sItemInfo>::iterator it = doc->m_ballList.begin(); it != doc->m_ballList.end(); ++it)
				if (it->typeId == 0x140000d8)
				{
					FrListItem* li = box->AddItem(&*it);
					if (usedBall == it->typeId)
					{
						selBall = it->typeId;
						box->SelectItem(li, true);
					}
					break;
				}
		for (std::list<sItemInfo>::iterator it = doc->m_ballList.begin(); it != doc->m_ballList.end(); ++it)
		{
			if (flag && (it->typeId == 0x14000000 || it->typeId == 0x140000d8))
				continue;
			FrListItem* li = box->AddItem(&*it);
			if (usedBall == it->typeId)
			{
				selBall = it->typeId;
				box->SelectItem(li, true);
			}
		}
		return;
	}
	if (sub == 10)
	{
		// caso 10 (0061af80): m_myItemList (+0x375c); passivos = sem cupom, nao escondidos (006336a0), sem flags 6 em +0xc0 do
		// IFF (00a17260), tid != 0x1a000042 e com 0x2000000 no tid
		for (std::list<sItemInfo>::iterator it = doc->m_myItemList.begin(); it != doc->m_myItemList.end(); ++it)
		{
			unsigned long tid = it->typeId;
			if (doc->m_itemManager.GetCouponKind(tid) != 0)
				continue;
			bool hidden;
			if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1e00000)
				hidden = true;
			else if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1f00000)
				hidden = !(doc->m_myInfo.info.dwIdentity & 0x2804);
			else
				hidden = ItemManager()->IsNonVisibleItem(tid) == true;
			if (hidden)
				continue;
			void* iff = doc->m_itemManager.FindItem(tid);
			if (iff && (*(unsigned long*)((char*)iff + 0xc0) & 6))
				continue;
			if (tid == 0x1a000042 || !(tid & 0x2000000))
				continue;
			box->AddItem(&*it);
		}
		return;
	}
	if (sub == 9)
	{
		// caso 9: m_myItemList (+0x375c); itens sem cupom (kind 0, tid sem 0x2000000) cuja quantidade (Common[0]) excede
		// quantas vezes o tid ja esta nos 10 slots +0xa36..0xa5a. 005b6500 = list::push_back (listas locais de
		// tipo 1/2, sem efeito visivel). Slots: 2a metade abaixo. Ramo !NT_SC_ITEMSLOT: so AddItem dos n slots.
		for (std::list<sItemInfo>::iterator it = doc->m_myItemList.begin(); it != doc->m_myItemList.end(); ++it)
		{
			if (doc->m_itemManager.GetCouponKind(it->typeId) != 0 || (it->typeId & 0x2000000))
				continue;
			int used = 0;
			for (int k = 0; k < 10; ++k)
				if (it->typeId == doc->m_myInfo.userEquip.tidItemSlot[k])
					++used;
			if (it->Common[0] != used && it->Common[0] - used >= 0)
				box->AddItem(&*it);
		}
		// 2a metade (0061bdb4): lista de slots (+0x190) a partir dos 10 slots doc+0xa36; so com a lista de slots criada.
		FrListBox* slotBox = m_pItemSlot983;
		jp983_rastro("D313 Active slotBox=%p slot0=%08x slot1=%08x nAdd=%d", slotBox, doc->m_myInfo.userEquip.tidItemSlot[0], doc->m_myInfo.userEquip.tidItemSlot[1], (int)doc->GetNumAddItemSlotByMascot());
		if (!slotBox)
			return;
		slotBox->ClearItem();
		int n = doc->GetNumAddItemSlotByMascot() + 8;
		bool nt = IsLocalContent(S4_NT_SC_ITEMSLOT);
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			CCardManager::Instance()->SetPlayerIndex(0xff);
			CCardManager::Instance()->CalcCardPeriodAndStatus();
			int ps = CCardManager::Instance()->GetCardPeriodSlot();
			if (!nt || ps > 0)
				n += ps;
		}
		FrListBox* itemBox = m_pItemList983;
		for (int i = 0; i < 10; ++i)
		{
			unsigned long* slot = &doc->m_myInfo.userEquip.tidItemSlot[i];
			if (i < n)
			{
				slotBox->AddItem(slot);
				continue;
			}
			if (!nt)
				continue; // 983: sem NT_SC_ITEMSLOT so os n primeiros slots entram
			// 00615ab0 = CRealMyRoomMain::UnequipItem (do form): o slot fora da faixa volta ao inventario e e zerado
			UnequipItem983(slot);
		}
		int sz = slotBox->GetCurrentItemSize(true);
		for (int k = sz; k < 10; ++k)
			slotBox->AddItem(0);
		return;
	}
	if (sub == 0x17)
	{
		// 0061af80 caso 0x17: pecas (e, com 0x58, UCC) de personagem que a conta nao tem; sem marcar nada equipado
		for (int l = 0; l < 2; ++l)
		{
			if (l == 1 && !IsLocalContent((localContentType_t)0x58))
				break;
			std::list<sItemInfo>& src = l == 0 ? doc->m_partsList : doc->m_uccItemList;
			for (std::list<sItemInfo>::iterator it = src.begin(); it != src.end(); ++it)
			{
				if (it->typeId & 0x600)
					continue;
				bool owned = false;
				for (std::map<unsigned int, sCharacterInfo>::iterator ci = doc->m_charMap.begin(); ci != doc->m_charMap.end(); ++ci)
					if (((it->typeId >> 18) & 0xff) == (ci->second.tid & 0x3ffffff))
					{
						owned = true;
						break;
					}
				if (owned)
					continue;
				if (l == 0 && IsLocalContent((localContentType_t)0x52))
				{
					if (const IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->typeId))
					{
						char* r = (char*)part;
						char* d = (char*)&*it;
						*(unsigned short*)(d + 0xa4) = *(unsigned short*)(r + 0x228);
						*(unsigned short*)(d + 0xa6) = *(unsigned short*)(r + 0x22a);
						*(unsigned short*)(d + 0xa8) = *(unsigned short*)(r + 0x22c);
					}
				}
				box->AddItem(&*it);
			}
		}
		return;
	}
	if (sub == 0x1b)
	{
		// 0061af80 caso 0x1b: so a lista UCC (+0x3878) filtrada pela aba, com a mesma selecao dos slots
		unsigned long uccMask = MyRoomTabMask983(sub);
		for (std::list<sItemInfo>::iterator it = doc->m_uccItemList.begin(); it != doc->m_uccItemList.end(); ++it)
		{
			if (!S5::UnderCategory(m_selCharTid983, it->typeId, uccMask))
				continue;
			if (IsLocalContent((localContentType_t)0x52))
			{
				if (const IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->typeId))
				{
					char* r = (char*)part;
					char* d = (char*)&*it;
					*(unsigned short*)(d + 0xa4) = *(unsigned short*)(r + 0x228);
					*(unsigned short*)(d + 0xa6) = *(unsigned short*)(r + 0x22a);
					*(unsigned short*)(d + 0xa8) = *(unsigned short*)(r + 0x22c);
				}
			}
			box->AddItem(&*it);
		}
		SelectEquippedSlots983(doc, box, m_pSelCharParts983, m_selCharItemIds983);
		return;
	}
	unsigned long charType = m_selCharTid983;
	unsigned long mask = MyRoomTabMask983(sub);
	jp983_rastro("B404 Fill sub=%d charType=%08x mask=%08x parts=%d box=%p", sub, charType, mask, (int)doc->m_partsList.size(), box);
	int* equipHead = *(int**)&doc->m_listTradeItem; // 983 +0x111d8 (1o dword do objeto)
	int nChar = 0, nCat = 0;
	for (std::list<sItemInfo>::iterator it = doc->m_partsList.begin(); it != doc->m_partsList.end(); ++it)
	{
		if (((it->typeId >> 18) & 0xff) == (charType & 0x3ffffff))
			++nChar;
		if (!S5::UnderCategory(charType, it->typeId, mask))
			continue;
		++nCat;
		// caso 5: pecas do grupo A (CreateCharacterOption) so com taxa de mau comportamento boa e nivel > 0x14
		if (sub == 5 && CreateCharacterOption::Instance()->IsPartGroupA983(it->typeId) &&
			!(IsGoodMannerRate983() && doc->m_myInfo.stat.Level > 0x14))
			continue;
		bool equipped = false;
		if (equipHead)
			for (int* n = (int*)*equipHead; n != equipHead; n = (int*)*n)
				if ((unsigned long)n[4] == it->id) { equipped = true; break; }
		if (equipped)
			continue;
		const IFF_STRUCT::sPart* part = (sub == 0x16 && !IsLocalContent(S4_UCC)) ? 0 : ItemManager()->FindPart(it->typeId);
		if (part)
		{
			char* r = (char*)part;
			char* d = (char*)&*it;
			*(unsigned short*)(d + 0xa4) = *(unsigned short*)(r + 0x228);
			*(unsigned short*)(d + 0xa6) = *(unsigned short*)(r + 0x22a);
			*(unsigned short*)(d + 0xa8) = *(unsigned short*)(r + 0x22c);
		}
		box->AddItem(&*it);
	}
	jp983_rastro("B407 Fill porChar=%d porCat=%d equipHead=%p first=%08x", nChar, nCat, equipHead, doc->m_partsList.empty() ? 0 : doc->m_partsList.front().typeId);
	// selecao dos slots equipados (0061af80, laco 0..0x17): acha na lista o item cujo typeId (e uid, se o personagem tiver
	// ItemIdList no slot) bate com o slot do My Room (+0x490 = typeIds, +0x494 = uids) e o seleciona.
	SelectEquippedSlots983(doc, box, m_pSelCharParts983, m_selCharItemIds983);
	if (sub != 5 && sub != 0x16)
		return;
	// caso 5: tambem a lista auxiliar (+0x37bc) e a selecao dos 5 tidAuxParts (+0x4f4 = ponteiro, 0061b232; laco 0..0x14 bytes)
	// A688: antes lia +0x4f4 como array (o proprio ponteiro e +0x4f8..) -> anel equipado vindo do servidor nunca vinha marcado (D817 id=8)
	for (std::list<sItemInfo>::iterator it = doc->m_auxPartList.begin(); it != doc->m_auxPartList.end(); ++it)
		box->AddItem(&*it);
	unsigned long* auxType = m_pSelCharAux983;
	int nSel = 0;
	for (int s = 0; auxType && s < 5; ++s)
	{
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
		{
			sItemInfo* info = (sItemInfo*)(*li)->pData;
			if (info && info->typeId == auxType[s])
			{
				box->SelectItem(*li, false);
				++nSel;
				break;
			}
		}
	}
	// A801 (revisor 17:40, aneis da Nuri(R) desmarcados): personagem do form x doc e os 5 slots auxiliares
	jp983_rastro("A801 aux sub=%d guid=%08x tid=%08x docChar=%08x aux=%08x,%08x,%08x,%08x,%08x naux=%d sel=%d",
		sub, m_selCharGuid983, m_selCharTid983, doc->m_myInfo.userEquip.guidChar,
		auxType ? auxType[0] : 0, auxType ? auxType[1] : 0, auxType ? auxType[2] : 0, auxType ? auxType[3] : 0,
		auxType ? auxType[4] : 0, (int)doc->m_auxPartList.size(), nSel);
	if (sub != 0x16 || !IsLocalContent(S4_UCC))
		return;
	// caso 0x16 com UCC: lista +0x3878 (m_uccItemList) filtrada pela aba, depois a mesma selecao dos slots
	for (std::list<sItemInfo>::iterator it = doc->m_uccItemList.begin(); it != doc->m_uccItemList.end(); ++it)
	{
		if (!S5::UnderCategory(charType, it->typeId, mask))
			continue;
		const IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->typeId);
		if (part)
		{
			char* r = (char*)part;
			char* d = (char*)&*it;
			*(unsigned short*)(d + 0xa4) = *(unsigned short*)(r + 0x228);
			*(unsigned short*)(d + 0xa6) = *(unsigned short*)(r + 0x22a);
			*(unsigned short*)(d + 0xa8) = *(unsigned short*)(r + 0x22c);
		}
		box->AddItem(&*it);
	}
	SelectEquippedSlots983(doc, box, m_pSelCharParts983, m_selCharItemIds983);
}

// 0061cc80: seleciona o taco pelo uid (m_clubSetMap): grava +0x514 (uid), +0x518 (tid), +0x51c/+0x520 (dados). Taco (bit 0x80) abaixo do nivel do
// jogador vira o taco padrao (tid 0x10000000, ret 2); senao vira o taco do jogo (doc+0xa2e, ret 1). Nao achou: 0. Na aba 7: previa
// 3D do taco (0061cde2: 00605640(+0x518, 0)).
int CRealMyRoomForm983::SelectClub983(unsigned long uid)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	std::map<unsigned int, sItemInfo>::iterator it = doc->m_clubSetMap.find(uid);
	if (it == doc->m_clubSetMap.end())
	{
		return 0;
	}
	m_selClubTid983 = it->second.typeId;
	m_selClubKey983 = it->first;
	m_pSelClubStats983 = (char*)&it->second + 8;
	m_pSelClubWork983 = (char*)&it->second + 0xa6;
	int ret = 0;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(it->second.typeId);
	if (common)
	{
		unsigned char lvl = common->Level & 0x7f;
		unsigned char my = doc->m_myInfo.stat.Level;
		if (common->Level & 0x80)
		{
			if (lvl >= my)
				doc->m_myInfo.userEquip.guidClubSet = it->first, ret = 1;
			else
			{
				ret = 2;
				for (std::map<unsigned int, sItemInfo>::iterator d = doc->m_clubSetMap.begin(); d != doc->m_clubSetMap.end(); ++d)
					if (d->second.typeId == 0x10000000)
					{
						doc->m_myInfo.userEquip.guidClubSet = d->first;
						m_selClubTid983 = d->second.typeId;
						m_selClubKey983 = d->first;
						m_pSelClubStats983 = (char*)&d->second + 8;
						m_pSelClubWork983 = (char*)&d->second + 0xa6;
						break;
					}
			}
		}
		else if (lvl <= my)
			doc->m_myInfo.userEquip.guidClubSet = it->first, ret = 1;
	}
	if (m_subTab983 == 7)
	{
		MyRoomPreview983(self, m_selClubTid983, false);
		m_previewOther983 = 0; // 0061aa80
		UpdateStats983(self);
	}
	return ret;
}

// 0061ce00: bola escolhida (aba 8): +0x524 e a bola em uso (doc+0xa32); na aba 8 previa 00605640(&+0x524) e 0061aab0
void CRealMyRoomForm983::SelectBall983(unsigned long tid)
{
	char* self = (char*)this;
	m_selBallTid983 = tid;
	Doc()->m_myInfo.userEquip.tidBall = tid;
	if (m_subTab983 == 8)
	{
		MyRoomPreview983(self, tid, false);
		if (FrWnd* w = m_pSelHide983)
			w->SetVisible(false);
		m_previewOther983 = 1;
		UpdateStats983(self);
	}
}

static void SendDresserParts983(CSharedDoc* sd);
// 0061d7a0 (parcial): troca a sub-aba (+0x530) e ajusta a visibilidade dos grupos. NAO portados: 0061af80 (selecao nas listas
// do CSharedDoc), 0061a750 (stats %d/%d), 00605640 (abre widget do storage), pacotes ao LobbyAvatar/Caddie.
void CRealMyRoomForm983::SelectSubTab983(int sub, bool force)
{
	char* self = (char*)this;
	int& cur = m_subTab983;
	bool& dirty = m_subTabDirty983;
	if (!force && cur == sub && !dirty)
		return;
	dirty = false;
	// 0061d7f0: refaz a previa a forca ao entrar/sair das sub-abas de skin (0xe-0x10) e da 0x11/0x22
	int mainTab = m_mainTab983;
	bool fromSkin = cur == 0xe || cur == 0xf || cur == 0x10 || cur == 0x22;
	bool repaint = ((cur < 6 || cur == 0x16 || cur == 0x17 || cur == 0x1b || cur == 0x11 || cur == 0x22) && sub >= 0xe && sub <= 0x10)
		|| (mainTab == 0 && (fromSkin || cur == 0x11));
	if (mainTab == 0 && (cur < 6 || cur == 0x16 || cur == 0x17 || cur == 0x1b) && !(sub < 6 || sub == 0x16 || sub == 0x17 || sub == 0x1b))
		SendDresserParts983(Doc()); // C231.1: sair de Clothes -> 0x0020 sub 0
	if (sub == 6)
		sub = 2;
	cur = sub;
	FrTabButton* subBar = m_pTabSub983;
	if (subBar)
		subBar->Select(cur);
	FrWnd* giftBg = m_pGiftBg983;
	FrWnd* giftDesc = m_pGiftDesc983;
	FrWnd* stats = m_pStatGroup983;
	FrWnd* list = ((FrWnd*)m_pItemList983);
	FrWnd* box = m_pUpDownGroup983;
	FrWnd* sbox = m_pSBox983;
	if (giftBg) giftBg->SetVisible(false);
	if (giftDesc) giftDesc->SetVisible(false);
	if (stats) stats->SetVisible(false);
	if (list) list->SetVisible(true);
	bool statsOn = false;
	int boxOn = -1; // -1: nao mexe
	switch (cur)
	{
	case 2: case 3: case 4: case 5: case 0x16: case 0x17: case 0x1b: case 7:
		statsOn = true; boxOn = 1; break;
	case 0xb: case 0xd:
		statsOn = true; break;
	case 8: case 9: case 10: case 0xe: case 0xf: case 0x10: case 0x11: case 0x1c: case 0x20:
		boxOn = 0; break;
	}
	ShowCardHolic983(false); // 0061d8b8: 00605720(0) (so mexe se o Card Holic ja foi criado)
	if (cur == 0x1c)
	{
		// 0061db9e..0061dbc2 (case 0x1c): lista do item some, Card Holic aparece (00605720(1)); updown_group some
		if (list) list->SetVisible(false);
		ShowCardHolic983(true);
		if (box) box->SetVisible(false);
	}
	if (stats) stats->SetVisible(statsOn);
	if (box && boxOn >= 0) box->SetVisible(boxOn != 0);
	if (sbox) sbox->SetVisible(cur == 9);
	if (cur == 7)
	{
		// 0061d933: 00605010(0, 1): mastery_m (+0x1b4) some; clubwork_m, upgrade, downgrade (+0x1b0/0x1a4/0x1a8) aparecem
		FrWnd* w[4] = { m_pMastery983, ((FrWnd*)m_pClubWork983), ((FrWnd*)m_pUpgrade983), ((FrWnd*)m_pDowngrade983) };
		if (w[0]) w[0]->SetVisible(false);
		for (int i = 1; i < 4; ++i)
			if (w[i]) w[i]->SetVisible(true);
	}
	// 0061d8d6 (switch da sub-aba): previa do personagem (+0x4f8) ou apaga (00605530). 0x22/0x23 e 0x18/0x1c portados (B461; resto de 0x1c: 00605720 nao)
	// O 983 escolhe a janela (00604ed0) no fim e o ShowPet roda depois na thread do CAnotherHand; aqui e na hora, entao antes.
	// 0061d7a0: byte +0xf8 da lista (+0x144): 1 nas abas 2-5/0x16/0x17/0x1b e 0x20; 0 em 7-0x15 e 0x18/0x1c (estes so com NT REAL_MYROOM)
	if (list)
	{
		bool f8 = cur == 2 || cur == 3 || cur == 4 || cur == 5 || cur == 0x16 || cur == 0x17 || cur == 0x1b || cur == 0x20;
		bool zero = (cur >= 7 && cur <= 0x15 && cur != 0xc && cur != 0x12) || (cur == 0x18 && IsLocalContent(S4_REPLAY_SYSTEM)) || (cur == 0x1c && IsLocalContent(S4_CARD_SYSTEM));
		if (f8 || zero)
			((FrListBox*)list)->SetMultiSelect(f8); // +0xf8 = m_multiSelect (B1472: cru andava -0xc no cl 14.44)
	}
	MyRoomPetSelectWnd983(self);
	if (cur == 7)
	{
		FillItemList983(7); // B527: o 983 chama 0061af80 antes do switch; ela grava +0x514 (taco do doc) e sem isso SelectClub983(0) falhava (b526-1: uid=0)
		SelectClub983(m_selClubKey983); // 0061d948; B524: depois de escolher a janela da previa (antes o ShowPet caia sem wnd)
	}
	switch (cur)
	{
	case 2: case 3: case 4: case 5: case 0x16: case 0x17: case 0x1b:
		if ((repaint || m_viewMode983 != 1) && g_lobbyModal983)
			MyRoomPreview983(self, m_selCharTid983, repaint);
		else if (repaint || m_viewMode983 != 1)
			MyRoomPetClear983(self);
		break;
	case 8: // 0061d7a0 caso 8: 0061ce00(+0x524) = previa da bola (d408-1/015 x orig-d405-1/015)
		FillItemList983(8); // D417: 0061af80 roda antes do switch e grava +0x524 (bola em uso); sem isso a previa saia de tid 0 (d414-1/015)
		SelectBall983(m_selBallTid983);
		break;
	case 9: case 10: case 0xb: case 0x20:
		MyRoomPetClear983(self);
		break;
	case 0x22: case 0x23: // 0061d7a0: 0x22 poe o fundo st_cutin; ambos 0061aab0 (esconde +0x12c, +0x528 = 1, stats) e 00605640(+0x4f8)
		if (cur == 0x22)
			if (FrArea* t = (FrArea*)m_pTabTxt2a983) // 0061db0b: ecx = middle_ui.tabtxt2
				t->SetBgImg("st_cutin");
		if (FrWnd* w = m_pSelHide983)
			w->SetVisible(false);
		m_previewOther983 = 1;
		UpdateStats983(self);
		MyRoomPreview983(self, m_selCharTid983, repaint);
		break;
	case 0xe: case 0xf: case 0x10: case 0x11:
		MyRoomPreview983(self, m_selCharTid983, repaint);
		// 0061d7a0: as 6 skins equipadas (doc+0xa76) vao para +0x964..+0x978; sem categoria (tid & 0x3c00000 == 0) zera o deslocamento
		for (int i = 0; i < 6; i++)
		{
			unsigned long t = Doc()->m_myInfo.userEquip.tidSkin[i];
			m_skinTid983[i] = t;
			if (t && ItemManager()->FindSkin(t) && (t & 0x3c00000) == 0)
			{
				m_skinScroll983[0] = 0.0f;
				m_skinScroll983[1] = 0.0f;
				m_skinHScroll983 = ItemManager()->FindSkin(t)->HScroll;
				m_skinVScroll983 = ItemManager()->FindSkin(t)->VScroll;
			}
		}
		break;
	case 0x18: // 983 0061db66: 00605530(1) apaga a previa so com S4_REPLAY_SYSTEM
		if (IsLocalContent(S4_REPLAY_SYSTEM))
			MyRoomPetClear983(self);
		break;
	case 0x1c: // 983 0061db8a
		if (IsLocalContent(S4_CARD_SYSTEM))
			MyRoomPetClear983(self);
		break;
	}
	UpdateStats983(self);
	// 983 (0061d847): 0061af80 sem condicao
	if ((cur >= 2 && cur <= 5) || (cur >= 7 && cur <= 0x11 && cur != 0xc) || cur == 0x13 || cur == 0x14 || cur == 0x15 || cur == 0x16 || cur == 0x17 || cur == 0x18 || cur == 0x19 || cur == 0x1b || (cur >= 0x1c && cur <= 0x23) || cur == 0x26)
		FillItemList983(cur);
	else if (FrListBox* list = m_pItemList983)
		list->ClearItem(); // abas ainda sem lista (Decoration etc.): vazia, nao a grade da aba anterior (d259-1/014 x orig-d247-1/016)
	// 0061af80 (fim): completa a grade com casas vazias ate 8 (item_base_empty via OwnerDraw), em todas as abas (C169.1; a 0x13 so nao ordena)
	if (FrListBox* pad = m_pItemList983)
	{
		// 0061af80 (fim): AddItem(0) ate 8 em TODAS as abas; so o SortItem(00604b40) e pulado na 0x13 (Trophies; d405-1 x orig-d405-1/017)
		for (int n = pad->GetCurrentItemSize(true); n < 8; ++n)
			pad->AddItem(0);
		if (cur != 0x13)
			pad->SortItem(EmptyCompares);
	}
	// B559/C324.1: trocar de sub-aba abre a lista no TOPO (o SelectItem(true) acima rolava ate o selecionado; c322-1 x orig-c322-1)
	if (FrWnd* topList = ((FrWnd*)m_pItemList983))
		if (FrScrollBar* topBar = ((FrWndScrJp983*)topList)->GetScrBar983())
			topBar->SetCurTopRow(0);
	// 0061df17..: tela/camera do lobby conforme a aba principal (+0x52c): Clothes e Decoration(sub 0x20) = 5; Caddie = 6; as demais = 7
	// (ItemCamera, C159/M4); LobbyAvatar/LobbyCaddie recebem 0x1ce/0x1cf e o lobby recalcula atores/camera (005f8020)
	if (RFLobbyDesktop* lobby = RFLobbyDesktop::Instance983())
	{
		int screen = 7, ma = 0x1cf, mc = 0x1cf;
		if (mainTab == 0 || (mainTab == 4 && cur == 0x20))
		{
			screen = 5; ma = 0x1ce; mc = 0x1cf;
		}
		else if (mainTab == 2)
		{
			screen = 6; ma = 0x1cf; mc = 0x1ce;
		}
		lobby->SetScreen983(screen);
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, ma, 1, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, mc, 1, 0, 0, 0, 0);
		lobby->UpdateLobbyActors983();
	}
}

// 0061eaa0 (parte local, param_3 = 0): acha o personagem do guidChar do CSharedDoc em m_charMap e guarda os ponteiros
// do no (tidParts +0x490, tidAuxParts +0x4f4, PCL +0x48c), tid +0x4f8, guid +0x4fc e a copia de ItemIdList (+0x494, 24 dwords).
// A copia e condicionada a 00a7a8d0(tid) (bitset de tipos); aqui sempre. Pacote 0x9 do 0060e810 e o resto (0061af80 etc.) nao portados. B383
void CRealMyRoomForm983::SelectCharacter983()
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	std::map<unsigned int, sCharacterInfo>::iterator ci = doc->m_charMap.find(doc->m_myInfo.userEquip.guidChar);
	if (ci == doc->m_charMap.end())
		return;
	sCharacterInfo& c = ci->second;
	m_selCharTid983 = c.tid;
	m_pSelCharParts983 = c.tidParts;
	memcpy(m_selCharItemIds983, c.ItemIdList, 24 * sizeof(unsigned long));
	m_pSelCharAux983 = c.tidAuxParts;
	m_pSelCharPCL983 = c.PCL;
	m_selCharGuid983 = c.guid;
}

// 0061eaa0 (com chave): achou o personagem `key` em m_charMap -> mesmos campos do SelectCharacter983 + guidChar do doc (+0xa2a);
// se o doc esta no modo +0x8cb == +0x10708 e a aba principal e 0/4: reconstroi a PartTidList (632df0) e a lista da sub-aba (0061af80).
// Nao portado: 0050cf40 (entrada do personagem num mapa do doc). B449; previa 00605640 (A264); +0x124/abas A277
void CRealMyRoomForm983::SelectCharacterByKey983(unsigned long key, int p3)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	if (m_cutinDirty983 == 1)
	{
		// 0060e810: o cut-in do personagem atual mudou (0063f4b0/0063f620 no equipar/tirar) -> 0x0020 sub 9 antes de trocar
		if (sCharacterInfo* cur = doc->FindChar983((unsigned long)-1))
		{
			WSendPacket w((enumClientPacket)0x20);
			w.Encode1(9);
			w.Encode4(cur->guid);
			w.EncodeBuffer(cur->cutIn, 0x10);
			w.Send(TO_GAME);
		}
		m_cutinDirty983 = 0;
	}
	if (p3 != 0 && doc->m_myInfo.userEquip.guidChar != key)
		return;
	std::map<unsigned int, sCharacterInfo>::iterator ci = doc->m_charMap.find(key);
	if (ci == doc->m_charMap.end())
		return;
	sCharacterInfo& c = ci->second;
	m_selCharTid983 = c.tid;
	m_pSelCharParts983 = c.tidParts;
	memcpy(m_selCharItemIds983, c.ItemIdList, 24 * sizeof(unsigned long));
	m_pSelCharAux983 = c.tidAuxParts;
	m_pSelCharPCL983 = c.PCL;
	m_selCharGuid983 = c.guid;
	jp983_rastro("A801 char key=%08x tid=%08x aux=%08x,%08x,%08x,%08x,%08x", key, c.tid,
		c.tidAuxParts[0], c.tidAuxParts[1], c.tidAuxParts[2], c.tidAuxParts[3], c.tidAuxParts[4]);
	doc->m_myInfo.userEquip.guidChar = c.guid;
	int mainTab = m_mainTab983;
	if (doc->m_myInfo.info.dwUID == doc->m_replayTargetUID)
	{
		if (mainTab == 0 || mainTab == 4)
		{
			doc->BuildMyPartTidList(c);
			FillItemList983(m_subTab983);
			MyRoomPreview983(self, c.tid, false); // 0061ebe5/0061ec40: 00605640(+0x4f8)
			if (mainTab == 0)
			{
				// 0061aa80: mostra +0x12c (vtbl +0x18), +0x528 = 0 e refaz os stats
				if (FrWnd* w = m_pSelHide983)
					w->SetVisible(true);
				m_previewOther983 = 0;
				UpdateStats983(self);
			}
		}
		else if (m_previewOther983 == 1)
			UpdateStats983(self); // 0061a750
		// 006091a0: lista +0x144 volta ao topo (FrScrollBar::SetCurTopRow(0) do +0x90)
		if (FrWnd* list = ((FrWnd*)m_pItemList983))
			if (FrScrollBar* bar = ((FrWndScrJp983*)list)->GetScrBar983())
				bar->SetCurTopRow(0);
	}
	else if (mainTab == 0)
		doc->BuildMyPartTidList(c);
}

// 00620770 (init do form, msg 0x75): com o doc no modo +0x8cb == +0x10708 reaplica personagem (0061eaa0), taco (0061cc80), bola (0061ce00)
// e, se !keep, caddie (0061c960) e mascote (0061cb70) do CSharedDoc; refaz aba/sub-aba (+0x52c/+0x530) e mostra/esconde +0x12c (0061aa60:
// aba > 2 esconde, como 0061aab0). NAO portado: 00605630(0) (previa); as skins +0x964 ja vem de SelectSubTab983.
void CRealMyRoomForm983::InitForm983(bool keepCaddieMascot)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	if (doc->m_myInfo.info.dwUID != doc->m_replayTargetUID)
		return;
	SelectCharacterByKey983(doc->m_myInfo.userEquip.guidChar, 0);
	SelectClub983(doc->m_myInfo.userEquip.guidClubSet);
	SelectBall983(doc->m_myInfo.userEquip.tidBall);
	if (!keepCaddieMascot)
	{
		SelectCaddie983(doc->m_myInfo.userEquip.guidCaddie);
		SelectMascot983(doc->m_myInfo.userEquip.guidMascot);
	}
	SetMainTab983(m_mainTab983, true);
	SelectSubTab983(m_subTab983, true);
	if (FrWnd* w = m_pSelHide983)
		w->SetVisible(m_mainTab983 <= 2);
	m_previewOther983 = m_mainTab983 > 2 ? 1 : 0;
}

static void SelectMascotTail983(CSharedDoc* doc);
// C231.1: o original compara o doc com o cache do form (+0x720..) ao fechar e manda 0x0020 sub 1/3/8 so do que mudou;
// aqui o cache = o estado do doc no fim da sincronizacao de abertura (caddie, taco, bola, mascote)
static unsigned long s_syncCaddie983, s_syncClub983, s_syncBall983, s_syncMascot983, s_syncChar983;
static unsigned long s_syncSlots983[10], s_syncSkin983[6], s_syncPoster983[2]; // +0x730, +0x770, +0x78c
static std::map<unsigned int, sCharacterInfo> s_syncChars983;
// 0x0020 sub 0 (C231.1, 516 B): ao sair da aba Clothes/fechar o original manda o personagem inteiro se as partes do doc
// diferem do cache do form (Net_EquipDresserParts do 645; orig-c229-1). Cache = doc na abertura.
static void SendDresserParts983(CSharedDoc* sd)
{
	typedef std::map<unsigned int, sCharacterInfo> CharMap;
	for (CharMap::iterator it = s_syncChars983.begin(); it != s_syncChars983.end(); ++it)
	{
		CharMap::iterator dc = sd->m_charMap.find(it->first);
		if (dc == sd->m_charMap.end())
			continue;
		if (memcmp(dc->second.tidParts, it->second.tidParts, sizeof(it->second.tidParts)) == 0 &&
			memcmp(dc->second.ItemIdList, it->second.ItemIdList, sizeof(it->second.ItemIdList)) == 0 &&
			memcmp(dc->second.tidAuxParts, it->second.tidAuxParts, sizeof(it->second.tidAuxParts)) == 0)
			continue;
		WSendPacket w((enumClientPacket)0x20);
		// no fio sao 513 B (516 do original com 2 de id + sub): sem o padding de 3 B entre PCL[5] e mastery
		unsigned char wire[513];
		const unsigned char* src = (const unsigned char*)&dc->second;
		size_t pre = offsetof(sCharacterInfo, PCL) + sizeof(dc->second.PCL);
		memcpy(wire, src, pre);
		memcpy(wire + pre, &dc->second.mastery, sizeof(dc->second.mastery) + sizeof(dc->second.card));
		w.Encode1(0); w.EncodeBuffer(wire, sizeof(wire)); w.Send(TO_GAME);
		if (g_lobbyModal983 == 1 && it->first == sd->m_myInfo.userEquip.guidChar) // 00613590: Lounge
		{ WSendPacket c((enumClientPacket)0xc); c.Encode1(4); c.Encode4(it->first); c.Send(TO_GAME); }
		it->second = dc->second;
	}
}
// 00620c82: copia de sUserEquip e do mapa de personagens que os helpers de envio comparam com o doc
static void SnapshotEquip983(CSharedDoc* sd)
{
	const sUserEquip& eq = sd->m_myInfo.userEquip;
	s_syncCaddie983 = eq.guidCaddie;
	s_syncChar983 = eq.guidChar;
	s_syncClub983 = eq.guidClubSet;
	s_syncBall983 = eq.tidBall;
	s_syncMascot983 = eq.guidMascot;
	memcpy(s_syncSlots983, eq.tidItemSlot, sizeof(s_syncSlots983));
	memcpy(s_syncSkin983, eq.tidSkin, sizeof(s_syncSkin983));
	memcpy(s_syncPoster983, eq.poster, sizeof(s_syncPoster983));
	s_syncChars983 = sd->m_charMap;
}

// 00613930 (antes do sub 4): skin equipada com ItemType diferente de 0 e 2 passa a ItemType 2
static void MarkEquippedSkins983(CSharedDoc* sd)
{
	for (std::list<sItemInfo>::iterator it = sd->m_cutinList.begin(); it != sd->m_cutinList.end(); ++it)
	{
		unsigned long tid = it->typeId, c = (tid >> 0x16) & 0xf; // 00b2d330
		int slot = (tid & 0xfc000000) != 0x38000000 ? -1 : c <= 4 ? (int)c : c == 6 ? 5 : -1;
		if (slot >= 0 && sd->m_myInfo.userEquip.tidSkin[slot] == tid && it->ItemType != 0 && it->ItemType != 2)
			it->ItemType = 2;
	}
}

// 00614de0 (inicio): mascote com prazo vencido sai do equipado; depois o sub 8 leva 0 ao servidor. PC bang (+0x3d) nao sai.
static void DropExpiredMascot983(CSharedDoc* sd)
{
	sMascotInfo* pet = sd->GetMyEquippedMascotInfo();
	if (pet && pet->Remain_Date && !pet->PCBangMascot && ItemManager()->CompareSystemTime(pet->endDate, sd->GetServerTime()) <= 0)
		sd->m_myInfo.userEquip.guidMascot = 0;
}

// 00613590..0060e810 (caso 0x104 e 006165d0): 0x0020 do que mudou desde a abertura, na ordem do 983; sub 9 vai sempre.
static void SendEquipChanges983(CSharedDoc* sd)
{
	const sUserEquip& eq = sd->m_myInfo.userEquip;
	SendDresserParts983(sd);
	if (eq.guidChar != s_syncChar983)
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(5); w.Encode4(eq.guidChar); w.Send(TO_GAME); }
	if (eq.guidCaddie != s_syncCaddie983 && (eq.guidCaddie == 0 || sd->m_caddieMap.find(eq.guidCaddie) != sd->m_caddieMap.end()))
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(1); w.Encode4(eq.guidCaddie); w.Send(TO_GAME); }
	if (memcmp(eq.tidItemSlot, s_syncSlots983, sizeof(s_syncSlots983)) != 0)
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(2); w.EncodeBuffer((char*)eq.tidItemSlot, 0x28); w.Send(TO_GAME); }
	if (eq.guidClubSet != s_syncClub983 || eq.tidBall != s_syncBall983)
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(3); w.Encode4(eq.tidBall); w.Encode4(eq.guidClubSet); w.Send(TO_GAME); }
	if (memcmp(eq.tidSkin, s_syncSkin983, sizeof(s_syncSkin983)) != 0)
	{ MarkEquippedSkins983(sd); WSendPacket w((enumClientPacket)0x20); w.Encode1(4); w.EncodeBuffer((char*)eq.tidSkin, 0x18); w.Send(TO_GAME); }
	DropExpiredMascot983(sd);
	if (eq.guidMascot != s_syncMascot983 && (eq.guidMascot == 0 || sd->m_mascotMap.find(eq.guidMascot) != sd->m_mascotMap.end()))
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(8); w.Encode4(eq.guidMascot); w.Send(TO_GAME); }
	std::map<unsigned int, sCharacterInfo>::iterator ci = sd->m_charMap.find(eq.guidChar);
	if (ci != sd->m_charMap.end())
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(9); w.Encode4(ci->second.guid); w.EncodeBuffer(ci->second.cutIn, 0x10); w.Send(TO_GAME); }
}

// 0060e4b0..0060e810 (apos 00615dd0 e apos o caso 0x75 de HandleMsg): 0x0020 subs 1,2,3,4,8,2,9 com o estado do doc.
static void SendDocSyncPackets983(CSharedDoc* sd)
{
	// C159.1 (00615dd0, apos 0060e4b0): os caches do form (+0x720..) nunca sao gravados aqui, entao no original
	// (orig-c151-1/009) cada helper envia 0x0020 com o estado do doc: sub 1 caddie (00614d20), 2 partes (0060e550),
	// 3 (0060e600), 4 (00613930), 8 mascote (00614de0), 2 de novo (0063de40), 9 (0060e810).
	MarkEquippedSkins983(sd);
	DropExpiredMascot983(sd);
	SnapshotEquip983(sd);
	// 00614d20/00614de0: mandam se o cache do form (+0x720/+0x788) difere do doc, salvo id != 0 fora do mapa (doc+0x3718/
	// +0x3728 = end). Na abertura o cache e lixo de heap (ctor 0061f1f0 nao o inicia; so o OnInit 00620c76 copia o doc
	// depois): quase sempre != doc -> sub 1/8 = 0 tambem saem (orig-d324-1, orig-a315-1, orig-c151-1; orig-a307-1 = lixo 0).
	unsigned long caddie = sd->m_myInfo.userEquip.guidCaddie, mascot = sd->m_myInfo.userEquip.guidMascot;
	if (caddie == 0 || sd->m_caddieMap.find(caddie) != sd->m_caddieMap.end())
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(1); w.Encode4(caddie); w.Send(TO_GAME); }
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(2); w.EncodeBuffer((char*)sd->m_myInfo.userEquip.tidItemSlot, 0x28); w.Send(TO_GAME); }
	{
		WSendPacket w((enumClientPacket)0x20); w.Encode1(3);
		w.Encode4(sd->m_myInfo.userEquip.tidBall); w.Encode4(sd->m_myInfo.userEquip.guidClubSet); w.Send(TO_GAME);
	}
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(4); w.EncodeBuffer((char*)sd->m_myInfo.userEquip.tidSkin, 0x18); w.Send(TO_GAME); }
	if (mascot == 0 || sd->m_mascotMap.find(mascot) != sd->m_mascotMap.end())
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(8); w.Encode4(mascot); w.Send(TO_GAME); }
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(2); w.EncodeBuffer((char*)sd->m_myInfo.userEquip.tidItemSlot, 0x28); w.Send(TO_GAME); }
	{
		std::map<unsigned int, sCharacterInfo>::iterator ci = sd->m_charMap.find(sd->m_myInfo.userEquip.guidChar);
		if (ci != sd->m_charMap.end())
		{
			// 0060e810: 16 bytes em sCharacterInfo+0x1b8 = cutIn[4] (A277; antes +0x1b4, deslocado 4 bytes)
			WSendPacket w((enumClientPacket)0x20); w.Encode1(9);
			w.Encode4(ci->second.guid); w.EncodeBuffer(ci->second.cutIn, 0x10); w.Send(TO_GAME);
		}
	}
}

// 983 00622a10 (functor do 0x2d3 da FrItemSearchDlg, D638): nome do CItemManager (Part de categoria 8/9 com nome UCC
// gravado no item (+0x29) -> o nome UCC; Part sem IFF = fora); strstr(nome, palavra) -> {tid, uid, Icon, nome, 0}
static void AddSearchItem983(std::list<sItemSearchResult983>& out, const std::string& word, unsigned long tid,
	unsigned long uid, const sItemInfo* info)
{
	if (tid == 0 || uid == 0xffffffff)
		return;
	IFF_ITEM_COMMON* item = ItemManager()->FindCommonItem(tid);
	if (!item)
		return;
	std::string name = item->Name;
	if ((tid & 0xfc000000) == 0x8000000)
	{
		const IFF_STRUCT::sPart* part = ItemManager()->FindPart(tid);
		if (!part)
			return;
		if ((unsigned char)(*((const unsigned char*)part + 0xe8) - 8) < 2 && info && info->ItemName[0])
			name = info->ItemName;
	}
	if (!strstr(name.c_str(), word.c_str()))
		return;
	sItemSearchResult983 r;
	r.typeId = tid;
	r.uid = uid;
	r.text = item->Icon;
	r.name = name;
	r.linkedTypeId = 0;
	out.push_back(r);
}

// 00623140/006231f0/006232a0 (for_each em list): elemento {uid, tid, ...}; so as listas de sItemInfo passam o item (UCC)
template <class LIST>
static void SearchList983(LIST& l, std::list<sItemSearchResult983>& out, const std::string& word, bool withInfo)
{
	for (typename LIST::iterator it = l.begin(); it != l.end(); ++it)
	{
		const unsigned long* v = (const unsigned long*)&*it;
		AddSearchItem983(out, word, v[1], v[0], withInfo ? (const sItemInfo*)v : 0);
	}
}

// 00622c40/00622d50/00622e40 (for_each em map): valor {uid, tid, ...}, sem item
template <class MAP>
static void SearchMap983(MAP& m, std::list<sItemSearchResult983>& out, const std::string& word)
{
	for (typename MAP::iterator it = m.begin(); it != m.end(); ++it)
	{
		const unsigned long* v = (const unsigned long*)&(*it).second;
		AddSearchItem983(out, word, v[1], v[0], 0);
	}
}

// 006245b0 (0x2d3): busca no inventario do doc pelo grupo do combo da FrItemSearchDlg (0 roupa, 1 item/taco/bola/carta,
// 2 caddie, 3 mascote, 4 cutin, 5 mobilia)
static void SearchMyItems983(CSharedDoc* doc, std::list<sItemSearchResult983>& out, int group, const std::string& word)
{
	// mapas: o 983 le a cabeca (+4 do std::map) -> doc+0x3718/0x3728/0x3744; listas: doc+X = o proprio membro
	C_ASSERT_VC10(offsetof(CSharedDoc, m_caddieMap) == 0x3714);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_mascotMap) == 0x3724);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_partsList) == 0x3734);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_clubSetMap) == 0x3740);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_ballList) == 0x3750);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cutinList) == 0x3798);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_furnitureList) == 0x37d4);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cardList) == 0x37e0);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_uccItemList) == 0x3878);
	switch (group)
	{
	case 0:
		SearchList983(doc->m_partsList, out, word, true);
		SearchList983(doc->m_uccItemList, out, word, true);
		break;
	case 1:
		SearchList983(doc->m_myItemList, out, word, true);
		SearchMap983(doc->m_clubSetMap, out, word);
		SearchList983(doc->m_ballList, out, word, true);
		SearchList983(doc->m_cardList, out, word, false);
		break;
	case 2:
		SearchMap983(doc->m_caddieMap, out, word);
		break;
	case 3:
		SearchMap983(doc->m_mascotMap, out, word);
		break;
	case 4:
		SearchList983(doc->m_cutinList, out, word, true);
		break;
	case 5:
		SearchList983(doc->m_furnitureList, out, word, false);
		break;
	}
}

// 983 00605de0 (0x2d6): aba principal do My Room pelo tipo do tid (0 roupa, 1 item/taco/bola/carta, 2 caddie, 3 mascote, 4 cutin)
static int MainTabOfTid983(unsigned long tid)
{
	if (tid != 0)
	{
		switch (tid >> 0x1a)
		{
		case 4: case 5: case 6: case 0x1f: return 1;
		case 7: case 8: return 2;
		case 0xe: return 4;
		case 0x10: return 3;
		}
	}
	return 0;
}

// 983 006112c0 (0x2d6): sub-aba do My Room pelo tipo do tid (roupa pela Category do Part)
static int SubTabOfTid983(unsigned long tid)
{
	if (tid != 0)
	{
		switch (tid >> 0x1a)
		{
		case 2:
			switch (ItemManager()->FindPart(tid)->Category)
			{
			case 1: return 3;
			case 2: case 3: case 5: return 5;
			case 4: return 4;
			case 6: return 6;
			case 7: case 8: case 9: return 0x1b;
			}
			break;
		case 4: return 7;
		case 5: return 8;
		case 6: return ((tid & 0x2000000) != 0) + 9;
		case 7: return 0xb;
		case 0xe:
			switch ((tid >> 0x16) & 0xf)
			{
			case 0: case 4: case 5: return 0x22;
			case 1: return 0xf;
			case 2: return 0x10;
			case 3: return 0x11;
			case 6: return 0x23;
			}
			break;
		case 0x10: return 0xd;
		case 0x12:
			switch ((tid >> 0xb) & 0x7ff)
			{
			case 7: return 0x1d;
			case 8: return 0x1f;
			case 9: return 0x1e;
			case 0xe: return 0x20;
			default: return 0x21;
			}
		case 0x1c: return 5;
		case 0x1f: return 0x1c;
		}
	}
	return 2;
}

// 983 00643ed0: guid do primeiro personagem do m_charMap com esse tid (-1 = nenhum)
static unsigned long CharGuidOfTid983(CSharedDoc* doc, unsigned long tid)
{
	for (std::map<unsigned int, sCharacterInfo>::iterator it = doc->m_charMap.begin(); it != doc->m_charMap.end(); ++it)
		if (it->second.tid == tid)
			return (tid & 0xfc000000) == 0x4000000 ? it->second.guid : 0xffffffff;
	return 0xffffffff;
}

// 983 0062485f (0x2d6, D638): clique num resultado da FrItemSearchDlg (tid, uid) -> aba/sub-aba do item e o seu dono
// (personagem da peca/cutin, caddie, aba do caderno de cartas). Sem o CAnotherHand::IsWorking (previa sincrona aqui).
// Nao portado: 0061ce60 (caderno com +0xacc == 0) antes do ButtonTabControl de 0061ea10.
void CRealMyRoomForm983::OnSearchPick983(unsigned long tid, unsigned long uid)
{
	char* self = (char*)this;
	if (tid == 0 || uid == 0xffffffff)
		return;
	CSharedDoc* doc = Doc();
	int tab = MainTabOfTid983(tid);
	int sub = SubTabOfTid983(tid);
	if (m_mainTab983 != tab)
	{
		if (FrTabButton* main = m_pTabMain983)
			main->Select(tab);
		SetMainTab983(tab, false);
	}
	if (m_subTab983 != sub)
	{
		if (sub == 0xc)
			sub = 0xb;
		SelectSubTab983(sub, false);
	}
	switch (tab)
	{
	case 0: // roupa: personagem dono da peca (sub 1 = tid>>13, cabelo = cCharID do HairStyle, demais tid>>18)
	{
		unsigned long ch;
		if (sub == 1)
			ch = (tid >> 0xd) & 0xff;
		else if (sub == 6)
		{
			C_ASSERT(offsetof(IFF_STRUCT::sHairStyle, cCharID) == 0xc1);
			ch = ItemManager()->FindHairStyle(tid)->cCharID;
		}
		else
			ch = (tid >> 0x12) & 0xff;
		SelectCharacterByKey983(CharGuidOfTid983(doc, ch | 0x4000000), 0);
		break;
	}
	case 1: // carta (sub Card Holic): aba do caderno = IFF +0x172 nas cartas 0..2, 0 nas 3/4
		if (sub == 0x1c && (tid >> 0x1a) == 0x1f)
		{
			C_ASSERT(offsetof(IFF_STRUCT::sCard, Volume) == 0x172);
			IFF_STRUCT::sCard* card = ItemManager()->FindCard(tid);
			unsigned int type = (tid >> 0x16) & 0xf;
			if (type <= 2)
				OnCardTabDown983(card->Volume);
			else if (type <= 4)
				OnCardTabDown983(0);
		}
		break;
	case 2:
		SelectCaddie983(uid);
		break;
	case 4: // cutin: personagem do CharIndex (se nao for o atual) -> +0x4fc
		if (m_subTab983 == 0x22)
		{
			IFF_STRUCT::S5::sCutinInformation* ci = ItemManager()->FindCutinInformation(tid);
			if (!ci)
				break;
			C_ASSERT(offsetof(IFF_STRUCT::S5::sCutinInformation, CharIndex) == 0x18);
			unsigned long ch = (unsigned long)ci->CharIndex | 0x4000000;
			std::map<unsigned int, sCharacterInfo>::iterator cur = doc->m_charMap.find(doc->m_myInfo.userEquip.guidChar);
			if (cur == doc->m_charMap.end() || cur->second.tid != ch)
				m_selCharGuid983 = CharGuidOfTid983(doc, ch);
			SelectCharacterByKey983(m_selCharGuid983, 0);
		}
		break;
	}
}

// 983 00624a7d (0x2d7, D638): escolha de um resultado da FrItemSearchDlg (tid, uid), depois do 0x2d6 ter posto a aba/sub-aba.
// +0x250 = uid escolhido; por tipo do tid: carta (00612190 caderno / 006133e0 pacote), item (006225d0 -> 0061f8b0), roupa/taco/
// cutin/0x12 (item da lista +0x144 pelo uid: previa nas sub-abas 0x1d..0x21, SelectItem na 0x22). Depois o personagem ao
// "LobbyAvatar" (0x185) e a janela de selecao (005d3b90), rola a lista ate o uid (006145c0) e 0x2d9 ao ator principal.
void CRealMyRoomForm983::OnSearchChoose983(unsigned long tid, unsigned long uid)
{
	char* self = (char*)this;
	m_searchUid983 = uid;
	if (uid == 0xffffffff)
		return;
	FrListBox* box = m_pItemList983;
	switch (tid >> 0x1a)
	{
	case 0x1f:
	{
		unsigned int type = (tid >> 0x16) & 0xf;
		if (type <= 2)
		{
			// 00612190: pagina do caderno pelo CardIndex (12 por pagina, 1-based) e a casa da carta nessa pagina -> 006107f0(casa, 1)
			C_ASSERT(offsetof(IFF_STRUCT::sCard, CardIndex) == 0x174);
			C_ASSERT(offsetof(CCardSlot, m_pItem) == 0x24 && sizeof(CCardSlot) == 0x3c);
			CCardBook* book = CardBook983();
			IFF_STRUCT::sCard* card = ItemManager()->FindCard(tid);
			if (!book || !card)
				break;
			CCardPage* page = book->GetCardPage();
			unsigned int idx = card->CardIndex / 12; // 007f2fc0
			if (card->CardIndex == idx * 12)
				--idx;
			C_ASSERT(offsetof(CCardPage, m_page) == 0xd);
			page->m_page = (unsigned char)idx; // 983 +0xd (era cru; censo VS 2022, A827)
			book->BuildCardPage((unsigned char)idx); // 007f5430
			unsigned char slot = 0; // 007f2ff0
			for (; slot < 12; ++slot)
				if (page->m_pSlots[slot].m_pItem && page->m_pSlots[slot].m_pItem->tid == card->c.TypeId)
					break;
			OnCardSlotDown983(slot < 12 ? slot : 0, 1);
		}
		else if (type <= 4)
		{
			// 006133e0: pacote da cardpacklist (+0xab8) com esse tid: sem selecao -> SelectItem; selecionado -> o caminho do
			// clique (botao de uso + previa 0060e8b0). A fila de pacotes de 00befeb0 e o CAnotherHand embutido do 983 (+0x5a0 CS, +0x598 m_curWork): nao criado aqui, previa sincrona.
			FrListBox* pack = m_pCardPackList983;
			if (!box || !pack)
				break;
			for (FrListBox::ITEM_LIST::iterator li = pack->m_itemList.begin(); li != pack->m_itemList.end(); ++li)
			{
				FrListItem* it = *li;
				if (!it->pData || ((sCards*)it->pData)->tid != tid)
					continue;
				if (!it->selected)
					pack->SelectItem(it, true);
				else
				{
					g_audio->PlaySfx("ui_desktop_icon_click");
					if (CardBook983() && CardBook983()->m_pUseButton)
						CardBook983()->m_pUseButton->SetVisible(true);
					MyRoomShowPet983(self, tid); // 0060e8b0
				}
				break;
			}
		}
		break;
	}
	case 6:
		// 006225d0: item da lista +0x144 com esse tid -> 0061f8b0 (sub-abas 9/10: previa 3D e sfx, como o OnItemListRDown983)
		if (!box)
			break;
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
		{
			if ((*li)->pData && ((sItemInfo*)(*li)->pData)->typeId == tid)
			{
				MyRoomPreview983(self, tid, false);
				g_audio->PlaySfx("ui_desktop_icon_click");
				break;
			}
		}
		break;
	case 2: case 4: case 0xe: case 0x12:
	{
		// 00624af9: item da lista +0x144 pelo uid; sub-abas 0x1d..0x21 = previa (00605640) e sfx; 0x22 (cutin) = SelectItem
		if (!box)
			break;
		FrListItem* found = NULL;
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			if ((*li)->pData && ((sItemInfo*)(*li)->pData)->id == uid)
			{
				found = *li;
				break;
			}
		int sub = m_subTab983;
		if (sub >= 0x1d && sub <= 0x21)
		{
			if (found)
				MyRoomPreview983(self, ((sItemInfo*)found->pData)->typeId, false);
			g_audio->PlaySfx("ui_desktop_icon_click");
		}
		else if (sub == 0x22 && found)
			box->SelectItem(found, true);
		break;
	}
	}
	// 00624ba6: personagem do tid (se o jogador o tem) -> "LobbyAvatar" 0x185 e a janela de selecao 005d3b90
	CSharedDoc* doc = Doc();
	if (doc->FindChar983(CharGuidOfTid983(doc, ((tid >> 0x12) & 0xff) | 0x4000000)))
	{
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x185, 0, 0, 0, 0, 0);
		if (RFLobbyCharacterSelectWnd* w = RFLobbyCharacterSelectWnd::Instance983())
			w->Refresh983();
	}
	// 006145c0: rola a lista +0x144 ate a linha do uid (2 por linha; sem achar = linha do fim)
	if (box && uid != 0)
	{
		int row = 0;
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li, ++row)
			if ((*li)->pData && *(unsigned long*)(*li)->pData == uid)
				break;
		box->ResetTopRow();
		if (FrScrollBar* bar = box->GetScrollBar())
			bar->SetCurTopRow(row / 2);
	}
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2d9, 0, 0, 0, 0, 0));
}

// 00606c50: partes com tempo em horas (as outras vem em dias)
static bool IsHourPart983(unsigned long tid)
{
	switch (tid)
	{
	case 0x801680c: case 0x805880c: case 0x809880c: case 0x80dc80c: case 0x811880c: case 0x816080c:
	case 0x819080c: case 0x81e280c: case 0x821480c: case 0x825480c: case 0x839c80a:
		return true;
	}
	return false;
}

// 00605060
static void SendStartPeriodItem983(unsigned long uid)
{
	WSendPacket packet(0x20);
	packet.Encode1(7);
	packet.Encode4(uid);
	packet.Send(TO_GAME);
}

// 007fe140/007fe1a0 nos INIT: esconde os controles do armazem. Ele nunca abre no 983 (C719): emissor de 0x2b5 so no OnPacket morto 005a8360
void CRealMyRoomForm983::BindStorageUI983()
{
	if (!CItemStorage::IsInstantiated())
		return;
	CItemStorage* st = CItemStorage::Instance();
	st->AddFrontUI(m_pStorageBgi983);
	st->AddFrontUI(m_pStoragePang983);
	st->AddFrontUI(m_pPangMinus983);
	st->AddFrontUI(m_pPangPlus983);
	st->AddFrontUI(m_pStorageIndex983);
	st->AddFrontUI(m_pStorageClose983);
	st->AddFrontUI(m_pStorageDragDrop983);
	st->AddFrontUI(m_pStorageList983);
	if (m_pStoragePageNext983)
		m_pStoragePageNext983->SetVisible(false); // 0060a940/0060aa00
	if (m_pStoragePagePrev983)
		m_pStoragePagePrev983->SetVisible(false);
	if (IsLocalContent(S4_ITEM_STORAGE))
	{
		st->AddBackGroundUI(m_pSBox983, false); // 00607b20
		st->AddBackGroundUI(m_pEquipCancel983, false); // 00607bd0
		st->AddBackGroundUI(m_pEquipReset983, false); // 00607c80
	}
}

// 00610ec0: botoes do armazem na janela de autoridade (007f24e0, 007f2580, 007f2620); a posicao vem do layout
void CRealMyRoomForm983::OpenStoragePanel983(unsigned char type)
{
	if (m_pStorageAccess983 == NULL)
		m_pStorageAccess983 = CreateForm<FrItemStorageAccessPanel>(g_pFresh->GetManager(), this, "item_storage_access_panel", NULL);
	if (m_pStorageAccess983)
	{
		m_pStorageAccess983->SetFormType((FrItemStorageAccessPanel::eFormType)type);
		m_pStorageAccess983->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnStoragePanelResult983, 9);
	}
}

// 00605d70
bool CRealMyRoomForm983::OnStoragePanelResult983(int result, FrForm* form)
{
	bool ret = true;
	if (form)
	{
		FrItemStorageAccessPanel* panel = (FrItemStorageAccessPanel*)form;
		if (result == 1)
			panel->SendPasswordToServer();
		else if (panel->GetFormType() == FrItemStorageAccessPanel::FORMTYPE_FIRST)
			ret = false;
	}
	m_pStorageAccess983 = NULL;
	return ret;
}

// 00623500 (+0xa0): CRealMyRoomMain::HandleMsg do 645 reduzido ao que o 983 manda ao form. 0x48 (param2->guid, param3):
// trocou o personagem. Os outros casos (0x49 caddie, 0x4a mascote, ...) ainda nao portados -> falso.
bool CRealMyRoomForm983::OnMsg983(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x48:
		SelectCharacterByKey983(((sCharacterInfo*)msg.param2)->guid, msg.param3);
		return true;
	case 0x49: // 006241b4: caddie (*param2 = chave) + 0061a750
		SelectCaddie983(*(unsigned long*)msg.param2);
		UpdateStats983((char*)this);
		return true;
	case 0x4a: // HandleMsg: taco (*param2 = uid) + 0061a750 (0098ad40 = espera de mutex, sem efeito)
		SelectClub983(*(unsigned long*)msg.param2);
		UpdateStats983((char*)this);
		return true;
	case 0x75: // 0062433x: 525b10 = CLoginInfo::IsPcBang (vtable+0x34); !PcBang -> 00620770(0). Ramo PcBang (00605af0 flags doc+0xa44..a47 por aba -> 0061e460 caddie/mascote) so para conta PC bang: nao portado
		InitForm983(false);
		SendDocSyncPackets983(Doc()); // 0060e4b0/613590/614d20/60e550/60e600/614de0/60e810 (sem cache gravado = sempre envia)
		return true;
	case 0x104: // 006165d0 sem o fim: sair do jogo ou do servidor com o My Room aberto nao perde o que mudou
		SendEquipChanges983(Doc());
		return true;
	case 0x2eb: // 00624ca7: personagem novo (0x6b sub 4), a copia acompanha o doc para 00613590 nao o reenviar
		s_syncChars983 = Doc()->m_charMap;
		return true;
	case 0x2d3: // 006245b0 (D638): FrItemSearchDlg (param1 grupo, param2 &palavra, param3 &lista de resultados)
		SearchMyItems983(Doc(), *(std::list<sItemSearchResult983>*)msg.param3, msg.param1,
			*(const std::string*)msg.param2);
		return true;
	case 0x2d6: // 0062485f (D638): clique num resultado da FrItemSearchDlg (param1 tid, param2 uid)
		OnSearchPick983(msg.param1, msg.param2);
		return true;
	case 0x2d7: // 00624a7d (D638): escolha de um resultado da FrItemSearchDlg (param1 tid, param2 uid)
		OnSearchChoose983(msg.param1, msg.param2);
		return true;
	case 0x108: // 006244af: botao direito no item da FrCardRemove (007ff3a0)
		OpenInformation983((FrListItem*)msg.param1);
		return true;
	case 0x320: // 00623500: cor do cabelo (categoria 0xf de CItemUpdater, 00588380) no personagem em uso
	{
		IFF_STRUCT::sHairStyle* pHair = (IFF_STRUCT::sHairStyle*)msg.param1;
		CPartTidList& tids = Doc()->GetMyPartTidList();
		if ((tids.m_charTid & 0x3ffffff) != (unsigned long)pHair->cCharID)
			return true;
		if (g_lobbyModal983 == 0)
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x18c, (int)&tids, (int)&pHair->cHairID, 0, 0, 0);
		else if (m_pPreview983)
			MyRoomPetFrameSetHairClr983(m_pPreview983->GetPetFrame(), &tids, pHair->cHairID);
		return true;
	}
	case 0x4b: // bola (*param2 = tid), sem stats
		SelectBall983(*(unsigned long*)msg.param2);
		return true;
	case 0x281: // 00624d7c: CTask::_RemoveWareHouseItem (0x1a000042) pede o guid do Tiki e o form esquece
		if (IsLocalContent((localContentType_t)0x14))
		{
			*(unsigned long*)msg.param1 = m_tikiGuid983;
			m_tikiGuid983 = 0;
		}
		return true;
	case 0x2b2: // 0062596b: "storage_changepass" da janela de autoridade
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenStoragePanel983(2);
		return true;
	case 0x2b3: // "storage_lock"
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenStoragePanel983(3);
		return true;
	case 0x2b4: // "storage_unlock"
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenStoragePanel983(4);
		return true;
	case 0x282: // 00625812: pacote 0x11a (lista do relatorio pronta) -> resultado da partida; param1 = &_SYSTEMTIME
		if (IsLocalContent((localContentType_t)0x14) && m_pUniteResultDlg983 == NULL)
		{
			_SYSTEMTIME time = *(_SYSTEMTIME*)msg.param1;
			m_pUniteResultDlg983 = CreateForm<FrUniteResultDlg>(g_pFresh->GetManager(), this, "uniteresult", NULL);
			m_pUniteResultDlg983->EnableDrag(false);
			m_pUniteResultDlg983->SetTikiReportState(time);
			m_pUniteResultDlg983->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnTikiReportResult983, 1);
			return true;
		}
		break;
	case 0x296: // 00625487: carta usada (param2 1 = falhou) -> janela de resultado. +0x218 do 983 nunca e gravado
		if (msg.param2 != 1)
		{
			OpenCardUseResult983(this, msg.param1);
			if (CardBook983())
				CardBook983()->ReBuildCurrentPage();
		}
		return true;
	case 0x297: // 645 0x21a (00625510): cartas mudaram; caderno so se aberto (+4) -> 007f58a0, e refaz a lista Pack (006141b0)
		if (CardBook983() && CardBook983()->IsShow())
			CardBook983()->ReBuildCurrentPage();
		BuildCardPackList983();
		return true;
	case 0x2a7: // 645 0x229 (0062552d): abriu o Card Pack (C360.1) -> 007f58a0 + 006141b0; a lista refeita perde a selecao
		if (CardBook983())
			CardBook983()->ReBuildCurrentPage();
		BuildCardPackList983();
		return true;
	case 0xe5: // 983: devolve aba principal (+0x52c) e sub-aba (+0x530) pelos ponteiros param1/param2
		*(int*)msg.param1 = m_mainTab983;
		*(int*)msg.param2 = m_subTab983;
		return true;
	case 0xe6: // 983: grava aba principal (param1 < 6, senao 0 e sub 2) e sub-aba (param2; 1 vira 0x16)
	{
		char* self = (char*)this;
		if (msg.param1 < 6)
			m_mainTab983 = msg.param1;
		else
		{
			m_mainTab983 = 0;
			m_subTab983 = 2;
		}
		m_subTab983 = (msg.param2 == 1) ? 0x16 : msg.param2;
		return true;
	}
	case 0xeb: // 983 (6235bd): param1 = atalho de aba. 0 chama SelectMenu983(0) (00622630); 6 so aba 0/0x16 (SelectMenu(6) autoridade: nao portado); 1/6 -> aba 0/sub 0x16; 2: aba 5/0x18; 3: 0/0x1b; 4: 1/0x1c (2/3/4 so com NT REAL_MYROOM, 00a7a8d0)
	{
		char* self = (char*)this;
		int sel = msg.param1;
		int tab, sub;
		bool clearPet = false;
		bool local = IsLocalContent(S4_REAL_MYROOM);
		if (sel == 1 || sel == 6) { tab = 0; sub = 0x16; }
		else if (local && sel == 2) { tab = 5; sub = 0x18; clearPet = true; }
		else if (local && sel == 3) { tab = 0; sub = 0x1b; }
		else if (local && sel == 4) { tab = 1; sub = 0x1c; clearPet = true; }
		else if (sel == 0)
			return true; // 006235bd: 00622630(1) = default da tabela (nada)
		else
			return true;
		if (sel == 6 && local)
			SelectMenu983(9); // 00622630(9): 0xd3 + 0x0020 do doc (ou msg 0x2af ao main actor com +0xa84)
		m_mainTab983 = tab;
		m_subTab983 = sub;
		SetMainTab983(tab, true); // 0061e4e0
		SelectSubTab983(sub, true); // 0061d7a0
		if (clearPet)
			MyRoomPetClear983(self); // 00605530
		else
			SelectCharacter983(); // 0061eaa0
		return true;
	}
	case 0x28d: // 00624ebb: chave da UCC pronta (CSecurityKeyManager::ResCreateKey) -> envia o desenho
		if (m_pUccDrawDlg983 == NULL)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)"Fail : upload identify", 0, 0, 0, 0));
		else
			m_pUccDrawDlg983->Upload(); // 007e7a60
		return true;
	case 0x28e: // 00625269: upload falhou (NetResourceManager); param3 = codigo numerico do site
	{
		FrUccDrawDlg* dlg = m_pUccDrawDlg983;
		if (dlg == NULL || dlg->m_pItemInfo->tid != *(unsigned long*)msg.param1
			|| strcmp(dlg->m_pItemInfo->UccIndex, (const char*)msg.param2) != 0)
			return true;
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(MakeStr(K2L_Compatibility("\xbe\xf7\xb7\xce\xb5\xe5\xb0\xa1\x20\xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9\x2e\x20\xb4\xd9\xbd\xc3\x20\xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4\x2e\x28\xc4\xda\xb5\xe5\x3a\x25\x64\x29"), msg.param3), false);
		pForm->Open(NULL, 1);
		dlg->CloseWaitingForm();
		return true;
	}
	case 0x28f: // 00624f10: pacote 0x12e tipo 0 (registro da roupa UCC)
	{
		bool bSuccess = (unsigned char)msg.param1 != 0;
		unsigned long id = *(unsigned long*)msg.param2;
		unsigned long typeId = *(unsigned long*)msg.param3;
		const char* uccIndex = (const char*)msg.param4;
		const char* uccName = (const char*)msg.param5;
		FrUccDrawDlg* dlg = m_pUccDrawDlg983;
		if (dlg == NULL || dlg->m_pItemInfo->tid != typeId || strcmp(dlg->m_pItemInfo->UccIndex, uccIndex) != 0)
			return true;
		dlg->m_bWaiting = false;
		if (!bSuccess)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
			pForm->SetMessage(K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce\x20\xc0\xc7\xbb\xf3\x20\xb5\xee\xb7\xcf\xbf\xa1\x20\xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9\x2e"), false);
			pForm->Open(NULL, 0);
			return true;
		}
		for (int i = 0; i < 2; i++)
		{
			char* pClothes = (char*)dlg->m_pClothes[i];
			Bitmap* pBitmap = *(Bitmap**)(pClothes + 8);
			const char* texName = pClothes + 0x8c;
			w_texlist* pTex = g_resrcmng->FindTexture(texName);
			if (pTex == NULL)
				g_resrcmng->UploadTexture(texName, pBitmap, 0, NULL);
			else
				g_resrcmng->FixTexture(pTex->texhandle, pBitmap, 0, NULL);
		}
		sUccClothes clothes;
		GetSystemTime(&clothes.date);
		clothes.id = id;
		clothes.downloadCount = 1;
		clothes.typeId = typeId;
		strncpy(clothes.uccIndex, uccIndex, 9);
		strncpy(clothes.uccName, uccName, 0x29);
		std::map<std::string, Bitmap*>::iterator itIcon = UccManager()->m_icon.find(GenerateIconFilename(typeId, uccIndex));
		if (itIcon != UccManager()->m_icon.end())
			clothes.icon = itIcon->second;
		UccManager()->AddClothes(1, clothes, true, false);
		std::list<sItemInfo>& itemList = Doc()->m_uccItemList;
		for (std::list<sItemInfo>::iterator it = itemList.begin(); it != itemList.end(); ++it)
		{
			if (it->tid == typeId && strcmp(it->UccIndex, uccIndex) == 0)
			{
				it->status = (it->status & 0xfd) | 1;
				break;
			}
		}
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce\x20\xc0\xc7\xbb\xf3\xc0\xcc\x20\xbf\xcf\xbc\xba\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9\x2e"), false);
		pForm->Open(NULL, 0);
		dlg->Close((eFormRet)1, true);
		return true;
	}
	case 0x290: // 00625339: pacote 0x12e, salvar temporario
	{
		bool bSave = (unsigned char)msg.param1 == 1;
		unsigned long typeId = *(unsigned long*)msg.param2;
		const char* uccIndex = (const char*)msg.param3;
		FrUccDrawDlg* dlg = m_pUccDrawDlg983;
		if (dlg == NULL || dlg->m_pItemInfo->tid != typeId || strcmp(dlg->m_pItemInfo->UccIndex, uccIndex) != 0)
			return true;
		dlg->m_bWaiting = false;
		if (bSave)
		{
			Bitmap* pBack = dlg->m_pClothes[1]->GetTexture32BPP(true);
			Bitmap* pFront = dlg->m_pClothes[0]->GetTexture32BPP(true);
			UccManager()->SetUccTempClothes(typeId, uccIndex, pFront, pBack);
			dlg->SetSaved(true); // 007e21d0
			return true;
		}
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce\x20\xc0\xc7\xbb\xf3\xc0\xc7\x20\xc0\xd3\xbd\xc3\x20\xc0\xfa\xc0\xe5\xbf\xa1\x20\xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9\x2e"), false);
		pForm->Open(NULL, 1);
		return true;
	}
	case 0x291: // 00625459: FrSelfDesignExhibit pediu a venda
		m_bExhibitWait983 = true;
		return true;
	case 0x292: // 00625469: resposta da loja Self Design
		m_bExhibitWait983 = false;
		return true;
	case 0x279: // 00624cf0: premios da bolsa (pacote 0x129 ok, realmyroomtask): janela com a lista
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		if (!m_pFortuneOpenDlg983)
			m_pFortuneOpenDlg983 = CreateForm<FrFortuneOpenDlg>(g_pFresh->GetManager(), this, "fortune_open_new", NULL);
		m_pFortuneOpenDlg983->SetItemNewList();
		m_pFortuneOpenDlg983->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnFortuneOpenResult983, 3);
		return true;
	case 0xf8: // 983 (M983(0xd3), fim do enchant): refaz as barras dos dialogos up/down (+0x1f8/+0x1fc) e os stats
		if (FrDowngradeDlg* dlg = m_pUpDownDlg983)
			dlg->SetLevelBar();
		if (FrDowngradeDlg* dlg = m_pUpDownDlg2_983)
			dlg->SetLevelBar();
		UpdateStats983((char*)this);
		return true;
	case 0xf9: // 983 (00624142, M983(0xd4) da seta do "downgrade"): abre o "enchant" (+0x1f4) com tipo/stat (00aa6d80), resultado 00605570
	{
		if (m_pEnchantDlg983)
			return false;
		jp983_rastro("A821 enchant tipo=%d stat=%d", msg.param1, msg.param2);
		FrEnchantDlg* dlg = CreateForm<FrEnchantDlg>(g_pFresh->GetManager(), this, "enchant", NULL);
		m_pEnchantDlg983 = dlg;
		dlg->SetType((unsigned char)msg.param1, (unsigned char)msg.param2);
		dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnEnchantResult983, 1);
		return true;
	}
	// 0xaa (taskmanager) com o My Room aberto: quem poe o item no doc e o form, o ramo geral do pacote nao roda
	case 0xed: // 00623a70: personagem
	{
		sCharacterInfo info;
		memset(&info, 0, sizeof(info));
		info.guid = msg.param1;
		info.tid = msg.param2;
		info.gift_flag = 1;
		ItemManager()->GetDefCombo(info.tid, info.tidParts);
		Doc()->m_charMap.insert(std::map<unsigned int, sCharacterInfo>::value_type(info.guid, info));
		return true;
	}
	case 0xee: // cor do cabelo no personagem da tid do estilo; com a sala 3D fechada (00e78cac) vai ao LobbyAvatar
	{
		IFF_STRUCT::sHairStyle* pHair = ItemManager()->FindHairStyle(msg.param2);
		if (pHair == NULL)
			return true;
		for (std::map<unsigned int, sCharacterInfo>::iterator it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			if (it->second.tid == (pHair->cCharID | 0x4000000u))
			{
				it->second.hairClr = pHair->cHairID;
				break;
			}
		}
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			return true;
		CPartTidList& tids = Doc()->GetMyPartTidList();
		if (g_lobbyModal983 != 0)
			MyRoomPetFrameSetHairClr983(m_pPreview983->GetPetFrame(), &tids, pHair->cHairID);
		else
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x18c, (int)&tids, (int)&pHair->cHairID, 0, 0, 0);
		return true;
	}
	case 0xef: // roupa
	{
		sItemInfo item;
		memset(&item, 0, sizeof(item));
		item.guid = msg.param1;
		item.tid = msg.param2;
		item.HourRemain = IsHourPart983(item.tid) ? msg.param3 : msg.param3 * 0x18;
		item.gift_flag = 1;
		item.ItemType = msg.param4;
		if (IsLocalContent(S4_UCC) && IsUccClothes(item.tid))
		{
			strncpy(item.UccIndex, ((sBuyItemResult*)msg.param5)->UccIndex, 9);
			item.Seq = 1;
			Doc()->m_uccItemList.push_back(item);
		}
		else
			Doc()->AddMyPartsList(item);
		return true;
	}
	case 0xf0: // caddie; +0xc/+0xd so nos tid 0x1c00000c..e (argumentos do 0xaa: Time = Exp, ItemType = Level)
	{
		sCaddieInfo caddie;
		memset(&caddie, 0, sizeof(caddie));
		caddie.guid = msg.param1;
		caddie.tid = msg.param2;
		caddie.gift_flag = 1;
		caddie.Rent_flag = 1;
		caddie.Remain_Date = 0x1e;
		if (caddie.tid > 0x1c00000b && caddie.tid < 0x1c00000f)
		{
			caddie.Level = (unsigned char)msg.param4;
			caddie.Exp = msg.param3;
		}
		Doc()->m_caddieMap.insert(std::map<unsigned int, sCaddieInfo>::value_type(caddie.guid, caddie));
		return true;
	}
	case 0xf1: // parte do caddie
	{
		std::map<unsigned int, sCaddieInfo>::iterator it = Doc()->m_caddieMap.find(msg.param1);
		if (it != Doc()->m_caddieMap.end())
		{
			it->second.tidPart = msg.param2;
			it->second.Remain_Partdate = (unsigned short)msg.param3;
			if (msg.param3 > it->second.Remain_Date)
				it->second.Remain_Date = (unsigned short)msg.param3;
		}
		return true;
	}
	case 0xf2: // mascote
	{
		sBuyItemResult* pResult = (sBuyItemResult*)msg.param5;
		sMascotInfo mascot;
		memset(&mascot, 0, sizeof(mascot));
		mascot.guid = msg.param1;
		mascot.tid = msg.param2;
		strcpy(mascot.szMsg, "PANGYA!");
		mascot.Remain_Date = pResult->Time;
		memcpy(&mascot.endDate, &pResult->endDate, sizeof(_SYSTEMTIME));
		Doc()->AddMascot(&mascot);
		return true;
	}
	case 0xf3: // 00623d10: pang, bola, item de consumo, aux part
	{
		unsigned long tid = msg.param2;
		if (tid == 0x1a000010)
		{
			Doc()->m_myInfo.stat.i64Pang += msg.param3;
			return true;
		}
		sBuyItemResult* pResult = (sBuyItemResult*)msg.param5;
		std::list<sItemInfo>* pList;
		int period = 0;
		unsigned char type = 0;
		switch (tid >> 26)
		{
		case 5:
		case 6:
			pList = (tid >> 26) == 5 ? &Doc()->m_ballList : &Doc()->m_myItemList;
			period = msg.param3;
			type = (unsigned char)msg.param4;
			break;
		case 0x1c:
			if (ItemManager()->FindAuxPart(tid) == NULL)
				return true;
			pList = &Doc()->m_auxPartList;
			break;
		default:
			return true;
		}
		if (tid >= 0x1a000080 && tid <= 0x1a000083)
			return true;
		int hours = type == 3 ? period : type == 4 ? period * 0x18 : type == 5 ? period * 0x2d0 : 0;
		std::list<sItemInfo>::iterator it = pList->end();
		if (msg.param4 == 0)
			for (it = pList->begin(); it != pList->end() && it->guid != (unsigned long)msg.param1; ++it)
				;
		if (it != pList->end())
		{
			if (type != 0)
				SendStartPeriodItem983(it->guid);
			it->HourRemain += hours;
			it->Common[0] = pResult->Count;
			if (it->ItemType != 0 && it->HourRemain > 0)
				SendStartPeriodItem983(it->guid);
		}
		else
		{
			sItemInfo item;
			memset(&item, 0, sizeof(item));
			item.guid = msg.param1;
			item.tid = tid;
			item.Common[0] = pResult->Count < 1 ? 1 : pResult->Count;
			item.gift_flag = 1;
			item.ItemType = type;
			if (type != 0)
			{
				item.HourRemain = hours;
				SendStartPeriodItem983(item.guid);
				item.ItemType = 2;
			}
			pList->push_back(item);
		}
		FillItemList983(m_subTab983);
		return true;
	}
	case 0xf4: // taco
	{
		sItemInfo item;
		memset(&item, 0, sizeof(item));
		item.guid = msg.param1;
		item.tid = msg.param2;
		Doc()->m_clubSetMap.insert(std::map<unsigned int, sItemInfo>::value_type(item.guid, item));
		return true;
	}
	case 0xf5: // cutin: grupo < 5 acumula pelo guid (tempo = param3, ItemType 4); o resto entra em dias sem tipo
	{
		sItemInfo item;
		memset(&item, 0, sizeof(item));
		item.guid = msg.param1;
		item.tid = msg.param2;
		item.HourRemain = msg.param3;
		if ((item.tid & 0x3c00000) < 0x1400000)
		{
			for (std::list<sItemInfo>::iterator it = Doc()->m_cutinList.begin(); it != Doc()->m_cutinList.end(); ++it)
			{
				if (it->guid == item.guid)
				{
					it->tid = item.tid;
					it->ItemType = 4;
					it->HourRemain += msg.param3;
					return true;
				}
			}
			item.ItemType = 4;
		}
		else
			item.HourRemain *= 0x18;
		Doc()->m_cutinList.push_back(item);
		return true;
	}
	case 0x105: // carta: acha por tid e a contagem substitui (0x3a = sCards)
	{
		unsigned long tid = msg.param2;
		if (ItemManager()->FindCard(tid) == NULL)
			return true;
		unsigned short count = ((sBuyItemResult*)msg.param5)->Count;
		for (std::list<sCards>::iterator it = Doc()->m_cardList.begin(); it != Doc()->m_cardList.end(); ++it)
		{
			if (it->tid == tid)
			{
				it->cardCount = count;
				return true;
			}
		}
		sCards card;
		memset(&card, 0, sizeof(card));
		card.uid = msg.param1;
		card.tid = tid;
		card.cardCount = count;
		Doc()->m_cardList.push_back(card);
		return true;
	}
	case 0xf7: // 983: se [form+0x1f4] != 0, grava param1 (byte) em [+0x1f4]+0x149; cai em 0x109
	{
		if (m_pEnchantDlg983)
#if _MSC_VER > 1600 // A908: m_resultCode +0x139 no cl novo; +0x149 passava do fim do objeto (0x140)
			m_pEnchantDlg983->SetResultCode((unsigned char)msg.param1);
#else // no VC10 o acessor sairia call
			*((char*)m_pEnchantDlg983 + 0x149) = (char)msg.param1;
#endif
	}
	// fallthrough
	case 0x109: // 983: refaz os stats (0061a750)
		UpdateStats983((char*)this);
		return true;
	case 0x107: // 983 (00623500): so nas sub-abas 0x1d..0x21 refaz a lista; nas outras vai ao fim (caseD_e3)
		if ((unsigned)(m_subTab983 - 0x1d) >= 5)
			return false;
		// fallthrough
	case 0x102: // 983: M983(0xdd) depois do 0x19D (caixa aberta) -> 0061af80 refaz a lista da sub-aba
	case 0x106: // 983: M983(0xe1) -> 0061af80
		jp983_rastro("A710 OnMsg983 %x refaz lista sub=%x", msg.message, m_subTab983);
		FillItemList983(m_subTab983);
		return true;
	case 0x5b: // 00624235: mascote (param2 -> {key, tid, texto +0xd, pcbang +0x3d}); 0061cb70 + msg na textura + stats + lista + 0063de40
	{
		char* self = (char*)this;
		unsigned long* pInfo = (unsigned long*)msg.param2;
		SelectMascot983(pInfo[0]);
		Doc()->m_myInfo.userEquip.guidMascot = pInfo[0];
		char* pv = (char*)m_pPreview983;
		if (pv && ((CExhibition*)pv)->GetModel() && pInfo[1])
		{
			if (IFF_STRUCT::sMascot* pMascot = Doc()->m_itemManager.FindMascot(pInfo[1]))
				CMascot::ChangeMascotMsgInTex((const char*)((char*)pInfo + 0xd), pMascot, ((CExhibition*)pv)->GetModel(),
					MakeStr("myroom%02d", *(unsigned long*)((char*)pMascot + 4) & 0x3ffffff));
		}
		if (*((char*)pInfo + 0x3d) != 0)
			COption::Instance()->mSetPCBangMascotMsg((const char*)((char*)pInfo + 0xd));
		UpdateStats983(self);
		FillItemList983(m_subTab983);
		SelectMascotTail983(Doc());
		return true;
	}
	}
	return false;
}

// 0061c960: seleciona o caddie `key` (guid) em m_caddieMap: +0x504 = key, +0x500 = tid, +0x508 = tidPart (ou tid se 0);
// grava guidCaddie do doc (+0xa26) e, na aba 0xb, reenche a grade. Nao achou: zera. B384
static void ResetStatsPanel983(char* self);

// 0061aa80: mostra +0x12c, +0x528 = 0 e refaz os stats
static void ResetStatsPanel983(char* self)
{
	if (FrWnd* w = ((CRealMyRoomForm983*)self)->m_pSelHide983)
		w->SetVisible(true);
	((CRealMyRoomForm983*)self)->m_previewOther983 = 0;
	UpdateStats983(self);
}

bool CRealMyRoomForm983::SelectCaddie983(unsigned long key)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	std::map<unsigned int, sCaddieInfo>::iterator ci = doc->m_caddieMap.find(key);
	bool changed = false;
	if (ci == doc->m_caddieMap.end())
	{
		jp983_rastro("D389 SelCaddie key=%u NAO esta em m_caddieMap (n=%d) tab=%d", (unsigned)key, (int)doc->m_caddieMap.size(), m_subTab983);
		m_selCaddieTid983 = 0;
		m_selCaddieKey983 = 0;
		doc->m_myInfo.userEquip.guidCaddie = 0;
		if (m_subTab983 == 0xb)
		{
			MyRoomPetClear983(self); // 00605530(1)
			if (FrListBox* lb = m_pItemList983) lb->UnselectAllItem(); // 00befe70 (B1472: era cru +0xd0/+0x108, anda no cl 14.44)
			ResetStatsPanel983(self); // 0061aa80
			if (IActor* lc = AfxGetTask()->GetActor("LobbyCaddie"))
				lc << MsgObject(NULL, 0x1a6, 0, 0, 0, 0, 0);
		}
		if (key == 0)
			doc->m_realEquip.guidCaddie = 0;
		return false;
	}
	const sCaddieInfo& c = ci->second;
	m_selCaddieKey983 = ci->first;
	// 983 0061c960: no do map (VS2010) chave +0xc, valor +0x10: guid +0x10, tid +0x14, tidPart +0x18
	m_selCaddieTid983 = c.tid;
	m_selCaddieShowTid983 = c.tidPart ? c.tidPart : c.tid;
	// 0061c960: so o caddie com nivel do item <= nivel do jogador (doc +0x91d) vira o equipado (00a172a0 = FindCommonItem)
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(c.tid); // 983 0061c960: 00a172a0(+0x14 = tid) (D331.1)
	if (common && (common->Level & 0x7f) <= doc->m_myInfo.stat.Level)
	{
		if (ci->first != doc->m_myInfo.userEquip.guidCaddie)
			changed = true;
		doc->m_myInfo.userEquip.guidCaddie = ci->first;
		doc->m_realEquip.guidCaddie = ci->first;
	}
	jp983_rastro("D316 SelCaddie key=%u tid=%08x common=%p lv=%d doclv=%d equip=%u changed=%d st0=%d", key, c.tid, common, common ? (common->Level & 0x7f) : -1, (int)doc->m_myInfo.stat.Level, doc->m_myInfo.userEquip.guidCaddie, (int)changed, CStatusHelper().GetStatus983(0));
	{ int n=0; char b[512]=""; for (std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator it = ItemManager()->m_CaddieMap.begin(); it != ItemManager()->m_CaddieMap.end() && n < 12; ++it, ++n) sprintf(b + strlen(b), "%08x ", it->first); jp983_rastro("D326 CaddieKeys %s tid=%08x", b, c.tid); }
	jp983_rastro("D320 IFF caddies=%d cadItems=%d chars=%d items=%d findCad=%p", ItemManager()->GetNumItems(PIG_CADDIE), ItemManager()->GetNumItems(PIG_CADITEM), ItemManager()->GetNumItems(PIG_CHAR), ItemManager()->GetNumItems(PIG_ITEM), ItemManager()->FindCaddie(c.tid));
	if (m_subTab983 == 0xb)
	{
		FillItemList983(0xb);
		if (g_lobbyModal983)
			MyRoomPreview983(self, m_selCaddieShowTid983, false); // 00605640: previa (ShowPet)
		else if (changed)
		{
			// 00643f40 so procura (sem efeito); TOPPAGE: o caddie 3D do lobby recarrega pela msg 0x1a9
			if (IActor* lc = AfxGetTask()->GetActor("LobbyCaddie"))
				lc << MsgObject(NULL, 0x1a9, 0, 0, 0, 0, 0);
		}
		ResetStatsPanel983(self); // 0061aa80
	}
	return changed;
}

// cauda do 0061cb70: com NT REAL_MYROOM (00a7a8d0) 0063de40 (0x0020 sub 2 com o estado do doc), senao 0063dd00 (SendEquipItemSlotByMascot)
static void SelectMascotTail983(CSharedDoc* doc)
{
	if (IsLocalContent(S4_REAL_MYROOM))
	{
		WSendPacket w((enumClientPacket)0x20); w.Encode1(2); w.EncodeBuffer((char*)doc->m_myInfo.userEquip.tidItemSlot, 0x28); w.Send(TO_GAME);
	}
	else
		doc->SendEquipItemSlotByMascot();
}

// 0061cb70: seleciona o mascote `key` em m_mascotMap: +0x510 = key, +0x50c = guid; grava guidMascot do doc (+0xa8e)
// e +0x110a4 (key; 0 = nao achou/sem selecao); na aba 0xd reenche a grade. Cauda 63dd00/63de40 portada (B476); 00befe70 portado (B477). B385
void CRealMyRoomForm983::SelectMascot983(unsigned long key)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	std::map<unsigned int, sMascotInfo>::iterator mi = doc->m_mascotMap.find(key);
	if (mi == doc->m_mascotMap.end())
	{
		m_selMascotGuid983 = 0;
		m_selMascotKey983 = 0;
		doc->m_myInfo.userEquip.guidMascot = 0;
		if (m_subTab983 == 0xd)
		{
			MyRoomPetClear983(self); // 0061cb70: aba 0xd sem mascote -> 00605530(1)
			if (FrListBox* lb = m_pItemList983) lb->UnselectAllItem(); // 00befe70 (B1472: era cru +0xd0/+0x108, anda no cl 14.44)
		}
		if (key == 0)
			doc->m_realEquip.guidMascot = 0;
		SelectMascotTail983(doc);
		return;
	}
	m_selMascotGuid983 = mi->second.guid;
	m_selMascotKey983 = mi->first;
	doc->m_myInfo.userEquip.guidMascot = mi->first;
	doc->m_realEquip.guidMascot = mi->first;
	if (m_subTab983 == 0xd)
		FillItemList983(0xd);
	if (m_subTab983 == 0xd)
	{
		MyRoomPreview983(self, mi->second.tid ? mi->second.tid : mi->second.guid, false); // 00605640: previa (ShowPet)
		// 0061aab0: esconde +0x12c, +0x528 = 1 e refaz os stats
		if (FrWnd* w = m_pSelHide983)
			w->SetVisible(false);
		m_previewOther983 = 1;
		UpdateStats983(self);
	}
	else
		MyRoomPreview983(self, mi->second.tid ? mi->second.tid : mi->second.guid, false); // 00605640: previa (ShowPet)
	SelectMascotTail983(doc);
}

// 006165d0 (parcial, B431/C174.1): ao fechar, o original manda 0x0020 sub 9 (60e810) e sub 2 (63de40) com o estado do doc
// (orig-c154-2: os caches do form nunca sao gravados). Os demais helpers nao portados.
bool CRealMyRoomForm983::OnCloseWidget983()
{
	CSharedDoc* sd = Doc();
	MyRoomPetClear983((char*)this); // 00605530
	SendEquipChanges983(sd); // C231.1: ordem do original (orig-c229-1)
	const sUserEquip& eq = sd->m_myInfo.userEquip;
	if (memcmp(eq.poster, s_syncPoster983, sizeof(s_syncPoster983)) != 0) // 0060e6c0
	{
		WSendPacket w((enumClientPacket)0x20); w.Encode1(10); w.EncodeBuffer((char*)eq.poster, 8); w.Send(TO_GAME);
		if (IActor* bg = AfxGetTask()->GetActor("LobbyBg"))
			bg << MsgObject(NULL, 0x182, 0, 0, 0, 0, 0);
	}
	SnapshotEquip983(sd);
	{ WSendPacket w((enumClientPacket)0x20); w.Encode1(2); w.EncodeBuffer((char*)eq.tidItemSlot, 0x28); w.Send(TO_GAME); } // 0063de40
	if (FrClubSetResetWnd::s_pInstance) // 983 0061666b: apaga a janela do reset do taco
	{
		delete FrClubSetResetWnd::s_pInstance;
		FrClubSetResetWnd::s_pInstance = NULL;
	}
	return FrTaskForm::OnCloseWidget983();
}


bool CRealMyRoomForm983::OnInit()
{
	char* self = (char*)this;
	for (size_t i = 0; i < sizeof(s_myRoomBinds983) / sizeof(s_myRoomBinds983[0]); ++i)
	{
		const MyRoomBind983& e = s_myRoomBinds983[i];
		FrWnd* w = FindByPath983(this, e.path);
		if (!w)
			continue;
		if (e.hide)
			w->SetVisible(false);
		else
			*(FrWnd**)(self + e.off) = w;
	}
	BindStorageUI983();
	if (FrListBox* il = m_pItemList983)
	{
		// 00606e90: item_list liga o botao direito (+0xfc = 1; sem isso o FrListBox ignora o RBUTTONDOWN), +0x114 = 1 e sem som de clique
		il->UseRightButton(true);
		static_cast<FrListBoxDummy983*>(il)->SetUseDummy983(); // A769: ex-cru +0x114 (VC10 m_useDummy; cl 14.44 anda)
		il->SetPushSound(NULL);
	}
	if (FrListBox* sl = DYNAMIC_CAST(FrListBox, (FrWnd*)m_pStorageList983))
	{
		// 0060a620: item_storage_list liga o duplo clique (+0x44 &= ~0x200) e o botao direito (+0xfc = 1); sem isso OnStorageListRDown983 nunca roda (D610)
		sl->UseDblClick(true);
		sl->UseRightButton(true);
	}
	{
		// 00607b60/00607c10/00608240/006082c0/00608340: equip_cancel/equip_reset/upgrade/downgrade/clubwork_m chamam EnableHover(1, 0) (D637)
		FrWnd* const hoverBtn[5] = { m_pEquipCancel983, m_pEquipReset983, m_pUpgrade983, m_pDowngrade983, m_pClubWork983 }; // C732: membros (+0x19c..0x1a8, +0x1b0): cru = VC10
		for (int i = 0; i < 5; ++i)
			if (FrWnd* w = hoverBtn[i])
				w->EnableHover(true, 0);
	}
	{
		// 0060a4a0/0060a560/0060a940/0060aa00/0060aac0: pang_minus/plus, item_storage_page_next/prev e close gravam +0x104 = 0.1f, +0x108 = 0.2f (D635)
		FrWnd* const storageBtn[5] = { m_pPangMinus983, m_pPangPlus983, m_pStoragePageNext983, m_pStoragePagePrev983, m_pStorageClose983 }; // C732: +0xa98/a9c/aa8/aac/ab0
		for (int i = 0; i < 5; ++i)
			if (FrButton* b = DYNAMIC_CAST(FrButton, storageBtn[i]))
				b->SetPushDelay(0.1f);
	}
	{
		// 00609850..00609e20: CCardBook em +0xa78 e os controles do Card Holic (INIT de card_book/tabs/card_use/cardpage_index/card_property)
		CCardBook* book = new CCardBook;
		CardBook983() = book;
		Bind983(book->m_pBackground, DYNAMIC_CAST(FrArea, FindByPath983(this, "card_book")));
		Bind983(book->m_pProperty, DYNAMIC_CAST(FrArea, FindByPath983(this, "card_property")));
		Bind983(book->m_pIndexList, DYNAMIC_CAST(FrListBox, FindByPath983(this, "cardpage_index")));
		Bind983(book->m_pUseButton, DYNAMIC_CAST(FrButton, FindByPath983(this, "card_use")));
		static const char* const tabNames[7] = { "card_pack", "card_volume1", "card_volume2", "card_volume3", "card_volume4", "card_volume5", "card_sp" };
		for (int i = 0; i < 7; ++i)
		{
			FrButton* tb = DYNAMIC_CAST(FrButton, FindByPath983(this, tabNames[i]));
			Bind983(book->m_pTabButton[i], tb);
			if (tb)
				tb->SetPushDelay(0.0f);
		}
		// card_slot1..12 -> CCardPage::BoundSlotControl (645 CRealMyRoomMain::BoundCardPage); cardpack_slot* ficam escondidos
		CCardPage* pPage = book->GetCardPage();
		for (int i = 0; i < 12 && pPage; ++i)
		{
			char nm[32];
			sprintf(nm, "card_slot%d", i + 1);
			FrButton* sb = DYNAMIC_CAST(FrButton, FindByPath983(this, nm));
			if (sb)
			{
				pPage->BoundSlotControl((unsigned char)i, sb);
				sb->UseRightButton(true);
			}
		}
	}
	{
		// 00615dd0 (bind item_storage_bgi): com doc+0x110c8 == 0 e +0xa84 == 0 o original manda 0x00D3 (vazio) e, via 0060e4b0,
		// 0x0020 sub 5 + guid do personagem (doc+0xa2a) quando +0x724 difere; +0x724 nunca e gravado aqui. A cond. 00a7a8d0
		// (bitset) e os helpers 00613590/614d20/60e550/60e600/613930/614de0/63dd00/63de40/60e810 (so enviam em diferencas) nao portados. B386
		CSharedDoc* sd = Doc();
		if (!sd->m_bItemStoragePin && !m_storageOpen983)
		{
			WSendPacket p((enumClientPacket)0xd3);
			p.Send(TO_GAME);
			if (m_storageGuidChar983 != (int)sd->m_myInfo.userEquip.guidChar)
			{
				WSendPacket q((enumClientPacket)0x20);
				q.Encode1(5);
				q.Encode4(sd->m_myInfo.userEquip.guidChar);
				jp983_rastro("A803 myroom envia 0x20/5 guidChar=%08x (abrir)", sd->m_myInfo.userEquip.guidChar);
				q.Send(TO_GAME);
			}
			SendDocSyncPackets983(sd);
		}
		else
			SnapshotEquip983(sd);
	}
	SelectCharacter983();
	{
		// 0061e460 (kind 1/2): reaplica a ultima selecao guardada no doc (+0x1103c caddie, +0x110a4 mascote); so se != 0
		// (o original zera guidCaddie/guidMascot do doc se nao achar; aqui nao mexemos no equip sem selecao previa). B385
		CSharedDoc* d = Doc();
		unsigned long kc = d->m_realEquip.guidCaddie;
		unsigned long km = d->m_realEquip.guidMascot;
		if (kc)
			SelectCaddie983(kc);
		if (km)
			SelectMascot983(km);
	}
	AddMainTabs983();
	m_mainTab983 = 0;
	m_subTab983 = 0x16;
	SetMainTab983(0, true);
	// B400: b393-1 (g_lobbyModal983 == 0 no lobby: TOPPAGE zera 00e78cac) ainda mostrava o painel azul (C1): teste = esconder
	// main_bg1..4 (+0x1dc..+0x1e8) sempre; chat_bg (+0x1ec) so no modal (00620db3, g == 1).
	for (int i = 0; i < 4; ++i)
	{
		FrWnd* bg = (&m_pMainBg1_983)[i];
		if (bg) bg->SetVisible(g_lobbyModal983 != 1); // B422: 983 00620db3 (cmp [e78cac],1; jne): com ==1 ESCONDE main_bg1..4 e mostra chat_bg; B417 estava invertido
	}
	// B403: b400-1 ainda mostra o painel azul com -/+ (nao e main_bg). Teste: esconde tambem item_hide_desc_bg/desc/name
	// (+0x178..0x180). A264: pet/skin_pet/skin_pet2 (+0x184..0x18c) ficam como no layout: o 983 nunca os esconde e a previa
	// e desenhada no OWNERDRAW deles (0061f6d0); o painel azul era o item_storage_* (B411).
	// C732: membros, nao self+0x178..0x180: no cl 14.44 FrTaskForm anda -0x10 e o cru escondia skin_pet2/pet/item_slot (previa e barra de slots sumiam so no VS)
	FrWnd* const itemHide[3] = { m_pItemHideDescBg983, m_pItemHideDesc983, m_pItemHideName983 };
	for (int i = 0; i < 3; ++i)
	{
		FrWnd* w = itemHide[i];
		if (w) w->SetVisible(false);
	}
	{
		// B412: b412-1 ainda mostra o painel azul; chat_bg (+0x1ec, 800x600) vem visivel do layout. 983 (00620db3) so o mostra com g == 1.
		FrWnd* chat = m_pChatBg983;
		if (chat) chat->SetVisible(g_lobbyModal983 == 1);
	}
	// B411: rastro b411-1: card_slot1..12/cardpack_slot1..8 (quadrados cinza) e item_storage_* (painel azul com -/+) vinham visiveis
	// no lobby; o original so os mostra nas abas My Box/Storage.
	{
		static const char* const hideNames[] = {
			"card_book_back_bg", "cardpacklist", "card_count", "cardpage_index", "click_area_list",
			"item_storage_bgi", "item_storage_pang", "pang_minus", "pang_plus", "item_storage_list", "item_storage_index",
			"item_storage_page_next", "item_storage_page_prev", "item_storage_close", "item_storage_dragdrop" };
		for (int i = 0; i < (int)(sizeof(hideNames) / sizeof(hideNames[0])); ++i)
		{
			FrWnd* w = FindChildByName(hideNames[i]);
			if (w) w->SetVisible(false);
		}
		char nm[32];
		for (int i = 1; i <= 12; ++i)
		{
			sprintf(nm, "card_slot%d", i);
			FrWnd* w = FindChildByName(nm);
			if (w) w->SetVisible(false);
			if (i <= 8)
			{
				sprintf(nm, "cardpack_slot%d", i);
				w = FindChildByName(nm);
				if (w) w->SetVisible(false);
			}
		}
	}
	{
		Dump983 d = { 0 };
		EnumerateChildWindow(DumpChild983, &d);
	}
	FrForm::ms_bModalLock983 = true; // 983 00620cd2: call 00be7950 -> o Open nao liga o escurecimento modal (lobby nao escurece)
	LoungeMyRoomState983(true); // 00620cd7
	return FrTaskForm::OnInit();
}

static void LoungeMyRoomState983(bool open)
{
	// 983 0061d5ed: ao fechar, 0x58 ao ator "AvatarChat" religa o ntAvatarManager (+0xb0, desligado em MakeRealMyRoom; RFLobbyDesktop::HandleMsg983)
	if (!open && CTaskManager::Instance())
		CTaskManager::Instance()->PostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);
	if (g_lobbyModal983 != 1)
		return;
	if (!open && RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->SetLayer983(0); // 0061d610
	ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance ? ntAvatarManager::m_pInstance->m_pMyAvatar : NULL;
	if (pMyAvatar)
		pMyAvatar->SetState(open ? (pMyAvatar->m_state983 | 2) : (pMyAvatar->m_state983 & ~2));
}

IMPLEMENT_OBJECT(CRealMyRoomForm983, FrTaskForm)

// 0061f530: clique na aba principal: troca a aba (0061e4e0) e seleciona a 1a sub-aba do caso
// (0061d7a0: 0x16,7,0xb,0xd,0xe,0x13). O CAnotherHand embutido (+0x588; aqui previa sincrona) e os helpers
// 0061eaa0/0061c960/0061cb70/0061aab0/00613590/00605530 ainda nao portados. B380
void CRealMyRoomForm983::OnTabMainUp983()
{
	FrTabButton* main = m_pTabMain983;
	if (!main || !main->GetSelected())
		return;
	int tab = (int)main->GetSelected()->index;
	SetMainTab983(tab, false);
	static const int firstSub[6] = { 0x16, 7, 0xb, 0xd, 0xe, 0x13 };
	if (tab >= 0 && tab < 6)
	{
		SelectSubTab983(firstSub[tab], false);
		// 0061f530: abas Caddie/Mascot reaplicam a selecao do doc (0061c960/0061cb70 -> 0x0020 sub 2; C240)
		if (tab == 2)
			SelectCaddie983(Doc()->m_myInfo.userEquip.guidCaddie);
		else if (tab == 3)
			SelectMascot983(Doc()->m_myInfo.userEquip.guidMascot);
	}
}

// 0061f6b0: clique na sub-aba: SelectSubTab(index da aba selecionada, 0). B382
void CRealMyRoomForm983::OnTabSubUp983()
{
	FrTabButton* sub = m_pTabSub983;
	if (!sub || !sub->GetSelected())
		return;
	SelectSubTab983((int)sub->GetSelected()->index, false);
}

void __fastcall ReviseItemIconPoint(unsigned long tid, WPoint& pt);
namespace S5 { bool __fastcall ISCutinItem(unsigned long tid); }
// 006166b0 casos de aba 0xb (caddie) e 0xd (mascote), parte basica (D): icone centrado, nome e, no mascote, "Time:" + dias/horas
// restantes (006333c0: fim - hora do servidor em horas; sem prazo -> "No Limit"; <1h em vermelho).
// 00614920: botao "Stats" do cartao (form +0x948 state_view_n / +0x94c state_view_o, em x+80,y+64). Mouse em cima: _o e a dica da
// lista (+0x144) com os 5 atributos (b30340 = MakeStatusString de cada um, "%s+%d/%s+%d" com o slot); o 983 guarda o item da dica
// em +0xad0 e a apaga quando o mouse sai. Dados (006166b0): taco 4 = Attr/Slot do sClubSet (00606580), caddie 7 = Attr do sCaddie,
// mascote 0x10 = Attr (char) do sMascot. Antes so desenhava o _n, sem dica (dono 28/10 04:15, PEDIDOS).
const char* __fastcall MakeStatusString(int type, int level, int slot); // 983 b2e010 (tradedlg.cpp)
static int s_statViewIdx983 = -1; // 983 form +0xad0
static void DrawStatView983(FrListBox* pList, FrGraphicInterface* pGI, FrListItem* pItem, const short* stats)
{
	const Bitmap* pN = g_pFresh->GetBitmap("state_view_n");
	if (pN == NULL)
		return;
	WRect rc(pItem->pos.x + 80.0f, pItem->pos.y + 64.0f, (float)pN->Width(), (float)pN->Height());
	if (pItem->underCursor && rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		const Bitmap* pO = g_pFresh->GetBitmap("state_view_o");
		pGI->DrawTexture(pO ? pO : pN, rc, 0xffffffff, 0);
		std::string text;
		for (int i = 0; i < 5; ++i)
		{
			const char* line = MakeStatusString(i, stats[i], stats[i + 5]);
			if (*line)
			{
				text += line;
				text += '\n';
			}
		}
		text.erase(text.find_last_not_of('\n') + 1);
		text.erase(0, text.find_first_not_of('\n'));
		if (!text.empty() && pList != NULL)
		{
			pList->m_nFlags.Turn(FWF_TOOLTIPS, true);
			pList->SetToolTipText(text);
			s_statViewIdx983 = pItem->idx;
		}
		return;
	}
	pGI->DrawTexture(pN, rc, 0xffffffff, 0);
	if (s_statViewIdx983 == pItem->idx && pList != NULL)
	{
		pList->SetToolTipText(std::string());
		pList->m_nFlags.Turn(FWF_TOOLTIPS, false);
		s_statViewIdx983 = -1;
	}
}

// 006166b0: botao da linha em (x+ofsX, y+66), tamanho do bitmap; com o cursor em cima, bitmap _o e dica
static void DrawListBtn983(FrGraphicInterface* pGI, FrListItem* pItem, const char* nameN, const char* nameO, float ofsX, eBarButton tip)
{
	const Bitmap* pBtn = g_pFresh->GetBitmap(nameN);
	if (pBtn == NULL)
		return;
	WRect rc(pItem->pos.x + ofsX, pItem->pos.y + 66.0f, (float)pBtn->Width(), (float)pBtn->Height());
	if (pItem->underCursor && rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		if (const Bitmap* pBtnO = g_pFresh->GetBitmap(nameO))
			pBtn = pBtnO;
		if (tip != (eBarButton)-1) // fita de replay nao tem dica
			CIconToolTip::GetInstance()->SetToolTip(tip);
	}
	pGI->DrawTexture(pBtn, rc, 0xffffffff, 0);
}

static bool IsSpecialUccTid983(unsigned long tid);

// 006166b0 casos 0x16/0x1b: botoes UCC usam os retangulos +0xa14/+0xa24 (00608600), ambos do tamanho de ucc_draw_n
static bool UccBtnRect983(FrListItem* pItem, float ofsX, WRect& rc)
{
	const Bitmap* pRef = g_pFresh->GetBitmap("ucc_draw_n");
	if (pRef == NULL)
		return false;
	rc = WRect(pItem->pos.x + ofsX, pItem->pos.y + 66.0f, (float)pRef->Width(), (float)pRef->Height());
	return true;
}

static void DrawUccBtn983(FrGraphicInterface* pGI, FrListItem* pItem, const char* nameN, const char* nameO, float ofsX, eBarButton tip)
{
	WRect rc;
	if (!UccBtnRect983(pItem, ofsX, rc))
		return;
	const Bitmap* pBtn = g_pFresh->GetBitmap(nameN);
	if (pItem->underCursor && rc.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		if (const Bitmap* pBtnO = g_pFresh->GetBitmap(nameO))
			pBtn = pBtnO;
		CIconToolTip::GetInstance()->SetToolTip(tip);
	}
	if (pBtn != NULL)
		pGI->DrawTexture(pBtn, rc, 0xffffffff, 0);
}

// 006166b0 (00617510..00617935 e caso 0x1b): roupa UCC sem desenho = copiar (cat 7 em x+103, cat 9 em x+144) e desenhar/redesenhar
// (cat 7/8 em x+144, redraw com status bit 2); ja desenhada, so na aba 0x1b: ucc_exhibit_btn
static void DrawUccBtns983(FrGraphicInterface* pGI, FrListItem* pItem, sItemInfo* pInfo, int sub)
{
	const IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->typeId);
	if (pPart == NULL)
		return;
	int cat = pPart->Category;
	if ((pInfo->status & 1) == 0)
	{
		if (cat == 7 || cat == 9)
			DrawUccBtn983(pGI, pItem, "ucc_copy_n", "ucc_copy_o", cat == 7 ? 103.0f : 144.0f, UCC_TOOLTIP_COPY);
		if (cat == 7 || cat == 8)
		{
			if (pInfo->status & 2)
				DrawUccBtn983(pGI, pItem, "ucc_redraw_btn_n", "ucc_redraw_btn_o", 144.0f, UCC_TOOLTIP_REDRAW);
			else
				DrawUccBtn983(pGI, pItem, "ucc_draw_n", "ucc_draw_o", 144.0f, UCC_TOOLTIP_DRAW);
		}
	}
	else if (sub == 0x1b && (cat == 7 || cat == 8))
		DrawUccBtn983(pGI, pItem, "ucc_exhibit_btn_n", "ucc_exhibit_btn_o", 144.0f, UCC_TOOLTIP_EXHIBIT);
}

// 006166b0 caso 0xb / 00620e00 caso 0xb: caddie de aluguel vencido, sem peca, com +0xc0 no IFF
static bool IsCaddieReemploy983(const sCaddieInfo* info)
{
	if (!info->Rent_flag || info->Remain_Date != 0 || info->tidPart != 0)
		return false;
	IFF_ITEM_COMMON* cm = ItemManager()->FindCommonItem(info->tid);
	return cm != NULL && *(int*)((char*)cm + 0xc0) != 0;
}

static void DrawCaddieMascotRow983(FrListBox* pList, FrGraphicInterface* pGI, FrListItem* pItem, int tab)
{
	if (pItem->pData == NULL)
		return;
	unsigned long tid = *(unsigned long*)((char*)pItem->pData + 4);
	const char* name = NULL;
	const char* icon = NULL;
	IFF_ITEM_COMMON* cm = NULL;
	if (tab == 0xb)
	{
		IFF_STRUCT::sCaddie* cd = ItemManager()->FindCaddie(tid);
		if (cd == NULL)
			return;
		name = cd->c.Name;
		icon = cd->c.Icon;
	}
	else
	{
		cm = ItemManager()->FindCommonItem(tid);
		if (cm == NULL)
			return;
		name = cm->Name;
		icon = cm->Icon;
	}
	const Bitmap* pIcon = g_pFresh->GetBitmap(icon);
	if (pIcon != NULL)
	{
		// 983: os casos 0xb (00616a6b) e 0xd (00616b20) saltam para o caminho comum 00617958 = aba Item: ponto = pos (cutin: y-12)
		// + ReviseItemIconPoint(tid), tamanho nativo; no mascote o tid vai sem o nibble baixo (tid&~0xf). Centrar em (41,52) punha o
		// icone do mascote 4-9 px abaixo do original (d271-1/013 x orig-c151-1/014).
		unsigned long rtid = (tab == 0xd && (tid & 0x3000000) != 0x3000000) ? (tid & 0xfffffff0) : tid;
		WPoint iconPt(pItem->pos.x, pItem->pos.y);
		if (S5::ISCutinItem(rtid))
			iconPt.y -= 12.0f;
		ReviseItemIconPoint(rtid, iconPt);
		WRect rect(iconPt.x, iconPt.y, (float)pIcon->Width(), (float)pIcon->Height());
		pGI->DrawTexture(pIcon, rect, 0xffffffff, 0);
		// 983 (00617b30): caddie alugado em ferias (sem prazo nem parte) = caddie_rest sobre o icone
		sCaddieInfo* ciRest = (sCaddieInfo*)pItem->pData;
		IFF_STRUCT::sCaddie* cdRest = tab == 0xb ? ItemManager()->FindCaddie(tid) : NULL;
		const Bitmap* pRest = g_pFresh->GetBitmap("caddie_rest");
		if (cdRest != NULL && ciRest->Rent_flag && ciRest->Remain_Date == 0 && ciRest->tidPart == 0 && cdRest->MonthlyFee != 0 && pRest != NULL)
			pGI->DrawTexture(pRest, WRect(iconPt.x, iconPt.y, (float)pRest->Width(), (float)pRest->Height()), 0xffffffff, 0);
	}
	// botao "Stats" da linha (00614920; caddie 7 = Attr do sCaddie, mascote 0x10 = Attr do sMascot)
	{
		short stats[10] = { 0 };
		if (tab == 0xb)
		{
			if (IFF_STRUCT::sCaddie* pCd = ItemManager()->FindCaddie(tid))
				memcpy(stats, pCd->Attr, sizeof(pCd->Attr));
		}
		else if (IFF_STRUCT::sMascot* pMs = ItemManager()->FindMascot(tid))
		{
			for (int i = 0; i < 5; ++i)
				stats[i] = pMs->Attr[i];
		}
		DrawStatView983(pList, pGI, pItem, stats);
	}
	pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f), 2, "%s", name);
	if (tab == 0xb)
	{
		// 006166b0 caso 7 (caddie sem aluguel): com parte (tidPart&0x1fff) escreve "Remaining Time:" e Remain_Partdate em dias/horas (C156.1)
		sCaddieInfo* ci = (sCaddieInfo*)pItem->pData;
		WPoint q1(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f);
		WPoint q2(pItem->pos.x + 182.0f, pItem->pos.y + 39.0f);
		IFF_STRUCT::sCaddie* cdr = ItemManager()->FindCaddie(tid);
		if (ci->Rent_flag && cdr != NULL && cdr->MonthlyFee != 0)
		{
			// 006166b0 caso 7 aluguel: Remain_Date em dias; com parte, "Remaining Time" da parte em +16/+32 (C156.1)
			if (ci->Remain_Date == 0)
			{
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(q1, 2, "%s", K2L_Compatibility("\xc8\xde\xb0\xa1\xc1\xdf"));
				return;
			}
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(q1, 2, "%s", K2L_Compatibility("\xb3\xb2\xc0\xba \xb0\xed\xbf\xeb\xb1\xe2\xb0\xa3:"));
			pGI->Print11(q2, 2, K2L_Compatibility("%d\xc0\xcf"), (int)ci->Remain_Date);
			if ((ci->tidPart & 0x1fff) != 0)
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(WPoint(q2.x, q2.y + 16.0f), 2, "%s", K2L_Compatibility("\xb2\xd9\xb9\xcc\xb1\xe2 \xb3\xb2\xc0\xba \xb1\xe2\xb0\xa3:"));
				pGI->Print11(WPoint(q2.x, q2.y + 32.0f), 2, K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), ci->Remain_Partdate / 24, ci->Remain_Partdate % 24);
			}
			return;
		}
		if ((ci->tidPart & 0x1fff) == 0)
			return;
		pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f), 2, "%s", K2L_Compatibility("\xb2\xd9\xb9\xcc\xb1\xe2 \xb3\xb2\xc0\xba \xb1\xe2\xb0\xa3:"));
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 39.0f), 2, K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), ci->Remain_Partdate / 24, ci->Remain_Partdate % 24);
		return;
	}
	bool timed = *((char*)cm + 0x82) == 1;
	int days = 0, hours = 0;
	if (timed)
	{
		sMascotInfo* mi = (sMascotInfo*)pItem->pData;
		// 00616b20: 006333c0 = GetRemainHours983 (horas em SystemTimeToHours983, nao o FILETIME exato)
		__int64 h = DOCJP()->GetRemainHours983(&mi->endDate);
		if (h != 0)
		{
			days = (int)(h / 24);
			hours = (int)(h % 24);
		}
	}
	WPoint p1(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f);
	WPoint p2(pItem->pos.x + 182.0f, pItem->pos.y + 39.0f);
	pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
	pGI->Print11(p1, 2, "%s", K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:"));
	if (!timed)
		pGI->Print11(p2, 2, "%s", K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"));
	else if (days == 0 && hours == 0)
	{
		pGI->SetTextColor(0xffff0000, 0xffffffff);
		pGI->Print11(p2, 2, "%s", K2L_Compatibility("1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8"));
	}
	else
		pGI->Print11(p2, 2, K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, hours);
}

char const* __fastcall MakeStatusString(int a, int b, int c); // 983 b2e010 (shopmain.h)

// 006166b0 (parcial, B410): a grade de itens do My Room. Fundo (item_base_n/d/empty), icone da FindCommonItem e nome.
// Faltam: icone UCC, grade/rental, botoes por aba, ItemBuff, caddie_no.
// 983 00615ab0 (UnequipItem): o item do slot volta a lista do inventario (se nao estiver la) e o slot e zerado
void CRealMyRoomForm983::UnequipItem983(unsigned long* slot)
{
	CSharedDoc* doc = Doc();
	FrListBox* itemBox = m_pItemList983;
	for (std::list<sItemInfo>::iterator it = doc->m_myItemList.begin(); it != doc->m_myItemList.end(); ++it)
	{
		if (it->typeId != *slot)
			continue;
		bool inList = false;
		for (FrListBox::ITEM_LIST::iterator li = itemBox->m_itemList.begin(); li != itemBox->m_itemList.end(); ++li)
		{
			sItemInfo* pi = (sItemInfo*)(*li)->pData;
			if (pi && pi->typeId == *slot)
			{
				inList = true;
				break;
			}
		}
		if (!inList)
		{
			itemBox->AddItem(&*it);
			itemBox->SortItem(EmptyCompares);
		}
	}
	*slot = 0;
}

// 983 0061a710 (OnItemSlotDClick): com NT_SC_ITEMSLOT ligado e +0xa84 == 0 (ou sem NT_SC_ITEMSLOT), desequipa o slot sob o cursor
// 0061a640: tira o item selecionado do slot; a selecao passa ao proximo slot ocupado
void CRealMyRoomForm983::OnEquipCancelUp983()
{
	if (FrListBox* slotBox = m_pItemSlot983)
	{
		if (FrListItem* sel = slotBox->GetSelected())
		{
			FrListBox::ITEM_LIST::iterator it = std::find(slotBox->m_itemList.begin(), slotBox->m_itemList.end(), sel);
			if (it != slotBox->m_itemList.end())
				++it;
			for (; it != slotBox->m_itemList.end(); ++it)
			{
				if ((*it)->pData && *(unsigned long*)(*it)->pData)
				{
					slotBox->SelectItem(*it, true);
					break;
				}
			}
			if (sel->pData)
				UnequipItem983((unsigned long*)sel->pData);
		}
	}
	OnTooltipHoverOff983();
}

// 0061a6c0: esvazia os 10 slots
void CRealMyRoomForm983::OnEquipResetUp983()
{
	if (FrListBox* slotBox = m_pItemSlot983)
	{
		for (FrListBox::ITEM_LIST::iterator it = slotBox->m_itemList.begin(); it != slotBox->m_itemList.end(); ++it)
		{
			if ((*it)->pData)
				UnequipItem983((unsigned long*)(*it)->pData);
		}
	}
	OnTooltipHoverOff983();
}

void CRealMyRoomForm983::OnItemSlotDClick983()
{
	if (IsLocalContent(S4_ITEM_STORAGE) && m_storageOpen983) // 983 0061a713
		return;
	FrListBox* slotBox = m_pItemSlot983;
	if (!slotBox)
		return;
	FrListItem* item = slotBox->GetItemUnderCursor();
	if (!item || !item->pData || !*(unsigned long*)item->pData)
		return;
	UnequipItem983((unsigned long*)item->pData);
}

// 983 0061f6d0 CRealMyRoomMain::OnPetOwnerDraw: fora da aba principal 4 (e na sub-aba 0x20 dela) desenha a previa (0060d9b0).
// Portao 00e78ca8+0x240 (lobby travado): nao desenha nada.
void CRealMyRoomForm983::OnPetOwnerDraw983(int)
{
	if (CLobbyState983::ms_pInstance && CLobbyState983::ms_pInstance->m_bLocked)
		return;
	// 0061f6d0: fora da aba 4, sub-aba 0x1c com card pack selecionado (tid&0x3c00000 == 0x1000000, cardCount < 10): mostra os dummies card_pack*_Dummy
	if (m_mainTab983 != 4 && m_subTab983 == 0x1c)
	{
		FrListBox* pack = m_pCardPackList983;
		FrListItem* sel = pack ? pack->GetSelected() : NULL;
		sCards* pInfo = sel ? (sCards*)sel->pData : NULL;
		CExhibition* pv = m_pPreview983;
		if (pInfo && pv && (pInfo->tid & 0x3c00000) == 0x1000000 && pInfo->cardCount < 10)
		{
			WBone* pRoot = pv->GetModel()->GetRootBone();
			WBone* pBone = pRoot->FindBone("card_pack_Dummy", 0);
			if (pBone)
				pBone->SetInvisible(false, true);
			for (int i = 1; i < 10 - (int)pInfo->cardCount; i++)
			{
				pBone = pRoot->FindBone(MakeStr("card_pack%02d_Dummy", i), 0);
				if (pBone)
					pBone->SetInvisible(false, true);
			}
		}
	}
	if (m_mainTab983 != 4)
	{
		// 00605e70: "ucc_draw" aberto com a roupa carregada desenha o personagem ele mesmo (007e2150)
		if (!(IsLocalContent(S4_UCC) && m_pUccDrawDlg983 != NULL && m_pUccDrawDlg983->GetItemInfo983() != NULL))
			MyRoomPetDraw983((char*)this);
	}
	else if (m_subTab983 == 0x20)
		MyRoomPetDraw983((char*)this);
	else if (m_subTab983 == 0x22)
		DrawCutin983(); // 0061ee90
	else if (m_subTab983 >= 0xe && m_subTab983 <= 0x10)
		DrawSkinBack983(); // 0060da80
	else if (m_subTab983 == 0x11 || m_subTab983 == 0x23)
		DrawSkinSlot983(); // 0060de60
}

// 983 0060de60 (sub-abas 0x11/0x23 da aba 4): cartao do jogador em (116, 308) (origem; o 983 soma o deslocamento da view, 0 aqui):
// fundo, skin +0x970, personagem (+0x4f8) em (+8,+36) com a skin +0x974, bandeira de sexo (Doc+0x832 bit 3) em (+5,+98),
// nivel (skin +0x978 ou level_%03d do Doc+0x91d) centrado em (+90,+78), selo em (+63,+5) e o nome (Doc+0x7b8) em (139,415).
// Mesmo desenho do 645 DrawSkinSlot; os bitmaps +0x320..+0x32c do 983 sao os mesmos nomes (team_base/i_male/i_female/master_sign).
void CRealMyRoomForm983::DrawSkinSlot983()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	const float ox = 116.0f, oy = 308.0f;
	const Bitmap* pBase = g_pFresh->GetBitmap("team_base");
	if (!pBase)
		return;
	pGI->DrawTexture(pBase, WRect(ox, oy, (float)pBase->Width(), (float)pBase->Height()), 0xffffffff, 0);
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBitmap;
	unsigned long tid = m_skinTid983[3];
	if (tid && (pSkin = ItemManager()->FindSkin(tid)) != NULL && (pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
		pGI->DrawTexture(pBitmap, WRect(ox, oy, (float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_selCharTid983);
	if (pChar && (pBitmap = g_pFresh->GetBitmap(pChar->Data)) != NULL)
		pGI->DrawTexture(pBitmap, WRect(ox + 8.0f, oy + 36.0f, (float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	tid = m_skinTid983[4];
	if (tid && (pSkin = ItemManager()->FindSkin(tid)) != NULL && (pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
		pGI->DrawTexture(pBitmap, WRect(ox + 8.0f, oy + 36.0f, (float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	pBitmap = g_pFresh->GetBitmap((*(unsigned char*)DOCPTR983(0x832) >> 3) & 1 /* 983 bit 3 do byte de bits apos dwEmblemVer */ ? "i_female" : "i_male");
	if (pBitmap)
		pGI->DrawTexture(pBitmap, WRect(ox + 5.0f, oy + 98.0f, (float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	tid = m_skinTid983[5];
	const char* pszIcon;
	if (tid == 0 || (pSkin = ItemManager()->FindSkin(tid)) == NULL)
		pszIcon = MakeStr("level_%03d", Doc()->m_myInfo.stat.Level + 1);
	else
		pszIcon = pSkin->Tex;
	pBitmap = g_pFresh->GetBitmap(pszIcon);
	if (pBitmap)
		pGI->DrawTexture(pBitmap, WRect(ox + 90.0f - (int)(pBitmap->Width() * 0.5f), oy + 78.0f - (int)(pBitmap->Height() * 0.5f),
			(float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	pBitmap = g_pFresh->GetBitmap("master_sign");
	if (pBitmap)
		pGI->DrawTexture(pBitmap, WRect((int)(ox + 63.5f - pBitmap->Width() * 0.5f), oy + 5.0f,
			(float)pBitmap->Width(), (float)pBitmap->Height()), 0xffffffff, 0);
	pGI->SetTextColor(0xffffffff, 0xff808080);
	pGI->SetTextStyle(2);
	g_pFresh->GetManager()->PrintText(WPoint(139.0f, 415.0f), 0, Doc()->m_myInfo.info.sNick, -1.0f, 0xffffffff);
}

// 983 0060da80 (sub-abas 0xe-0x10 da aba 4): fundo rolante da skin +0x964 ladrilhado na janela +0x150 (deslocamento +0x97c/+0x980),
// previa (Display) e as skins +0x968/+0x96c esticadas sobre a janela. Mesmo desenho do 645 DrawSkinFrame.
void CRealMyRoomForm983::DrawSkinBack983()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	FrWnd* wnd = m_pSkinWnd983;
	if (!pGI || !wnd)
		return;
	WRect area = wnd->GetRect();
	float* scroll = m_skinScroll983;
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBitmap;
	unsigned long tid = m_skinTid983[0];
	if (tid && (pSkin = ItemManager()->FindSkin(tid)) != NULL && (pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		float texW = (float)pBitmap->Width();
		float texH = (float)pBitmap->Height();
		for (int i = 0; i < 2; i++)
		{
			float t = i ? texH : texW;
			if (scroll[i] > t)
				scroll[i] -= t;
			else if (scroll[i] < 0.0f)
				scroll[i] += t;
		}
		float endX = area.x + area.w, endY = area.y + area.h;
		float dy = area.y;
		float sy = scroll[1];
		float h = texH - sy;
		while (dy < endY)
		{
			if (dy + h > endY)
				h = endY - dy;
			float dx = area.x;
			float sx = scroll[0];
			float w = texW - sx;
			while (dx < endX)
			{
				if (dx + w > endX)
					w = endX - dx;
				WRect src(sx, sy, w, h);
				WRect dst(dx, dy, w, h);
				pGI->DrawTexture(pBitmap, src, dst, 0xffffffff, 0);
				dx += w;
				sx = 0.0f;
				w = texW;
			}
			dy += h;
			sy = 0.0f;
			h = texH;
		}
	}
	MyRoomPetDraw983((char*)this);
	for (int i = 1; i <= 2; ++i)
	{
		tid = m_skinTid983[i]; // +0x968/+0x96c
		if (tid && (pSkin = ItemManager()->FindSkin(tid)) != NULL && (pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
			pGI->DrawTexture(pBitmap, area, 0xffffffff, 0);
	}
}

// 983 0061d180: imagem "<Tex>.jpg" da skin (tid), em cache por tid (std::map em +0x474; aqui um map estatico)
static std::map<unsigned long, WOverlay*> s_cutin983;

// 983 0061ee90 (sub-aba 0x22 da aba 4): Display da previa e, se ha item selecionado na lista (+0x144), a imagem da skin em (72, 67)
// (DAT_00d09468/00d14a2c = 72/67, como o 645 DrawCutinFrame). NAO portado: a fila +0x598/+0x59c e o deslocamento da view (009b7810).
void CRealMyRoomForm983::DrawCutin983()
{
	MyRoomPetDraw983((char*)this);
	FrListBox* box = m_pItemList983;
	FrListItem* sel = box ? box->GetSelected() : NULL;
	sItemInfo* info = sel ? (sItemInfo*)sel->pData : NULL;
	if (!info)
		return;
	WOverlay* img = NULL;
	std::map<unsigned long, WOverlay*>::iterator it = s_cutin983.find(info->typeId);
	if (it != s_cutin983.end())
		img = it->second;
	else if (IFF_STRUCT::sSkin* skin = ItemManager()->FindSkin(info->typeId))
	{
		char name[64];
		sprintf(name, "%s.jpg", skin->Tex);
		img = g_resrcmng->GetOverlay(name, 0);
		if (img)
			s_cutin983[info->typeId] = img;
	}
	if (!img)
		return;
	WRect dst(72.0f, 67.0f, (float)img->GetWidth(), (float)img->GetHeight());
	WRect src(0.0f, 0.0f, 1.0f, 1.0f);
	img->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
}

// 983 0060d0d0: fundo item_slot_n/o/d (normal/cursor/selecionado; vazio = normal) e, com o tid != 0, o icone em (x+2, y+4)
void CRealMyRoomForm983::OnItemSlotOwnerDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL || pItem->pData == NULL)
		return;
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	unsigned long tid = *(unsigned long*)pItem->pData;
	const char* bn = "item_slot_n";
	static int s_trace; // C729: slots e previa somem so no VS 2022; causa nao achada no estatico
	if (s_trace < 3 && ++s_trace)
		jp983_rastro("C729 slot item=%p tid=%08x pos=%.0f,%.0f bmp=%p", pItem, tid, pItem->pos.x, pItem->pos.y, g_pFresh->GetBitmap(bn));
	if (tid != 0)
	{
		if (pItem->selected)
			bn = "item_slot_d";
		else if (pItem->underCursor)
			bn = "item_slot_o";
	}
	const Bitmap* pSlot = g_pFresh->GetBitmap(bn);
	if (pSlot != NULL)
	{
		WRect rect(pItem->pos.x, pItem->pos.y, (float)pSlot->Width(), (float)pSlot->Height());
		pGI->DrawTexture(pSlot, rect, 0xffffffff, 0);
	}
	if (tid == 0)
		return;
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
	if (pCommon == NULL)
		return;
	const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);
	if (pIcon != NULL)
	{
		WRect rect(pItem->pos.x + 2.0f, pItem->pos.y + 4.0f, (float)pIcon->Width(), (float)pIcon->Height());
		pGI->DrawTexture(pIcon, rect, 0xffffffff, 0);
	}
}

// 006166b0 caso 0x13 (0061759b): icone do trofeu (FindMatch, tid sem o nibble = indice do icone; tid 0x3000000 inteiro), nome em linhas
// (x+182, y+7, passo 15) e a quantidade em (x+180, y+67)
static void DrawTrophyRow983(FrGraphicInterface* pGI, FrListItem* pItem)
{
	const sSpecialTrophy* t = (const sSpecialTrophy*)pItem->pData;
	unsigned long tid = t->tid;
	unsigned long idx = 0;
	if ((tid & 0x3000000) != 0x3000000)
	{
		idx = tid & 0xf;
		tid &= 0xfffffff0;
	}
	IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(tid);
	if (pMatch == NULL)
		return;
	if (const Bitmap* pIcon = g_pFresh->GetBitmap(pMatch->Icon[idx]))
	{
		WPoint iconPt(pItem->pos.x, pItem->pos.y);
		ReviseItemIconPoint(tid, iconPt);
		pGI->DrawTexture(pIcon, WRect(iconPt.x, iconPt.y, (float)pIcon->Width(), (float)pIcon->Height()), 0xffffffff, 0);
	}
	char text[100];
	strncpy(text, pMatch->Name, sizeof(text) - 1);
	text[sizeof(text) - 1] = 0;
	char* line = text;
	int n = 0;
	for (char* nl = strstr(line, "\n"); nl != NULL; nl = strstr(line, "\n"), ++n)
	{
		*nl = 0;
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f + n * 15.0f), 2, "%s", line);
		line = nl + 1;
	}
	pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f + n * 15.0f), 2, "%s", line);
	pGI->Print11(WPoint(pItem->pos.x + 180.0f, pItem->pos.y + 67.0f), 2, "%d", (int)t->count);
}

void CRealMyRoomForm983::OnItemListOwnerDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	const Bitmap* pBack;
	unsigned long color = 0xff000000;
	if (pItem->pData == NULL)
		pBack = g_pFresh->GetBitmap("item_base_empty");
	else
	{
		const char* bn = "item_base_n";
		if (pItem->selected)
		{
			color = 0xffff0000;
			bn = "item_base_d";
		}
		else if (pItem->underCursor)
			bn = "item_base_o";
		pBack = g_pFresh->GetBitmap(bn);
		pGI->SetTextColor(color, 0xffffffff);
	}
	pGI->SetTextStyle(0);
	if (pBack != NULL)
	{
		WRect rect(pItem->pos.x, pItem->pos.y, (float)pBack->Width(), (float)pBack->Height());
		pGI->DrawTexture(pBack, rect, 0xffffffff, 0);
	}
	int curTab = m_subTab983;
	if (curTab == 0x13)
	{
		if (pItem->pData != NULL)
			DrawTrophyRow983(pGI, pItem);
		return;
	}
	if (curTab == 0xb || curTab == 0xd)
	{
		DrawCaddieMascotRow983(m_pItemList983, pGI, pItem, curTab);
		if (curTab == 0xb && pItem->pData != NULL && IsCaddieReemploy983((sCaddieInfo*)pItem->pData)) // 983 botao +0x7e8
			DrawListBtn983(pGI, pItem, "reemploy_btn_n", "reemploy_btn_o", 144.0f, TOOLTIP_REEMPLOY);
		return;
	}
	sItemInfo* pInfo = (sItemInfo*)pItem->pData;
	if (pInfo == NULL)
		return;
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pInfo->typeId);
	if (pCommon == NULL)
		return;
	// 983 (00617510 / caso 0x1b): roupa UCC usa nome (007e96a0) e icone (007e9620) proprios
	bool ucc = curTab == 0x1b || (IsLocalContent(S4_UCC) && IsUccClothes(pInfo->typeId));
	const Bitmap* pIcon = ucc ? GetUccItemIcon(pInfo) : g_pFresh->GetBitmap(pCommon->Icon);
	// 983 (00617a1b): ponto do icone = pos (cutin: y-12) + ReviseItemIconPoint(typeId)
	WPoint iconPt(pItem->pos.x, pItem->pos.y);
	if (S5::ISCutinItem(pInfo->typeId))
		iconPt.y -= 12.0f;
	ReviseItemIconPoint(pInfo->typeId, iconPt);
	if (pIcon != NULL)
	{
		WRect rect(iconPt.x, iconPt.y, (float)pIcon->Width(), (float)pIcon->Height());
		pGI->DrawTexture(pIcon, rect, 0xffffffff, 0);
	}
	// 983 (006166b0 casos 9 e default): tipo do item (+0x17 bits 4-6); dias/horas so no default. As demais abas deixam 0
	bool tabTipo = !(curTab == 0x12 || curTab == 0x13 || curTab == 0x15 || curTab == 0x18 || (curTab >= 0x1b && curTab <= 0x21) || curTab == 0x26);
	int grade = tabTipo ? pInfo->ItemType : 0;
	int days = (tabTipo && curTab != 9) ? pInfo->HourRemain / 0x18 : 0;
	int hours = (tabTipo && curTab != 9) ? pInfo->HourRemain % 0x18 : 0;
	WPoint pos1(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f);
	WPoint pos2(pos1.x, pos1.y + 16.0f);
	unsigned int kind = pInfo->typeId >> 26;
	// 983 (00616846, caso 9): quantidade menos cada um dos 10 slots com o mesmo tid
	int qty = (int)pInfo->Common[0];
	if (curTab == 9)
		if (CSharedDoc* docQty = CSharedDoc::Instance())
			for (int i = 0; i < 10; ++i)
				if (docQty->m_myInfo.userEquip.tidItemSlot[i] == pInfo->typeId)
					--qty;
	// 983 (00617ac7): relogio +0x830 (32,30) sobre o icone de item com prazo; titulo so tipo 2; tipo 6 (aluguel) nao
	if ((kind == 0xe ? grade == 2 : grade > 1 && pInfo->typeId != 0x1a000042) && grade != 6)
	{
		if (const Bitmap* pClock = g_pFresh->GetBitmap("clock"))
			pGI->DrawTexture(pClock, WRect(iconPt.x + 32.0f, iconPt.y + 30.0f, (float)pClock->Width(), (float)pClock->Height()), 0xffffffff, 0);
	}
	pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f), 2, "%s", ucc ? GetUccItemName(pInfo) : pCommon->Name);
	// 983 (00617e2c): marca da roupa UCC em (x+51, y+50): original, copia (fora da tabela 00606c30) ou rascunho
	if (IsLocalContent(S4_UCC) && IsUccClothes(pInfo->typeId))
	{
		const char* mark = NULL;
		if ((pInfo->status & 1) && pInfo->Seq == 1)
			mark = "ucc_state_original";
		else if (pInfo->status & 1)
		{
			if (pInfo->Seq >= 2 && !IsSpecialUccTid983(pInfo->typeId))
				mark = "ucc_state_copied";
		}
		else if (pInfo->status & 2)
			mark = "ucc_state_temp_saved";
		if (const Bitmap* pMark = mark ? g_pFresh->GetBitmap(mark) : NULL)
			pGI->DrawTexture(pMark, WRect(pItem->pos.x + 51.0f, pItem->pos.y + 50.0f, (float)pMark->Width(), (float)pMark->Height()), 0xffffffff, 0);
	}
	// 983 (006196fe/0061977c, caso 6 = itens ativos/passivos): "%d<gae>" (DAT_00d144c0) em (x+182, y+23) com a quantidade (Common[0], +0xc)
	// 00619cc6: tid 0x1a000006 (Lucky Necklace) nao mostra quantidade; 0x1a000010 (Pang) mostra "%d<pa>" (DAT_00d14818)
	if (kind == 6 && grade > 1)
	{
		// 983 (006197b4): item com prazo = "Remaining Time:" + "%dd %dh" ou "<1h" em vermelho; Tiki Report = data de +0x18
		if (pInfo->typeId == 0x1a000042 && IsLocalContent(S3_CADDIE_REPORT))
		{
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2, K2L_Compatibility("%d\xb3\xe2 %d\xbf\xf9 %d\xc0\xcf"), pInfo->ItemDate.wYear, pInfo->ItemDate.wMonth, pInfo->ItemDate.wDay);
			pGI->Print11(pos2, 2, K2L_Compatibility(" %d\xbd\xc3 %d \xba\xd0"), pInfo->ItemDate.wHour, pInfo->ItemDate.wMinute);
		}
		else
		{
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2, "%s", K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:"));
			if (days == 0 && hours == 0)
			{
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(pos2, 2, "%s", K2L_Compatibility("1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8"));
			}
			else
				pGI->Print11(pos2, 2, K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, hours);
		}
	}
	else if (kind == 6 && curTab == 0x26)
	{
		// 983 (00619b20): texto do artefato em azul (x+182, y+21); "(com mana)" ou icone mana_no + "(sem mana)" em (x+182, y+35)
		const char* art = ItemManager()->FindArtifactDesc983(pInfo->typeId);
		pGI->SetTextColor(0xff0000ff, 0xffffffff);
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 21.0f), 2, "%s", art ? art : "");
		pGI->SetTextColor(color, 0xffffffff);
		bool usable = ((sArtifactItem983*)pInfo)->bUsable;
		if (!usable)
			if (const Bitmap* pMana = g_pFresh->GetBitmap("mana_no"))
				pGI->DrawTexture(pMana, WRect(iconPt.x, iconPt.y, (float)pMana->Width(), (float)pMana->Height()), 0xffffffff, 0);
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 35.0f), 2, "%s",
			K2L_Compatibility(usable ? "(\xb8\xb6\xb7\xc2 \xc0\xd6\xc0\xbd)" : "(\xb8\xb6\xb7\xc2 \xbe\xf8\xc0\xbd)"));
	}
	else if (kind == 6 && (curTab == 9 || curTab == 10 || curTab == 0x14 || curTab == 0x19) && pInfo->typeId != 0x1a000006) // 983: qualquer aba do default
	{
		if (pInfo->typeId == 0x1a000010)
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f), 2, K2L_Compatibility("%d\xc6\xce"), qty);
		else
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f), 2, K2L_Compatibility("%d\xb0\xb3"), qty);
	}
	// 983 (006191f2 case 5, D407): bola mostra a quantidade em (x+182, y+23), exceto a bola padrao/sem bound+COM (00a18f90) e,
	// com bit 0xe de doc+0x812, a 0x140000d8
	if ((pInfo->typeId >> 26) == 5)
	{
		bool plain = false;
		if (pInfo->typeId == 0x14000000)
			plain = true;
		else if (IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(pInfo->typeId))
			plain = *(int*)((char*)pBall + 0x74) == 0 && *(short*)((char*)pBall + 0x324) == 0;
		CSharedDoc* doc983 = CSharedDoc::Instance();
		bool hideD8 = pInfo->typeId == 0x140000d8 && doc983 && (doc983->m_myInfo.info.dwIdentity >> 0xe & 1) != 0;
		if (!plain && !hideD8)
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f), 2, K2L_Compatibility("%d\xb0\xb3"), qty);
	}
	// 983 (00617f8a): 0x2c2 modo 0 ao "ItemBuff": item de buff desenha o mini botao dele e nenhum botao da aba
	char buffHandled = 0;
	if (curTab != 0x18)
		AfxGetTask()->GetActor("ItemBuff") << MsgObject(NULL, M983(0x246), 0, (int)&buffHandled,
			(int)pItem, (int)&g_pFresh->GetManager()->GetMousePos(), pInfo->typeId);
	// 983 (006181ac..0061895c, aba 9 = Active): botao equip_btn_n em (x+103,y+66), dica 0x1f; se o item pode ser apagado
	// (!(Common+0x80 & 1)), delete_btn_n em (x+144,y+66), dica 0x21 (bitmaps/retangulos criados em 00608801..00608909)
	if (curTab == 9 && !buffHandled)
	{
		DrawListBtn983(pGI, pItem, "equip_btn_n", "equip_btn_o", 103.0f, TOOLTIP_EQUIP);
		if (!(*((unsigned char*)pCommon + 0x80) & 1))
			DrawListBtn983(pGI, pItem, "delete_btn_n", "delete_btn_o", 144.0f, TOOLTIP_DELETE);
	}
	// 983 (00618572, aba 10 = Passive): giftmove_btn em (x+103,y+66), dica 0x26 (0x3b: 0x25); so os tids de HasGiftButton983.
	// 006188d2: cut-in 0x1f00000 tem delete_btn em (x+144,y+66), sem dica
	if (curTab == 10 && !buffHandled)
	{
		if (HasGiftButton983(pInfo->typeId))
			DrawListBtn983(pGI, pItem, "giftmove_btn_n", "giftmove_btn_o", 103.0f, pInfo->typeId == 0x1a00003b ? TOOLTIP_HANDSEL : TOOLTIP_OPENGIFT);
		if ((pInfo->typeId >> 26) == 6 && (pInfo->typeId & 0x1f00000) == 0x1f00000)
			DrawListBtn983(pGI, pItem, "delete_btn_n", "delete_btn_o", 144.0f, (eBarButton)-1);
	}
	// 983 (00618e7a): play sempre, apagar so com o cursor no item; sem dica. Texto (caso 6, aba 0x18): curso e data da gravacao
	if (curTab == 0x18 && IsLocalContent(S4_REPLAY_SYSTEM))
	{
		DrawListBtn983(pGI, pItem, "replaymode_btn_n", "replaymode_btn_o", 144.0f, (eBarButton)-1);
		DrawListBtn983(pGI, pItem, "replaydel_btn_n", "replaydel_btn_o", 103.0f, (eBarButton)-1);
		const sRecordedItemInfo* pRec = (const sRecordedItemInfo*)pInfo;
		if (IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(pRec->course | 0x28000000))
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 22.0f), 2, pCourse->c.Name);
		if (pRec->date.wYear != 0)
		{
			char text[52];
			wsprintfA(text, "%d\xb3\xe2%02d\xbf\xf9%02d\xc0\xcf", pRec->date.wYear, pRec->date.wMonth, pRec->date.wDay);
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 33.0f), 2, text);
			wsprintfA(text, "%02d\xbd\xc3%02d\xba\xd0", pRec->date.wHour, pRec->date.wMinute);
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 44.0f), 2, text);
		}
	}
	// 983 (00617951, casos 0x20/0x21): poster (categoria 0xe) = poster_btn_n; marcado +0x8c0 = poster_btn_d com a dica 0x48 (00609113; _o nao existe)
	if ((curTab == 0x20 || curTab == 0x21) && ((pInfo->typeId >> 0xb) & 0x7ff) == 0xe)
	{
		if (const Bitmap* pPoster = g_pFresh->GetBitmap(pItem->selected ? "poster_btn_d" : "poster_btn_n"))
		{
			pGI->DrawTexture(pPoster, WRect(pItem->pos.x + 144.0f, pItem->pos.y + 66.0f, (float)pPoster->Width(), (float)pPoster->Height()), 0xffffffff, 0);
			if (pItem->selected)
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_USE);
		}
	}
	if (ucc && (curTab == 0x16 || curTab == 0x1b))
		DrawUccBtns983(pGI, pItem, pInfo, curTab);
	// 983 (006166b0 caso 0x19): botao do Tiki Report no mesmo retangulo +0x794 do giftmove
	if (curTab == 0x19 && !buffHandled && pInfo->typeId == 0x1a000042 && IsLocalContent(S3_CADDIE_REPORT))
		DrawListBtn983(pGI, pItem, "giftmove_btn_n", "giftmove_btn_o", 103.0f, TOOLTIP_REPORT);
	// 983 (0061a2ab, caso 4 = taco): "Cost of Upgrades:" (verde) em (x+182, y+23) e "%d Pang" em +16; soma de ReqdPang dos Enchant (D267)
	if ((pInfo->typeId >> 26) == 4)
	{
		int total = 0;
		if (curTab != 0x12)
			for (int stat = 0; stat < 5; ++stat)
				for (int n = 0; n < pInfo->Common[stat]; ++n)
				{
					IFF_STRUCT::sEnchant* pEnchant = ItemManager()->FindEnchant(((stat | 0x340) << 20) | n);
					if (pEnchant != NULL)
						total += pEnchant->ReqdPang;
				}
		pGI->SetTextColor(0xff009000, 0xffffffff);
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f), 2, "%s", K2L_Compatibility("\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xc7\xd1 \xba\xf1\xbf\xeb:"));
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 39.0f), 2, K2L_Compatibility("%d\xc6\xce"), total);
		// botao "Stats" do taco (00614920; caso 4 = Attr/Slot do sClubSet, 00606580)
		{
			short stats[10] = { 0 };
			if (IFF_STRUCT::sClubSet* pSet = ItemManager()->FindClubSet(pInfo->typeId))
			{
				memcpy(stats, pSet->Attr, sizeof(pSet->Attr));
				memcpy(stats + 5, pSet->Slot, sizeof(pSet->Slot));
			}
			DrawStatView983(m_pItemList983, pGI, pItem, stats);
		}
		// 006166b0 caso 4 (0061a14f): icone de oficina (clubmade_icon_make) em (x+155, y+53) se o taco mudou de passo/grau (C192.1)
		if (curTab != 0x12 && (IsClubStepChanged983(pInfo->id) || IsClubGradeChanged983(pInfo->id)))
		{
			const Bitmap* pMade = g_pFresh->GetBitmap("clubmade_icon_make");
			if (pMade != NULL)
			{
				WRect rcMade(pItem->pos.x + 155.0f, pItem->pos.y + 53.0f, (float)pMade->Width(), (float)pMade->Height());
				pGI->DrawTexture(pMade, rcMade, 0xffffffff, 0);
			}
		}
	}
	// 983 (006191f2 case 2 -> 00604d10): partes mostram as linhas de bonus ("Spin+1/Slot+1"), azul, passo 14 a partir de y+36
	if ((pInfo->typeId >> 26) == 2)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->typeId);
		if (pPart != NULL)
		{
			float dy = 0.0f;
			for (int i = 0; i < 5; ++i)
			{
				if (pPart->Attr[i] == 0 && pPart->Slot[i] == 0)
					continue;
				pGI->SetTextColor(0xff0000ff, 0xffffffff);
				pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 36.0f + dy), 2, "%s", MakeStatusString(i, pPart->Attr[i], pPart->Slot[i]));
				dy += 14.0f;
			}
			// 983 (006194b3): marca de iniciante +0x3fc em (x+5, y+65): Beginner sem slots de personagem/caddie
			const Bitmap* pBegin = g_pFresh->GetBitmap("mark_begin");
			if (pPart->Beginner == 1 && pPart->CharacterSlot == 0 && pPart->CaddieSlot == 0 && pBegin != NULL)
				pGI->DrawTexture(pBegin, WRect(pItem->pos.x + 5.0f, pItem->pos.y + 65.0f, (float)pBegin->Width(), (float)pBegin->Height()), 0xffffffff, 0);
		}
		// 983 (00619562/006195a9): tipo 6 = aluguel: relogio ou faixa de vencido; tipos 2..5 so o tempo
		bool expired = false;
		if (grade == 6)
		{
			if (pInfo->Expired)
			{
				expired = true;
				if (const Bitmap* pExp = g_pFresh->GetBitmap("rental_item_expired"))
					pGI->DrawTexture(pExp, WRect(pItem->pos.x, pItem->pos.y, (float)pExp->Width(), (float)pExp->Height()), 0xffffffff, 0);
			}
			else if (const Bitmap* pRent = g_pFresh->GetBitmap("myroom_rental_clock"))
				pGI->DrawTexture(pRent, WRect(pItem->pos.x + 70.0f, pItem->pos.y + 43.0f, (float)pRent->Width(), (float)pRent->Height()), 0xffffffff, 0);
		}
		if (!expired && grade >= 2)
		{
			if (days < 1)
			{
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(pos1, 2, K2L_Compatibility("\xb3\xb2\xc0\xba\xb1\xe2\xb0\xa3 : %d\xbd\xc3\xb0\xa3"), hours);
			}
			else
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos1, 2, K2L_Compatibility("\xb3\xb2\xc0\xba\xb1\xe2\xb0\xa3 : %d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, hours);
			}
		}
	}
	// 983 (0061a3be): titulo que o personagem ainda nao pode usar = "do not fit" em (x+1, y+1)
	if (kind == 0xe && (pInfo->typeId & 0x3c00000) == 0x1800000)
	{
		Title* pTitle = g_TitleManager->GetTitle((eTitle)(pInfo->typeId & 0x3fffff));
		const Bitmap* pNoFit = g_pFresh->GetBitmap("do not fit");
		if (pTitle != NULL && !pTitle->CanEquip() && pNoFit != NULL)
			pGI->DrawTexture(pNoFit, WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, (float)pNoFit->Width(), (float)pNoFit->Height()), 0xffffffff, 0);
	}
	// 983 (006166b0 caso 0x1c): quantidade so sem os bits 0x1f0000
	if (kind == 0x1c && (pInfo->typeId & 0x1f0000) == 0)
		pGI->Print11(pos1, 2, K2L_Compatibility("%d\xb0\xb3"), qty);
	// 983 (0061a42d): item escolhido na busca (uid +0x250) = SelectItem_Mark sobre a linha inteira
	if (m_searchUid983 != 0xffffffff && m_searchUid983 == pInfo->id)
		if (const Bitmap* pMark = g_pFresh->GetBitmap("SelectItem_Mark"))
			pGI->DrawTexture(pMark, WRect(pItem->pos.x, pItem->pos.y, (float)m_pItemList983->GetItemWidth(), (float)m_pItemList983->GetItemHeight()), 0xffffffff, 0);
}

// 00605720 (+007f6d20/00befe70): mostra/esconde o Card Holic (CCardBook em +0xa78 e a lista cardpacklist em +0xab8)
// 006208f0 (+0060a390/00605a50, 00605b60, 0061ee10): modo da janela em +0xa3c (1 = normal, 2, 4 = bit 8); +0xa40 guarda o anterior
void CRealMyRoomForm983::SetViewMode983(int mode)
{
	char* self = (char*)this;
	for (;;)
	{
		int& cur = m_viewMode983;
		if (mode != cur && mode != 5)
			m_prevViewMode983 = cur;
		if (mode == 5) { mode = 2; continue; }
		if (mode != 1 && mode != 2 && mode != 4)
		{
			cur = mode;
			return;
		}
		unsigned bit = mode == 4 ? 8 : mode;
		cur = (cur & bit) ? (cur & ~bit) : (cur | bit); // 0060a390
		*(unsigned*)m_viewFlags983 = 0; // 00605a50
		m_viewFlagA48983 = 0;
		if (bit == 1) m_viewFlags983[0] = 1;
		else if (bit == 2) m_viewFlags983[1] = !m_viewFlags983[1];
		else if (bit == 4) { m_viewFlags983[2] = !m_viewFlags983[2]; m_viewFlags983[0] = m_viewFlags983[1] = m_viewFlags983[2]; }
		else if (bit == 8) m_viewFlags983[3] = !m_viewFlags983[3];
		m_viewNormal983 = (mode == 1);
		bool on = mode == 1;
		FrWnd* mid = m_pMiddleUi983;
		FrWnd* menu = m_pItemSearchBtn983;
		if (on)
		{
			if (mid) mid->SetElementFadeOut(false);
			SetMainTab983(m_mainTab983, true);
			if (menu) menu->SetVisible(true);
		}
		else
		{
			if (mid && mid->IsVisible()) mid->SetElementFadeOut(true); // B1471: era cru +0x44 & 1 (m_dwStyle VC10; cl 14.44 0x40)
			if (menu) menu->SetVisible(false);
		}
		bool showTab = (mode == 4);
		if (FrWnd* f = m_pFItemList983) f->SetVisible(showTab); // 00605b60
		if (FrWnd* f = m_pFTabMain983) f->SetElementFadeOut(!showTab);
		if (FrTabButton* t = (FrTabButton*)((FrWnd*)m_pTabMain983)) t->Select(m_cardTab983);
		cur = mode;
		return;
	}
}

// 00622630 casos 0 e 2 (8 = aviso notify so sem flag a47, sem chamador direto: nao portado) (so o ramo sem PC bang importa: flags +0xa44.. por modo)
void CRealMyRoomForm983::SelectMenu983(int menu)
{
	char* self = (char*)this;
	if (menu == 6)
	{
		// 00622730: abre "realmyroom_authority" (+0x244) modal 0x81, sem fade
		if (m_pAuthDlg983 == NULL)
		{
			CRealMyRoom_AuthorityDlg* dlg = CreateForm<CRealMyRoom_AuthorityDlg>(g_pFresh->GetManager(), this, "realmyroom_authority", NULL);
			m_pAuthDlg983 = dlg;
			FrForm::ms_bHasTail = false;
			dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnAuthDlgResult983, 0x81);
			dlg->SetFadeout(false);
		}
		return;
	}
	if (menu == 9)
	{
		// 00622795 (so com NT REAL_MYROOM)
		if (!IsLocalContent(S4_REAL_MYROOM))
			return;
		if (m_storageOpen983 == 0)
		{
			WSendPacket p((enumClientPacket)0xd3);
			p.Send(TO_GAME);
			SendDocSyncPackets983(Doc());
		}
		else
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2af, 0, 0, 0, 0, 0));
		return;
	}
	if (menu == 2)
	{
		// 006226c0: modo 2 (flag a47 = 00605af0(8)) ou 4; Card Holic some; aba +0x554 = 0
		SetViewMode983(m_viewFlags983[3] ? 2 : 4);
		ShowCardHolic983(false);
		m_cardTab983 = 0;
		return;
	}
	if (menu != 0)
		return;
	if (m_viewFlags983[0]) // 00605af0(1)
	{
		SetViewMode983(2);
		ShowCardHolic983(false);
		return;
	}
	SetViewMode983(1);
	if (m_viewA74983 == 1)
		ShowCardHolic983(true);
}

void CRealMyRoomForm983::ShowCardHolic983(bool show)
{
	CCardBook* book = CardBook983();
	if (book)
	{
		book->Show(show);
		if (show)
			for (int i = 3; i < 7; ++i)
				if (book->m_pTabButton[i])
					book->m_pTabButton[i]->SetVisible(true);
	}
	// C187.1: caderno (espiral/papel/"Pangya. Card Book"): card_book_back_bg, escondido no init (B411), so com o Card Holic aberto
	{
		FrWnd* back = FindChildByName("card_book_back_bg");
		if (back)
			back->SetVisible(show);
	}
	FrListBox* pack = m_pCardPackList983;
	if (pack)
	{
		pack->SetVisible(show);
		if (show)
			BuildCardPackList983();
	}
}

// aba Pack: cartas/pacotes do doc (tipos 3 e 4), completando ate 6 casas (645 BuildCardPackList)
void CRealMyRoomForm983::BuildCardPackList983()
{
	FrListBox* pack = m_pCardPackList983;
	if (!pack)
		return;
	pack->ClearItem();
	std::list<sCards>& cards = Doc()->m_cardList;
	for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		unsigned int type = (it->tid >> 22) & 0xf;
		if (type == 4 || type == 3)
			pack->AddItem(&(*it));
	}
	if (cards.size() & 1)
		pack->AddItem(NULL);
	for (int n = pack->GetCurrentItemSize(true); n < 6; ++n)
		pack->AddItem(NULL);
	pack->UseRightButton(true);
	pack->UseDummy(true);
}

void CRealMyRoomForm983::OnCardTabDown983(int tab)
{
	CCardBook* book = CardBook983();
	if (book)
	{
		book->ButtonTabControl((unsigned char)tab);
		book->ShowSlotControl(true);
	}
	FrListBox* pack = m_pCardPackList983;
	if (pack)
	{
		pack->SetVisible(tab == 0);
		if (tab == 0)
			pack->UnselectAllItem();
	}
	if (tab != 0)
		MyRoomPetClear983((char*)this); // 00605810..00605950: 00605530(0)
}

// cardpacklist OWNERDRAW (645 OnCardPackListDraw, mesmos bitmaps do item_list)
void CRealMyRoomForm983::OnCardPackListDraw983(int param)
{
	FrListBox* pack = m_pCardPackList983;
	if (!pack || !param)
		return;
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	pGI->SetTextStyle(0);
	const char* bn;
	if (!pItem->pData)
		bn = "item_base_empty";
	else if (pItem->selected)
	{
		bn = "item_base_d";
		pGI->SetTextColor(0xffff0000, 0xffffffff);
	}
	else
	{
		pGI->SetTextColor(0xff000000, 0xffffffff);
		bn = pItem->underCursor ? "item_base_o" : "item_base_n";
	}
	const Bitmap* pBitmap = g_pFresh->GetBitmap(bn);
	if (pBitmap)
	{
		WRect dst(pItem->pos.x, pItem->pos.y, (float)pack->GetItemWidth(), (float)pack->GetItemHeight());
		// 0060ff10: src = bitmap inteiro (Width/Height do Bitmap), esticado na casa -> margem entre as casas como no 983 (foto 45)
		WRect src(0.0f, 0.0f, (float)(unsigned)pBitmap->Width(), (float)(unsigned)pBitmap->Height());
		pGI->DrawTexture(pBitmap, src, dst, 0xffffffff, 0);
	}
	sCards* pCard = (sCards*)pItem->pData;
	if (!pCard)
		return;
	const char* name = K2L_Compatibility("\xbe\xcb \xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb");
	const char* icon = "caddie_no";
	IFF_STRUCT::sCard* pInfo = ItemManager()->FindCard(pCard->tid);
	if (pInfo)
	{
		icon = pInfo->c.Icon;
		name = pInfo->c.Name;
	}
	pBitmap = g_pFresh->GetBitmap(icon);
	float w = (float)pack->GetItemWidth();
	if (pBitmap)
	{
		WRect rect(pItem->pos.x + 12.0f, pItem->pos.y + 12.0f, (float)pBitmap->Width(), (float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	WPoint pt(w - 12.0f + pItem->pos.x, pItem->pos.y + 12.0f);
	pGI->Print11(pt, 2, "%s", name);
	int h = pGI->GetFontHeight11();
	pt = WPoint(w - 12.0f + pItem->pos.x, (float)h + 17.0f + pItem->pos.y);
	if ((pCard->tid & 0x3c00000) == 0x1000000)
		pGI->Print11(pt, 2, K2L_Compatibility("\xb3\xb2\xc0\xba \xc4\xab\xb5\xe5 \xc6\xd1 %d\xb0\xb3"), (int)pCard->cardCount);
	else
		pGI->Print11(pt, 2, K2L_Compatibility("%d\xb0\xb3"), (int)pCard->cardCount);
	if (CardBook983() && pItem == pack->GetSelected())
		CardBook983()->DrawCardPackHelp(pCard->tid);
}

// A685 (item 10 do dono, efeitos no My Room): fim do 0061a810 no My Room (00e78cac != 0): +0x50 da previa = 0
// (ResetCameraRecoverTime), CExhibition::AttachFx (00873a30: refaz os fx do frame -> brilho do anel/brinco/asa) e
// CPetFrame::Refresh (008b1030) com a lista do doc (0062b750). Sem isso a previa ficava com os fx das pecas de antes.
static void MyRoomPreviewFx983(char* self, CSharedDoc* doc)
{
	if (!g_lobbyModal983)
		return;
	CExhibition* pv = ((CRealMyRoomForm983*)self)->m_pPreview983;
	if (!pv)
		return;
	pv->ResetCameraRecoverTime();
	pv->AttachFx();
	if (pv->GetPetFrame())
		pv->GetPetFrame()->Refresh(&doc->m_partTidList);
}

// 0061f8b0 (equipar) / 0061dfb0 (tirar), casos 2-4 e 0x1b: manda 0x18a ao "LobbyAvatar" (lista do doc, &tid, &uid, &ret, &equip);
// com sucesso grava as pecas no personagem da conta e em +0x490/+0x494. Recusas com aviso (0x23): grupo A com maneiras ruins,
// nivel, CanSetPart e falha da troca (D980).
static void EquipNotice983(const char* text)
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)text, 0, 0, 0, 0));
}

void CRealMyRoomForm983::ItemEquip983(FrListBox* box, FrListItem* item, bool equip)
{
	char* self = (char*)this;
	CSharedDoc* doc = Doc();
	CPartTidList* list = &doc->m_partTidList;
	sItemInfo* info = (sItemInfo*)item->pData;
	if (!info)
		return;
	if (m_subTab983 == 0x22)
	{
		// 0061f8b0/0061dfb0 caso 0x22: equipar = 0063f4b0 (liga +0x124 se mudou), tirar = 0063f620; depois limpa a selecao (00befe70)
		// e remarca os cut-ins equipados (0063f450)
		g_audio->PlaySfx("ui_desktop_icon_click");
		bool ok = equip ? CutinEquip983(doc, info->id, info->typeId) : CutinUnequip983(doc, info->id);
		if (ok)
			m_cutinDirty983 = 1;
		box->UnselectAllItem(); // 00befe70
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			if ((*li)->pData && CutinEquipped983(doc, ((sItemInfo*)(*li)->pData)->id))
				box->SelectItem(*li, false);
		if (CutinEquipped983(doc, info->id))
			box->SelectItem(item, true);
		return;
	}
	int sub = m_subTab983;
	// 0061fa90: roupa de aluguel vencida nao equipa; desfaz a selecao e abre a janela de renovar/apagar
	if (equip && (sub == 2 || sub == 3 || sub == 4 || sub == 5 || sub == 0x16 || sub == 0x1b)
		&& (info->typeId & 0xfc000000) != 0x70000000 && ItemManager()->FindPart(info->typeId) != NULL
		&& info->ItemType == 6 && info->Expired)
	{
		box->ToggleItem(item);
		S5::FrRentalExtensionDlg* pDlg = CreateForm<S5::FrRentalExtensionDlg>(g_pFresh->GetManager(), this, "FrRentalExtensionDlg", NULL);
		pDlg->Initialize(info);
		pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnRentalExtensionResult983, 3);
		return;
	}
	if ((sub == 5 || sub == 0x16) && (info->typeId & 0xfc000000) == 0x70000000)
	{
		// A679 (dono9 aneis): 0061f8b0/0061dfb0 casos 5/0x16 com tid 0x70 (peca auxiliar: anel...): slot (tid >> 21) & 0x1f do
		// tidAuxParts do personagem (+0x4f4 aponta para o doc -> 0x0020 sub 0 ao sair de Clothes); fora do My Room "LobbyAvatar"
		// 0x18b (&tid, 1 = equipar), senao CPetFrame::SetAuxPart (008acd40) na previa (+0x584); equipar desmarca o anel antigo do
		// slot (00beea90); depois 0061a810. Antes o clique em 0x70 voltava sem fazer nada: o anel nunca ia ao servidor.
		unsigned long* aux = m_pSelCharAux983;
		if (!aux)
			return;
		unsigned char slot = (unsigned char)((info->typeId >> 21) & 0x1f);
		unsigned long old = aux[slot];
		aux[slot] = equip ? info->typeId : 0;
		if (!g_lobbyModal983)
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x18b, (int)&info->typeId, equip ? 1 : 0, 0, 0, 0);
		else if (CExhibition* pv = m_pPreview983)
		{
			if (pv->GetPetFrame())
				MyRoomPetFrameSetAuxPart983(pv->GetPetFrame(), slot, equip ? info->typeId : 0);
		}
		if (equip && old)
			for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			{
				sItemInfo* o = (sItemInfo*)(*li)->pData;
				if (o && (*li)->selected && *li != item && o->typeId == old)
					box->UnselectItem(*li);
			}
		jp983_rastro("A679 aux tid=%08x slot=%d equip=%d old=%08x modal=%d", info->typeId, slot, (int)equip, old, g_lobbyModal983);
		doc->BuildMyPartTidList();
		UpdateStats983(self);
		MyRoomPreviewFx983(self, doc);
		return;
	}
	if (equip)
	{
		const IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(info->typeId);
		if (!pPart)
			return; // 983 0061fa7f
		// 983 0061fb13: peca do grupo A com taxa de maneiras ruim
		if (CreateCharacterOption::Instance()->IsPartGroupA983(info->typeId) && !IsGoodMannerRate983())
		{
			if (item->selected)
				box->ToggleItem(item);
			EquipNotice983(K2L_Compatibility("\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xc0\xbd"));
			return;
		}
		// 983 0061fbc9: nivel do item (bit 0x80 = "ou abaixo")
		unsigned char reqLevel = pPart->c.Level & 0x7f;
		bool under = (pPart->c.Level & 0x80) != 0;
		if ((!under && reqLevel > doc->m_myInfo.stat.Level) || (under && reqLevel < doc->m_myInfo.stat.Level))
		{
			if (item->selected)
				box->ToggleItem(item);
			EquipNotice983(K2L_Compatibility(under
				? "\xc0\xcc\xb9\xcc \xc2\xf8\xbf\xeb\xb0\xa1\xb4\xc9\xc7\xd1 \xb7\xb9\xba\xa7\xc0\xbb \xb3\xd1\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xc7\xa5\xbd\xc3\xb5\xc8 \xb7\xb9\xba\xa7 \xc0\xcc\xc7\xcf\xb8\xb8 \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."
				: "\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc2\xf8\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."));
			return;
		}
		if (!list->CanSetPart(info->typeId))
		{
			jp983_rastro("A294 equip CanSetPart=0 tid=%08x", info->typeId);
			box->UnselectItem(item); // 983 0061fcbf: 00beea90 (SelectItem(x, false) MARCA o item)
			// 983 0061fcdb: com "vestir com" (+0x1f8) diz qual peca falta
			if (pPart->EquippableWith[0] != 0)
				EquipNotice983(MakeStr(K2L_Compatibility("\xba\xbb \xbb\xf3\xc7\xb0\xc0\xba \\c0xffff0000\\c%s\xc0\xbb(\xb8\xa6) \xc0\xd4\xc0\xba \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb8\xb8\\c0xff000000\\c \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."), pPart->EquippableWith));
			else
				EquipNotice983(K2L_Compatibility("\\c0xffff0000\\c\xbb\xf3\xc7\xb0\xc0\xbb \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
		g_audio->PlaySfx("item_select_cloth");
		unsigned long mask = list->GetOverlapped(info->typeId);
		if (mask)
		{
			for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			{
				sItemInfo* o = (sItemInfo*)(*li)->pData;
				if (o && (*li)->selected && *li != item && list->IsEquipped(o->typeId, o->id) && (mask & list->GetOverlapped(o->typeId)))
				{
					box->UnselectItem(*li); // 983 0061fec6: 00beea90 (B1300: SelectItem(x, false) deixava o brinco antigo marcado)
					ItemEquip983(box, *li, false);
				}
			}
		}
	}
	else if (!list->IsEquipped(info->typeId, info->id))
	{
		box->SelectItem(item, true);
		return;
	}
	int ret = 0;
	bool bEquip = equip;
	CExhibition* pv = m_pPreview983;
	if (g_lobbyModal983)
	{
		// My Room (00e78cac != 0): o 983 nao manda a 0x18a; troca as pecas no frame da previa (+0x584) e refresca (0061a810)
		if (pv && pv->GetPetFrame())
		{
			ret = MyRoomPetFrameSetParts983(pv->GetPetFrame(), list, info->typeId, info->id);
			pv->GetPetFrame()->Refresh(list); // 008b1030
		}
	}
	else
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x18a, (int)list, (int)&info->typeId, (int)&info->id, (int)&ret, (int)&bEquip);
	jp983_rastro("D296 equip 0x18a tid=%08x equip=%d ret=%d modal=%d pet584=%08x", info->typeId, (int)equip, ret, g_lobbyModal983, (unsigned int)m_pPreview983);
	if (equip && !ret)
		EquipNotice983(K2L_Compatibility("\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9")); // 983 00620092
	if (ret)
	{
		std::map<unsigned int, sCharacterInfo>::iterator ci = doc->m_charMap.find(doc->m_myInfo.userEquip.guidChar);
		if (ci != doc->m_charMap.end())
			for (int j = 0; j < list->m_partNum && j < 24; ++j)
			{
				ci->second.tidParts[j] = list->m_tid[j];
				ci->second.ItemIdList[j] = list->m_uid[j];
			}
		m_pSelCharParts983 = list->m_tid;
	}
	memcpy(m_selCharItemIds983, list->m_uid, 24 * sizeof(unsigned long));
	// C231.1: trocar a peca (chapeu) deixava as duas vermelhas; a marca da lista segue o que esta equipado (IsEquipped)
	for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
	{
		sItemInfo* o = (sItemInfo*)(*li)->pData;
		if (!o)
			continue;
		bool want = list->IsEquipped(o->typeId, o->id);
		if (((*li)->selected != 0) != want)
		{
			if (want)
				box->SelectItem(*li, false);
			else
				box->UnselectItem(*li); // B1300: SelectItem(x, false) marca; tirar a marca = UnselectItem (983 00beea90)
		}
	}
	UpdateStats983(self);
	MyRoomPreviewFx983(self, doc); // 0061a810 + 00620146 (o 983 chama o AttachFx de novo depois de copiar +0x494)
}

// 00613d50 (aba Active, equipar): poe o tid na 1a casa livre dos slots doc+0xa36 (n = 8 + casas do mascote [+ cartas]; 10 sem
// NT_SC_ITEMSLOT); 1800002d/2f nao repetem (msg 0x23); se todas as copias ja estao nos slots, o item sai da lista. Sem pacote (o 0x20/2 sai no fechar).
void CRealMyRoomForm983::EquipActiveItem983(sItemInfo* info)
{
	CSharedDoc* doc = Doc();
	unsigned long* slots = doc->m_myInfo.userEquip.tidItemSlot;
	int n = 10;
	if (IsLocalContent(S4_NT_SC_ITEMSLOT))
		n = doc->GetNumAddItemSlotByMascot() + 8;
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(0xff);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
		int ps = CCardManager::Instance()->GetCardPeriodSlot();
		if (ps < 1)
		{
			if (doc->GetNumAddItemSlotByMascot() == 1)
				slots[9] = 0;
			else
				slots[8] = slots[9] = 0;
		}
		else
			n += ps;
	}
	int i = 0;
	while (i < n && i < 10 && slots[i] != 0)
		++i;
	// 613e15 cheios e 00632d00 repetido: aviso 0x23 ao ator principal (texto 00d1462c)
	bool repeated = false;
	if (info->typeId == 0x1800002d || info->typeId == 0x1800002f)
		for (int k = 0; k < 10; ++k)
			repeated |= slots[k] == info->typeId;
	if (i >= n || i >= 10 || repeated)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)K2L_Compatibility("\xb4\xf5 \xc0\xcc\xbb\xf3 \xc0\xe5\xc2\xf8\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"), 0, 0, 0, 0));
		return;
	}
	slots[i] = info->typeId;
	int used = 0;
	for (int k = 0; k < 10; ++k)
		if (slots[k] == info->typeId)
			++used;
	if (info->Common[0] <= used)
	{
		MyRoomPetClear983((char*)this); // 00605530(1)
		FrListBox* box = m_pItemList983;
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			if ((*li)->pData && ((sItemInfo*)(*li)->pData)->typeId == info->typeId) // 613f3b: compara o tid
			{
				box->_DelItem(li);
				break;
			}
		box->SortItem(EmptyCompares);
	}
}

// item_list DBLCLICK (00614be0): na aba Active (sub 9) equipa o item sob o cursor
void CRealMyRoomForm983::OnItemListDClick983()
{
	if (m_subTab983 != 9)
		return;
	FrListBox* box = m_pItemList983;
	FrListItem* item = box ? box->GetItemUnderCursor() : NULL;
	if (item && item->pData)
		EquipActiveItem983((sItemInfo*)item->pData);
}

static unsigned long s_fortuneTid983;
// 00aa85e0: tipos de caixa abriveis pelo botao direito (230 tids)
static bool IsFortuneBox983(unsigned long tid)
{
	static const unsigned long k[] = { 0x1a000004,0x1a00001b,0x1a000039,0x1a00004d,0x1a000054,0x1a0000b5,0x1a0000bb,0x1a0000d4,0x1a0000d5,0x1a0000e5,0x1a0000e6,0x1a0000ec,0x1a0000fd,0x1a00010f,0x1a000116,0x1a000118,0x1a000138,0x1a00013e,0x1a00013f,0x1a000148,0x1a000149,0x1a00014c,0x1a000165,0x1a00016a,0x1a000185,0x1a00018a,0x1a00018b,0x1a00018c,0x1a000195,0x1a000196,0x1a000197,0x1a000198,0x1a000199,0x1a00019a,0x1a00019b,0x1a00019c,0x1a00019d,0x1a00019e,0x1a0001a2,0x1a0001cf,0x1a0001d6,0x1a000212,0x1a00021d,0x1a00021e,0x1a00021f,0x1a000220,0x1a00023c,0x1a000240,0x1a000242,0x1a000248,0x1a000249,0x1a00024a,0x1a000263,0x1a000271,0x1a00027b,0x1a00027c,0x1a00027d,0x1a00027e,0x1a00028d,0x1a000294,0x1a00029c,0x1a00029d,0x1a0002a1,0x1a0002a3,0x1a0002a5,0x1a0002aa,0x1a0002b4,0x1a0002cc,0x1a0002cd,0x1a0002ce,0x1a0002cf,0x1a0002d0,0x1a0002d1,0x1a0002d2,0x1a0002d3,0x1a0002d4,0x1a0002d5,0x1a0002d6,0x1a0002d8,0x1a0002da,0x1a0002db,0x1a0002dc,0x1a0002dd,0x1a0002de,0x1a0002df,0x1a0002e0,0x1a0002e1,0x1a0002e2,0x1a0002e3,0x1a0002e4,0x1a0002e5,0x1a0002ee,0x1a0002ef,0x1a0002fa,0x1a0002fb,0x1a0002fc,0x1a000309,0x1a00030b,0x1a00030d,0x1a00030e,0x1a00030f,0x1a000310,0x1a000311,0x1a000312,0x1a000313,0x1a000317,0x1a000318,0x1a00031b,0x1a00031c,0x1a00031d,0x1a00031e,0x1a00031f,0x1a000320,0x1a00032d,0x1a00032e,0x1a00032f,0x1a000330,0x1a000331,0x1a000332,0x1a000333,0x1a000334,0x1a000335,0x1a000336,0x1a000337,0x1a000339,0x1a00033a,0x1a00033b,0x1a00033c,0x1a00033d,0x1a00033e,0x1a00033f,0x1a000340,0x1a000341,0x1a000342,0x1a000343,0x1a000349,0x1a00034c,0x1a00034d,0x1a00034f,0x1a000350,0x1a000351,0x1a000355,0x1a000356,0x1a00035a,0x1a00035c,0x1a00035d,0x1a00035e,0x1a00035f,0x1a000360,0x1a000361,0x1a000363,0x1a000364,0x1a000366,0x1a000367,0x1a000369,0x1a00036a,0x1a00036b,0x1a00036e,0x1a00036f,0x1a000370,0x1a000371,0x1a000372,0x1a000373,0x1a000374,0x1a000375,0x1a000376,0x1a000377,0x1a000378,0x1a000379,0x1a00037a,0x1a00037b,0x1a00037c,0x1a00037d,0x1a00037e,0x1a00037f,0x1a000380,0x1a000381,0x1a000382,0x1a000383,0x1a000384,0x1a000385,0x1a000386,0x1a000387,0x1a000388,0x1a000389,0x1a00038a,0x1a00038c,0x1a00038d,0x1a00038f,0x1a000391,0x1a000397,0x1a000398,0x1a00039c,0x1a00039d,0x1a0003a2,0x1a0003a3,0x1a0003a5,0x1a0003a6,0x1a0003b5,0x1a0003b8,0x1a0003ba,0x1a0003bb,0x1a0003bc,0x1a0003bd,0x1a0003be,0x1a0003c0,0x1a0003c3,0x1a0003c4,0x1a0003c5,0x1a0003c6,0x1a0003c7,0x1a0003c8,0x1a0003c9,0x1a0003ca,0x1a0003cb,0x1a0003cc,0x1a0003cd,0x1a0003ce,0x1a0003cf,0x1a0003d0,0x1a0003d1,0x1a0003d2,0x1a0003d3,0x1a0003d4,0x1a0003d5,0x1a0003d6,0x1a0003d7,0x1a0003d8,0x1a0003d9,0x1a0003da };
	for (size_t i = 0; i < sizeof(k) / sizeof(k[0]); ++i)
		if (k[i] == tid)
			return true;
	return false;
}

// item_list LBUTTONDOWN (620e00, so abas de roupa 2-4/0x1b): o FrListBox ja alternou a selecao; selecionado = equipa, senao tira
// 00622330 (parcial, D290): botao direito na grade abre a janela "information" do item sob o cursor (caso padrao do
// 0061a8a0). B846: porta a selecao (00befeb0), abas 0x17/0x22, UCC e o 61f8b0 (via OnItemListDown983).
void CRealMyRoomForm983::OnItemListRDown983()
{
	char* self = (char*)this;
	FrListBox* box = m_pItemList983;
	FrListItem* it = box ? box->GetItemUnderCursor() : NULL;
	if (!it || !it->pData)
		return;
	int sub = m_subTab983;
	// 00622330: (004d5200 = CAnotherHand::IsWorking nao existe aqui, previa sincrona.) Com +0xa84, na aba 0x20 ou nas abas 2-5/0x16/0x1b
	// com item de periodo (ItemType 6) vencido: alterna a selecao (00befeb0) e abre a janela de informacao, sem equipar
	bool onlyInfo = m_storageOpen983 != 0 || sub == 0x20;
	if (sub == 2 || sub == 3 || sub == 4 || sub == 5 || sub == 0x16 || sub == 0x1b)
	{
		sItemInfo* flag = (sItemInfo*)it->pData;
		if (flag->ItemType == 6 && flag->Expired)
			onlyInfo = true;
	}
	if (onlyInfo)
	{
		box->ToggleItem(it);
		OpenInformation983(it);
		return;
	}
	// 00622330, sub 10 (Passive/Item): caixa (00aa85e0) abre "fortune_new" ("Do you want open it?"); o resultado 1 manda 0xb2 (0060fa40)
	if (sub == 10 && IsFortuneBox983(((sItemInfo*)it->pData)->typeId))
	{
		FrFortuneDlg*& dlg = *(FrFortuneDlg**)&m_pFortuneDlg983; // +0x20c
		if (!dlg)
			dlg = CreateForm<FrFortuneDlg>(g_pFresh->GetManager(), this, "fortune_new", NULL);
		if (!dlg)
			return;
		s_fortuneTid983 = ((sItemInfo*)it->pData)->typeId;
		// 00aa78a0/00aa7860: ret=portrait(+0x110)->SetBgImg(FindCommonItem(tid)->Icon); FrFortuneDlg liga "fortune_img"
		IFF_ITEM_COMMON* fcommon = ItemManager()->FindCommonItem(s_fortuneTid983);
		if (fcommon)
			dlg->SetBox983(s_fortuneTid983, fcommon->Icon, fcommon->Name);
		IFF_STRUCT::sDesc* desc = ItemManager()->FindDesc(s_fortuneTid983);
		dlg->SetMessage(desc ? desc->Desc : "Do you want open it?", false);
		dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnFortuneResult983, 3);
	}
	else
		OpenInformation983(it);
	// 00622330 depois da janela: aba 0x17 so alterna; aba 0x22 limpa a selecao e remarca o cut-in clicado se equipado
	if (sub == 0x17)
	{
		box->ToggleItem(it);
		return;
	}
	if (sub == 0x22)
	{
		CSharedDoc* doc = Doc();
		box->UnselectAllItem();
		for (FrListBox::ITEM_LIST::iterator li = box->m_itemList.begin(); li != box->m_itemList.end(); ++li)
			if ((*li)->pData && CutinEquipped983(doc, ((sItemInfo*)(*li)->pData)->id))
				box->SelectItem(*li, false);
		if (CutinEquipped983(doc, ((sItemInfo*)it->pData)->id))
			box->SelectItem(it, true);
		return;
	}
	// UCC (007e9580): roupa UCC sem desenho baixado so alterna a selecao
	sItemInfo* info = (sItemInfo*)it->pData;
	if (IsLocalContent(S4_UCC) && IsUccClothes(info->typeId)) // 983 00622534
	{
		if (!(info->status & 1) || !UccManager()->IsDownloadComplete(info->typeId, info->UccIndex))
		{
			box->ToggleItem(it);
			return;
		}
	}
	// item ja selecionado = 0061f8b0 (mesmo caminho do clique esquerdo; o 983 so pre-visualiza nas abas de caixa); senao so seleciona
	if (!it->selected)
	{
		box->ToggleItem(it);
		return;
	}
	if (sub == 9 || sub == 10)
	{
		MyRoomPreview983(self, info->typeId, false);
		g_audio->PlaySfx("ui_desktop_icon_click");
		return;
	}
	// D602: 0061f8b0 caso 0xd (00620639) = 0061cb70(chave) sempre equipa; o OnItemListDown983 (clique esquerdo, B529) desequipa
	// o mascote ja equipado, e o direito nele tirava o mascote
	if (sub == 0xd)
	{
		SelectMascot983(*(unsigned long*)it->pData);
		g_audio->PlaySfx("ui_desktop_icon_click");
		return;
	}
	OnItemListDown983();
}

// 00611110: 3 = renovar (0xe6), 4 = apagar (0xe7), com msg 1 ao ator principal antes
bool CRealMyRoomForm983::OnRentalExtensionResult983(int result, FrForm* form)
{
	S5::FrRentalExtensionDlg* pDlg = DYNAMIC_CAST(S5::FrRentalExtensionDlg, form);
	if (pDlg != NULL && (result == 3 || result == 4))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)(result == 3 ? 0xe6 : 0xe7));
		packet.Encode4(pDlg->GetItemInfo()->id);
		packet.Send(TO_GAME);
	}
	return true;
}

bool CRealMyRoomForm983::OnFortuneResult983(int result, FrForm* form)
{
	if (result == 1)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0xb2);
		packet.Encode4(s_fortuneTid983);
		packet.Send(TO_GAME);
	}
	m_pFortuneDlg983 = NULL;
	return true;
}

// 0061ed70: fecha a janela dos premios e refaz a sub-aba 10 (Passive/Item), que perdeu a bolsa
bool CRealMyRoomForm983::OnFortuneOpenResult983(int result, FrForm* form)
{
	m_pFortuneOpenDlg983 = NULL;
	m_subTabDirty983 = true;
	SelectSubTab983(10, false);
	return true;
}

// 00620e00 caso 0x18: um dialogo por vez (+0x214)
void CRealMyRoomForm983::OpenReplayDlg983(sRecordedItemInfo* pReplay, const char* form, int mode, FRESH_PFN_RESULT pfn)
{
	if (m_pReplayDlg983 != NULL)
		return;
	m_pReplayDlg983 = CreateForm<FrReplayModeDlg>(g_pFresh->GetManager(), this, form, NULL);
	Doc()->SetFileName(pReplay->fileName);
	m_pReplayDlg983->SetReplayMode(mode);
	m_pReplayDlg983->Open(pfn, 1);
}

bool CRealMyRoomForm983::OnReplayPlayResult983(int result, FrForm* form)
{
	m_pReplayDlg983 = NULL;
	if (result == 1)
	{
		memset(&Doc()->m_roomInfo, 0, sizeof(sRoomInfo));
		Doc()->m_playingMode = (eReplayModeType)2;
		Doc()->m_golfGame.gameType = 0;
		strcpy(Doc()->m_golfGame.gameTypeName, Doc()->m_gameTypeInfo[0].name);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
	}
	return true;
}

bool CRealMyRoomForm983::OnReplayDeleteResult983(int result, FrForm* form)
{
	m_pReplayDlg983 = NULL;
	if (result == 1)
		FillItemList983(0x18);
	return true;
}

// 00611fa0: o mesmo Tiki Report so de novo depois de responder; senao pergunta e 0060f5c0 pede ao servidor
void CRealMyRoomForm983::OpenTikiReport983(sItemInfo* pInfo)
{
	if (!IsLocalContent(S3_CADDIE_REPORT) || pInfo == NULL)
		return;
	if (pInfo->id == m_tikiGuid983)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc0\xaf\xc8\xbf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae \xc0\xd4\xb4\xcf\xb4\xd9. "),
			0, 0, 0, 0));
		return;
	}
	m_tikiGuid983 = pInfo->id;
	m_tikiTid983 = pInfo->typeId;
	m_tikiParam983 = pInfo->Common[1] * 0x8000 + pInfo->Common[2];
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xb1\xe2\xb7\xcf\xc0\xce \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xbf\xad\xb6\xf7 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnOpenTikiReport983, 1);
}

// 0060f5c0: sim = 0xab {guid, param} e tela de espera (0x1); nao = esquece o guid
bool CRealMyRoomForm983::OnOpenTikiReport983(int result, FrForm* form)
{
	if (!IsLocalContent(S3_CADDIE_REPORT))
		return false;
	if (result == 1)
	{
		if (m_tikiGuid983 && m_tikiParam983)
		{
			WSendPacket packet((enumClientPacket)0xab);
			packet.Encode4(m_tikiGuid983);
			packet.Encode4(m_tikiParam983);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
	}
	else
		m_tikiGuid983 = 0;
	m_tikiParam983 = 0;
	return true;
}

// 00605d30
bool CRealMyRoomForm983::OnTikiReportResult983(int result, FrForm* form)
{
	m_pUniteResultDlg983 = NULL;
	return true;
}

// 00613b00: sem "caddie_info" aberta, cria a janela (modal 3), SetCaddieInfo (a9a6c0) e desabilita o widget +0x1ac
void CRealMyRoomForm983::OpenCaddieInfo983(const sCaddieInfo& info)
{
	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName("caddie_info"))
		return;
	FrCaddieInfoDlg* dlg = CreateForm<FrCaddieInfoDlg>(g_pFresh->GetManager(), this, "caddie_info", NULL);
	if (!dlg)
		return;
	dlg->SetCaddieInfo(info);
	dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnCaddieInfoResult983, 3);
	FrWnd* w = m_pCaddieInfoBtn983;
	if (w)
		w->Enable(false);
}

// 00613a50 (= OnCaddieInfoDlgResult do 645): resultado 2 (contratar) reabilita +0x1ac, salvo se o nivel do caddie > o do jogador
// 00605ea0: mov [ecx+0x240], 0; return true
// 00605580: mov [ecx+0x204], 0; return true
bool CRealMyRoomForm983::OnMascotInfoResult983(int result, FrForm* form)
{
	m_pMascotInfoDlg983 = NULL;
	return true;
}

bool CRealMyRoomForm983::OnClubInfoResult983(int result, FrForm* form)
{
	m_pClubInfoDlg983 = NULL;
	return true;
}

bool CRealMyRoomForm983::OnCaddieInfoResult983(int result, FrForm* form)
{
	FrWnd* w = m_pCaddieInfoBtn983;
	if (result == 2 && w)
	{
		w->Enable(true);
		if (form)
		{
			sCaddieInfo info = ((FrCaddieInfoDlg*)form)->GetCaddieInfo();
			for (CaddieIterator it = ItemManager()->m_CaddieMap.begin(); it != ItemManager()->m_CaddieMap.end(); ++it)
			{
				if ((*it).second.c.TypeId == info.tid)
				{
					if (((*it).second.c.Level & 0x7f) > Doc()->m_myInfo.stat.Level) // 613a50: (byte)node.Level & 0x7f
						w->Enable(false);
					break;
				}
			}
		}
	}
	return true;
}

// 0061a8a0: grava a posicao do item em +0x440/+0x444 (x-300.0, y-450.0 em double) e abre a janela de informacao da aba
void CRealMyRoomForm983::OpenInformation983(FrListItem* item)
{
	char* self = (char*)this;
	m_infoX983 = (float)(item->pos.x - 300.0);
	m_infoY983 = (float)(item->pos.y - 450.0);
	int sub = m_subTab983;
	// 0061a8a0 aba 0xb (Caddie): "caddie_info" (613b00) com o sCaddieInfo do item
	if (sub == 0xb)
	{
		OpenCaddieInfo983(*(sCaddieInfo*)item->pData);
		return;
	}
	// 0061a8a0 aba 7 (tacos, tid 0x10xxxxxx): "information" = FrClubSetInfoDlg guardada em +0x240 (0060bea0), SetItem983 (00499730)
	if (sub == 7)
	{
		FrClubSetInfoDlg*& dlg = m_pClubInfoDlg983;
		if (!dlg)
			dlg = CreateForm<FrClubSetInfoDlg>(g_pFresh->GetManager(), this, "information", NULL);
		if (dlg)
		{
			dlg->SetItem983((sItemInfo*)item->pData);
			dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnClubInfoResult983, 1);
		}
		return;
	}
	// 0061a8a0 aba 0xd (Mascote): "mascot_info" para os tids 40000000/4/5/8/f, senao "mascot_info_ok"; a81710 = SetMascotInfo(info, +0x584->+0xac);
	// guardada em +0x204; callback 00605580 zera +0x204
	if (sub == 0xd)
	{
		sMascotInfo* mi = (sMascotInfo*)item->pData;
		unsigned long t = mi->tid;
		const char* nm = (t == 0x40000000 || t == 0x40000004 || t == 0x40000005 || t == 0x40000008 || t == 0x4000000f) ? "mascot_info" : "mascot_info_ok";
		FrMascotInfoDlg* dlg = CreateForm<FrMascotInfoDlg>(g_pFresh->GetManager(), this, nm, NULL);
		m_pMascotInfoDlg983 = dlg; // +0x204
		char* pv = (char*)m_pPreview983;
		if (dlg)
		{
			dlg->SetMascotInfo(*mi, pv ? ((CExhibition*)pv)->GetModel() : NULL);
			dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnMascotInfoResult983, 1);
		}
		return;
	}
	if (sub == 0x12)
		return;
	ShowInformation983(((sItemInfo*)item->pData)->typeId, (sItemInfo*)item->pData);
}

// 00606c30: tid na tabela de 35 em d13d80 (mesma de IsSpecialPartTid983)
static bool IsSpecialUccTid983(unsigned long tid)
{
	static const unsigned long tbl[35] = {
		0x08006055, 0x0800a035, 0x08044085, 0x08048025, 0x080440b3, 0x08084044, 0x0808a025, 0x080c408b, 0x080ca023, 0x080c40b2,
		0x0810404f, 0x08108030, 0x08144084, 0x0814a02c, 0x081440bf, 0x08184087, 0x08188020, 0x081840c7, 0x081c404b, 0x081ca01f,
		0x0820404a, 0x0820a00e, 0x08204091, 0x08244064, 0x0824a004, 0x0824408f, 0x08284026, 0x08284027, 0x0828a012, 0x0830401b,
		0x0830401e, 0x0830a00c, 0x0838400e, 0x08384011, 0x0838a006,
	};
	for (int i = 0; i < 35; i++)
		if (tbl[i] == tid)
			return true;
	return false;
}

// 00615010(&typeId, 0, 0): janela "information" do item (tambem pelo botao direito do cardpacklist, 00615c10)
void CRealMyRoomForm983::ShowInformation983(unsigned long typeId, const sItemInfo* info)
{
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
	if (!common)
		return;
	FrDesktop* desk = g_pFresh->GetManager()->GetDesktop();
	if (desk->FindChildByName("information") || desk->FindChildByName("information_setitem"))
		return;
	// 615010: item de conjunto (tid & fc000000 == 24000000) abre "information_setitem" (0060bf70) com SetSetItem (008ed8a0)
	if ((common->TypeId & 0xfc000000) == 0x24000000)
	{
		FrSetItemDlg* setDlg = CreateForm<FrSetItemDlg>(g_pFresh->GetManager(), this, "information_setitem", NULL);
		if (setDlg)
		{
			setDlg->SetSetItem(common->TypeId);
			setDlg->Open(NULL, 3);
		}
		return;
	}
	FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this, "information", NULL);
	if (!form)
		return;
	FrArea* portrait = DYNAMIC_CAST(FrArea, form->FindChildByName("portrait"));
	// 615010 (007e9580 + 007ea620/007ea690): roupa UCC com uid usa o icone e o nome do desenho (FindClothes); o icone so
	// existe com o desenho baixado (+0x60 == 1), senao o retrato fica sem imagem
	sUccClothes* ucc = NULL;
	if (info && info->id != 0 && IsLocalContent(S4_UCC) && IsUccClothes(common->TypeId)) // 983 006151b9
		ucc = UccManager()->FindClothes(info->id);
	if (portrait)
	{
		if (!ucc)
			portrait->SetBgImg(common->Icon);
		else if (ucc->downloadCount == 1 && ucc->icon)
			portrait->SetBgImg(ucc->icon);
	}
	// 615010: "reset" e "icon_clubsetworkshop" (FrArea) comecam escondidos na janela
	const char* hide[2] = { "reset", "icon_clubsetworkshop" };
	for (int h = 0; h < 2; ++h)
	{
		FrArea* ha = DYNAMIC_CAST(FrArea, form->FindChildByName(hide[h]));
		if (ha)
			ha->SetVisible(false);
	}
	FrEdit* name = DYNAMIC_CAST(FrEdit, form->FindChildByName("name"));
	FrArea* level = DYNAMIC_CAST(FrArea, form->FindChildByName("level"));
	if (name && level)
	{
		// 615010: nome do item (strncpy common+8, 0x80) seguido de quebra de linha no campo "name"
		char nm[0x80];
		strncpy(nm, ucc ? ucc->uccName : common->Name, sizeof(nm));
		nm[sizeof(nm) - 1] = 0;
		// 615010: formato coreano "nome: %s\n\n" (d00464) passa por K2L_Compatibility (00b4ec60) e MakeStr (98c3f0). Peca do grupo A
		// (004ce760) com taxa de mau comportamento ruim (!0050f810): d14754 acrescenta "(uso proibido)" em vermelho.
		bool noWear = CreateCharacterOption::Instance()->IsPartGroupA983(common->TypeId) && !IsGoodMannerRate983();
		name->AddText(MakeStr(K2L_Compatibility(noWear ? "\xc0\xcc\xb8\xa7: %s \\c0xffff0000\\c(\xc2\xf8\xbf\xeb\xba\xd2\xb0\xa1)\\c0xff000000\\c\n\n" : "\xc0\xcc\xb8\xa7: %s\n\n"), nm), false, true);
	}
	if (name && level)
	{
		// 615010 apos o nome: linha de nivel (d0042c "nivel: ... acima" / d00448 "... abaixo" pelo bit 0x80 do Level); 613bc0 da tipo/horas/expirado
		// do item (ItemType, HourRemain, Expired): tipo 1 = "periodo de uso: %dd %dh" (d00580); tipos 2..6 = expirado (d005e8), leitura
		// (S3_CADDIE_REPORT + 1a000042, d005c8), "menos de 1h" (d005b0) ou "tempo restante" (d00598). Variante de aluguel (param_3) portada abaixo.
		name->AddText(K2L_Compatibility((common->Level & 0x80) ? "\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf\n" : "\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3\n"), false, true);
	}
	if (level)
		level->SetBgImg(MakeStr("level_%03d", (common->Level & 0x7f) + 1));
	if (name && level && info && info->ItemType != 0)
	{
		int days = info->HourRemain / 24, hours = info->HourRemain % 24;
		name->AddText(K2L_Compatibility("\n"), false, true);
		const char* txt = NULL;
		if (info->ItemType == 1)
			txt = MakeStr(K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, hours);
		else if (info->ItemType <= 6)
		{
			if (info->Expired)
				txt = K2L_Compatibility("\\c0xffff0000\\c\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3\\c0xff000000\\c\xc0\xcc \\c0xffff0000\\c\xb8\xb8\xb7\xe1\\c0xff000000\\c\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n");
			else if (IsLocalContent(S3_CADDIE_REPORT) && common->TypeId == 0x1a000042)
				txt = MakeStr(K2L_Compatibility("\xbf\xad\xb6\xf7 \xb0\xa1\xb4\xc9 \xb1\xe2\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n"), days, hours);
			else if (days == 0 && hours == 0)
				txt = K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n");
			else
				txt = MakeStr(K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n"), days, hours);
		}
		if (txt)
			name->AddText(txt, false, true);
	}
	// 615010 (S4_CARD_SYSTEM): "avility" recebe uma linha azul por atributo da peca (FindPart, 00a17160; Attr/Slot x5, MakeStatusString
	// b2e010) e "static_avility" sobe 3.0 (d66cf8). Aba 0x26 portada (00a19320). Variante de aluguel (local_49a, com param_3 e
	// ItemType != 0 via 613bc0): linha vazia antes dos atributos e "static_avility" desce 25.0 (cfebf8).
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		bool rental = info && info->ItemType != 0; // 615010: local_49a = 1 quando 613bc0 acha ItemType != 0 (param_3 e sempre 0)
		FrEdit* ability = DYNAMIC_CAST(FrEdit, form->FindChildByName("avility"));
		FrStatic* abilityBg = DYNAMIC_CAST(FrStatic, form->FindChildByName("static_avility"));
		if (ability)
		{
			if (abilityBg)
			{
				WRect rc = abilityBg->GetRect();
				rc.y += rental ? 25.0f : -3.0f;
				abilityBg->SetRect(rc);
			}
			if (rental)
				ability->AddText("", false, true);
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(common->TypeId);
			if (part)
			{
				for (int i = 0; i < 5; ++i)
				{
					if (part->Attr[i] != 0 || part->Slot[i] != 0)
						ability->AddText(MakeStr("\\c0xff0000ff\\c%s\\c0xff000000\\c", MakeStatusString(i, part->Attr[i], part->Slot[i])), false, true);
				}
			}
			// 615010: aba 0x26 (this+0x530) acrescenta o texto do artefato (00a19320) ao final de "avility"
			if (m_subTab983 == 0x26)
			{
				const char* art = ItemManager()->FindArtifactDesc983(common->TypeId);
				if (art)
					ability->AddText(art, false, true);
			}
		}
	}
	IFF_STRUCT::sDesc* desc = ItemManager()->FindDesc(common->TypeId);
	if (desc)
	{
		// 615010 (S4_CARD_SYSTEM + 007e9580 + !00606c30 = tid fora da tabela de 35 em d13d80): a descricao de roupa UCC ganha dois avisos
		// em vermelho, cada um precedido de "\n": d14704 (direitos de terceiros) e d14694 (ou d146bc se a peca for categoria 7/9)
		std::string text(desc->Desc);
		if (IsLocalContent(S4_CARD_SYSTEM) && IsUccClothes(common->TypeId) && !IsSpecialUccTid983(common->TypeId))
		{
			static const char* red = "\\c0xffff0000\\c";
			static const char* black = "\\c0xff000000\\c";
			IFF_STRUCT::sPart* up = ItemManager()->FindPart(common->TypeId);
			bool copy = up && (up->Category == 7 || up->Category == 9);
			text += "\n";
			text += red;
			text += K2L_Compatibility("\xc1\xa6\x33\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb");
			text += black;
			text += "\n";
			text += red;
			text += K2L_Compatibility(copy ? "\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce\xc0\xc7 \xba\xb9\xbb\xe7 \xb9\xd7 \xb1\xb8\xc0\xd4\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"
				: "\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0");
			text += black;
		}
		// 615010: so chama SetMessage (vt+0x74) com texto nao vazio
		if (!text.empty())
			form->SetMessage(text.c_str(), false);
	}
	form->Open(NULL, 3);
}

// 983 (00618572 / 0061aae0 default): caixa que se abre pelo botao giftmove_btn = IFF sItem::RandomBox (+0xc0 bit 0)
static bool IsOpenableBox983(unsigned long tid)
{
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);
	return pItem != NULL && pItem->IsRandomBox();
}

// 983 (00618572): tids com o botao giftmove_btn na aba 10 que ja abrem algo no form
static bool HasGiftButton983(unsigned long tid)
{
	switch (tid)
	{
	case 0x1a00003b: // tabela 0061a53c e cmp 0xd6 em 00618544: estes desenham sem olhar o IFF
	case 0x1a00003e:
	case 0x1a000048:
	case 0x1a000052:
	case 0x1a000055:
	case 0x1a0000bc:
	case 0x1a0000d6:
	case 0x1a000105:
	case 0x1a00014b:
	case 0x1a00015b:
	case 0x1a000247:
	case 0x1a00024b:
		return true;
	}
	return IsOpenableBox983(tid);
}

// 983 (0061aae0): Supply Pack (0x105/0x14b) abre o FrSupplyPackDlg (00b35eb0 = Initialize); Key of Fortune (0x15b) manda
// M983(0x276) ao Gateway (CSpecialBoxHandler); 0x3b ano novo (006120c0), 0x52 Natal (00610d20), 0x55/0xbc presente S4 (00612110);
// 0x3e/0x48 nada; o resto (RandomBox) vai ao 00615ca0. Reset do taco 0x247/0x24b (00606cb0 -> 0049bee0): A712
void CRealMyRoomForm983::GiftButtonDown983(sItemInfo* pInfo)
{
	unsigned long tid = pInfo->tid;
	jp983_rastro("A710 GiftButtonDown983 tid=%08x", tid);
	switch (tid)
	{
	case 0x1a000055:
	case 0x1a0000bc:
		OpenS4EventGiftBox983(pInfo);
		return;
	case 0x1a00003b:
		if (IsLocalContent(S3_NEWYEAR_MONEY))
			OpenNewYearMoney983(pInfo);
		return;
	case 0x1a00003e: // 983 0061ab15: 0x3e/0x48 saem do switch sem nada (nem RandomBox nem reset do taco)
	case 0x1a000048:
		return;
	case 0x1a000052:
		if (IsLocalContent((localContentType_t)0x25))
			OpenEventXmasGiftBox983(pInfo);
		return;
	case 0x1a000105:
	case 0x1a00014b:
		if (g_pFresh->GetDesktop()->FindChildForm("FrSupplyPackDlg") == NULL)
		{
			S5::FrSupplyPackDlg* pDlg = CreateForm<S5::FrSupplyPackDlg>(g_pFresh->GetManager(), NULL, "FrSupplyPackDlg", NULL);
			pDlg->Initialize(tid);
			pDlg->EnableDrag(false);
			pDlg->Open(NULL, 0);
		}
		return;
	case 0x1a00015b:
		AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, M983(0x276), 0, 0, 0, 0, 0);
		return;
	}
	if (IsOpenableBox983(tid))
		OpenNewRandomBox983(tid);
	// 983 0061ac8f: reset do taco (00606cb0 = 0x247/0x24b) -> FrClubSetResetWnd com o form como dono (0049a230 ecx=this);
	// os resultados dos dialogos (0060ac10..0060ac70, 00605eb0) so usam o s_pInstance
	if (tid == 0x1a000247 || tid == 0x1a00024b)
	{
		jp983_rastro("A712 ClubSetReset983 tid=%08x inst=%p", tid, FrClubSetResetWnd::s_pInstance);
		if (FrClubSetResetWnd::s_pInstance)
		{
			FrClubSetResetWnd::s_pInstance->Start983(tid, 0);
			return;
		}
		if (FrClubSetResetWnd* pWnd = FrClubSetResetWnd::Create983(this))
			pWnd->Start983(tid, 0);
	}
}

// 983 (006108e0): cut-in 0x1f00000 da aba 10, so com identidade 0x2804
void CRealMyRoomForm983::DeleteCutin983(sItemInfo* pInfo)
{
	if ((pInfo->typeId >> 26) != 6 || (pInfo->typeId & 0x1f00000) != 0x1f00000 || (Doc()->m_myInfo.info.dwIdentity & 0x2804) == 0)
		return;
	FrDeleteItemDlg* dlg = CreateForm<FrDeleteItemDlg>(g_pFresh->GetManager(), this, "delete", NULL);
	dlg->SetDelItem(pInfo->typeId, (short)pInfo->Common[0]);
	dlg->Open(NULL, 1);
}

// 983 (006120c0): guid em +0x558 e o "notify_yesno" (0060cd60) com o resultado em 0060f6f0
void CRealMyRoomForm983::OpenNewYearMoney983(sItemInfo* pInfo)
{
	jp983_rastro("A711 OpenNewYearMoney983 guid=%08x", pInfo->guid);
	m_newYearGuid983 = pInfo->guid;
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(
		K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xad\xbe\xee \xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnOpenNewYearMoneyDlgResult983, 1);
}

// 983 (0060f6f0): "sim" = M983(0x1) ao ator principal e pacote 0x93 com o guid
bool CRealMyRoomForm983::OnOpenNewYearMoneyDlgResult983(int result, FrForm* pForm)
{
	unsigned long& guid = m_newYearGuid983;
	jp983_rastro("A711 OnOpenNewYearMoneyDlgResult983 r=%d guid=%08x", result, guid);
	if (result == 1 && guid != 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0x93);
		packet.Encode4(guid);
		packet.Send(TO_GAME);
	}
	guid = 0;
	return true;
}

// 983 (00610d20): CGiftBoxEvent 0x25; guid em +0x560, tid no evento e "notify_yesno" com o resultado em 00605d60
void CRealMyRoomForm983::OpenEventXmasGiftBox983(sItemInfo* pInfo)
{
	CGiftBoxEvent* pEvent;
	if (!CContentsDoc::Instance()->GetContainer((localContentType_t)0x25, pEvent))
		return;
	jp983_rastro("A711 OpenEventXmasGiftBox983 guid=%08x", pInfo->guid);
	m_giftGuid983 = pInfo->guid;
	pEvent->SetOpenedBoxTID(pInfo->tid);
	if (pEvent->GetOpenedBoxTID() == 0)
		return;
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(
		K2L_Compatibility("\xc5\xa9\xb8\xae\xbd\xba\xb8\xb6\xbd\xba \xbc\xb1\xb9\xb0 \xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnOpenEventXmasGiftDlgResult983, 1);
}

// 983 (00605d60): so fecha
bool CRealMyRoomForm983::OnOpenEventXmasGiftDlgResult983(int result, FrForm* pForm)
{
	return true;
}

// 983 (00612110): CGiftBoxEvent 0x1e; tid em +0x564 e no evento, "notify_yesno" com o resultado em 0060f8f0
void CRealMyRoomForm983::OpenS4EventGiftBox983(sItemInfo* pInfo)
{
	CGiftBoxEvent* pEvent;
	if (!CContentsDoc::Instance()->GetContainer((localContentType_t)0x1e, pEvent))
		return;
	jp983_rastro("A711 OpenS4EventGiftBox983 tid=%08x", pInfo->tid);
	m_openTid983 = pInfo->tid;
	pEvent->SetOpenedBoxTID(pInfo->tid);
	if (pEvent->GetOpenedBoxTID() == 0)
		return;
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(
		K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xad\xbe\xee \xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnOpenS4EventGiftDlgResult983, 1);
}

// 983 (0060f8f0): "sim" = M983(0x1) ao ator principal e pacote 0xb2 com o tid
bool CRealMyRoomForm983::OnOpenS4EventGiftDlgResult983(int result, FrForm* pForm)
{
	unsigned long& tid = m_openTid983;
	jp983_rastro("A711 OnOpenS4EventGiftDlgResult983 r=%d tid=%08x", result, tid);
	if (result == 1 && tid != 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0xb2);
		packet.Encode4(tid);
		packet.Send(TO_GAME);
	}
	tid = 0;
	return true;
}

// 983 (00615ca0): guarda o tid em +0x564 e abre o "notify_yesno" (0060cd60) "Abrir o item selecionado?" com o resultado em 00610e00
void CRealMyRoomForm983::OpenNewRandomBox983(unsigned long tid)
{
	jp983_rastro("A709 OpenNewRandomBox983 tid=%08x", tid);
	m_openTid983 = tid;
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(
		K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xad\xbe\xee \xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnOpenNewRandomBoxDlgResult983, 1);
}

// 983 (00610e00): "sim" abre o FrRandomBoxProgress "newRandomBox" com o tid/icone (0060c960); a barra manda o 0xEF (008c5610)
bool CRealMyRoomForm983::OnOpenNewRandomBoxDlgResult983(int result, FrForm* pForm)
{
	unsigned long& tid = m_openTid983;
	jp983_rastro("A709 OnOpenNewRandomBoxDlgResult983 r=%d tid=%08x", result, tid);
	if (result == 1 && tid != 0)
	{
		FrRandomBoxProgress* pDlg = CreateForm<FrRandomBoxProgress>(g_pFresh->GetManager(), this, "newRandomBox", NULL);
		if (pDlg == NULL)
			return false;
		pDlg->SetBoxTid983(tid);
		pDlg->Open(NULL, 1);
	}
	tid = 0;
	return true;
}

void CRealMyRoomForm983::OnItemListDown983()
{
	int sub = m_subTab983;
	jp983_rastro("A294 OnItemListDown983 sub=%x", sub);
	if (sub == 9)
	{
		// 620e00 case 9: clique no botao equip_btn (x+103,y+66, tamanho do bitmap) equipa (00613d50)
		FrListBox* b9 = m_pItemList983;
		FrListItem* it9 = b9 ? b9->GetItemUnderCursor() : NULL;
		const Bitmap* pEq = g_pFresh->GetBitmap("equip_btn_n");
		if (it9 && it9->pData && pEq != NULL)
		{
			WRect rcEq(it9->pos.x + 103.0f, it9->pos.y + 66.0f, (float)pEq->Width(), (float)pEq->Height());
			if (rcEq.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				EquipActiveItem983((sItemInfo*)it9->pData);
				return;
			}
		}
		// 620e00 case 9 (2o retangulo, delete_btn x+144,y+66): item apagavel e com copias fora dos slots abre o FrDeleteItemDlg (60b590 "delete")
		const Bitmap* pDel = g_pFresh->GetBitmap("delete_btn_n");
		if (it9 && it9->pData && pDel != NULL)
		{
			sItemInfo* di = (sItemInfo*)it9->pData;
			WRect rcDel(it9->pos.x + 144.0f, it9->pos.y + 66.0f, (float)pDel->Width(), (float)pDel->Height());
			IFF_ITEM_COMMON* dc = ItemManager()->FindCommonItem(di->typeId);
			if (rcDel.IsInRect(g_pFresh->GetManager()->GetMousePos()) && dc != NULL && !(*((unsigned char*)dc + 0x80) & 1))
			{
				int used = 0;
				if (!(di->typeId & 0x2000000))
				{
					unsigned long* slots = Doc()->m_myInfo.userEquip.tidItemSlot;
					for (int k = 0; k < 10; ++k)
						if (slots[k] == di->typeId)
							++used;
				}
				int cnt = (int)(short)di->Common[0] - used;
				if (cnt > 0)
				{
					FrDeleteItemDlg* dlg = CreateForm<FrDeleteItemDlg>(g_pFresh->GetManager(), this, "delete", NULL);
					dlg->SetDelItem(di->typeId, cnt);
					dlg->Open(NULL, 1);
				}
			}
		}
	}
	if (sub == 10)
	{
		// 620e00 case 10: 0x2c2 modo 1 ao "ItemBuff" (item de buff so usa o mini botao); clique no retangulo giftmove_btn
		// (x+103,y+66) = 0061aae0; caixa abrivel (RandomBox) -> 00615ca0; delete_btn (x+144) = 006108e0
		FrListBox* b10 = m_pItemList983;
		FrListItem* it10 = b10 ? b10->GetItemUnderCursor() : NULL;
		const Bitmap* pGift = g_pFresh->GetBitmap("giftmove_btn_n");
		char buffHandled = 0;
		if (it10 && it10->pData)
			AfxGetTask()->GetActor("ItemBuff") << MsgObject(NULL, M983(0x246), 1, (int)&buffHandled,
				(int)it10, (int)&g_pFresh->GetManager()->GetMousePos(), ((sItemInfo*)it10->pData)->typeId);
		if (buffHandled)
			it10 = NULL;
		if (it10 && it10->pData && pGift != NULL && HasGiftButton983(((sItemInfo*)it10->pData)->typeId))
		{
			WRect rcGift(it10->pos.x + 103.0f, it10->pos.y + 66.0f, (float)pGift->Width(), (float)pGift->Height());
			if (rcGift.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				GiftButtonDown983((sItemInfo*)it10->pData);
				return;
			}
		}
		const Bitmap* pDel10 = g_pFresh->GetBitmap("delete_btn_n");
		if (it10 && it10->pData && pDel10 != NULL)
		{
			WRect rcDel(it10->pos.x + 144.0f, it10->pos.y + 66.0f, (float)pDel10->Width(), (float)pDel10->Height());
			if (rcDel.IsInRect(g_pFresh->GetManager()->GetMousePos()))
				DeleteCutin983((sItemInfo*)it10->pData);
		}
	}
	if (sub == 0x18 && IsLocalContent(S4_REPLAY_SYSTEM))
	{
		// 620e00 case 0x18: retangulos +0x858 (play, 144,66) e +0x874 (apagar, 103,66)
		FrListItem* it18 = m_pItemList983 ? m_pItemList983->GetItemUnderCursor() : NULL;
		const Bitmap* pPlay = g_pFresh->GetBitmap("replaymode_btn_n");
		const Bitmap* pDelTape = g_pFresh->GetBitmap("replaydel_btn_n");
		if (it18 && it18->pData && pPlay != NULL && pDelTape != NULL)
		{
			sRecordedItemInfo* pTape = (sRecordedItemInfo*)it18->pData;
			const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
			if (WRect(it18->pos.x + 144.0f, it18->pos.y + 66.0f, (float)pPlay->Width(), (float)pPlay->Height()).IsInRect(mouse))
			{
				if (pTape->bValid != 1)
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)K2L_Compatibility("\xb9\xf6\xc1\xaf\xc0\xcc \xb3\xb7\xbe\xc6\xbc\xad \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
				else if (AfxGetTask()->GetActor("AvatarChat") != NULL) // 00621591: com o Lounge aberto nao toca e nem testa o apagar
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)GetGameString(0xea65), 0, 0, 0, 0));
					return;
				}
				else
					OpenReplayDlg983(pTape, "replaymode", 1, (FRESH_PFN_RESULT)&CRealMyRoomForm983::OnReplayPlayResult983);
			}
			if (WRect(it18->pos.x + 103.0f, it18->pos.y + 66.0f, (float)pDelTape->Width(), (float)pDelTape->Height()).IsInRect(mouse))
				OpenReplayDlg983(pTape, "replaytape_del", 2, (FRESH_PFN_RESULT)&CRealMyRoomForm983::OnReplayDeleteResult983);
		}
	}
	if (sub == 0x20)
	{
		// 620e00 case 0x20: o clique nao marca a linha; retangulo +0x8ac = 00615b30. Poster 0x62/0x63 (IFF +0xee) vai a doc+0xa92/0xa96;
		// 00613fc0: desmarca o outro do mesmo tipo e alterna este
		FrListBox* b20 = m_pItemList983;
		FrListItem* it20 = b20 ? b20->GetItemUnderCursor() : NULL;
		const Bitmap* pPoster = g_pFresh->GetBitmap("poster_btn_n");
		if (it20 && it20->pData && pPoster != NULL)
		{
			it20->selected = false;
			unsigned long tid = ((sFurniture_List*)it20->pData)->typeId;
			IFF_STRUCT::sFurniture* pFurn = ItemManager()->FindFurniture(tid);
			C_ASSERT_VC10(offsetof(IFF_STRUCT::sFurniture, IsFunction) == 0xee);
			if (pFurn != NULL && (pFurn->IsFunction == 0x62 || pFurn->IsFunction == 0x63)
				&& WRect(it20->pos.x + 144.0f, it20->pos.y + 66.0f, (float)pPoster->Width(), (float)pPoster->Height()).IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				CSharedDoc* doc = Doc();
				unsigned long& slot = doc->m_myInfo.userEquip.poster[pFurn->IsFunction != 0x62];
				for (FrListBox::ITEM_LIST::iterator it = b20->m_itemList.begin(); it != b20->m_itemList.end(); ++it)
				{
					IFF_STRUCT::sFurniture* pOther = (*it)->selected && (*it)->pData ? ItemManager()->FindFurniture(((sFurniture_List*)(*it)->pData)->typeId) : NULL;
					if (pOther != NULL && pOther->IsFunction == pFurn->IsFunction)
						(*it)->selected = false;
				}
				it20->selected = slot != tid;
				slot = it20->selected ? tid : 0;
				g_audio->PlaySfx("ui_desktop_icon_click");
			}
		}
	}
	if (sub == 0x19)
	{
		// 620e00 case 0x19: retangulo +0x794 no item 0x1a000042 = 00611fa0; segue para a previa
		FrListBox* b19 = m_pItemList983;
		FrListItem* it19 = b19 ? b19->GetItemUnderCursor() : NULL;
		const Bitmap* pRep = g_pFresh->GetBitmap("giftmove_btn_n");
		if (it19 && it19->pData && pRep != NULL && ((sItemInfo*)it19->pData)->typeId == 0x1a000042)
		{
			WRect rcRep(it19->pos.x + 103.0f, it19->pos.y + 66.0f, (float)pRep->Width(), (float)pRep->Height());
			if (rcRep.IsInRect(g_pFresh->GetManager()->GetMousePos()))
				OpenTikiReport983((sItemInfo*)it19->pData);
		}
	}
	if (sub == 7)
	{
		// 0061f8b0 caso 7: SelectClub983 (0061cc80) do uid clicado; ret 0 (nao achou) ou 2 (nivel incompativel, vira o padrao) -> aviso 0x23
		// "Chagyong level-eul chogwa..." (00d14a7c); depois "item_select_club"
		FrListBox* b7 = m_pItemList983;
		FrListItem* it7 = b7 ? b7->GetItemUnderCursor() : NULL;
		if (!it7 || !it7->pData)
			return;
		if (!it7->selected)
		{
			b7->SelectItem(it7, true); // 0061dfb0 default (clique no ja selecionado): remarca; sem nova selecao
			return;
		}
		int r7 = SelectClub983(((sItemInfo*)it7->pData)->id);
		if (r7 == 0 || r7 == 2)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)K2L_Compatibility("\xc2\xf8\xbf\xeb \xb7\xb9\xba\xa7\xc0\xbb \xc3\xca\xb0\xfa\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
		g_audio->PlaySfx("item_select_club");
		return;
	}
	if (sub == 8)
	{
		// 0061f8b0 caso 8: 0061ce00(typeId do item clicado) e sfx item_select_aztec
		FrListBox* b8 = m_pItemList983;
		FrListItem* it8 = b8 ? b8->GetItemUnderCursor() : NULL;
		if (!it8 || !it8->pData)
			return;
		if (!it8->selected)
		{
			b8->SelectItem(it8, true); // 0061dfb0 default: remarca
			return;
		}
		SelectBall983(((sItemInfo*)it8->pData)->typeId);
		g_audio->PlaySfx("item_select_aztec");
		return;
	}
	if (sub == 0xb || sub == 0xd)
	{
		// 0061f8b0 casos 0xb/0xd (006205b5/00620639): caddie = 0061c960(chave), sem troca -> aviso 0x23 de nivel (00d14bc4);
		// mascote = 0061cb70(chave); os dois tocam "ui_desktop_icon_click"
		FrListBox* bk = m_pItemList983;
		FrListItem* ik = bk ? bk->GetItemUnderCursor() : NULL;
		if (!ik || !ik->pData)
			return;
		unsigned long key = *(unsigned long*)ik->pData; // sCaddieInfo/sMascotInfo::guid = chave do mapa
		jp983_rastro("A296 listdown sub=%x key=%08x sel=%d", sub, key, (int)ik->selected);
		// 620e00 case 0xb: retangulo +0x7e8 (reemploy) abre caddie_info (00613b00); a selecao segue normal
		const Bitmap* pRe = g_pFresh->GetBitmap("reemploy_btn_n");
		if (sub == 0xb && pRe != NULL && IsCaddieReemploy983((sCaddieInfo*)ik->pData))
		{
			WRect rcRe(ik->pos.x + 144.0f, ik->pos.y + 66.0f, (float)pRe->Width(), (float)pRe->Height());
			if (rcRe.IsInRect(g_pFresh->GetManager()->GetMousePos()))
				OpenCaddieInfo983(*(sCaddieInfo*)ik->pData);
		}
		// B529 (C240, orig-c238-1): clicar no mascote JA equipado (+0x510 == chave) desequipa no original (X manda sub 08 = 0)
		bool unequipMascot = (sub == 0xd && m_selMascotKey983 == key);
		if (!ik->selected || unequipMascot)
		{
			// 00620e00: item sem selecao (clique no ja equipado) -> 0061dfb0 (tirar) caso 0xb = 0061c960(0) + 0061aab0; caso 0xd = 0061cb70(0)
			if (sub == 0xd)
				SelectMascot983(0);
			else
				SelectCaddie983(0);
			g_audio->PlaySfx("ui_desktop_icon_click");
			return;
		}
		if (sub == 0xd)
			SelectMascot983(key);
		else if (!SelectCaddie983(key))
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc2\xf8\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."), 0, 0, 0, 0));
		g_audio->PlaySfx("ui_desktop_icon_click");
		return;
	}
	// 0061f8b0 casos 9/10/0x13/0x14/0x18/0x19/0x1d-0x1f/0x21/0x26 (fim do 00620e00, item ja selecionado): 00605640 (previa 3D do item)
	// e sfx ui_desktop_icon_click. D431: orig-d430-1/016 (clique no Lucky Bag, Passive) mostra a selecao e a bolsa 3D; o nosso nada.
	if (sub == 9 || sub == 10 || sub == 0x13 || sub == 0x14 || sub == 0x18 || sub == 0x19 || (sub >= 0x1d && sub <= 0x1f) || sub == 0x21 || sub == 0x26)
	{
		FrListBox* pb = m_pItemList983;
		FrListItem* pi = pb ? pb->GetItemUnderCursor() : NULL;
		if (pi && pi->pData && pi->selected)
		{
			MyRoomPreview983((char*)this, ((sItemInfo*)pi->pData)->typeId, false);
			g_audio->PlaySfx("ui_desktop_icon_click");
		}
		else if (pi && pi->pData)
			pb->SelectItem(pi, true); // 0061dfb0 default: remarca, a previa fica
		return;
	}
	// 0061f8b0 casos 5/0x16 (Clothes/All): fora de tid 0x70xxxxxx (set, nao portado) caem no caso 2 (equipar)
	if (sub != 2 && sub != 3 && sub != 4 && sub != 5 && sub != 0x16 && sub != 0x1b && sub != 0x22)
		return;
	FrListBox* box = m_pItemList983;
	if (!box)
		return;
	FrListItem* item = box->GetItemUnderCursor();
	jp983_rastro("A294 listdown sub=%x item=%p data=%p tid=%08x sel=%d", sub, item, item ? item->pData : NULL,
		(item && item->pData) ? ((sItemInfo*)item->pData)->typeId : 0, item ? (int)item->selected : -1);
	if (!item || !item->pData)
		return;
	if ((sub == 0x16 || sub == 0x1b) && IsLocalContent(S4_UCC) && OnUccListBtnDown983(item))
		return;
	ItemEquip983(box, item, item->selected); // A679: tid 0x70 (anel) tratado no ItemEquip983 (0061f8b0/0061dfb0)
}

void CRealMyRoomForm983::ShowUccHiddenCtrls983(bool bShow)
{
	if (m_pMastery983)
		m_pMastery983->SetVisible(bShow);
	if (m_pStatGroup983)
		m_pStatGroup983->SetVisible(bShow);
	if (m_pUpDownGroup983)
		m_pUpDownGroup983->SetVisible(bShow);
	for (int i = 0; i < 5; ++i)
		if ((&m_pEPower983)[i])
			(&m_pEPower983)[i]->SetVisible(bShow);
}

// 00610c00: fecha "ucc_draw": a lista volta; no Lounge a previa do personagem, senao o LobbyAvatar (0x185) e os grupos escondidos
bool CRealMyRoomForm983::OnUccDrawResult983(int result, FrForm* form)
{
	m_pUccDrawDlg983 = NULL;
	unsigned long charTid = Doc()->GetMyPartTidList().m_charTid;
	m_pItemList983->SetVisible(true);
	if (g_lobbyModal983 != 0)
	{
		if (g_view)
			MyRoomShowPet983((char*)this, charTid);
		return true;
	}
	MyRoomPetClear983((char*)this);
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x185, 0, 0, 1, 0, 0);
	ShowUccHiddenCtrls983(true);
	return true;
}

// 00605d40
bool CRealMyRoomForm983::OnUccCopyResult983(int result, FrForm* form)
{
	m_pUccCopyDlg983 = NULL;
	m_pItemList983->SetVisible(true);
	return true;
}

// 00620e00 (00621b5f..006222bc), abas 0x16/0x1b: botoes da roupa UCC e o filtro da selecao. true = clique consumido
bool CRealMyRoomForm983::OnUccListBtnDown983(FrListItem* pItem)
{
	sItemInfo* pInfo = (sItemInfo*)pItem->pData;
	const IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->typeId);
	bool ucc = IsUccClothes(pInfo->typeId);
	bool drawn = (pInfo->status & 1) != 0;
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	WRect rc103, rc144;
	bool hasRc = UccBtnRect983(pItem, 103.0f, rc103) && UccBtnRect983(pItem, 144.0f, rc144);
	if (pPart != NULL && !drawn && hasRc)
	{
		int cat = pPart->Category;
		if ((cat == 7 || cat == 8) && rc144.IsInRect(mouse) && ucc && m_pUccDrawDlg983 == NULL)
		{
			CIconToolTip::GetInstance()->ClearToolTip();
			m_pUccDrawDlg983 = CreateForm<FrUccDrawDlg>(g_pFresh->GetManager(), this, "ucc_draw", NULL);
			m_pUccDrawDlg983->SetUccDrawInfo((eDrawMode)0, pInfo, false);
			if (!m_pUccDrawDlg983->IsInitialized())
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)MakeStr(K2L_Compatibility("\xbc\xad\xb9\xf6\xb7\xce\xba\xce\xc5\xcd \xc7\xca\xbf\xe4\xc7\xd1 \xc1\xa4\xba\xb8\xb8\xa6 \xbe\xf2\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9(\xc4\xda\xb5\xe5:%d)"), 2), 0, 0, 0, 0));
			else
			{
				WPoint pos(m_pMainBg3_983 ? m_pMainBg3_983->GetRect().x : 0.0f, m_pMainBg3_983 ? m_pMainBg3_983->GetRect().y : 0.0f);
				m_pUccDrawDlg983->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnUccDrawResult983, pos, 1);
				m_pUccDrawDlg983->SetMyRoom983(this);
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x186, 0, 0, 0, 0, 0);
				ShowUccHiddenCtrls983(false);
				MyRoomShowPet983((char*)this, Doc()->GetMyPartTidList().m_charTid);
				m_pItemList983->SetVisible(false);
			}
		}
		WRect& rcCopy = cat == 7 ? rc103 : rc144;
		if ((cat == 7 || cat == 9) && rcCopy.IsInRect(mouse) && ucc && m_pUccCopyDlg983 == NULL)
		{
			CIconToolTip::GetInstance()->ClearToolTip();
			m_pItemList983->SetVisible(false);
			m_pUccCopyDlg983 = CreateForm<FrUccCopyDlg>(g_pFresh->GetManager(), this, "ucc_copy", NULL);
			m_pUccCopyDlg983->SetItem(pInfo);
			WPoint pos(359.0f, 71.0f);
			m_pUccCopyDlg983->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnUccCopyResult983, pos, 1);
		}
	}
	if (ucc && (!drawn || !UccManager()->IsDownloadComplete(pInfo->typeId, pInfo->UccIndex)))
	{
		m_pItemList983->UnselectItem(pItem);
		return true;
	}
	if (hasRc && rc144.IsInRect(mouse) && drawn && pPart != NULL && (pPart->Category == 7 || pPart->Category == 8))
	{
		if (!m_bExhibitWait983)
		{
			m_pItemList983->UnselectAllItem();
			// 00622163: GetUccItemName num sItemInfo so com guid, tid, indice e status 1
			sItemInfo copy;
			memset(&copy, 0, sizeof(copy));
			copy.id = pInfo->id;
			copy.typeId = pInfo->typeId;
			strcpy(copy.UccIndex, pInfo->UccIndex);
			copy.status = 1;
			char name[0x29];
			memset(name, 0, sizeof(name));
			strcpy(name, GetUccItemName(&copy));
			if (name[0] != 0)
			{
				FrSelfDesignAccept* dlg = CreateForm<FrSelfDesignAccept>(g_pFresh->GetManager(), this, "selfdesign_accept", NULL);
				if (dlg != NULL)
				{
					dlg->SetData983(pInfo->typeId, pInfo->id, pInfo->UccIndex, std::string(name));
					dlg->SetCaption(K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce" "CP\xc6\xc7\xb8\xc5\xb5\xee\xb7\xcf\xbd\xc5\xc3\xbb"));
					dlg->Open(NULL, 0);
				}
			}
		}
		else
		{
			FrForm* dlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
			dlg->SetMessage(K2L_Compatibility("Please wait..."), false);
			dlg->Open(NULL, 3);
		}
		return true;
	}
	return false;
}

// cardpacklist LBUTTONDOWN (645 OnCardPackListDown): seleciona o item; segundo clique mostra o botao de uso
void CRealMyRoomForm983::OnCardPackListDown983()
{
	FrListBox* pack = m_pCardPackList983;
	if (!pack)
		return;
	// 006101e0: so na sub-aba Card Holic (0x1c); o ramo CAnotherHand::IsWorking -> ToggleItem nao existe aqui (previa sincrona)
	if (m_subTab983 != 0x1c)
		return;
	FrListItem* pItem = pack->GetItemUnderCursor();
	if (pItem == NULL || pItem->pData == NULL)
		return;
	if (pItem->selected)
	{
		g_audio->PlaySfx("ui_desktop_icon_click");
		if (CardBook983() && CardBook983()->m_pUseButton)
			CardBook983()->m_pUseButton->SetVisible(true);
		MyRoomShowPet983((char*)this, ((sCards*)pItem->pData)->tid); // 0060e8b0
		return;
	}
	pack->SelectItem(pItem, true);
}

// cardpacklist RBUTTONDOWN 00615c10: item selecionado = mesmo caminho do clique (006101e0); senao alterna a selecao;
// depois abre a "information" do pacote (00615010(&tid, 0, 0))
void CRealMyRoomForm983::OnCardPackListRDown983()
{
	FrListBox* pack = m_pCardPackList983;
	if (!pack || m_subTab983 != 0x1c)
		return;
	FrListItem* pItem = pack->GetItemUnderCursor();
	if (pItem == NULL || pItem->pData == NULL)
		return;
	if (pItem->selected)
		OnCardPackListDown983();
	else
		pack->ToggleItem(pItem);
	ShowInformation983(((sCards*)pItem->pData)->tid);
}

// item_storage_list RBUTTONDOWN 00615fa0: so com NT REAL_MYROOM (00a7a8d0) e o armazem aberto (+0xa84): abre a "information" do
// item (00615010(&tid, 0, ...)) e seleciona o item sob o cursor (SelectItem 1). (O teste de DAT_00e768fc+0x2d nao tem par aqui.)
void CRealMyRoomForm983::OnStorageListRDown983()
{
	char* self = (char*)this;
	FrListBox* box = m_pStorageList983;
	// 00615fa0: IsLocalContent(0x70) = S4_ITEM_STORAGE (D667; antes S4_REAL_MYROOM 0x57)
	if (!IsLocalContent(S4_ITEM_STORAGE) || !m_storageOpen983 || !box)
		return;
	FrListItem* it = box->GetItemUnderCursor();
	if (!it)
		return;
	if (it->pData)
		ShowInformation983(((sItemInfo*)it->pData)->typeId, (sItemInfo*)it->pData);
	box->SelectItem(it, true);
}

// D667: handlers do armazem no form vivo (os do 645 estao no layout "REALMYROOM." morto, D631); corpos = 645
// realmyroommainui.cpp 14494..14561/7482 com m_pFrListBox_DB4 -> +0xaa0, m_b_D98 -> +0xa84.
// 0060a6b0 item_storage_list OWNERDRAW
void CRealMyRoomForm983::OnStorageListOwnerDraw983(int param)
{
	if (CItemStorage::Instance() && param)
		CItemStorage::Instance()->DrawStoredItemInfo(*(const FrListBox**)&m_pStorageList983, *(FrListItem*)param);
}

// 0060a6e0 item_storage_list LBUTTONDOWN: prepara o arrasto (MODE_NONE, indice, item guardado) e seleciona
void CRealMyRoomForm983::OnStorageListDown983()
{
	char* self = (char*)this;
	FrListBox* box = m_pStorageList983;
	if (!IsLocalContent(S4_ITEM_STORAGE) || !m_storageOpen983 || !box)
		return;
	CItemStorage* pStorage = CItemStorage::Instance();
	if (pStorage == NULL || pStorage->IsLock())
		return;
	FrListItem* it = box->GetItemUnderCursor();
	if (it == NULL)
		return;
	if (sStoredItemInfo* pInfo = (sStoredItemInfo*)it->pData)
	{
		sStoredItemDragDrop drag;
		memset(&drag, 0, sizeof(drag));
		drag.index = it->idx;
		drag.mode = sStoredItemDragDrop::MODE_NONE;
		drag.stored = *pInfo;
		pStorage->SetDragDropItem(drag);
	}
	box->SelectItem(it, true);
}

// 0060a7d0 item_storage_list DBLCLICK: seleciona e pede a retirada (ReqPopStoredItemInfo 007fe390)
void CRealMyRoomForm983::OnStorageListDClick983()
{
	char* self = (char*)this;
	FrListBox* box = m_pStorageList983;
	if (!IsLocalContent(S4_ITEM_STORAGE) || !m_storageOpen983 || !box)
		return;
	CItemStorage* pStorage = CItemStorage::Instance();
	if (pStorage == NULL || pStorage->IsLock())
		return;
	FrListItem* it = box->GetItemUnderCursor();
	if (it == NULL)
		return;
	box->SelectItem(it, true);
	pStorage->ReqPopStoredItemInfo(*it);
}

// 0060a830 item_storage_list MOUSEMOVE
void CRealMyRoomForm983::OnStorageListMove983()
{
	if (CItemStorage::Instance())
		CItemStorage::Instance()->SetDrag(sStoredItemDragDrop::MODE_POP);
}

// 00607010 middle_ui.item_list MOUSEMOVE: arrasto lista -> armazem. CAnotherHand::IsWorking (+0x588) nao existe aqui
// (previa sincrona); sem armazem aberto (+0xa84) o 983 ainda exige sub-aba (+0x530) != 0x17.
void CRealMyRoomForm983::OnItemListMove983()
{
	char* self = (char*)this;
	FrListBox* il = m_pItemList983;
	if (il == NULL || il->GetItemUnderCursor() == NULL)
		return;
	if (!m_storageOpen983 && m_subTab983 == 0x17)
		return;
	if (IsLocalContent(S4_ITEM_STORAGE) && CItemStorage::Instance())
		CItemStorage::Instance()->SetDrag(sStoredItemDragDrop::MODE_PUSH);
}

// card_use LBUTTONDOWN 006142e0. Lista do Card Holic visivel (+0xab8): o item selecionado (+0x108) abre o FrCardBoxDlg
// (Init(tid, uid, cardCount), Open modal) e apaga a previa (00605530(0)); ja aberto = nada. Senao, no caderno (+0xa78):
// carta SP (tid & 0x3c00000 == 0x800000) na casa selecionada da pagina de cartas -> UseCard 007f3190; outra carta ->
// janela de maestria kam_mastery_main_dialog (005be450(NULL) + DoModal 004783e0), se ainda nao aberta (00e78154).
void CRealMyRoomForm983::OnCardUseDown983()
{
	FrListBox* pack = m_pCardPackList983;
	if (pack->IsVisible())
	{
		FrListItem* pItem = pack->GetSelected();
		if (pItem == NULL || pItem->pData == NULL)
			return;
		FrWnd* pOpen = g_pFresh->GetDesktop()->FindChildForm("FrCardBoxDlg");
		if (pOpen && DYNAMIC_CAST(FrCardBoxDlg, pOpen))
			return;
		sCards* pCards = (sCards*)pItem->pData;
		FrCardBoxDlg* pDlg = CreateForm<FrCardBoxDlg>(g_pFresh->GetManager(), this, "FrCardBoxDlg", NULL);
		pDlg->Init(pCards->tid, pCards->uid, pCards->cardCount);
		pDlg->Open(NULL, 1);
		MyRoomPetClear983((char*)this);
		return;
	}
	CCardBook* book = CardBook983();
	if (book == NULL)
		return;
	CCardPage* page = book->GetCardPage();
	if (page)
	{
		sCards* pCards = page->GetCardSlot()[book->m_selectedSlot].GetCards();
		if (pCards && (pCards->tid & 0x3c00000) == 0x800000)
		{
			book->UseCard();
			return;
		}
	}
	if (RFCharMasteryMainDlg::ms_pInstance == NULL)
	{
		RFCharMasteryMainDlg* dlg = RFCharMasteryMainDlg::Create983(NULL);
		if (dlg)
			dlg->DoModal();
	}
}

// updown_group.mastery_m LBUTTONUP 0060d460 (o botao "Char. Mastery"; antes sem manipulador no form, o clique nao fazia nada).
// Servico 0x40000000 suspenso -> so o aviso (00b2e8c0); janela de maestria (00e78154) aberta -> nada; senao avisa o ator principal
// (msg 1), liga +0x11411 e pede ao servidor (0x157 com o meu UID).
void CRealMyRoomForm983::OnMasteryMUp983()
{
	if (DOCJP()->IsControlServerService983(0x40000000))
	{
		S5::UTIL::NotifyServiceStopped983(0x40000000);
		return;
	}
	if (RFCharMasteryMainDlg::ms_pInstance != NULL)
		return;
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	Doc()->m_goStopMode = true;
	WSendPacket packet((enumClientPacket)0x157);
	packet.Encode4(Doc()->m_myInfo.info.dwUID);
	packet.Send(TO_GAME);
}

// 0060d220 (upgrade, modo 0) / 0060d340 (downgrade, modo 1): clube (+0x518) com nivel incompativel -> aviso (msg 0x23); senao abre
// o dialogo "downgrade" (+0x1f8) para o clubset equipado (doc+0xa2e)
void CRealMyRoomForm983::OnUpDownUp983(int mode)
{
	IFF_STRUCT::sClubSet* pClubSet = Doc()->m_itemManager.FindClubSet(m_selClubTid983);
	if (pClubSet)
	{
		if ((pClubSet->c.Level > Doc()->m_myInfo.stat.Level && !pClubSet->c.IsUnderLvl) ||
			(pClubSet->c.Level < Doc()->m_myInfo.stat.Level && pClubSet->c.IsUnderLvl))
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xca\xbd\xc3\xbf\xc0."),
				0, 0, 0, 0));
			return;
		}
	}
	FrDowngradeDlg* dlg = CreateForm<FrDowngradeDlg>(g_pFresh->GetManager(), this, "downgrade", NULL);
	m_pUpDownDlg983 = dlg;
	dlg->SetInfo(Doc()->m_myInfo.userEquip.guidClubSet, mode);
	dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnUpDownResult983, 3);
	OnTooltipHoverOff983();
}

// HOVERON 0060ce20: dica CIconToolTip do botao (D637; antes o mapa so tinha LBUTTONUP -> nenhuma dica)
void CRealMyRoomForm983::OnTooltipHoverOn983(int param)
{
	char* self = (char*)this;
	FrButton* btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	eBarButton tip;
	if (btn == m_pUpgrade983)
		tip = TOOLTIP_UPGRADE; // 0x28
	else if (btn == m_pDowngrade983)
		tip = TOOLTIP_DOWNGRADE; // 0x29
	else if (btn == m_pEquipCancel983)
		tip = TOOLTIP_EQUIPCANCEL; // 0x2a
	else if (btn == m_pEquipReset983)
		tip = TOOLTIP_EQUIPRESET; // 0x2b
	else if (btn == m_pClubWork983)
		tip = TOOLTIP_CLUBMADE; // 0x2d
	else
		return;
	CIconToolTip::GetInstance()->SetToolTip(tip);
}

// HOVEROFF 0060cf70: limpa so se a dica atual e a do botao; com param NULL (fim do clique) sempre limpa
void CRealMyRoomForm983::OnTooltipHoverOff983(int param)
{
	char* self = (char*)this;
	FrButton* btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	bool bClear = false;
	switch (CIconToolTip::GetInstance()->GetCurTipIndex())
	{
	case TOOLTIP_UPGRADE: bClear = btn == m_pUpgrade983; break;
	case TOOLTIP_DOWNGRADE: bClear = btn == m_pDowngrade983; break;
	case TOOLTIP_EQUIPCANCEL: bClear = btn == m_pEquipCancel983; break;
	case TOOLTIP_EQUIPRESET: bClear = btn == m_pEquipReset983; break;
	case TOOLTIP_CLUBMADE: bClear = btn == m_pClubWork983; break;
	}
	if (btn == NULL || bClear)
		CIconToolTip::GetInstance()->ClearToolTip();
}

// 00610980: resultado do dialogo de autoridade: OK (1) manda 0x00B8 com o bloco do doc (+0x1070c, 0x6b) salvo senha vazia com
// autoridade ligada (aviso rmr_notify -> reabre pelo 00622940)
bool CRealMyRoomForm983::OnAuthDlgResult983(int result, FrForm* form)
{
	char* self = (char*)this;
	if (form)
		FrForm::ms_bHasTail = true;
	CRealMyRoom_AuthorityDlg* dlg = m_pAuthDlg983;
	if (result == 1 && dlg != NULL)
	{
		bool bAuth = dlg->GetAuthSetting();
		dlg->GetPassWord();
		CSharedDoc* d = Doc();
		if (strlen(d->m_rmrAuthority.password) == 0 && bAuth)
		{
			FrForm* pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this, "rmr_notify", NULL);
			pNotify->SetMessage(K2L_Compatibility("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xd4\xb7\xc2\xc0\xcc \xb5\xc7\xc1\xf6 \xbe\xca\xbe\xc6\xbc\xad \xb1\xc7\xc7\xd1 \xbc\xb3\xc1\xa4\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."), false);
			pNotify->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnAuthDlgFailResult983, 2);
		}
		else
		{
			d->m_rmrAuthority.bPrivate = dlg->GetAuthSetting();
			WSendPacket packet((enumClientPacket)0xb8);
			packet.EncodeBuffer((char*)&d->m_rmrAuthority, sizeof(d->m_rmrAuthority));
			packet.Send(TO_GAME);
		}
	}
	m_pAuthDlg983 = NULL;
	return true;
}

// 00622940: aviso fechado com OK -> abre o dialogo de novo
bool CRealMyRoomForm983::OnAuthDlgFailResult983(int result, FrForm* form)
{
	if (result == 1)
		SelectMenu983(6);
	return true;
}

// 0061f850: abre a oficina de clubset (clubset_workshop, +0x23c)
void CRealMyRoomForm983::OnClubWorkMUp983()
{
	FrClubSetWorkshopMainDlg* dlg = FrClubSetWorkshopMainDlg::Create983(g_pFresh->GetManager(), this);
	m_pClubWorkDlg983 = dlg;
	dlg->Open((FRESH_PFN_RESULT)&CRealMyRoomForm983::OnClubWorkResult983, 1); // 983 0061f850: so FrESCAPE; com 3 o Enter fechava a oficina
	OnTooltipHoverOff983();
}

// 0061e4a0: fecha a oficina com a aba 7 selecionada -> volta a sub-aba 7
bool CRealMyRoomForm983::OnClubWorkResult983(int result, FrForm* form)
{
	FrClubSetWorkshopMainDlg* dlg = m_pClubWorkDlg983;
	if (dlg != NULL && dlg->IsRefresh983() && m_subTab983 == 7) // +0x168 (B1472: cru andava -0xc no cl 14.44)
		SelectSubTab983(7, true);
	m_pClubWorkDlg983 = NULL;
	return true;
}

// card_slot1..12 LBUTTONDOWN -> 006107f0(slot, type). Com o caderno (+0xa78) e +0xa84 == 0: casa selecionada (book+10),
// page->SelectSlot(type, slot); botao usar (book+0x8c) so com casa selecionada e carta usavel (009d9d40: tid != 0x7c800026 e
// != 0x7c800041; o 645 so excluia 0x7c800026)
void CRealMyRoomForm983::OnCardSlotDown983(unsigned char slot, unsigned char type)
{
	CCardBook* book = CardBook983();
	if (book == NULL || m_storageOpen983)
		return;
	CCardPage* page = book->GetCardPage();
	if (page == NULL)
		return;
	book->SetSelectedSlot(slot);
	page->SelectSlot(type, slot);
	if (book->m_pUseButton == NULL)
		return;
	bool use = false;
	if (page->IsSelectedSlot())
	{
		sCards* pCards = page->GetCardSlot()[slot].GetCards();
		use = pCards == NULL || (pCards->tid != 0x7c800026 && pCards->tid != 0x7c800041);
	}
	book->m_pUseButton->SetVisible(use);
}

// msgmap 00e1347c: so o X (close_wnd) por enquanto; os demais handlers ainda estao no CRealMyRoomMain (B379)
BEGIN_FRESH_MSGMAP(CRealMyRoomForm983, FrTaskForm)
ON_FRESH_VV("close_wnd", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnCloseWndUp983)
ON_FRESH_VV("item_search_btn", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnItemSearchBtnUp983)
ON_FRESH_VV("middle_ui.storage_open_btn", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnStorageOpenBtnUp983)
ON_FRESH_VV("middle_ui.tab_main", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnTabMainUp983)
ON_FRESH_VV("middle_ui.item_list", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnItemListDown983)
ON_FRESH_VV("middle_ui.item_list", FRCMD_DBLCLICK, CRealMyRoomForm983::OnItemListDClick983)
ON_FRESH_VV("middle_ui.item_list", FRCMD_RBUTTONDOWN, CRealMyRoomForm983::OnItemListRDown983)
ON_FRESH_VV("middle_ui.sbox.item_slot", FRCMD_DBLCLICK, CRealMyRoomForm983::OnItemSlotDClick983)
ON_FRESH_VI("middle_ui.sbox.item_slot", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnItemSlotOwnerDraw983)
ON_FRESH_VI("middle_ui.pet", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnPetOwnerDraw983)
ON_FRESH_VI("middle_ui.item_list", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnItemListOwnerDraw983)
ON_FRESH_VV("middle_ui.tab_sub", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnTabSubUp983)
ON_FRESH_VI("card_book_back_bg", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnCardBookBackDraw983)
ON_FRESH_VI("card_book", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnCardBookDraw983)
ON_FRESH_VI("cardpacklist", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnCardPackListDraw983)
ON_FRESH_VV("cardpacklist", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardPackListDown983)
ON_FRESH_VV("cardpacklist", FRCMD_RBUTTONDOWN, CRealMyRoomForm983::OnCardPackListRDown983)
ON_FRESH_VV("item_storage_list", FRCMD_RBUTTONDOWN, CRealMyRoomForm983::OnStorageListRDown983)
ON_FRESH_VI("item_storage_list", FRCMD_OWNERDRAW, CRealMyRoomForm983::OnStorageListOwnerDraw983)
ON_FRESH_VV("item_storage_list", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnStorageListDown983)
ON_FRESH_VV("item_storage_list", FRCMD_DBLCLICK, CRealMyRoomForm983::OnStorageListDClick983)
ON_FRESH_VV("item_storage_list", FRCMD_MOUSEMOVE, CRealMyRoomForm983::OnStorageListMove983)
ON_FRESH_VV("middle_ui.item_list", FRCMD_MOUSEMOVE, CRealMyRoomForm983::OnItemListMove983)
ON_FRESH_VV("card_pack", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardPackTab983)
ON_FRESH_VV("card_volume1", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardVol1Tab983)
ON_FRESH_VV("card_volume2", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardVol2Tab983)
ON_FRESH_VV("card_volume3", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardVol3Tab983)
ON_FRESH_VV("card_volume4", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardVol4Tab983)
ON_FRESH_VV("card_volume5", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardVol5Tab983)
ON_FRESH_VV("card_sp", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSpTab983)
ON_FRESH_VV("cardpage_index", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardIndexDown983)
ON_FRESH_VV("cardpage_index", FRCMD_MOUSEMOVE, CRealMyRoomForm983::OnCardIndexMove983)
ON_FRESH_VV("card_use", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardUseDown983)
ON_FRESH_VV("updown_group.mastery_m", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnMasteryMUp983)
ON_FRESH_VV("updown_group.upgrade", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnUpgradeUp983)
ON_FRESH_VV("updown_group.downgrade", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnDowngradeUp983)
ON_FRESH_VV("updown_group.clubwork_m", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnClubWorkMUp983)
ON_FRESH_VV("middle_ui.sbox.equip_cancel", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnEquipCancelUp983)
ON_FRESH_VV("middle_ui.sbox.equip_reset", FRCMD_LBUTTONUP, CRealMyRoomForm983::OnEquipResetUp983)
ON_FRESH_VI("middle_ui.sbox.equip_cancel", FRCMD_HOVERON, CRealMyRoomForm983::OnTooltipHoverOn983)
ON_FRESH_VI("middle_ui.sbox.equip_cancel", FRCMD_HOVEROFF, CRealMyRoomForm983::OnTooltipHoverOff983)
ON_FRESH_VI("middle_ui.sbox.equip_reset", FRCMD_HOVERON, CRealMyRoomForm983::OnTooltipHoverOn983)
ON_FRESH_VI("middle_ui.sbox.equip_reset", FRCMD_HOVEROFF, CRealMyRoomForm983::OnTooltipHoverOff983)
ON_FRESH_VI("updown_group.upgrade", FRCMD_HOVERON, CRealMyRoomForm983::OnTooltipHoverOn983)
ON_FRESH_VI("updown_group.upgrade", FRCMD_HOVEROFF, CRealMyRoomForm983::OnTooltipHoverOff983)
ON_FRESH_VI("updown_group.downgrade", FRCMD_HOVERON, CRealMyRoomForm983::OnTooltipHoverOn983)
ON_FRESH_VI("updown_group.downgrade", FRCMD_HOVEROFF, CRealMyRoomForm983::OnTooltipHoverOff983)
ON_FRESH_VI("updown_group.clubwork_m", FRCMD_HOVERON, CRealMyRoomForm983::OnTooltipHoverOn983)
ON_FRESH_VI("updown_group.clubwork_m", FRCMD_HOVEROFF, CRealMyRoomForm983::OnTooltipHoverOff983)
ON_FRESH_VV("card_slot1", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot1Down983)
ON_FRESH_VV("card_slot2", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot2Down983)
ON_FRESH_VV("card_slot3", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot3Down983)
ON_FRESH_VV("card_slot4", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot4Down983)
ON_FRESH_VV("card_slot5", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot5Down983)
ON_FRESH_VV("card_slot6", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot6Down983)
ON_FRESH_VV("card_slot7", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot7Down983)
ON_FRESH_VV("card_slot8", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot8Down983)
ON_FRESH_VV("card_slot9", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot9Down983)
ON_FRESH_VV("card_slot10", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot10Down983)
ON_FRESH_VV("card_slot11", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot11Down983)
ON_FRESH_VV("card_slot12", FRCMD_LBUTTONDOWN, CRealMyRoomForm983::OnCardSlot12Down983)
END_FRESH_MSGMAP()

static FrForm* CreateRealMyRoom983(FrWndManager* pManager, const char* name)
{
	return CreateForm<CRealMyRoomForm983>(pManager, NULL, name);
}
static FrFormRegistrar983 s_realMyRoomRegistrar("realmyroom", CreateRealMyRoom983);
