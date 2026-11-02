#include "minatl.h"
// HACK: emission order, see globalnetworkdefine.h (sStoredItemInfo accessors).
#define REBANG_DEFER_STOREDITEMINFO \
	if (0) \
		((FrWnd*)0)->GetRect();
#define REBANG_ITEMSTORAGE_FULL // HACK: see itemstorage.h
#include "itemstorage.h"
#include "indexcontrol.h"
#include "packet.h"
#include "objectfactory.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frlistbox.h"
#include "fresh.h"
#include "jp/gamestringmanager.h"
#include "jp/rftooltiphelper.h"
#include "frwndmanager.h"
#include "inputmanager.h"
#include "shareddoc.h"
#include "wresrcmng.h"
#include "wfont.h"
#include "frgraphicinterface.h"
#include "../shared/localize.h"
#include "../shared/itemmanager.h"
#include "uccmanager.h"
#include "ucclibrary.h"
#include "taskmanager.h"
#include "binstr.h"
#include "commonutil.h"
#include "wtemplate.h"


extern Fresh* g_pFresh;

const Bitmap* GetUccItemIcon(unsigned long typeId, const char* uccIndex);
const char* GetUccItemName(unsigned long typeId, const char* uccIndex);

void FrItemStorageNotice::OnProc(float)
{
}

void FrItemStorageNotice::OnDraw()
{
}

IMPLEMENT_OBJECT(FrItemStorageNotice, FrForm)

BEGIN_FRESH_MSGMAP(FrItemStorageNotice, FrForm)

ON_FRESH_VV("confirm", FRCMD_LBUTTONDOWN, FrItemStorageNotice::OnLBDownConfirm)
ON_FRESH_VI("print_out", FRCMD_OWNERDRAW,
	FrItemStorageNotice::OnOwnerDrawPrintOut)

END_FRESH_MSGMAP()

void FrItemStorageNotice::OnLBDownConfirm()
{
	Close(true);
}

void FrItemStorageNotice::OnOwnerDrawPrintOut(int param)
{
	const char* lines[7] = { // "Hello, PangYa users!"
		K2L_Compatibility("\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4, \xc6\xce\xbe\xdf \xc0\xaf\xc0\xfa \xbf\xa9\xb7\xaf\xba\xd0!"),
		"",
		// "To prevent the account theft hacks seen often lately"
		K2L_Compatibility("\xc3\xd6\xb1\xd9 \xba\xf3\xb9\xf8\xc7\xcf\xb0\xd4 \xb9\xdf\xbb\xfd\xc7\xcf\xb4\xc2 \xb0\xe8\xc1\xa4\xb5\xb5\xbf\xeb \xc7\xd8\xc5\xb7 \xbb\xe7\xb7\xca\xb8\xa6 \xb9\xe6\xc1\xf6\xc7\xcf\xbf\xa9"),
		// "and let users play PangYa more safely,"
		K2L_Compatibility("\xc0\xaf\xc0\xfa \xbf\xa9\xb7\xaf\xba\xd0\xb2\xb2\xbc\xad \xb4\xf5\xbf\xed \xbe\xc8\xbd\xc9\xc7\xcf\xb0\xed \xc6\xce\xbe\xdf\xb8\xa6 \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xc0\xd6\xb5\xb5\xb7\xcf"),
		"",
		// "we opened the 'Dolfini's secret safe' system!"
		K2L_Compatibility("'\xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xba\xf1\xb9\xd0\xb1\xdd\xb0\xed' \xbd\xc3\xbd\xba\xc5\xdb\xc0\xbb \xbf\xc0\xc7\xc2\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9!"),
		// "To use Dolfini's secret safe safely,"
		K2L_Compatibility("\xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xba\xf1\xb9\xd0\xb1\xdd\xb0\xed\xb8\xa6 \xbe\xc8\xc0\xfc\xc7\xcf\xb0\xd4 \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xc0\xd6\xb5\xb5\xb7\xcf,")
	};

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	WPoint pt(GetRect().x, GetRect().y);

	if (pGDI)
	{
		float x = pt.x + 10.0f;

		for (int i = 0; i < 7; i++)
		{
			pGDI->Print(WPoint(x, (float)i * 15.0f + pt.y + 45.0f), 0,
				lines[i]);
		}
	}

	// "on first use"
	pGDI->Print(WPoint(pt.x + 10.0f, pt.y + 150.0f), 0,
		K2L_Compatibility("\xc3\xd6\xc3\xca \xbb\xe7\xbf\xeb\xbd\xc3"));
	pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	// "set a 4-digit number code"
	pGDI->Print(WPoint(pt.x + 80.0f, pt.y + 150.0f), 0,
		K2L_Compatibility("4\xc0\xda\xb8\xae\xc0\xc7 \xbc\xfd\xc0\xda \xbe\xcf\xc8\xa3\xb8\xa6 \xbc\xb3\xc1\xa4"));
	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(0);
	// "you must"
	pGDI->Print(WPoint(pt.x + 220.0f, pt.y + 150.0f), 0,
		K2L_Compatibility("\xc7\xd8 \xc1\xd6\xbc\xc5\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9."));
}

FrItemStorageAccessPanel::FrItemStorageAccessPanel()
{
	m_formType = (eFormType)0;
	m_inputStep = 1;

	memset(m_password, 0, sizeof(m_password));
	memset(m_newPassword, 0, sizeof(m_newPassword));
	memset(m_confirmPassword, 0, sizeof(m_confirmPassword));

	memset(m_numberSlot, 0, sizeof(m_numberSlot));

	m_pPrintOut = 0;

	memset(m_pNumberSlotBn, 0, sizeof(m_pNumberSlotBn));

	m_pConfirmBn = 0;
	m_pCancelBn = 0;
	m_pOptionBn = 0;

	// Shuffle the digits of the number buttons.
	srand(rand());

	int i;
	for (i = 0; i < 10; i++)
	{
		m_numberSlot[i] = (unsigned char)i;
	}

	for (i = 0; i < 10; i++)
	{
		int other = rand() % 10;

		unsigned char digit = m_numberSlot[i];
		m_numberSlot[i] = m_numberSlot[other];
		m_numberSlot[other] = digit;
	}
}

FrItemStorageAccessPanel::~FrItemStorageAccessPanel()
{
}

// Appends the digit of a number button to the password being typed (4 digits at most).
bool FrItemStorageAccessPanel::InputPassword(unsigned char number)
{
	bool ret = false;
	char digit[32] = "\0";
	char prev[4];
	char* password;

	switch (m_formType)
	{
	case FORMTYPE_FIRST:
		switch (m_inputStep)
		{
		case 1:
		{
			password = m_newPassword;
			int len = strlen(password);

			if (len < 4)
			{
				if (len > 0)
				{
					strcpy(prev, itoa(m_numberSlot[number], digit, 10));
				}
				strcat(password, itoa(m_numberSlot[number], digit, 10));
			}
			else
			{
				ret = true;
			}
			break;
		}
		case 2:
		{
			password = m_confirmPassword;
			int len = strlen(password);

			if (len < 4)
			{
				if (len > 0)
				{
					strcpy(prev, itoa(m_numberSlot[number], digit, 10));
				}
				strcat(password, itoa(m_numberSlot[number], digit, 10));
			}
			else
			{
				ret = true;
			}
			break;
		}
		}
		break;
	case FORMTYPE_INPUT:
	case FORMTYPE_LOCK:
	case FORMTYPE_UNLOCK:
		if ((int)strlen(m_newPassword) < 4)
		{
			strcat(m_newPassword, itoa(m_numberSlot[number], digit, 10));
		}
		else
		{
			ret = true;
		}
		break;
	case FORMTYPE_CHANGE:
		switch (m_inputStep)
		{
		case 0:
			if ((int)strlen(m_password) < 4)
			{
				strcat(m_password, itoa(m_numberSlot[number], digit, 10));
			}
			else
			{
				ret = true;
			}
			break;
		case 1:
		{
			password = m_newPassword;
			int len = strlen(password);

			if (len < 4)
			{
				if (len > 0)
				{
					strcpy(prev, itoa(m_numberSlot[number], digit, 10));
				}
				strcat(password, itoa(m_numberSlot[number], digit, 10));
			}
			else
			{
				ret = true;
			}
			break;
		}
		case 2:
		{
			password = m_confirmPassword;
			int len = strlen(password);

			if (len < 4)
			{
				if (len > 0)
				{
					strcpy(prev, itoa(m_numberSlot[number], digit, 10));
				}
				strcat(password, itoa(m_numberSlot[number], digit, 10));
			}
			else
			{
				ret = true;
			}
			break;
		}
		}
		break;
	}

	if (ret)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		// "The password cannot be longer than 4 digits."
		pForm->SetMessage(
			K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 4\300\332\270\256\270\246 \263\321\300\273 \274\366 \276\370\275\300\264\317\264\331."),
			false);
		pForm->Open((FRESH_PFN_RESULT)0, 9);
	}

	return ret;
}

bool FrItemStorageAccessPanel::DiffPassword()
{
	bool ret = false;

	if (strcmp(m_newPassword, m_confirmPassword) == 0)
	{
		ret = true;
	}

	return ret;
}

void FrItemStorageAccessPanel::SetButtonImage(FrButton* pButton,
	unsigned char number)
{
	if (pButton)
	{
		char normalImage[0x80] = "\0";
		char overImage[0x80] = "\0";

		unsigned char digit = m_numberSlot[number];
		sprintf(normalImage, "%d_n", digit);
		sprintf(overImage, "%d_o", digit);

		pButton->SetStyle(FrButton::BT_3TYPES);
		pButton->SetButtonImg(normalImage, FrButton::NORMAL);
		pButton->SetButtonImg(overImage, FrButton::OVER);
		pButton->SetButtonImg(overImage, FrButton::PRESSED);
	}
}

IMPLEMENT_OBJECT(FrItemStorageAccessPanel, FrForm)

void FrItemStorageAccessPanel::SendPasswordToServer()
{
	switch (m_formType)
	{
	case 0:
	{
		WSendPacket packet((enumClientPacket)0xd0); // JP 983 (s63): 0xd2 no KR 645
		packet.EncodeStr(m_newPassword);
		packet.Send(TO_GAME);
		break;
	}
	case 1:
	{
		WSendPacket packet((enumClientPacket)0xcc); // JP 983 (s63): 0xce no KR 645
		packet.EncodeStr(m_newPassword);
		packet.Send(TO_GAME);
		break;
	}
	case 2:
	{
		WSendPacket packet((enumClientPacket)0xd1); // JP 983 (s63): 0xd3 no KR 645
		packet.EncodeStr(m_password);
		packet.EncodeStr(m_newPassword);
		packet.Send(TO_GAME);
		break;
	}
	case 3:
	{
		WSendPacket packet((enumClientPacket)0xd2); // JP 983 (s63): 0xd4 no KR 645
		packet.Encode1(1);
		packet.EncodeStr(m_newPassword);
		packet.Send(TO_GAME);
		break;
	}
	case 4:
	{
		WSendPacket packet((enumClientPacket)0xd2); // JP 983 (s63): 0xd4 no KR 645
		packet.Encode1(0);
		packet.EncodeStr(m_newPassword);
		packet.Send(TO_GAME);
		break;
	}
	}
}

BEGIN_FRESH_MSGMAP(FrItemStorageAccessPanel, FrForm)

ON_FRESH_VI("print_out", FRCMD_INIT, FrItemStorageAccessPanel::OnInitPrintOut)
ON_FRESH_VI("print_out", FRCMD_OWNERDRAW,
	FrItemStorageAccessPanel::OnOwnerDrawPrintOut)
ON_FRESH_VI("btn_option", FRCMD_INIT, FrItemStorageAccessPanel::OnInitOption983)
ON_FRESH_VI("btn_slot_0", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot0)
ON_FRESH_VI("btn_slot_1", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot1)
ON_FRESH_VI("btn_slot_2", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot2)
ON_FRESH_VI("btn_slot_3", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot3)
ON_FRESH_VI("btn_slot_4", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot4)
ON_FRESH_VI("btn_slot_5", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot5)
ON_FRESH_VI("btn_slot_6", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot6)
ON_FRESH_VI("btn_slot_7", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot7)
ON_FRESH_VI("btn_slot_8", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot8)
ON_FRESH_VI("btn_slot_9", FRCMD_INIT,
	FrItemStorageAccessPanel::OnInitNumberSlot9)
ON_FRESH_VV("btn_option", FRCMD_LBUTTONDOWN, FrItemStorageAccessPanel::OnLBDownOption983)
ON_FRESH_VV("btn_slot_0", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot0)
ON_FRESH_VV("btn_slot_1", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot1)
ON_FRESH_VV("btn_slot_2", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot2)
ON_FRESH_VV("btn_slot_3", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot3)
ON_FRESH_VV("btn_slot_4", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot4)
ON_FRESH_VV("btn_slot_5", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot5)
ON_FRESH_VV("btn_slot_6", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot6)
ON_FRESH_VV("btn_slot_7", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot7)
ON_FRESH_VV("btn_slot_8", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot8)
ON_FRESH_VV("btn_slot_9", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownNumberSlot9)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrItemStorageAccessPanel::OnInitConfirm)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownConfirm)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrItemStorageAccessPanel::OnInitCancel)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN,
	FrItemStorageAccessPanel::OnLBDownCancel)
ON_FRESH_VI("btn_option", FRCMD_HOVERON, FrItemStorageAccessPanel::OnHoverOnOption983)
ON_FRESH_VI("btn_option", FRCMD_HOVEROFF, FrItemStorageAccessPanel::OnHoverOffOption983)

END_FRESH_MSGMAP()

void FrItemStorageAccessPanel::OnInitPrintOut(int param)
{
	TBindControl(m_pPrintOut, param);
}

void FrItemStorageAccessPanel::OnOwnerDrawPrintOut(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		WPoint pt(GetRect().x, GetRect().y);
		const Bitmap* pTextBox = g_pFresh->GetBitmap("text_box");
		const Bitmap* pTip = g_pFresh->GetBitmap("btn_tip_o");

		pGDI->SetTextStyle(1);

		switch (m_formType)
		{
		case FORMTYPE_FIRST:
		{
			int len = strlen(m_newPassword);

			if (m_inputStep == 1 && pTextBox)
			{
				pGDI->DrawTexture(pTextBox,
					WRect(pt.x + 79.0f, pt.y + 30.0f, 88.0f, 21.0f), 0xFFFFFFFF,
					0);
			}

			// "password"
			pGDI->Print(WPoint(pt.x + 20.0f, pt.y + 35.0f), 0,
				K2L_Compatibility("\272\361\271\320\271\370\310\243"));
			if (len > 0 && pTip)
			{
				for (int i = 0; i < len; i++)
				{
					pGDI->DrawTexture(pTip,
						WRect((float)i * 22.0f + pt.x + 82.0f, pt.y + 32.0f,
							16.0f, 16.0f),
						0xFFFFFFFF, 0);
				}
			}

			len = strlen(m_confirmPassword);

			if (m_inputStep == 2 && pTextBox)
			{
				pGDI->DrawTexture(pTextBox,
					WRect(pt.x + 79.0f, pt.y + 55.0f, 88.0f, 21.0f), 0xFFFFFFFF,
					0);
			}

			// "confirm"
			pGDI->Print(WPoint(pt.x + 20.0f, pt.y + 60.0f), 0,
				K2L_Compatibility("\310\256\300\316\300\324\267\302"));
			if (len > 0 && pTip)
			{
				for (int i = 0; i < len; i++)
				{
					pGDI->DrawTexture(pTip,
						WRect((float)i * 22.0f + pt.x + 82.0f, pt.y + 57.0f,
							16.0f, 16.0f),
						0xFFFFFFFF, 0);
				}
			}
			break;
		}
		case FORMTYPE_INPUT:
		case FORMTYPE_LOCK:
		case FORMTYPE_UNLOCK:
		{
			int len = strlen(m_newPassword);

			pGDI->Print(WPoint(pt.x + 20.0f, pt.y + 55.0f), 0,
				K2L_Compatibility("\272\361\271\320\271\370\310\243"));

			if (pTextBox)
			{
				pGDI->DrawTexture(pTextBox,
					WRect(pt.x + 79.0f, pt.y + 50.0f, 88.0f, 21.0f), 0xFFFFFFFF,
					0);
			}

			if (len > 0 && pTip)
			{
				for (int i = 0; i < len; i++)
				{
					pGDI->DrawTexture(pTip,
						WRect((float)i * 22.0f + pt.x + 82.0f, pt.y + 52.0f,
							16.0f, 16.0f),
						0xFFFFFFFF, 0);
				}
			}
			break;
		}
		case FORMTYPE_CHANGE:
			if (m_inputStep != 0)
			{
				// "new password"
				pGDI->Print(WPoint(pt.x + 20.0f, pt.y + 35.0f), 0,
					K2L_Compatibility("\273\365 \272\361\271\320\271\370\310\243"));
				pGDI->Print(WPoint(pt.x + 30.0f, pt.y + 60.0f), 0,
					K2L_Compatibility("\310\256\300\316\300\324\267\302"));

				if (pTextBox)
				{
					if (m_inputStep == 1)
					{
						pGDI->DrawTexture(pTextBox,
							WRect(pt.x + 88.0f, pt.y + 30.0f, 88.0f, 21.0f),
							0xFFFFFFFF, 0);
					}
					else
					{
						pGDI->DrawTexture(pTextBox,
							WRect(pt.x + 88.0f, pt.y + 55.0f, 88.0f, 21.0f),
							0xFFFFFFFF, 0);
					}
				}

				int len = strlen(m_newPassword);

				if (len > 0 && pTip)
				{
					for (int i = 0; i < len; i++)
					{
						pGDI->DrawTexture(pTip,
							WRect((float)i * 22.0f + pt.x + 92.0f, pt.y + 32.0f,
								16.0f, 16.0f),
							0xFFFFFFFF, 0);
					}
				}

				len = strlen(m_confirmPassword);

				if (len > 0 && pTip)
				{
					for (int i = 0; i < len; i++)
					{
						pGDI->DrawTexture(pTip,
							WRect((float)i * 22.0f + pt.x + 92.0f, pt.y + 57.0f,
								16.0f, 16.0f),
							0xFFFFFFFF, 0);
					}
				}
			}
			else
			{
				int len = strlen(m_password);

				// "old password"
				pGDI->Print(WPoint(pt.x + 20.0f, pt.y + 55.0f), 0,
					K2L_Compatibility("\307\366 \272\361\271\320\271\370\310\243"));

				if (pTextBox)
				{
					pGDI->DrawTexture(pTextBox,
						WRect(pt.x + 88.0f, pt.y + 50.0f, 88.0f, 21.0f),
						0xFFFFFFFF, 0);
				}

				if (len > 0 && pTip)
				{
					for (int i = 0; i < len; i++)
					{
						pGDI->DrawTexture(pTip,
							WRect((float)i * 22.0f + pt.x + 92.0f, pt.y + 52.0f,
								16.0f, 16.0f),
							0xFFFFFFFF, 0);
					}
				}
			}
			break;
		}

		pGDI->SetTextStyle(0);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot0(int param)
{
	if (TBindControl(m_pNumberSlotBn[0], param))
	{
		m_pNumberSlotBn[0]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[0], 0);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot1(int param)
{
	if (TBindControl(m_pNumberSlotBn[1], param))
	{
		m_pNumberSlotBn[1]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[1], 1);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot2(int param)
{
	if (TBindControl(m_pNumberSlotBn[2], param))
	{
		m_pNumberSlotBn[2]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[2], 2);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot3(int param)
{
	if (TBindControl(m_pNumberSlotBn[3], param))
	{
		m_pNumberSlotBn[3]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[3], 3);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot4(int param)
{
	if (TBindControl(m_pNumberSlotBn[4], param))
	{
		m_pNumberSlotBn[4]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[4], 4);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot5(int param)
{
	if (TBindControl(m_pNumberSlotBn[5], param))
	{
		m_pNumberSlotBn[5]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[5], 5);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot6(int param)
{
	if (TBindControl(m_pNumberSlotBn[6], param))
	{
		m_pNumberSlotBn[6]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[6], 6);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot7(int param)
{
	if (TBindControl(m_pNumberSlotBn[7], param))
	{
		m_pNumberSlotBn[7]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[7], 7);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot8(int param)
{
	if (TBindControl(m_pNumberSlotBn[8], param))
	{
		m_pNumberSlotBn[8]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[8], 8);
	}
}

void FrItemStorageAccessPanel::OnInitNumberSlot9(int param)
{
	if (TBindControl(m_pNumberSlotBn[9], param))
	{
		m_pNumberSlotBn[9]->SetPushDelay(0.1f);
		SetButtonImage(m_pNumberSlotBn[9], 9);
	}
}

void FrItemStorageAccessPanel::OnLBDownNumberSlot0()
{
	InputPassword(0);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot1()
{
	InputPassword(1);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot2()
{
	InputPassword(2);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot3()
{
	InputPassword(3);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot4()
{
	InputPassword(4);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot5()
{
	InputPassword(5);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot6()
{
	InputPassword(6);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot7()
{
	InputPassword(7);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot8()
{
	InputPassword(8);
}
void FrItemStorageAccessPanel::OnLBDownNumberSlot9()
{
	InputPassword(9);
}

void FrItemStorageAccessPanel::OnInitConfirm(int param)
{
	if (TBindControl(m_pConfirmBn, param))
	{
		m_pConfirmBn->SetPushDelay(0.1f);
	}
}

void FrItemStorageAccessPanel::OnLBDownConfirm()
{
	switch (m_formType)
	{
	case FORMTYPE_FIRST:
		switch (m_inputStep)
		{
		case 1:
		{
			int len = strlen(m_newPassword);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					false);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				m_inputStep = 2;
			}
			break;
		}
		case 2:
		{
			int len = strlen(m_confirmPassword);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					false);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				if (DiffPassword())
				{
					m_inputStep = 3;
					Close(FrOK, true);
				}
				else
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "notify", 0);
					// "The passwords entered differ. Please enter them again."
					pForm->SetMessage(
						K2L_Compatibility("\300\324\267\302 \271\336\300\272 \272\361\271\320\271\370\310\243\260\241 \274\255\267\316 \264\331\270\250\264\317\264\331 \264\331\275\303 \300\324\267\302 \307\330 \301\326\275\312\275\303\277\300."),
						true);
					pForm->Open((FRESH_PFN_RESULT)0, 9);

					m_inputStep = 1;
					memset(m_newPassword, 0, sizeof(m_newPassword));
					memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
				}
			}
			break;
		}
		}
		break;
	case FORMTYPE_INPUT:
	case FORMTYPE_LOCK:
	case FORMTYPE_UNLOCK:
		switch (m_inputStep)
		{
		case 1:
		{
			int len = strlen(m_newPassword);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					true);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				m_inputStep = 3;
				Close(FrOK, true);
			}
			break;
		}
		}
		break;
	case FORMTYPE_CHANGE:
		switch (m_inputStep)
		{
		case 0:
		{
			int len = strlen(m_password);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					false);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				m_inputStep = 1;
			}
			break;
		}
		case 1:
		{
			int len = strlen(m_newPassword);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					false);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				m_inputStep = 2;
			}
			break;
		}
		case 2:
		{
			int len = strlen(m_confirmPassword);

			if (len < 4)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				// "The password must be 4 digits."
				pForm->SetMessage(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\264\302 \271\335\265\345\275\303 4\300\332\270\256\270\246 \300\324\267\302\307\330\276\337 \307\325\264\317\264\331."),
					false);
				pForm->Open((FRESH_PFN_RESULT)0, 9);
			}
			else if (len == 4)
			{
				if (DiffPassword())
				{
					m_inputStep = 3;
					Close(FrOK, true);
				}
				else
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "notify", 0);
					// "The passwords entered differ. Please enter them again."
					pForm->SetMessage(
						K2L_Compatibility("\300\324\267\302 \271\336\300\272 \272\361\271\320\271\370\310\243\260\241 \274\255\267\316 \264\331\270\250\264\317\264\331 \264\331\275\303 \300\324\267\302 \307\330 \301\326\275\312\275\303\277\300."),
						true);
					pForm->Open((FRESH_PFN_RESULT)0, 9);

					m_inputStep = 1;
					memset(m_newPassword, 0, sizeof(m_newPassword));
					memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
				}
			}
			break;
		}
		}
		break;
	}
}

// 983 007f8c50: o "btn_option" (configuracao) do layout comeca escondido; so a entrada de senha o mostra (SetFormType)
void FrItemStorageAccessPanel::OnInitOption983(int param)
{
	if (TBindControl(m_pOptionBn, param))
	{
		m_pOptionBn->SetVisible(false);
		m_pOptionBn->EnableHover(true, 0.0f);
	}
}

// 983 007f8ce0: passa para a troca de senha e esconde o botao e a dica
void FrItemStorageAccessPanel::OnLBDownOption983()
{
	SetFormType(FORMTYPE_CHANGE);
	if (m_pOptionBn)
		m_pOptionBn->SetVisible(false);
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 983 007fd170
void FrItemStorageAccessPanel::OnHoverOnOption983(int param)
{
	if (TBindControl(m_pOptionBn, param))
	{
		const char* s = GetGameString(0x30e6c);
		std::string tip(s ? s : "");
		if (RFTooltipHelper::Instance983())
			RFTooltipHelper::Instance983()->Show983(0, std::string(tip.c_str()), 1, 0xb);
	}
}

// 983 007f8d50
void FrItemStorageAccessPanel::OnHoverOffOption983(int)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

void FrItemStorageAccessPanel::OnInitCancel(int param)
{
	if (TBindControl(m_pCancelBn, param))
	{
		m_pCancelBn->SetPushDelay(0.1f);
	}
}

void FrItemStorageAccessPanel::OnLBDownCancel()
{
	switch (m_formType)
	{
	case 0:
		switch (m_inputStep)
		{
		case 1:
			if (strlen(m_newPassword) != 0)
			{
				memset(m_newPassword, 0, sizeof(m_newPassword));
			}
			else
			{
				Close(FrCANCEL, true);
			}
			break;
		case 2:
			if (strlen(m_confirmPassword) != 0)
			{
				memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
			}
			else
			{
				memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
				m_inputStep = 1;
			}
			break;
		case 3:
			memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
			m_inputStep = 2;
			break;
		}
		break;
	case 1:
	case 3:
	case 4:
		m_inputStep = 1;
		if (strlen(m_newPassword) != 0)
		{
			memset(m_newPassword, 0, sizeof(m_newPassword));
			memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
		}
		else
		{
			Close(FrCANCEL, true);
		}
		break;
	case 2:
		switch (m_inputStep)
		{
		case 0:
			if (strlen(m_password) != 0)
			{
				memset(m_password, 0, sizeof(m_password));
			}
			else
			{
				Close(FrCANCEL, true);
			}
			break;
		case 1:
			if (strlen(m_newPassword) != 0)
			{
				memset(m_newPassword, 0, sizeof(m_newPassword));
			}
			else
			{
				memset(m_newPassword, 0, sizeof(m_newPassword));
				m_inputStep = 0;
			}
			break;
		case 2:
			if (strlen(m_confirmPassword) != 0)
			{
				memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
			}
			else
			{
				memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
				m_inputStep = 1;
			}
			break;
		case 3:
			memset(m_confirmPassword, 0, sizeof(m_confirmPassword));
			m_inputStep = 2;
			break;
		}
		break;
	}
}

FrItemCounter::FrItemCounter()
{
	m_pDrawArea = 0;
	m_pInput = 0;
	m_pConfirmBn = 0;
	m_pCancelBn = 0;
	memset(&m_storedItemInfo, 0, sizeof(m_storedItemInfo));
}

FrItemCounter::~FrItemCounter()
{
}

IMPLEMENT_OBJECT(FrItemCounter, FrForm)

BEGIN_FRESH_MSGMAP(FrItemCounter, FrForm)

ON_FRESH_VI("draw_area", FRCMD_INIT, FrItemCounter::OnInitDrawArea)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW, FrItemCounter::OnOwnerDrawDrawArea)
ON_FRESH_VI("edit_count", FRCMD_INIT, FrItemCounter::OnInitInput)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrItemCounter::OnInitConfirm)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN, FrItemCounter::OnLBDownConfirm)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrItemCounter::OnInitCancel)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN, FrItemCounter::OnLBDownCancel)

END_FRESH_MSGMAP()

void FrItemCounter::OnInitDrawArea(int param)
{
	m_pDrawArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrItemCounter::OnOwnerDrawDrawArea(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		WPoint pt(GetRect().x, GetRect().y);

		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem(m_storedItemInfo.itemInfo.dwTid);

		if (pCommon)
		{
			const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);
			if (pIcon)
			{
				float scale = 100.0f; // percent
				pGDI->DrawTexture(pIcon,
					WRect(pt.x + 35.0f, pt.y + 40.0f,
						(float)pIcon->Width() * 0.01f * scale,
						(float)pIcon->Height() * 0.01f * scale),
					0xFFFFFFFF, 0);
			}

			pGDI->Print(WPoint(pt.x + 100.0f, pt.y + 55.0f), 0, pCommon->Name);
		}

		// "enter the quantity"
		pGDI->Print(WPoint(pt.x + 15.0f, pt.y + 135.0f), 0,
			K2L_Compatibility("\xbc\xf6\xb7\xae\xc0\xd4\xb7\xc2"));
	}
}

void FrItemCounter::OnInitInput(int param)
{
	m_pInput = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrItemCounter::OnInitConfirm(int param)
{
	m_pConfirmBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pConfirmBn)
	{
		m_pConfirmBn->SetPushDelay(0.5f);
	}
}

void FrItemCounter::OnLBDownConfirm()
{
	char text[0x20] = "\0";

	if (m_pInput)
	{
		m_pInput->GetWindowText(text, sizeof(text));
		if (strlen(text) >= 1)
		{
			unsigned long count = atoi(text);

			if (count > 0 && m_storedItemInfo.GetCount() >= count)
			{
				m_storedItemInfo.SetCount(count);
				Close(FrOK, true);
			}
		}
	}
}

void FrItemCounter::OnInitCancel(int param)
{
	m_pCancelBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pCancelBn)
	{
		m_pCancelBn->SetPushDelay(0.5f);
	}
}

void FrItemCounter::OnLBDownCancel()
{
	Close(FrCANCEL, true);
}

void FrItemStoragePangBank::SetType(unsigned long type)
{
	// ORDER: dead reference to a late PCH inline, so this inline is emitted at
	// the end of the main pass as in the original.
	if (0)
	{
		GetRect();
	}
	m_type = type;

	switch (type)
	{
	case TYPE_DEPOSIT:
		SetCaption(K2L_Compatibility("\306\316 \272\270\260\374\307\317\261\342"));
		SetDesc(K2L_Compatibility("\272\270\260\374\307\322 \306\316\300\273 "
				"\300\324\267\302\307\317\274\274\277\344."));
		break;
	case TYPE_WITHDRAW:
		SetCaption(K2L_Compatibility("\306\316 \260\241\301\256\277\300\261\342"));
		SetDesc(K2L_Compatibility("\260\241\301\256\277\303 \306\316\300\273 "
				"\300\324\267\302\307\317\274\274\277\344."));
		break;
	}
}

FrItemStoragePangBank::FrItemStoragePangBank()
{
	m_pDrawArea = 0;
	m_pInput = 0;
	m_pConfirmBn = 0;
	m_pCancelBn = 0;
	m_storedPang = 0;
	m_myPang = 0;
}

FrItemStoragePangBank::~FrItemStoragePangBank()
{
}

IMPLEMENT_OBJECT(FrItemStoragePangBank, FrForm)

BEGIN_FRESH_MSGMAP(FrItemStoragePangBank, FrForm)

ON_FRESH_VI("draw_area", FRCMD_INIT, FrItemStoragePangBank::OnInitDrawArea)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW,
	FrItemStoragePangBank::OnOwnerDrawDrawArea)
ON_FRESH_VI("edit_count", FRCMD_INIT, FrItemStoragePangBank::OnInitInput)
ON_FRESH_BI("edit_count", FRCMD_ENTERKEY,
	FrItemStoragePangBank::OnInputEnterKey)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrItemStoragePangBank::OnInitConfirm)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN,
	FrItemStoragePangBank::OnLBDownConfirm)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrItemStoragePangBank::OnInitCancel)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN,
	FrItemStoragePangBank::OnLBDownCancel)

END_FRESH_MSGMAP()

void FrItemStoragePangBank::OnInitDrawArea(int param)
{
	m_pDrawArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrItemStoragePangBank::OnOwnerDrawDrawArea(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		WPoint pt(GetRect().x, GetRect().y);

		// The pang item.
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(0x1A000010);

		if (pCommon)
		{
			const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);
			if (pIcon)
			{
				pGDI->DrawTexture(pIcon,
					WRect(pt.x + 35.0f, pt.y + 40.0f,
						pIcon->Width() * 0.01f * 100.0f,
						pIcon->Height() * 0.01f * 100.0f),
					0xFFFFFFFF, 0);
			}

			unsigned long pang = 0;

			if (m_pInput)
			{
				char text[32];
				memset(text, 0, sizeof(text));

				m_pInput->GetWindowText(text, sizeof(text));

				int len = strlen(text);
				if (len)
				{
					pang = atoi(text);
				}
			}

			char myPang[32] = "\0";
			char storedPang[32] = "\0";

			if (m_type == TYPE_DEPOSIT)
			{
				if (GetMyPang() - pang < 0)
				{
					// "not enough pang" / "cannot calculate"
					strcat(myPang, K2L_Compatibility("\306\316 \272\316\301\267"));
					strcat(storedPang, K2L_Compatibility("\260\350\273\352 \272\322\260\241"));
				}
				else
				{
					MakePriceStrA(myPang, GetMyPang() - pang);
					MakePriceStrA(storedPang, GetStoredPang() + pang);
				}
			}
			else if (m_type == TYPE_WITHDRAW)
			{
				if (GetStoredPang() - pang < 0)
				{
					strcat(myPang, K2L_Compatibility("\260\350\273\352 \272\322\260\241"));
					strcat(storedPang, K2L_Compatibility("\306\316 \272\316\301\267"));
				}
				else
				{
					MakePriceStrA(myPang, GetMyPang() + pang);
					MakePriceStrA(storedPang, GetStoredPang() - pang);
				}
			}

			// "pang held :" / "pang stored :"
			pGDI->Print(WPoint(pt.x + 100.0f, pt.y + 55.0f), 0,
				K2L_Compatibility("\272\270\300\257 \306\316 :"));
			pGDI->Print(WPoint(pt.x + 100.0f, pt.y + 75.0f), 0,
				K2L_Compatibility("\272\270\260\374 \306\316 :"));

			// The side that changes is printed last, in blue (983 007fb3f0).
			if (m_type == TYPE_DEPOSIT)
			{
				pGDI->Print(WPoint(pt.x + 240.0f, pt.y + 75.0f), 2, "%s P",
					storedPang);

				if (pang > 0)
				{
					pGDI->SetTextColor(0xFF0000FF, 0xFFFFFFFF);
					pGDI->SetTextStyle(1);
				}

				pGDI->Print(WPoint(pt.x + 240.0f, pt.y + 55.0f), 2, "%s P",
					myPang);
			}
			else if (m_type == TYPE_WITHDRAW)
			{
				pGDI->Print(WPoint(pt.x + 240.0f, pt.y + 55.0f), 2, "%s P",
					myPang);

				if (pang > 0)
				{
					pGDI->SetTextColor(0xFF0000FF, 0xFFFFFFFF);
					pGDI->SetTextStyle(1);
				}

				pGDI->Print(WPoint(pt.x + 240.0f, pt.y + 75.0f), 2, "%s P",
					storedPang);
			}

			pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
			pGDI->SetTextStyle(0);
		}

		// "pang amount"
		pGDI->Print(WPoint(pt.x + 15.0f, pt.y + 135.0f), 0,
			K2L_Compatibility("\306\316 \300\324\267\302"));
	}
}


void FrItemStoragePangBank::OnInitInput(int param)
{
	m_pInput = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrItemStoragePangBank::OnInputEnterKey(int param)
{
	OnLBDownConfirm();

	return true;
}

void FrItemStoragePangBank::OnInitConfirm(int param)
{
	m_pConfirmBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pConfirmBn)
	{
		m_pConfirmBn->SetPushDelay(0.5f);
	}
}

void FrItemStoragePangBank::OnLBDownConfirm()
{
	char text[32] = "\0";
	int pang = 0;
	bool bValid = false;

	if (m_pInput)
	{
		m_pInput->GetWindowText(text, 32);
		if (strlen(text) >= 1)
		{
			pang = atoi(text);

			if (m_type == TYPE_WITHDRAW)
			{
				if (pang > 0 && m_storedPang >= pang)
				{
					bValid = true;
				}
			}
			else
			{
				if (pang > 0 && m_myPang >= pang)
				{
					bValid = true;
				}
			}

			if (bValid)
			{
				m_storedPang = pang;
				Close(FrOK, true);
				return;
			}
		}
	}

	__int64 limit = m_type == TYPE_WITHDRAW ? m_storedPang : m_myPang;
	if (pang > limit)
	{
		// "Not enough pang."
		AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
			(int)K2L_Compatibility("\xb1\xdd\xbe\xd7\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
	}
	else if (pang <= 0)
	{
		// "You entered an invalid amount."
		AfxGetTask()->SendMsgToMainActor(MsgObject(0, M983(0x23),
			(int)K2L_Compatibility("\xc0\xdf\xb8\xf8\xb5\xc8 \xb1\xdd\xbe\xd7\xc0\xbb \xc0\xd4\xb7\xc2\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
	}

	m_pInput->ClearLine();
}

void FrItemStoragePangBank::OnInitCancel(int param)
{
	m_pCancelBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pCancelBn)
	{
		m_pCancelBn->SetPushDelay(0.5f);
	}
}

void FrItemStoragePangBank::OnLBDownCancel()
{
	Close(FrCANCEL, true);
}

FrItemConfirm::FrItemConfirm()
{
	m_pDrawArea = 0;
	m_pConfirmBn = 0;
	m_pCancelBn = 0;
	memset(&m_storedItemInfo, 0, sizeof(m_storedItemInfo));
}

FrItemConfirm::~FrItemConfirm()
{
}

IMPLEMENT_OBJECT(FrItemConfirm, FrForm)

BEGIN_FRESH_MSGMAP(FrItemConfirm, FrForm)

ON_FRESH_VI("draw_area", FRCMD_INIT, FrItemConfirm::OnInitDrawArea)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW, FrItemConfirm::OnOwnerDrawDrawArea)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrItemConfirm::OnInitConfirm)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN, FrItemConfirm::OnLBDownConfirm)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrItemConfirm::OnInitCancel)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN, FrItemConfirm::OnLBDownCancel)

END_FRESH_MSGMAP()

void FrItemConfirm::OnInitDrawArea(int param)
{
	m_pDrawArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrItemConfirm::OnOwnerDrawDrawArea(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		WPoint pt(GetRect().x, GetRect().y);

		// The parentheses test the bool of the whole condition, not the type id.
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
			IsUccClothes(m_storedItemInfo.itemInfo.dwTid &&
				(m_storedItemInfo.itemInfo.status & 1)))
		{
			const Bitmap* pUccIcon =
				GetUccItemIcon(m_storedItemInfo.itemInfo.dwTid,
					m_storedItemInfo.itemInfo.UccIndex);

			if (pUccIcon)
			{
				pGDI->DrawTexture(pUccIcon,
					WRect(pt.x + 35.0f, pt.y + 40.0f,
						pUccIcon->Width() * 0.01f * 100.0f,
						pUccIcon->Height() * 0.01f * 100.0f),
					0xFFFFFFFF, 0);
			}

			pGDI->Print(WPoint(pt.x + 110.0f, pt.y + 55.0f), 0,
				GetUccItemName(m_storedItemInfo.itemInfo.dwGuid));
			return;
		}

		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem(m_storedItemInfo.itemInfo.dwTid);

		if (pCommon)
		{
			const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);
			if (pIcon)
			{
				pGDI->DrawTexture(pIcon,
					WRect(pt.x + 35.0f, pt.y + 40.0f,
						pIcon->Width() * 0.01f * 100.0f,
						pIcon->Height() * 0.01f * 100.0f),
					0xFFFFFFFF, 0);
			}

			pGDI->Print(WPoint(pt.x + 110.0f, pt.y + 55.0f), 0, pCommon->Name);
		}
	}
}

void FrItemConfirm::OnInitConfirm(int param)
{
	m_pConfirmBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pConfirmBn)
	{
		m_pConfirmBn->SetPushDelay(0.5f);
	}
}

void FrItemConfirm::OnLBDownConfirm()
{
	if (m_storedItemInfo.GetCount() > 0)
	{
		Close(FrOK, true);
	}
}

void FrItemConfirm::OnInitCancel(int param)
{
	m_pCancelBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pCancelBn)
	{
		m_pCancelBn->SetPushDelay(0.5f);
	}
}

void FrItemConfirm::OnLBDownCancel()
{
	Close(FrCANCEL, true);
}

CItemStorage::CItemStorage()
{
	m_visible = false;
	m_lock = false;

	m_frontUI.clear();
	m_backGroundUI.clear();
	m_backGroundVisibleUI.clear();

	m_pTitleFont = 0;
	m_storedPang = 0;
	m_totalPage = 0;
	m_curPage = 0;
	m_indexControl = 0;

	TSafeNew<CIndexControl>(m_indexControl, 1);
	m_storedItemInfo.clear();

	// The title font file of the storage's numbers.
	static char* fontFile = "[font_wind.jpg";

	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));

	m_pPangBank = 0;
	m_pItemCounter = 0;
	m_pItemConfirm = 0;

	info.filename = &fontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);

	memset(&m_dragDrop, 0, sizeof(m_dragDrop));
}

CItemStorage::~CItemStorage()
{
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = 0;
	}
	TSafeDelete(m_indexControl);
}

void CItemStorage::Lock()
{
	m_lock = true;

	std::list<FrWnd*>::iterator it;
	for (it = m_frontUI.begin(); it != m_frontUI.end(); ++it)
	{
		if (*it)
		{
			(*it)->Enable(false);
		}
	}
}

void CItemStorage::UnLock()
{
	m_lock = false;

	std::list<FrWnd*>::iterator it;
	for (it = m_frontUI.begin(); it != m_frontUI.end(); ++it)
	{
		if (*it)
		{
			(*it)->Enable(true);
		}
	}
}

void CItemStorage::AddFrontUI(FrWnd* pWnd)
{
	if (pWnd)
	{
		pWnd->SetVisible(false);
		m_frontUI.push_back(pWnd);
	}
}

void CItemStorage::AddBackGroundUI(FrWnd* pWnd, bool bVisibleGroup)
{
	if (pWnd)
	{
		if (!bVisibleGroup)
		{
			m_backGroundUI.push_back(pWnd);
		}
		else
		{
			m_backGroundVisibleUI.push_back(pWnd);
		}
	}
}

void CItemStorage::SetVisibleFrontUI(bool bVisible)
{
	std::list<FrWnd*>::iterator it;
	for (it = m_frontUI.begin(); it != m_frontUI.end(); ++it)
	{
		m_visible = bVisible;

		FrWnd* pWnd = *it;
		if (pWnd)
		{
			pWnd->SetVisible(bVisible);
		}
	}
}

void CItemStorage::SetVisibleBackGroundUI(bool bVisible)
{
	std::list<FrWnd*>::iterator it;
	for (it = m_backGroundVisibleUI.begin(); it != m_backGroundVisibleUI.end();
		++it)
	{
		FrWnd* pWnd = *it;
		if (pWnd)
		{
			pWnd->SetVisible(bVisible);
		}
	}
}

void CItemStorage::SetEnableFrontUI(bool bEnable)
{
	std::list<FrWnd*>::iterator it;
	for (it = m_frontUI.begin(); it != m_frontUI.end(); ++it)
	{
		FrWnd* pWnd = *it;
		if (pWnd)
		{
			pWnd->Enable(bEnable);
		}
	}
}

void CItemStorage::SetEnableBackGroundUI(bool bEnable)
{
	std::list<FrWnd*>::iterator it;
	for (it = m_backGroundUI.begin(); it != m_backGroundUI.end(); ++it)
	{
		FrWnd* pWnd = *it;
		if (pWnd)
		{
			pWnd->Enable(bEnable);
		}
	}
}

// Prints text with the title font; align 1 centres it on x, 2 ends it at x.
void CItemStorage::Print(float x, float y, const char* text, float spacing,
	int align, float scale, unsigned long color)
{
	if (!text)
	{
		return;
	}

	if (strlen(text) == 0)
	{
		return;
	}

	const char* charSet = m_pTitleFont->GetFontInfo()->pCharSet;
	float oldScale = m_pTitleFont->GetScale();

	m_pTitleFont->SetScale(scale);
	float width = 0.0f;

	switch (align)
	{
	case 1:
		for (const char* p = text; *p;)
		{
			int ch = *p;

			int index = strchr(charSet, ch) - charSet;
			p++;

			width +=
				m_pTitleFont->GetCharWidth(g_view, index) + spacing * scale;
		}
		width = (width - spacing) * 0.5f;
		break;
	case 2:
		for (const char* p = text; *p;)
		{
			int ch = *p;

			int index = strchr(charSet, ch) - charSet;
			p++;

			width +=
				m_pTitleFont->GetCharWidth(g_view, index) + spacing * scale;
		}
		width -= spacing;
		break;
	}

	x -= width;

	while (*text)
	{
		int ch = *text;

		int index = strchr(charSet, ch) - charSet;
		text++;

		x += m_pTitleFont->PutChar(g_view, x, y, index, 0, color) + spacing;
	}

	m_pTitleFont->SetScale(oldScale);
	m_pTitleFont->Flush(g_view);
}

void CItemStorage::ClearStoredItemInfo()
{
	std::vector<sStoredItemInfo*>::iterator it;
	for (it = m_storedItemInfo.begin(); it != m_storedItemInfo.end(); ++it)
	{
		TSafeDelete(*it);
	}

	std::vector<sItemInfo*>::iterator itItem;
	for (itItem = m_itemInfo.begin(); itItem != m_itemInfo.end(); ++itItem)
	{
		TSafeDelete(*itItem);
	}

	m_storedItemInfo.clear();
}

void CItemStorage::AddStoredItemInfo(const sStoredItemInfo& info)
{
	sStoredItemInfo* pStored = 0;
	TSafeNew<sStoredItemInfo>(pStored, 1);

	if (pStored)
	{
		memcpy(pStored, &info, sizeof(sStoredItemInfo));
		m_storedItemInfo.push_back(pStored);

		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
			IsUccClothes(pStored->itemInfo.dwTid))
		{
			sItemInfo* pItem = new sItemInfo;
			memset(pItem, 0, sizeof(sItemInfo));

			if (pItem)
			{
				pItem->tid = pStored->itemInfo.dwTid;
				pItem->guid = pStored->itemInfo.dwGuid;
				pItem->Seq = pStored->itemInfo.Seq;
				pItem->status = pStored->itemInfo.status;
				memcpy(pItem->ItemName, pStored->itemInfo.ItemName,
					sizeof(pItem->ItemName));
				memcpy(pItem->UccIndex, pStored->itemInfo.UccIndex,
					sizeof(pItem->UccIndex));

				m_itemInfo.push_back(pItem);

				sUccClothes clothes;
				clothes.id = pItem->guid;
				clothes.typeId = pItem->tid;
				strncpy(clothes.uccIndex, pItem->UccIndex, 9);
				strncpy(clothes.uccName, pItem->ItemName, 0x29);
				clothes.downloadCount = !(pStored->itemInfo.status & 1);

				UccManager()->AddClothes(0, clothes, true, false);
			}
		}
	}
}

bool CItemStorage::CheckEquiped(unsigned long id)
{
	bool ret = false;

	std::map<unsigned int, sCharacterInfo>::iterator it;
	for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
	{
		for (int i = 0; i < 24; i++)
		{
			if (it->second.ItemIdList[i] != 0)
			{
				if (it->second.ItemIdList[i] == id)
				{
					return true;
				}
			}
		}
	}

	return ret;
}

// Fills `stored` from an inventory item; only some item categories (the
// type id's top 6 bits) can be stored.
bool CItemStorage::CheckStoredItemInfo(sStoredItemInfo& stored,
	const sItemInfo& info)
{
	bool ret = false;

	switch (info.tid >> 26)
	{
	case 2:
		if (info.ItemType != 0) // 983 007f96f7: test byte [info+0x17], 0x70 (rental/period items)
			return false;
		break;
	case 4:
	case 5:
	case 6:
	case 28:
		break;
	default:
		return false;
	}

	memset(&stored, 0, sizeof(stored));

	stored.SetTypeID(info.tid);
	stored.SetGuid(info.guid);
	strcpy(stored.itemInfo.CopierNick,
		info.CopierNick); // sItemInfo 0x5E holds the UCC copier's nick
	strcpy(stored.itemInfo.UccIndex, info.UccIndex);

	switch (stored.itemInfo.dwTid >> 26)
	{
	case 28:
		if (info.Common[0] > 1)
		{
			ret = false;
			break;
		}
		stored.SetCount(1);
		ret = true;
		break;
	case 2:
		stored.SetCount(1);
		ret = true;
		break;
	case 4:
		// 983 007f977f: tid 0x10000000 is refused, every other type-4 item is stored with count 1
		if (stored.itemInfo.dwTid != 0x10000000)
		{
			stored.SetCount(1);
			ret = true;
		}
		break;
	case 5:
	case 6:
		ret = false;
		break;
	default:
		ret = false;
		break;
	}

	if (ret == true)
	{
		ret = true;
	}
	else
	{
		memset(&stored, 0, sizeof(stored));
	}

	return ret;
}

void CItemStorage::ReqPopStoredItemInfo(const FrListItem& item)
{
	const sStoredItemInfo* pInfo = (const sStoredItemInfo*)item.pData;

	if (pInfo)
	{
		ReqPopStoredItemInfo(*pInfo);
	}
}

void CItemStorage::ReqPopStoredItemInfo(const sStoredItemInfo& info)
{
	if (info.GetCount() > 1)
	{
		if (m_pItemCounter == 0)
		{
			m_pItemCounter = CreateForm<FrItemCounter>(g_pFresh->GetManager(),
				this, "item_storage_item_counter", 0);
		}

		if (m_pItemCounter)
		{
			m_pItemCounter->SetType(1);
			m_pItemCounter->SetStoredItemInfo(info);
			m_pItemCounter->Open(
				(FRESH_PFN_RESULT)&CItemStorage::OnResultItemCounter, 9);
		}
	}
	else if (info.GetCount() == 1)
	{
		if (m_pItemConfirm == 0)
		{
			m_pItemConfirm = CreateForm<FrItemConfirm>(g_pFresh->GetManager(),
				this, "item_storage_item_confirm", 0);
		}

		if (m_pItemConfirm)
		{
			m_pItemConfirm->SetType(1);
			m_pItemConfirm->SetStoredItemInfo(info);
			m_pItemConfirm->Open(
				(FRESH_PFN_RESULT)&CItemStorage::OnResultItemConfirm, 9);
		}
	}
}

void CItemStorage::ReqPushStoredItemInfo(const FrListItem& item)
{
	const sItemInfo* pInfo = (const sItemInfo*)item.pData;

	if (pInfo)
	{
		ReqPushStoredItemInfo(*pInfo);
	}
}

// 983: avisos pelo ator principal (MsgObject 0x23 + texto), sem o form "notify" do 645
static __forceinline void NoticeStorage983(const char* text)
{
	AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23), (int)text, 0, 0, 0, 0);
}

// 983 0063b230: algum cartao encaixado na parte `guid` (lista m_cardAbilityList[0], partsUid)
static __forceinline bool IsCardAttachedParts983(unsigned long guid)
{
	std::list<sSCardAvilityPeriodInfo>& l = Doc()->m_cardAbilityList[0];
	for (std::list<sSCardAvilityPeriodInfo>::iterator it = l.begin(); it != l.end(); ++it)
	{
		if (it->partsUid == guid)
			return true;
	}
	return false;
}

void CItemStorage::ReqPushStoredItemInfo(const sItemInfo& info)
{
	sStoredItemInfo stored;

	if (CheckStoredItemInfo(stored, info) != true)
	{
		NoticeStorage983(K2L_Compatibility("\xba\xb8\xb0\xfc \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb \xc0\xd4\xb4\xcf\xb4\xd9."));
		return;
	}

	if (CheckEquiped(stored.itemInfo.dwGuid))
	{
		NoticeStorage983(K2L_Compatibility("\xc0\xe5\xc2\xf8\xc1\xdf\xc0\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xba\xb8\xb0\xfc \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		return;
	}

	unsigned long itemType = stored.itemInfo.dwTid >> 26;
	bool checkCard = (itemType == 4);

	if (itemType == 2)
	{
		checkCard = true;
		CSharedDoc* pDoc = Doc();
		std::list<sItemInfo>::iterator it = std::find_if(
			pDoc->m_partsList.begin(), pDoc->m_partsList.end(),
			_FindByGuid(stored.itemInfo.dwGuid));
		if (it != pDoc->m_partsList.end() && it->ItemType == 6)
		{
			// "rental clothes cannot be stored"
			NoticeStorage983(K2L_Compatibility("\xb4\xeb\xbf\xa9 \xc0\xc7\xbb\xf3\xc0\xba \xc3\xa2\xb0\xed\xbf\xa1 \xba\xb8\xb0\xfc \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
	}

	if (checkCard && IsCardAttachedParts983(stored.itemInfo.dwGuid))
	{
		// "items with a card attached cannot be stored"
		NoticeStorage983(K2L_Compatibility("\xc4\xab\xb5\xe5\xb0\xa1 \xc0\xe5\xc2\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xba\xb8\xb0\xfc\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		return;
	}

		if (stored.GetCount() > 1)
		{
			if (m_pItemCounter == 0)
			{
				m_pItemCounter =
					CreateForm<FrItemCounter>(g_pFresh->GetManager(), this,
						"item_storage_item_counter", 0);
			}

			if (m_pItemCounter)
			{
				m_pItemCounter->SetType(0);
				m_pItemCounter->SetStoredItemInfo(stored);
				m_pItemCounter->Open(
					(FRESH_PFN_RESULT)&CItemStorage::OnResultItemCounter,
					9);
			}
		}
		else if (stored.GetCount() == 1)
		{
			if (m_pItemConfirm == 0)
			{
				m_pItemConfirm =
					CreateForm<FrItemConfirm>(g_pFresh->GetManager(), this,
						"item_storage_item_confirm", 0);
			}

			if (m_pItemConfirm)
			{
				m_pItemConfirm->SetType(0);
				m_pItemConfirm->SetStoredItemInfo(stored);
				m_pItemConfirm->Open(
					(FRESH_PFN_RESULT)&CItemStorage::OnResultItemConfirm,
					9);
			}
		}
		else if (stored.GetCount() == 0)
		{
			NoticeStorage983(K2L_Compatibility("\xba\xb8\xb0\xfc \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb \xc0\xd4\xb4\xcf\xb4\xd9."));
		}
}

// 983 007f9810: recebe a posicao da area (WPoint, guardada em m_itemStorageArea.x/.y); os quatro
// pedacos do fundo ficam em (pos - (30,80)) e o preco em (pos.x + 245, pos.y - 22)
void CItemStorage::DrawBackgroundImage(const WPoint& pos)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI)
	{
		const Bitmap* pBg1 = g_pFresh->GetBitmap("bg_01");
		const Bitmap* pBg2 = g_pFresh->GetBitmap("bg_02");
		const Bitmap* pBg3 = g_pFresh->GetBitmap("bg_03");
		const Bitmap* pBg4 = g_pFresh->GetBitmap("bg_04");

		m_itemStorageArea.x = pos.x;
		m_itemStorageArea.y = pos.y;
		float ax = m_itemStorageArea.x;
		float ay = m_itemStorageArea.y;

		if (pBg1 && pBg2 && pBg3 && pBg4)
		{
			double x0 = ax - 30.0;
			double y0 = ay - 80.0;

			pGDI->DrawTexture(pBg1,
				WRect((float)x0, (float)y0, (float)pBg1->Width(),
					(float)pBg1->Height()),
				0xFFFFFFFF, 0);
			pGDI->DrawTexture(pBg2,
				WRect((float)(pBg1->Width() + x0), (float)y0,
					(float)pBg2->Width(), (float)pBg2->Height()),
				0xFFFFFFFF, 0);
			pGDI->DrawTexture(pBg3,
				WRect((float)(x0 + 7.0), (float)(pBg1->Height() + y0),
					(float)pBg3->Width(), (float)pBg3->Height()),
				0xFFFFFFFF, 0);
			pGDI->DrawTexture(pBg4,
				WRect((float)(pBg3->Width() + x0 + 7.0),
					(float)(pBg2->Height() + y0), (float)pBg4->Width(),
					(float)pBg4->Height()),
				0xFFFFFFFF, 0);
		}

		char price[32];
		memset(price, 0, sizeof(price));
		MakePriceStrA(price, GetStoredPang());

		Print((float)(ax + 245.0), (float)(ay - 22.0), price, 2.0f, 2, 1.0f,
			0xFFFFD18E);
	}
}

void CItemStorage::MakeStoredItemInfo(FrListBox*& pListBox)
{
	if (pListBox)
	{
		pListBox->ClearItem();

		for (std::vector<sStoredItemInfo*>::iterator it =
				 m_storedItemInfo.begin();
			it != m_storedItemInfo.end(); ++it)
		{
			sStoredItemInfo* pInfo = *it;
			if (pInfo)
			{
				pListBox->AddItem(pInfo);
			}
		}
	}
}

void CItemStorage::DrawStoredItemInfo(const FrListBox*& pListBox,
	const FrListItem& item)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI && pListBox)
	{
		const Bitmap* pBg;

		if (item.selected == false && item.underCursor == false)
		{
			pBg = g_pFresh->GetBitmap("button_n");
		}
		else
		{
			pBg = g_pFresh->GetBitmap("button_o");
		}

		if (pBg)
		{
			pGDI->DrawTexture(pBg,
				WRect(item.pos.x, item.pos.y, (float)pBg->Width(),
					(float)pBg->Height()),
				0xFFFFFFFF, 0);
		}

		const sStoredItemInfo* pInfo = (const sStoredItemInfo*)item.pData;

		if (pInfo)
		{
			IFF_ITEM_COMMON* pCommon =
				ItemManager()->FindCommonItem(pInfo->itemInfo.dwTid);

			if (pCommon)
			{
				// The character the item belongs to.
				IFF_ITEM_COMMON* pChar = ItemManager()->FindCommonItem(
					((pInfo->itemInfo.dwTid >> 18) & 0xFF) | 0x04000000);

				if (pChar)
				{
					const Bitmap* pCharIcon =
						g_pFresh->GetBitmap(MakeStr("s_%s", pChar->Icon));

					if (pCharIcon)
					{
						pGDI->DrawTexture(pCharIcon,
							WRect(item.pos.x + 5.0f, item.pos.y + 3.0f,
								pCharIcon->Width() * 0.01f * 90.0f,
								pCharIcon->Height() * 0.01f * 90.0f),
							0xFFFFFFFF, 0);
					}
				}

				const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);

				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
					IsUccClothes(pInfo->itemInfo.dwTid) &&
					(pInfo->itemInfo.status & 1))
				{
					pIcon = GetUccItemIcon(pInfo->itemInfo.dwTid,
						pInfo->itemInfo.UccIndex);
				}

				unsigned long dragIndex =
					(m_dragDrop.mode == sStoredItemDragDrop::MODE_POP)
					? m_dragDrop.index
					: (unsigned long)-1;

				if (dragIndex != (unsigned long)item.idx && pIcon)
				{
					pGDI->DrawTexture(pIcon,
						WRect(item.pos.x + 30.0f, item.pos.y + 1.0f,
							pIcon->Width() * 0.01f * 34.0f,
							pIcon->Height() * 0.01f * 34.0f),
						0xFFFFFFFF, 0);
				}

				if (item.selected == true)
				{
					pGDI->SetTextStyle(1);
					pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
				}
				else
				{
					pGDI->SetTextStyle(0);
					pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
				}

				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
					IsUccClothes(pInfo->itemInfo.dwTid) &&
					(pInfo->itemInfo.status & 1))
				{
					pGDI->Print(WPoint(item.pos.x + 61.0f, item.pos.y + 10.0f),
						0,
						GetUccItemName(pInfo->itemInfo.dwTid,
							pInfo->itemInfo.UccIndex));
				}
				else
				{
					pGDI->Print(WPoint(item.pos.x + 61.0f, item.pos.y + 10.0f),
						0, pCommon->Name);
				}

				if ((pInfo->itemInfo.dwTid >> 26) != 2)
				{
					pGDI->Print(WPoint(item.pos.x + 220.0f, item.pos.y + 10.0f),
						0, K2L_Compatibility("%d\xb0\xb3"), pInfo->itemInfo.iNum);
				}

				pGDI->SetTextStyle(0);
				pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
			}
		}
	}
}

void CItemStorage::InitIndexControl(unsigned long totalCount,
	unsigned long countPerPage)
{
	if (m_indexControl)
	{
		m_indexControl->Init(totalCount, countPerPage);
		m_indexControl->SetSelectedNumber(m_curPage);
	}
}

void CItemStorage::IndexControlClick()
{
	if (m_indexControl)
	{
		if (m_indexControl->Click() == true)
		{
			unsigned long selectedNumber = m_indexControl->GetSelectedNumber();

			WSendPacket packet((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			packet.Encode4(0x63);
			packet.Encode2((unsigned char)selectedNumber);
			packet.Send(TO_GAME);
			Lock();

			m_curPage = selectedNumber;
		}
	}
}

void CItemStorage::IndexControlNext()
{
	if (m_indexControl)
	{
		if (m_indexControl->NextPage() == true)
		{
			unsigned long selectedNumber = m_indexControl->GetSelectedNumber();

			WSendPacket packet((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			packet.Encode4(0x63);
			packet.Encode2((unsigned char)selectedNumber);
			packet.Send(TO_GAME);
			Lock();

			m_curPage = selectedNumber;
		}
	}
}

void CItemStorage::IndexControlPrev()
{
	if (m_indexControl)
	{
		if (m_indexControl->PrevPage() == true)
		{
			unsigned long selectedNumber = m_indexControl->GetSelectedNumber();

			WSendPacket packet((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			packet.Encode4(0x63);
			packet.Encode2((unsigned char)selectedNumber);
			packet.Send(TO_GAME);
			Lock();

			m_curPage = selectedNumber;
		}
	}
}

void CItemStorage::IndexControlDraw(WPoint& pt)
{
	if (m_indexControl)
	{
		m_indexControl->Draw(pt);
	}
}

bool CItemStorage::NeedPageIndexer()
{
	bool ret = false;

	if (m_indexControl)
	{
		if (m_indexControl->GetTotalPage() > 1)
		{
			ret = true;
		}
	}

	return ret;
}

void CItemStorage::OpenPangBank(FrItemStoragePangBank::eType type)
{
	if (m_pPangBank == 0)
	{
		m_pPangBank = CreateForm<FrItemStoragePangBank>(g_pFresh->GetManager(),
			this, "item_storage_pang_bank", 0);
	}

	if (m_pPangBank)
	{
		m_pPangBank->SetStoredPang(m_storedPang);
		unsigned long myPang = (unsigned long)Doc()->m_myInfo.stat.i64Pang;
		m_pPangBank->SetMyPang(myPang);
		m_pPangBank->SetType(type);
		m_pPangBank->Open((FRESH_PFN_RESULT)&CItemStorage::OnResultPangBank, 9);
	}
}

bool CItemStorage::OnResultPangBank(int ret, FrForm* pForm)
{
	switch (ret)
	{
	case FrOK:
	{
		FrItemStoragePangBank* pPangBank = (FrItemStoragePangBank*)pForm;

		if (pPangBank)
		{
			WSendPacket packet((enumClientPacket)0xd4);

			if (pPangBank->GetType() == FrItemStoragePangBank::TYPE_WITHDRAW)
			{
				packet.Encode1(0);
			}
			else
			{
				packet.Encode1(1);
			}

			packet.Encode8(pPangBank->GetStoredPang());
			packet.Send(TO_GAME);
			Lock();
		}
		break;
	}
	}

	m_pPangBank = 0;

	return true;
}

bool CItemStorage::OnResultItemCounter(int ret, FrForm* pForm)
{
	bool result = false;

	if (ret == FrOK)
	{
		FrItemCounter* pCounter = (FrItemCounter*)pForm;

		if (pCounter)
		{
			sStoredItemInfo info = pCounter->GetStoredItemInfo();

			switch (pCounter->GetType())
			{
			case 1:
			{
				WSendPacket packet((enumClientPacket)0xcf); // JP 983 (s63): 0xd1 no KR 645
				packet.Encode1(1);
				packet.EncodeBuffer(&info, sizeof(info));
				packet.Send(TO_GAME);
				Lock();

				result = true;
				break;
			}
			case 0:
			{
				WSendPacket packet((enumClientPacket)0xce); // JP 983 (s63): 0xd0 no KR 645
				packet.Encode1(1);
				packet.EncodeBuffer(&info, sizeof(info));
				packet.Send(TO_GAME);
				Lock();

				result = true;
				break;
			}
			}
		}
	}
	else
	{
		result = true;
	}

	m_pItemCounter = 0;

	return result;
}

bool CItemStorage::OnResultItemConfirm(int ret, FrForm* pForm)
{
	bool result = false;

	if (ret == FrOK)
	{
		FrItemConfirm* pConfirm = (FrItemConfirm*)pForm;

		if (pConfirm)
		{
			sStoredItemInfo info = pConfirm->GetStoredItemInfo();

			switch (pConfirm->GetType())
			{
			case 1:
			{
				WSendPacket packet((enumClientPacket)0xcf); // JP 983 (s63): 0xd1 no KR 645
				packet.Encode1(1);
				packet.EncodeBuffer(&info, sizeof(info));
				packet.Send(TO_GAME);
				Lock();

				result = true;
				break;
			}
			case 0:
			{
				WSendPacket packet((enumClientPacket)0xce); // JP 983 (s63): 0xd0 no KR 645
				packet.Encode1(1);
				packet.EncodeBuffer(&info, sizeof(info));
				packet.Send(TO_GAME);
				Lock();

				result = true;
				break;
			}
			}
		}
	}
	else
	{
		result = true;
	}

	m_pItemConfirm = 0;

	return result;
}

bool CItemStorage::CheckDrag()
{
	bool ret = false;

	if (g_input->GetButton(LEFT_BUTTON) == 3)
	{
		const WPoint& pt = g_pFresh->GetManager()->GetMousePos();

		m_dragDrop.pos.x = pt.x;
		m_dragDrop.pos.y = pt.y;

		ret = true;
	}

	return ret;
}

bool CItemStorage::CheckDrop()
{
	const WPoint& pt = g_pFresh->GetManager()->GetMousePos();

	if (g_input->GetButton(LEFT_BUTTON) == 2)
	{
		switch (m_dragDrop.mode)
		{
		case sStoredItemDragDrop::MODE_POP:
			if (m_itemListArea.IsInRect(pt) == true)
			{
				ReqPopStoredItemInfo(m_dragDrop.stored);
			}
			break;
		case sStoredItemDragDrop::MODE_PUSH:
			if (m_itemStorageArea.IsInRect(pt) == true)
			{
				if (m_dragDrop.pItemInfo)
				{
					ReqPushStoredItemInfo(*m_dragDrop.pItemInfo);
				}
			}
			break;
		}

		memset(&m_dragDrop, 0, sizeof(m_dragDrop));
	}

	return false;
}

void CItemStorage::SetDragDropItem(sStoredItemDragDrop& dragDrop)
{
	memcpy(&m_dragDrop, &dragDrop, sizeof(m_dragDrop));
}

void CItemStorage::SetDrag(sStoredItemDragDrop::MODE mode)
{
	if (m_dragDrop.stored.GetTypeID())
	{
		if (m_dragDrop.mode == sStoredItemDragDrop::MODE_NONE)
		{
			m_dragDrop.mode = mode;
		}
	}
}

void CItemStorage::DragDropItemDraw()
{
	if (CheckDrag())
	{
		if (m_dragDrop.mode != sStoredItemDragDrop::MODE_NONE)
		{
			FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
			IFF_ITEM_COMMON* pCommon =
				ItemManager()->FindCommonItem(m_dragDrop.stored.itemInfo.dwTid);

			if (pGDI && pCommon)
			{
				const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);

				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/) &&
					IsUccClothes(pCommon->TypeId) &&
					(m_dragDrop.stored.itemInfo.status & 1))
				{
					const Bitmap* pUccIcon =
						GetUccItemIcon(m_dragDrop.stored.itemInfo.dwTid,
							m_dragDrop.stored.itemInfo.UccIndex);

					if (pUccIcon)
					{
						pIcon = pUccIcon;
					}
				}

				if (pIcon)
				{
					float w = (float)pIcon->Width();
					float h = (float)pIcon->Height();

					pGDI->DrawTexture(pIcon,
						WRect(m_dragDrop.pos.x - w * 0.5f,
							m_dragDrop.pos.y - h * 0.5f, w, h),
						0xFFFFFFFF, 0);
				}
			}
		}
	}
	else
	{
		CheckDrop();
	}
}

unsigned long CItemStorage::GetDragingItemIndex(sStoredItemDragDrop::MODE mode)
{
	unsigned long index = (unsigned long)-1;

	if (m_dragDrop.mode == mode)
	{
		index = m_dragDrop.index;
	}

	return index;
}
