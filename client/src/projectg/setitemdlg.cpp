#include "minatl.h"
#include "setitemdlg.h"
#include "frarea.h"
#include "fredit.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "shareddoc.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrSetItemDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrSetItemDlg, FrForm)

ON_FRESH_VI("portrait", FRCMD_INIT, FrSetItemDlg::OnPortraitInit)
ON_FRESH_VI("name", FRCMD_INIT, FrSetItemDlg::OnNameInit)
ON_FRESH_VI("component", FRCMD_INIT, FrSetItemDlg::OnComponentInit)
ON_FRESH_VI("component", FRCMD_OWNERDRAW, FrSetItemDlg::OnComponentOwnerDraw)
ON_FRESH_VV("component", FRCMD_RBUTTONDOWN, FrSetItemDlg::OnComponentRBtnDown)

END_FRESH_MSGMAP()

FrSetItemDlg::FrSetItemDlg()
{
	m_pPortrait = NULL;
	m_pName = NULL;
	m_pComponent = NULL;
	m_pSetItem = NULL;
}

void FrSetItemDlg::SetSetItem(unsigned long typeId, const char** pPeriod)
{
	if ((typeId & 0xFC000000) == 0x24000000)
	{
		m_pSetItem = ItemManager()->FindSetItem(typeId);

		if (m_pPortrait)
		{
			if (m_pSetItem)
				m_pPortrait->SetBgImg(m_pSetItem->c.Icon);
			else
				m_pPortrait->SetBgImg("hide");
		}

		if (m_pSetItem)
		{
			if (m_pName)
			{
				const char* szType = "";

				switch (m_pSetItem->c.InStock)
				{
				case 2:
					szType = K2L_Compatibility("(\xbc\xb1\xb9\xb0\xc0\xfc\xbf\xeb)");
					break;
				case 3:
					szType = K2L_Compatibility("(\xb1\xb8\xc0\xd4\xc0\xfc\xbf\xeb)");
					break;
				case 4:
					szType = K2L_Compatibility("(\xc0\xfc\xbd\xc3\xbf\xeb)");
					break;
				}

				m_pName->AddText(MakeStr("%s\\c0xffff0000\\c%s\\c0xffff0000\\c",
									 m_pSetItem->c.Name, szType),
					0, true);
			}

			if (m_pComponent)
			{
				for (int i = 0; i < m_pSetItem->nElems; i++)
				{
					IFF_ITEM_COMMON* pCommon =
						ItemManager()->FindCommonItem(m_pSetItem->ElemIds[i]);

					if (pCommon)
						m_pComponent->AddItem(pCommon);
				}
			}

			IFF_STRUCT::sDesc* pDesc =
				ItemManager()->FindDesc(m_pSetItem->c.TypeId);
			if (pDesc)
				SetMessage(pDesc->Desc, false);

			// 983 008ed8a0: o Recycle (008e0df6) passa as 2 linhas do periodo de troca no lugar do periodo de venda
			if (pPeriod && pPeriod[0] && pPeriod[1])
			{
				m_pName->AddText(pPeriod[0], 0, true);
				m_pName->AddText(pPeriod[1], 0, true);
				return;
			}

			_SYSTEMTIME* pSaleS = NULL;
			_SYSTEMTIME* pSaleE = NULL;

			// 983 0063f280 (estado de venda, CSharedDoc+0x113e0) + 00a186b0 (ShopLimitItem.iff: periodo
			// +0x1c/+0x2c, total +0x0c, por vez +0x10, horas +0x14) vem antes do periodo do item comum
			unsigned long saleState = 0;
			std::map<unsigned long, sShopSaleState983>::iterator itState =
				Doc()->m_shopSaleState983.find(m_pSetItem->c.TypeId);
			if (m_pSetItem->c.TypeId && itState != Doc()->m_shopSaleState983.end())
				saleState = itState->second.state;
			IFF983::sShopLimitItem* pLimit = saleState
				? ItemManager()->FindShopLimitItem983(m_pSetItem->c.TypeId)
				: NULL;

			if (pLimit)
			{
				pSaleS = &pLimit->saleStart;
				pSaleE = &pLimit->saleEnd;
				if (pSaleE->wYear)
				{
					unsigned long nTotal = pLimit->total;
					unsigned long nEach = pLimit->perDay;
					unsigned long nHours = pLimit->hours;
					std::string szCount;
					if (nHours)
						szCount = MakeStr(K2L_Compatibility("%d\xc0\xcf %d \xb0\xb3\xbe\xbf"),
							nHours / 24, nEach);
					else
						szCount = MakeStr(K2L_Compatibility("\xc3\xd1 %d \xb0\xb3"), nTotal);

					m_pName->AddText(
						MakeStr(
							K2L_Compatibility("\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf\\c0xff000000\\c"),
							pSaleS->wMonth, pSaleS->wDay, pSaleE->wMonth,
							pSaleE->wDay),
						0, true);
					// 983 008edb8a: so tid 0x18 (nunca num set) mostra a quantidade
					if ((m_pSetItem->c.TypeId & 0xFC000000) == 0x18000000)
						m_pName->AddText(
							MakeStr(K2L_Compatibility("\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xbc\xf6\xb7\xae: %s\\c0xff000000\\c"),
								szCount.c_str()),
							0, true);
				}
				else
					m_pName->AddText("\n", 0, true);
			}
			else if (ItemManager()->HasSalePeriod(m_pSetItem->c.TypeId, &pSaleS,
					&pSaleE) && pSaleS->wYear && pSaleE->wYear)
			{
				m_pName->AddText(
					MakeStr(
						K2L_Compatibility("\\c0xffff0000\\c\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf\\c0xff000000\\c"),
						pSaleS->wMonth, pSaleS->wDay, pSaleE->wMonth,
						pSaleE->wDay),
					0, true);
			}
			else
				m_pName->AddText("\n", 0, true);
		}
	}
}

void FrSetItemDlg::OnPortraitInit(int param)
{
	m_pPortrait = DYNAMIC_CAST(FrArea, param);
}

void FrSetItemDlg::OnNameInit(int param)
{
	m_pName = DYNAMIC_CAST(FrEdit, param);
}

void FrSetItemDlg::OnComponentInit(int param)
{
	m_pComponent = DYNAMIC_CAST(FrListBox, param);
	// 983 008ed290: liga o direito (+0xfc); sem isso o OnComponentRBtnDown nunca chega
	if (m_pComponent)
		m_pComponent->UseRightButton(true);
}

void FrSetItemDlg::OnComponentOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	IFF_ITEM_COMMON* pCommon = (IFF_ITEM_COMMON*)pItem->pData;
	if (!pCommon)
		return;

	// 983 008ed310: caixa button_n/button_o do tamanho da linha atrás da peça; ícone em +5,+5
	const Bitmap* pBg = g_pFresh->GetBitmap(
		(pItem->selected || pItem->underCursor) ? "button_o" : "button_n");
	if (pBg)
	{
		WRect dest(pItem->pos.x, pItem->pos.y,
			(float)m_pComponent->GetItemWidth(),
			(float)m_pComponent->GetItemHeight());

		pDevice->DrawTexture(pBg, dest, 0xffffffff, 0);
	}

	const Bitmap* pBmp =
		g_pFresh->GetManager()->GetBitmap("ITEMS", pCommon->Icon);
	if (!pBmp)
		pBmp =
			g_pFresh->GetManager()->GetBitmap("ITEMS_FASHION", pCommon->Icon);

	if (pBmp)
	{
		WRect dest(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f,
			(float)pBmp->Width(), (float)pBmp->Height());

		pDevice->DrawTexture(pBmp, dest, 0xffffffff, 0);
	}

	pDevice->SetTextColor(0xff000000, 0xffffffff);
	pDevice->SetTextStyle(1);
	pDevice->Print(WPoint(pItem->pos.x + 90.0f, pItem->pos.y + 14.0f), 0, "%s",
		pCommon->Name);

	pDevice->SetTextStyle(0);

	const Bitmap* pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS",
		MakeStr("level_%03d", pCommon->Level + 1));
	if (pLevel)
	{
		WRect dest;
		dest.x = pDevice->GetTextExtendOld983(
			K2L_Compatibility("\xb7\xb9    \xba\xa7:")) +
			pItem->pos.x + 102.0f;
		dest.y = pItem->pos.y + 31.0f;
		dest.w = pLevel->Width();
		dest.h = pLevel->Height();
		pDevice->DrawTexture(pLevel, dest, 0xffffffff, 0);

		pDevice->Print(WPoint(pItem->pos.x + 90.0f, pItem->pos.y + 34.0f), 0,
			K2L_Compatibility("\xb7\xb9    \xba\xa7:"));

		if (pCommon->Level > Doc()->m_myInfo.stat.Level)
		{
			pDevice->Print(WPoint(dest.Right() + 5.0f, pItem->pos.y + 34.0f), 0,
				K2L_Compatibility("\xc0\xcc\xbb\xf3"));

			pDevice->SetTextColor(0xffff0000, 0xffffffff);
		}
		else
			pDevice->Print(WPoint(dest.Right() + 5.0f, pItem->pos.y + 34.0f), 0,
				K2L_Compatibility("\xc0\xcc\xbb\xf3"));
	}
}

// 983 008ed670: direito numa peça abre a "information" dela (sem checar se já há uma aberta)
void FrSetItemDlg::OnComponentRBtnDown()
{
	FrListItem* pItem = m_pComponent ? m_pComponent->GetItemUnderCursor() : NULL;
	IFF_ITEM_COMMON* pCommon = pItem ? (IFF_ITEM_COMMON*)pItem->pData : NULL;
	if (!pCommon)
		return;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "information", NULL);
	if (!pForm)
		return;

	// o 983 esconde reset e icon_clubsetworkshop antes do Open
	FrButton* pBtn = DYNAMIC_CAST(FrButton, pForm->FindChildByName("reset"));
	if (pBtn)
		pBtn->SetVisible(false);
	pBtn = DYNAMIC_CAST(FrButton, pForm->FindChildByName("icon_clubsetworkshop"));
	if (pBtn)
		pBtn->SetVisible(false);

	FrArea* pPortrait = DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
	if (pPortrait)
		pPortrait->SetBgImg(pCommon->Icon);

	FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
	FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
	if (pName && pLevel)
	{
		pLevel->SetBgImg(MakeStr("level_%03d", pCommon->Level + 1));
		// 983 d16c28 (IsUnderLvl: "이하") / d16c04 ("이상")
		pName->AddText(MakeStr(K2L_Compatibility(pCommon->IsUnderLvl
			? "\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xc7\xcf"
			: "\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xbb\xf3"),
			pCommon->Name), 0, true);
	}

	IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pCommon->TypeId);
	if (pDesc)
		pForm->SetMessage(pDesc->Desc, false);
	pForm->Open(NULL, 3);
}
