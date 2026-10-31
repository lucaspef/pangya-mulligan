#include "minatl.h"

class Bitmap;
// the original instantiates this vector before the dialog's own containers
typedef char _BitmapVectorSize[sizeof(std::vector<const Bitmap*>)];

#include "cardboxdlg.h"
#include "frarea.h"
#include "frbutton.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "titlefontex.h"
#include "exhibition.h"
#include "particle.h"
#define WBONE_GETLOCALMATRIX
#include "wbone.h"
#include "taskmanager.h"
#include "actor.h"
#include "soundmanager.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "wtemplate.h"
#include "mathconsts.h"

#include <algorithm>


extern Fresh* g_pFresh;

// bongdarishopdlg.h (not converted yet): only what this unit uses
struct sBonusBall
{
	int index;
	unsigned long typeId;
	int reserved08;
	int count;
	int reserved10;
};

class FrListBox;
class WTitleFont;

class FrTotalItemDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTotalItemDlg)

	class MiniBtn
	{
	public:
		MiniBtn();

		_WRECT rect;
		const Bitmap* pBitmap;
		int reserved14;
	};

	FrTotalItemDlg();
	virtual ~FrTotalItemDlg();

	void SetListItem(std::list<sBonusBall>* pList);

protected:
	DECLARE_FRESH_MSGMAP()

	virtual bool OnInit();

private:
	FrListBox* m_pTotalItemList;
	const Bitmap* m_pIconBase;
	WTitleFont* m_pTitleFont;
	unsigned long m_reserved11c[18];
	MiniBtn m_deleteBtn;
	MiniBtn m_clockBtn;
	unsigned long m_reserved194[18];
	std::list<sBonusBall> m_bonusBallList;
};

// instantiated by bongdarishopdlg (its RTTI has no symbol of its own there yet)
template <>
FrTotalItemDlg* CreateForm<FrTotalItemDlg>(FrWndManager* pManager,
	FrCmdTarget* pCmdDest, const char* lpszTemplateID, FrWnd* pParent);

IMPLEMENT_OBJECT(FrCardBoxDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrCardBoxDlg, FrForm)

ON_FRESH_VI("ID_BTN_ABOUT", FRCMD_INIT, FrCardBoxDlg::OnResultBtnInit)
ON_FRESH_VV("ID_BTN_ABOUT", FRCMD_LBUTTONUP, FrCardBoxDlg::OnResultBtnUp)
ON_FRESH_VI("ID_BTN_CLOSE", FRCMD_INIT, FrCardBoxDlg::OnCloseBtnInit)
ON_FRESH_VV("ID_BTN_CLOSE", FRCMD_LBUTTONUP, FrCardBoxDlg::OnCloseBtnUp)
ON_FRESH_VI("ID_AREA_BG", FRCMD_INIT, FrCardBoxDlg::OnAreaBgInit)
ON_FRESH_VV("ID_AREA_BG", FRCMD_OWNERDRAW, FrCardBoxDlg::OnAreaBgOwnerDraw)
ON_FRESH_VI("ID_BTN_PACKOPEN", FRCMD_INIT, FrCardBoxDlg::OnPackOpenBtnInit)
ON_FRESH_VV("ID_BTN_PACKOPEN", FRCMD_LBUTTONUP, FrCardBoxDlg::OnPackOpenBtnUp)
ON_FRESH_VI("ID_NEXT_OPEN_OK", FRCMD_INIT, FrCardBoxDlg::OnOpenNextBtnInit)
ON_FRESH_VV("ID_NEXT_OPEN_OK", FRCMD_LBUTTONUP, FrCardBoxDlg::OnOpenNextBtnUp)

END_FRESH_MSGMAP()

FrCardBoxDlg::FrCardBoxDlg()
	: m_pFont(NULL),
	  m_pExhibition(NULL),
	  m_animation(0),
	  m_pPackOpenBtn(NULL),
	  m_pOpenNextBtn(NULL),
	  m_pCloseBtn(NULL),
	  m_pResultBtn(NULL),
	  m_typeId(0),
	  m_uid(0),
	  m_packOpenShake(0.0f)
{
}

FrCardBoxDlg::~FrCardBoxDlg()
{
	m_resultMap.clear();
	if (m_pParticle)
	{
		delete m_pParticle;
		m_pParticle = NULL;
	}
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
}

void FrCardBoxDlg::Init(unsigned long typeId, unsigned long uid, int count)
{
	FrForm::EnableDrag(false);
	std::list<std::string> names;

	for (int i = 0; i < 5; i++)
		names.push_back(std::string(MakeStr("particle_shine_%d", i)));

	m_pParticle = new CParticle2D;
	m_pParticle->Initialize(names, m_bgRect, 20, CParticle2D::STYLE_NORMAL,
		20.0f, 100.0f, WVector2D(0.0f, 1.0f), WVector2D(0.0f, 1.0f));

	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
	if (pCard == NULL || count < 1)
		return;

	m_typeId = typeId;
	m_uid = uid;

	if (((typeId >> 22) & 0xf) == 4)
		LoadCardBoxPet(typeId, count);
	else if (((typeId >> 22) & 0xf) == 3)
		LoadCardPackPet(typeId);
}

void FrCardBoxDlg::RecievedPacket(WReceivedPacket* pPacket)
{
	if (!m_pExhibition)
		return;

	unsigned long result = pPacket->Decode4();
	if (result == 0)
	{
		SetAnimation(ANI_PACK_WAIT);

		sCards card;
		pPacket->DecodeBuffer(&card, sizeof(sCards));
		std::list<sCards>::iterator it =
			std::find_if(CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(card.uid));
		if (it != CSharedDoc::Instance()->m_cardList.end())
		{
			if ((*it).tid == card.tid)
			{
				(*it).cardCount--;
				if ((*it).cardCount < 1)
					CSharedDoc::Instance()->m_cardList.erase(it);
			}
		}

		int count = pPacket->Decode1();
		for (int i = 0; i < count; i++)
		{
			pPacket->DecodeBuffer(&card, sizeof(sCards));
			pPacket->Decode4(); // 983 008c3140 descarta: a contagem vem do sCards

			std::map<unsigned long, unsigned int>::iterator mit =
				m_resultMap.find(card.tid);
			if (mit == m_resultMap.end())
				m_resultMap[card.tid] = 1;
			else
				mit->second++;

			std::list<sCards>::iterator cit = std::find_if(
				CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(card.uid));
			if (cit != CSharedDoc::Instance()->m_cardList.end())
			{
				// 983 008c31a9: total do servidor, não soma (o 645 somava o Decode4)
				if ((*cit).tid == card.tid)
					(*cit).cardCount = card.cardCount;
			}
			else
			{
				CSharedDoc::Instance()->m_cardList.push_back(card);
			}

			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(card.tid);
			if (m_pExhibition && pCard)
			{
				m_pExhibition->ChangeBoneTexture(
					MakeStr("card_base%02d_Dummy", i + 1), "card_back-00.png",
					MakeStr("%s.png", pCard->Image));
			}
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x229), 0, 0, 0, 0, 0));
	}
	else if (result == 1)
	{
		FrForm::OnCancel();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc4\xab\xb5\xe5\xc6\xd1 \xbf\xc0\xc7\xc2\xbf\xa1 \xbd\xc7\xc6\xd0 "
				 "\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0));
	}
}

bool FrCardBoxDlg::OnInit()
{
	if (!m_pFont)
		m_pFont = new CWindFont;

	return true;
}

void FrCardBoxDlg::OnProc(const float dt)
{
	if (!m_pExhibition)
		return;

	w_motion_data* pMotion = m_pExhibition->GetMotionData();
	if (!pMotion)
		return;

	switch (m_animation)
	{
	case ANI_BOX_OPEN:
		if (stricmp(pMotion->name, "default\xb3\xa1") == 0)
		{
			if (m_typeId == 0x7d000000)
				LoadCardPackPet(0x7cc00000);
		}
		break;

	case ANI_PACK_DOWN:
		if (stricmp(pMotion->name,
				"\xc6\xd1\xb4\xd9\xbf\xee\xc1\xbe\xb7\xe1") == 0)
		{
			if (m_pPackOpenBtn && !m_pPackOpenBtn->IsVisible())
				m_pPackOpenBtn->SetVisible(true);
		}
		break;

	case ANI_PACK_TURN:
		if (stricmp(pMotion->name,
				"\xc6\xd1\xc1\xa4\xc1\xf6\xbb\xf3\xc5\xc2") == 0)
			SetAnimation(ANI_PACK_TURN);
		break;

	case ANI_PACK_WAIT:
		if (stricmp(pMotion->name,
				"\xc6\xd1\xc1\xa4\xc1\xf6\xbb\xf3\xc5\xc2") == 0)
			SetAnimation(ANI_PACK_OPEN);
		break;

	case ANI_PACK_OPEN:
		if (stricmp(pMotion->name,
				"\xc6\xd1\xbf\xc0\xc7\xc2\xc1\xbe\xb7\xe1") == 0)
			SetAnimation(ANI_ZOOM_IN);
		break;

	case ANI_ZOOM_IN:
	{
		float distance = m_pExhibition->GetDistance();
		// 983 008c23c0: zoom ends at 5.251 (00d378f0), speed dt * (double)4.949f * 3.0 (00d378e8, 00d66cf8); the 645 used 4.8 / 16.2
		if (distance > 5.251f)
		{
			distance = (float)(distance - dt * (double)4.949f * 3.0);
			if (distance < 5.251f)
				distance = 5.251f;

			m_pExhibition->SetDistance(distance, true, true);
		}
		else
		{
			m_pExhibition->SetDistance(5.251f, true, true);
			SetAnimation(ANI_RESULT);

			std::list<sCards>::iterator it = std::find_if(
				CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(m_uid));
			if (it != CSharedDoc::Instance()->m_cardList.end())
			{
				if (m_pOpenNextBtn)
					m_pOpenNextBtn->SetVisible(true);
			}

			m_pCloseBtn->Enable(true);
		}
		break;
	}

	case ANI_CARD_COVER:
		if (stricmp(pMotion->name,
				"\xc4\xab\xb5\xe5\xb9\xf6\xb8\xb2\xc1\xbe\xb7\xe1") == 0)
		{
			m_pExhibition->SetDistance(10.2f, true, true);

			std::list<sCards>::iterator it = std::find_if(
				CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(m_uid));
			if (it != CSharedDoc::Instance()->m_cardList.end())
			{
				if (((*it).tid & 0x3c00000) == 0x1000000)
					LoadCardBoxPet((*it).tid, (*it).cardCount);
				else
					SetAnimation(ANI_PACK_DOWN);
			}
			else
			{
				FrForm::OnCancel();
			}
		}
		break;
	}

	if (m_pExhibition)
		m_pExhibition->ProcessBongdariShop(dt);

	if (m_pParticle)
		m_pParticle->Process(dt);

	if (m_pPackOpenBtn && m_pPackOpenBtn->IsVisible())
	{
		static bool s_bShakeBack = false;

		if (s_bShakeBack)
		{
			if (SAFE_SUBVALUE(m_packOpenShake, 0.0f, dt * 15.0f) <= 0.0f)
				s_bShakeBack = false;
		}
		else
		{
			if (SAFE_ADDVALUE(m_packOpenShake, 15.0f, dt * 120.0f) >= 15.0f)
				s_bShakeBack = true;
		}

		WRect rect = m_packOpenRect;
		rect.x += m_packOpenShake;
		m_pPackOpenBtn->SetRect(rect);
	}
}

void FrCardBoxDlg::OnOK()
{
}

// 983 008c1920: Esc fecha nos estados 1 e 6, com o botao close ativo
void FrCardBoxDlg::OnCancel()
{
	if (m_pCloseBtn->IsEnabled() && (m_animation == 6 || m_animation == 1))
		FrForm::OnCancel();
}

void FrCardBoxDlg::OnResultBtnInit(int param)
{
	m_pResultBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	m_pResultBtn->SetPushSound("ui_icon_click");
}

void FrCardBoxDlg::OnResultBtnUp()
{
	std::list<sBonusBall> resultList;
	FrTotalItemDlg* pDlg =
		CreateForm<FrTotalItemDlg>(g_pFresh->GetManager(), this, "total_item");
	if (pDlg)
	{
		for (std::map<unsigned long, unsigned int>::iterator it =
				 m_resultMap.begin();
			it != m_resultMap.end(); ++it)
		{
			sBonusBall ball;
			memset(&ball, 0, sizeof(ball));
			ball.typeId = it->first;
			ball.count = it->second;
			resultList.push_back(ball);
		}

		pDlg->SetListItem(&resultList);
		pDlg->Open(NULL, 3);
	}
}

void FrCardBoxDlg::OnCloseBtnInit(int param)
{
	m_pCloseBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrCardBoxDlg::OnCloseBtnUp()
{
	FrForm::OnCancel();
}

void FrCardBoxDlg::OnAreaBgInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
		m_bgRect = pArea->GetRect();
	else
		m_bgRect = GetRect();
}

namespace {
// 983: durante o desenho o estado de video (vtbl +188h / +174h do WVideoDev) e
// guardado e zerado; restaurado na saida (sem destrutor: o 983 nao tem EH aqui).
typedef bool(__thiscall* VideoGetFn983)(void*);
typedef void(__thiscall* VideoSetFn983)(void*, bool);
inline bool SaveVideoState983()
{
	void* dev = g_resrcmng->VideoReference();
	void** vt = *(void***)dev;
	bool old = ((VideoGetFn983)vt[0x188 / 4])(dev);
	((VideoSetFn983)vt[0x174 / 4])(dev, false);
	return old;
}
inline void RestoreVideoState983(bool old)
{
	void* dev = g_resrcmng->VideoReference();
	((VideoSetFn983)(*(void***)dev)[0x174 / 4])(dev, old);
}
}

void FrCardBoxDlg::OnAreaBgOwnerDraw()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!m_pExhibition)
		return;
	bool videoOld = SaveVideoState983();

	if (m_animation == ANI_BOX_OPEN)
		SetPackVisibleInBox();

	Display2DBg();

	m_pExhibition->DisplayClip(m_bgRect, 55.0f, 0.0f, 0.0f);

	if (m_animation == ANI_RESULT)
	{
		std::list<sCards>::iterator it =
			std::find_if(CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(m_uid));
		if (it == CSharedDoc::Instance()->m_cardList.end())
		{
			const Bitmap* pEmpty = g_pFresh->GetBitmap("text_cardpack_empty");
			if (!pEmpty)
				goto done;

			WRect rect(m_bgRect.x +
					(m_bgRect.w - (float)pEmpty->Width()) * 0.5f,
				m_bgRect.y + m_bgRect.h * 0.9f, (float)pEmpty->Width(),
				(float)pEmpty->Height());
			pGDI->DrawTexture(pEmpty, rect, 0xffffffff, 0);
		}
		else
		{
			const Bitmap* pNext =
				g_pFresh->GetBitmap("text_cardpack_open_next");
			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(m_typeId);
			if (!pCard)
				goto done;
			const Bitmap* pIcon = g_pFresh->GetBitmap(pCard->c.Icon);

			if (!pIcon || !pNext)
				goto done;

			WRect rect = m_bgRect;
			rect.x += 80.0f;
			rect.y += rect.h - (float)pIcon->Height() - 4.0f;
			rect.w = (float)pIcon->Width();
			rect.h = (float)pIcon->Height();

			pGDI->DrawTexture(pIcon, rect, 0xffffffff, 0);

			rect.x += 50.0f;
			rect.y += (float)pIcon->Height() * 0.6f;

			float width;
			std::list<sCards>::iterator cit = std::find_if(
				CSharedDoc::Instance()->m_cardList.begin(),
				CSharedDoc::Instance()->m_cardList.end(), _FindByUid(m_uid));
			if (cit != CSharedDoc::Instance()->m_cardList.end())
			{
				width = m_pFont->Print(rect.x, rect.y,
					MakeStr("x %02d",
						(*cit).cardCount > 99 ? 99 : (*cit).cardCount));
			}
			else
			{
				width = m_pFont->Print(rect.x, rect.y, MakeStr("x %02d", -1));
			}

			rect.x += width;

			rect.x += 10.0f;
			rect.y -= 4.0f;
			rect.w = (float)pNext->Width();
			rect.h = (float)pNext->Height();

			pGDI->DrawTexture(pNext, rect, 0xffffffff, 0);
		}
	}
done:
	RestoreVideoState983(videoOld);
}

void FrCardBoxDlg::OnPackOpenBtnInit(int param)
{
	m_pPackOpenBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pPackOpenBtn)
	{
		m_packOpenRect = m_pPackOpenBtn->GetRect();
		m_pPackOpenBtn->SetPushDelay(0.3f);
		m_pPackOpenBtn->SetVisible(false);
		m_pPackOpenBtn->SetPushSound("cardpack_open_click");
	}
}

void FrCardBoxDlg::OnPackOpenBtnUp()
{
	if (m_pPackOpenBtn->IsVisible() && m_pExhibition)
	{
		m_pCloseBtn->Enable(false);
		m_pPackOpenBtn->SetVisible(false);

		SetAnimation(ANI_PACK_TURN);

		WSendPacket packet((enumClientPacket)0xca);
		packet.Encode4(m_typeId);
		packet.Encode4(m_uid);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}
}

void FrCardBoxDlg::OnOpenNextBtnInit(int param)
{
	m_pOpenNextBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pOpenNextBtn)
	{
		m_pOpenNextBtn->SetPushDelay(0.3f);
		m_pOpenNextBtn->SetVisible(false);
		m_pOpenNextBtn->SetPushSound("ui_icon_click");
	}
}

void FrCardBoxDlg::OnOpenNextBtnUp()
{
	if (m_pOpenNextBtn)
		m_pOpenNextBtn->SetVisible(false);
	SetAnimation(ANI_CARD_COVER);
}

void FrCardBoxDlg::SetAnimation(unsigned long animation)
{
	if (!m_pExhibition)
		return;

	m_animation = animation;
	switch (animation)
	{
	case ANI_BOX_OPEN:
		m_pExhibition->SetMotion("default", false, 0.0f);
		g_audio->PlaySfx("cardbox_open", NULL, 0, 0, 0, 0.5f, 200.0f);
		break;
	case ANI_PACK_DOWN:
		m_pExhibition->SetMotion("\xc6\xd1\xb4\xd9\xbf\xee\xbd\xc3\xc0\xdb",
			false, 0.0f);
		break;
	case ANI_PACK_TURN:
		m_pExhibition->SetMotion("\xc6\xd1\xb5\xb9\xb8\xae\xb1\xe2", false,
			0.0f);
		break;
	case ANI_PACK_OPEN:
		m_pExhibition->SetMotion("\xc6\xd1\xbf\xc0\xc7\xc2\xbd\xc3\xc0\xdb",
			false, 0.0f);

		if ((m_typeId & 0x3c00000) == 0xc00000 && m_typeId != 0x7cc00000 &&
			m_typeId != 0x7cc00004)
		{
			g_audio->PlaySfx("cardpack_open_one", NULL, 0, 0, 0, 0.5f, 200.0f);
		}
		else
		{
			g_audio->PlaySfx("cardpack_open_multi", NULL, 0, 0, 0, 0.5f,
				200.0f);
		}
		break;
	case ANI_CARD_COVER:
		m_pExhibition->SetMotion(
			"\xc4\xab\xb5\xe5\xb9\xf6\xb8\xb2\xbd\xc3\xc0\xdb", false, 0.0f);
		break;
	}
}

void FrCardBoxDlg::LoadCardBoxPet(unsigned long typeId, int count)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
	if (pCard == NULL)
		return;

	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}

	m_pExhibition = new CExhibitionByHardwareTnL(m_bgRect.x + m_bgRect.w * 0.5f,
		m_bgRect.y + m_bgRect.h * 0.5f, 0.0f, true);

	m_pExhibition->SetDistance(10.8f, true, false);
	m_pExhibition->SetArea(m_bgRect.w, m_bgRect.h);

	if (!m_pExhibition->SetModel(MakeStr("%s_open", pCard->Image),
			(eExhibitType)12))
		return;

	if (count < 10)
	{
		WBone* pRoot = m_pExhibition->GetModel()->GetRootBone();
		if (pRoot == NULL)
			return;

		WBone* pMain = pRoot->FindBone("Main_Dummy01", 0);
		WBone* pPack =
			pRoot->FindBone(MakeStr("card_pack%02d_Dummy", 10 - count), 0);
		if (pMain && pPack)
		{
			WVector move =
				pMain->GetLocalMatrix().pivot - pPack->GetLocalMatrix().pivot;

			move.y = pMain->GetMatrix().ym;
			pMain->Move(move, false);
		}
	}

	SetAnimation(ANI_BOX_OPEN);
}

void FrCardBoxDlg::LoadCardPackPet(unsigned long typeId)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
	if (pCard == NULL)
		return;

	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}

	m_pExhibition = new CExhibitionByHardwareTnL(m_bgRect.x + m_bgRect.w * 0.5f,
		m_bgRect.y + m_bgRect.h * 0.5f, -0.01f, true);

	m_pExhibition->SetDistance(10.2f, true, false);
	m_pExhibition->SetArea(m_bgRect.w, m_bgRect.h);
	if (!m_pExhibition->SetModel(MakeStr("%s_open", pCard->Image),
			(eExhibitType)12))
		return;

	SetAnimation(ANI_PACK_DOWN);
}

void FrCardBoxDlg::SetPackVisibleInBox()
{
	std::list<sCards>::iterator it =
		std::find_if(CSharedDoc::Instance()->m_cardList.begin(),
			CSharedDoc::Instance()->m_cardList.end(), _FindByUid(m_uid));
	if (it == CSharedDoc::Instance()->m_cardList.end())
		return;
	int count = (*it).cardCount;

	if (count >= 10)
		return;

	WBone* pRoot = m_pExhibition->GetModel()->GetRootBone();
	for (int i = 0; i < 10 - count; i++)
	{
		WBone* pPack =
			pRoot->FindBone(MakeStr("card_pack%02d_Dummy", i + 1), 0);
		if (pPack)
			pPack->SetInvisible(false, true);
	}
}

void FrCardBoxDlg::Display2DBg()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	float x = m_bgRect.x;
	float y = m_bgRect.y;

	const Bitmap* pBitmap = g_pFresh->GetBitmap("bg_cardcenter_0");
	if (pBitmap)
		pGDI->DrawTexture(pBitmap,
			WRect(x, y - 28.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height()),
			0xffffffff, 0);
	pBitmap = g_pFresh->GetBitmap("bg_cardcenter_1");
	if (pBitmap)
		pGDI->DrawTexture(pBitmap,
			WRect(x + 256.0f, y - 28.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height()),
			0xffffffff, 0);
	pBitmap = g_pFresh->GetBitmap("bg_cardcenter_2");
	if (pBitmap)
		pGDI->DrawTexture(pBitmap,
			WRect(x, y + 256.0f - 28.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height()),
			0xffffffff, 0);
	pBitmap = g_pFresh->GetBitmap("bg_cardcenter_3");
	if (pBitmap)
		pGDI->DrawTexture(pBitmap,
			WRect(x + 256.0f, y + 256.0f - 28.0f, (float)pBitmap->Width(),
				(float)pBitmap->Height()),
			0xffffffff, 0);

	if (m_pParticle)
		m_pParticle->Display();
}
