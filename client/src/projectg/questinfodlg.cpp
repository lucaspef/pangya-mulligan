#include "minatl.h"
#include "questinfodlg.h"

#include "wfont.h"
#include "actor.h"
#include "../shared/sharedtables.h"
#include "wresrcmng.h"
#include <string.h>

extern Fresh* g_pFresh;

extern WResourceManager* g_resrcmng;
extern WView* g_view;

struct QuestReward
{
	unsigned long flag;
	unsigned long typeId;
	unsigned long quantity;
};
static const QuestReward questRewards[20] = {	// 00d27bd0 (983)
	{ 0x1,    0x1a000002, 0xa   },
	{ 0x2,    0x1800000b, 0x3   },
	{ 0x4,    0x18000025, 0x3   },
	{ 0x8,    0x18000005, 0x3   },
	{ 0x10,   0x18000004, 0x3   },
	{ 0x20,   0x1800000a, 0x3   },
	{ 0x40,   0x18000000, 0x3   },
	{ 0x80,   0x1a000010, 0x3e8 },
	{ 0x100,  0x1a000002, 0xa   },
	{ 0x200,  0x18000028, 0x1   },
	{ 0x400,  0x18000006, 0x1   },
	{ 0x800,  0x1800000a, 0x3   },
	{ 0x1000, 0x18000000, 0x4   },
	{ 0x2000, 0x18000001, 0x4   },
	{ 0x8000, 0x18000005, 0x3   },
	{ 0x10000,0x18000005, 0x3   },
	{ 0x20000,0x18000005, 0x3   },
	{ 0x40000,0x18000005, 0x3   },
	{ 0x80000,0x18000005, 0x3   },
	{ 0x80000,0x18000005, 0x3   },
};
// 00d27cc0 (983): presente da serie (SetData); flag = indice da quantidade em OnGiftListOwnerDraw
static const QuestReward gradeGifts[4] = {
	{ 0, 0x10000014, 0x1    },
	{ 1, 0x1c000000, 0x1    },
	{ 2, 0x1a000010, 0x2710 },
	{ 3, 0x18000027, 0xa    },
};

struct QuestText
{
	unsigned char index;
	bool complete;
	const char* name;
	const char* npc;
	const char* description;
	const char* message;
};
static QuestText questTexts[14] = {
	{ 0,  false, K2L_Compatibility("\305\254\267\264\274\261\305\303"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\270\360\270\247\301\366\261\342 \270\305\273\347\277\243 \261\342\303\312\260\241 \306\260\306\260\307\330\276\337 \307\317\264\302 \271\375! \306\316\276\337\300\307 \261\342\272\273\260\372 \305\254\267\264 \274\261\305\303 \271\346\271\375\300\273 \276\313\276\306\272\270\300\332!"),
     K2L_Compatibility(
			"\307\245\275\303\265\310 \260\367\277\241 \263\253\307\317\301\366\301\241\012\277\305\261\342\261\342")                                         },
	{ 1,  false, K2L_Compatibility("\305\270\261\270\271\346\271\375"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\306\316\276\337! \270\332\301\370 \274\246\300\273 \307\317\264\302 \271\346\271\375\277\241 \264\353\307\330 \276\313\276\306\272\270\300\332!"),
     K2L_Compatibility(
			"\307\245\275\303\265\310 \260\367\300\270\267\316\012\274\246 \303\304\272\270\261\342")                                                         },
	{ 2,  false, K2L_Compatibility("\306\316\276\337\260\324\300\314\301\366"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\264\365 \270\326\270\256! \264\365 \260\255\307\317\260\324! \306\316\276\337\304\336\272\270\260\324\300\314\301\366\270\246 \273\347\277\353\307\317\277\251 \306\304\277\366\274\246\300\273 \303\304\272\270\300\332!"),
     K2L_Compatibility(
			"\306\316\276\337\304\336\272\270\260\324\300\314\301\366\270\246\012\273\347\277\353\307\317\277\251 \306\304\277\366\274\246 \304\241\261\342") },
	{ 3,  false, K2L_Compatibility("\306\333\306\303\300\307 \261\342\272\273"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\310\246\300\307 \270\266\271\253\270\256! \306\333\306\303\277\241 \264\353\307\330 \271\350\277\366\272\270\300\332"),
     K2L_Compatibility("\306\333\306\303 \271\350\277\354\261\342")																						   },
	{ 4,  false, K2L_Compatibility("\276\306\300\314\305\333\273\347\277\353"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\276\321, \300\314\267\261 \275\305\261\342\307\321 \276\306\300\314\305\333\300\314? \276\306\300\314\305\333 \273\347\277\353\271\375\300\273 \276\313\276\306\272\270\300\332!"),
     K2L_Compatibility(
			"\276\306\300\314\305\333 \273\347\277\353\300\270\267\316\012\306\304\277\366\274\246 \304\241\261\342")                                         },
	{ 5,  false, K2L_Compatibility("\301\366\270\351\274\323\274\272"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\307\256\271\347, \270\360\267\241\271\347, \271\331\264\331, \277\254\270\370... \264\331\276\347\307\321 \301\366\270\351\277\241 \264\353\307\330 \260\370\272\316\307\317\300\332!"),
     K2L_Compatibility(
			"\304\373\301\356\267\316 \276\313\276\306\272\270\264\302\012\301\366\270\351\300\307 \274\323\274\272")                                         },
	{ 6,  false, K2L_Compatibility("\275\272\304\332\276\356\260\350\273\352"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\276\313\275\366\264\336\275\366? \306\316\276\337\300\307 \301\241\274\366 \260\350\273\352\271\375\300\273 \276\313\276\306\272\270\300\332!"),
     K2L_Compatibility(
			"\304\373\301\356\267\316 \276\313\276\306\272\270\264\302\012\306\316\276\337\300\307 \301\241\274\366")                                         },
	{ 7,  false,
     K2L_Compatibility(
			"4\310\246\277\241\274\255 \310\246\300\316\307\317\261\342"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\303\312\272\270 \264\353\305\273\303\342! \301\366\261\335\261\356\301\366 \271\350\277\356 \270\360\265\347 \260\315\300\273 \300\300\277\353\307\317\277\251 \275\307\275\300 \265\265\300\374!"),
     K2L_Compatibility(
			"\306\304 4\310\246\277\241\274\255\012Par\300\314\273\363 \261\342\267\317\307\317\261\342")                                                     },
	{ 8,  false,
     K2L_Compatibility("\260\370 \277\305\261\342\261\342"),                     K2L_Compatibility("\272\300\264\331\270\256"), K2L_Compatibility("\265\265\300\372\310\367 \274\246\300\273 \304\245 \274\366 \276\370\264\302 \273\363\310\262\300\314\266\363\270\351? \260\370\277\305\261\342\261\342 \261\342\264\311\300\273 \271\350\277\366\272\270\300\332!"),
     K2L_Compatibility(
			"\260\370\277\305\261\342\261\342 \261\342\264\311 \276\313\276\306\272\270\261\342")                                                             },
	{ 9,  false, K2L_Compatibility("\260\370 \261\342\277\357\261\342"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\301\366\270\351\300\307 \261\342\277\357\261\342\270\246 \300\320\260\355 \264\365\277\355 \301\244\261\263\307\321 \274\246\277\241 \265\265\300\374\307\317\300\332!"),
     K2L_Compatibility(
			"\301\366\270\351\300\307 \261\342\277\357\261\342\270\246 \300\320\264\302 \271\375")                                                            },
	{ 10, false, K2L_Compatibility("\271\331\266\367 \300\320\261\342"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\271\331\266\367\300\273 \300\320\260\355 \260\370\300\307 \261\313\300\373\300\273 \277\271\303\370\307\317\266\363!"),
     K2L_Compatibility(
			"\271\331\266\367\300\307 \274\274\261\342\277\315 \271\346\307\342 \276\313\276\306\272\270\261\342")                                            },
	{ 11, false, K2L_Compatibility("\305\270\301\241\300\314\265\277"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\260\370\300\307 \305\270\301\241\300\273 \301\266\300\375\307\317\277\251 \261\313\300\373\300\307 \260\242\265\265\270\246 \271\331\262\343\272\270\300\332!"),
     K2L_Compatibility(
			"\260\370\300\307 \305\270\301\241 \301\266\300\375\307\317\261\342")                                                                             },
	{ 12, false, K2L_Compatibility("\306\304\277\366\275\272\307\311"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\260\255\267\302\307\321 \275\272\307\311! \306\304\277\366\275\272\307\311\277\241 \264\353\307\330 \276\313\276\306\272\270\300\332."),
     K2L_Compatibility(
			"\306\304\277\366 \275\272\307\311 \274\246 \300\315\310\367\261\342")                                                                            },
	{ 13, false, K2L_Compatibility("\306\304\277\366\304\277\272\352"),
     K2L_Compatibility("\272\300\264\331\270\256"),
     K2L_Compatibility(
			"\260\305\264\353\307\321 \300\345\276\326\271\260\265\265 \277\344\270\256\301\266\270\256 \272\361\304\321\260\241\264\302 \306\304\277\366 \304\277\272\352\274\246\300\273 \271\350\277\366\272\270\300\332!"),
     K2L_Compatibility(
			"\300\345\276\326\271\260\300\273 \307\307\307\330 \270\361\307\245\301\366\301\241\300\270\267\316 \260\370 \272\270\263\273\261\342")           },
};

IMPLEMENT_ACTOR(FrQuestInfoDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrQuestInfoDlg, FrForm)

ON_FRESH_VI("quest_info", FRCMD_INIT, FrQuestInfoDlg::OnQuestInfoInit)
ON_FRESH_VI("reward_item", FRCMD_INIT, FrQuestInfoDlg::OnRewardItemListInit)
ON_FRESH_VI("reward_item", FRCMD_OWNERDRAW,
	FrQuestInfoDlg::OnRewardItemListOwnerDraw)
ON_FRESH_VI("name", FRCMD_INIT, FrQuestInfoDlg::OnQuestNameEditInit)
ON_FRESH_VI("message", FRCMD_INIT, FrQuestInfoDlg::OnQuestMessageEditInit)
ON_FRESH_VI("explane", FRCMD_INIT, FrQuestInfoDlg::OnQuestExplaneEditInit)

END_FRESH_MSGMAP()

FrQuestInfoDlg::FrQuestInfoDlg()
{
	m_pRewardItemList = NULL;
	m_pItemFrame = NULL;
	m_bReward = false;
	m_questIndex = 0;
	m_pQuestName = NULL;
	m_pQuestMessage = NULL;
	m_pQuestExplane = NULL;
	static char* filenames[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = filenames;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);
}

FrQuestInfoDlg::~FrQuestInfoDlg()
{
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void FrQuestInfoDlg::OnQuestInfoInit(int)
{
}

void FrQuestInfoDlg::OnRewardItemListInit(int param)
{
	m_pRewardItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pRewardItemList)
	{
		m_pRewardItemList->SetPushSound(NULL);
		m_pRewardItemList->ClearItem();
		m_pItemFrame = g_pFresh->RegisterBitmap("re_itemframe_n");
	}
}

void FrQuestInfoDlg::OnRewardItemListOwnerDraw(int param)
{
	if (!param)
		return;
	FrListItem* item = (FrListItem*)param;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	float offsetX = 0.0f, offsetY = 0.0f;
	if (m_pRewardItemList->m_itemList.size() > 1)
	{
		switch (item->idx)
		{
		case 0:
			offsetY = 7.0f;
			offsetX = 20.0f;
			break;
		case 1:
			offsetY = 7.0f;
			offsetX = 20.0f;
			break;
		}
	}
	else
	{
		offsetY = 7.0f;
		offsetX = 100.0f;
	}
	device->SetTextStyle(0);
	if (m_pItemFrame)
		device->DrawTexture(m_pItemFrame,
			WRect(item->pos.x + offsetX, item->pos.y + offsetY,
				(float)m_pItemFrame->Width(), (float)m_pItemFrame->Height()),
			0xffffffff, 0);
	unsigned long typeId = (unsigned long)item->pData;
	if (typeId)
	{
		const char* icon = "caddie_no";
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
		if (common)
		{
			device->SetTextStyle(1);
			WPoint pos(item->pos.x + 160.0f, item->pos.y + 7.0f);
			pos.x += offsetX;
			pos.y += offsetY;
			device->Print(pos, 2, "%s", common->Name);
			device->SetTextStyle(0);
			icon = common->Icon;
		}
		const Bitmap* bitmap = g_pFresh->GetBitmap(icon);
		if (bitmap)
		{
			float x, y;
			switch (typeId >> 26)
			{
			case 1:
			case 2:
			case 8:
			case 9:
			case 15:
			case 16:
			case 28:
				x = 7.0f;
				y = 2.0f;
				break;

			case 11:
				x = 9.0f;
				y = 12.0f;
				break;
			case 14:
				if ((typeId & 0x3c00000) == 0x800000)
				{
					x = 7.0f;
					y = 12.0f;
				}
				else
				{
					x = 16.0f;
					y = 27.0f;
				}
				break;
			default:
				x = 10.0f;
				y = 20.0f;
				break;
			}
			device->DrawTexture(bitmap,
				WRect((item->pos.x + x) + offsetX, (item->pos.y + y) + offsetY,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
			{
				int count = questRewards[m_questIndex].quantity;
				const char* quantity = MakeStr("%d", count);
				float posX = item->pos.x + 70.0f;
				for (count = 0; count < strlen(quantity); ++count)
					posX -= m_pTitleFont->GetCharWidth(g_view,
						quantity[count] - '0');
				m_pTitleFont->Print(g_view, posX + offsetX - 10.0f,
					item->pos.y + offsetY + 50.0f, quantity, 0, 0xffffffff,
					NULL);
			}
		}
	}
}

void FrQuestInfoDlg::OnQuestNameEditInit(int param)
{
	m_pQuestName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrQuestInfoDlg::OnQuestMessageEditInit(int param)
{
	m_pQuestMessage = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrQuestInfoDlg::OnQuestExplaneEditInit(int param)
{
	m_pQuestExplane = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrQuestInfoDlg::SetQuestExplane(int index)
{
	m_pQuestName->AddText(MakeStr("%d : %s", index + 1, questTexts[index].name),
		false, true);
	m_pQuestMessage->AddText(questTexts[index].message, false, true);
	m_pQuestExplane->AddText(questTexts[index].description, false, true);
	if (questRewards[index].typeId)
	{
		m_pRewardItemList->AddItem((void*)questRewards[index].typeId);
		m_bReward = true;
	}
	else if (index == 7)
	{
		if (Doc()->m_myInfo.info.gender % 2 == 0)
			m_pRewardItemList->AddItem((void*)0x800a010);
		else
			m_pRewardItemList->AddItem((void*)0x8048009);
		m_bReward = true;
	}
	m_questIndex = index;
}

IMPLEMENT_ACTOR(FrQuestGift, FrForm)

BEGIN_FRESH_MSGMAP(FrQuestGift, FrForm)

ON_FRESH_VI("reward_item", FRCMD_INIT, FrQuestGift::OnGiftListInit)
ON_FRESH_VI("reward_item", FRCMD_OWNERDRAW, FrQuestGift::OnGiftListOwnerDraw)
ON_FRESH_VV("reward_item", FRCMD_LBUTTONDOWN, FrQuestGift::OnGiftListLBtnDown)
ON_FRESH_VI("select", FRCMD_INIT, FrQuestGift::OnSelectInit)
ON_FRESH_VI("itemback4", FRCMD_INIT, FrQuestGift::OnItemBack4)
ON_FRESH_VI("itemback5", FRCMD_INIT, FrQuestGift::OnItemBack5)
ON_FRESH_VI("itemback6", FRCMD_INIT, FrQuestGift::OnItemBack6)
ON_FRESH_VI("itemback7", FRCMD_INIT, FrQuestGift::OnItemBack7)
ON_FRESH_VI("itemback8", FRCMD_INIT, FrQuestGift::OnItemBack8)
ON_FRESH_VI("itemback9", FRCMD_INIT, FrQuestGift::OnItemBack9)
ON_FRESH_VI("ok", FRCMD_INIT, FrQuestGift::OnOkInit)

END_FRESH_MSGMAP()

FrQuestGift::FrQuestGift()
	: m_pGiftList(NULL), m_pSelect(NULL), m_pOk(NULL)
{
	memset(m_pItemBack, 0, sizeof(m_pItemBack));
	m_questIndex = 0;
	m_pItemFrame = NULL;
	m_pNotify = NULL;
	m_bDataSet = false;
	static char* filenames[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = filenames;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);
}

FrQuestGift::~FrQuestGift()
{
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void FrQuestGift::OnGiftListInit(int param)
{
	m_pGiftList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pItemFrame = g_pFresh->RegisterBitmap("re_itemframe_n");
}

void FrQuestGift::OnGiftListOwnerDraw(int param)
{
	if (!param)
		return;
	FrListItem* item = (FrListItem*)param;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	float offsetX = 0.0f, offsetY = 0.0f;
	if (m_pGiftList->m_itemList.size() == 2)
	{
		switch (item->idx)
		{
		case 0:
			offsetY = 7.0f;
			offsetX = 5.0f;
			break;
		case 1:
			offsetY = 7.0f;
			offsetX = 70.0f;
			break;
		}
	}
	else if (m_pGiftList->m_itemList.size() == 3)
	{
		switch (item->idx)
		{
		case 0:
			offsetY = 7.0f;
			offsetX = 5.0f;
			break;
		case 1:
			offsetY = 7.0f;
			offsetX = 70.0f;
			break;
		case 2:
			offsetX = 5.0f;
			offsetY = 10.0f;
			break;
		}
	}
	else
	{
		offsetY = 7.0f;
		offsetX = 100.0f;
	}
	device->SetTextStyle(0);
	if (m_pItemFrame)
		device->DrawTexture(m_pItemFrame,
			WRect(item->pos.x + offsetX, item->pos.y + offsetY,
				(float)m_pItemFrame->Width(), (float)m_pItemFrame->Height()),
			0xffffffff, 0);
	// 007d1180: com SetData o item e a entrada de gradeGifts, que da o tid e o indice da quantidade
	unsigned long typeId = (unsigned long)item->pData;
	if (m_bDataSet && typeId)
	{
		const QuestReward* gift = (const QuestReward*)item->pData;
		m_questIndex = (unsigned char)gift->flag;
		typeId = gift->typeId;
	}
	if (typeId)
	{
		const char* icon = "caddie_no";
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
		if (common)
		{
			device->SetTextStyle(1);
			WPoint pos(item->pos.x + 160.0f, item->pos.y + 7.0f);
			pos.x += offsetX;
			pos.y += offsetY;
			device->Print(pos, 2, "%s", common->Name);
			device->SetTextStyle(0);
			icon = common->Icon;
		}
		const Bitmap* bitmap = g_pFresh->GetBitmap(icon);
		if (bitmap)
		{
			float x, y;
			switch (typeId >> 26)
			{
			case 2:
				x = 7.0f;
				y = -5.0f;
				break;
			case 1:
			case 8:
			case 9:
			case 15:
			case 16:
			case 28:
				x = 7.0f;
				y = 2.0f;
				break;
			case 11:
				x = 9.0f;
				y = 12.0f;
				break;
			case 14:
				if ((typeId & 0x3c00000) == 0x800000)
				{
					x = 7.0f;
					y = 12.0f;
				}
				else
				{
					x = 16.0f;
					y = 27.0f;
				}
				break;
			default:
				x = 10.0f;
				y = 20.0f;
				break;
			}
			device->DrawTexture(bitmap,
				WRect((item->pos.x + x) + offsetX, (item->pos.y + y) + offsetY,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
			const QuestReward* rewards = m_bDataSet ? gradeGifts : questRewards;
			if (rewards[m_questIndex].quantity != 1)
			{
				int count = rewards[m_questIndex].quantity;
				const char* quantity = MakeStr("%d", count);
				float posX = item->pos.x + 70.0f;
				for (count = 0; count < strlen(quantity); ++count)
					posX -= m_pTitleFont->GetCharWidth(g_view,
						quantity[count] - '0');
				m_pTitleFont->Print(g_view, posX + offsetX - 10.0f,
					item->pos.y + offsetY + 50.0f, quantity, 0, 0xffffffff,
					NULL);
			}
		}
	}
}

void FrQuestGift::OnGiftListLBtnDown()
{
	if (m_pGiftList && !m_bDataSet && m_pGiftList->m_itemList.size() != 1)
	{
		FrListItem* item = m_pGiftList->GetItemUnderCursor();
		if (item)
		{
			unsigned long typeId = (unsigned long)item->pData;
			if (typeId)
			{
				WSendPacket packet((enumClientPacket)0xb0);
				packet.Encode4(typeId);
				packet.Send(TO_GAME);
				m_pNotify =
					CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
				m_pNotify->SetMessage(
					K2L_Compatibility("\274\261\305\303\307\321 \274\261\271\260\300\273 \277\344\303\273\307\317\277\264\275\300\264\317\264\331."),
					false);

				m_pNotify->Open((FRESH_PFN_RESULT)&FrQuestGift::OnNotifyResult,
					WPoint((g_view->GetWidth() - m_pNotify->GetRect().w) * 0.5f,
						80.0f),
					FrENTER);
				m_pNotify->EnableDrag(false);
				m_pNotify->MoveCursor("ok");
			}
		}
	}
}

void FrQuestGift::OnSelectInit(int param)
{
	m_pSelect = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pSelect)
		m_pSelect->SetVisible(false);
}

void FrQuestGift::OnItemBack4(int param)
{
	m_pItemBack[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnItemBack5(int param)
{
	m_pItemBack[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnItemBack6(int param)
{
	m_pItemBack[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnItemBack7(int param)
{
	m_pItemBack[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnItemBack8(int param)
{
	m_pItemBack[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnItemBack9(int param)
{
	m_pItemBack[5] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuestGift::OnOkInit(int param)
{
	m_pOk = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

bool FrQuestGift::OnNotifyResult(int, FrForm*)
{
	Close(FrOK, true);
	return true;
}

void FrQuestGift::SetQuestExplane(int index)
{
	if (questRewards[index].typeId)
		m_pGiftList->AddItem((void*)questRewards[index].typeId);
	else if (index == 7)
	{
		if (m_pSelect)
			m_pSelect->SetVisible(true);
		if (Doc()->m_myInfo.info.gender % 2 == 0)
			m_pGiftList->AddItem((void*)0x800a010);
		else
			m_pGiftList->AddItem((void*)0x8048009);
	}
	m_questIndex = index;
}

void FrQuestGift::SetData(int type)
{
	// 007d1660: 2 itens de gradeGifts por serie; a roupa por genero do 645 nao existe no 983
	switch (type)
	{
	case 0:
		m_pGiftList->AddItem((void*)&gradeGifts[0]);
		m_pGiftList->AddItem((void*)&gradeGifts[1]);
		break;
	case 1:
		m_pGiftList->AddItem((void*)&gradeGifts[2]);
		m_pGiftList->AddItem((void*)&gradeGifts[3]);
		SetSize();
		break;
	}
	m_bDataSet = true;
}

void FrQuestGift::SetSize()
{
	int height = m_pGiftList->GetItemHeight();
	WRect rect = GetRect();
	rect.h += height;
	SetRect(rect);
	rect = m_pGiftList->GetRect();
	rect.h += height;
	m_pGiftList->SetRect(rect);
	WPoint point;
	rect = m_pOk->GetRect();
	point.x = rect.x;
	point.y = rect.y + height;
	m_pOk->MoveWindow(point);
	for (int i = 0; i < 3; ++i)
	{
		rect = m_pItemBack[i]->GetRect();
		rect.h += height + 10;
		m_pItemBack[i]->SetRect(rect);
	}
	for (int i = 3; i < 6; ++i)
	{
		rect = m_pItemBack[i]->GetRect();
		point.x = rect.x;
		point.y = rect.y + (height + 10.0f);
		m_pItemBack[i]->MoveWindow(point);
	}
}
