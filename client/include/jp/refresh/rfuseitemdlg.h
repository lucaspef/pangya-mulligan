#pragma once
#include "vc10_layout.h"
#include "rfformmodalbase.h"

struct sItemInfo;
namespace IFF_STRUCT { struct sItem; }

// JP 983: diálogos modais comuns da RF (unidade rfuseitemdlg.cpp, 005ccb80–005ce010; sessão B137). Base RFFormModalBase<RFWndEx>.
// - RFUseItemDlg ("common_use_item_dialog.xml"; RTTI .?AVRFUseItemDlg@@, vtables 00d106ac / 00d106a0; 0x194): "usar o item?" com
//   ícone (item_img), quantidade "x%d" (item_num_label), texto (message), ok_btn = Close(1), cancel_btn = Close(2). Instância
//   00e781e4. Aberto pelas janelas da maestria (005bc6b0: item 0x1a0000c2).
// - RFNoticeDlg ("common_notice_dialog.xml"; vtables 00d10744 / 00d10738; 0x18c): aviso com texto (message) e ok_btn = Close(1).
// Nomes dos métodos e campos meus (…983), menos as classes e os da RF.
class RFUseItemDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFUseItemDlg(); // 005ccbd0
	virtual ~RFUseItemDlg(); // 005ccc70 (??_G 005cd140; 2ª base 005ccc60)

	virtual void OnInitialUpdate(); // 005cda40 (slot 10)

	void SetInstance983(); // 005ccb80 (ninguém chama)
	static void ClearInstance983(); // 005ccb90 (ninguém chama)
	// cópias na unidade rfcharmasterycardselectdlg (sessão B142), sem chamador
	static RFUseItemDlg* GetInstance983(); // 005b82a0
	static RFUseItemDlg* GetInstance2_983(); // 005b82b0
	static bool IsCreated983(); // 005b87e0
	// 005cd6f0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFUseItemDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005ccba0: Close(1)
	void OnCancel983(EventArgs& e); // 005ccbb0: Close(2)
	void SetItem983(sItemInfo* pItem); // 005cd2c0: ícone pelo Item.iff e quantidade do item do jogador
	void SetIffItem983(IFF_STRUCT::sItem* pIff); // 005cd470: ícone e quantidade (COM[0]) do registro do Item.iff
	void SetMessage983(const std::wstring& text); // 005cd830
	void SetMessage983(const char* text); // 005cd900 (multibyte)

	static RFUseItemDlg* ms_pInstance; // 00e781e4
	static RFWnd* s_pParent983; // 00e781e0

	RFPicture* m_pItemImg983; // +0x188
	RFLabel* m_pItemNum983; // +0x18c
	RFTextArea* m_pMessage983; // +0x190
};
C_ASSERT_VC10(sizeof(RFUseItemDlg) == 0x194);

class RFNoticeDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFNoticeDlg(); // 005ccce0
	virtual ~RFNoticeDlg(); // 005ccd70 (??_G 005cd170; 2ª base 005ccd60)

	virtual void OnInitialUpdate(); // 005cdea0 (slot 10)

	// 005cdd60: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFNoticeDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005ccbc0: Close(1)
	void SetMessage983(const char* text); // 005cd610 (multibyte)

	static RFWnd* s_pParent983; // 00e781e8

	RFLabel* m_pMessage983; // +0x188
};
C_ASSERT_VC10(sizeof(RFNoticeDlg) == 0x18c);
