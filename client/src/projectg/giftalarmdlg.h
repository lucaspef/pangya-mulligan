#pragma once

#include <map>
#include "frform.h"

class FrListBox;
class FrStatic;

class FrGiftAlarmDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrGiftAlarmDlg)
	FrGiftAlarmDlg();
	virtual ~FrGiftAlarmDlg();

	virtual void OnProc(const float dt);

protected:
	void MakeSenderMsg(const char* msg, char** const lines);

	void OnInfoMsgInit(int param);
	void OnGiftListInit(int param);
	void OnGiftListOwnerDraw(int param);

	std::map<unsigned long, const Bitmap*> m_bitmap;
	FrListBox* m_pGiftList;
	FrStatic* m_pInfoMsg;
	int m_drawIndex;
	int m_giftNum;
	float m_blinkTime;
	const Bitmap* m_pArrivalMail;
	const Bitmap* m_pOpenMail;

private:
	DECLARE_FRESH_MSGMAP()
};

class CTreasureAlarmDlg : public FrForm
{
public:
	DECLARE_OBJECT(CTreasureAlarmDlg)
	CTreasureAlarmDlg();
	virtual ~CTreasureAlarmDlg();

	virtual void OnProc(const float dt);

protected:
	void OnListBox_Gift_Init(int param);
	void OnListBox_Gift_OwnerDraw(int param);
	void OnStatic_Msg_Init(int param);

	FrListBox* m_pGiftList;
	FrStatic* m_pMsg;
	int m_drawIndex;
	int m_giftNum;
	float m_blinkTime;

private:
	DECLARE_FRESH_MSGMAP()
};
