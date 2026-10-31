#pragma once

#include "frform.h"
class FrValEventDlg : public FrForm
{
	DECLARE_OBJECT(FrValEventDlg)

	FrValEventDlg();
	virtual ~FrValEventDlg();

	virtual bool OnInit();

	void BuildHeartCount();
	void DrawInDlg(const char* name, WPoint& pos);
	void DrawGauge(WPoint& pos, unsigned char count);

	void SetGiftFlag(unsigned long flag)
	{
		m_giftFlag = flag;
		CheckButtonClick();
	}

protected:
	void BindButton(FrButton*& pButton, int param);

	void OnBackImgInit(int param);
	void OnBackImgOwnerDraw(int param);
	void OnGaugeImgOwnerDraw(int param);

	void OnNuriBtnInit(int param);
	void OnNuriBtnLBDown();
	void OnAzerBtnInit(int param);
	void OnAzerBtnLBDown();
	void OnMaxBtnInit(int param);
	void OnMaxBtnLBDown();
	void OnKazBtnInit(int param);
	void OnKazBtnLBDown();
	void OnHanaBtnInit(int param);
	void OnHanaBtnLBDown();
	void OnArinBtnInit(int param);
	void OnArinBtnLBDown();
	void OnCecilBtnInit(int param);
	void OnCecilBtnLBDown();
	void OnKoohBtnInit(int param);
	void OnKoohBtnLBDown();
	void OnLuciaBtnInit(int param);
	void OnLuciaBtnLBDown();
	void OnNellBtnInit(int param);
	void OnNellBtnLBDown();

	unsigned long m_giftFlag;
	unsigned char m_hanaHeart;
	unsigned char m_cecilHeart;
	unsigned char m_koohHeart;
	unsigned char m_arinHeart;
	unsigned char m_luciaHeart;
	unsigned char m_nellHeart;
	unsigned char m_nuriHeart;
	unsigned char m_azerHeart;
	unsigned char m_maxHeart;
	unsigned char m_kazHeart;
	FrButton* m_pHanaBtn;
	FrButton* m_pCecilBtn;
	FrButton* m_pKoohBtn;
	FrButton* m_pArinBtn;
	FrButton* m_pLuciaBtn;
	FrButton* m_pNellBtn;
	FrButton* m_pNuriBtn;
	FrButton* m_pAzerBtn;
	FrButton* m_pMaxBtn;
	FrButton* m_pKazBtn;

private:
	void CheckButtonClick();

	static sFRESH_ENTRY _MsgEntries[];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
};

class NtValentineEventDlg : public FrForm
{
	DECLARE_OBJECT(NtValentineEventDlg)

	NtValentineEventDlg();
	virtual ~NtValentineEventDlg();

	void SetUserData(int valen0, int valen1, int valen2, int valen3);
	void SetUserDataWhite(int white0, int white1, int white2, int white3);

protected:
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnDetailBtnInit(int param);
	void OnDetailBtnUp();
	void OnCaptionInit(int param);
	void OnBg1Init(int param);
	void OnBg2Init(int param);
	void OnBg3Init(int param);
	void OnBg4Init(int param);
	void OnValen0Init(int param);
	void OnValen1Init(int param);
	void OnValen2Init(int param);
	void OnValen3Init(int param);
	void OnValen0OwnerDraw();
	void OnValen1OwnerDraw();
	void OnValen2OwnerDraw();
	void OnValen3OwnerDraw();
	void OnWhite0Init(int param);
	void OnWhite1Init(int param);
	void OnWhite2Init(int param);
	void OnWhite3Init(int param);
	void OnWhite0OwnerDraw();
	void OnWhite1OwnerDraw();
	void OnWhite2OwnerDraw();
	void OnWhite3OwnerDraw();
	bool OnDetailDlgResult(int result, FrForm* form);
	bool OnNotifyDlgResult(int result, FrForm* form);
	void OnCombine0BtnInit(int param);
	void OnCombine0BtnUp();
	void OnCombine1BtnInit(int param);
	void OnCombine1BtnUp();
	void OnCombine2BtnInit(int param);
	void OnCombine2BtnUp();
	void OnCombine3BtnInit(int param);
	void OnCombine3BtnUp();

	tagWTITLEFONT m_fontInfo;
	WTitleFont* m_pFont;
	FrArea* m_pCountArea[8];
	int m_count[8];
	FrButton* m_pCombineBtn[4];

private:
	void UpdateCombinerState();

	static sFRESH_ENTRY _MsgEntries[];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
};

class ntHolesEventDlg : public FrForm
{
	DECLARE_OBJECT(ntHolesEventDlg)

	ntHolesEventDlg();
	virtual ~ntHolesEventDlg();

protected:
	void OnPrize0Init(int param);
	void OnPrize1Init(int param);
	void OnPrize2Init(int param);
	void OnPrize3Init(int param);
	void OnPrize4Init(int param);
	void OnGaugeOwnerDraw();

	tagWTITLEFONT m_fontInfo;
	WTitleFont* m_pFont;

	DECLARE_FRESH_MSGMAP()
};

class ntMightyMACEventDlg : public FrForm
{
public:
	ntMightyMACEventDlg();
	virtual ~ntMightyMACEventDlg();
};
