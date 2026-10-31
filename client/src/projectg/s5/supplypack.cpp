#include "minatl.h"
#include "s5/supplypack.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "exhibition.h"
#include "titlefontex.h"
#include "inputmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "rtti.h"
#include "bitmap.h"
#include "../../shared/sharedtables.h"

const float g_Five = 5.0f;

// HACK: forces the three-float constant table (5.0, HUGE, EPSILON) that the
// original unit emitted; the inline functions that used it are unknown.
inline void rb_UseConsts()
{
	(void)g_Five;
	(void)g_HUGE;
}

// HACK: these inlines live in frwnd.h in the original source
inline void FrWnd::EnableToolTip(bool enable)
{
	m_nFlags.Turn(FWF_TOOLTIPS, enable);
}

inline bool FrWnd::IsEnableToolTip() const
{
	return m_nFlags.GetFlag(FWF_TOOLTIPS);
}

extern Fresh* g_pFresh;

class CFontAnimation
{
public:
	CFontAnimation(WTitleFontEx* pFont, const WVector2D* pStart);

	void SetStartPoint(const WVector2D& pos);
	void AddFontFx(const char* text, const WVector2D& dir, float speed,
		float time);
	void Process(float deltaTime);
	void Display();

private:
	struct FONTFXPIECE
	{
		~FONTFXPIECE() { }

		std::string text;
		float time;
		float alpha;
		WVector2D pos;
		WVector2D dir;
		float speed;
	};

	WTitleFontEx* m_pFont;
	std::list<FONTFXPIECE> m_fxList;
	WVector2D m_start;
};

CFontAnimation::CFontAnimation(WTitleFontEx* pFont, const WVector2D* pStart)
	: m_pFont(pFont)
{
	if (pStart)
		m_start = *pStart;
	else
	{
		m_start.x = 0.0f;
		m_start.y = 0.0f;
	}
}

void CFontAnimation::SetStartPoint(const WVector2D& pos)
{
	m_start = pos;
	if (0)
		Wabs(pos.x);
}

void CFontAnimation::AddFontFx(const char* text, const WVector2D& dir,
	float speed, float time)
{
	FONTFXPIECE fx;
	fx.alpha = 255.0f;
	fx.time = time;
	fx.dir = dir;
	fx.pos = m_start;
	fx.speed = speed;
	fx.text = text;
	m_fxList.push_back(fx);
}

void CFontAnimation::Process(float deltaTime)
{
	std::list<FONTFXPIECE>::iterator it = m_fxList.begin();
	while (it != m_fxList.end())
	{
		FONTFXPIECE& fx = *it;
		if (fx.time > 0.0f)
		{
			float rate = 1.0f / fx.time;
			fx.alpha -= rate * deltaTime * 255.0f;
			fx.pos += fx.dir * fx.speed * deltaTime;
		}
		else
			fx.alpha = 0.0f;

		if (fx.alpha <= 0.0f)
			it = m_fxList.erase(it);
		else
			++it;
	}
}

void CFontAnimation::Display()
{
	for (std::list<FONTFXPIECE>::iterator it = m_fxList.begin();
		it != m_fxList.end(); ++it)
	{
		FONTFXPIECE& fx = *it;
		m_pFont->SetColor(((unsigned char)(int)fx.alpha << 24) | 0xffffff);
		m_pFont->Print(fx.pos.x, fx.pos.y, fx.text.c_str(), TFA_CENTER, 1.0f,
			1.0f);
	}
}

namespace
{
	int GetSupplyGroup(unsigned long tid)
	{
		switch (tid)
		{
		case 0x1a000105:
		case 0x1a00014b:
			return 5;
		}
		return -1;
	}

	bool IsSuppliableItem(unsigned long tid)
	{
		if (IsLicenseLimitedItem(tid))
			return false;

		if (!ItemManager()->IsCanOverlapped(tid, 0, 0))
			return false;

		if ((tid & 0xfc000000) == 0x14000000)
		{
			if (ItemManager()->IsBasicAztecFamily(tid))
				return false;

			if (!ItemManager()->FindBall(tid))
				return false;
		}

		return true;
	}
}

namespace S5
{
	IMPLEMENT_OBJECT(FrSupplyPackDlg, FrForm)

	BEGIN_FRESH_MSGMAP(FrSupplyPackDlg, FrForm)

	ON_FRESH_VI("lt_item", FRCMD_INIT, FrSupplyPackDlg::OnItemListInit)
	ON_FRESH_VI("bt_cancel", FRCMD_INIT, FrSupplyPackDlg::OnCancelBtnInit)
	ON_FRESH_VI("bt_ok", FRCMD_INIT, FrSupplyPackDlg::OnOkayBtnInit)
	ON_FRESH_VV("bt_cancel", FRCMD_LBUTTONUP, FrSupplyPackDlg::OnCancelBtnUp)
	ON_FRESH_VV("bt_ok", FRCMD_LBUTTONUP, FrSupplyPackDlg::OnOkayBtnUp)
	ON_FRESH_VV("bg_area", FRCMD_OWNERDRAW, FrSupplyPackDlg::OnBgAreaDraw)
	ON_FRESH_VI("lt_item", FRCMD_OWNERDRAW, FrSupplyPackDlg::OnItemListDraw)
	ON_FRESH_VV("lt_item", FRCMD_LBUTTONUP, FrSupplyPackDlg::OnItemListUp)

	END_FRESH_MSGMAP()

	FrSupplyPackDlg::FrSupplyPackDlg()
		: m_tid(0),
		  m_pItemList(NULL),
		  m_pCancelBtn(NULL),
		  m_pExhibition(NULL),
		  m_alpha(0.0f),
		  m_pSelected(NULL),
		  m_pFontNum(NULL),
		  m_resultTimer(5.0f),
		  m_bResult(0),
		  m_resultCount(0),
		  m_elapsed(1.0f),
		  m_arrowOffset(0.0f),
		  m_lastCount(0),
		  m_total(0),
		  m_step(0)
	{
		m_pFontNum = new CWindFont;
		m_pFontBig = new CBlueWindFontBig;
		m_pFontAnim = new CFontAnimation(m_pFontBig, NULL);

		memset(m_resultNum, 0, sizeof(m_resultNum));

		g_input->BlockKey("ESCAPE");
	}

	FrSupplyPackDlg::~FrSupplyPackDlg()
	{
		g_input->FreeKey("ESCAPE");

		if (m_pFontAnim)
		{
			delete m_pFontAnim;
			m_pFontAnim = NULL;
		}

		if (m_pFontBig)
		{
			delete m_pFontBig;
			m_pFontBig = NULL;
		}

		if (m_pFontNum)
		{
			delete m_pFontNum;
			m_pFontNum = NULL;
		}

		if (m_pExhibition)
		{
			delete m_pExhibition;
			m_pExhibition = NULL;
		}
	}

	void FrSupplyPackDlg::Initialize(unsigned long tid)
	{
		m_tid = tid;
		m_pItemList->EnableToolTip(true);

		m_pFontAnim->SetStartPoint(
			WVector2D(m_rect.x + m_rect.w * 0.5f, m_rect.y + m_rect.h * 0.5f));

		if (GetSupplyGroup(m_tid) != 5)
		{
			m_tid = 0;
			return;
		}

		FrForm::SetDesc(
			K2L_Compatibility("\xbc\xb1\xc5\xc3\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xbc\xf6\xb7\xae\xc0\xbb \xba\xb8\xc3\xe6 \xc7\xd5\xb4\xcf\xb4\xd9."));
		FrForm::SetCaption(
			K2L_Compatibility("\xbe\xc6\xc1\xee\xc5\xd8 \xbc\xf6\xb7\xae \xba\xb8\xc3\xe6 \xc6\xd1"));

		for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
			it != Doc()->m_ballList.end(); ++it)
		{
			if (IsSuppliableItem((*it).tid) && (*it).Common[0] > 0)
			{
				m_itemList.push_back(*it);
				m_pItemList->AddItem(&m_itemList.back());
			}
		}

		while (m_pItemList->GetCurrentItemSize(true) % 3 != 0)
			m_pItemList->AddItem(NULL);
	}

	void FrSupplyPackDlg::UpdateData(unsigned int count)
	{
		if (m_bResult == 0 && count >= 10)
		{
			m_elapsed = 1.0f;
			m_bResult = 1;
			m_resultCount = count;

			for (int i = 0; i < 10; i++)
				m_resultNum[i]++;

			for (unsigned int n = count - 10; n != 0; n--)
			{
				int idx = rand() % 10;
				m_resultNum[idx]++;
			}

			return;
		}

		FrForm::Close(FrCANCEL, true);

		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), NULL, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbe\xf7\xb5\xa5\xc0\xcc\xc6\xae \xbf\xa1\xb4\xcf\xb8\xde\xc0\xcc\xbc\xc7\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
	}

	void FrSupplyPackDlg::OnProc(const float deltaTime)
	{
		static bool s_bArrowUp = true;

		if (m_tid == 0)
		{
			FrForm::Close(FrCANCEL, true);
			return;
		}

		if (m_pSelected && m_pExhibition)
		{
			if (m_alpha < 255.0f)
			{
				m_alpha += deltaTime * 120.0f;
				if (m_alpha > 255.0f)
					m_alpha = 255.0f;
			}

			m_pExhibition->Process(deltaTime, false);
		}

		if (m_bResult && m_alpha >= 250.0f)
		{
			if (s_bArrowUp)
			{
				if (m_arrowOffset < 3.0f)
					m_arrowOffset += deltaTime * 12.0f;

				if (m_arrowOffset >= 3.0f)
					s_bArrowUp = false;
			}
			else
			{
				if (m_arrowOffset > -3.0f)
					m_arrowOffset -= deltaTime * 12.0f;

				if (m_arrowOffset <= -3.0f)
					s_bArrowUp = true;
			}

			if (!(m_resultTimer > 0.0f) && m_step >= 10)
			{
				if (m_step == 10)
				{
					m_pOkayBtn->Enable(true);
					m_step++;
				}
			}
			else
			{
				m_elapsed += deltaTime;
				m_resultTimer -= deltaTime;

				if (m_elapsed > 0.5f)
				{
					m_elapsed = 0.0f;

					int idx = m_step;
					if (idx < 0)
						idx = 0;
					else if (idx > 9)
						idx = 9;

					int num = m_resultNum[idx];

					float rx = (rand() % 201 - 100) * 0.01f;
					{
						WVector2D dir;
						dir.x = rx;
						dir.y = -1.0f;
						dir.Normalize();

						m_pFontAnim->AddFontFx(MakeStr("+%d", num), dir, 80.0f,
							1.5f);
					}
					m_total += num;
					m_step++;
				}
			}

			m_pFontAnim->Process(deltaTime);
		}
	}

	bool FrSupplyPackDlg::OnResultConfirm(int result, FrForm* pForm)
	{
		m_pItemList->EnableToolTip(true);

		if (result == FrOK)
		{
			sItemInfo* pInfo = (sItemInfo*)m_pSelected->pData;
			IFF_ITEM_COMMON* pCommon = NULL;
			if (pInfo)
				pCommon = ItemManager()->FindCommonItem(pInfo->tid);

			if (pCommon)
			{
				m_pItemList->EnableToolTip(false);

				WRect rect = m_pItemList->GetRect();
				rect.x -= 7.0f;

				m_pExhibition = new CExhibitionByHardwareTnL(rect.x + rect.w * 0.5f,
					rect.y + rect.h * (1.0f / 3.0f), 0.0f, false);
				m_pExhibition->SetArea(rect.w, rect.h);
				m_pExhibition->SetDistance(1.2f, true, false);
				m_pExhibition->SetModel(pInfo->tid);
				m_pExhibition->SetEnableRotate(false);
				m_pExhibition->SetEnableZoom(false);
				m_pExhibition->SetAlpha((unsigned char)m_alpha);

				m_pOkayBtn->SetVisible(true);
				m_pOkayBtn->Enable(false);
				m_pCancelBtn->Enable(false);
				m_pCancelBtn->SetVisible(false);

				m_resultTimer = 5.0f;
				m_total = m_lastCount = pInfo->Common[0];

				WSendPacket packet((enumClientPacket)0xec);
				packet.Encode4(m_tid);
				packet.Encode4(pInfo->tid);
				packet.Send((eSendTo)0);

				AfxGetTask()->GetMainActor()
					<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
			}
		}
		else
			m_pSelected = NULL;

		return true;
	}

	void FrSupplyPackDlg::OnItemListInit(int param)
	{
		m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	}

	void FrSupplyPackDlg::OnOkayBtnInit(int param)
	{
		m_pOkayBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		m_pOkayBtn->SetVisible(false);
	}

	void FrSupplyPackDlg::OnCancelBtnInit(int param)
	{
		m_pCancelBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void FrSupplyPackDlg::OnCancelBtnUp()
	{
		FrForm::Close(FrCANCEL, true);
	}

	void FrSupplyPackDlg::OnOkayBtnUp()
	{
		FrForm::Close(FrOK, true);
	}

	void FrSupplyPackDlg::OnBgAreaDraw()
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		if (m_pExhibition)
		{
			const Bitmap* pBmp = g_pFresh->GetBitmap("supplypack_bg_2");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += 9.0f;
				rect.y += 53.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pBmp = g_pFresh->GetBitmap("supplypack_tex_2");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += (rect.w - pBmp->Width()) * 0.5f;
				rect.y += 28.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			m_pExhibition->SetAlpha((unsigned char)m_alpha);
			m_pExhibition->Display(0.0f, 0.0f);

			pBmp = g_pFresh->GetBitmap("supplypack_num");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += (rect.w - pBmp->Width()) * 0.5f;
				rect.y += 192.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			m_pFontNum->SetColor(
				((unsigned char)(int)m_alpha << 24) | 0xffffff);

			{
				WRect rc = m_rect;
				float y = m_rect.y + 212.0f;
				m_pFontNum->Print(rc.x + 71.0f, y, MakeStr("%d", m_lastCount),
					TFA_CENTER, 1.0f, 1.0f);

				m_pFontNum->SetColor(
					((unsigned char)(int)m_alpha << 24) | 0xff80ff);
				m_pFontNum->Print(rc.x + 177.0f, y, MakeStr("%d", m_total),
					TFA_CENTER, 1.0f, 1.0f);
			}

			pBmp = g_pFresh->GetBitmap("supplypack_arrow");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += (rect.w - pBmp->Width()) * 0.5f;
				rect.x += m_arrowOffset;
				rect.y += 205.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect,
					((unsigned char)(int)m_alpha << 24) | 0xffffff, 0);
			}
		}
		else
		{
			const Bitmap* pBmp = g_pFresh->GetBitmap("supplypack_bg_1");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += 13.0f;
				rect.y += 53.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pBmp = g_pFresh->GetBitmap("supplypack_tex_1");
			if (pBmp)
			{
				WRect rect = m_rect;
				rect.x += (rect.w - pBmp->Width()) * 0.5f;
				rect.y += 28.0f;
				rect.w = pBmp->Width();
				rect.h = pBmp->Height();
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}
		}

		if (m_bResult)
			m_pFontAnim->Display();
	}

	void FrSupplyPackDlg::OnItemListDraw(int param)
	{
		if (!m_pItemList->IsVisible())
			return;

		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		FrListItem* pListItem = (FrListItem*)param;
		if (!pListItem)
			return;

		float alpha = 255.0f - m_alpha;
		if (Wabs(alpha) < g_EPSILON)
		{
			m_pItemList->SetVisible(false);
			return;
		}

		sItemInfo* pInfo = (sItemInfo*)pListItem->pData;
		if (!pInfo)
		{
			if (pListItem == m_pItemList->GetItemUnderCursor() &&
				m_pItemList->IsEnableToolTip())
				m_pItemList->SetToolTipText(std::string(""));
			return;
		}

		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pInfo->tid);
		if (!pCommon)
			return;

		const Bitmap* pBmp = g_pFresh->GetBitmap(pCommon->Icon);
		if (!pBmp)
			return;

		float dx = (m_pItemList->GetItemWidth() - pBmp->Width()) * 0.5f;
		float dy = (m_pItemList->GetItemHeight() - pBmp->Height()) * 0.5f;
		WRect rect(pListItem->pos.x + dx, pListItem->pos.y + dy, pBmp->Width(),
			pBmp->Height());

		unsigned long color = ((unsigned char)(int)alpha << 24) | 0xffffff;
		pGDI->DrawTexture(pBmp, rect, color, 0);

		m_pFontNum->SetColor(color);
		m_pFontNum->Print(pListItem->pos.x + m_pItemList->GetItemWidth(),
			pListItem->pos.y + m_pItemList->GetItemHeight() -
				m_pFontNum->GetFontH(),
			MakeStr("%d", (int)pInfo->Common[0]), TFA_RIGHT, 1.0f, 1.0f);

		bool bSel = false;
		if (m_pSelected == NULL)
		{
			if (pListItem == m_pItemList->GetItemUnderCursor())
				bSel = true;
		}
		else if (pListItem == m_pSelected)
			bSel = true;

		if (bSel)
		{
			pBmp = g_pFresh->GetBitmap("supplypack_btn_ch_o");
			pGDI->DrawTexture(pBmp,
				WRect(pListItem->pos.x, pListItem->pos.y,
					m_pItemList->GetItemWidth(), m_pItemList->GetItemHeight()),
				color, 0);

			if (m_pItemList->IsEnableToolTip())
				m_pItemList->SetToolTipText(
					std::string(MakeStr("%s", pCommon->Name)));
		}
	}

	void FrSupplyPackDlg::OnItemListUp()
	{
		FrListItem* pSelected = m_pItemList->GetItemUnderCursor();
		if (!pSelected)
			return;

		sItemInfo* pInfo = (sItemInfo*)pSelected->pData;
		if (!pInfo)
			return;

		if (!ItemManager()->FindCommonItem(pInfo->tid))
			return;

		if (m_pExhibition || m_pSelected)
			return;

		m_pSelected = pSelected;
		m_pItemList->EnableToolTip(false);

		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		pForm->SetMessage(
			MakeStr(
				K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xcf\xbd\xc5 \xbe\xc6\xc0\xcc\xc5\xdb '%s'\xc0\xc7 \xbc\xf6\xb7\xae\xc0\xbb \xba\xb8\xc3\xe6 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				ItemManager()->GetItemName(pInfo->tid)),
			false);
		pForm->Open((FRESH_PFN_RESULT)&FrSupplyPackDlg::OnResultConfirm, 1);
	}
}
